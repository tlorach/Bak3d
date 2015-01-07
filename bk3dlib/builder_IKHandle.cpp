#include "builder.h"
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
size_t	CIKHandle::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	// Add reloc slots for the children ptrs
    int numTransforms = 0;
    if(m_effectorTransforms.size() > 0)
    {
    } else {
        CBone * pTr = m_effectorTransformStart;
        m_effectorTransforms.push_back(pTr);
        while(pTr)
        {
            if(pTr == m_effectorTransformEnd)
                break;
            pTr = static_cast<CBone*>(pTr->GetParent());
            m_effectorTransforms.push_back(pTr);
        }
    }
    numTransforms = (int)m_effectorTransforms.size();
	relocationSlots += numTransforms;
    size_t sz = CBone::getTotalSize(relocationSlots);
    // add the difference in size of IKHandle comared to already calculated size 
	size_t szdiff = sizeof(bk3d::IKHandle) - sizeof(bk3d::Bone);
    sz += szdiff;
	
	if(numTransforms > 0)
	{
		sz += sizeof(bk3d::TransformPool);
	}

	if(numTransforms > 1)
	{
		relocationSlots += numTransforms-1;
		sz += (numTransforms-1)*sizeof(bk3d::Ptr64<bk3d::Transform>);
	}
    for(int i=0; i<m_weights.size(); i++)
    {
        // any value != 1.0 forces us to take the table into account
        if(m_weights[i] != 1.0)
        {
	        if(m_weights.size() > 0)
	        {
		        sz += sizeof(bk3d::FloatPool);
	        }

	        if(m_weights.size() > 1)
	        {
		        sz += (m_weights.size()-1)*sizeof(float);
	        }
            break;
        }
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
	DPF(("IKHandle : %d\n", sz));
	return sz;
}

bk3d::Bone *CIKHandle::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
    //
    // reach the place to add details on IKHandle
    // CBone::build increments nBones, so do it before
    // the offset 0 starts at offsetIKHandles... so subract it
    //
    // first time initialization: if 0, it means we must set it to the first handle
    if(transformPool->offsetIKHandles == 0)
        transformPool->offsetIKHandles = transformPool->nBones;
    bk3d::IKHandleData* pIKHD = transformPool->tableIKHandleData.p + (transformPool->nBones - transformPool->offsetIKHandles);
	//
	// create and copy data
	//
    m_bk3dHandle = new TIKHandle;
    m_bk3dHandle->pIKHandleData = pIKHD;
    bk3d::Bone	* ttr = CBone::build(transformPool, CVPtrs, transform2bk3d, m_bk3dHandle, childrenOffset, effectorOffset);
    switch(m_mode)
    {
    case Default:
        m_bk3dHandle->nodeType = NODE_IKHANDLE;
        break;
    case RotateInfluence:
        m_bk3dHandle->nodeType = NODE_IKHANDLEROTATEINFLUENCE;
        break;
    case RollInfluence:
        m_bk3dHandle->nodeType = NODE_IKHANDLEROLLINFLUENCE;
        break;
    }
	//ikhandleMap[this] = m_bk3dHandle;

	SETNAME(m_bk3dHandle, m_name.c_str()); // default name for the node

    pIKHD->weight = m_weight;
    pIKHD->maxIter = m_maxIter;
    pIKHD->priority = m_priority;

    //memcpy(m_bk3dHandle->effectorPos, m_EffectorPos.vec_array, sizeof(m_EffectorPos) );
    // Removed: now IKHandle is the transform
    //memcpy(m_bk3dHandle->handlePos, m_HandlePos.vec_array, sizeof(m_HandlePos) );

	//ikHandlePool->p[ikHandlePool->n++] = m_bk3dHandle;
    // Removed: now IKHandle is the transform
    //MapTransform2bk3d::const_iterator iT = transform2bk3d.find(m_handleTransforms);
    //if(iT != transform2bk3d.end())
    //    m_bk3dHandle->handleTransform = iT->second;
    //else
    //    m_bk3dHandle->handleTransform = NULL;

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_bk3dHandle->nodeByteSize = (unsigned int)nodeByteSize;
	m_bk3dHandle->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

    // transforms of IK chain is now always in m_effectorTransforms (because of CIKHandle::getTotalSize)
    int numTransforms = (int)m_effectorTransforms.size();
    pIKHD->numEffectors = numTransforms;
    if(numTransforms > 0)
    {
        m_bk3dHandle->pEffectorTransforms = new(sizeof(bk3d::Ptr64<bk3d::Transform>)*(numTransforms - 1)) TTransformPool2;
        m_bk3dHandle->pEffectorTransforms->n = numTransforms;
        pIKHD->effectorWeightAndTransformListID = effectorOffset;
        bk3d::EffectorTransformAndWeight* pTandW = transformPool->tableEffectorWeights.p + effectorOffset;
        effectorOffset += numTransforms + 1; //+1 because the first pair contains 'numTransforms'
        pTandW->transformID = numTransforms;
        pTandW->weight = 0.0f;
        pTandW++;
            // the references to transforms will be done in CIKHandle::setupDOFAndChildrenLinks
        assert((m_weights.size() == numTransforms)||((m_weights.size() == 0)));
        for(int i=0; i<numTransforms; i++)
        {
            pTandW->weight = m_weights.size()>0 ? m_weights[i] : 1.0f;
            pTandW++;

            // will be completed later in CBone::setupDOFAndChildrenLinks
            m_bk3dHandle->pEffectorTransforms->p[i] = NULL;
        }
    }
    int numWeights = (int)m_weights.size();
    m_bk3dHandle->pEffectorWeights = NULL;
    for(int i=0; i<numWeights; i++)
    {
        // any value != 1.0 forces us to take the table into account
        if(m_weights[i] != 1.0)
        {
            m_bk3dHandle->pEffectorWeights = new(sizeof(float)*(numWeights - 1)) TFloatPool;
            m_bk3dHandle->pEffectorWeights->n = numWeights;
            for(int i=0; i<numWeights; i++)
            {
		        m_bk3dHandle->pEffectorWeights->f[i] = m_weights[i];
            }
            break;
        }
    }
