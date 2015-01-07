#include <windows.h> // used for Virtual file mapping
#include "PoolAllocator.h"
#include "builder.h"

#ifdef USE_FILE_MAPPING
    static SECURITY_ATTRIBUTES sa = {
      sizeof(SECURITY_ATTRIBUTES),
      /*lpSecurityDescriptor*/NULL,
      /*bInheritHandle*/FALSE
    };
#endif

/*void *Bk3dPool::operator new( size_t stAllocateBlock)
{
    void *pvTemp = malloc( stAllocateBlock );
    if( pvTemp != 0 )
        memset( pvTemp, 0, stAllocateBlock );
    return pvTemp;
}
void * Bk3dPool::operator new[] (size_t stAllocateBlock)
{
    void *pvTemp = malloc( stAllocateBlock );
    if( pvTemp != 0 )
        memset( pvTemp, 0, stAllocateBlock );
    return pvTemp;
}
void   Bk3dPool::operator delete (void* ptr)
{
	if(ptr) free(ptr);
}
void   Bk3dPool::operator delete[] (void* ptr)
{
	if(ptr) free(ptr);
}
*/
Bk3dPool::Bk3dPool(LPCSTR file)
{
	pool = NULL;
	availablePtr = NULL;
	size = 0;
	usedSize = 0;
	//availablePtrHi = NULL;
	//usedSizeLow = usedSizeHi = 0;

#ifdef USE_FILE_MAPPING
    hmmap = NULL;
    hfile = CreateFile(file,GENERIC_READ | GENERIC_WRITE,
        0, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    setSize(1024);
#endif

}
Bk3dPool::~Bk3dPool()
{
#ifdef USE_FILE_MAPPING
    FlushViewOfFile(pool, 0);
    UnmapViewOfFile(pool);
    CloseHandle(hmmap);
    CloseHandle(hfile);
#else
	if(pool)
        free(pool);
#endif
}
bool Bk3dPool::setSize( size_t sizeBytes)
{
#ifdef USE_FILE_MAPPING
    if(sizeBytes > 0x1F400000) // 500Mb max
    {
        PRINTF(("Error>> reached the limit of 500Mb file(%d Mbytes)!!\n", (sizeBytes*2)/(1024*1024)));
        return false;
    }
  //#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Bk3dPool::setSize() - WARNING: I have a bug in the evaluation of the required size... multiplied by 2")
    if(pool)
        UnmapViewOfFile(pool);
    if(hmmap)
        CloseHandle(hmmap);
    hmmap = CreateFileMapping(
      hfile,
      &sa,
      PAGE_READWRITE,
      0x00000000, sizeBytes,
      NULL );
    BOOL bRes;
    /*if(pool == NULL)
    {
        pool = VirtualAlloc(NULL, 500*1024*1024, MEM_RESERVE,PAGE_READWRITE);
        bRes = VirtualFree(pool, 500*1024*1024, MEM_DECOMMIT);
    }*/
    pool = MapViewOfFileEx(hmmap, FILE_MAP_ALL_ACCESS, 0, 0, sizeBytes, NULL);//pool);
#else
    if(pool)
        free(pool); // do realloc instead ??
	pool = malloc(sizeBytes * 2); // WARNING: I have a bug in the evaluation of the required size... multiplied by 2
#endif
    if(pool == NULL)
    {
#ifdef USE_FILE_MAPPING
        PRINTF(("Error>> failed to map memory to file(%d Mbytes)!!\n", (sizeBytes*2)/(1024*1024)));
#else
        PRINTF(("Error>> failed to malloc(%d Mbytes)!!\n", (sizeBytes*2)/(1024*1024)));
#endif
        return false;
    }
	availablePtr = (char*)pool;
	size = sizeBytes;
    return true;
}
void *Bk3dPool::alloc( size_t sizeBytes)
{
	void *p = availablePtr;
	//static char dbgMsg[100];
	//sprintf(dbgMsg, "allocate : %d ; %d used; %d free\n", sizeBytes, sizeBytes+usedSize, size-(usedSize + sizeBytes));
	//printf(dbgMsg);
#ifdef USE_FILE_MAPPING
	if((usedSize + sizeBytes) > size)
	{
        // For now I can't make the file grow...
        // raising an error : we assume all should have be setup before
        // TODO: allow setSize() to be called here
		EPF(("usedSize(%d) + sizeBytes(%d) = %d > maxsize(%d)\n", usedSize, sizeBytes, usedSize+sizeBytes, size));
		throw("insufficient memory in Bk3dPool");
        //size_t extraSz = (usedSize + sizeBytes) - size;
        //setSize(usedSize + sizeBytes);
    }
#else
	if((usedSize + sizeBytes) > (size*2))
	{
		EPF(("usedSize(%d) + sizeBytes(%d) = %d > maxsize(%d)\n", usedSize, sizeBytes, usedSize+sizeBytes, size));
		throw("insufficient memory in Bk3dPool");
		//return NULL;
	}
	if((usedSize + sizeBytes) > size)
	{
    PRINTF(("Warning>> usedSize(%d) + sizeBytes(%d) = %d > maxsize(%d)\n", usedSize, sizeBytes, usedSize+sizeBytes, size));
	}
#endif
	availablePtr += sizeBytes;
	usedSize += sizeBytes;
	memset( p, 0, sizeBytes );
	return p;
}

bool Bk3dPool::isValidPtr(void* p)
{
    size_t pp = (size_t)p;
    size_t min = (size_t)pool;
    size_t max = min + (size_t)usedSize;
    return (pp >= min)&&(pp < max);
}

Bk3dPool *Bk3dPool::cur_bk3dPool = NULL;
Bk3dPool *Bk3dPool::cur_bk3dPool2 = NULL;

