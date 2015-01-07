#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "builder.h"
#include "poolAllocator.h"

int debugRelocationSlots = 0;
int relocCnt = 0;
int emptySlot = 0;


//===============================================================================
/*------------------------------------------------------------------
  Sequences for changing the pointers to offsets
  ------------------------------------------------------------------*/
//
// TODO: add the offset value with the offset of the ptr
//
//#define PTR2OFFSET(type, ptr) if(ptr){ relocCnt++;\
//	ptr = (type*)((char*)ptr - (char*)m_fileHeader);\
//	relocSlot->offset = (unsigned long)ptr;\
//	relocSlot->ptrOffset = (unsigned long)(((char*)(&(ptr)) - (char*)m_fileHeader));\
//    *relocSlot++;\
//	localSlots++;\
//} else { emptySlot++; }

// NEW version allows to handle 2 pools
#define PTR2OFFSET(type, ptr) if(ptr){\
    relocCnt++;\
    char* pBase = NULL;\
    size_t offset = 0;\
    if(m_bk3dPool->isValidPtr(ptr))\
        pBase = (char*)m_bk3dPool->getPool();\
    else if(m_bk3dPool2->isValidPtr(ptr))\
    {\
        pBase = (char*)m_bk3dPool2->getPool();\
        offset = m_bk3dPool->getUsedSize();\
    }\
	relocSlot->offset = (unsigned long)(((char*)ptr - (char*)pBase)+offset);\
	if(!noPtrConv) ptr = (type*)(((char*)ptr - (char*)pBase)+offset);\
    pBase = NULL;\
    offset = 0;\
    if(m_bk3dPool->isValidPtr(&(ptr)))\
        pBase = (char*)m_bk3dPool->getPool();\
    else if(m_bk3dPool2->isValidPtr(&(ptr)))\
    {\
        pBase = (char*)m_bk3dPool2->getPool();\
        offset = m_bk3dPool->getUsedSize();\
    }\
	relocSlot->ptrOffset = (unsigned long)(((char*)(&(ptr)) - (char*)pBase)+offset);\
    *relocSlot++;\
	localSlots++;\
} else { emptySlot++; }

//#define PTR2OFFSET(type, ptr) relocCnt++; if(!ptr) emptySlot++;

//===============================================================================

void	CFileHeader::CPrimGroup_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::PrimGroup *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
    PTR2OFFSET(bk3d::Material, p->pMaterial);
    PTR2OFFSET(void, p->pIndexBufferData);       // this pointer will have to be resolved after load operation
	PTR2OFFSET(bk3d::PrimGroup, p->pOwnerOfIB);
	if(p->pTransforms) for(int i=0; i< p->pTransforms->n; i++)
	{
		//This transformation will be processed by CFileHeader
		//no need... CTransform_ptrToOffset(relocSlot,  p->pTransforms->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Transform, p->pTransforms->p[i].p);
	}
  PTR2OFFSET(bk3d::TransformRefs, p->pTransforms);

    p->userPtr = NULL;

	DPF(("CPrimgoup slots : %d\n", localSlots));
}

void	CFileHeader::CSlot_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Slot *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
	if(p->pAttributes) for(int i=0; i< p->pAttributes->n; i++)
	{
		// already done in Mesh...CSlot_ptrToOffset(relocSlot, p->pSlots->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Attribute, p->pAttributes->p[i].p);
	}
    PTR2OFFSET(bk3d::AttributePool, p->pAttributes);
    //PTR2OFFSET(void, p->pVtxBufferData);
if(p->pVtxBufferData){
relocCnt++;
char* pBase = NULL;
size_t offset = 0;
if(m_bk3dPool->isValidPtr(p->pVtxBufferData))
    pBase = (char*)m_bk3dPool->getPool();
else if(m_bk3dPool2->isValidPtr(p->pVtxBufferData))
{
    pBase = (char*)m_bk3dPool2->getPool();
    offset = m_bk3dPool->getUsedSize();
}
relocSlot->offset = (unsigned long)(((char*)p->pVtxBufferData - (char*)pBase)+offset);
if(!noPtrConv) 
    p->pVtxBufferData = (void*)(((char*)p->pVtxBufferData - (char*)pBase)+offset);
pBase = NULL;
offset = 0;
if(m_bk3dPool->isValidPtr(&(p->pVtxBufferData)))
    pBase = (char*)m_bk3dPool->getPool();
else if(m_bk3dPool2->isValidPtr(&(p->pVtxBufferData)))
{
    pBase = (char*)m_bk3dPool2->getPool();
    offset = m_bk3dPool->getUsedSize();
}
relocSlot->ptrOffset = (unsigned long)(((char*)(&(p->pVtxBufferData)) - (char*)pBase)+offset);
*relocSlot++;
localSlots++;
} else { emptySlot++; }

    p->userData = 0;
    p->userPtr = NULL;

	DPF(("CSlot slots : %d\n", localSlots));
}

