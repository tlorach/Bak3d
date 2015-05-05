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
#  pragma warning(disable:4786)
#include <algorithm>

#include <maya/MPxCommand.h>

#include "bk3dExport.h"
#include "MayaHelpers.h"
#include "MiscHelpers.h"

MStatus bk3dTranslator::findDOFLocator(const MDagPath &p, MDagPath &PathDOFLocator)
{
	MStatus status;
	MFnDependencyNode depNode(p.node(), &status);
	if(!status)
		return status;
	// Try to find the attribute
	MObject attribute = depNode.attribute("translate", &status);
	if (!status)
		return status;
	// Get the plug and then the sub-plugs that hold the components
	MPlug plug(p.node(), attribute);
	MPlugArray plugArray;
	bool bRes = plug.connectedTo(plugArray, false, true, &status);
	if(!status)
		return status;
	if(plugArray.length() == 0)
		return MStatus(MStatus::kNotFound);
	plug = plugArray[0];
	MObject child = plug.node(&status);
	if(!status)   
		return status;
	if(child.apiType() == MFn::kTransform)
	{
		MDagPath transfPath(p);
		transfPath.pop();
		transfPath.push(child);
		int cc2 = transfPath.childCount();
		for(int j=0; j<cc2; j++)
		{
			MObject child2 = transfPath.child(j, &status);
			if(child2.apiType() == MFn::kPluginLocatorNode)
			{
				PathDOFLocator = transfPath;
				PathDOFLocator.push(child2);
				return MStatus(MStatus::kSuccess);
			}
		}
	}
	// success
	return status;

	return MStatus(MStatus::kNotFound);
}

