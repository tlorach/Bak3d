#include "builder.h"
/*------------------------------------------------------------------
	Transforms DOF
  ------------------------------------------------------------------*/
CTransformDOF::CTransformDOF(CTransformDOF *p)
{
    if(p) {
        m_DOFAlpha = p->m_DOFAlpha;
        m_mode = p->m_mode;
        m_AxisLimitStart = p->m_AxisLimitStart;
        m_AxisLimitRange = p->m_AxisLimitRange;
        m_quat = p->m_quat;
    } else {
        m_DOFAlpha = 0.0f;
        m_mode = bk3dlib::DOF_CONE;
        m_AxisLimitStart = 0.0f;
        m_AxisLimitRange = 0.0f;
        m_quat = quatf(0,0,0,1);
    }
}
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
CBone::CBone(const char * name, CBone *p)
{
	if(name)
		m_name = name;
#define ZEROFLOATS(v, n) memset (v, 0, sizeof(float)*n);
#define IDENTITY4x4(matrix)\
    memset (matrix.mat_array, 0, sizeof(float)*16);\
    matrix.a00 = 1.0f; matrix.a11 = 1.0f; matrix.a22 = 1.0f; matrix.a33 = 1.0f; 
    m_validComps = 0;
	IDENTITY4x4(m_matrix);
	IDENTITY4x4(m_abs_matrix);
	IDENTITY4x4(m_bindpose_matrix);     
	ZEROFLOATS(m_pos.vec_array,3);
    m_Quat = m_Quat.Identity;
    m_abs_Quat = m_abs_Quat.Identity;
    ZEROFLOATS(m_posBoneTail.vec_array,3);
    
    m_userData = NULL;
    m_TransfDOF = NULL;

    m_transform = NULL;
    m_transformSimple = NULL;
    m_bone = NULL;
    m_parentTransf = NULL;

	bDirty = true;

}
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
CTransformSimple::CTransformSimple(const char * name, CTransformSimple *p) : CBone(name, p)
{
	ZEROFLOATS(m_scale.vec_array,3);
}
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
CTransform::CTransform(const char * name, CTransform *p) : CTransformSimple(name, p)
{
	ZEROFLOATS(m_rotation.vec_array,3);    				// Euler Rotation in degres
	strcpy(m_rotationOrder, "xyz");  				// 3 chars for "xyz" or any other
	ZEROFLOATS(m_scalePivot.vec_array,3);
	ZEROFLOATS(m_scalePivotTranslate.vec_array,3);
	ZEROFLOATS(m_rotationPivot.vec_array,3);
	ZEROFLOATS(m_rotationPivotTranslate.vec_array,3);

	ZEROFLOATS(m_rotationOrientation.comp,4); 		//Quaternion
	ZEROFLOATS(m_jointOrientation.comp, 4);
    ZEROFLOATS(m_posBoneTail.vec_array,3);
}

size_t	CBone::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::Bone);
	if(m_childrenTransf.size() > 0)
	{
		sz += sizeof(bk3d::TransformPool);
	}

	if(m_childrenTransf.size() > 1)
	{
		relocationSlots += (int)m_childrenTransf.size()-1;
		sz += (m_childrenTransf.size())*sizeof(bk3d::Ptr64<bk3d::Transform>);
	}
#if 1
	//
	// How many curves (TODO: could be other things) are connected to this transform
	//
	if(connectedCurveVectors.size() > 0)
	{
		relocationSlots += 1; // for pFloatArrays
		sz += sizeof(bk3d::FloatArrayPool);
		sz += sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size()-1);
	}
	relocationSlots += (int)connectedCurveVectors.size()*2; // 2 ptrs per FloatArray
#endif
	DPF(("Transform : %d\n", sz));
	return sz;
}