void	CFileHeader::CTransform_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Bone *p, bool noPtrConv)
{
	int localSlots = 0;
    // relocations for: 
    //      nextnode; 
    //      + parentPool 
    //      + pBoneData 
    //      + pFloatArrays
    //      + pPArent
    //      + pChildren
    //      + pDOF
    //      + pIKHandles
    //      + pMatrixAbs
    //      + pMatrixAbsInvBindPoseMatrix
    //      + pMatrixInvBindPoseMatrix
	PTR2OFFSET(bk3d::Node, p->nextNode);
    PTR2OFFSET(bk3d::TransformPool, p->parentPool.p);
	PTR2OFFSET(bk3d::BoneDataType, p->pBoneData);
	PTR2OFFSET(bk3d::Transform, p->pParent);
	PTR2OFFSET(bk3d::TransformDOF, p->pDOF);
	PTR2OFFSET(bk3d::IKHandlePool, p->pIKHandles);
    PTR2OFFSET(bk3d::MatrixType, p->pMatrixAbs);
    PTR2OFFSET(bk3d::MatrixType, p->pMatrixAbsInvBindposeMatrix);
    PTR2OFFSET(bk3d::MatrixType, p->pMatrixInvBindpose);
	if(p->pChildren)
	{
		for(int i=0; i<p->pChildren->n; i++)
		{
			PTR2OFFSET(bk3d::Transform, p->pChildren->p[i].p);
		}
		PTR2OFFSET(bk3d::TransformPool2, p->pChildren);
	}
	if(p->pFloatArrays) for(int i=0; i< p->pFloatArrays->n; i++)
	{
		PTR2OFFSET(bk3d::FloatArray, p->pFloatArrays->p[i].p);
		PTR2OFFSET(float, p->pFloatArrays->p[i].pfTarget);
	}
    PTR2OFFSET(bk3d::FloatArrayPool, p->pFloatArrays);

	DPF(("CTransform slots : %d\n", localSlots));
}
void	CFileHeader::CMaterial_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Material *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
    PTR2OFFSET(char, p->shaderName);
    PTR2OFFSET(char, p->techniqueName);

	PTR2OFFSET(bk3d::MaterialPool, p->parentPool.p);
    PTR2OFFSET(bk3d::MaterialData, p->pMaterialData);

	PTR2OFFSET(char, p->diffuseTexture.name);
	PTR2OFFSET(char, p->diffuseTexture.filename);
	PTR2OFFSET(char, p->specExpTexture.name);
	PTR2OFFSET(char, p->specExpTexture.filename);
	PTR2OFFSET(char, p->ambientTexture.name);
	PTR2OFFSET(char, p->ambientTexture.filename);
	PTR2OFFSET(char, p->reflectivityTexture.name);
	PTR2OFFSET(char, p->reflectivityTexture.filename);
	PTR2OFFSET(char, p->transparencyTexture.name);
	PTR2OFFSET(char, p->transparencyTexture.filename);
	PTR2OFFSET(char, p->translucencyTexture.name);
	PTR2OFFSET(char, p->translucencyTexture.filename);
	PTR2OFFSET(char, p->specularTexture.name);
	PTR2OFFSET(char, p->specularTexture.filename);

    p->userData[0] = p->userData[1] = 0;
}

