#include "builder.h"
#include "bk3dPtr2Offset.h"

/*------------------------------------------------------------------
	File Header
  ------------------------------------------------------------------*/
CFileHeader::CFileHeader(LPCSTR name)
{
	if(name)
		m_name  = name;
	m_fileHeader  = NULL;
    m_bk3dPool    = NULL;
    m_bk3dPool2   = NULL;
}

void	CFileHeader::OffsetToPtr()
{
	m_fileHeader->resolvePointers(m_bk3dPool2->getPool());
	//m_fileHeader->debugDumpAll();
}

size_t	CFileHeader::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::FileHeader);
	//
	// curve pool
	//
#if 1
	sz += sizeof(bk3d::MayaCurvePool) + sizeof(bk3d::Ptr64<bk3d::MayaCurveVector>)*(m_mayacurves.size()-1);
	relocationSlots += (int)m_mayacurves.size(); // reloc for elements in the pool
	MapCurveVec::iterator iCV = m_mayacurves.begin();
	while(iCV != m_mayacurves.end())
	{
		CMayaCurveVector* pCV = iCV->second;
		sz += (unsigned int)pCV->getTotalSize(relocationSlots);
		iCV++;
	}
	sz += sizeof(bk3d::QuatCurvePool) + sizeof(bk3d::Ptr64<bk3d::QuatCurve>)*(m_quatcurves.size()-1);
	relocationSlots += (int)m_quatcurves.size(); // reloc for elements in the pool
	MapQuatCurve::iterator iQCV = m_quatcurves.begin();
	while(iQCV != m_quatcurves.end())
	{
		CQuatCurve* pCV = iQCV->second;
		sz += (unsigned int)pCV->getTotalSize(relocationSlots);
		iQCV++;
	}
#endif
	//
	// transformpool1
	//
#if 1
	sz += sizeof(bk3d::TransformPool);
	if(m_transforms.size() > 1)
		sz += sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transforms.size()-1 );
	relocationSlots += (int)m_transforms.size();
    // TransformPool pointers:
    // - tableMatrixAbs;
    // - tableMatrixInvBindpose;
    // - tableMatrixAbsInvBindposeMatrix;
    // - tableBoneData;                  
    // - tableChildrenLists;             
    //// - tableIKHandlesLists;            
    // - tableMayaTransformData;         
    // - tableTransformDOF;
    // - tableIKHandleData;
    // - tableEffectorWeights;
    relocationSlots += 9;//10;
	VecTransform::iterator iT = m_transforms.begin();
	while(iT != m_transforms.end())
	{
		// recusrively add transforms in the pool. Recursive process helps to easily put them all
		// only handle root transforms since the rest will be done recursively
		//if(iT->second->GetParent() == NULL)
			sz += (unsigned int)(*iT)->getTotalSize(relocationSlots);
		++iT;
	}
#endif
	//
	// IKHandles
	//
	sz += sizeof(bk3d::IKHandlePool);
	if(m_ikHandles.size()>1)
		sz += sizeof(bk3d::Ptr64<bk3d::IKHandle>)*(m_ikHandles.size()-1);
	relocationSlots += (int)m_ikHandles.size(); // reloc for elements in the pool
    // Not needed: done previously with m_transforms
	//MapIKHandle::iterator iIKH = m_ikHandles.begin();
	//while(iIKH != m_ikHandles.end())
	//{
	//	sz += iIKH->second->getTotalSize(relocationSlots);
	//	++iIKH;
	//}
	//
	// Materials
	//
	sz += sizeof(bk3d::MaterialPool);
	if(m_materials.size()>1)
		sz += sizeof(bk3d::Ptr64<bk3d::Material>)*(m_materials.size()-1);
	relocationSlots += (int)m_materials.size(); // reloc for elements in the pool
	MapMaterial::iterator iMat = m_materials.begin();
	while(iMat != m_materials.end())
	{
		sz += iMat->second->getTotalSize(relocationSlots);
		++iMat;
	}
	//
	// Meshes
	//
	sz += sizeof(bk3d::MeshPool);
	if(m_meshes.size() > 1)
		sz += sizeof(bk3d::Ptr64<bk3d::Mesh>)*(m_meshes.size()-1);
	relocationSlots += (int)m_meshes.size(); // reloc for elements in the pool
	MapMesh::iterator iM = m_meshes.begin();
	while(iM != m_meshes.end())
	{
		sz += iM->second->getTotalSize(relocationSlots);
		++iM;
	}
	//
	// Relocation table
	//
	sz += sizeof(bk3d::RelocationTable);
	sz += (relocationSlots) * sizeof(unsigned long);

	DPF(("Fileheader : %d\n", sz));
	return sz;
}
/*------------------------------------------------------------------

	BUILD the contiguous dataset

  ------------------------------------------------------------------*/