#if 1
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(connectedCurveVectors.size() > 0)
	{
		m_bk3dHandle->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size() - 1)) TFloatArrayPool;
		m_bk3dHandle->pFloatArrays->n = (int)connectedCurveVectors.size();
		std::set< CurveVecConnection >::iterator iC = connectedCurveVectors.begin();
		int i=0;
		while(iC != connectedCurveVectors.end())
		{
			// get back the pointer in the structure that we are currently baking
			MapMayaCVPtrs::const_iterator iPtr = CVPtrs.find(iC->first);
			assert(iPtr != CVPtrs.end() );
			bk3d::MayaCurveVector *pMCurve = iPtr->second;
			assert(pMCurve->pFloatArray);
			bk3d::FloatArrayPool::Connection *pConn = m_bk3dHandle->pFloatArrays->p + i;
			pConn->p = pMCurve->pFloatArray;
			switch(iC->second)
			{
			case bk3dlib::IKHANDLE_POS:
				sprintf_s(pConn->destName, 32, "POS_%s", m_bk3dHandle->name);
				pConn->pfTarget = m_bk3dHandle->Pos() ;//handlePos;
				break;
			case bk3dlib::IKHANDLE_WEIGHT:
                pMCurve->pFloatArray->dim = 1; // force to 1 by safety
				sprintf_s(pConn->destName, 32, "WEIGHT_%s", m_bk3dHandle->name);
				pConn->pfTarget = &pIKHD->weight;
				break;
			case bk3dlib::IKHANDLE_PRIORITY:
                pMCurve->pFloatArray->dim = 1; // force to 1 by safety
				sprintf_s(pConn->destName, 32, "PRI_%s", m_bk3dHandle->name);
				pConn->piTarget = &pIKHD->priority;
				break;
			default:
				sprintf_s(pConn->destName, 32, "NOTARGET_%s", m_bk3dHandle->name);
				pConn->pfTarget = NULL;
				EPF(("Transformation component ' %s ' not handled for connection, yet\n", pMCurve->name));
				//assert(1);
				break;
			}
			++iC; ++i;
		}
	}
#endif
	DPF(("CIKHandle sz : %d\n", nodeByteSize));
	return ttr;
}

bool CIKHandle::setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d)
{
    bool bRes = CBone::setupDOFAndChildrenLinks(transformPool, transform2bk3d);
    int numTransforms = (int)m_effectorTransforms.size();
    if(numTransforms > 0)
    {
        bk3d::IKHandleData& iKHD = m_bk3dHandle->IKHandleData();
        bk3d::EffectorTransformAndWeight* pTandW = transformPool->tableEffectorWeights.p + iKHD.effectorWeightAndTransformListID;
        pTandW++;
        CBone * pTr = NULL;
        for(int i=0; i<numTransforms; i++)
        {
            pTr = m_effectorTransforms[i];

		    MapTransform2bk3d::const_iterator iT = transform2bk3d.find(pTr);
		    assert(iT != transform2bk3d.end());
		    assert(iT->second);
		    // Now we can set the reference to the transform
		    m_bk3dHandle->pEffectorTransforms->p[i] = iT->second;

            bk3d::Bone *       pTrChild = NULL;
            int j;
            for(j=0; j<transformPool->nBones; j++)
            {
                if(!strncmp(pTr->GetName(), transformPool->pBones[j]->name, NODENAMESZ))
                    break;
            }
            assert(j<transformPool->nBones);
            pTandW->transformID = j;
            pTandW++;
        }
    }
    return true;
}