void	CFileHeader::CAttribute_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Attribute *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
    PTR2OFFSET(void, p->pAttributeBufferData);

    p->userData[0] = p->userData[1] = 0;

	DPF(("CAttribute slots : %d\n", localSlots));
}
void	CFileHeader::CMayaCurve_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::MayaCurve *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
	DPF(("CMayaCurve slots : %d\n", localSlots));
}
void	CFileHeader::CMayaCurveVector_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::MayaCurveVector *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
	PTR2OFFSET(bk3d::FloatArray, p->pFloatArray);
	for(int i=0; i<p->nCurves; i++)
	{
		CMayaCurve_ptrToOffset(relocSlot, p->pCurve[i], noPtrConv);
		PTR2OFFSET(bk3d::MayaCurve, p->pCurve[i].p);
	}
	DPF(("CMayaCurveVector slots : %d\n", localSlots));
}
void	CFileHeader::CQuatCurve_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::QuatCurve *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
	PTR2OFFSET(bk3d::FloatArray, p->pFloatArray);
	DPF(("CQuatCurve slots : %d\n", localSlots));
}

void	CFileHeader::CMesh_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Mesh *p, bool noPtrConv)
{
	int localSlots = 0;
	PTR2OFFSET(bk3d::Node, p->nextNode);
	if(p->pSlots) for(int i=0; i< p->pSlots->n; i++)
	{
		CSlot_ptrToOffset(relocSlot, p->pSlots->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Slot, p->pSlots->p[i].p);
	}
    PTR2OFFSET(bk3d::SlotPool, p->pSlots);
	if(p->pPrimGroups) for(int i=0; i< p->pPrimGroups->n; i++)
	{
		CPrimGroup_ptrToOffset(relocSlot, p->pPrimGroups->p[i], noPtrConv);
		PTR2OFFSET(bk3d::PrimGroup, p->pPrimGroups->p[i].p);
	}
    PTR2OFFSET(bk3d::PrimGroupPool, p->pPrimGroups);
	if(p->pAttributes) for(int i=0; i< p->pAttributes->n; i++)
	{
		CAttribute_ptrToOffset(relocSlot, p->pAttributes->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Attribute, p->pAttributes->p[i].p);
	}
	PTR2OFFSET(bk3d::AttributePool, p->pAttributes);
	if(p->pBSAttributes) for(int i=0; i< p->pBSAttributes->n; i++)
	{
		CAttribute_ptrToOffset(relocSlot, p->pBSAttributes->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Attribute, p->pBSAttributes->p[i].p);
	}
	PTR2OFFSET(bk3d::AttributePool, p->pBSAttributes);
	if(p->pBSSlots) for(int i=0; i< p->pBSSlots->n; i++)
	{
		CSlot_ptrToOffset(relocSlot, p->pBSSlots->p[i].p, noPtrConv);
		PTR2OFFSET(bk3d::Slot, p->pBSSlots->p[i].p);
	}
    PTR2OFFSET(bk3d::SlotPool, p->pBSSlots);
	if(p->pTransforms) for(int i=0; i< p->pTransforms->n; i++)
	{
		//This transformation will be processed by CFileHeader
		//no need... CTransform_ptrToOffset(relocSlot, p->pTransforms->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Transform, p->pTransforms->p[i].p);
	}
    PTR2OFFSET(bk3d::TransformRefs, p->pTransforms);

	if(p->pFloatArrays) for(int i=0; i< p->pFloatArrays->n; i++)
	{
		PTR2OFFSET(bk3d::FloatArray, p->pFloatArrays->p[i].p);
		PTR2OFFSET(float, p->pFloatArrays->p[i].pfTarget);
	}
    PTR2OFFSET(bk3d::FloatArrayPool, p->pFloatArrays);
    PTR2OFFSET(bk3d::FloatArray, p->pBSWeights);

    p->userPtr = NULL;

	DPF(("CMesh slots : %d\n", localSlots));
}

void CFileHeader::CIKHandle_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::IKHandle *p, bool noPtrConv)
{
    int localSlots = 0;
    PTR2OFFSET(bk3d::Node, p->nextNode);

    if(p->pEffectorTransforms) for(int i=0; i< p->pEffectorTransforms->n; i++)
    {
        //This transformation will be processed by CFileHeader
        //no need... CTransform_ptrToOffset(relocSlot, p->pTransforms->p[i], noPtrConv);
        PTR2OFFSET(bk3d::Transform, p->pEffectorTransforms->p[i].p);
    }
    PTR2OFFSET(bk3d::TransformPool2, p->pEffectorTransforms);

    PTR2OFFSET(bk3d::IKHandleData, p->pIKHandleData);

    PTR2OFFSET(bk3d::FloatPool, p->pEffectorWeights);

    if(p->pFloatArrays) for(int i=0; i< p->pFloatArrays->n; i++)
    {
        PTR2OFFSET(bk3d::FloatArray, p->pFloatArrays->p[i].p);
        PTR2OFFSET(float, p->pFloatArrays->p[i].pfTarget);
    }
    PTR2OFFSET(bk3d::FloatArrayPool, p->pFloatArrays);
    DPF(("CIKHandle slots : %d\n", localSlots));
}

