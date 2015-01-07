#include "builder.h"
//
/// Classes for managing the creation of our binary data
/// Each class will take care of what to put in the structures that got 
/// allocated within the Pool
//
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO -  Curves")
/*------------------------------------------------------------------
	Curves
  ------------------------------------------------------------------*/
size_t	CQuatCurve::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	int nKeys = (int)m_keys.size();
	if(nKeys == 0) 
		nKeys = 1;
	size_t sz = sizeof(bk3d::QuatCurve) + sizeof(bk3d::QuatReadKey)*(nKeys-1);
	DPF(("QuatCurve : %d\n", sz));
	return sz;
}
TQuatCurve *CQuatCurve::build()
{
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_curve = new((m_keys.size()-1)*sizeof(bk3d::QuatReadKey)) TQuatCurve;
	SETNAME(m_curve, m_name.c_str()); // default name for the node

	m_curve->nKeys = (int)m_keys.size();
	for(int i=0; i<m_curve->nKeys; i++)
	{
		bk3d::QuatReadKey &keyDst = m_curve->key[i];
		bk3d::QuatReadKey &keySrc = m_keys[i];
		keyDst = keySrc;
	}

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_curve->nodeByteSize = (unsigned int)nodeByteSize;
	m_curve->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

	//
	// Now the subset Node for FloatArray
	//
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_curve->pFloatArray = new(0) TFloatArray;
	SETNAME(m_curve->pFloatArray, m_name.c_str()); // same name as the curve
	m_curve->pFloatArray->dim = 1;
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_curve->pFloatArray->nodeByteSize = (unsigned int)nodeByteSize;
	m_curve->pFloatArray->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !

    DPF(("CQuatCurve sz : %d\n", nodeByteSize));
	return m_curve;
}