/*******************************************************/ /**
// DOF is a transform object in bk3dlib
// So we need to transform the parent transformation to a TransformDOF
// Let's pur the condition that this parent must have this DOF as the only child
**/
void bk3dTranslator::extractDOF(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone &bk3dTransform)
{
#if 0
    MStatus status;
    bk3dlib::PTransformDOF bk3dTransformDOF;
    MDagPath dagPathParent(dagPath);
    status = dagPathParent.pop();
    if(!status) // no parent ? Should not happen
        return;
    MFnTransform t(dagPathParent, &status);
    if(!status)
    {
        status.perror("Warning : error parent of the DOF is not a Transformation (failed to get MFnTransform)");
        return;
    }
    int numchildren = dagPathParent.childCount();
    if(numchildren > 1) // We have more than one Node on this parent Let's just create a DOF from scratch
    {
        bk3dTransformDOF = bk3dlib::TransformDOF::Create(t.name().asChar());
        m_bk3dHeader->AttachTransform(bk3dTransformDOF);
    }
    else // The Transform is dedicated to this DOF, so let's merge the Transform and the DOF
    {
        MSTRING mname = dagPathParent.partialPathName();
        LPCSTR name = mname.asChar();
        bk3dlib::PBone pDebug = m_bk3dHeader->GetTransform(name);
        LPCSTR name2 = bk3dTransform->GetName();
        assert(!strcmp(name, name2));
        // the existing transform must be replaced by the DOF
        bk3dlib::PBone pParent = bk3dTransform->GetParent();
        bk3dTransformDOF = bk3dlib::TransformDOF::Create(bk3dTransform->GetName());
        m_bk3dHeader->AttachTransform(bk3dTransformDOF);
        bk3dTransformDOF->CopyFrom(bk3dTransform);
        // Warning : we must now change any reference to bk3dTransform as a DOF, before Destroying it
        // in Maya exporter, the transform of a DOF locator is seen as a regular Transform at first glance.
        int nt = m_bk3dHeader->GetNumTransforms();
        for(int i=0; i<nt; i++)
        {
            bk3dlib::PBone pTr = m_bk3dHeader->GetTransform(i);
            if(pTr->GetDOF() == bk3dTransform)
            {
                pTr->SetDOF(bk3dTransformDOF);
            }
        }

        bk3dTransform->Destroy();
        //bk3dTransformDOF->SetParent(pParent); // Done in the copy of the transform in CreateTransformDOF()
    }
	PF2(("extractDOF\n"));

	float	DOFAlpha;// "dofa" :  angle limit. Used to dot product...
	bool	SingleDOF;// "sdof": boolean  to tell the DOF is just one axis along Z
	bool	OxAxisLimit;// "oxlim" : if we want a DOF along the Ox axis (Ox is the axis of the bone
	float	AxisLimitStart;// "oxstart" : start for the limitation
	float	AxisLimitRange;// "oxrange" : range for the limitation
	status = getAttributeFloat(dagPath.node(), "dofa", DOFAlpha);
	status = getAttribute(dagPath.node(), "sdof", SingleDOF);
	status = getAttribute(dagPath.node(), "oxlim", OxAxisLimit);
	status = getAttributeFloat(dagPath.node(), "oxstart", AxisLimitStart);
	status = getAttributeFloat(dagPath.node(), "oxrange", AxisLimitRange);
    assert(!"TODO: changed the DOF params. Need to update the Locator accordingly");
#pragma message(__FILE__"(120):TODO... changed the DOF params. Need to update the Locator accordingly")
    bk3dTransformDOF->SetDOFValues(bk3dlib::DOF_CONE, &DOFAlpha, &AxisLimitStart, &AxisLimitRange);
#endif
}
/*******************************************************/ /**
// Maya Transform must be exported with all the components :
// we never know where animation curves will connect
**/
bk3dlib::PBone bk3dTranslator::extractTransform(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dBone, bk3dlib::PBone bk3dTransformParent)
{
	PF2(("extractTransform\n"));
    double v[4];
    MStatus status;
    //MMatrix mat, matInv;
    //MTransform   curtransform;
    MFnTransform t(dagPath, &status);
    if(!status)
    {
        status.perror("Warning : error in turning the node into a transform");
        return NULL;
    }
    bk3dlib::PBone bk3dExistingTransform = m_bk3dHeader->GetTransform(t.name().asChar());
    if(bk3dExistingTransform)
    {
        status.perror("Warning : transform already there");
        return bk3dExistingTransform;
    }
    DPF2((">>> Exporting Transform %s\n", t.name().asChar()));
    if(bk3dBone == NULL)
    {
        bk3dBone = bk3dlib::Transform::Create(t.name().asChar());
        m_bk3dHeader->AttachTransform(bk3dBone);
    }
    bk3dBone->SetParent(bk3dTransformParent);
    bk3dlib::PTransform bk3dTransform = bk3dBone->AsTransf();
	//
    // resolved final matrix.
    // Note that this will certainly be recomputed in the renderer
    // from separate attributes of the transformation
    //
    //mat = t.transformationMatrix();
    //mat.get(curtransform.transform.matrix4x4); // temporary store bindpose for later use here (we keep double precision here)
    //if(m_pCurTransform) // matrix mult absolute matrix
    //{
    //    MMatrix matParent(m_pCurTransform->transform.matrix4x4); // use double like for bindposes ?
    //    MMatrix matAbs = mat * matParent;
    //    matAbs.get(curtransform.transform.abs_matrix4x4);
    //} else { 
    //    mat.get(curtransform.transform.abs_matrix4x4); // temporary store bindpose for later use here (we keep double precision here)
    //}
    //curtransform.transform.bDirty = false;
    //curtransform.transform.validComps = 0xFFFF;
    //
    // Translation
    //
MMatrix mmm = t.transformationMatrix();
double m[4][4];
mmm.get(m);
    MVector tr;
    tr = t.translation(MSpace::kTransform);
    bk3dTransform->SetPos((float)tr.x, (float)tr.y, (float)tr.z);
    //curtransform.transform.pos[0] = (float)tr.x;
    //curtransform.transform.pos[1] = (float)tr.y;
    //curtransform.transform.pos[2] = (float)tr.z;
    //
    // Scale
    //
    t.getScale(v);
    bk3dTransform->SetScale((float)v[0], (float)v[1], (float)v[2]);
    //curtransform.transform.scale[0] = (float)v[0];
    //curtransform.transform.scale[1] = (float)v[1];
    //curtransform.transform.scale[2] = (float)v[2];
    //
    // Rotation Order
    //
    MTransformationMatrix::RotationOrder ro;
    t.getRotation(v, ro);
    switch(ro)
    {
    case MTransformationMatrix::kXYZ:
        bk3dTransform->SetRotationOrder('x', 'y', 'z');
        break;
    case MTransformationMatrix::kYZX:
        bk3dTransform->SetRotationOrder('y', 'z', 'x');
        break;
    case MTransformationMatrix::kZXY:
        bk3dTransform->SetRotationOrder('z', 'x', 'y');
        break;
    case MTransformationMatrix::kXZY:
        bk3dTransform->SetRotationOrder('x', 'z', 'y');
        break;
    case MTransformationMatrix::kYXZ:
        bk3dTransform->SetRotationOrder('y', 'x', 'z');
        break;
    case MTransformationMatrix::kZYX:
        bk3dTransform->SetRotationOrder('z', 'y', 'x');
        break;
    }
    bk3dTransform->SetRotation((float)(180.0f*v[0]/M_PI), (float)(180.0f*v[1]/M_PI), (float)(180.0f*v[2]/M_PI) );
    //curtransform.transform.rotation[0] = (float)(180.0f*v[0]/M_PI);
    //curtransform.transform.rotation[1] = (float)(180.0f*v[1]/M_PI);
    //curtransform.transform.rotation[2] = (float)(180.0f*v[2]/M_PI);
    //
    // pivot data
    //
    MPoint pt = t.scalePivot(MSpace::kObject, &status);
    bk3dTransform->SetScalePivot((float)pt.x, (float)pt.y, (float)pt.z);
    //curtransform.transform.scalePivot[0] = (float)pt.x;
    //curtransform.transform.scalePivot[1] = (float)pt.y;
    //curtransform.transform.scalePivot[2] = (float)pt.z;

    MVector vec = t.scalePivotTranslation(MSpace::kObject, &status);
    bk3dTransform->SetScalePivotTranslate((float)vec.x, (float)vec.y, (float)vec.z);
    //curtransform.transform.scalePivotTranslate[0] = (float)vec.x;
    //curtransform.transform.scalePivotTranslate[1] = (float)vec.y;
    //curtransform.transform.scalePivotTranslate[2] = (float)vec.z;

    pt = t.rotatePivot(MSpace::kObject, &status);
    bk3dTransform->SetRotationPivot((float)pt.x, (float)pt.y, (float)pt.z);
    //curtransform.transform.rotationPivot[0] = (float)pt.x;
    //curtransform.transform.rotationPivot[1] = (float)pt.y;
    //curtransform.transform.rotationPivot[2] = (float)pt.z;

    vec = t.rotatePivotTranslation(MSpace::kObject, &status);
    bk3dTransform->SetRotationPivotTranslate((float)vec.x, (float)vec.y, (float)vec.z);
    //curtransform.transform.rotationPivotTranslate[0] = (float)vec.x;
    //curtransform.transform.rotationPivotTranslate[1] = (float)vec.y;
    //curtransform.transform.rotationPivotTranslate[2] = (float)vec.z;

    MQuaternion quat = t.rotateOrientation(MSpace::kObject, &status);
    bk3dTransform->SetRotationOrientation((float)quat.x, (float)quat.y, (float)quat.z, (float)quat.w);
    //curtransform.transform.rotationOrientation[0] = (float)quat.x;
    //curtransform.transform.rotationOrientation[1] = (float)quat.y;
    //curtransform.transform.rotationOrientation[2] = (float)quat.z;
    //curtransform.transform.rotationOrientation[3] = (float)quat.w;

    //
    // basic data init
    //
    //strcpy_s(curtransform.transform.name, 32, t.name().asChar());
    //curtransform.transform.pChildren = NULL;
    //curtransform.transform.pParent = NULL;
    //
    // if the object is also a iKJoint
    //
    MFnIkJoint joint(dagPath, &status);
    if(status)
    {
        MQuaternion q;
        joint.getOrientation(q);
        bk3dTransform->SetJointOrientation((float)q.x, (float)q.y, (float)q.z, (float)q.w);
        //curtransform.transform.jointOrientation[0] = (float)q.x;
        //curtransform.transform.jointOrientation[1] = (float)q.y;
        //curtransform.transform.jointOrientation[2] = (float)q.z;
        //curtransform.transform.jointOrientation[3] = (float)q.w;
        //curtransform.transform.nodeType = NODE_JOINT;
        // Bone needs a tail to be nicely represented later
        // let's take the mean of all the children
        int numChildren = dagPath.childCount();
        float pSum[3] = {0.0f, 0.0f, 0.0f};
        if(numChildren > 0)
        {
            double p[3];
            for(int i=0; i< numChildren; i++)
            {
                MObject childobj;
                childobj = dagPath.child( i, &status);
                MDagPath dagPathChild(dagPath);
                dagPathChild.push(childobj);
                MFnIkJoint jointChild(dagPathChild, &status);
                MFnTransform tc(dagPathChild, &status);
                tr = tc.translation(MSpace::kTransform);
                MVector v = jointChild.getTranslation(MSpace::kPreTransform);
                v.get(p);
                tr.get(p);
                pSum[0] += (float)p[0];
                pSum[1] += (float)p[1];
                pSum[2] += (float)p[2];
            }
            pSum[0] /= (float)numChildren;
            pSum[1] /= (float)numChildren;
            pSum[2] /= (float)numChildren;
        } else {
            pSum[0] = 0.0f;
            pSum[1] = 0.0f;
            pSum[2] = 0.1f;
        }
        bk3dTransform->SetTailPos(pSum[0], pSum[1], pSum[2]);
    }

    //else
    //{
    //    curtransform.transform.nodeType = NODE_TRANSFORM;
    //    memset(curtransform.transform.jointOrientation, 0, sizeof(float)*4);
    //}
    //
    // get the bindpose. For now we just take the current transformation...
    // So we assume we do the export with all the matrices set back in bindpose
    //
    // NOTE: the matrix library is supposed to compute the bindpose matrix
    //MTransformationMatrix restPos;
    //restPos = t.transformation(); // we may want to get the bind pose somewhere...TODO later
    //matInv = restPos.asMatrixInverse();
    //if(m_pCurTransform) // matrix mult for correct bindpose
    //{
    //    MMatrix matParentInv(m_pCurTransform->bindpose);
    //    MMatrix matBP = matParentInv * matInv;
    //    matBP.get(curtransform.bindpose); // temporary store bindpose for later use here (we keep double precision here)
    //    matBP.get(curtransform.transform.bindpose_matrix4x4); // store the one for export
    //} else { 
    //    matInv.get(curtransform.bindpose); // temporary store bindpose for later use here (we keep double precision here)
    //    matInv.get(curtransform.transform.bindpose_matrix4x4); // store the one for export
    //}
	//
	// store the references to curves for later encoding
	//
//TODO
    //
    // temporary linkage for export work
    // m_numTransformsByteSz is here to give the total size of the transformation pool
    // since the size vary depending on structure sizes (the # of children)
    //
    //if(m_pCurTransform)
    //{
    //    curtransform.pParentTransform = m_pCurTransform;
    //    m_pCurTransform->children.push_back(curtransform);
    //    if(m_pCurTransform->children.size() > 1) // 1 because we already have children[1]
    //        m_numTransformsByteSz += sizeof(bk3d::Transform*); // add the ptr size
    //} else {
    //    m_transforms.push_back(curtransform);
    //    curtransform.pParentTransform = NULL;
    //}
    //m_numTransformsByteSz += sizeof(bk3d::Transform); // add the size of the structure
    //m_numTransforms++;

    //
    // Find the possible DOF related to this transform
    // If so, let's partially do the recursive walk through the DAG :
    // we want to create the part that is referenced here as a DOF element (i.e. Transform + DOF locator)
    //
    MObject o = getConnectedObject(dagPath.node(), "translate", false, true, &status);
    if(!o.isNull())
    {
        MFnTransform tdof(o, &status);
        if(!status)
        {
            status.perror("Warning : error in getting DOF transform via 'translate' attribute!");
            return bk3dTransform;
        }
        MDagPath dpDOF(dagPath);
        // get the parent and append the object we found
        dpDOF.pop();
        status = dpDOF.push(o);
        if(!status)
        {
            status.perror("Warning : couldn't append the DOF transform to the path. Are you sure this DOF is correctly located ?");
            return bk3dTransform;
        }
        LPCSTR DOFName = dpDOF.partialPathName().asChar();
        {
            // TODO: the DOF management has changed in bk3d. Need to adjust
            // find the parent of the DOF
            // then create the DOF attached to this transform
            //bk3dTransform = m_bk3dHeader->GetTransform(dagPath.partialPathName().asChar());
            bk3dlib::PTransformDOF pbk3dTrForDOF = bk3dTransform->CreateDOF();
		    //
		    // Export related curves... if exist
		    //
    #if 1
		    if(bExportAnimCVs)
		    {
			    exportCurve(o, level-1);
		    }
    #endif
            bk3dlib::PBone pbk3dTr = extractTransform(dpDOF, level-1, bIntermediate, NULL, bk3dTransform->GetParent());
            //----> Process the children : we expect the DOF locator !
            int numchildrenDOF = dpDOF.childCount();
            for(int i=0; i< numchildrenDOF; i++)
            {
                MObject childobj;
                childobj = dpDOF.child( i, &status);
                MDagPath dagPathChildDOF(dpDOF);
                dagPathChildDOF.push(childobj);
                status = readDagPath(dagPathChildDOF, level/*-1*/, bIntermediate, pbk3dTr);
            }
            // TODO: keep working on this part
        }
    }

    //
    // Update the connections of the curves. m_curvesSorted has them with owner info and component to which they are connected
    //
    int N = m_bk3dHeader->GetNumCurves();
    for(int n=0; n<N; n++)
    {
        bk3dlib::CurveVec *pCVV = m_bk3dHeader->GetCurveVec(n);
        LPCSTR name = pCVV->GetName();
        MSTRING mname(name);
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
                if(sarray[1] == MString("scale"))
                {
                    pCVV->Connect(bk3dTransform, bk3dlib::TRANSF_SCALE);
                }
                else if(sarray[1] == MString("rotation"))
                {
                    pCVV->Connect(bk3dTransform, bk3dlib::TRANSF_EULERROT);
                }
                else if(sarray[1] == MString("translate"))
                {
                    pCVV->Connect(bk3dTransform, bk3dlib::TRANSF_POS);
                }
                else
                {
                    EPF(("Cannot connect component : %s\n", sarray[1].asChar()));
                }
            }
        }
    }
    //Connect(Transform *pTransf, TransfComponent comp) = 0;
    return bk3dTransform;
}
/*******************************************************/ /**
    Gather the table of points (XYZ) for Vertices and blendshapes
    vertexArrayRef is the reference array
 **/
