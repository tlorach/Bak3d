#include "builder.h"
/*------------------------------------------------------------------
	Mesh
  ------------------------------------------------------------------*/
CMesh::CMesh(const char * name)
{
    m_maxNumBones = 0;
    mesh = NULL;
    transformPool = NULL;
    pMayaCurves = NULL;
    if(name)
        m_name = std::string(name);

    memset(&aabbox, 0, sizeof(float)*6);
    memset(&bsphere, 0, sizeof(float)*6);

    m_maxNumBones = 0;
}
/*------------------------------------------------------------------

	Mesh size calc

  ------------------------------------------------------------------*/
size_t	CMesh::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::Mesh);
	//
	// Primgroup
	//
	// pool
	sz += sizeof(bk3d::PrimGroupPool) + sizeof(bk3d::Ptr64<bk3d::PrimGroup>)*(m_idxBuffers.size()-1);
	relocationSlots += (int)m_primgroups.size(); // reloc for elements in the pool
	std::vector<CPrimGroup*>::iterator iB = m_primgroups.begin();
	while(iB != m_primgroups.end())
	{
		sz += (*iB)->getTotalSize(relocationSlots);
		++iB;
	}
	//
	// size for Attribute Pool
	// # of attribs == # of vertex buffers (before cooking)
	//
	sz += sizeof(bk3d::AttributePool) + sizeof(bk3d::Ptr64<bk3d::Attribute>)*(m_vtxBuffers.size()-1);
	relocationSlots += (int)m_vtxBuffers.size(); // reloc for elements in the pool
	relocationSlots += (int)m_vtxBuffers.size() * 2;// slots in Attribute... ????????
	//
	// Attributes
	//
	sz += sizeof(bk3d::Attribute) * m_vtxBuffers.size();
	//
	// Slots : use multimap for counting
	//
	int numSlots = 0;
	for(unsigned int i=0; i<m_vtxBuffers.size(); i++)
	{
		if(m_vtxBuffers.count(i) == 0)
			break;
		std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair;
		pair = m_vtxBuffers.equal_range(i);
		CSlot cslot(this, pair, NULL, NULL);
		sz += cslot.getTotalSize(relocationSlots);
		numSlots++;
	}
	sz += sizeof(bk3d::SlotPool) + sizeof(bk3d::Ptr64<bk3d::Slot>)*(numSlots-1);
	relocationSlots += numSlots; // reloc for elements in the pool
	//
	// Blendshapes
	//
	sz += sizeof(bk3d::Attribute) * m_vtxBuffersBS.size();
	int numSlotsBS = 0;
	for(unsigned int i=0; i<m_vtxBuffersBS.size(); i++)
	{
		if(m_vtxBuffersBS.count(i) == 0)
			break;
		std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair;
		pair = m_vtxBuffersBS.equal_range(i);
		CSlot cslot(this, pair, NULL, NULL);
		sz += cslot.getTotalSize(relocationSlots);
		numSlots++;
	}
	sz += sizeof(bk3d::SlotPool) + sizeof(bk3d::Ptr64<bk3d::Slot>)*(numSlots-1);
	relocationSlots += numSlots; // reloc for elements in the pool
#if 0
	slotList.clear();
	slotList.push_back(std::vector<VertexAttribute*>());
	numAttribs = 0;
	numSlots = 0;//objTranslator->m_blendShapeNames.size();
	for(int i=0; i<MAXATTRIBS; i++)
	{
		//if(objTranslator->m_attributesInBS[i].attribute == None)
		//	continue;
		//slot 0 only... all BS have the the same (1 BS == 1 Slot)
		fakeva[i].m_type = None;//objTranslator->m_attributesInBS[i].attribute;
		fakeva[i].m_attributePos = i;
		slotList[0].push_back(fakeva+i);
		numAttribs++;
	}
	//
	// size for BS Attribute Pool
	//
	sz += sizeof(bk3d::AttributePool) + sizeof(bk3d::Ptr64<bk3d::Attribute>)*(numSlots*numAttribs-1);
	relocationSlots += numAttribs*numSlots; // reloc for elements in the pool
	relocationSlots += numAttribs*numSlots * 2;// slots in Attribute...
	//
	// BS Attributes
	//
	sz += sizeof(bk3d::Attribute) * numAttribs*numSlots;
