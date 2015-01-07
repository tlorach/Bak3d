// obj2bk3d.cpp : Defines the entry point for the console application.
//

#pragma warning(disable:4312)

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501    // Change this to the appropriate value to target other versions of Windows.
#endif                        

#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#include "assert.h"

#include <vector>
#include <string>

#include "bk3dlib.h"
#include "bk3dEx.h"
#include "bk3dDbgFuncs.inl" // additional functions for debug purpose : to print out the binary data
#include "nvModel.h"

#include "nv_math.h"
using namespace nv_math;

extern bool verbose;
struct SConv {
    char * englishName;
    unsigned char *jpName;
};
extern bool convertJtoEn(SConv *table, char *name);
extern SConv boneTable_table[];
extern SConv skinTable_table[];

class VMD
{
private:
    //------------------------------------------------------------------------
    // See http://mikumikudance.wikia.com/wiki/VMD_file_format
    //------------------------------------------------------------------------
    #pragma pack(push, 1)


    struct Header {
        char magic[30]; // "Vocaloid Motion Data file" for 1.30 or "Vocaloid Motion Data 0002"
        char compat[20];
    };
    struct BoneFrame {
        char name[15];
        unsigned int frame;
        vec3f pos;
        quatf rot;
        unsigned char whathever[64];
    };
    struct BoneFrames {
        unsigned int nKeys;
        BoneFrame frames[1];
    };
    struct MorphFrame {
        char name[15];
        unsigned int frame;
        float weight;
    };
    struct MorphFrames {
        unsigned int nKeys;
        MorphFrame frames[1];
    };

    #pragma pack(pop)

    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    FILE*       fd;
    Header      header;
    BoneFrames  *boneFrames;
    MorphFrames *morphFrames;

    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool readHeader()
    {
        if(fread(header.magic, 30, 1, fd) == EOF)
            return false;
        int l;
        if(strstr(header.magic, "0002") > 0)
            l =20;
        else
            l= 10;
        if(fread(&header.compat, l, 1, fd) == EOF)
            return false;
        return true;
    }
    bool readBoneFrames()
    {
        int N;
        if(fread(&N, sizeof(long), 1, fd) == EOF)
            return false;
        if(N==0)
            return true;
        boneFrames = (BoneFrames*)malloc(sizeof(BoneFrames)+(N-1)*sizeof(BoneFrame));
        boneFrames->nKeys = N;
        if(fread(boneFrames->frames, sizeof(BoneFrame), N, fd) == EOF)
                return false;
        for(int i=0; i<N; i++)
        {
            convertJtoEn(boneTable_table, boneFrames->frames[i].name);
        }
        return true;
    }
    bool readMorphFrames()
    {
        int N;
        if(fread(&N, sizeof(long), 1, fd) == EOF)
            return false;
        if(N==0)
            return true;
        morphFrames = (MorphFrames*)malloc(sizeof(MorphFrames)+(N-1)*sizeof(MorphFrame));
        morphFrames->nKeys = N;
        if(fread(morphFrames->frames, sizeof(MorphFrame), N, fd) == EOF)
                return false;
        for(int i=0; i<N; i++)
        {
            convertJtoEn(skinTable_table, morphFrames->frames[i].name);
        }
        return true;
    }
public:
    VMD()
    {
        fd = NULL;
        boneFrames = NULL;
        morphFrames = NULL;
    }
    ~VMD()
    {
        if(fd) fclose(fd);
        free(boneFrames);
        free(morphFrames);
    }
    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool read(const char* fullInStr)
    {
        if(verbose) printf( "loading file...\n");
        fd = fopen(fullInStr, "rb");
        if(!fd)
        {
            if(verbose) printf( "failed loading file...\n");
            return false;
        }
        if(!readHeader())
            goto failed;
        if(!readBoneFrames())
            goto failed;
        if(!readMorphFrames())
            goto failed;
        fclose(fd);
        return true;
    failed:
        fclose(fd);
        return false;
    }
    //------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------
    bool buildKey(BoneFrame* pBoneKey, bk3dlib::PFileHeader fileHeader)
    {
        // check if already created
        bk3dlib::PQuatCurve q = fileHeader->GetQuatCurve(pBoneKey->name);
        bk3dlib::PCurveVec pos = fileHeader->GetCurveVec(pBoneKey->name);
        // if not, create it!
        if(q == NULL)
        {
            assert(pos == NULL);
            q = bk3dlib::QuatCurve::Create(pBoneKey->name);
            fileHeader->AttachQuatCurve(q);
            pos = bk3dlib::CurveVec::Create(pBoneKey->name, 3);
            fileHeader->AttachCurveVec(pos);
        } else {
            assert(pos);
        }
        // then add this key
        //char tmp[50];
        //sprintf(tmp, "%s \n", pBoneKey->name);
        //OutputDebugString(tmp);
        q->AddKey((float)pBoneKey->frame, pBoneKey->rot.comp);
        pos->AddKey((float)pBoneKey->frame, pBoneKey->pos.vec_array,
            bk3dlib::CurveVec::kTangentLinear, bk3dlib::CurveVec::kTangentLinear, 0.0, 0.0, 0.0, 0.0);
        return true;
    }

    bool buildbk3d(const char* singleName, bk3dlib::PFileHeader fileHeader)
    {
        for(unsigned int i=0; i<boneFrames->nKeys; i++)
            buildKey(boneFrames->frames+i, fileHeader);
        return true;
    }
}; // Class PMD

bool readVMD(const char* fullInStr, const char* singleName, bk3dlib::PFileHeader fileHeader)
{
    VMD vmd;
    if(!vmd.read(fullInStr))
        return false;
    return vmd.buildbk3d(singleName, fileHeader);
}