MStatus         bk3dTranslator::extractPointTable(bk3dlib::PBuffer bk3dBuffer, MPointArray &vertexArray, const char *name, int bsnum, MPointArray *vertexArrayRef)
{
  PF2(("extractPointTable\n"));
  MStatus status;
  unsigned int i;
  static char tmpname[64];
  if(bsnum >= 0)
    sprintf_s(tmpname, 64, "%s%d",name, bsnum);
  else
    sprintf_s(tmpname, 64, "%s", name);
  bk3dBuffer->SetName(tmpname);
  //m_outattr.push_back(VertexAttribute(tmpname, 3));
  //m_inattr.push_back(VertexAttribute(tmpname, 3));
  DPF2(("Vertex array %s size = %d\n", tmpname, vertexArray.length()));
  bk3dBuffer->SetData((float*)NULL,0,vertexArray.length()*3);
  for(i=0; i<vertexArray.length(); i++)
  {
    if(bDeltaBS && vertexArrayRef)
    {
        float p[3];
        p[0] = (float)(vertexArray[i][0] - (*vertexArrayRef)[i][0]);
        p[1] = (float)(vertexArray[i][1] - (*vertexArrayRef)[i][1]);;
        p[2] = (float)(vertexArray[i][2] - (*vertexArrayRef)[i][2]);;
        bk3dBuffer->SetData(p, i*3, 3);
    } else {
        MPoint pt = vertexArray[i];
        // if we ask for absolute positions (for skinned meshes)
        if(bAbsPositions && bTransformed)
        {
// TODO: take back the transforms if ever not in selection so we can compute m_pCurTransform")

// WARNING: mesh not having any parent tranform (group)
// if this right then data will be fine. If not
// the mesh could be inconsistent for skinning\n");*/

// TODO: get the related transform and apply it
            //if(m_pCurTransform) { // NOW this is wrong. Need to change this...
            //    MMatrix absmat(m_pCurTransform->transform.matrix4x4);
            //    pt *= absmat;
            //}
        }
        float p[3];
        p[0] = (float)pt.x;
        p[1] = (float)pt.y;
        p[2] = (float)pt.z;
        bk3dBuffer->SetData(p, i*3, 3);
    }
  }
  return status;
}
/*******************************************************/ /**
Hacky : we fake the creation of the table.
In fact, we don't need any creation because
it is rather easy to get vertex IDs... no need to store
														  **/
MStatus         bk3dTranslator::extractVertexIDs(bk3dlib::PBuffer bk3dBuffer, const char *name, int bsnum)
{
	PF2(("Extract VertexID\n"));
assert(!"TODO !");
    MStatus status;
  static char tmpname[64];
  if(bsnum >= 0)
    sprintf_s(tmpname, 64, "%s%d",name, bsnum);
  else
    sprintf_s(tmpname, 64, "%s", name);
    //m_inattr_vertexID = m_inattr.size();
    //m_inattr.push_back(VertexAttribute(tmpname,1));
    //m_outattr.push_back(VertexAttribute(tmpname,1));
    return status;
}
/*******************************************************/ /**
    Gather the table of points (XYZ)
 **/