#endif

	//
	// Transform references
	//
	if(m_transformRefs.size())
		sz += sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transformRefs.size() - 1);
	sz += sizeof(bk3d::TransformRefs);
	relocationSlots += (int)m_transformRefs.size();
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

	// BUG... TODO: Check why I need to add 8 bytes per mesh...
	sz += 2*sizeof(char*);

	//relocationSlots += (int)cvVectors.size()*2; // 2 ptrs per FloatArray

	DPF(("Mesh : %d\n", sz));
	return sz;
}
/*------------------------------------------------------------------

	Mesh build : in contiguous memory
	using transform2bk3d to reference correctly to the transformation

  ------------------------------------------------------------------*/
TMesh	*CMesh::build(const MapTransform2bk3d &transform2bk3d, const MapMayaCVPtrs &CVPtrs)
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//
	// Contiguous memory allocation
	//
	mesh = new TMesh();
	SETNAME(mesh, m_name.c_str());
	DPF(("Mesh %s\n", m_name.c_str()));

    mesh->userPtr = NULL;
    // Mesh visibility
    mesh->visible = 1.0;

	//memcpy((void*)&(mesh->aabbox), );
	//memcpy((void*)&(mesh->bsphere), );
	//
	// Primitive Groups allocation
	//
	int npg = (int)m_primgroups.size();
	mesh->pPrimGroups = new
		(sizeof(bk3d::Ptr64<bk3d::PrimGroup>)*(npg-1)) 
		TPrimGroupPool;
	DPF((" Num Primitive Groups = %d\n", npg));
	mesh->pPrimGroups->n = 0;
	for(int i=0, j=0; i<npg; i++)
	{
		TPrimGroup *pg = m_primgroups[i]->buildBegin(transform2bk3d);
		if(pg)
		{
			mesh->pPrimGroups->p[mesh->pPrimGroups->n++] = pg;
			j++;
		}
	}
	//
	// second part : allocate the index buffers and copy data in
	// this allows us to have all the buffer contiguous !
	//
	for(int i=0, j=0; i<npg; i++)
	{
		m_primgroups[i]->buildEnd();
	}
	//
	// Clear the pointer of bk3d allocations...
	//
	//for(int i=0, j=0; i<npg; i++)
	//{
	//	m_primgroups[i]->m_idxBuffer->SetBk3dPointer(NULL);
	//}
	//
	// Tricky part : the last Node must point to the next node by jumping above the index tables...
	//
	if(mesh->pPrimGroups->n > 0)
		mesh->pPrimGroups->p[mesh->pPrimGroups->n-1]->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr();
	//
    //
    // Num Joint influences in the mesh
    //
    mesh->numJointInfluence = m_maxNumBones; 

    //
	// Attributes... all attributes of all Slots together
    //
	int numAttribs = (int)m_vtxBuffers.size();

	// Create the pool
	mesh->pAttributes = new(sizeof(bk3d::Ptr64<bk3d::Attribute>)*(numAttribs-1)) TAttributePool;
	mesh->pAttributes->n = numAttribs;
	DPF((" Num Attributes = %d\n", numAttribs));

	MMapBuffer::iterator iVB = m_vtxBuffers.begin();
	int j=0;
	while(iVB != m_vtxBuffers.end())
	{
		CAttribute cattribute(iVB->second);
		TAttribute *attr = cattribute.build();
		if(attr)
		{
			assert(j < mesh->pAttributes->n);
			mesh->pAttributes->p[j++] = attr;
		}
		++iVB;
	}
    //
	// Attributes for Blendshape... attributes of one of the Slots (We assume Blendshapes share the same attribute layout
    //
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Check consistency of Buffers for Blendshapes ??")
	int numAttribsBS = (int)m_vtxBuffersBS.size();

	// Create the pool
	mesh->pBSAttributes = new(sizeof(bk3d::Ptr64<bk3d::Attribute>)*(numAttribsBS-1)) TAttributePool;
	mesh->pBSAttributes->n = numAttribsBS;
	DPF((" Num BS Attributes = %d\n", numAttribsBS));

	iVB = m_vtxBuffersBS.begin();
	j=0;
	while(iVB != m_vtxBuffersBS.end())
	{
        // Only do this for Slot #0 : the first Blendshape. We assume the others have the same use of attributes
        //if(iVB->second->GetSlot() == 0)
        {
		    CAttribute cattribute(iVB->second);
		    TAttribute *attr = cattribute.build();
		    if(attr)
		    {
			    assert(j < mesh->pBSAttributes->n);
			    mesh->pBSAttributes->p[j++] = attr;
		    }
        }
		++iVB;
	}
	//
	// Slots : use multimap for counting
	//
	int numSlots = 0;
	for(unsigned int i=0; i<m_vtxBuffers.size(); i++)
	{
		if(m_vtxBuffers.count(i) == 0)
			break;
		numSlots++;
	}
	DPF((" Num Slots = %d\n", numSlots));
	mesh->pSlots = new(sizeof(bk3d::Ptr64<bk3d::Slot>)*(numSlots-1)) TSlotPool;
	mesh->pSlots->n = numSlots;
	//
	// first pass for slots
	//
	std::vector<CSlot *> slotTable;
	slotTable.resize(numSlots);
    for(int slot=0; slot<numSlots; slot++)
    {
		std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair;
		pair = m_vtxBuffers.equal_range(slot);
        LPCSTR name = slot >= m_SlotNames.size() ? NULL:m_SlotNames[slot].c_str();
		slotTable[slot] = new CSlot(this, pair, mesh->pAttributes, name);
		mesh->pSlots->p[slot] = slotTable[slot]->buildBegin();
    } //for(slot...)
	//
	// the last Node must point to the next node by jumping above the index tables...
	//
	if(mesh->pSlots->n > 0)
		mesh->pSlots->p[mesh->pSlots->n-1]->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr();

	///////////////////////////////////////////////////////////////////////
	// BlendShapes
	//
	// Attributes... all attributes of all Slots together
	// for BS, only one slot per blendshape. All attribute are in the same slot
	//

	int numSlotsBS = 0;
	for(unsigned int i=0; i<m_vtxBuffersBS.size(); i++)
	{
		if(m_vtxBuffersBS.count(i) == 0)
			break;
		numSlotsBS++;
	}
	DPF((" Num BS Slots = %d\n", numSlotsBS));
	mesh->pBSSlots = new(sizeof(bk3d::Ptr64<bk3d::Slot>)*(numSlotsBS-1)) TSlotPool;
	mesh->pBSSlots->n = numSlotsBS;
    //
    // Blenshape weight array
    //
    mesh->pBSWeights = new(sizeof(float)*(numSlotsBS - 1)) TFloatArray;
    // TODO: fill in with proper coefs
    memset(mesh->pBSWeights->f, 0, sizeof(float)*numSlotsBS);

	//
	// first pass for slots
	//
	std::vector<CSlot *> slotTableBS;
	slotTableBS.resize(numSlotsBS);
    for(int slot=0; slot<numSlotsBS; slot++)
    {
		std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair;
		pair = m_vtxBuffersBS.equal_range(slot);
        LPCSTR name = slot >= m_BSSlotNames.size() ? NULL:m_BSSlotNames[slot].c_str();
		slotTableBS[slot] = new CSlot(this, pair, mesh->pBSAttributes, name);
		mesh->pBSSlots->p[slot] = slotTableBS[slot]->buildBegin();
    } //for(slot...)
	//
	// the last Node must point to the next node by jumping above the index tables...
	//
	if(mesh->pBSSlots->n > 0)
		mesh->pBSSlots->p[mesh->pBSSlots->n-1]->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr();

