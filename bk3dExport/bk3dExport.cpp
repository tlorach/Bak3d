/*
    Copyright (c) 2013, Tristan Lorach. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Neither the name of its contributors may be used to endorse 
       or promote products derived from this software without specific
       prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
    OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    feedback to lorachnroll@gmail.com (Tristan Lorach)
*/
#define _CRT_SECURE_NO_DEPRECATE

#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - ")
#pragma message("******************************** WARNING******************************************")
#pragma message("Under construction. Still need to work on multiple mesh written in one single file")
#pragma message("Still coding the transition from old style and new style for multiple mesh case")
#pragma message("******************************** WARNING******************************************")
#pragma message("")

#  pragma warning(disable:4786)
#include <algorithm>

#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

int VSPrintf(LPCSTR fmt, ...)
{
	int r = 0;
	size_t fmt2_sz	= 2048;
	char *fmt2		= (char*)malloc(fmt2_sz);
	va_list  vlist;
	va_start(vlist, fmt);
	while((_vsnprintf(fmt2, fmt2_sz, fmt, vlist)) < 0) // means there wasn't anough room
	{
		fmt2_sz *= 2;
		if(fmt2) free(fmt2);
		fmt2 = (char*)malloc(fmt2_sz);
	}
	OutputDebugStringA(fmt2);
	printf(fmt2);
    fflush(stdout);
	free(fmt2);
	return r;
}

bk3dTranslator::bk3dTranslator()
{
    m_bk3dHeader = NULL;
	m_blendShapeNames.clear();
	m_BSDeltaThreshold = 0.0001f;

    /// Name of the MEL script used to setup some params into Maya
    //const char *const tmlOptionScript = "TrimeshExportOptions";
    /// Default options for the Mel script
    //const char *const tmlDefaultOptions = "strips=1;tanbinorm=1;colors=0";
    bAnimCVsAsText = false; //TODO
    bAnimCVsOnly	= false; //TODO
    bTrimBS		= false; //TODO
    bExportAnimCVs = true;
    bKeepQuads     = true; // TODO: add in the settings...
    bDoTriStrips   = false;
    bNoShaders     = false;
    bAdjTris       = false;
    bDeltaBS       = true;
    bMultiIndex    = false;
    //bTransformed   = false is rather buggy for now...
    bTransformed   = true; // true if we want the final mesh after modifier, skinning...
    bAbsPositions  = true; // to write absolute positions. Important for skinning
    bComputeBoundingVolumes = true; // to ask for bounding box/sphere computation
    resetUnits();
}
/*
    BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS 
    BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS 
    BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS 
    BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS BASIC OVERLOADS 
 */
/*******************************************************/ /**
    Creation "factory"
 **/
void* bk3dTranslator::creator()
{
    return new bk3dTranslator();
}

/*******************************************************/ /**
    The reader is not available...
 **/
MStatus bk3dTranslator::reader ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode)
{
    EPF(("bk3dTranslator::reader called in error\n"));
    return MS::kFailure;
}

/*******************************************************/ /**
    Exporting the data in our format happens here.

    Here are the steps :
    -# check options
    -# call bk3dTranslator::exportSelected
    .

 **/
MStatus bk3dTranslator::writer ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode )

{
  MStatus status(MS::kSuccess);
  try {
    MStatus status;
       PF(("===========================================\n"));
       PF(("Writer\n"));
       PF(("===========================================\n"));

    //m_pCurTransform = NULL;
    //m_numTransforms = 0;
    //m_numTransformsByteSz = 0;
    //m_transforms.clear();
	for(int i=0; i<MAXATTRIBS; i++)
	{
		m_attributesIn[i] = AttributeInfo();
		m_attributesInBS[i] = AttributeInfo();
	}
	for(int i=0; i<4; i++)
        m_blindDataInfos[i] = BlindDataInfo();

    m_meshName = file.fullName();
    const char *fname = m_meshName.asChar();
    //
    // Create the bk3d base
    //
    m_bk3dHeader = bk3dlib::FileHeader::Create(m_meshName.asChar());
    //
    //====> Options from MEL script window
    //
    if (options.length() > 0) {
        int i, length;
        // Start parsing.
        MStringArray optionList;
        MStringArray theOption;
        options.split(';', optionList); // break out all the options.

        length = optionList.length();
        for( i = 0; i < length; ++i )
        {
            theOption.clear();
            optionList[i].split( '=', theOption );
              DPF(("%d) option %s\n", i, optionList[i].asChar() ));
            if( theOption[0] == MString("strips") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bDoTriStrips = true;
                else
                    bDoTriStrips = false;
            }
            else if( theOption[0] == MString("exportCVs") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bExportAnimCVs = true;
                else
                    bExportAnimCVs = false;
            }
            else if( theOption[0] == MString("cvsAsText") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bAnimCVsAsText = true;
                else
                    bAnimCVsAsText = false;
            }
            else if( theOption[0] == MString("cvsOnly") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bAnimCVsOnly = true;
                else
                    bAnimCVsOnly = false;
            }
            else if( theOption[0] == MString("trimBS") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bTrimBS = true;
                else
                    bTrimBS = false;
            }
            else if( theOption[0] == MString("noshaders") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bNoShaders = true;
                else
                    bNoShaders = false;
            }
            else if( theOption[0] == MString("adjtris") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bAdjTris = true;
                else
                    bAdjTris = false;
            }
            else if( theOption[0] == MString("deltabs") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bDeltaBS = true;
                else
                    bDeltaBS = false;
            }
            else if( theOption[0] == MString("multiindex") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bMultiIndex = true;
                else
                    bMultiIndex = false;
            }
            else if( theOption[0] == MString("originalmesh") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bTransformed = false;
                else
                    bTransformed = true;
            }
            else if( theOption[0] == MString("worldposmesh") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bAbsPositions = true;
                else
                    bAbsPositions = false;
            }
            else if( theOption[0] == MString("bvols") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bComputeBoundingVolumes = true;
                else
                    bComputeBoundingVolumes = false;
            }
            else if( theOption[0] == MString("keepquads") && theOption.length() > 1 ) 
            {
                if( theOption[1].asInt() > 0 )
                    bKeepQuads = true;
                else
                    bKeepQuads = false;
            }
            else 
            {
              const char *pC = theOption[0].asChar();
              if( strncmp(pC, "attrib", 6) == 0 ) 
              {
                //PF(("attrib[%d] = %d\n", pC[6]-'0', theOption[1].asInt()-1));
                m_attributesIn[pC[6]-'0'].attribute = (AttributeType)(theOption[1].asInt()-1);
              }
              else if( strncmp(pC, "slot", 4) == 0 ) 
              {
                //PF(("slot[%d] = %d\n", pC[4]-'0', theOption[1].asInt()-1));
                m_attributesIn[pC[4]-'0'].slot = (theOption[1].asInt()-1);
              }
              else if( strncmp(pC, "bsattrib", 8) == 0 ) 
              {
                //PF(("attrib[%d] = %d\n", pC[8]-'0', theOption[1].asInt()-1));
                m_attributesInBS[pC[8]-'0'].attribute = (AttributeType)(theOption[1].asInt()-1);
                m_attributesInBS[pC[8]-'0'].slot = 0;
              }
            }
        }
    }

	///////////////////////////////////////////////
	// Curve stuff... TODO: change this
	// TODO: add some options in the exporter pane
#ifdef DOCURVETEXTEXPORT
	MStringArray sa;
	MString file2;
	fp2 = NULL;
	fp3 = NULL;
	file2 = file.fullName();
	file2.split('.', sa);
	if((sa.length() >= 1)&&(bAnimCVsAsText))
	{
		//
		// File for curve keys in text format
		//
		file2 = sa[0];
		file2 += "_CV.txt";
		fname = file2.asChar();
		fp2 = fopen(fname,"w");
		if (fp2 == NULL)
		{
			throw("Error in Curve File creation");
		}
		//
		// File for connections
		//
		file2 = sa[0];
		file2 += "_connections.h";
		fname = file2.asChar();
		fp3 = fopen(fname,"w");
		if (fp3 == NULL)
		{
			throw("Error in Curve File creation");
		}
	}
#endif

	///////////////////////////////////////////////

    if( ( mode == MPxFileTranslator::kExportAccessMode ) ||
        ( mode == MPxFileTranslator::kSaveAccessMode ) )
    {
        //cerr << "Error: Only exporting Selection for this time..." << endl;
        EPF(("WARNING: Only exporting Selection for this time...\n"));
        status = MS::kFailure;
        //exportAll();
    }
    else if( mode == MPxFileTranslator::kExportActiveAccessMode )
    {
        exportSelected();
    }
    //
    // TODO: finalize the bk3d
    //
    //m_meshName = file.fullName();
    //const char *fname = m_meshName.asChar();
    unsigned int sz = 0;
    if(m_bk3dHeader)
    {
        m_bk3dHeader->Cook(file.fullName().asChar(), NULL, &sz);
        m_bk3dHeader->Save(file.fullName().asChar());
    } else {
	  EPF(("Failed to create the data for %s\n", file.fullName().asChar()));
	  status = MS::kFailure;
    }
  }
  catch(char * str)
  {
	  EPF(("NVBinaryMeshExporter Exception raised:\n %s\n", str ? str : ""));
	  status = MS::kFailure;
  }
  catch(...)
  {
	  EPF(("NVBinaryMeshExporter : unkown Exception raised:\n"));
	  status = MS::kFailure;
  }
	m_blendShapeNames.clear();
    m_emptySet.clear();
    m_shaders.clear();
    m_shadersindices.clear();
    m_trianglesets.clear();
    m_idxGroups.clear();

	//m_materials.clear();
	m_meshTransformNames.clear();
    m_DAGNodesProcessed.clear();

	// Curves...
#ifdef DOCURVETEXTEXPORT
	if(fp2) fclose(fp2);
	if(fp3) fclose(fp3);
	m_VectorsProcessed.clear();
	m_CurvesProcessed.clear();
    fp2 = NULL;
    fp3 = NULL;
#endif
	m_NodesProcessed.clear();
	m_CurveVectors.clear();
	m_curvesSorted.clear();

    // cleanup allocated stuff
#define DESTROY(a) if(a) a->Destroy(); a= NULL;
#define DESTROYN(a,n) for(int ii=0; ii<n;ii++) { if(a[ii]) a[ii]->Destroy(); a[ii]=NULL;}
    DESTROY(m_bk3dHeader);
    DESTROY(bk3dSingleIdxBuffer);
    DESTROY(bk3dBufferIdxPosition);
    DESTROY(bk3dBufferIdxColor);
    DESTROY(bk3dBufferIdxNormal);
    DESTROY(bk3dBufferIdxTangent);
    DESTROY(bk3dBufferIdxBinormal);
    DESTROY(bk3dBufferIdxVertexNormal);
    DESTROYN(bk3dBufferIdxTexCoord,4);
    DESTROYN(bk3dBufferIdxBlind,4);

    DESTROY(bk3dBufferPosition);
    DESTROY(bk3dBufferColor);
    DESTROY(bk3dBufferNormal);
    DESTROY(bk3dBufferTangent);
    DESTROY(bk3dBufferBinormal);
    DESTROY(bk3dBufferVertexNormal);
    DESTROYN(bk3dBufferTexCoord,4);
    DESTROYN(bk3dBufferBlind,4);
    DESTROY(bk3dBufferSkinWeights);
    DESTROY(bk3dBufferSkinOffsets);

    DESTROY(bk3dBufferPosition_SIB);
    DESTROY(bk3dBufferColor_SIB);
    DESTROY(bk3dBufferNormal_SIB);
    DESTROY(bk3dBufferTangent_SIB);
    DESTROY(bk3dBufferBinormal_SIB);
    DESTROY(bk3dBufferVertexNormal_SIB);
    DESTROYN(bk3dBufferTexCoord_SIB,4);
    DESTROYN(bk3dBufferBlind_SIB,4);
    DESTROY(bk3dBufferSkinWeights_SIB);
    DESTROY(bk3dBufferSkinOffsets_SIB);

    for(int jj=0; jj<bsBuffers.size(); jj++)
    {
        DESTROY(bsBuffers[jj].bk3dBufferPosition);
        DESTROY(bsBuffers[jj].bk3dBufferNormal);
        DESTROY(bsBuffers[jj].bk3dBufferTangent);
        DESTROY(bsBuffers[jj].bk3dBufferBinormal);
        DESTROY(bsBuffers[jj].bk3dBufferVertexNormal);

        DESTROY(bsBuffers[jj].bk3dBufferPosition_SIB);
        DESTROY(bsBuffers[jj].bk3dBufferNormal_SIB);
        DESTROY(bsBuffers[jj].bk3dBufferTangent_SIB);
        DESTROY(bsBuffers[jj].bk3dBufferBinormal_SIB);
        DESTROY(bsBuffers[jj].bk3dBufferVertexNormal_SIB);
    }
    bk3dlib::DestroyAllObjects();

    PF(("Done\n"));
    fflush(stdout);
  return status;
}
/*******************************************************/ /**
    no, we don't have
 **/
bool bk3dTranslator::haveReadMethod () const
{
    return false;
}
/*******************************************************/ /**
    yes, we have
 **/
bool bk3dTranslator::haveWriteMethod () const
{
    return true;
}

/*******************************************************/ /**
    default extension of the file to be saved
 **/
MString bk3dTranslator::defaultExtension () const
{
    return "bk3d";
}
/*******************************************************/ /**
    
 **/