void CFileHeader::CPhRigidBody_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::RigidBody *p, bool noPtrConv)
{
    int localSlots = 0;
    // nextNode is already modified via the Bone table (a RigidBody derives from a Bone)
    //PTR2OFFSET(bk3d::Node, p->nextNode);

    DPF(("CPhRigidBody slots : %d\n", localSlots));
}

void CFileHeader::CPhConstraint_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Constraint *p, bool noPtrConv)
{
    int localSlots = 0;
    // nextNode is already modified via the Bone table (a RigidBody derives from a Bone)
    //PTR2OFFSET(bk3d::Node, p->nextNode);
    PTR2OFFSET(bk3d::Bone, p->pRigidBody1);
    PTR2OFFSET(bk3d::Bone, p->pRigidBody2);

    DPF(("CPhConstraint slots : %d\n", localSlots));
}

/*------------------------------------------------------------------
  method available in the API
  ------------------------------------------------------------------*/
bool CFileHeader::PtrToOffset()
{
    if((m_fileHeader == NULL) || (m_bk3dPool == NULL) || (m_bk3dPool2 == NULL))
        return false;
    return ptrToOffset(false);
}
/*------------------------------------------------------------------
  Sequences for changing the pointers to offsets
  ------------------------------------------------------------------*/
bool CFileHeader::ptrToOffset(bool noPtrConv)
{
	int localSlots = 0;
	relocCnt = 0;
	emptySlot = 0;
	bk3d::RelocationTable::Offsets *relocSlot = 
        m_fileHeader->pRelocationTable->pRelocationOffsets;
	PTR2OFFSET(bk3d::Node, m_fileHeader->nextNode);

	if(m_fileHeader->pRigidBodies) for(int i=0; i< m_fileHeader->pRigidBodies->n; i++)
	{
		CPhRigidBody_ptrToOffset(relocSlot, m_fileHeader->pRigidBodies->p[i], noPtrConv);
		PTR2OFFSET(bk3d::RigidBody, m_fileHeader->pRigidBodies->p[i].p);
	}
	PTR2OFFSET(bk3d::RigidBodyPool, m_fileHeader->pRigidBodies);

	if(m_fileHeader->pConstraints) for(int i=0; i< m_fileHeader->pConstraints->n; i++)
	{
		CPhConstraint_ptrToOffset(relocSlot, m_fileHeader->pConstraints->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Constraint, m_fileHeader->pConstraints->p[i].p);
	}
	PTR2OFFSET(bk3d::ConstraintPool, m_fileHeader->pConstraints);

	if(m_fileHeader->pIKHandles) for(int i=0; i< m_fileHeader->pIKHandles->n; i++)
	{
		CIKHandle_ptrToOffset(relocSlot, m_fileHeader->pIKHandles->p[i], noPtrConv);
		PTR2OFFSET(bk3d::IKHandle, m_fileHeader->pIKHandles->p[i].p);
	}
	PTR2OFFSET(bk3d::IKHandlePool, m_fileHeader->pIKHandles);

	if(m_fileHeader->pMeshes) for(int i=0; i< m_fileHeader->pMeshes->n; i++)
	{
		CMesh_ptrToOffset(relocSlot, m_fileHeader->pMeshes->p[i], noPtrConv);
		PTR2OFFSET(bk3d::Mesh, m_fileHeader->pMeshes->p[i].p);
	}
	PTR2OFFSET(bk3d::MeshPool, m_fileHeader->pMeshes);

	if(m_fileHeader->pMayaCurves) for(int i=0; i< m_fileHeader->pMayaCurves->n; i++)
	{
		CMayaCurveVector_ptrToOffset(relocSlot, m_fileHeader->pMayaCurves->p[i], noPtrConv);
		PTR2OFFSET(bk3d::MayaCurveVector, m_fileHeader->pMayaCurves->p[i].p);
	}
	PTR2OFFSET(bk3d::MayaCurvePool, m_fileHeader->pMayaCurves);

	if(m_fileHeader->pQuatCurves) for(int i=0; i< m_fileHeader->pQuatCurves->n; i++)
	{
		CQuatCurve_ptrToOffset(relocSlot, m_fileHeader->pQuatCurves->p[i], noPtrConv);
		PTR2OFFSET(bk3d::QuatCurve, m_fileHeader->pQuatCurves->p[i].p);
	}
	PTR2OFFSET(bk3d::QuatCurvePool, m_fileHeader->pQuatCurves);

	//TransformPool   *pTransforms;      // to be resolved at load time. Contains 0 to N transform offsets
	if(m_fileHeader->pTransforms)
    {
        for(int i=0; i< m_fileHeader->pTransforms->nBones; i++)
	    {
		    CTransform_ptrToOffset(relocSlot, m_fileHeader->pTransforms->pBones[i], noPtrConv);
		    PTR2OFFSET(bk3d::Transform, m_fileHeader->pTransforms->pBones[i].p);
	    }
	    PTR2OFFSET(bk3d::MatrixType, m_fileHeader->pTransforms->tableMatrixAbs.p);
	    PTR2OFFSET(bk3d::MatrixType, m_fileHeader->pTransforms->tableMatrixInvBindpose.p);
	    PTR2OFFSET(bk3d::MatrixType, m_fileHeader->pTransforms->tableMatrixAbsInvBindposeMatrix.p);
	    PTR2OFFSET(bk3d::BoneDataType, m_fileHeader->pTransforms->tableBoneData.p);
	    PTR2OFFSET(unsigned int, m_fileHeader->pTransforms->tableChildrenLists.p);
	    //PTR2OFFSET(unsigned int, m_fileHeader->pTransforms->tableIKHandlesLists.p);
	    PTR2OFFSET(bk3d::MayaTransformData, m_fileHeader->pTransforms->tableMayaTransformData.p);
	    PTR2OFFSET(bk3d::TransformDOF, m_fileHeader->pTransforms->tableTransformDOF.p);
	    PTR2OFFSET(bk3d::IKHandleData, m_fileHeader->pTransforms->tableIKHandleData.p);
	    PTR2OFFSET(bk3d::EffectorTransformAndWeight, m_fileHeader->pTransforms->tableEffectorWeights.p);
    }
	PTR2OFFSET(bk3d::TransformPool, m_fileHeader->pTransforms);

	//MaterialPool
	if(m_fileHeader->pMaterials)
    {
	    PTR2OFFSET(bk3d::MaterialData, m_fileHeader->pMaterials->tableMaterialData.p);
        for(int i=0; i< m_fileHeader->pMaterials->nMaterials; i++)
	    {
		    CMaterial_ptrToOffset(relocSlot, m_fileHeader->pMaterials->pMaterials[i], noPtrConv);
		    PTR2OFFSET(bk3d::Material, m_fileHeader->pMaterials->pMaterials[i].p);
	    }
    }
	PTR2OFFSET(bk3d::MaterialPool, m_fileHeader->pMaterials);

	// relocation table
	PTR2OFFSET(bk3d::Node, m_fileHeader->pRelocationTable->nextNode);
    if(!noPtrConv)
    {
        m_fileHeader->pRelocationTable->pRelocationOffsets = (bk3d::RelocationTable::Offsets*)((char*)m_fileHeader->pRelocationTable->pRelocationOffsets - (char*)m_fileHeader);\
	    m_fileHeader->pRelocationTable = (bk3d::RelocationTable*)((char*)m_fileHeader->pRelocationTable - (char*)m_fileHeader);\
    }
DPF(("debugRelocationSlots=%d ; relocCnt(%d) + EmptySlot(%d) = %d\n", debugRelocationSlots, relocCnt, emptySlot, relocCnt+emptySlot));
	if((debugRelocationSlots > 0) && (debugRelocationSlots < relocCnt))
	{
		assert("debugRelocationSlots != relocCnt");
		throw("debugRelocationSlots != relocCnt");
        return false;
	}
	DPF(("CFileHeader slots : %d\n", localSlots));
    return true;
}

