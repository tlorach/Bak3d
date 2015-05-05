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

/*----------------------------------------------------------------------------------*/ /**
  \file TrimeshExport.cpp
  Exporter in a binary format.
**/ //----------------------------------------------------------------------------------
#define DOCURVETEXTEXPORT

#pragma warning(disable: 4786)
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4996) // 'strncpy': This function or variable may be unsafe
#include <vector> 
#include <set> 
#include <map> 
#include <stack> 

#include <assert.h> 
//#include <iostream.h> 
//#include <fstream.h>
#include <string.h> 

//#include "nvIO/NVMeshMender.h" 

#include <maya/MPxFileTranslator.h>

#include <sys/types.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnIkHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MItDag.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MObjectArray.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnComponent.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MFnTripleIndexedComponent.h>
#include <maya/MItDag.h>
#include <maya/MPlugArray.h>
#include <maya/MFnGeometryFilter.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MItGeometry.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MAngle.h>
#include <maya/MTime.h>
#include <maya/MDistance.h>
#include <maya/MFnIkJoint.h>

#include <maya/MFnLambertShader.h>
#include <maya/MFnReflectShader.h>
#include <maya/MFnPhongShader.h>

#include <string>

#include "bk3dEx.h"
#include "bk3dLib.h"

using namespace std;

#ifdef _DEBUG
class MSTRING : public MString
{
public:
    const char * dbgStr;
    MSTRING() : MString() {}
    MSTRING( const MString& other ) : MString(other)
    {
        dbgStr = asChar();
    }
	MSTRING( const char * other ) : MString(other)
    {
        dbgStr = asChar();
    }
	MSTRING&	operator =  ( const MString& other )
    {
        MString::operator =  ( other );
        dbgStr = asChar();
        return *this;
    }
	MSTRING&	operator =  ( const char * other )
    {
        MString::operator =  ( other );
        dbgStr = asChar();
        return *this;
    }
};
#else
#define MSTRING MString
#endif

#define OFFSETFOR(type, ptr) (type*)((char*)ptr - (char*)m_fileheader)

#define FAILURE(msg)\
  {\
    status.perror(msg);\
    return status;\
  }
#define IFFAILUREMSG(msg)\
  if(!status)\
  {\
    status.perror(msg);\
    return status;\
  }
#define IFFAILURE()\
  if(!status)\
  {\
    return status;\
  }


#define MAXATTRIBS 9
#define MAXSLOTS 16
// Must match the order in the mel script !!
enum AttributeType 
{
  None = 0,
  Position,
  Color,
  Normal,
  Tangent,
  Binormal,
  FaceNormal,
  TexCoord0,
  TexCoord1,
  TexCoord2,
  TexCoord3,
  BonesOffsets,
  BonesWeights,
  VertexID,
  Blind0,
  Blind1,
  Blind2,
  Blind3
};
struct AttributeInfo
{
	AttributeInfo() : attribute(None), slot(0) {}
  AttributeType attribute;
  int           slot;
};
struct BlindDataInfo
{
	BlindDataInfo() : formatDXGI(DXGI_FORMAT_UNKNOWN), formatDX9(D3DDECLTYPE_UNDEF) { name[0] = '\0'; }
  char          name[64];
  DXGI_FORMAT   formatDXGI;
  D3DDECLTYPE   formatDX9;
};
struct VertexAttribute
{
    typedef std::vector< int > IntVector;
    typedef std::vector< float > FloatVector;
    std::string		m_Name;
	AttributeType	m_type;
	int				m_attributePos; // where in the list is this attribute
    IntVector		m_intVector;
    FloatVector		m_floatVector;
    int				m_numComponents;

    VertexAttribute& operator=( const VertexAttribute& rhs )
    {
        m_attributePos = rhs.m_attributePos;
        m_Name   = rhs.m_Name;
        m_intVector = rhs.m_intVector;
        m_floatVector = rhs.m_floatVector;
        m_numComponents = rhs.m_numComponents;
		m_type = rhs.m_type;
        return *this;
    }
    VertexAttribute(  ) : m_numComponents(0), m_attributePos(0), m_type(None) {}
    VertexAttribute( const char* pName ) : m_Name(pName), m_attributePos(0), m_type(None) {m_numComponents=1;}
    VertexAttribute( const char* pName, int nc ) : m_Name(pName), m_attributePos(None), m_type(None) {m_numComponents=nc;}
    VertexAttribute( const VertexAttribute& rhs )
    {
        *this = rhs;
    }
    bool operator==( const VertexAttribute& rhs )
    {
        return ( m_Name == rhs.m_Name );
    }
};
typedef std::vector< VertexAttribute > VAVector;
/*
  PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS 
  PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS 
  PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS 
  PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS PLUGIN CLASS 
 */
