#include "builder.h"
/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
size_t	CPhRigidBody::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();

    size_t sz = sizeof(bk3d::RigidBody);
    DPF(("RigidBody : %d\n", sz));
	return sz;
}

#define COPYVEC(s, v, a, n) { memcpy(s->v, m_##v##a, sizeof(m_##v##a[0])*n );}
bk3d::Bone*   CPhRigidBody::build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();

    // first time initialization: if 0, it means we must set it to the first body
    //if(transformPool->offsetRigidBodies == 0)
    //    transformPool->offsetRigidBodies = transformPool->nBones;
    //
	// create and copy data
	//
    m_bk3dRigidBody = new TRigidBody;
    bk3d::Bone	* ttr = CBone::build(transformPool, CVPtrs, transform2bk3d, m_bk3dRigidBody, childrenOffset, effectorOffset);

    m_bk3dRigidBody->collision_group    = m_grp;
    m_bk3dRigidBody->shape_type         = (unsigned char)m_type;
    m_bk3dRigidBody->mode               = m_mode;
    m_bk3dRigidBody->collision_group_mask = m_collision_group_mask;
    m_bk3dRigidBody->shape_size[0]      = m_shape_size[0];
    m_bk3dRigidBody->shape_size[1]      = m_shape_size[1];
    m_bk3dRigidBody->shape_size[2]      = m_shape_size[2];
    m_bk3dRigidBody->mass               = m_mass;
    m_bk3dRigidBody->linear_dampening   = m_linear_dampening;
    m_bk3dRigidBody->angular_dampening  = m_angular_dampening;
    m_bk3dRigidBody->restitution        = m_restitution;
    m_bk3dRigidBody->friction           = m_friction;

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_bk3dRigidBody->nodeByteSize = (unsigned int)nodeByteSize;
	m_bk3dRigidBody->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

	DPF(("CPhRigidBody sz : %d\n", nodeByteSize));
	return m_bk3dRigidBody;
}

bool CPhRigidBody::setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d)
{
    return CBone::setupDOFAndChildrenLinks(transformPool, transform2bk3d);
}
