#include "builder.h"
//
/// Classes for managing the creation of our binary data
/// Each class will take care of what to put in the structures that got 
/// allocated within the Pool
//

/*------------------------------------------------------------------

	Attribute Pool

  ------------------------------------------------------------------*/
CAttribute::CAttribute(CBuffer *buf) :
	m_buffer(buf),
	m_attribute(NULL)
{
}
/*------------------------------------------------------------------

	Attribute Pool

  ------------------------------------------------------------------*/
/*size_t	CAttribute::getTotalSize(int &relocationSlots)
{
	return 0;
}*/
/*------------------------------------------------------------------

	Attribute Pool

  ------------------------------------------------------------------*/
TAttribute	*CAttribute::build(const char *prefix, int bs)
{
	DPF(("--------------\n"));
	//
	// contiguous allocation
	//
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_attribute = new TAttribute;
	SETNAME(m_attribute, m_buffer->GetName());
	m_attribute->slot = m_buffer->GetSlot();

	static char fullattrname[64];
	static char attrname[64];
	strcpy_s(attrname, 64, m_buffer->GetName());
	char *c = attrname + strlen(attrname);
	while(((*(c-1)) >= '0')&&((*(c-1)) <= '9'))
		c--;
	int semanticIdx = atoi(c);
	//*c = '\0';
	if(prefix != NULL)
	{
		semanticIdx = 0; // we cancel semantic idx for Blendshapes
		sprintf_s(fullattrname, 64,"%s%s%d", prefix, m_buffer->GetName(), bs);
	} else
		sprintf_s(fullattrname, 64,"%s", attrname);

	strcpy_s(m_attribute->name, 31, fullattrname);
	m_attribute->semanticIdx = semanticIdx;
	m_attribute->formatDXGI = m_buffer->FmtDXGI();
	m_attribute->formatDX9 =  m_buffer->FmtDX9();
	m_attribute->formatGL = m_buffer->FmtOpenGL();
	m_attribute->numComp = m_buffer->GetNumComps();

	m_attribute->dataOffsetBytes = 0;
    m_attribute->strideBytes = 0;

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_attribute->nodeByteSize = (unsigned int)nodeByteSize;
	m_attribute->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

	//
	// Some debug info
	//
	DPF(("CAttribute %s, sz : %d\n", m_attribute->name, nodeByteSize));
	DPF((" attribute fmt = %s\n", bk3d::formatAsString(m_attribute->formatDXGI)));
	DPF((" attribute num comp = %d\n", m_attribute->numComp));
	return m_attribute;
}

/*------------------------------------------------------------------

	Attribute Pool

  ------------------------------------------------------------------*/
//void	CAttribute::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Attribute *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//    PTR2OFFSET(void, p->pAttributeBufferData);
//	DPF(("CAttribute slots : %d\n", localSlots));
//}