/*MStatus         bk3dTranslator::extractPointTable(bk3dlib::PBuffer bk3dBuffer, MPointArray &vertexArray)
{
    MStatus status;
    unsigned int i;
    m_inattr_vtx =  m_inattr.size();
    m_outattr.push_back(VertexAttribute(MESH_POSITION,3));
    m_inattr.push_back(VertexAttribute(MESH_POSITION,3));
    DPF2(("-- Vertex array size = %d\n", vertexArray.length()));
    for(i=0; i<vertexArray.length(); i++)
    {
        m_inattr.back().m_floatVector.push_back((float)vertexArray[i][0]);
        m_inattr.back().m_floatVector.push_back((float)vertexArray[i][1]);
        m_inattr.back().m_floatVector.push_back((float)vertexArray[i][2]);
    }
    return status;
}*/
/*******************************************************/ /**
    Gather the table of colors (RGBA)

  \remark there is an issue about colors: unable to see if some are shared between polys
            thus the optimization is disturbed. See bk3dTranslator::findmatch
 **/
MStatus         bk3dTranslator::extractColorTable(bk3dlib::PBuffer bk3dBuffer, MColorArray &colorArray)
{
	PF2(("extractColorTable\n"));
    MStatus status;
    if(colorArray.length() == 0)
    {
        EPF(("WARNING: no color table \n"));
        return status;
    }
    DPF2(("-- color array size = %d\n", colorArray.length()));
    bk3dBuffer->SetData((float*)NULL,0,colorArray.length()*4);
    for(unsigned int i=0; i<colorArray.length(); i++)
    {
      if(colorArray[i][0] < 0)
      {
          static float c[] = {0.0f,0.0f,0.0f,0.0f};
        bk3dBuffer->SetData(c, i*4, 4);
      } else {
        DPF2(("%d color %f %f\n", i, (float)colorArray[i][0], (float)colorArray[i][1]));
        float c[4] = {
            (float)colorArray[i][0],
            (float)colorArray[i][1],
            (float)colorArray[i][2],
            (float)colorArray[i][3]
        };
        bk3dBuffer->SetData(c, i*4, 4);
      }
    }
    return status;
}
/*******************************************************/ /**

														  **/
MStatus         bk3dTranslator::extractSimpleTriangleSetTable(bk3dlib::PBuffer bk3dBuffer, int np)
{
assert(!"TODO");
	PF2(("extractSimpleTriangleSetTable\n"));
    MStatus status;
    //m_trianglesets.push_back(std::vector<unsigned int>());
    //std::vector<unsigned int> &attr = m_trianglesets.back();
    //DPF2(("-- simple triangleset size = %d\n", np));
    //for(int i=0; i<np; i++)
    //{
    //    attr.push_back(i);
    //}
    return status;
}
/*******************************************************/ /**
														  **/
MStatus         bk3dTranslator::extractTriangleSetTable(bk3dlib::PBuffer bk3dBuffer, MFnSet &set)//MFnSingleIndexedComponent &sngcomp)
{
	PF2(("extractTriangleSetTable\n"));
    MStatus status;
assert(!"TODO");
//    MSelectionList  slist;
//    status = set.getMembers(  slist, true );
//    int n = slist.length();
//    MItSelectionList iter( slist );
//    for ( ; !iter.isDone(); iter.next() )
//    {     
//        MObject curobj;
//        if(!(status = iter.getDependNode(curobj)))
//        {
//            status.perror("Unable to get DependNode in selection list");
//            continue;
//        }
//                const char * nodename = curobj.apiTypeStr();
//        const char * apitype = curobj.apiTypeStr();
//        MFnDependencyNode depNode(curobj, &status);
//        const char * name = depNode.name().asChar();
//    }
///*    m_trianglesets.push_back(std::vector<unsigned int>());
//    std::vector<unsigned int> &attr = m_trianglesets.back();
//    DPF2(("-- triangleset size = %d\n", sngcomp.elementCount()));
//    for(int i=0; i<sngcomp.elementCount(); i++)
//    {
//        attr.push_back(sngcomp.element(i));
//    }*/
    return status;
}
/*******************************************************/ /**
    Write the table of colors (RGBA) in a separate binary file
 **/
MStatus         bk3dTranslator::extractUVTable(bk3dlib::PBuffer bk3dBuffer, int idx, MFloatArray &uArray, MFloatArray &vArray)
{
	PF2(("extractUVTable\n"));
    MStatus status;
    if(uArray.length() == 0)
    {
        EPF(("WARNING extractUVTable() : not set %d not a UV\n", idx));
        return status;
    }
    //char tmp[10];
    //sprintf_s(tmp, 10, "texcoord%d", idx);
    //m_inattr_uv[idx] = m_inattr.size();
    //m_outattr.push_back(VertexAttribute(tmp,2));
    //m_inattr.push_back(VertexAttribute(tmp,2));
    DPF2(("  uv set %d, size = %d\n", idx, uArray.length()));
    bk3dBuffer->SetData((float*)NULL, 0, uArray.length()*2);
    unsigned int l = uArray.length();
    for(unsigned int i=0; i<l; i++)
    {
        //m_inattr.back().m_floatVector.push_back((float)uArray[i]);
        //m_inattr.back().m_floatVector.push_back((float)vArray[i]);
        float uv[2] = {
            (float)uArray[i],
            (float)vArray[i]
        };
        bk3dBuffer->SetData(uv, i*2, 2);
    }
    return status;
}
/*******************************************************/ /**
    Write the table of normals
 **/
MStatus         bk3dTranslator::extractNormalsTable(bk3dlib::PBuffer bk3dBuffer, MFloatVectorArray &normals, const char *name, int bsnum, MFloatVectorArray *normalsRef)
{
	PF2(("extractNormalsTable\n"));
    MStatus status;
    unsigned int i,l;
    l = normals.length();
    if(l == 0)
    {
        EPF(("WARNING: not color table \n"));
        return status;
    }
    static char tmpname[64];
    if(bsnum >= 0)
      sprintf_s(tmpname, 64, "%s_BS%d",name, bsnum);
    else
      sprintf_s(tmpname, 64, "%s", name);
    bk3dBuffer->SetName(tmpname);
    //m_outattr.push_back(VertexAttribute(tmpname,3));
    //m_inattr.push_back(VertexAttribute(tmpname,3));
    DPF2(("-- normal array size = %d\n", normals.length()));

// TODO: transform Normal if (bAbsPositions && bTransformed) !!!

    bk3dBuffer->SetData((float*)NULL, 0, l*3);
    for(i=0; i<l; i++)
    {
        float n[3] = {
            (float)normals[i][0],
            (float)normals[i][1],
            (float)normals[i][2]
        };
        if(bDeltaBS && normalsRef)
        {
          n[0] -= (float)(*normalsRef)[i][0];
          n[1] -= (float)(*normalsRef)[i][1];
          n[2] -= (float)(*normalsRef)[i][2];
        }
        bk3dBuffer->SetData(n, i*3, 3);
    }
    return status;
}
/*******************************************************/ /**
    Write the table of Per-Triangle-per-Vertex normals
 **/