MPxFileTranslator::MFileKind bk3dTranslator::identifyFile (
                                        const MFileObject& fileName,
                                        const char* buffer,
                                        short size) const
{
    MSTRING name = fileName.name().asChar();
    int   nameLength = strlen(name.asChar());
    
    if ((nameLength > 4) && !strcasecmp(name.asChar()+nameLength-4, ".bk3d"))
        return kCouldBeMyFileType;
    else
        return kNotMyFileType;
}

/*******************************************************/ /**
    PLUGIN Entry point for initializePlugin
 **/
MStatus initializePlugin( MObject obj )
{
	PF(("Initialize Plugin\n"));
    // Let's say version is the same as the NVRawMesh.h so we know this exporter is compatible with the file fmt
    char ver[20];
    MFnPlugin plugin( obj, "(c) Tristan Lorach 2011", ver, "Any");

    // Register the translator with the system
    //@#$!@#$@#$ !!!!! : execute("source \"TrimeshExportOptions.mel\"");
    MStatus s = plugin.registerFileTranslator( "bk3dExporter", "none",
    bk3dTranslator::creator,
    "bk3dExporterOptions",
    "strips=0;"
    "adjtris=0;deltabs=1;noshaders=0;bvols=1;trimBS=0;cvsAsText=0;exportCVs=1;keepquads=0"
    "slot0=0;slot1=0;slot2=0;slot3=0;slot4=0;slot5=0;slot6=0;slot7=0;slot8=0;"
    "attrib0=1;attrib1=0;attrib2=0;attrib3=0;attrib4=0;attrib5=0;attrib6=0;attrib7=0;attrib8=0;"
    "bsattrib0=1;bsattrib1=0;bsattrib2=0;bsattrib3=0;bsattrib4=0;bsattrib5=0;bsattrib6=0;bsattrib7=0;bsattrib8=0;",
    true
    );
    return s;
}

/*******************************************************/ /**
    PLUGIN Entry point for uninitializePlugin
 **/
MStatus uninitializePlugin( MObject obj )
{
	PF(("UninitializePlugin\n"));
        MFnPlugin plugin( obj );
        return plugin.deregisterFileTranslator( "bk3dExporter" );

}

/*******************************************************/ /**

    Export selected is doing the job of iterating through the selected meshes.
    
    Each DAGNode is then processed by bk3dTranslator::readDagPath.
 **/
MStatus bk3dTranslator::exportSelected()
{
    PF(("\n*************************\n"));
    PF(("* Export Selected Items *\n"));
    PF(("*************************\n\n"));
    MStatus status;
    MString filename;

    // Create an iterator for the active selection list
    //
    MSelectionList slist;
    MGlobal::getActiveSelectionList( slist );
    MItSelectionList iter( slist );

    if (iter.isDone()) {
        fprintf(stderr,"Error: Nothing is selected.\n");
        return MS::kFailure;
    }
    // DEBUG :
    PF(("%d Selected Items\n", slist.length()));

    m_DAGNodesProcessed.clear();

    // Selection list loop
    for ( ; !iter.isDone(); iter.next() )
    {     
        MDagPath dagPath;
        iter.getDagPath(dagPath);
        if(!(status = readDagPath(dagPath, 0, true)))
        {
            status.perror("Warning : some Nodes could not be processed");
            //return status;
        }
    }
// TODO?: bk3dTranslator::exportSelected() : Process some added nodes (Skin bones...)
    return status;
}
/******************************************************************************************/ /**
    Recursive reading of the Dag Nodes.
    
    Walking through the nodes and do something when they are MFn::kMesh : call bk3dTranslator::parseMayaMesh.

 **/
