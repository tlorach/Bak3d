#include<vector>
#include<map>
#include<set>
#include<list>
#include<string>

#define SIBOPTIM
#include "builder.h"

#ifdef SIBOPTIM
struct IdxSet2 {
	static int nindex;
	unsigned int newIndex;
    unsigned int index[1];
    bool operator== (const IdxSet2 &rhs) {
        for(int i=0; i<nindex; i++)
        {
            if (index[i] != rhs.index[i])
                return false;
        }
        return true;
	}
    bool operator< ( const IdxSet2 &rhs) const {
        for(int i=0; i<nindex; i++)
        {
            if (index[i] < rhs.index[i])
                return true;
            if (index[i] > rhs.index[i])
                return false;
        }
        return false;
    }
};
int IdxSet2::nindex = 0;
#endif
/*-------------------------------------------------------------------------
  
 */
void CBuffer::SIB_ClearBuffers()
{
	m_SIBBuffers.clear();
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
#ifdef SIBOPTIM
	// create a vector of vector
	// The idea is to create a sort of bucket system
	// with the same amount of items in each
	// when one reach its limit: all are getting one more item
	// and instead of having an array of buckets that each have N items
	// we have N layers of arrays of bucket-item
	std::vector<IdxSet2* > layerbucket;
	// One default layer
	int numlayers = 1;
	layerbucket.resize(numlayers);

	// check integrity of the buffers
	int numidx = 0;
	int indexCounter = 0;
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
	// how many multi-index in this model
	IdxSet2::nindex = (int)m_SIBBuffers.size();
	size_t IdxSetSz = (sizeof(IdxSet2) + sizeof(int)*(IdxSet2::nindex));
	// default layer at the size of the maximum possible index for position
	layerbucket[0] = (IdxSet2*)malloc(numidx * IdxSetSz );
	for(int i=0; i<numidx; i++)
	{
		IdxSet2* pIdxSet = ((IdxSet2*)((char*)(layerbucket[0]) + i*IdxSetSz));
		pIdxSet->newIndex = -1; // init value
	}
	IdxSet2 *idxset = (IdxSet2 *)malloc(IdxSetSz);
    // let's take the first idxset buffer to walking through
	for(int i=0; i<numidx; i++)
	{
		// build the index set
		for(int b=0; b<IdxSet2::nindex; b++)
			idxset->index[b] = m_SIBBuffers[b].idxBuffers->GetValueAsUInt(i,0);
		int index = idxset->index[0];
		// Now look for this combination
		int j;
		for(j=0; j<numlayers; j++)
		{
			IdxSet2 &bucketidxset = *((IdxSet2*)((char*)(layerbucket[j]) + index*IdxSetSz));
			if(bucketidxset.newIndex == 0xFFFFFFFF)
				break;
			if(bucketidxset == (*idxset))
				break;
		}
		// if reached the end: add a new layer
		if(j == numlayers)
		{
			numlayers++;
			layerbucket.resize(numlayers);
			layerbucket[numlayers-1] = (IdxSet2*)malloc(numidx * IdxSetSz );
			j = numlayers-1;
			for(int i=0; i<numidx; i++)
			{
				IdxSet2* pIdxSet = ((IdxSet2*)((char*)(layerbucket[numlayers-1]) + i*IdxSetSz));
				pIdxSet->newIndex = -1; // init value
			}
		}
		IdxSet2* bucketidxset = ((IdxSet2*)((char*)(layerbucket[j]) + index*IdxSetSz));
		// if not found: create it
		if (bucketidxset->newIndex == 0xFFFFFFFF) 
		{
			//
			// add the new idx value in the new idx table
			//
			m_UIVals.push_back(indexCounter);
			//
			// add this new index combination in the bucket
			//
			for(int b=0; b<IdxSet2::nindex; b++)
				bucketidxset->index[b] = idxset->index[b];
			bucketidxset->newIndex = indexCounter;
			indexCounter++;
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
						v[c] = m_SIBBuffers[b].vtxBuffersSrc->GetValueAsFloat(idxset->index[b],c);
					m_SIBBuffers[b].vtxBuffersDst->AddData(v, nc);
				}
				else // unsigned int format
				{
					unsigned int v[4];
					assert(nc <= 4);
					for(int c=0; c<nc; c++)
						v[c] = m_SIBBuffers[b].vtxBuffersSrc->GetValueAsUInt(idxset->index[b],c);
					m_SIBBuffers[b].vtxBuffersDst->AddData(v, nc);
				}
			}
		}
		else {
			//
			// idx already there : just add its #...
			//
			m_UIVals.push_back(bucketidxset->newIndex);
		}
	}
	for(int i=0; i<numlayers; i++)
		free(layerbucket[i]);
	free(idxset);
	return true;
#else
    //merge the points
	std::map<IdxSet, unsigned int> pts;

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
		if(mit == pts.end()) 
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
						v[c] = m_SIBBuffers[b].vtxBuffersSrc->GetValueAsUInt(idx.getIndex(b),c);
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
	return true;
#endif
}