MStatus         bk3dTranslator::extractVertexNormalsTable(bk3dlib::PBuffer bk3dBuffer, MFnMesh &fnMesh, const char *name, int bsnum, MObject meshobjRef)
{
	PF2(("extractVertexNormalsTable\n"));
    MStatus status, statusRef;
    MFnMesh fnMeshRef(meshobjRef, &statusRef);
    int n = fnMesh.numNormals();
    if(n == 0)
    {
        EPF(("WARNING: not Per Vertex Normals\n"));
        return status;
    }
    static char tmpname[64];
    if(bsnum >= 0)
      sprintf_s(tmpname, 64, "%s%d",name, bsnum);
    else
      sprintf_s(tmpname, 64, "%s", name);
    //m_outattr.push_back(VertexAttribute(tmpname,3));
    //m_inattr.push_back(VertexAttribute(tmpname,3));
    bk3dBuffer->SetName(tmpname);
    DPF2(("-- vertex normal array size = %d\n", n));
    MFloatVectorArray normalArray;
    MFloatVectorArray normalArrayRef;
    status = fnMeshRef.getNormals(normalArrayRef, MSpace::kObject);
    status = fnMesh.getNormals(normalArray, MSpace::kObject);
    /*assert(n == normalArray.length())*/n = normalArray.length(); // must be the same as previous n
    bk3dBuffer->SetData((float*)NULL, 0, n*3);
    for(int i=0; i<n; i++)
    {
      MVector normal;
      MVector normal0;
      normal0 = MVector(0,0,0);
      if(bDeltaBS && statusRef)
      {
        normal0 = normalArrayRef[i];
      }
      normal = normalArray[i];
      float n[3] = {
          (float)(normal[0]-normal0[0]),
          (float)(normal[1]-normal0[1]),
          (float)(normal[2]-normal0[2])
      };
      bk3dBuffer->SetData(n, i*3, 3);
    }
    return status;
}
/*******************************************************/ /**
    Write the table of tangents
    Table is made of tangents for the FACE-VERTICES
 **/
MStatus         bk3dTranslator::extractTanBinormTables(bk3dlib::PBuffer bk3dBuffer, MFloatVectorArray &table, const char *name, int bsnum, MFloatVectorArray *tableRef)
{
	PF2(("extractTanBinormTables\n"));
    MStatus status;
    unsigned int i,l,l2;
    l = table.length();
    if(l == 0)
    {
        EPF(("WARNING: not tangent/binorm table \n"));
        return status;
    }
// NOTE : this to Prevent the bug of Tanbinorm table size
MFloatVectorArray *ptable = &table;
if(tableRef)
{
    l2 = (*tableRef).length();
    if(l2 > l)
    {
      DPF(("BUG WARNING : looks like the table %s is too small : %d < %d. Skipping these data...", name, l, l2));
      l = l2;
      ptable = tableRef;
    }
}
    static char tmpname[64];
    if(bsnum >= 0)
      sprintf_s(tmpname, 64, "%s%d",name, bsnum);
    else
      sprintf_s(tmpname, 64, "%s", name);
    //m_outattr.push_back(VertexAttribute(tmpname,3));
    //m_inattr.push_back(VertexAttribute(tmpname,3));
    bk3dBuffer->SetName(tmpname);
    DPF2(("-- tangent/binorm array size = %d\n", ptable->length()));

// TODO: transform Tan/Binorm if (bAbsPositions && bTransformed) !!!

    bk3dBuffer->SetData((float*)NULL,0,l*3);
    for(i=0; i<l; i++)
    {
      double x0=0,y0=0,z0=0;
      if(bDeltaBS && tableRef)
      {
        x0 = (*tableRef)[i][0];
        y0 = (*tableRef)[i][1];
        z0 = (*tableRef)[i][2];
      }
      //DPF2(("tangent (%f, %f, %f)\n", (float)table[i][0],(float)table[i][1],(float)table[i][2]));
      float tb[3] = {
          (float)((*ptable)[i][0]-x0),
          (float)((*ptable)[i][1]-y0),
          (float)((*ptable)[i][2]-z0)
      };
      bk3dBuffer->SetData(tb, i, 3);
    }
    return status;
}

/*******************************************************/ /**
    Write the Blind Data
 **/
MStatus         bk3dTranslator::gatherBlindData(MFnMesh &fnMesh, unsigned int size)
{
	PF2(("gatherBlindData\n"));
  MStatus status;
assert(!"TODO");
//  char tmp[100];
//  MIntArray templateIndices;
//
//  status = fnMesh.getBlindDataTypes(MFn::kMeshVertComponent, templateIndices);
//
//  uint blindDataTemplateCount = templateIndices.length();
//  for (uint i = 0; i < blindDataTemplateCount; ++i)
//  { 
//    if(i == 4)
//    {
//      EPF(("WARNING: not enough place for all the Blind Data\n"));
//      break;
//    }
//    uint templateIndex = templateIndices[i];
//    MStringArray longNames, shortNames, typeNames;
//    fnMesh.getBlindDataAttrNames(templateIndex, longNames, shortNames, typeNames);
//    uint valueCount = typeNames.length();
//    if(valueCount >= 4)
//    {
//      EPF(("WARNING: too many components in the Blind template %d\n", templateIndex));
//      valueCount = 4;
//    }
//    // Keep in mind the name of the blind data and its format
////    formatDX(valueCount, m_blindDataInfos[i].formatDXGI, m_blindDataInfos[i].formatDX9);
//    strcpy_s(m_blindDataInfos[i].name, 64, shortNames[0].asChar());
//    for (uint j = 1; j < valueCount; ++j)
//    {
//      strcat_s(m_blindDataInfos[i].name, 64, "_");
//      strcat_s(m_blindDataInfos[i].name, 64, shortNames[j].asChar());
//    }
//
//    sprintf_s(tmp, 100, "blind%d", templateIndices[i]);
//    m_inattr_bd[i] = m_inattr.size();
//    m_outattr.push_back(VertexAttribute(tmp,4));
//    m_inattr.push_back(VertexAttribute(tmp,4));
//    DPF2(("Blind Data %d size = %d\n", i, m_inattr_bd[i]));
//    // fill with default values
//    // TODO : do this faster, please :$
//    for(unsigned int j=0; j<size; j++)
//    {
//        m_inattr.back().m_floatVector.push_back(0.0f);
//        m_inattr.back().m_floatVector.push_back(0.0f);
//        m_inattr.back().m_floatVector.push_back(0.0f);
//        m_inattr.back().m_floatVector.push_back(0.0f);
//    }
//    // Now fill the array with data depending on idx
//    for (uint j = 0; j < valueCount; ++j)
//    { 
//      MString typeName = typeNames[j];
//      DPF2(("Blind Data typeName = %s\n", longNames[j].asChar()));
//      if (typeName == "double")
//      {
//        MIntArray indices;
//        MDoubleArray indexedData;
//        fnMesh.getDoubleBlindData(MFn::kMeshVertComponent,
//           templateIndex, longNames[j], indices, indexedData);
//
//        for(uint kk=0; kk< indices.length(); kk++)
//        {
//          double d = indexedData[kk];
//          assert(indices[kk]*4 + j < size*4);
//          DPF2(("idx=%d val = %f\n", indices[kk], (float)d));
//          m_inattr.back().m_floatVector[indices[kk]*4 + j] = (float)d;
//        }
//      }
//      else // TODO
//      {
//        EPF(("WARNING: type %s not [yet] supported\n", typeName));
//      }
//    }
//  }
  return status;
}

/**
    Write the table of TriIndices. Each component has a specific indice, here.
    return the # of indices per polygon.
    If there was a mix, then all will be turned into the least common case (i.e. 3 indices)
 **/