MStatus bk3dTranslator::readDagPath(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransformParent)
{
    int numchildren;
    MStatus status;
    MObject object = dagPath.node(&status);
    IFFAILUREMSG("dagPath.node()");

    MFnDagNode dagNode(object);

    MSTRING dagPathDBG = dagPath.fullPathName();
    DPF(("  %s \n", dagPath.partialPathName().asChar()));
    DPF(("dagPath %s \n", dagPathDBG.asChar() ));
    if(m_DAGNodesProcessed.find(dagNode.name()) != m_DAGNodesProcessed.end())
    {
        DPF(("Already done\n"));
        return status;
    }
    m_DAGNodesProcessed.insert(dagNode.name());
    // METTRE EN OPTION METTRE EN OPTION METTRE EN OPTION METTRE EN OPTION
    if((bIntermediate) && dagNode.isIntermediateObject()) 
    {
        DPF(("%s is Intermediate...\n", dagNode.fullPathName().asChar()));
        return status;
    }
    //
    // ====> TRANSFORM
    //
    else if(object.hasFn(MFn::kTransform)) // or kJoint for skinning...
    {
        //
        // Export related curves... if exist
        //
#if 1
        if(bExportAnimCVs)
        {
            exportCurve(object, level);
        }
#endif
        // Case of a Transformation that is also a IK Handle
        // if we see this handle here it means that it is part of the DAG tree we selected
        // We will extract this Handle...
        if( object.hasFn(MFn::kIkHandle))
        {
            DPF(("DAGNODE %s\n", dagNode.fullPathName().asChar()));
            extractIKHandle(dagPath, level, bIntermediate, bk3dTransformParent);
            return status;
        }
        // Case of a Transformation that is also a and-effector for IK Handle
        // End-effector are part of the DAG tree : at the end, most of the time
        // we will find back the connected Handle to be sure we export it, too
        else if( object.hasFn(MFn::kIkEffector))
        {
            DPF(("DAGNODE %s\n", dagNode.fullPathName().asChar()));
            extractIKEffector(dagPath, level, bIntermediate, bk3dTransformParent);
            return status;
        }

		bk3dlib::PBone bk3dTransform = extractTransform(dagPath, level, bIntermediate, NULL, bk3dTransformParent);
        //if(bk3dTransform) // can be null if the transform was processed as a DOF base from extractTransform()
        {
            //----> Process the children
            //MTransform *pCurTransform = m_pCurTransform;
            //m_pCurTransform = m_pCurTransform ? &(m_pCurTransform->children.back()) : &(m_transforms.back());
            numchildren = dagPath.childCount();
            for(int i=0; i< numchildren; i++)
            {
                MObject childobj;
                childobj = dagPath.child( i, &status);
                MDagPath dagPathChild(dagPath);
                dagPathChild.push(childobj);
                status = readDagPath(dagPathChild, level+1, bIntermediate, bk3dTransform);
            }
            //m_pCurTransform = pCurTransform;
        }
        return status;
    }
    // METTRE EN OPTION METTRE EN OPTION METTRE EN OPTION METTRE EN OPTION
    //----> Process this Node
    else if ((  object.hasFn(MFn::kMesh)) &&
             (  object.hasFn(MFn::kTransform)))
    {
        // We want only the shape, 
        // not the transform-extended-to-shape.
        DPF(("skipping Mesh %s : NOT a real shape\n", dagNode.fullPathName().asChar()));
    }
#if 1
    //
    // The LOCATOR for my Custom Degree of Freedom
    //
	else if(  object.hasFn(MFn::kPluginLocatorNode))//kPluginIkSolver))
	{
        DPF(("DAGNODE %s\n", dagNode.fullPathName().asChar()));
		float f;
        // This is the way to detect it...
		if(!getAttributeFloat(object, "dofa", f))
		{
	        DPF(("DAGNODE %s not IK Locator\n", dagNode.fullPathName().asChar()));
			return status;
		}
#if 1
		//
		// Export related curves... if exist
		//
		if(bExportAnimCVs)
		{
			exportCurve(object, level);
		}
#endif
		extractDOF(dagPath, level, bIntermediate, bk3dTransformParent);
	}
#endif
    //
    // MESH MESH MESH MESH MESH MESH MESH MESH MESH
    //
    else if (  object.hasFn(MFn::kMesh))
    {
#if 1
		//
		// Export related curves... if exist
		//
		if(bExportAnimCVs)
		{
			exportCurve(object, level);
		}
#endif
		//
		// Gather the materials used by this mesh
		//
		MFnMesh fnMesh( object, &status );
        // getConnectedShaders :
        // [in]  instanceNumber  The instance number of the mesh to query 
        // [out]  shaders  Storage for set objects (shader objects) 
        // [out]  indices  Storage for indices matching faces to shaders. For each face, this array contains the index into the shaders array for the shader assigned to the face. 
		status = fnMesh.getConnectedShaders ( 0, m_shaders, m_shadersindices );
		int n = m_shaders.length();
		n = m_shadersindices.length();
		DPF2(("material indices: %d indices \n", m_shadersindices.length()));
		m_shaders.length();
		for(int i=0; i<(int)m_shaders.length(); i++)
		{
			MSTRING tname;
			MObject o = getConnectedObject(m_shaders[i], "surfaceShader", &status);
			MFnDependencyNode dn(o, &status);
			tname = dn.name();
            extractMaterial(o);
			//m_materials[std::string(tname)] = MaterialPair(o, NULL);
		}
#if 1
		//
		// Check what we need to add that this mesh would need : transform for skinning...
		//
		MItDependencyGraph dgIter(object,
								  MFn::kSkinClusterFilter,
								  MItDependencyGraph::kUpstream,
								  MItDependencyGraph::kBreadthFirst,
								  MItDependencyGraph::kNodeLevel,
								  &status);
		if(!dgIter.isDone())
		{        
			MSTRING p;
		    MFnSkinCluster skinCluster(dgIter.thisNode(), &status); assert(status);
			MDagPathArray influencePaths;
			int numInfluencePaths;
			numInfluencePaths = skinCluster.influenceObjects( influencePaths, &status );
			DPF2(("Found %d transformations for some Skinning on %s", numInfluencePaths, dagNode.fullPathName().asChar()));
			//m_usedTransforms.clear(); // we don't need any other reference but Bones. So let's clear
			for(int i=0; i<numInfluencePaths; i++)
			{
                MDagPath dagPath2(dagPath);
				influencePaths[i].getPath(dagPath2);
				p = dagPath2.fullPathName();
				dagPath2.pop(dagPath2.length()-1);
				p = dagPath2.fullPathName();
				MItDag dagIter(MItDag::kDepthFirst, MFn::kTransform, &status);
				while(!dagIter.isDone())
				{
					p = dagIter.fullPathName();
					if(dagIter.fullPathName() == dagPath2.fullPathName())
					{
						status = readDagPath(dagPath2, level+1, false);
					}
					dagIter.next();
				}
			}
		}
#endif
        // extract the Mesh
        DPF(("parseMayaMesh on dagPath %s \n", dagPathDBG.asChar() ));
        parseMayaMesh(dagPath, level, false);
    } // Mesh
    //
    // CURVE CURVE CURVE CURVE CURVE CURVE CURVE CURVE
    //
#if 1
    else if (  object.hasFn(MFn::kNurbsCurve) && bExportAnimCVs)
    {
        // TODO: TEST
        if(!(status = exportCurve(object, level)))
            return status;
    }
	else if (  object.hasFn(MFn::kAnimCurve) && bExportAnimCVs)
	{
        // TODO: TEST
#if 0
		m_curvesSorted[dagPath.partialPathName()] = CurveAndOwner(object, object);
		ProcessSortedCurves();
#endif
	}
#endif
    //----> Process the children
    numchildren = dagPath.childCount();
    for(int i=0; i< numchildren; i++)
    {
        MObject childobj;
        childobj = dagPath.child( i, &status);
        MDagPath dagPathChild(dagPath);
        dagPathChild.push(childobj);
        status = readDagPath(dagPathChild, level+1, bIntermediate);
    }
    return status;
}//bk3dTranslator::readDagPath

/******************************************************************************************/ /**
    Read Mesh data and export them in our format.

    -# allocate & init the FileHeader
    -# get vtx array, normals array etc. and write them : extractPointTable, extractColorTable,
    extractNormalsTable, extractUVTable
    -# walk through the shaders to create some primitive groups into the mesh.
    -# bk3dTranslator::extractTriIndicesTable will write the indice tables depending on the triangles
    -# bk3dTranslator::ComputeSingleIndex will then turn the multiple index element in a single index element.
    -# create primitive groups : bk3dTranslator::makePrimGroups
    -# if needed, call TriStrip to optimize the way to call the primitives : bk3dTranslator::doTriStrips
    .
 **/