#if 0
	DPF((" Blendshapes\n"));
	int numBSSlots = 0;//objTranslator->m_blendShapeNames.size();
	int numBSAttribs = 0;
	for(int i=0; i<MAXATTRIBS; i++)
	{
		//if(objTranslator->m_attributesInBS[i].attribute == None)
		//	continue;
		numBSAttribs++;
	}
	// all attributes == sum of all BS and their attributes
	numBSAttribs *= numBSSlots;
    // for further processing, keep track of attributes for each slot
	slotList.clear();
    slotList.resize(numBSSlots);
	// Create the pool
	mesh->pBSAttributes = new(sizeof(bk3d::Ptr64<bk3d::Attribute>)*(numBSAttribs-1)) TAttributePool;
	mesh->pBSAttributes->n = numBSAttribs;

	// build will loop "numBSSlots" times for the given attribute
	for(int bs=0, j=0; bs<numBSSlots; bs++) 
	{
		for(int i=0; i<MAXATTRIBS; i++)
		{
			CAttribute cattribute;//(objTranslator->m_attributesInBS[i], i, objTranslator);
			TAttribute *attr = cattribute.build(slotList, "bs_", bs);
			if(attr)
			{
				assert(j < mesh->pBSAttributes->n);
				mesh->pBSAttributes->p[j++] = attr;
			}
		}
	}

	// Slots
	mesh->pBSSlots = new(sizeof(bk3d::Ptr64<bk3d::Slot>)*(numBSSlots-1)) TSlotPool;
	mesh->pBSSlots->n = numBSSlots;
	std::vector<CSlot *> bsSlotTable;
	bsSlotTable.resize(numBSSlots);
    for(int slot=0; slot<numBSSlots; slot++)
    {
      if(slotList[slot].size() == 0)
      {
        assert(!"shouldn't happen...");
        break;
      }
	  bsSlotTable[slot] = new CSlot(this, slotList[slot], numVtx, mesh->pBSAttributes);
	  mesh->pBSSlots->p[slot] = bsSlotTable[slot]->buildBegin(true);
	  strncpy(mesh->pBSSlots->p[slot]->name, /*objTranslator->m_blendShapeNames[slot].c_str()*/"TOTO", 31);
    } //for(slot...)