MStatus bk3dTranslator::extractTriIndicesTable(bk3dlib::PMesh bk3dMesh, MObject meshobj, unsigned int np, int nuvsets, MStringArray &setNames, MIntArray *uvCounts)
{
	PF2(("extractTriIndicesTable\n"));
    MStatus status;
    int i,j;
    int s;

    MFnMesh fnMesh(meshobj, &status);
    IFFAILUREMSG("not a Mesh !!");
    MItMeshPolygon iPoly(meshobj, &status);
    IFFAILUREMSG("Could not get MItMeshPolygon from mesh !!");

    //m_outattr.push_back(VertexAttribute("indices"));

    DPF2(("-- triangle array size = %d\n", np));
    s = 0;
    unsigned int tanidx = 0; // tangent components are stored in the same order than Polygons.
    // TODO: make shaderindices an Index table
    // so that it could become an attribute to point to a shaer ID...
    // TODO: create here the separate primGroups depending on shader indices
    MIntArray shadersindices(m_shadersindices);

    MItMeshPolygon iPoly2(meshobj, &status);
    // No need anymore...
    //int tri_numvtx = -1;
    //for(i=0; i<(int)np; i++, iPoly2.next())
    //{
    //    int n = iPoly2.polygonVertexCount( &status );
    //    if(tri_numvtx < 0)
    //        tri_numvtx = n;
    //    else if(tri_numvtx != n) 
    //    {
    //        EPF(("WARNING WriteTriIndicesTable() : Polygons table have <> # of vertices (Quad mixed with tris). Turning into triangles...\n"));
    //        EPF(("WARNING Forcing bKeepQuads to false\n"));
    //        bKeepQuads = false;
    //        tri_numvtx = 3;
    //        break;
    //    }
    //}
    //
    // Allocate separate Idx buffers
    //
    if(findAttrInfo(Position)) {
        bk3dBufferIdxPosition = bk3dlib::Buffer::CreateIdxBuffer("IndexPosition", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxPosition);
    }
    if(findAttrInfo(Color)) {
        bk3dBufferIdxColor = bk3dlib::Buffer::CreateIdxBuffer("IndexColor", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxColor);
    }
    if(findAttrInfo(FaceNormal)) {
        bk3dBufferIdxNormal = bk3dlib::Buffer::CreateIdxBuffer("IndexFaceNormal", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxNormal);
    }
    if(findAttrInfo(Tangent)) {
        bk3dBufferIdxTangent = bk3dlib::Buffer::CreateIdxBuffer("IndexTangent", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxTangent);
    }
    if(findAttrInfo(Binormal)) {
        bk3dBufferIdxBinormal = bk3dlib::Buffer::CreateIdxBuffer("IndexBinormal", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxBinormal);
    }
    if(findAttrInfo(Normal)) {
        bk3dBufferIdxVertexNormal = bk3dlib::Buffer::CreateIdxBuffer("IndexNormal", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxVertexNormal);
    }
    if(findAttrInfo(TexCoord0)) {
        bk3dBufferIdxTexCoord[0] = bk3dlib::Buffer::CreateIdxBuffer("IndexTexCoord0", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxTexCoord[0]);
    }
    if(findAttrInfo(TexCoord1)) {
        bk3dBufferIdxTexCoord[1] = bk3dlib::Buffer::CreateIdxBuffer("IndexTexCoord1", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxTexCoord[1]);
    }
    if(findAttrInfo(TexCoord2)) {
        bk3dBufferIdxTexCoord[2] = bk3dlib::Buffer::CreateIdxBuffer("IndexTexCoord2", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxTexCoord[2]);
    }
    if(findAttrInfo(TexCoord3)) {
        bk3dBufferIdxTexCoord[3] = bk3dlib::Buffer::CreateIdxBuffer("IndexTexCoord3", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxTexCoord[3]);
    }
    if(findAttrInfo(Blind0)) {
        bk3dBufferIdxBlind[0] = bk3dlib::Buffer::CreateIdxBuffer("IndexBlind0", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxBlind[0]);
    }
    if(findAttrInfo(Blind1)) {
        bk3dBufferIdxBlind[1] = bk3dlib::Buffer::CreateIdxBuffer("IndexBlind1", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxBlind[1]);
    }
    if(findAttrInfo(Blind2)) {
        bk3dBufferIdxBlind[2] = bk3dlib::Buffer::CreateIdxBuffer("IndexBlind", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxBlind[2]);
    }
    if(findAttrInfo(Blind3)) {
        bk3dBufferIdxBlind[3] = bk3dlib::Buffer::CreateIdxBuffer("IndexBlind3", bk3dlib::UINT32, 1);
        bk3dMesh->AttachIndexBuffer(bk3dBufferIdxBlind[3]);
    }
    //
    // Loop in polygons
    //
    for(i=0; i<(int)np; i++, iPoly.next(), s++)
    {
        //MIntArray vertexList;
        MVector pnormal;
        int numvtx = iPoly.polygonVertexCount( &status );// fnMesh.polygonVertexCount( i, &status );
        int vtxCnt = numvtx; // needed for a fucking issue down here
        MIntArray polyvertices;
        iPoly.getVertices(polyvertices);
    // We don't want this here anymore
    //    if((tri_numvtx2 > 4))
    //        EPF(("Warning Polygon %d is > 4 : %d\n",i, tri_numvtx));
    //    if(tri_numvtx2 < 3)
    //    {
    //        EPF(("ERROR : Polygon %d is not a triangle (=%d)\n",i, tri_numvtx));
    //        // TODO ? Lines ?
    //        // TO stop and retrun err...
    //    }
    //    if(((tri_numvtx == 3) && (tri_numvtx2 == 4)))
    //    { 
    //        if(i==0) 
				//EPF(("WARNING WriteTriIndicesTable() : Polygons are Quads. Turning them into 2 triangles...\n");) 
    //        tri_numvtx2 = 3;
    //    }
        int numTriangles;
        int loopForTriangles;
        status = iPoly.numTriangles(numTriangles);
        // Hack to keep the quads if needed :
        loopForTriangles = numTriangles;
        numvtx = 3;
        if((numTriangles == 2) && bKeepQuads)
        {
            loopForTriangles = 1;
            numvtx = 4;
        }
        for(int t=0; t<loopForTriangles; t++)
        {
            MPointArray points;
            MIntArray vertexList;
            if((numTriangles == 2)&&(loopForTriangles==1)) // this case is for Quads
            {
                // just get the inices of iPoly...
                for(int l=0; l<numvtx; l++)
                {
                    int idx = iPoly.vertexIndex(l);
                    vertexList.append(idx);
                }
            }
            else
                status = iPoly.getTriangle( t, points, vertexList, MSpace::kObject);
            for(j=0; j<(int)numvtx; j++)
            {
              // no more here
              //if(j >= 4)
              //{
              //  EPF(("%d!",tri_numvtx));
              //  tanidx++;
              //  continue;
              //}
                int colorIndex;
                int uvId;
                MVector normal;
                //
                // Blind DATA ??? TODO : CHECK !
                //
                if(bk3dBufferIdxBlind[0])
                    bk3dBufferIdxBlind[0]->AddData(vertexList[j]); // TODO : NOT TESTED !!
                if(bk3dBufferIdxBlind[1])
                    bk3dBufferIdxBlind[1]->AddData(vertexList[j]);
                if(bk3dBufferIdxBlind[2])
                    bk3dBufferIdxBlind[2]->AddData(vertexList[j]);
                if(bk3dBufferIdxBlind[3])
                    bk3dBufferIdxBlind[3]->AddData(vertexList[j]);
                //
                // VERTEX IDX
                //
                if(bk3dBufferIdxPosition)
                    bk3dBufferIdxPosition->AddData(vertexList[j]);
                //
                // NORMAL IDX
                //
                if(bk3dBufferIdxVertexNormal)
                {
                    // REALLY ANNOYING ! NO WAY TO GET BACK THE NORMAL IDX OF TRIANGLES !!!!!!!
                    int fuck=0;
                    for(; fuck<vtxCnt; fuck++)
                        if(polyvertices[fuck] == vertexList[j])
                            break;
                    unsigned int nidx = iPoly.normalIndex(fuck);
                    bk3dBufferIdxVertexNormal->AddData(nidx);
                }
                if(bk3dBufferIdxNormal)
                    bk3dBufferIdxNormal->AddData(i); // ID of the current polygon // TODO : TESTED... BUG SOMEWHERE
                //
                // TANGENT BINORM IDX
// TODO !!!!
                //
                if(bk3dBufferIdxTangent) // TODO : NOT TESTED !!
                        bk3dBufferIdxTangent->AddData(tanidx);
                if(bk3dBufferIdxBinormal)
                    bk3dBufferIdxBinormal->AddData(tanidx);
                tanidx++;
                //
                // COLOR IDX
                //
                if(bk3dBufferIdxColor)
                {
                    status = fnMesh.getFaceVertexColorIndex (i, vertexList[j], colorIndex); // TODO : NOT TESTED !!
                    bk3dBufferIdxColor->AddData(colorIndex);
                }
                //
                // UVs
                //
                for(int k=0; k<4; k++)
                {
                    if(!bk3dBufferIdxTexCoord[k])
                        continue;
                    if(k >= nuvsets)
                    {
                        bk3dBufferIdxTexCoord[k]->AddData(0);
                    }
                    else
                    {
                        MSTRING uvsetName = setNames[k];
                        status = iPoly.getUVIndex(j, uvId, &(uvsetName));// fnMesh.getPolygonUVid( i, j, uvId, &(setNames[k]) );
                        if(status.error())
                        {
						    DPF(("Warning : Couldn't get Correctly UV index\n"));
                            bk3dBufferIdxTexCoord[k]->AddData(0);
                        }
                        else if(uvCounts[k][i] == 0)
                        {
                            bk3dBufferIdxTexCoord[k]->AddData(0);
                        }
                        else 
                            bk3dBufferIdxTexCoord[k]->AddData(uvId);
                    }
                }
    //            DPF(("vtxindices = {vtx=%d, norm=%d, col=%d, uv(%d, %d, %d, %d)}\n", vtxindices[j].vtx, vtxindices[j].norm, vtxindices[j].color, vtxindices[j].uv[0], vtxindices[j].uv[1], vtxindices[j].uv[2], vtxindices[j].uv[3]));
                // No done anymore here...
                //if((j == 3) && (!bKeepQuads)) // case of the 4 vertex of a quad : create a new triangle.
                //{
                //    assert(!"TODO");
                //    DPF(("WriteTriIndicesTable() : add to %d : (%d,%d,%d)\n",i,vtxindices[0].vtx,vtxindices[2].vtx,vtxindices[3].vtx));
                //    m_tris_multiidx.push_back(vtxindices[0]);
                //    m_tris_multiidx.push_back(vtxindices[2]);
                //    m_tris_multiidx.push_back(vtxindices[3]);
                //    //
                //    // Add an index for m_shadersindices
                //    //
                //    m_shadersindices.insert(m_shadersindices[s], s);
                //    s++;
                //}
                //else
                {
                    //m_tris_multiidx.push_back(vtxindices[j]);
                    // Strange case of triangle in the group of quads... turn it to quad... for now
                    // not needed anymore
                    //if((j == 2)&&(tri_numvtx == 4)&&(tri_numvtx2 == 3))
                    //{
                    //    assert(!"TODO");
                    //    m_tris_multiidx.push_back(vtxindices[j]);
                    //}
                }
            } //for(j=0; j<(int)tri_numvtx; j++)
        }
    } //for(i=0; i<(int)np; i++, iPoly.next(), s++)
    //int ii = m_tris_multiidx.size();
    //DPF2(("WriteTriIndicesTable() : %d elements pushed in m_tris_multiidx\n", ii));
    DPF2(("tanidx = %d \n", tanidx));
    return status;
}