size_t	CTransformSimple::getTotalSize(int &relocationSlots)
{
    size_t sz = CBone::getTotalSize(relocationSlots);
    size_t szdiff = sizeof(bk3d::TransformSimple) - sizeof(bk3d::Bone);
	return sz + szdiff;
}

size_t	CTransform::getTotalSize(int &relocationSlots)
{
    size_t sz = CTransformSimple::getTotalSize(relocationSlots);
    size_t szdiff = sizeof(CTransform) - sizeof(CTransformSimple);
	return sz + szdiff;
}

#define COPYVEC(s, v, b, a, n) { memcpy(s->v##b, m_##v##a, sizeof(m_##v##a[0])*n );}
bk3d::Bone	*CBone::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//
	// create and copy data
	//
    if(pTr == NULL)
	    m_bone = new TBone;
    else
	    m_bone = static_cast<TBone*>(pTr);
	transform2bk3d[static_cast<CBone*>(this)] = m_bone;
    m_bone->parentPool = transformPool;
    m_bone->ID = transformPool->nBones;
	SETNAME(m_bone, m_name.c_str()); // default name for the node
    bk3d::BoneDataType& boneData = transformPool->tableBoneData.p[m_bone->ID];

    m_bone->pBoneData = &boneData;
    m_bone->pMatrixAbs = &transformPool->tableMatrixAbs[m_bone->ID];
    m_bone->pMatrixAbsInvBindposeMatrix = &transformPool->tableMatrixAbsInvBindposeMatrix[m_bone->ID];
    m_bone->pMatrixInvBindpose = &transformPool->tableMatrixInvBindpose[m_bone->ID];

    boneData.validComps = m_validComps;

    if(m_validComps & TRANSFCOMP_matrix)
	    memcpy(boneData.matrix, m_matrix.mat_array,sizeof(float)*16);
    else
	    memcpy(boneData.matrix, array16_id,sizeof(float)*16);
    // when the position is valid, overwrite the translation part of the matrix
    if(m_validComps & TRANSFCOMP_pos)
	    memcpy(boneData.matrix.pos(), m_pos.vec_array,sizeof(float)*3);
	memcpy(boneData.quat, m_Quat.comp,sizeof(float)*4);
	memcpy(boneData.quatAbs, m_abs_Quat.comp,sizeof(float)*4);
	memcpy(boneData.posBoneTail, m_posBoneTail.vec_array,sizeof(float)*3); 			//bone tail pos

	memcpy(m_bone->MatrixAbs(), m_abs_matrix.mat_array,sizeof(float)*16);
    // bindpose is the abs_matrix inverted: we assume the model skeleton is in the right bindpose position when exporting
    // bindpose job: bring the vertex back to local space. But here we do a simplified approach... from world to local...
    //invert(m_bindpose_matrix, m_abs_matrix);
	memcpy(m_bone->MatrixInvBindpose(), m_bindpose_matrix.mat_array,sizeof(float)*16);
    mat4f Mabsinvbp = m_abs_matrix * m_bindpose_matrix;
	memcpy(m_bone->MatrixAbsInvBindposeMatrix(), Mabsinvbp.mat_array,sizeof(float)*16);

    if(bDirty)
	    boneData.bDirty = 1;

	//
	// update the pool : append the transform into it
	//
	transformPool->pBones[transformPool->nBones++] = m_bone;
	//
	// Set parent. But we need to check 3 slots
	//
    bk3d::Bone* pBone = NULL;
    if(m_parentTransf) {
        if(m_parentTransf->m_bone)
            pBone = m_parentTransf->m_bone;
        else if(m_parentTransf->m_transformSimple)
            pBone = m_parentTransf->m_transformSimple;
        else if(m_parentTransf->m_transform)
            pBone = m_parentTransf->m_transform;
    }
    m_bone->pParent = pBone;
    m_bone->BoneData().parentID = pBone ? pBone->ID : 0xFFFF;
	//
	// process children
	//
	int additionalsz = 0;
	if(m_childrenTransf.size() > 1) 
		additionalsz = sizeof(bk3d::Ptr64<bk3d::Transform>)*(int)m_childrenTransf.size()-1;

    // The children connections will be done later.
    // For now, we just keep track of the offset where to find the IDs
    // hack: using offsetIKHandles to temporarily save the available offset
	boneData.childrenListID = 0;
	m_bone->pChildren = NULL;
	if(m_childrenTransf.size() > 0)
	{
	    boneData.childrenListID = childrenOffset;
        transformPool->tableChildrenLists[boneData.childrenListID] = (unsigned int)m_childrenTransf.size();
        childrenOffset += 1+(unsigned int)m_childrenTransf.size(); // [nchildren, childID0, childID1...
		m_bone->pChildren = new(additionalsz) TTransformPool2;
        // The children connections will be done later.
        // Because we first need to put them in the same order (for skinning matrix tables)
		m_bone->pChildren->n = (int)m_childrenTransf.size();
		for(int i=0; i< m_bone->pChildren->n; i++)
			m_bone->pChildren->p[i] = NULL;
	}
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_bone->nodeByteSize = (unsigned int)nodeByteSize;
	m_bone->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