#endif
	//
	// second pass : so that the remaining allocations for vertex buffers is done
	// in a row : all numBSSlots buffers are contiguous
	//
    for(int slot=0; slot<numSlots; slot++)
    {
	  slotTable[slot]->buildEnd();
	  delete slotTable[slot];
	}
    for(int slot=0; slot<numSlotsBS; slot++)
    {
	  slotTableBS[slot]->buildEnd();
	  delete slotTableBS[slot];
	}
#if 0
	//
	// second pass : so that the remaining allocations for vertex buffers is done
	// in a row : all numBSSlots buffers are contiguous
	//
    for(int slot=0; slot<numBSSlots; slot++)
    {
	  bsSlotTable[slot]->buildEnd();
	  delete bsSlotTable[slot];
	}
	//
	// the last Node must point to the next node by jumping above the index tables...
	//
	if(mesh->pBSSlots->n > 0)
		mesh->pBSSlots->p[mesh->pBSSlots->n-1]->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr();

#endif
	//
	// Transform references
	//
	mesh->pTransforms = new(sizeof(bk3d::Ptr64<bk3d::Transform>)*(m_transformRefs.size() - 1)) TTransformRefs;
    mesh->pTransforms->n = m_transformRefs.size();
    // Setup the references to the correct transforms
	DPF((" Num Transforms = %d\n", m_transformRefs.size() ));
    for(unsigned int j=0; j<m_transformRefs.size(); j++)
	{
		MapTransform2bk3d::const_iterator iT = transform2bk3d.find(m_transformRefs[j]);
		assert(iT->second);
		// Now we can set the reference to the transform
		mesh->pTransforms->p[j] = iT->second;
	}
#if 0
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(cvVectors.size() > 0)
	{
		mesh->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(cvVectors.size() - 1)) TFloatArrayPool;
		mesh->pFloatArrays->n = (int)cvVectors.size();
		for(unsigned int i=0; i < cvVectors.size(); i++)
		{
			//TODO: Change the name : we should give a name corresponding to the destination
			strncpy(mesh->pFloatArrays->p[i].destName, cvVectors[i]->name, 31);
			mesh->pFloatArrays->p[i].p = pMayaCurves->p[cvVectors[i]->idx]->pFloatArray;
		}
	}
