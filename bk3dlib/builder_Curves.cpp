#include "builder.h"
//
/// Classes for managing the creation of our binary data
/// Each class will take care of what to put in the structures that got 
/// allocated within the Pool
//
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Curves")
/*------------------------------------------------------------------
	Curves
  ------------------------------------------------------------------*/
size_t	CMayaCurve::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	int nKeys = (int)m_keys.size();
	if(nKeys == 0) 
		nKeys = 1;
	size_t sz = sizeof(bk3d::MayaCurve) + sizeof(bk3d::MayaReadKey)*(nKeys-1);
	DPF(("MayaCurve : %d\n", sz));
	return sz;
}
TMayaCurve *CMayaCurve::build()
{
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_curve = new((m_keys.size()-1)*sizeof(bk3d::MayaReadKey)) TMayaCurve;
	SETNAME(m_curve, m_name.c_str()); // default name for the node
	m_curve->inputIsTime = m_inputIsTime;
	m_curve->outputIsAngular = m_outputIsAngular;
	m_curve->isWeighted = m_isWeighted;
	m_curve->preInfinity = (bk3d::EtInfinityType)m_preInfinity;
	m_curve->postInfinity = (bk3d::EtInfinityType)m_postInfinity;

	m_curve->nKeys = (int)m_keys.size();
	for(int i=0; i<m_curve->nKeys; i++)
	{
		bk3d::MayaReadKey &keyDst = m_curve->key[i];
		bk3d::MayaReadKey &keySrc = m_keys[i];
		keyDst = keySrc;
	}

	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_curve->nodeByteSize = (unsigned int)nodeByteSize;
	m_curve->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
	DPF(("CMayaCurve sz : %d\n", nodeByteSize));
	return m_curve;
}

/*------------------------------------------------------------------
	Curve Vectors
  ------------------------------------------------------------------*/
size_t	CMayaCurveVector::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	int nCurves = (int)m_mayacurves.size();
	assert(nCurves > 0);
	relocationSlots += nCurves;
	size_t sz = sizeof(bk3d::MayaCurveVector) + sizeof(bk3d::MayaCurve)*(nCurves-1);

	for(unsigned int i=0; i<m_mayacurves.size(); i++)
	{
		sz += m_mayacurves[i]->getTotalSize(relocationSlots);
	}
	DPF(("MayaCurveVec : %d\n", sz));
	return sz;
}
TMayaCurveVector *CMayaCurveVector::build() // fileHeader for connection resolve
{
	DPF(("--------------\n"));
	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_curveVector = new((m_mayacurves.size()-1)*sizeof(bk3d::Ptr64<bk3d::MayaCurve>)) TMayaCurveVector;
	SETNAME(m_curveVector, m_name.c_str());
	m_curveVector->nCurves = (int)m_mayacurves.size();
	for(int i=0; i<m_curveVector->nCurves; i++)
	{
		m_curveVector->pCurve[i] = m_mayacurves[i]->build();
	}
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_curveVector->nodeByteSize = (unsigned int)nodeByteSize;
	m_curveVector->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
	//
	// Now the subset Node for FloatArray
	//
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	m_curveVector->pFloatArray = new((m_mayacurves.size()-1)*sizeof(float)) TFloatArray;
	SETNAME(m_curveVector->pFloatArray, m_name.c_str()); // same name as the curve
	m_curveVector->pFloatArray->dim = (int)m_mayacurves.size();
	nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize() - nodeByteSize;
	m_curveVector->pFloatArray->nodeByteSize = (unsigned int)nodeByteSize;
	m_curveVector->pFloatArray->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
    //
    // Connections
    //
    //std::set< TransformConnection >::iterator iTC = connectedTransforms.begin();
    //while(iTC != connectedTransforms.end())
    //{
    //    TransformConnection &tc = *iTC;
    //    //tc.first->m
    //}

	DPF(("CMayaCurveVec sz : %d\n", nodeByteSize));
	return m_curveVector;
}

//void	CMayaCurve::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::MayaCurve *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//	DPF(("CMayaCurve slots : %d\n", localSlots));
//}
//void	CMayaCurveVector::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::MayaCurveVector *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//	PTR2OFFSET(bk3d::FloatArray, p->pFloatArray);
//	for(int i=0; i<p->nCurves; i++)
//	{
//		CMayaCurve::ptrToOffset(relocSlot, pHead, p->pCurve[i]);
//		PTR2OFFSET(bk3d::MayaCurve, p->pCurve[i].p);
//	}
//	DPF(("CMayaCurveVector slots : %d\n", localSlots));
//}

