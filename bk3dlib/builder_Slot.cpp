#include "builder.h"
/*------------------------------------------------------------------

	Slot

  ------------------------------------------------------------------*/
CSlot::CSlot(CMesh* parent, 
			 std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair, 
			 bk3d::AttributePool *attributePool, LPCSTR name) :
	m_parent(parent),
	m_attrPair(pair),
	m_attributePool(attributePool),
	m_slot(NULL),
	m_sz(0),
	m_stride(0),
    m_name(name ? name:"SLOT")
{
	m_numAttr = 0;
	MMapBuffer::iterator i = pair.first;
	while(i != pair.second)
	{
		m_numAttr++;
		++i;
	}
}
/*------------------------------------------------------------------

	Get the total size

  ------------------------------------------------------------------*/
size_t	CSlot::getTotalSize(int &relocationSlots, bool doPruning)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::Slot);

	//
	// size of attribute pool
	//
	sz += sizeof(bk3d::AttributePool) + sizeof(bk3d::Attribute*)*(m_numAttr-1);
	relocationSlots += (int)m_numAttr;
	// There is no need to add the attributes : already available in the Mesh

	//
	// compute size of vertices
	//
	int vtxsz = 0;
	MMapBuffer::iterator iB = m_attrPair.first;
	while(iB != m_attrPair.second)
	{
		//
		// Size of these data.
		// Note: special case when no array but we still need to fill the array
		//
		int sz = iB->second->GetNumItems() * iB->second->GetItemSizeBytes();
		vtxsz += sz;
		//
		// compute stride
		//
		m_stride += iB->second->GetItemSizeBytes();
		++iB;
	} 
	DPF(("Slot : %d\n", sz + vtxsz));
	return sz + vtxsz;
}
TSlot	*CSlot::buildBegin(bool doPruning)
{
	DPF(("--------------\n"));
	size_t nodeByteSize;
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_slot = new TSlot;
	m_sz = 0;
	m_stride = 0;
	TAttributePool *pAttrPool;
	SETNAME(m_slot, m_name.c_str() );
	//      sz = writeInterleavedAttributes(attributes, tempslot, tempattributePool, numVtx);
	//int ObjTranslator::writeInterleavedAttributes(
	//	std::vector<VertexAttribute*> &attributeList, bk3d::Slot &slot, bk3d::AttributePool &attributePool, unsigned int numVtx)

	pAttrPool = new(sizeof(bk3d::Ptr64<bk3d::Attribute>)*(m_numAttr-1)) TAttributePool;
	m_slot->pAttributes = pAttrPool;
	pAttrPool->n = (int)m_numAttr;

	//
	// Take the lowest size (if ever the buffers didn't fit in size)
	//
	MMapBuffer::iterator iB = m_attrPair.first;
	m_numVtx = iB->second->GetNumItems();
	++iB;
	while(iB != m_attrPair.second)
	{
		unsigned int n = iB->second->GetNumItems();
		if(n < m_numVtx)
		{
			DPF(("Warning : buffer sizes don't match\n"));
			m_numVtx = n;
		}
		++iB;
	}
	//
	// Compute here the amount of vertices that will be taken into account
	//
	unsigned newNumVtx = m_attrPair.first->second->GetNumItems();
	//if(bDeltaBS && doPruning)
	{
		newNumVtx = 0;
		//DPF(("Filtering BS vertices magnitude < 0.01f...\n"));
		MMapBuffer::iterator iB = m_attrPair.first;
		//while(iB != m_attrPair.second) Only do it on the first attrib
		{
			unsigned int n = m_numVtx;// iB->second->GetNumItems();
			for(unsigned int i=0; i<n; i++)
			{
				float mag = 0;
				for(int c=0; c<iB->second->GetNumComps(); c++)
				{
					float f = iB->second->GetValueAsFloat(i,c);
					mag += f*f;
				}
				//if(mag >= objTranslator->m_BSDeltaThreshold)
					newNumVtx++;
				m_validVtxTable.push_back(/*mag >= objTranslator->m_BSDeltaThreshold ?*/ true /*: false*/);
			}
			++iB;
		}
		//PF(("BS %d%% used: %d/%d (%d removed)\n", (int)(100.0*newNumVtx/numVtx), numVtx, newNumVtx, numVtx- newNumVtx));
	}
	DPF(("Slot %s has %d vertices out of %d originally\n", m_slot->name, m_numVtx, newNumVtx));
	//
	// compute total size and set some pointers
	//
	unsigned int j=0;
	iB = m_attrPair.first;
	while(iB != m_attrPair.second)
	{
		int ssz = newNumVtx * iB->second->GetItemSizeBytes();
		m_sz += ssz;
		// update the pointer in the local Attribute pool
		pAttrPool->p[j] = m_attributePool->p[iB->second->m_bufNum];
		//
		// compute stride
		//
		m_stride += iB->second->GetItemSizeBytes();
		DPF((" Slots Attrib %d = %s\n",j, pAttrPool->p[j]->name));
		j++;
		++iB;
	}
	m_slot->vtxBufferStrideBytes = m_stride;
	//
	// Store to numVertices : DX9 wants it
	//
	m_slot->vertexCount = newNumVtx; // shall we remove this occurence ?
	//
	// Finalize the Slot
	//
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_slot->nodeByteSize = (unsigned int)nodeByteSize;
	m_slot->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
	DPF(("CSlot sz : %d\n", nodeByteSize));
	return m_slot;
}
//
// Second part of the Slot building : here we allocate the vtx buffer and fill it
// we will allocate all the Slots one after another so the memory is contiguous
// and we can point all the Blendshape at once starting from the first one !
// NOTE: now with the second pool (Bk3dPool::cur_bk3dPool2), not really necessary to have split
// 
TSlot	*CSlot::buildEnd()
{
	//
	// reserve the memory
	//
	m_slot->vtxBufferSizeBytes = m_sz;
	m_slot->pVtxBufferData = Bk3dPool::cur_bk3dPool2->alloc(m_sz); // in the second pool
	//
	// write interleaved data 
	//
	DPF(("Slot %s writing interleaved data for %d\n", m_slot->name, m_numVtx));
	int tmpOffs = 0; //CHECK this...
	char *pData = (char*)m_slot->pVtxBufferData;
	unsigned int AlignedByteOffset = 0;
	for(unsigned int vtx=0; vtx<m_numVtx; vtx++)
	{
		// if the vertex is not welcome to be exported...
		if((!m_validVtxTable.empty()) && (m_validVtxTable[vtx] == false))
			continue;
		// export the vertex with interleaved attribs
		// walk through all attributes:
		MMapBuffer::iterator iB = m_attrPair.first;
		while(iB != m_attrPair.second)
		{
            CBuffer* pCBuffer = iB->second;
			int a = pCBuffer->m_bufNum;
			//
			// update atribute pointers (espacially for OpenGL)
			//
			if(vtx == 0)
			{
				m_attributePool->p[a]->dataOffsetBytes = tmpOffs;
				m_attributePool->p[a]->alignedByteOffset = AlignedByteOffset; // DXGI offset for layout definition
				m_attributePool->p[a]->pAttributeBufferData = (void*)pData;
				m_attributePool->p[a]->strideBytes = m_stride; // already computed
				DPF(("Updating Attrib details :\n    attrib offs %d \n", tmpOffs));
				DPF(("    aligned byte offs %d \n", AlignedByteOffset));
				DPF(("    strideBytes %d \n", m_stride));
			}
			int nc = pCBuffer->GetNumComps();
			if(nc > 4)
			{
				assert(!"more than 4 components. Need to check this situation!");
				DPF(("more than 4 components. Need to check this situation!"));
			}
			int bsz = pCBuffer->GetItemSizeBytes();
			for(int comp=0; comp < nc; comp++)
			{
				float v = 1.0;
				unsigned int l = 0;
                switch(pCBuffer->GetDataType())
                {
                case bk3dlib::FLOAT32:
					v = pCBuffer->GetValueAsFloat(vtx, comp);
					((float*)pData)[comp] = v;
                    break;
                case bk3dlib::FLOAT16:
					//v = pCBuffer->GetValueAsFloat(vtx, comp);
					//((float16*)pData)[comp] = v;
                    assert(!"TODO");
                    break;
                case bk3dlib::UINT32:
					l = pCBuffer->GetValueAsUInt(vtx, comp);
					((unsigned int*)pData)[comp] = l;
                    break;
                case bk3dlib::UINT16:
					l = pCBuffer->GetValueAsUInt(vtx, comp);
					((unsigned short*)pData)[comp] = (unsigned short)l;
                    break;
                case bk3dlib::UINT8:
					l = pCBuffer->GetValueAsUInt(vtx, comp);
					((unsigned char*)pData)[comp] = (unsigned char)l;
                    break;
                }
			}
			pData += bsz;
			tmpOffs += bsz;
			AlignedByteOffset += bsz;
			++iB;
		}
	}
#if 0
	//TODO: Solve this hack of only 4 bones weights !")
	// TEMP HACK :( for skinning limited to 4 items
	MapBuffer iB = m_attrPair.first;
	while(iB != m_attrPair.second)
    //for(unsigned int j=0; j < m_numAttr; j++)
    {
        int a = attributes[j]->m_attributePos;
        if(m_attributePool->p[a]->numComp > 4) m_attributePool->p[a]->numComp = 4; 
    }
#endif
	return m_slot;
}

//void	CSlot::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Slot *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//	if(p->pAttributes) for(int i=0; i< p->pAttributes->n; i++)
//	{
//		// already done in Mesh...CSlot::ptrToOffset(relocSlot, pHead, p->pSlots->p[i]);
//		PTR2OFFSET(bk3d::Attribute, p->pAttributes->p[i].p);
//	}
//    PTR2OFFSET(bk3d::AttributePool, p->pAttributes);
//    PTR2OFFSET(void, p->pVtxBufferData);
//	DPF(("CSlot slots : %d\n", localSlots));
//}