MStatus bk3dTranslator::parseMayaMesh(MDagPath &dagPath, int level, bool bIntermediate)
{
	PF2(("ParseMayaMesh\n"));

	int i;
    AttributeInfo *attr;
    bk3dlib::PBuffer bk3dBuffer;
    MObjectArray sets, comps;
    MFloatArray uArray[4], vArray[4];
    MIntArray    uvCounts[4], uvIds[4];

    MFloatVectorArray normals, tangents, binormals;
    MPointArray vertexArray;
    MColorArray colors, facecolors;

    MStringArray setNames;

    MStatus status;
    MStatus status2;
    MFnMesh fnMesh( dagPath, &status );
    MFnMesh fnMeshOrig;
	MObject meshOrig;

    if ( !status ) 
    {
        status.perror ("MFnMesh not here");
        return status;
    }
	MSTRING meshname = fnMesh.name();
    PF(( "====> Exporting Mesh %s (%s) <====\n", meshname.asChar(), bTransformed ? "Transformed" : "before transformations"));

    bk3dlib::PMesh bk3dMesh;
    bk3dMesh = m_bk3dHeader->FindMesh(meshname.asChar());
    if(bk3dMesh)
    {
        // NOTE: instances of the same Mesh is now not really supported :
        // only solution : reference many transforms... and this is conflicting with skinning
        // However this is not enough : what about connected curves (if different anims for each instance) ?
        // instances could have little variations...
        // so another solution : create a Mesh but share the same buffers with previous one...
        // OR create a special Mesh or sub-section : instanced mesh...
        // in any case instancing must be taken into account
        PF(("Mesh already there\n"));
        MDagPath parentPath(dagPath);
        parentPath.pop();
        MSTRING pname = parentPath.partialPathName();
        bk3dlib::PBone parentTransf = m_bk3dHeader->GetTransform(pname.asChar());
        if(!parentTransf)
        {
            PF(("Error : can't find the parent transformation %s", pname.asChar()));
            return status;
        }
        PF(("TODO: trasnform added... warning because skin transform (TRANSFCOMP_isBone) are at the same place\n"));
        bk3dMesh->AddTransformReference(parentTransf);
        return status;
    }
    bk3dMesh = bk3dlib::Mesh::Create(meshname.asChar());
    m_bk3dHeader->AttachMesh(bk3dMesh);

    MDagPath parentPath(dagPath);
    parentPath.pop();
    MSTRING pname = parentPath.partialPathName();
    bk3dlib::PBone parentTransf = m_bk3dHeader->GetTransform(pname.asChar());
    if(parentTransf)
        bk3dMesh->AddTransformReference(parentTransf);
    else {
        PF(("Error : can't find the parent transformation %s", pname.asChar()));
    }

    //
    // If we want the mesh before transformation, we need to get back to the Orig one
    //
    status = fnMeshOrig.setObject(dagPath);
    // TODO
    meshOrig = dagPath.node();
    //if(!bTransformed)
    //{
    //    meshOrig = getConnectedObjectType(dagPath.node(), "tweakLocation", MFn::kMesh, true, false, 10, &status);
    //    if(status)
    //    {
    //        status = fnMeshOrig.setObject(meshOrig);
    //        DPF(( "  Found Original Mesh %s. Using this one instead\n", fnMesh.name().asChar()));
    //    }
    //    else 
    //    {
    //        status = fnMeshOrig.setObject(dagPath);
    //        EPF(( "  WARNINGL Cannot find Original Mesh %s\n", fnMesh.name().asChar()));
    //    }
    //}

    m_trianglesets.clear();
    m_bsnames.clear();

    m_emptySet.clear();
    m_shaders.clear();
    m_shadersindices.clear();
    m_idxGroups.clear();

    bk3dSingleIdxBuffer = NULL;
    bk3dBufferIdxPosition = NULL;
    bk3dBufferIdxColor = NULL;
    bk3dBufferIdxNormal = NULL;
    bk3dBufferIdxTangent = NULL;
    bk3dBufferIdxBinormal = NULL;
    bk3dBufferIdxVertexNormal = NULL;
    for(int i=0; i<4; i++)
    {
        bk3dBufferIdxTexCoord[i] = NULL;
        bk3dBufferIdxBlind[i] = NULL;
        bk3dBufferTexCoord[i] = NULL;
        bk3dBufferBlind[i] = NULL;
        bk3dBufferTexCoord_SIB[i] = NULL;
        bk3dBufferBlind_SIB[i] = NULL;
    }
    bk3dBufferPosition = NULL;
    bk3dBufferColor = NULL;
    bk3dBufferNormal = NULL;
    bk3dBufferTangent = NULL;
    bk3dBufferBinormal = NULL;
    bk3dBufferVertexNormal = NULL;

    bk3dBufferSkinWeights = NULL;
    bk3dBufferSkinOffsets = NULL;

    bk3dBufferPosition_SIB = NULL;
    bk3dBufferColor_SIB = NULL;
    bk3dBufferNormal_SIB = NULL;
    bk3dBufferTangent_SIB = NULL;
    bk3dBufferBinormal_SIB = NULL;
    bk3dBufferVertexNormal_SIB = NULL;
    bk3dBufferSkinWeights_SIB = NULL;
    bk3dBufferSkinOffsets_SIB = NULL;

    bsBuffers.clear();

    int nuvs = fnMeshOrig.numUVs( &status );
    int nnorm = fnMeshOrig.numNormals( &status );
    //
    // VERTEX TABLE
    //
    // change reference for if(bAbsPositions && bTransformed)
    status = fnMeshOrig.getPoints( vertexArray, MSpace::kObject);
    IFFAILUREMSG("Failed at fnMeshOrig.getPoints()");
    attr = findAttrInfo(Position);
    assert(attr);
    if(!bMultiIndex) {
        bk3dBufferPosition_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_POSITION, 3, attr->slot);
        bk3dMesh->AttachVtxBuffer(bk3dBufferPosition_SIB);
    }
    bk3dBufferPosition = bk3dlib::Buffer::CreateVtxBuffer(MESH_POSITION, 3, attr->slot);
    bk3dMesh->AttachVtxBuffer(bk3dBufferPosition);
    status = extractPointTable(bk3dBufferPosition, vertexArray, MESH_POSITION);
    IFFAILUREMSG("failed at extractPointTable");
    //
    // VertexID TABLE
    // To be computed after SIB compilation
    //
    //attr = findAttrInfo(VertexID);
    //if(attr)
    //{
    //    bk3dBufferVtxID = bk3dMesh->CreateVtxBuffer(MESH_VERTEXID, 3, attr->slot);
    //    status = extractVertexIDs(bk3dBuffer, MESH_VERTEXID);
    //    IFFAILURE();
    //}
    //
    // COLOR TABLE
    //
    attr = findAttrInfo(Color);
    if(attr)
    {
        if(!bMultiIndex) {
            bk3dBufferColor_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_COLOR, 4, attr->slot);
            bk3dMesh->AttachVtxBuffer(bk3dBufferColor_SIB);
        }
        bk3dBufferColor = bk3dlib::Buffer::CreateVtxBuffer(MESH_COLOR, 4, attr->slot);
        bk3dMesh->AttachVtxBuffer(bk3dBufferColor);
        status = fnMeshOrig.getFaceVertexColors ( facecolors);
        //IFFAILUREMSG("Failed at fnMeshOrig.getFaceVertexColors()");
        status = extractColorTable(bk3dBufferColor, facecolors);
        IFFAILURE();
    }
    //
    // PER FACE NORMALS
    //
    attr = findAttrInfo(FaceNormal);
    if(attr)
    {
        if(!bMultiIndex) {
            bk3dBufferNormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_FACENORMAL, 3, attr->slot);
            bk3dMesh->AttachVtxBuffer(bk3dBufferNormal_SIB);
        }
        bk3dBufferNormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_FACENORMAL, 3, attr->slot);
        bk3dMesh->AttachVtxBuffer(bk3dBufferNormal);
        status = fnMeshOrig.getNormals ( normals, MSpace::kObject);// kWorld );
        IFFAILUREMSG("Failed at fnMeshOrig.getNormals()");
        status = extractNormalsTable(bk3dBufferNormal, normals, MESH_FACENORMAL);
        IFFAILURE();
    }
    //
    // VERTEX NORMALS TABLE
    //
    attr = findAttrInfo(Normal);
    if(attr)
    {
        if(!bMultiIndex) {
            bk3dBufferVertexNormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_NORMAL, 3, attr->slot);
            bk3dMesh->AttachVtxBuffer(bk3dBufferVertexNormal_SIB);
        }
        bk3dBufferVertexNormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_NORMAL, 3, attr->slot);
        bk3dMesh->AttachVtxBuffer(bk3dBufferVertexNormal);
        status = extractVertexNormalsTable(bk3dBufferVertexNormal, fnMeshOrig, MESH_NORMAL, -1, MObject());
        IFFAILURE();
    }
    //
    // TANGENTS TABLE
    //
    if(findAttrInfo(Tangent) || findAttrInfo(Binormal))
    {
        if(fnMeshOrig.numUVSets() > 0)
        {
          MStringArray  uvsets;
          fnMeshOrig.getUVSetNames(uvsets);
          status = fnMeshOrig.getTangents(  tangents,  MSpace::kObject, &(uvsets[0]) ); // take the first uvset
          if(status)
          {
              attr = findAttrInfo(Tangent);
              if(attr)
              {
                  if(!bMultiIndex) {
                      bk3dBufferTangent_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_TANGENT, 3, attr->slot);
                      bk3dMesh->AttachVtxBuffer(bk3dBufferTangent_SIB);
                  }
                  bk3dBufferTangent = bk3dlib::Buffer::CreateVtxBuffer(MESH_TANGENT, 3, attr->slot);
                  bk3dMesh->AttachVtxBuffer(bk3dBufferTangent);
                  status = extractTanBinormTables(bk3dBufferTangent, tangents, MESH_TANGENT);
              }
              attr = findAttrInfo(Binormal);
              if(attr)
              {
                  if(!bMultiIndex) {
                      bk3dBufferBinormal_SIB = bk3dlib::Buffer::CreateVtxBuffer(MESH_BINORMAL, 3, attr->slot);
                      bk3dMesh->AttachVtxBuffer(bk3dBufferBinormal_SIB);
                  }
                  bk3dBufferBinormal = bk3dlib::Buffer::CreateVtxBuffer(MESH_BINORMAL, 3, attr->slot);
                  bk3dMesh->AttachVtxBuffer(bk3dBufferBinormal);
                  status = fnMeshOrig.getBinormals(  binormals,  MSpace::kObject, &(uvsets[0]) ); // take the first uvset
                  status = extractTanBinormTables(bk3dBufferBinormal, binormals, MESH_BINORMAL);
                  IFFAILURE();
              }
          }
          else DPF(("Failed at fnMeshOrig.getTangents()"));
        }
        else EPF(("WARNING : no UVSets. Cannot compute Tangents\n"));
    }
    //
    // BLINDVALUES
    //
    //attr = findAttrInfo();
    //if(attr)
    //{
    //    if(!bMultiIndex) bk3dBuffer = bk3dMesh->CreateVtxBuffer(MESH_, 3, attr->slot);
    //    bk3dBufferBlind = bk3dMesh->CreateVtxBuffer(MESH_BLIND, 3, attr->slot);
    //    status = gatherBlindData(bk3dBuffer, fnMeshOrig, vertexArray.length());
    //    IFFAILUREMSG("Failed at gatherBlindData()");
    //}
    //
    // UVSets UVSets UVSets UVSets UVSets UVSets UVSets
    //
    status = fnMeshOrig.getUVSetNames ( setNames);
    IFFAILUREMSG("Failed at fnMeshOrig.getUVSetNames()");
    unsigned int nuvsets = fnMeshOrig.numUVSets(&status);
    if(nuvsets > 4)
    {
        EPF(("WARNING : too many UVSets (%d)", nuvsets));
        nuvsets = 4;
    }
    for(i=0; i<(int)nuvsets; i++)
    {
        attr = findAttrInfo((AttributeType)(TexCoord0 + i) );
        if(attr)
        {
            static char str[] = MESH_TEXCOORD0;
            str[strlen(str)-1] = '0'+i;
            if(!bMultiIndex) {
                bk3dBufferTexCoord_SIB[i] = bk3dlib::Buffer::CreateVtxBuffer(str, 2, attr->slot);
                bk3dMesh->AttachVtxBuffer(bk3dBufferTexCoord_SIB[i]);
            }
            bk3dBufferTexCoord[i] = bk3dlib::Buffer::CreateVtxBuffer(str, 2, attr->slot);
            bk3dMesh->AttachVtxBuffer(bk3dBufferTexCoord[i]);
            MObjectArray textures;
            DPF2(("UVSet %d : %s\n", i, setNames[i].asChar()));
            status = fnMeshOrig.getAssociatedUVSetTextures (setNames[i], textures);
            status = fnMeshOrig.getUVs( uArray[i], vArray[i], &(setNames[i]));
            status = extractUVTable(bk3dBufferTexCoord[i], i, uArray[i], vArray[i]);
            IFFAILURE();
            status = fnMeshOrig.getAssignedUVs ( uvCounts[i], uvIds[i], &(setNames[i]));
        }
    }
    // ??
    int nv = fnMeshOrig.numVertices(&status);
    int ne = fnMeshOrig.numEdges( &status );
    unsigned int np = fnMeshOrig.numPolygons( &status );
    //
    // Bounding Volumes
    //
    //computeAABB();
    //computeBSphere();
    //
    // TriangleSet TriangleSet TriangleSet TriangleSet TriangleSet TriangleSet
    //
    //
    // Enumerate connected shaders : will generate primitive groups
    //