void	*CFileHeader::build(LPCSTR file, std::string &headerName)
{
	int i;
	int relocationSlots=0;
    // first mandatory action: set the pool in which we want to work
    Bk3dPool::setCurPool(m_bk3dPool, m_bk3dPool2);
#if 1//ndef USE_FILE_MAPPING // only if not using file mapping
	//
	// Reserve the size of the pool
	//
	// compute the needed size and # of relocations slots
	size_t totalSZ = getTotalSize(relocationSlots);
	debugRelocationSlots = relocationSlots;
#pragma message("TODO: to be clean, we should change the getTotalSize so that it returns 2 totalSZ...")
	if(!m_bk3dPool->setSize(totalSZ))
        return NULL;
	if(!m_bk3dPool2->setSize(totalSZ))
        return NULL;
#endif
	//
	// Allocate the contiguous memory
	//
	m_fileHeader = new TFileHeader;
	SETNAME(m_fileHeader, headerName.c_str());
	m_fileHeader->nodeByteSize = 0;
	DPF(("FileHeader %s\n", m_fileHeader->name));
	//
	// Animation Curves
	//
#if 0
	m_fileHeader->pMayaCurves = NULL;
#else
	//
	// Animation Curves
	//
	MapMayaCVPtrs CVPtrs; // used to match bk3d ptr with CMayaCurveVector ptr
	if(m_mayacurves.size()>0)
	{
		TMayaCurvePool *pMayaCurvePool = new(sizeof(bk3d::Ptr64<bk3d::MayaCurveVector>)*(m_mayacurves.size()-1)) TMayaCurvePool;
		m_fileHeader->pMayaCurves = pMayaCurvePool;
		pMayaCurvePool->n = 0;
		MapCurveVec::iterator iCV = m_mayacurves.begin();
		int n = 0;
		while(iCV != m_mayacurves.end())
		{
			CMayaCurveVector* pCV = iCV->second;
			bk3d::MayaCurveVector* pbk3dCV = pCV->build();
			// Keep track of the pointer related to the original curve so later we can easily connect with them
			CVPtrs[pCV] = pbk3dCV;
			pMayaCurvePool->p[pMayaCurvePool->n++] = pbk3dCV;
			++iCV;
		}
	}
	//
	// Quaternion Curves
	//
	MapQuatCVPtrs CVQPtrs; // used to match bk3d ptr with CMayaCurveVector ptr
	if(m_quatcurves.size()>0)
	{
		TQuatCurvePool *pQuatCurvePool = new(sizeof(bk3d::Ptr64<bk3d::QuatCurve>)*(m_quatcurves.size()-1)) TQuatCurvePool;
		m_fileHeader->pQuatCurves = pQuatCurvePool;
		pQuatCurvePool->n = 0;
		MapQuatCurve::iterator iCV = m_quatcurves.begin();
		int n = 0;
		while(iCV != m_quatcurves.end())
		{
			CQuatCurve* pCV = iCV->second;
			bk3d::QuatCurve* pbk3dCV = pCV->build();
			// Keep track of the pointer related to the original curve so later we can easily connect with them
			CVQPtrs[pCV] = pbk3dCV;
			pQuatCurvePool->p[pQuatCurvePool->n++] = pbk3dCV;
			++iCV;
		}
	}
#endif
#if 1
	//
	// Transforms
	//
	MapTransform2bk3d transform2bk3d;
	m_fileHeader->pTransforms = NULL;
    TTransformPool *pTransformPool = NULL;
	if(m_transforms.size()>0)
	{
		pTransformPool = new(sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transforms.size()-1)) TTransformPool;
		m_fileHeader->pTransforms = pTransformPool;
		// total amount of transformations will be incremented here:
		pTransformPool->nBones = 0;
		pTransformPool->pBones[0] = NULL;
        // walk through the transformations and check if specific transforms are being used
        // then allocate tables accordingly
		VecTransform::iterator iT = m_transforms.begin();
        bool containsTransfTypes = false;
        bool containsTransfSimpleTypes = false;
        int childrenListSize = 1; // item #0 is for when no children: this 0 offset will be referenced and will show value = 0 children
        int tableEffectorWeightsSize = 1; // same idea as above
        //int tableIKHandlesListsSize = 0;
		while(iT != m_transforms.end())
		{
            CBone* pBone = (*iT);
			if(pBone->AsTransf())
                containsTransfTypes = true;
			if(pBone->AsTransfSimple())
                containsTransfSimpleTypes = true;
            childrenListSize += pBone->GetNumChildren() + 1; // +1 because we will store the # of children at the beginning of each series
            if(pBone->AsIKHandle())
            {
                CIKHandle *pIKH = static_cast<CIKHandle*>(pBone->AsIKHandle() );
                tableEffectorWeightsSize += (int)pIKH->m_effectorTransforms.size() + 1; // +1 because we will add the amount of effector as the first field
                tableEffectorWeightsSize++;
            }
			++iT;
		}
        // Now allocate tables to fill-in from transforms
		pTransformPool->tableMatrixAbs = new TMatrixType[m_transforms.size()];
        memset(pTransformPool->tableMatrixAbs.p, 0, sizeof(TMatrixType)*m_transforms.size());
        pTransformPool->tableMatrixInvBindpose = new TMatrixType[m_transforms.size()];
        memset(pTransformPool->tableMatrixInvBindpose.p, 0, sizeof(TMatrixType)*m_transforms.size());
        pTransformPool->tableMatrixAbsInvBindposeMatrix = new TMatrixType[m_transforms.size()];
        memset(pTransformPool->tableMatrixAbsInvBindposeMatrix.p, 0, sizeof(TMatrixType)*m_transforms.size());
        pTransformPool->tableBoneData = new TBoneDataType[m_transforms.size()];
        memset(pTransformPool->tableBoneData.p, 0, sizeof(TBoneDataType)*m_transforms.size());
        pTransformPool->tableChildrenLists = (unsigned int*)new TUINT[childrenListSize];
        memset(pTransformPool->tableChildrenLists.p, 0, sizeof(TUINT)*childrenListSize);
        //pTransformPool->tableIKHandlesLists = (unsigned int*)new TUINT[tableIKHandlesListsSize];
        //memset(pTransformPool->tableIKHandlesLists.p, 0, sizeof(TUINT)*tableIKHandlesListsSize);
        pTransformPool->tableMayaTransformData = new TMayaTransformData[m_transforms.size()];
        memset(pTransformPool->tableMayaTransformData.p, 0, sizeof(TMayaTransformData)*m_transforms.size());
        pTransformPool->tableTransformDOF = new TTransformDOF[m_transforms.size()];
        memset(pTransformPool->tableTransformDOF.p, 0, sizeof(TTransformDOF)*m_transforms.size());
        pTransformPool->tableIKHandleData = new TIKHandleData[m_transforms.size()];
        memset(pTransformPool->tableIKHandleData.p, 0, sizeof(TIKHandleData)*m_transforms.size());
        pTransformPool->tableEffectorWeights = new TEffectorTransformAndWeight[tableEffectorWeightsSize];
        memset(pTransformPool->tableEffectorWeights.p, 0, sizeof(TEffectorTransformAndWeight)*tableEffectorWeightsSize);
		// process the root transforms, first. Then we go into children...
        pTransformPool->offsetIKHandles = 0; // used temporarily in 

        // used to increment for next batch of children IDs and Effector IDs
        int childrenOffset = 1;
        int effectorOffset = 1;
        //
		// transformpool1
		//
		iT = m_transforms.begin();
		while(iT != m_transforms.end())
		{
			(*iT)->build(pTransformPool, CVPtrs, transform2bk3d, NULL, childrenOffset, effectorOffset);
			++iT;
		}
	    //
	    // IKHandles: impacts the transform table because IKHandles are transforms
	    //
	    m_fileHeader->pIKHandles = NULL;
	    if(m_ikHandles.size() > 0)
	    {
		    DPF((" %d ikHandles\n", m_ikHandles.size()));
		    TIKHandlePool *pIKHandlePool = new
			    (sizeof(bk3d::Ptr64<bk3d::IKHandle>)*(m_ikHandles.size()-1)) 
			    TIKHandlePool;
		    pIKHandlePool->n = (int)m_ikHandles.size();
		    m_fileHeader->pIKHandles = pIKHandlePool;

		    MapIKHandle::iterator iIK = m_ikHandles.begin();
		    i = 0;
            int N = (int)m_ikHandles.size();
		    while(iIK != m_ikHandles.end())
		    {
                if(g_progressCallback) g_progressCallback("Cooking IKHandles", i, N);
                // Find back the bk3d transform created from this CTransform
                int j;
                for(j=0; j<pTransformPool->nBones; j++)
                {
                    if(!strncmp(iIK->second->GetName(), pTransformPool->pBones[j]->name, NODENAMESZ))
                    {
                        pIKHandlePool->p[i++] = pTransformPool->pBones[j].p->asIKHandle();
                        break;
                    }
                }
                // we MUST have found the IKHandle in the Bones
                assert(j<pTransformPool->nBones);
			    ++iIK;
		    }
	    }
        //
        // PhRigidBody
        //
	    m_fileHeader->pRigidBodies = NULL;
	    if(m_phRigidBodies.size() > 0)
	    {
		    DPF((" %d ikRigidBodies\n", m_phRigidBodies.size()));
		    TRigidBodyPool *pRigidBodyPool = new
			    (sizeof(bk3d::Ptr64<bk3d::RigidBody>)*(m_phRigidBodies.size()-1)) 
			    TRigidBodyPool;
		    pRigidBodyPool->n = (int)m_phRigidBodies.size();
		    m_fileHeader->pRigidBodies = pRigidBodyPool;

		    MapPhRigidBody::iterator iRB = m_phRigidBodies.begin();
		    i = 0;
            int N = (int)m_phRigidBodies.size();
		    while(iRB != m_phRigidBodies.end())
		    {
                if(g_progressCallback) g_progressCallback("Cooking RigidBodies", i, N);
                // Find back the bk3d transform created from this CTransform
                int j;
                for(j=0; j<pTransformPool->nBones; j++)
                {
                    if(!strncmp(iRB->second->GetName(), pTransformPool->pBones[j]->name, NODENAMESZ))
                    {
                        pRigidBodyPool->p[i++] = (bk3d::RigidBody*)pTransformPool->pBones[j].p;
                        break;
                    }
                }
                assert(j<pTransformPool->nBones);
			    // passing transform2bk3d so that we can setup the references to transformations
			    //pRigidBodyPool->p[i++] = iIK->second->build(pTransformPool, transform2bk3d, CVPtrs);
			    ++iRB;
		    }
	    }

        //
        // PhConstraint
        //
	    m_fileHeader->pConstraints = NULL;
	    if(m_phConstraints.size() > 0)
	    {
		    DPF((" %d ikConstraints\n", m_phConstraints.size()));
		    TConstraintPool *pConstraintPool = new
			    (sizeof(bk3d::Ptr64<bk3d::Constraint>)*(m_phConstraints.size()-1)) 
			    TConstraintPool;
		    pConstraintPool->n = (int)m_phConstraints.size();
		    m_fileHeader->pConstraints = pConstraintPool;

		    MapPhConstraint::iterator iRB = m_phConstraints.begin();
		    i = 0;
            int N = (int)m_phConstraints.size();
		    while(iRB != m_phConstraints.end())
		    {
                if(g_progressCallback) g_progressCallback("Cooking Constraints", i, N);
                // Find back the bk3d transform created from this CTransform
                int j;
                for(j=0; j<pTransformPool->nBones; j++)
                {
                    if(!strncmp(iRB->second->GetName(), pTransformPool->pBones[j]->name, NODENAMESZ))
                    {
                        pConstraintPool->p[i++] = (bk3d::Constraint*)pTransformPool->pBones[j].p;
                        break;
                    }
                }
                assert(j<pTransformPool->nBones);
			    // passing transform2bk3d so that we can setup the references to transformations
			    //pConstraintPool->p[i++] = iIK->second->build(pTransformPool, transform2bk3d, CVPtrs);
			    ++iRB;
		    }
	    }
        // second round for DOF connections
		iT = m_transforms.begin();
		while(iT != m_transforms.end())
		{
			(*iT)->setupDOFAndChildrenLinks(pTransformPool, transform2bk3d);
			++iT;
		}
	}