/*----------------------------------------------------------------------------------*/ /**
  Translator class for Maya.
  This object does all the work : it is considered as a translator available when exporting 
  the selection or all the scene.
 **/ /*----------------------------------------------------------------------------------*/
class bk3dTranslator : public MPxFileTranslator
{
private:
    bk3dlib::PFileHeader m_bk3dHeader;
    /// Name of the MEL script used to setup some params into Maya
    //const char *const tmlOptionScript = "TrimeshExportOptions";
    /// Default options for the Mel script
    //const char *const tmlDefaultOptions = "strips=1;tanbinorm=1;colors=0";
    bool bAnimCVsAsText; //TODO
    bool bAnimCVsOnly; //TODO
    bool bTrimBS; //TODO
    bool bExportAnimCVs;
    bool bKeepQuads; // TODO: add in the settings...
    bool bDoTriStrips;
    bool bNoShaders;
    bool bAdjTris;
    bool bDeltaBS;
    bool bMultiIndex;
    //bTransformed;
    bool bTransformed; // true if we want the final mesh after modifier, skinning...
    bool bAbsPositions; // to write absolute positions. Important for skinning
    bool bComputeBoundingVolumes; // to ask for bounding box/sphere computation

    bk3dlib::PBuffer bk3dSingleIdxBuffer;
    bk3dlib::PBuffer bk3dBufferIdxPosition;
    bk3dlib::PBuffer bk3dBufferIdxColor;
    bk3dlib::PBuffer bk3dBufferIdxNormal;
    bk3dlib::PBuffer bk3dBufferIdxTangent;
    bk3dlib::PBuffer bk3dBufferIdxBinormal;
    bk3dlib::PBuffer bk3dBufferIdxVertexNormal;
    bk3dlib::PBuffer bk3dBufferIdxTexCoord[4];
    bk3dlib::PBuffer bk3dBufferIdxBlind[4];

    bk3dlib::PBuffer bk3dBufferPosition;
    bk3dlib::PBuffer bk3dBufferColor;
    bk3dlib::PBuffer bk3dBufferNormal;
    bk3dlib::PBuffer bk3dBufferTangent;
    bk3dlib::PBuffer bk3dBufferBinormal;
    bk3dlib::PBuffer bk3dBufferVertexNormal;
    bk3dlib::PBuffer bk3dBufferTexCoord[4];
    bk3dlib::PBuffer bk3dBufferBlind[4];
    bk3dlib::PBuffer bk3dBufferSkinWeights; // for now we only deal with a limited amount of items
    bk3dlib::PBuffer bk3dBufferSkinOffsets; // for now we only deal with a limited amount of items

    bk3dlib::PBuffer bk3dBufferPosition_SIB;
    bk3dlib::PBuffer bk3dBufferColor_SIB;
    bk3dlib::PBuffer bk3dBufferNormal_SIB;
    bk3dlib::PBuffer bk3dBufferTangent_SIB;
    bk3dlib::PBuffer bk3dBufferBinormal_SIB;
    bk3dlib::PBuffer bk3dBufferVertexNormal_SIB;
    bk3dlib::PBuffer bk3dBufferTexCoord_SIB[4];
    bk3dlib::PBuffer bk3dBufferBlind_SIB[4];
    bk3dlib::PBuffer bk3dBufferSkinWeights_SIB; // for now we only deal with a limited amount of items
    bk3dlib::PBuffer bk3dBufferSkinOffsets_SIB; // for now we only deal with a limited amount of items

    // for Blendshapes : # will depend on how many BS we have...
    struct BS_Buffers {
        BS_Buffers() { memset(this, 0, sizeof(BS_Buffers)); }
        bk3dlib::PBuffer bk3dBufferPosition;
        bk3dlib::PBuffer bk3dBufferNormal;
        bk3dlib::PBuffer bk3dBufferTangent;
        bk3dlib::PBuffer bk3dBufferBinormal;
        bk3dlib::PBuffer bk3dBufferVertexNormal;