#if 1
	// we cannot use fnMeshOrig : the original mesh doesn't have the shaders...
    status = fnMesh.getConnectedShaders ( 0, m_shaders, m_shadersindices );
    int n = m_shaders.length();
    n = m_shadersindices.length();
	DPF2(("material indices: %d indices / shaders : %d\n", m_shadersindices.length(), m_shaders.length()));
    m_shaders.length();
    for(i=0; i<(int)m_shaders.length(); i++)
    {
		LPCSTR tname = m_shaders[i].apiTypeStr();
        MObject o = getConnectedObject(m_shaders[i], "surfaceShader", &status);
		tname = o.apiTypeStr();
        m_shaders[i] = o; // replace the object with the material
        MFnLambertShader lambertshd(o, &status);
        if(status)
        {
          //lambertshd.attribute(
          DPF2(("LAMBERT Ok..."));
        }
        MFnDependencyNode shd(o, &status);
        if(status)
        {
            DPF2(("material %s \n", shd.name().asChar(), shd.typeName().asChar()));
        }
    }
#else
    status = fnMesh.getConnectedSetsAndMembers ( 0, sets, comps, false);//renderableSetsOnly
    if(sets.length() != comps.length())
    {
        EPF(( "\t WARNING: sets != comps !!!! %d != %d\n", sets.length(), comps.length()));
    }
    int numshaders = sets.length() < comps.length() ? sets.length() : comps.length();
    int curshader = 0;
    for(i=0; i<numshaders; i++)
    {
        if(sets[i].apiType() != MFn::kShadingEngine)
            continue;
        MFnComponent compnode(comps[i], &status);
        if(status)
        {
            MFnDependencyNode depnode(sets[i], &status);
            if(!status)
                continue;
            DPF(("\tSet[%d] = %s of %s\n", i, depnode.name().asChar(), depnode.typeName().asChar()));
            MFn::Type type = compnode.type();        
            if(compnode.isEmpty())
                continue;
            MFnSingleIndexedComponent sngcomp(comps[i], &status);
            if(!status)
                continue;
        }
    }
