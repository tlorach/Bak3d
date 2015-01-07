#include<vector>
#include<map>
#include<set>
#include<string>

#include "builder.h"


/*-------------------------------------------------------------------------
  
 */
void CBuffer::SIB_ClearBuffers()
{
	m_SIBBuffers.clear();
	m_idxset.clearIndex();
}
/*-------------------------------------------------------------------------
  
 */
void CBuffer::SIB_AddBuffers(bk3dlib::PBuffer bufferIdx, bk3dlib::PBuffer bufferAttrSrc, bk3dlib::PBuffer bufferAttrDst)
{
	BufferGroup grp;
	grp.idxBuffers = static_cast<CBuffer*>(bufferIdx);
	grp.vtxBuffersSrc = static_cast<CBuffer*>(bufferAttrSrc);
	grp.vtxBuffersDst = bufferAttrDst ? static_cast<CBuffer*>(bufferAttrDst) : NULL;
	m_SIBBuffers.push_back(grp);
}
/*-------------------------------------------------------------------------
  
 */
bool CBuffer::SIB_Compile()
{
    //merge the points
	std::map<IdxSet, unsigned int> pts;

    //find whether a position is unique
	std::set<unsigned int> ptSet;
	// check integrity of the buffers
	int numidx = 0;
    for(unsigned int i=0; i<m_SIBBuffers.size(); i++)
	{
		if(numidx == 0)
		{
			numidx = m_SIBBuffers[i].idxBuffers->GetNumItems();
		}
		else if(numidx != m_SIBBuffers[i].idxBuffers->GetNumItems())
			return false;
		m_SIBBuffers[i].vtxBuffersDst->ClearData();
	}
    // let's take the first idx buffer to walking through
	for(int i=0; i<numidx; i++)
	{
		IdxSet idx;
		for(unsigned int b=0; b<m_SIBBuffers.size(); b++)
		{
			idx.addIndex(m_SIBBuffers[b].idxBuffers->GetValueAsUInt(i,0) );
		}
		std::map<IdxSet,unsigned int>::iterator mit = pts.find(idx);
		//
		// idx combinations not found... so create a new one
		//
		if (mit == pts.end()) 
		{
			//
			// add the new idx value in the new idx table
			//
			m_UIVals.push_back( (unsigned int)pts.size());
			//
			// Keep track of the new combination of idx
			//
			pts.insert( std::map<IdxSet,unsigned int>::value_type(idx, (unsigned int)pts.size()));
			//
			// Store vertex values to the output buffers
			//
			for(unsigned int b=0; b<m_SIBBuffers.size(); b++)
			{
				int nc = m_SIBBuffers[b].vtxBuffersSrc->GetNumComps();
				if(m_SIBBuffers[b].vtxBuffersSrc->isFloatFormat())
				{
					float v[4];
					assert(nc <= 4);
					for(int c=0; c<nc; c++)
						v[c] = m_SIBBuffers[b].vtxBuffersSrc->GetValueAsFloat(idx.getIndex(b),c);
					m_SIBBuffers[b].vtxBuffersDst->AddData(v, nc);
				}
				else // unsigned int format
				{
					unsigned int v[4];
					assert(nc <= 4);
					for(int c=0; c<nc; c++)
						v[c] = m_SIBBuffers[b].vtxBuffersSrc->GetValueAsUInt(idx.getIndex(i),c);
					m_SIBBuffers[b].vtxBuffersDst->AddData(v, nc);
				}
			}
		}
		else {
			//
			// idx already there : just add its #...
			//
			m_UIVals.push_back( mit->second);
		}
	}
	return false;
}