        bk3dlib::PBuffer bk3dBufferPosition_SIB;
        bk3dlib::PBuffer bk3dBufferNormal_SIB;
        bk3dlib::PBuffer bk3dBufferTangent_SIB;
        bk3dlib::PBuffer bk3dBufferBinormal_SIB;
        bk3dlib::PBuffer bk3dBufferVertexNormal_SIB;
    };
    std::vector<BS_Buffers> bsBuffers;
public:
                    bk3dTranslator ();
    virtual         ~bk3dTranslator () {};
    static void*    creator();

    MStatus         reader ( const MFileObject& file,
                             const MString& optionsString,
                             FileAccessMode mode);

    MStatus         writer ( const MFileObject& file,
                             const MString& optionsString,
                             FileAccessMode mode );
    bool            haveReadMethod () const;
    bool            haveWriteMethod () const;
    MString         defaultExtension () const;
    MFileKind       identifyFile ( const MFileObject& fileName,
                                   const char* buffer,
                                   short size) const;
public:
	// things related to internal update process
	// Comparison class for the set of m_DAGNodesProcessed, using MString
	struct ltstr
	{
		bool operator()(const MString s1, const MString s2) const
		{
		  return strcmp(s1.asChar(), s2.asChar()) < 0;
		}
	};
	struct ltstr_inv
	{
	  bool operator()(const MString s1, const MString s2) const
	  {
	    return strcmp(s2.asChar(), s1.asChar()) < 0;
	  }
	};

  bk3d::AABBox      m_aabb;
  bk3d::BSphere     m_bsphere;

  set<MString, ltstr>   m_DAGNodesProcessed; // set of Nodes which are actually processed.
  MString               m_meshName;  // mesh name
  int                   m_cntmesh;
  float					m_BSDeltaThreshold;
  vector<MString>       m_bsnames;  // blendshape names
  // the body of the real data to write (FileHeader)
  int                   m_numNodes; // # of nodes in the file we write
  vector<MString>		m_blendShapeNames;
  unsigned long         m_meshOffset; // to reconstruct m_pMeshHeader when realloc()
  unsigned int          m_offs; // temp counter when feeding the file data
  int                   m_numbones; // how many bones we have in the attributes
  AttributeInfo         m_attributesIn[MAXATTRIBS];
  AttributeInfo         m_attributesInBS[MAXATTRIBS]; // attributes to pass to Blendshape part
  BlindDataInfo         m_blindDataInfos[4];

    //
	// structure for temporary storage so we can write them into the bk3d pool...
	//
	struct MayaCurve2 : public bk3d::MayaCurve
	{
		std::vector<bk3d::MayaReadKey> m_keys;
	};
	struct MayaCurveVector2 : public bk3d::MayaCurveVector
	{
		int idx; // idx in the list...
		MObject owner;
		MString component;
		std::vector<MayaCurve2> m_cvs;
	};

  // Some internal/intermediate structures
  // Index group, gathering index arrays for triangles, strips and fans
  struct IdxGroup
  {
        D3D10_PRIMITIVE_TOPOLOGY    primitiveTypeDX10;
        D3DPRIMITIVETYPE            primitiveTypeDX9;
        GLTopology                  primitiveTypeGL;
        std::vector<unsigned int>   elements;
        bk3d::AABBox            aabb;
        bk3d::BSphere           bsphere;
        const char *                shaderName;
  };
  /**
    Structure gathering various indices for vtx, norm, color and texcoords.
    Maya is providing various indices for all these various attributes, while in D3D & OpenGL we just
    have one index for all of them. Although Maya is avoiding redundancy, this isn't what we want.
    So we must convert thes multi index as a single index.
    This is used in WriteTriIndicesTable and findmatch
   **/
  struct VtxIndices
  {
        int vtx, norm, tanbinorm, color, uv[4], bd[4];
  };
  typedef std::set< unsigned int >  IdxSet;
  typedef std::vector< IdxSet >     IdenticalComp;
  // array of multiple index structures, gathered from Maya and to convert in a single index mode.
  std::vector<VtxIndices>   m_tris_multiidx;
  std::vector< std::vector<unsigned int> > m_trianglesets;
  IdxSet                    m_emptySet;
  MObjectArray              m_shaders; // shaders used in the mesh. This is used to create as many Primitive groups as shaders.
  MIntArray                 m_shadersindices; // shader indices giving us which shaders are connected to the mesh
  std::vector< IdxGroup >   m_idxGroups;