#endif
    //
    // Blendshapes
	// Don't take the original mesh : it may not be connected to the blendshapes...
    //
    ManageBlendshapes(dagPath, bk3dMesh);
    //
    // Use the current tranform as the one on which ths mesh is referencing
    // Then potential Skinning will overhide this. Normal because other transforms get ignored when in skinning...
    //
    //m_usedTransforms.clear();
    //if(meshTransformName.length() > 0)
    //    m_usedTransforms.push_back(meshTransformName);
    //
    // Skin. If so, then the related tranforms will be the bones. Previous one will be removed.
    //
    ManageSkinning(dagPath, bk3dMesh);
    //
    // POLYGONS POLYGONS POLYGONS POLYGONS POLYGONS POLYGONS
    //
    //
    // Note: can modify the triangle sets and m_shadersindices : new triangles from quad split.
    // returns the # of idx per poly. For now it means that a mesh can only export one kind of polygon (triangle, quad)
    // No way to mix.
    // TODO: add the possible case where quads, triangles and even lines are mixed
    //
    extractTriIndicesTable(bk3dMesh, meshOrig, np, nuvsets, setNames, uvCounts);
    //
    // In adjacent triangles mode
    //
    //ConvertTrisToAdjTris();
    //
    // Now we create new tables in order to have only one index for each vertex of a triangle
    //
    if(!bMultiIndex)
    {
        bk3dSingleIdxBuffer = bk3dlib::Buffer::CreateIdxBuffer("Index", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dSingleIdxBuffer);
        //bk3dSingleIdxBuffer->SIB_ClearBuffers(); // no need... just got created
        if(bk3dBufferIdxPosition) {
            bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxPosition, bk3dBufferPosition, bk3dBufferPosition_SIB);
            if(bk3dBufferSkinOffsets) bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxPosition, bk3dBufferSkinOffsets, bk3dBufferSkinOffsets_SIB);
            if(bk3dBufferSkinWeights) bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxPosition, bk3dBufferSkinWeights, bk3dBufferSkinWeights_SIB);
        }
        if(bk3dBufferIdxColor)          bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxColor, bk3dBufferColor, bk3dBufferColor_SIB);
        if(bk3dBufferIdxNormal)         bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxNormal, bk3dBufferNormal, bk3dBufferNormal_SIB);
        if(bk3dBufferIdxTangent)        bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxTangent, bk3dBufferTangent, bk3dBufferTangent_SIB);
        if(bk3dBufferIdxBinormal)       bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxBinormal, bk3dBufferBinormal, bk3dBufferBinormal_SIB);
        if(bk3dBufferIdxVertexNormal)   bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxVertexNormal, bk3dBufferVertexNormal, bk3dBufferVertexNormal_SIB);
        // TODO: skin weights and offset
        for(int i=0; i<4; i++)
        {
            if(bk3dBufferIdxTexCoord[i]) bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxTexCoord[i], bk3dBufferTexCoord[i], bk3dBufferTexCoord_SIB[i]);
            if(bk3dBufferIdxBlind[i]) bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxBlind[i], bk3dBufferBlind[i], bk3dBufferBlind_SIB[i]);
        }
        // Now work on possible blendshapes : they also need to be re-organized
        for(int bs=0; bs<bsBuffers.size(); bs++)
        {
            // NOTE: re-use the idx buffers of the original geometry... is it a good thing ??
            // TO CHECK !
            if(bk3dBufferIdxPosition)       bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxPosition, bsBuffers[bs].bk3dBufferPosition, bsBuffers[bs].bk3dBufferPosition_SIB);
            if(bk3dBufferIdxNormal)         bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxNormal, bsBuffers[bs].bk3dBufferNormal, bsBuffers[bs].bk3dBufferNormal_SIB);
            if(bk3dBufferIdxTangent)        bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxTangent, bsBuffers[bs].bk3dBufferTangent, bsBuffers[bs].bk3dBufferTangent_SIB);
            if(bk3dBufferIdxBinormal)       bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxBinormal, bsBuffers[bs].bk3dBufferBinormal, bsBuffers[bs].bk3dBufferBinormal_SIB);
            if(bk3dBufferIdxVertexNormal)   bk3dSingleIdxBuffer->SIB_AddBuffers(bk3dBufferIdxVertexNormal, bsBuffers[bs].bk3dBufferVertexNormal, bsBuffers[bs].bk3dBufferVertexNormal_SIB);
        }

        bk3dSingleIdxBuffer->SIB_Compile();

#define DETACHDESTROY(a) if(a) { bk3dMesh->DetachBuffer(a); a->Destroy(); a = NULL; }
        DETACHDESTROY(bk3dBufferIdxPosition)
        DETACHDESTROY(bk3dBufferIdxColor)
        DETACHDESTROY(bk3dBufferIdxNormal)
        DETACHDESTROY(bk3dBufferIdxTangent)
        DETACHDESTROY(bk3dBufferIdxBinormal)
        DETACHDESTROY(bk3dBufferIdxVertexNormal)

        DETACHDESTROY(bk3dBufferPosition)
        DETACHDESTROY(bk3dBufferColor)
        DETACHDESTROY(bk3dBufferNormal)
        DETACHDESTROY(bk3dBufferTangent)
        DETACHDESTROY(bk3dBufferBinormal)
        DETACHDESTROY(bk3dBufferVertexNormal)
        for(int i=0; i<4; i++)
        {
            DETACHDESTROY(bk3dBufferTexCoord[i])
            DETACHDESTROY(bk3dBufferBlind[i])
            DETACHDESTROY(bk3dBufferIdxTexCoord[i])
            DETACHDESTROY(bk3dBufferIdxBlind[i])
        }
        DETACHDESTROY(bk3dBufferSkinOffsets)
        DETACHDESTROY(bk3dBufferSkinWeights)
    }
    //
    // make primitive groups
    //
    makePrimGroups(dagPath, bk3dMesh);
    //
    // Bounding volumes computed from the position buffer
    // done after prim-group creation : B-volumes are done for prim groups, too
    //
    bk3dMesh->ComputeBoundingVolumes(bMultiIndex ? bk3dBufferPosition : bk3dBufferPosition_SIB);
    //
    // Tris stripifier
    //
    if(bDoTriStrips)
    {
        //TODO : not implemented in the bk3d library, yet.
    }
    //
    // Update the connections of the curves. m_curvesSorted has them with owner info and component to which they are connected
    //
    int N = m_bk3dHeader->GetNumCurves();
    for(int n=0; n<N; n++)
    {
        bk3dlib::CurveVec *pCVV = m_bk3dHeader->GetCurveVec(n);
        LPCSTR name = pCVV->GetName();
        MString mname(name);
        MStringArray sarray;
        mname.split('_', sarray);
        if(sarray.length() > 2)
        {
            EPF(("Don't name a curve with '_' (%s)!\n", name));
        }
        if(sarray.length() == 2)
        {
            if(sarray[0] == dagPath.partialPathName())
            {
                if(sarray[1] == MString("visibility"))
                {
                    pCVV->Connect(bk3dMesh, bk3dlib::MESH_VISIBILITY);
                }
                else
                {
                    EPF(("Cannot connect component : %s\n", sarray[1].asChar()));
                }
            }
        }
    }
    return status;
}