#if 1
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(connectedCurveVectors.size() > 0)
	{
		m_bone->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size() - 1)) TFloatArrayPool;
		m_bone->pFloatArrays->n = (int)connectedCurveVectors.size();
		std::set< CurveVecConnection >::iterator iC = connectedCurveVectors.begin();
		int i=0;
		while(iC != connectedCurveVectors.end())
		{
			// get back the pointer in the structure that we are currently baking
			MapMayaCVPtrs::const_iterator iPtr = CVPtrs.find(iC->first);
			assert(iPtr != CVPtrs.end() );
			bk3d::MayaCurveVector *pMCurve = iPtr->second;
			assert(pMCurve->pFloatArray);
			bk3d::FloatArrayPool::Connection *pConn = m_bone->pFloatArrays->p + i;
			pConn->p = pMCurve->pFloatArray;
			switch(iC->second)
			{
			case bk3dlib::TRANSF_POS:
				sprintf_s(pConn->destName, 32, "POS_%s", m_bone->name);
				pConn->pfTarget = m_bone->Pos();
				break;
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - handle more connection cases with other components of transformation")
			//case bk3dlib::TRANSF_ROTQUAT:
			//case bk3dlib::TRANSF_EULERJOINT:
			default:
				break;
			}
			++iC; ++i;
		}
	}
#endif
	DPF(("CBone sz : %d\n", nodeByteSize));
	return m_bone;
}
bk3d::Bone	*CTransformSimple::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//
	// create and copy data
	//
    if(pTr == NULL)
	    m_transformSimple = new TTransformSimple;
    else
	    m_transformSimple = static_cast<TTransformSimple*>(pTr);

    CBone::build(transformPool, CVPtrs, transform2bk3d, m_transformSimple, childrenOffset, effectorOffset);

	memcpy(m_transformSimple->Scale(), m_scale.vec_array,sizeof(m_scale));
#if 1
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(connectedCurveVectors.size() > 0)
	{
		m_transformSimple->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size() - 1)) TFloatArrayPool;
		m_transformSimple->pFloatArrays->n = (int)connectedCurveVectors.size();
		std::set< CurveVecConnection >::iterator iC = connectedCurveVectors.begin();
		int i=0;
		while(iC != connectedCurveVectors.end())
		{
			// get back the pointer in the structure that we are currently baking
			MapMayaCVPtrs::const_iterator iPtr = CVPtrs.find(iC->first);
			assert(iPtr != CVPtrs.end() );
			bk3d::MayaCurveVector *pMCurve = iPtr->second;
			assert(pMCurve->pFloatArray);
			bk3d::FloatArrayPool::Connection *pConn = m_transformSimple->pFloatArrays->p + i;
			pConn->p = pMCurve->pFloatArray;
			switch(iC->second)
			{
			case bk3dlib::TRANSF_SCALE:
				sprintf_s(pConn->destName, 32, "SCALE_%s", m_transformSimple->name);
				pConn->pfTarget = m_transformSimple->Scale();
				break;
			default:
				break;
			}
			++iC; ++i;
		}
	}
