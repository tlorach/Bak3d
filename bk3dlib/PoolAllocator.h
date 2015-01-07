#pragma once
#  pragma warning(disable:4786)
#include <algorithm>

//#define USE_FILE_MAPPING

extern int debugRelocationSlots;
extern int relocCnt;
extern int emptySlot;
/*----------------------------------------------------------------------------------*/ /**

Allocator
Allocate a chunck of memory, then do memory management withing this area
Very simple, since we don't need to free anything...

(CANCELED FOR NOW...
Added Hi/Low : we can allocate from the beginning and from the end. This allows to have 2 groups.
In our case, the second groups would typically be data to put in video memory, while the first is
related to structures that stay in CPU.)

**/ //----------------------------------------------------------------------------------
class Bk3dPool
{
private:
    void    *hfile;
    void    *hmmap;
	void	*pool;
	char	*availablePtr;
//	char	*availablePtrHi;
	size_t	size, usedSize; // usedSize == usedSizeLow + usedSizeHi
//	size_t	usedSizeLow, usedSizeHi;

public:
    //void *operator new( size_t stAllocateBlock);
	//void * operator new[] (size_t);
	//void   operator delete (void*);
	//void   operator delete[] (void*);
	Bk3dPool(const char* file);
	~Bk3dPool();
	bool setSize( size_t sizeBytes);
	size_t getSize() {return size;}
	size_t getUsedSize() {return usedSize;}
	//size_t getUsedSizeLow() {return usedSizeLow;}
	//size_t getUsedSizeHi() {return usedSizeHi;}
	void* getPool() {return pool;};
	void* getAvailablePtr() {return availablePtr;};
	//void* getAvailablePtrHi() {return availablePtrHi;};
	void *alloc( size_t sizeBytes);
    bool isValidPtr(void* p);

    static Bk3dPool *cur_bk3dPool;
    static Bk3dPool *cur_bk3dPool2;
    static void setCurPool(Bk3dPool *bk3dPool, Bk3dPool *bk3dPool2)
    {
        cur_bk3dPool = bk3dPool;
        cur_bk3dPool2 = bk3dPool2;
    }
    void* orphanPool()
    {
        void* p = pool;
        pool = NULL;
	    availablePtr = NULL;
	    size = usedSize = 0;
        return p;
    }
};

template <class T>
struct Pool : public T
{
    void	*operator new( size_t stAllocateBlock, size_t additionalBytes = 0, bool bHiPart=false);
	void	*operator new[] (size_t, bool bHiPart=false);
	void    operator delete (void*, bool bHiPart);
	void    operator delete[] (void*, bool bHiPart);
	void    operator delete( void* ptr, size_t additionalBytes, bool bHiPart);
};
template <class T>
void *Pool<T>::operator new( size_t stAllocateBlock, size_t additionalBytes, bool bHiPart)
{
    if(bHiPart && Bk3dPool::cur_bk3dPool2)
        return Bk3dPool::cur_bk3dPool2->alloc(stAllocateBlock + additionalBytes);
    else if((!bHiPart) && Bk3dPool::cur_bk3dPool)
        return Bk3dPool::cur_bk3dPool->alloc(stAllocateBlock + additionalBytes);
    return NULL;
}
template <class T>
void * Pool<T>::operator new[] (size_t stAllocateBlock, bool bHiPart)
{
    if(bHiPart && Bk3dPool::cur_bk3dPool2)
		return Bk3dPool::cur_bk3dPool2->alloc(stAllocateBlock);
    else if((!bHiPart) && Bk3dPool::cur_bk3dPool)
		return Bk3dPool::cur_bk3dPool->alloc(stAllocateBlock);
    return NULL;
}
template <class T>
void   Pool<T>::operator delete (void* ptr, bool bHiPart)
{
    assert(!"TODO");
}
template <class T>
void   Pool<T>::operator delete[] (void* ptr, bool bHiPart)
{
    assert(!"TODO");
}
template <class T>
void	Pool<T>::operator delete( void* ptr, size_t additionalBytes, bool bHiPart)
{
    assert(!"TODO");
}