bk3dlib::PMaterial bk3dTranslator::extractMaterial(MObject o)
{
	MStatus status;
	MFnDependencyNode dn(o, &status);

    bk3dlib::PMaterial bk3dMat = m_bk3dHeader->GetMaterial(dn.name().asChar());
    if(bk3dMat)
        return bk3dMat;

    bk3dMat = bk3dlib::Material::Create(dn.name().asChar());
    m_bk3dHeader->AttachMaterial(bk3dMat);

    std::string shadername;
	bool bRes = GetString(dn, "shader", shadername);
	if(!bRes)
		bRes = GetString(dn, "Shader", shadername);
	if(!bRes)
		bRes = GetString(dn, "SHADER", shadername);
	if(!bRes)
		bRes = GetString(dn, "effect", shadername);
	if(!bRes)
		bRes = GetString(dn, "Effect", shadername);
	if(!bRes)
		bRes = GetString(dn, "EFFECT", shadername);

    std::string techname;
    bRes = GetString(dn, "technique", techname);
	if(!bRes)
		bRes = GetString(dn, "Technique", techname);
	if(!bRes)
		bRes = GetString(dn, "TECHNIQUE", techname);
	if(!bRes)
		bRes = GetString(dn, "tech", techname);
	if(!bRes)
		bRes = GetString(dn, "Tech", techname);
	if(!bRes)
		bRes = GetString(dn, "TECH", techname);

    bk3dMat->setShaderName(shadername.c_str(), techname.c_str() );

	//
	// Try to locate some textures
	//
    std::string diffuseTexture;
	std::string specExpTexture;
	std::string ambientTexture;
	std::string reflectivityTexture;
	std::string transparencyTexture;
	std::string translucencyTexture;
	std::string specularTexture;

    std::string diffuseTextureFile;
	std::string specExpTextureFile;
	std::string ambientTextureFile;
	std::string reflectivityTextureFile;
	std::string transparencyTextureFile;
	std::string translucencyTextureFile;
	std::string specularTextureFile;

    MObject otex;
	#define GETTEXNAME(attr, str, f)\
		otex = getConnectedObjectFromAttribute(o, MString(attr), status);\
		if(status)\
		{\
			std::string texname;\
			MFnDependencyNode dntex(otex, &status);\
			GetString(dntex, "fileTextureName", texname);\
			int offs = texname.find_last_of("/",texname.length());\
			if(offs < 0)\
				offs = texname.find_last_of("\\",texname.length());\
			str##File = std::string(texname.c_str()+offs+1);\
			str = dntex.name().asChar();\
            bk3dMat->f(str.c_str(), str##File.c_str());\
		}
	GETTEXNAME("color", diffuseTexture, setDiffuseTexture);
	GETTEXNAME("cosinePower", specExpTexture, setSpecExpTexture);
	GETTEXNAME("ambientColor", ambientTexture, setAmbientTexture);
	GETTEXNAME("reflectivity", reflectivityTexture, setReflectivityTexture);
	GETTEXNAME("transparency", transparencyTexture, setTransparencyTexture);
	GETTEXNAME("transluence", translucencyTexture, setTranslucencyTexture);
	GETTEXNAME("specularColor", specularTexture, setSpecularTexture);

	//
	// Fill properties
	//
	MFnLambertShader mat(o, &status);
    bk3dMat->setAmbient(mat.ambientColor()[0], mat.ambientColor()[1], mat.ambientColor()[2]);
    bk3dMat->setDiffuse(
        mat.color()[0] * mat.diffuseCoeff(),
        mat.color()[1] * mat.diffuseCoeff(),
        mat.color()[2] * mat.diffuseCoeff() );
    bk3dMat->setTranslucency(mat.translucenceCoeff());
    bk3dMat->setTransparency(mat.transparency()[0], mat.transparency()[1], mat.transparency()[2]);
	MFnReflectShader ref(o, &status);
    bk3dMat->setSpecular(ref.specularColor()[0], ref.specularColor()[1], ref.specularColor()[2]);
	MFnPhongShader ph(o, &status);
	if(status)
		bk3dMat->setSpecexp(ph.cosPower());
	else
		bk3dMat->setSpecexp(30.0);

	return bk3dMat;
}

// the only job of this method : find back the related Handle
void bk3dTranslator::extractIKEffector(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransformParent)
{
    MStatus status;
    MObject oIKHandle = getConnectedObject(dagPath.node(), "handlePath", false, true, &status);
    if(!status)
        return;
    // we need to find back the DAGPath from the node...
    // we assume the Node is used only once
    MDagPath dp;
    status = MDagPath::getAPathTo(oIKHandle, dp);
    if(!status)
        return;
    LPCSTR dbgName = dp.fullPathName().asChar();
    // take the root as a starting point
    int l = dp.length() - 1;
    if(l > 0)
        dp.pop(l);
    dbgName = dp.fullPathName().asChar();
    // Now let's run the recursive walk-through of DAGPath :
    status = readDagPath(dp, 0, bIntermediate, NULL);
}

void bk3dTranslator::extractIKHandle(MDagPath &dagPath, int level, bool bIntermediate, bk3dlib::PBone bk3dTransformParent)
{
    MStatus status;
    MDagPath startJoint;
    MDagPath effector;
    MFnIkHandle ikHandle(dagPath.node());
    status = ikHandle.getStartJoint(startJoint);
    if(!status)
        return;
    status = ikHandle.getEffector(effector);
    if(!status)
        return;
    // Note: many other parameters could be exporter... to be continued

    LPCSTR handleName = dagPath.partialPathName().asChar();
    bk3dlib::PIKHandle bk3dIKHandle = m_bk3dHeader->GetIKHandle(handleName);
    if(!bk3dIKHandle)
    {
        bk3dIKHandle = bk3dlib::IKHandle::Create(handleName);
        m_bk3dHeader->AttachIKHandle(bk3dIKHandle);
    }
    bk3dIKHandle->SetParent(bk3dTransformParent);
    // Now we need to perform a partial walk-through of the DAG paths for effector down to startJoint
    MDagPath dp(effector);
    dp.pop();
    LPCSTR dbgName = dp.fullPathName().asChar();
    std::stack<MDagPath> dpStack;
    int l = dp.length();
    do {
        dbgName = dp.fullPathName().asChar();
        dpStack.push(dp);
        dp.pop();
        l--;
    } while(l > 0);
    // Now let's unroll the stack so we start from the root of the transform
    bk3dlib::PBone bk3dTransform = NULL;
    int level2 = 0;
    while(!dpStack.empty())
    {
        dp = dpStack.top();
        dbgName = dp.fullPathName().asChar();
#if 1
        if(bExportAnimCVs)
        {
            exportCurve(dp.node(), 0);
        }
#endif
        // Case of a Transformation that is also a IK Handle
        assert( !dp.node().hasFn(MFn::kIkHandle));
        assert( !dp.node().hasFn(MFn::kIkEffector));

        // process ONLY the transform and no children : this will be/was done in bk3dTranslator::readDagPath()
        bk3dTransform = extractTransform(dp, level2, bIntermediate, NULL, bk3dTransform);
        {
            //----> Process the children
            int numchildren = dp.childCount();
            for(int i=0; i< numchildren; i++)
            {
                MObject childobj;
                childobj = dp.child( i, &status);
                MDagPath dagPathChild(dp);
                dagPathChild.push(childobj);
                status = readDagPath(dagPathChild, level2+1, bIntermediate, bk3dTransform);
            }
            //m_pCurTransform = pCurTransform;
        }
        level2++;
        dpStack.pop();
    }
    // positions of effector and Handle
    MFnTransform TrHandle(dagPath.node());
    MVector p = TrHandle.getTranslation(MSpace::kObject, &status);
    bk3dIKHandle->setHandlePos((float)p.x, (float)p.y, (float)p.z);
    MFnTransform TrEffector(effector.node());
    p = TrEffector.getTranslation(MSpace::kObject, &status);
    //bk3dIKHandle->setEffectorPos((float)p.x, (float)p.y, (float)p.z);
    // Now we can reference these transforms in the IK Handle
    bk3dlib::PBone pEnd = m_bk3dHeader->GetTransform(startJoint.partialPathName().asChar());
    pEnd->SetTailPos((float)p.x, (float)p.y, (float)p.z);
    bk3dIKHandle->setEffectorTransformEnd(pEnd);
    effector.pop();
    bk3dlib::PBone pStart = m_bk3dHeader->GetTransform(effector.partialPathName().asChar());
    bk3dIKHandle->setEffectorTransformStart(pStart);
    bk3dIKHandle->setWeight((float)ikHandle.weight());
    // NOTE: this would be a good thing to then sort the Handles according to their priorities
    // so that the bk3d data are ready for the default use
    bk3dIKHandle->setPriority(ikHandle.priority());
    //
    // Update the connections of the curves. m_curvesSorted has them with owner info and component to which they are connected
    //
    int N = m_bk3dHeader->GetNumCurves();
    for(int n=0; n<N; n++)
    {
        bk3dlib::CurveVec *pCVV = m_bk3dHeader->GetCurveVec(n);
        LPCSTR name = pCVV->GetName();
        MSTRING mname(name);
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
                if(sarray[1] == MString("scale"))
                {
                    EPF(("Can't assign some scale to a IKHANDLE\n"));
                    //pCVV->Connect(bk3dIKHandle, bk3dlib::IKHANDLE_SCALE);
                }
                else if(sarray[1] == MString("rotation"))
                {
                    EPF(("Can't assign some rotation to a IKHANDLE, YET...\n"));
                    //pCVV->Connect(bk3dIKHandle, bk3dlib::IKHANDLE_EULERROT);
                }
                else if(sarray[1] == MString("translate"))
                {
                    pCVV->Connect(bk3dIKHandle, bk3dlib::IKHANDLE_POS);
                }
                else if(sarray[1] == MString("priority"))
                {
                    pCVV->Connect(bk3dIKHandle, bk3dlib::IKHANDLE_PRIORITY);
                }
                else if(sarray[1] == MString("weight"))
                {
                    pCVV->Connect(bk3dIKHandle, bk3dlib::IKHANDLE_WEIGHT);
                }
                else
                {
                    EPF(("Cannot connect component : %s\n", sarray[1].asChar()));
                }
            }
        }
    }
}