#endif
	//
	// Materials
	//
	m_fileHeader->pMaterials = NULL;
	if(m_materials.size() > 0)
	{
		DPF((" %d materials\n", m_materials.size()));
		TMaterialPool *pMaterialPool = new
			(sizeof(bk3d::Ptr64<bk3d::Material>)*(m_materials.size()-1)) 
			TMaterialPool;
		pMaterialPool->nMaterials = (int)m_materials.size();
		m_fileHeader->pMaterials = pMaterialPool;
        // allocate the tables
        pMaterialPool->tableMaterialData = new TMaterialData[m_materials.size()];
		MapMaterial::iterator iMat = m_materials.begin();
		for(i = 0; iMat != m_materials.end(); i++)
		{
			pMaterialPool->pMaterials[i] = iMat->second->build(pMaterialPool, i);
			++iMat;
		}
	}
	//
	// Meshes
	//
	// Allocate an array of pointers + the structure header (made of 1 pointer only)
	m_fileHeader->pMeshes = NULL;
	if(m_meshes.size() > 0)
	{
		DPF((" %d meshes\n", m_meshes.size()));
		TMeshPool *pMeshPool = new
			(sizeof(bk3d::Ptr64<bk3d::Mesh>)*(m_meshes.size()-1)) 
			TMeshPool;
		pMeshPool->n = (int)m_meshes.size();
		m_fileHeader->pMeshes = pMeshPool;

		MapMesh::iterator iM = m_meshes.begin();
		i = 0;
        int N = (int)m_meshes.size();
		while(iM != m_meshes.end())
		{
            if(g_progressCallback) g_progressCallback("Cooking Meshes", i, N);
			// passing transform2bk3d so that we can setup the references to transformations
			pMeshPool->p[i++] = iM->second->build(transform2bk3d, CVPtrs);
			++iM;
		}
	}
	//
	// Reloc-table
	//
	DPF(("Relocation table: %d pointers\n", relocationSlots));
	m_fileHeader->pRelocationTable = new TRelocationTable;
	m_fileHeader->pRelocationTable->numRelocationOffsets = relocationSlots;
    m_fileHeader->pRelocationTable->pRelocationOffsets = 
        (bk3d::RelocationTable::Offsets*)m_bk3dPool->alloc(sizeof(bk3d::RelocationTable::Offsets)*relocationSlots);
	//
	// set the basic Node info
	//
    // The size of this Header Node encapsulates everything that it owns.
    // except the vtx/idx/... data that are in m_bk3dPool2
	m_fileHeader->nodeByteSize = (unsigned int)(m_bk3dPool->getUsedSize());
	m_fileHeader->nextNode = (bk3d::Node*)m_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
    DPF(("CFileHeader sz : %fKb ( + %fKb of Vtx/idx/other tables) \n", (float)m_fileHeader->nodeByteSize/1024.0f, (float)m_bk3dPool2->getUsedSize()/1024.0f));
	return (void*)m_fileHeader;
}