  MStatus         exportCurve(MObject curveobj, int level, bool bIntermediate=false);
  MStatus		  findDOFLocator(const MDagPath &p, MDagPath &PathDOFLocator);
  MStatus         exportSelected();
  MStatus         readDagPath(MDagPath &dagPath, int level, bool bIntermediate=false, bk3dlib::PBone bk3dTransformParent = NULL);
  MStatus         gatherBlindData(MFnMesh &fnMesh, unsigned int size);
  MStatus         extractPointTable(bk3dlib::PBuffer bk3dBuffer, MPointArray &vertexArray, const char *name, int bsnum = -1, MPointArray *vertexArrayRef=NULL);
  MStatus         extractColorTable(bk3dlib::PBuffer bk3dBuffer, MColorArray &colorArray);
  MStatus         extractNormalsTable(bk3dlib::PBuffer bk3dBuffer, MFloatVectorArray &normals, const char *name, int bsnum = -1, MFloatVectorArray *normalsRef=NULL);
  MStatus         extractVertexNormalsTable(bk3dlib::PBuffer bk3dBuffer, MFnMesh &fnMesh, const char *name, int bsnum, MObject meshobjRef);
  MStatus         extractTanBinormTables(bk3dlib::PBuffer bk3dBuffer, MFloatVectorArray &table, const char *name, int bsnum=-1, MFloatVectorArray *tableRef=NULL);
  MStatus         extractSimpleTriangleSetTable(bk3dlib::PBuffer bk3dBuffer, int np);
  MStatus         extractTriangleSetTable(bk3dlib::PBuffer bk3dBuffer, MFnSet &set);//ingleIndexedComponent &sngcomp);
  MStatus         extractTriIndicesTable(bk3dlib::PMesh bk3dMesh, MObject obj, unsigned int np, int nuvsets, MStringArray &setNames, MIntArray *uvCounts);
  MStatus         extractVertexIDs(bk3dlib::PBuffer bk3dBuffer, const char *name, int bsnum=-1);
  bk3dlib::PMaterial extractMaterial(MObject o);
  MStatus         extractUVTable(bk3dlib::PBuffer bk3dBuffer, int idx, MFloatArray &uArray, MFloatArray &vArray);
  MStatus         ManageBlendshapes(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh);
  MStatus         ManageSkinning(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh);
  MStatus         ConvertTrisToAdjTris();
  MStatus         ComputeSingleIndex();
  MStatus         ComputeMultiIndex();
  MStatus         computeAABB();
  MStatus         computeAABB_IdxGroup();
  MStatus         computeBSphere();
  AttributeInfo  *findAttrInfo(AttributeType t, bool inBS=false);

  int        findmatch(  IdenticalComp &identicalvertices, 
                IdenticalComp &identicalnormals, 
                IdenticalComp &identicaltangents, 
                IdenticalComp &identicalcolors, 
                IdenticalComp *identicaluvs, 
                IdenticalComp *identicalbds, 
                VtxIndices &p);
  MStatus      writeFileHeader(MString &fname, MString &headerName);
  MStatus      writeGatheredTransforms();
  MStatus      parseMayaMesh(MDagPath &dagPath, int level, bool bIntermediate=false);
  MStatus      writeParsedMesh(MString &meshName, int num);
  MStatus      writeFile(MString &fname);
  MStatus      doTriStrips();
  MStatus      makePrimGroups(MDagPath &dagPath, bk3dlib::PMesh bk3dMesh);

  //
  // transformations
  //
  public:
  struct MTransform
  {
	    bool				bUsedForLocator; // true if the transform is just the parent of a locator. The exporter will merge it with the locator...
        bk3d::Transform    transform;  // the transform that we'll store in bk3d
        double                  bindpose[4][4]; // temporary bindpose data
        std::vector<MTransform> children;
        MTransform              *pParentTransform;
		MString DOFLocatorName; // the name of the object used for DOF 
  };
  //int                       m_numTransforms; // total amount of transformations
  //unsigned int              m_numTransformsByteSz; // amount of bytes used for Matrix Storage
  //std::vector<MTransform>   m_transforms;
  //MTransform                *m_pCurTransform; // when traversing
  //std::vector<MString>      m_usedTransforms; // used by the mesh (base tranforms or the Bones)