#endif
	DPF(("CTransformSimple sz : %d\n", nodeByteSize));
	return m_transformSimple;
}
bk3d::Bone	*CTransform::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//
	// create and copy data
	//
    if(pTr == NULL)
	    m_transform = new TTransform;
    else
	    m_transform = static_cast<TTransform*>(pTr);

    CTransformSimple::build(transformPool, CVPtrs, transform2bk3d, m_transform, childrenOffset, effectorOffset);

    bk3d::MayaTransformData& mData = transformPool->tableMayaTransformData.p[m_transform->ID];
    //m_transform->pMayaTransformData = &mData; TODO

	memcpy(mData.rotation, m_rotation.vec_array,sizeof(m_rotation));    				// Euler Rotation in degres
	memcpy(mData.rotationOrder, m_rotationOrder,sizeof(m_rotationOrder));  				// 3 chars for "xyz" or any other
	memcpy(mData.scalePivot, m_scalePivot.vec_array,sizeof(m_scalePivot));
	memcpy(mData.scalePivotTranslate, m_scalePivotTranslate.vec_array,sizeof(m_scalePivotTranslate));
	memcpy(mData.rotationPivot, m_rotationPivot.vec_array,sizeof(m_rotationPivot));
	memcpy(mData.rotationPivotTranslate, m_rotationPivotTranslate.vec_array,sizeof(m_rotationPivotTranslate));
	memcpy(mData.rotationOrientation, m_rotationOrientation.comp,sizeof(m_rotationOrientation)); 		//Quaternion
	memcpy(mData.jointOrientation, m_jointOrientation.comp,sizeof(m_jointOrientation)); 			//Quaternion
#if 1
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(connectedCurveVectors.size() > 0)
	{
		m_transform->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size() - 1)) TFloatArrayPool;
		m_transform->pFloatArrays->n = (int)connectedCurveVectors.size();
		std::set< CurveVecConnection >::iterator iC = connectedCurveVectors.begin();
		int i=0;
		while(iC != connectedCurveVectors.end())
		{
			// get back the pointer in the structure that we are currently baking
			MapMayaCVPtrs::const_iterator iPtr = CVPtrs.find(iC->first);
			assert(iPtr != CVPtrs.end() );
			bk3d::MayaCurveVector *pMCurve = iPtr->second;
			assert(pMCurve->pFloatArray);
			bk3d::FloatArrayPool::Connection *pConn = m_transform->pFloatArrays->p + i;
			pConn->p = pMCurve->pFloatArray;
			switch(iC->second)
			{
			case bk3dlib::TRANSF_EULERROT:
				sprintf_s(pConn->destName, 32, "EULERROT_%s", m_transform->name);
				pConn->pfTarget = mData.rotation;
				break;
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - handle more connection cases with other components of transformation")
			//case bk3dlib::TRANSF_VISIBILITY:
			//case bk3dlib::TRANSF_EULERJOINT:
			default:
				break;
			}
			++iC; ++i;
		}
	}
#endif
	DPF(("CTransform sz : %d\n", nodeByteSize));
	return m_transform;
}
//void	CTransform::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Transform *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//	PTR2OFFSET(bk3d::Transform, p->pParent);
//	if(p->getNumChildren())
//	{
//		for(int i=0; i<p->getNumChildren()->n; i++)
//		{
//			PTR2OFFSET(bk3d::Transform, p->getChild(i).p);
//		}
//		PTR2OFFSET(bk3d::TransformPool, p->pChildren);
//	}
//	if(p->pFloatArrays) for(int i=0; i< p->pFloatArrays->n; i++)
//	{
//		PTR2OFFSET(bk3d::FloatArray, p->pFloatArrays->p[i].p);
//		PTR2OFFSET(float, p->pFloatArrays->p[i].pfTarget);
//	}
//    PTR2OFFSET(bk3d::FloatArrayPool, p->pFloatArrays);
//	DPF(("CTransform slots : %d\n", localSlots));
//}