/*------------------------------------------------------------------
  Sequences for changing the pointers to offsets
  ------------------------------------------------------------------*/
//bool bk3dlib::ptrToOffset(bk3d::FileHeader * p)
//{
//    bk3d::FileHeader* pHead = (bk3d::FileHeader*)p;
//	int localSlots = 0;
//	relocCnt = 0;
//	emptySlot = 0;
//	bk3d::RelocationTable::Offsets *relocSlot = 
//        pHead->pRelocationTable->pRelocationOffsets;
//	PTR2OFFSET(bk3d::Node, pHead->nextNode);
//
//	if(pHead->pMeshes) for(int i=0; i< pHead->pMeshes->n; i++)
//	{
//		CMesh::ptrToOffset(relocSlot, pHead, pHead->pMeshes->p[i]);
//		PTR2OFFSET(bk3d::Mesh, pHead->pMeshes->p[i].p);
//	}
//	PTR2OFFSET(bk3d::MeshPool, pHead->pMeshes);
//
//	if(pHead->pMayaCurves) for(int i=0; i< pHead->pMayaCurves->n; i++)
//	{
//		CMayaCurveVector::ptrToOffset(relocSlot, pHead, pHead->pMayaCurves->p[i]);
//		PTR2OFFSET(bk3d::MayaCurveVector, pHead->pMayaCurves->p[i].p);
//	}
//	PTR2OFFSET(bk3d::MayaCurvePool, pHead->pMayaCurves);
//
//	//TransformPool   *pTransforms;      // to be resolved at load time. Contains 0 to N transform offsets
//	if(pHead->pTransforms) for(int i=0; i< pHead->pTransforms->nBones; i++)
//	{
//		CTransform::ptrToOffset(relocSlot, pHead, pHead->pTransforms->p[i]);
//		PTR2OFFSET(bk3d::Transform, pHead->pTransforms->p[i].p);
//	}
//	PTR2OFFSET(bk3d::TransformPool, pHead->pTransforms);
//
//	//MaterialPool
//	if(pHead->pMaterials) for(int i=0; i< pHead->pMaterials->n; i++)
//	{
//		CMaterial::ptrToOffset(relocSlot, pHead, pHead->pMaterials->p[i]);
//		PTR2OFFSET(bk3d::Material, pHead->pMaterials->p[i].p);
//	}
//	PTR2OFFSET(bk3d::MaterialPool, pHead->pMaterials);
//
//	// relocation table
//	PTR2OFFSET(bk3d::Node, pHead->pRelocationTable->nextNode);
//    pHead->pRelocationTable->pRelocationOffsets = (bk3d::RelocationTable::Offsets*)((char*)pHead->pRelocationTable->pRelocationOffsets - (char*)pHead);\
//	pHead->pRelocationTable = (bk3d::RelocationTable*)((char*)pHead->pRelocationTable - (char*)pHead);\
//
//DPF(("debugRelocationSlots=%d ; relocCnt(%d) + EmptySlot(%d) = %d\n", debugRelocationSlots, relocCnt, emptySlot, relocCnt+emptySlot));
//	if((debugRelocationSlots > 0) && (debugRelocationSlots < relocCnt))
//	{
//		assert("debugRelocationSlots != relocCnt");
//		throw("debugRelocationSlots != relocCnt");
//        return false;
//	}
//	DPF(("CFileHeader slots : %d\n", localSlots));
//    return true;
//}