#endif
#if 1
	//
	// Curve references : pFloatArrays contains the generic stub
	//
	if(connectedCurveVectors.size() > 0)
	{
		mesh->pFloatArrays = new(sizeof(bk3d::FloatArrayPool::Connection)*(connectedCurveVectors.size() - 1)) TFloatArrayPool;
		mesh->pFloatArrays->n = (int)connectedCurveVectors.size();
		std::set< CurveVecConnection >::iterator iC = connectedCurveVectors.begin();
		int i=0;
		while(iC != connectedCurveVectors.end())
		{
			MapMayaCVPtrs::const_iterator iPtr = CVPtrs.find(iC->first);
			assert(iPtr != CVPtrs.end() );
			bk3d::MayaCurveVector *pMCurve = iPtr->second;
			assert(pMCurve->pFloatArray);
			bk3d::FloatArrayPool::Connection *pConn = mesh->pFloatArrays->p + i;
			pConn->p = pMCurve->pFloatArray;
			if(iC->second == bk3dlib::MESH_VISIBILITY)
			{
                pMCurve->pFloatArray->dim = 1; // force it by safety
				sprintf_s(pConn->destName, 32, "VIS_%s", mesh->name);
				pConn->pfTarget = &mesh->visible;
            }
            // case where we use a big vector of N components for N Blendshapes
            else if(iC->second == bk3dlib::MESH_BLENDSHAPE)
            {
                //pMCurve->pFloatArray->dim = check if the dim is equal or less than # of BS ?
                sprintf_s(pConn->destName, 32, "BSVEC_%s", mesh->name);
                pConn->pfTarget = mesh->pBSWeights->f;
            }
            int n = (int)iC->second - (int)bk3dlib::MESH_BLENDSHAPE0;
            if(n >= 0)
            {
                pMCurve->pFloatArray->dim = 1; // force it by safety
                sprintf_s(pConn->destName, 32, "BS%3d_%s", n, mesh->name);
                pConn->pfTarget = mesh->pBSWeights->f + n;
            }
			++iC; ++i;
		}
	}
#endif
	//
	// Bounding Box/Sphere
	//
#if 1

	mesh->aabbox = aabbox;
	mesh->bsphere = bsphere;

#endif
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	mesh->nodeByteSize = (unsigned int)nodeByteSize;
	mesh->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
	DPF(("CMesh sz : %d\n", nodeByteSize));
	return mesh;
 }
//void	CMesh::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Mesh *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//	if(p->pSlots) for(int i=0; i< p->pSlots->n; i++)
//	{
//		CSlot::ptrToOffset(relocSlot, pHead, p->pSlots->p[i]);
//		PTR2OFFSET(bk3d::Slot, p->pSlots->p[i].p);
//	}
//    PTR2OFFSET(bk3d::SlotPool, p->pSlots);
//	if(p->pPrimGroups) for(int i=0; i< p->pPrimGroups->n; i++)
//	{
//		CPrimGroup::ptrToOffset(relocSlot, pHead, p->pPrimGroups->p[i]);
//		PTR2OFFSET(bk3d::PrimGroup, p->pPrimGroups->p[i].p);
//	}
//    PTR2OFFSET(bk3d::PrimGroupPool, p->pPrimGroups);
//	if(p->pAttributes) for(int i=0; i< p->pAttributes->n; i++)
//	{
//		CAttribute::ptrToOffset(relocSlot, pHead, p->pAttributes->p[i]);
//		PTR2OFFSET(bk3d::Attribute, p->pAttributes->p[i].p);
//	}
//	PTR2OFFSET(bk3d::AttributePool, p->pAttributes);
//	if(p->pBSAttributes) for(int i=0; i< p->pBSAttributes->n; i++)
//	{
//		CAttribute::ptrToOffset(relocSlot, pHead, p->pBSAttributes->p[i]);
//		PTR2OFFSET(bk3d::Attribute, p->pBSAttributes->p[i].p);
//	}
//	PTR2OFFSET(bk3d::AttributePool, p->pBSAttributes);
//	if(p->pBSSlots) for(int i=0; i< p->pBSSlots->n; i++)
//	{
//		CSlot::ptrToOffset(relocSlot, pHead, p->pBSSlots->p[i].p);
//		PTR2OFFSET(bk3d::Slot, p->pBSSlots->p[i].p);
//	}
//    PTR2OFFSET(bk3d::SlotPool, p->pBSSlots);
//	if(p->pTransforms) for(int i=0; i< p->pTransforms->n; i++)
//	{
//		//This transformation will be processed by CFileHeader
//		//no need... CTransform::ptrToOffset(relocSlot, pHead, p->pTransforms->p[i]);
//		PTR2OFFSET(bk3d::Transform, p->pTransforms->p[i].p);
//	}
//    PTR2OFFSET(bk3d::TransformRefs, p->pTransforms);
//
//	if(p->pFloatArrays) for(int i=0; i< p->pFloatArrays->n; i++)
//	{
//		PTR2OFFSET(bk3d::FloatArray, p->pFloatArrays->p[i].p);
//		PTR2OFFSET(float, p->pFloatArrays->p[i].pfTarget);
//	}
//    PTR2OFFSET(bk3d::FloatArrayPool, p->pFloatArrays);
//	DPF(("CMesh slots : %d\n", localSlots));
//}

