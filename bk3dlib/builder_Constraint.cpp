#include "builder.h"
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
size_t	CPhConstraint::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::Constraint);
	DPF(("Constraint : %d\n", sz));
	return sz;
}

#define COPYVEC(s, v, a, n) { memcpy(s->v, m_##v##a, sizeof(m_##v##a[0])*n );}
bk3d::Bone*   CPhConstraint::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();

    // first time initialization: if 0, it means we must set it to the first constraint
    //if(transformPool->offsetConstraints == 0)
    //    transformPool->offsetConstraints = transformPool->nBones;
    //
	// create and copy data
	//
    m_bk3dConstraint = new TConstraint;
    bk3d::Bone	* ttr = CBone::build(transformPool, CVPtrs, transform2bk3d, m_bk3dConstraint, childrenOffset, effectorOffset);

    m_bk3dConstraint->pRigidBody1 = NULL; // filled in antoher pass later
    m_bk3dConstraint->pRigidBody2 = NULL;
    memcpy(m_bk3dConstraint->translation_limit_min, m_translation_limit_min, sizeof(float)*3);
    memcpy(m_bk3dConstraint->translation_limit_max, m_translation_limit_max, sizeof(float)*3);
    memcpy(m_bk3dConstraint->rotation_limit_min, m_rotation_limit_min, sizeof(float)*3);
    memcpy(m_bk3dConstraint->rotation_limit_max, m_rotation_limit_max, sizeof(float)*3);
    memcpy(m_bk3dConstraint->spring_constant_translation, m_spring_constant_translation, sizeof(float)*3);
    memcpy(m_bk3dConstraint->spring_constant_rotation, m_spring_constant_rotation, sizeof(float)*3);

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_bk3dConstraint->nodeByteSize = (unsigned int)nodeByteSize;
	m_bk3dConstraint->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

	DPF(("CPhConstraint sz : %d\n", nodeByteSize));
	return m_bk3dConstraint;
}

bool CPhConstraint::setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d)
{
    bool bRes = CBone::setupDOFAndChildrenLinks(transformPool, transform2bk3d);
    if(m_pRigidBody1)
    {
	    m_bk3dConstraint->pRigidBody1 = m_pRigidBody1->m_bk3dRigidBody;
    }
    if(m_pRigidBody1)
    {
	    m_bk3dConstraint->pRigidBody2 = m_pRigidBody2->m_bk3dRigidBody;
    }
    return bRes;
}