/******************************************************************************************/ /**

    create various primitive groups on the basis of shaders connected to the mesh : 
    each shader will make a primitive group.

    If we didn't attach any shader to this mesh, a default primitive group will be created.

    NOTE: for now, all groups must have the SAME primitive type.
    TODO: allow to have a mix of quads / triangles

**/ /******************************************************************************************/
MStatus bk3dTranslator::makePrimGroups(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh)
{
	PF2(("makePrimGroups\n"));
    MStatus status;
    int npg;
    int nTriangles;
    MItMeshPolygon iPoly(dagPath.node(), &status);
    if(!status)
        return status;
    int shadersindices = m_shadersindices.length();
    int nPolygons = iPoly.count();
    assert(nPolygons == shadersindices );
    //
    // search for the indices
    //
    // m_shaders : Storage for set objects (shader objects) 
    npg = bNoShaders ? 1 : m_shaders.length();
    if(npg > 0)
    {
        // TODO: later we may want to have the option of many components (many indices for pos, normal etc)
        bk3dlib::PBuffer idxBuffer = bk3dlib::Buffer::CreateIdxBuffer("shader-ordered", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(idxBuffer);
        // create buckets of idx for each shader and for triangle and quads
        // triangles are the first ones, quads are the following 'npg' ones
        std::vector<unsigned int> *pIdxBuckets = new std::vector<unsigned int>[npg*2];
        // run through polys and put them in the right shader bucket
        // m_shadersindices : Storage for indices matching faces to shaders. For each face, this array contains the index into the shaders array for the shader assigned to the face. 
        unsigned int offsitem = 0;
        //if(bAdjTris)
        //{
        //    assert(!"bAdjTris TODO");
        //    // TODO
        //}
        //else 
          for(int i=0; i<shadersindices; i++)
          {
              int shdi = bNoShaders ? 1 : m_shadersindices[i];
				if(shdi < 0)
				{
					PF(("!"));
					shdi = 0;
				}
            // we really need to know about the # of triangles to land on our feet
            status = iPoly.numTriangles(nTriangles);
            if((nTriangles == 2) && bKeepQuads) // QUAD : we may want to keep them
            {
                for(int k=0; k<4; k++)
                {
                  MPointArray points;
                  MIntArray vertexList;
                  // we don't need and don't want to read anything here : the correct indices are the ones
                  // we re-computed in bk3dSingleIdxBuffer. This bk3dSingleIdxBuffer MUST match ! Or it's a bug
                  //status = iPoly.getTriangle( j, points, vertexList, MSpace::kObject);
                  unsigned int idx;
                  bk3dSingleIdxBuffer->GetData(&idx, offsitem, 1);
                  // store in the buckets for Quads
                  pIdxBuckets[npg + shdi].push_back(idx);
                  offsitem++; // increment the access to the idx in bk3dSingleIdxBuffer
                }
            }
            else // when no Quad, we assume things got turned to triangles or were already triangles
              for(int j=0; j< nTriangles; j++)
                for(int k=0; k< 3; k++)
                {
                  MPointArray points;
                  MIntArray vertexList;
                  // we don't need and don't want to read anything here : the correct indices are the ones
                  // we re-computed in bk3dSingleIdxBuffer. This bk3dSingleIdxBuffer MUST match ! Or it's a bug
                  //status = iPoly.getTriangle( j, points, vertexList, MSpace::kObject);
                  unsigned int idx;
                  bk3dSingleIdxBuffer->GetData(&idx, offsitem, 1);
                  pIdxBuckets[shdi].push_back(idx);
                  offsitem++; // increment the access to the idx in bk3dSingleIdxBuffer
                }
            iPoly.next();
          }

        // create a new index buffer and append all so we will have one Elements buffer for all the PGs
        unsigned int sz = 0;
        for(int i=0; i<npg*2; i++)
            sz += pIdxBuckets[i].size();
        unsigned int * orderedIndices = new unsigned int[sz];
        std::vector<unsigned int> offsetElement;
        std::vector<unsigned int> numElements;
        sz = 0;
        for(int i=0; i<npg*2; i++)
        {
            offsetElement.push_back(sz);
            numElements.push_back(pIdxBuckets[i].size());
            for(int j=0; j<pIdxBuckets[i].size(); j++)
                orderedIndices[j + sz] = pIdxBuckets[i][j];
            //memcpy(orderedIndices + sz, &pIdxBuckets[i], sizeof(unsigned int)*pIdxBuckets[i].size());
            sz += pIdxBuckets[i].size();
        }
        delete [] pIdxBuckets;
        idxBuffer->AddData(orderedIndices, sz);
        delete [] orderedIndices; orderedIndices = NULL;

        // Create the primitive group
        for(int i=0; i<npg*2; i++)
        {
            if(numElements[i] == 0)
                continue;
            MFnDependencyNode shd(m_shaders[i%npg], &status);
            MSTRING name = shd.name(); // is it clean ?
            bk3dlib::PMaterial pMat = m_bk3dHeader->GetMaterial(i%npg);
            bk3dMesh->CreatePrimGroup(name.asChar(), idxBuffer, i < npg ? bk3dlib::TRIANGLES : bk3dlib::QUADS, pMat, offsetElement[i], numElements[i]);
            //DPF(("shader %s having %d polygons (%d triangle(s))\n", name, nTriangles));
        }
        // release the previous idx buffer and replace it with the new one
        bk3dSingleIdxBuffer->Destroy();
        bk3dSingleIdxBuffer = idxBuffer;
    }
    return status;
}


AttributeInfo    *bk3dTranslator::findAttrInfo(AttributeType t, bool inBS)
{
    if(!inBS)
        for(int i=0; i<MAXATTRIBS; i++)
        {
            if(m_attributesIn[i].attribute == t)
                return m_attributesIn + i;
        }
    else
        for(int i=0; i<MAXATTRIBS; i++)
        {
            if(m_attributesInBS[i].attribute == t)
                return m_attributesInBS + i;
        }
    // ? m_blindDataInfos[4];
    return NULL;
}