bk3d::TransformDOF	*CTransformDOF::build(bk3d::Bone* pTr)
{
    bk3d::TransformDOF* pTTrDOF = pTr->parentPool->tableTransformDOF.p + pTr->ID; //BoneData().DOFID;
    pTTrDOF->DOFAlpha = m_DOFAlpha;
    // wild casting... enough if we know both are the same
    pTTrDOF->mode = (bk3d::TransformDOFMode)m_mode;
    pTTrDOF->AxisLimitStart = m_AxisLimitStart;
    pTTrDOF->AxisLimitRange = m_AxisLimitRange;
    memcpy(pTTrDOF->Quat(), m_quat.comp, sizeof(m_quat));
    quatf Qabs;
    Qabs = Qabs * m_quat; 
    memcpy(pTTrDOF->QuatAbs(), Qabs.comp, sizeof(Qabs));
    return pTTrDOF;
}

bool CBone::setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d)
{
    bk3d::Bone *            pTr = NULL;
    LPCSTR                  DOFName = NULL;
    // Find back the bk3d transform created from this CTransform
    for(int i=0; i<transformPool->nBones; i++)
    {
        if(!strncmp(m_name.c_str(), transformPool->pBones[i]->name, NODENAMESZ))
        {
            pTr = transformPool->pBones[i];
            break;
        }
    }
    if(pTr == NULL)
        return false;
    pTr = m_bone;
    if(pTr == NULL)
        pTr = m_transformSimple;
    if(pTr == NULL)
        pTr = m_transform; // the cooked struct

    if(m_TransfDOF)
    {
        pTr->pDOF = m_TransfDOF->build(pTr);
#pragma message("pTr->BoneData().DOFID = TODO")
        pTr->BoneData().DOFID = 0xFFFF; // TODO!!!
    } else {
        pTr->pDOF = NULL;
        pTr->BoneData().DOFID = 0xFFFF;
    }

	if(m_childrenTransf.size() > 0)
	{
        unsigned int * pChildrenIDs = transformPool->tableChildrenLists.p + pTr->BoneData().childrenListID;
        DBGASSERT(pTr->BoneData().childrenListID > 0)
		for(unsigned int i=0; i< m_childrenTransf.size(); i++)
        {
            bk3d::Bone *       pTrChild = NULL;
            int j;
            for(j=0; j<transformPool->nBones; j++)
            {
                if(!strncmp(m_childrenTransf[i]->m_name.c_str(), transformPool->pBones[j]->name, NODENAMESZ))
                {
                    pTrChild = transformPool->pBones[j];
                    break;
                }
            }
            assert(j<transformPool->nBones);
			pChildrenIDs[1+i] = pTrChild->ID;
            assert(pTrChild);
			pTr->pChildren->p[i] = pTrChild;
        }
	} else {
        DBGASSERT(pTr->BoneData().childrenListID == 0)
    }

char tmpstr[300];
sprintf(tmpstr, "  Children (%d/%d)", pTr->getNumChildren(), pTr->pChildren ? pTr->pChildren->n : 0);
printf(tmpstr);
for(int i=0; i<pTr->getNumChildren(); i++)
{
    bk3d::Bone *pCh = pTr->getChild(i);
    sprintf(tmpstr, " %d/%d", pCh->ID, pTr->pChildren && (i<pTr->pChildren->n) ? pTr->pChildren->p[i]->ID : -1);
    printf(tmpstr);
}
printf("\n");
    return true;
}