  //Hugly... change me this...
  std::vector<MString>		m_meshTransformNames; // 
  //std::vector<MObject>      m_meshObjects; // vector of objects to process...
  struct MaterialPair
  {
	  MaterialPair(MObject o_, bk3d::Material *pm_ = NULL) : o(o_), pm(pm_) {}
	  MaterialPair() : pm(NULL) {}
	  MObject o;
	  bk3d::Material *pm;
  };
  //std::map<std::string, MaterialPair>		m_materials; //vector of materials to process

  struct MDOFLocator
  {
	float	DOFAlpha;// "dofa" :  angle limit. Used to dot product...
	bool	SingleDOF;// "sdof": boolean  to tell the DOF is just one axis along Z
	bool	OxAxisLimit;// "oxlim" : if we want a DOF along the Ox axis (Ox is the axis of the bone

	float	OxLimitStart;// "oxstart" : start for the limitation
	float	OxLimitRange;// "oxrange" : range for the limitation
	//float	theColor[3];// "tc" : Color to display the DOF...
	//bool	drawLast;// "dL" : if we want the DOF to be rendered after the 3D objects...
	MString transformName; 
	MString Name;
  };
  std::vector<MDOFLocator> m_DOFLocators;
  ////////////////////////////////////////////////////////
  // Curves
  //
	MStatus         ProcessSortedCurves();
    MStatus         WalkConnections(MObject &object);
	
	MStatus processAnimCurveAsBinary(MObject &object, MObject &owner, MString &connectionName, bool verboseUnits=false);
	enum AnimBaseType			{kAnimBaseUnitless, kAnimBaseTime,
									kAnimBaseLinear, kAnimBaseAngular};

#ifdef DOCURVETEXTEXPORT
	MStatus processAnimCurveAsText(MObject &object,MObject &owner, MString &connectionName, bool verboseUnits=false);
	const char *				tangentTypeAsWord(MFnAnimCurve::TangentType);
	const char *				infinityTypeAsWord(MFnAnimCurve::InfinityType);
	const char *				outputTypeAsWord(MFnAnimCurve::AnimCurveType);
	const char *				boolInputTypeAsWord(bool);
#endif
	void						resetUnits();

	MTime::Unit					timeUnit;
	MAngle::Unit				angularUnit;
	MDistance::Unit				linearUnit;

#ifdef DOCURVETEXTEXPORT
    FILE *fp2;
    FILE *fp3;
	struct cvprops
	{
		cvprops(bool t, int d) : bTimeIn(t), dim(d) {}
		int dim;
		bool bTimeIn;
	};
	map<MString, cvprops, ltstr> m_CurvesProcessed;
	set<MString, ltstr> m_VectorsProcessed;
#endif
    struct CurveAndOwner
    {
        CurveAndOwner() {owner = MObject(); curve = MObject(); }
        CurveAndOwner(MObject c, MObject o, MString _connectionName) { owner = o; curve = c; connectionName= _connectionName; }
        MObject owner;
        MObject curve;
        MString connectionName;
    };
	map<MString, CurveAndOwner , ltstr_inv> m_curvesSorted;
	map<MString, MayaCurveVector2 , ltstr> m_CurveVectors;
	set<MString, ltstr> m_NodesProcessed;
  //
  //
  //
  bk3dlib::PBone extractTransform(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransform, bk3dlib::PBone bk3dTransformParent);
  void extractDOF(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone &bk3dTransform);
  void extractIKEffector(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransformParent);
  void extractIKHandle(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransformParent);

  void writeTransforms_rec(bk3d::Transform *pt, std::vector<MTransform> &cht, bk3d::Transform *&pItTr, int &curTr);
};
// number of components to choose if we need to provide external binary file of vectors
#define COMPONENTSLIMIT 20

extern bool bKeepQuads; // TODO: add in the settings...
extern bool bDoTriStrips;
extern bool bNoShaders;
extern bool bAdjTris;
extern bool bDeltaBS;
extern bool bMultiIndex;
extern bool bTransformed; // true if we want the final mesh after modifier, skinning...
extern bool bAbsPositions; // to write absolute positions. Important for skinning
extern bool bComputeBoundingVolumes; // to ask for bounding box/sphere computation
extern bool bAnimCVsAsText;
extern bool bAnimCVsOnly;
