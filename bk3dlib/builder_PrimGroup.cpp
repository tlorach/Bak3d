#include "builder.h"
/*------------------------------------------------------------------
	Prim Group
  ------------------------------------------------------------------*/

size_t	CPrimGroup::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::PrimGroup);
	const std::vector<unsigned int> &array = idxgroup.elements;
	// Only count the bytes of the idx buffer when the buffer belongs to this primitive group
	for(unsigned int i=0; i<m_idxBuffers.size(); i++)
	{
		if(m_idxBuffers[i]->GetOwner() == this)
		{
			size_t idxsz;
			if(m_idxBuffers[i]->m_UIVals.empty())
				idxsz = m_idxBuffers[i]->m_FVals.size();
			else
				idxsz = m_idxBuffers[i]->m_UIVals.size();

			if(m_idxBuffers[i]->m_type == bk3dlib::UINT32)
				idxsz *= sizeof(unsigned int);
			else if(m_idxBuffers[i]->m_type == bk3dlib::UINT16)
				idxsz *= sizeof(unsigned short);
			sz += idxsz;
			DPF(("PrimGroup owns the index buffer\n", sz));
		}
	}
	//
	// Transform references
	//
	if(m_transformRefs.size())
		sz += sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transformRefs.size() - 1);
	sz += sizeof(bk3d::TransformRefs);
	relocationSlots += (int)m_transformRefs.size();

  DPF(("PrimGroup : %d\n", sz));
	return sz;
}
TPrimGroup	*CPrimGroup::buildBegin(const MapTransform2bk3d &transform2bk3d)
{
	DPF(("--------------\n"));
	m_sz = 0;
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//
	// Contiguous mem allocations
	//
	primgroup = new TPrimGroup;
	SETNAME(primgroup, m_name.c_str());
	DPF(("Primitive Group %s\n", m_name.c_str()));

    primgroup->visible = 1.0f;
	primgroup->indexPerVertex = m_idxBuffers.size();
	primgroup->indexOffset = m_offsetElement;
    //primgroup->indexCount = (m_numElements == 0)&&(m_idxBuffers.size() > 0) ? (unsigned int)m_idxBuffers[0]->m_UIVals.size() : m_numElements;
    if((m_numElements == 0)&&(m_idxBuffers.size() > 0))
        primgroup->indexCount = (unsigned int)m_idxBuffers[0]->m_UIVals.size();
    else
        primgroup->indexCount = m_numElements;
    primgroup->primitiveCount = numPrimitivesDX9(primgroup->indexCount, m_topo);
	DPF(("index count = %d\n", primgroup->indexCount));
	DPF(("Primitive count = %d\n", primgroup->primitiveCount));
    std::vector<unsigned int>::const_iterator it;
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Just walk through the proper range : offset/size")
	m_sz = 0;
    if(m_idxBuffers.size() > 0)
    {
	    for(unsigned int i=0; i<m_idxBuffers.size(); i++)
	    {
		    it = std::max_element(m_idxBuffers[i]->m_UIVals.begin(), m_idxBuffers[i]->m_UIVals.end());
        if(it != m_idxBuffers[i]->m_UIVals.end() )
  		    primgroup->maxIndex = *it;
		    it = std::min_element(m_idxBuffers[i]->m_UIVals.begin(), m_idxBuffers[i]->m_UIVals.end());
        if(it != m_idxBuffers[i]->m_UIVals.end() )
		      primgroup->minIndex = *it;
		    m_sz += m_idxBuffers[i]->GetNumComps() * m_idxBuffers[i]->GetNumItems();
	    }
    } else {
        primgroup->maxIndex = m_offsetElement + m_numElements - 1;
        primgroup->minIndex = m_offsetElement;
    }
	primgroup->indexArrayByteOffset = m_offsetElement;
    //
    // TODO : Add the mode ELT_USHORT when possible
	// only use the idx buffer 0... the others *must* be of same type
    //
    if(m_idxBuffers.size() > 0)
    {
        if(m_idxBuffers[0]->m_type == bk3dlib::UINT32)
        {
            DPF(("Index Fmt = UInt 32\n"));
            primgroup->indexFormatDXGI = DXGI_FORMAT_R32_UINT;
            primgroup->indexFormatDX9 = D3DFMT_INDEX32;
            primgroup->indexFormatGL = GL_UNSIGNED_INT;
            primgroup->indexArrayByteOffset *= sizeof(int);
            m_sz *= sizeof(int);
        }
        else if(m_idxBuffers[0]->m_type == bk3dlib::UINT16)
        {
            DPF(("Index Fmt = UInt 16\n"));
            primgroup->indexFormatDXGI = DXGI_FORMAT_R16_UINT;
            primgroup->indexFormatDX9 = D3DFMT_INDEX16;
            primgroup->indexFormatGL = GL_UNSIGNED_SHORT;
            primgroup->indexArrayByteOffset *= sizeof(short);
            m_sz *= sizeof(short);
        }
        primgroup->indexArrayByteSize = (unsigned int)m_sz;
        primgroup->primRestartIndex = m_idxBuffers[0]->getPrimitiveRestartIndex();
    } else {
        primgroup->indexArrayByteSize = 0;
    }
    primgroup->topologyDX11 = TopologyDXGI(m_topo);
    primgroup->topologyDX9 = TopologyDX9(m_topo);
    primgroup->topologyGL = TopologyOpenGL(m_topo);

    nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
    primgroup->nodeByteSize = (unsigned int)nodeByteSize;
    primgroup->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
    //
    // Material : we must find it in m_materials, unless we have a bug
    //
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Add material references")
    primgroup->pMaterial = NULL;
    if(m_pMaterial)
    {
        assert(m_pMaterial->pMat);
        primgroup->pMaterial = m_pMaterial->pMat;
    }
    //
    // Bounding volumes
    //
    primgroup->aabbox = aabbox;
    primgroup->bsphere = bsphere;
    //
    // Transform references
    //
    primgroup->pTransforms = new(sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transformRefs.size() - 1)) TTransformRefs;
    primgroup->pTransforms->n = m_transformRefs.size();
    // Setup the references to the correct transforms
    DPF((" Num Transforms = %d\n", m_transformRefs.size() ));
    for(unsigned int j=0; j<m_transformRefs.size(); j++)
    {
        MapTransform2bk3d::const_iterator iT = transform2bk3d.find(m_transformRefs[j]);
        assert(iT->second);
        // Now we can set the reference to the transform
        primgroup->pTransforms->p[j] = iT->second;
    }
  DPF(("CPrimGroup sz : %d\n", nodeByteSize));
    return primgroup;
}
//
// Second part : allocate the index table.
// so we can allocate all the index tables in a row so they are contiguous in memory !
// NOTE: doing this in a second part is not anymore so interesting now I added a second pool
// for indices and other vertices...
//
TPrimGroup	*CPrimGroup::buildEnd()
{
	//
	// allocate room for elements
	// copy elements
	//
	if(!primgroup)
		return NULL;
	primgroup->pIndexBufferData = NULL;
	//
	// If the buffer is owned by another prim group, just point to it
	// again... use idxbuffer #0 only
	//
    if(m_sz > 0)
    {
        if(m_idxBuffers[0]->GetOwner() == this)//if(!primgroup->pIndexBufferData) ?
        {
            //
            // Allocate on contiguous memory
            //
            primgroup->pIndexBufferData = Bk3dPool::cur_bk3dPool2->alloc(m_sz); // m_sz contains the total needed for all the index buffers
            //
            // Keep track of this pointer for later primgroups that would reference it
            //
            m_idxBuffers[0]->SetBk3dPointer(primgroup);
            primgroup->pOwnerOfIB = primgroup; // tell bk3d that this guy is the main owner
            //
            // DUMB copy data... TODO: make this faster
            //
            for(unsigned int j=0; j< m_idxBuffers.size(); j++)
            {
                for(unsigned int i=0; i< m_idxBuffers[j]->m_UIVals.size(); i++)
                {
                    if(m_idxBuffers[j]->m_type == bk3dlib::UINT32) // here we could make a memcpy...
                        ((unsigned int*)primgroup->pIndexBufferData)[i*m_idxBuffers.size() + j] = m_idxBuffers[j]->m_UIVals[i];
                    else if(m_idxBuffers[j]->m_type == bk3dlib::UINT16)
                        ((unsigned short*)primgroup->pIndexBufferData)[i*m_idxBuffers.size() + j] = m_idxBuffers[j]->m_UIVals[i];
                }
            }
        } else {
            primgroup->pOwnerOfIB = m_idxBuffers[0]->GetBk3dPointer();
            primgroup->pIndexBufferData = primgroup->pOwnerOfIB->pIndexBufferData; // Non Null if already allocated
        }
    } else {
        primgroup->pOwnerOfIB = NULL;
    }
	return primgroup;
}
//void	CPrimGroup::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::PrimGroup *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//    PTR2OFFSET(bk3d::Material, p->pMaterial);
//    PTR2OFFSET(void, p->pIndexBufferData);       // this pointer will have to be resolved after load operation
//	PTR2OFFSET(bk3d::PrimGroup, p->pOwnerOfIB);
//	if(p->pTransforms) for(int i=0; i< p->pTransforms->nBones; i++)
//	{
//		//This transformation will be processed by CFileHeader
//		//no need... CTransform::ptrToOffset(relocSlot, pHead, p->pTransforms->p[i]);
//		PTR2OFFSET(bk3d::Transform, p->pTransforms->p[i].p);
//	}
//  PTR2OFFSET(bk3d::TransformRefs, p->pTransforms);
//	DPF(("CPrimgoup slots : %d\n", localSlots));
//}
