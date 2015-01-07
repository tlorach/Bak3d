// obj2bk3d.cpp : Defines the entry point for the console application.
//

#pragma warning(disable:4312)

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#include "assert.h"

#include <vector>
#include <string>

#include "bk3dlib.h"
#ifndef NOGZLIB
#   include <zlib.h>
#endif
#include "bk3dEx.h"
#include "bk3dDbgFuncs.inl" // additional functions for debug purpose : to print out the binary data
#include "nvModel.h"

#include "nv_math.h"
using namespace nv_math;

extern bool readPMD(const char* fullInStr, const char* singleName, bk3dlib::PFileHeader fileHeader);
extern bool readOBJ(const char* fullInStr, const char* singleName, bk3dlib::PFileHeader fileHeader);

enum CmdToken {
    delmesh,// "meshname" -o <ofile> <infile>
    delpg,// "meshname" "pgname"  -o <ofile> <infile>
    deltransf,// "transfName"  -o <ofile> <infile>
    delcv,// "cvName"  -o <ofile> <infile>
    delAttr,// "[meshName.]attrName"  -o <ofile> <infile>
    delMat,// "matName"  -o <ofile> <infile>
    switchcomp,// XZ "attrName" "otherAttr" ...;
    negcomp,// [X][Y][Z] "attrName" "otherAttr" ...;
    oneminus,// [X][Y][Z] "attrName" "otherAttr" ...;
    edittransf,// "transfName" name|pos|scale|rot <xval> <yval> <zval> <wval> OR "transfName" [abs]mat <v00> <v01> <v02> ... <v33>
    editmat,// "matName" name|diff|spec|... <xval> <yval> <zval> <wval>
    hidemesh,// "meshname"  -o <ofile> <infile>
    showmesh,// "meshname" -o <ofile> <infile>
    print, // print all/brief/<name>
    dumpvtx,// dumpvtx <mesh> <attr>
    dumpidx,// dumpidx <mesh> <pg>
    setvtx,// dumpvtx <mesh> <attr> <file of vtx or '-'>
    setidx,// dumpidx <mesh> <pg> <file of idx or '-'>
	objdst,// <dstfile> : flags the tool to generate ad dumpvtx/dumpidx in 1 or more obj files so we can directly use the extracted data in a DCC app
};

struct Cmd {
    enum Component {
        Pos,
        Scale,
        RelMat,
        AbsMat,
        //...
        Ambient,
        Diffuse,
        Spec,
    };
    CmdToken cmdType;
    std::vector<std::string> names;
    bool comps[4];
    Component comp; //pos|scale|rot...
    float vec[16];
    int   ivec[16];
};
std::vector<Cmd>    cmdList;
std::string         targetName;
std::string         fullInStr;
std::string         singleName;
bk3dlib::PFileHeader fileHeader = NULL; // header for complex database (bk3dlibNeeded == true)
bk3d::FileHeader*   bk3dHeader = NULL;  // header for when directly loading the baked data from file (bk3dlibNeeded == false)
void *              bufferMemory;
unsigned int        bufferMemorySz;
// in some trivial cases, we just need to load the bk3d file and perform the editing, then save again
bool bk3dlibNeeded = false;
bool saveBk3d = false;
bool verbose = true;
// data for obj mode where we put together a max of 3 attributes together
bool asobj = false;
FILE * objfd = stdout;
int curprefix = 0;
char * vtxobjprefix[] = { "v ", "vt ", "vn " };

/*
example :
dump data :
 bk3dutil.exe -dumpidx NV_Shaderball4Shape lambert1 sb.bk3d >idx.txt

 bk3dutil.exe
     -setidx NV_Shaderball4Shape lambert1 idx.txt
     -setvtx NV_Shaderball4Shape position vtx.txt
     sb.bk3d

 or

cat idx.txt | bk3dutil.exe -setidx NV_Shaderball4Shape lambert1 - sb.bk3d

 */

void printHelp()
{
    printf("bk3dutil.exe cmd args [cmd args] ... <inputFile>\n");
    printf("-o <destFileName>\n");
    printf("-bake\n");
    printf("-print all OR print ALL (will dump vtx and idx!) OR print brief OR print <name>\n");
    printf("-delmesh <meshname>\n");
    printf("-delpg <meshname> <pgname>\n");
    printf("-deltransf <transfName>\n");
    printf("-delcv <cvName>\n");
    printf("-delAttr <[meshName.]attrName>\n");
    printf("-delMat <matName>\n");
    printf("-switchcomp XZ <attrName> [otherAtt...]\n");
    printf("-negcomp [X][Y][Z] <attrName> [otherAttr...]\n");
    printf("-oneminus [X][Y][Z] <attrName> [otherAttr...]\n");
    printf("-edittransf <transfName> pos|scale|rot <xval> <yval> <zval> <wval>\n");
    printf("            <transfName> [abs]mat <v00> <v01> <v02> ... <v33>\n");
    printf("-editmat <matName> args");
    printf("args :      diff|spec <xval> <yval> <zval> <wval>\n");
    printf("            TODO: more to come\n");
    printf("            diffusetex <name> <filename ref>");
    printf("            diffusetex <name> <filename ref>");
    printf("            diffusetex <name> <filename ref>");
    printf("            specexptex <name> <filename ref>");
    printf("            ambienttex <name> <filename ref>");
    printf("            reflectivitytex <name> <filename ref>");
    printf("            transparencytex <name> <filename ref>");
    printf("            transluencytex <name> <filename ref>");
    printf("            speculartex <name> <filename ref>");
    //printf("-hidemesh <meshname>\n");
    //printf("-showmesh <meshname>\n");
    printf("-dumpvtx <meshname> <attrname>\n");
    printf("-dumpidx <meshname> <PGname>\n");
    printf("-setvtx <meshname> <attrname> <datafile|-> '-' means stdin\n");
    printf("-setidx <meshname> <PGname> <datafile|-> '-' means stdin\n");
}

//
// parse the commands from the cmd line
//
bool parseCmd(int &argc, char** &argv)
{
    if(argc < 1)
        return false;
    Cmd cmd;
    #define IFCMD(c) if(!strcmp(argv[0], #c))
    #define CMD(c) (!strcmp(argv[0], #c))
    IFCMD(-o)
    {
        // special command : just tell the target file name
        if(argc < 2)
            return false;
        targetName = std::string(argv[1]);
        argc -= 2;
        argv += 2;
        saveBk3d = true;
        return true;
    }
    else if(CMD(-help)||CMD(-h))
    {
        printHelp();
        exit(0);
    }
    else IFCMD(-print)
    {
        cmd.cmdType = print;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = false;
    }
    else IFCMD(-objdst)// <dstfile> : flags the tool to generate ad dumpvtx/dumpidx in 1 or more obj files so we can directly use the extracted data in a DCC app
    {
        // objfd != NULL toggles the OBJ output mode
        // see http://en.wikipedia.org/wiki/Wavefront_.obj_file
        asobj = true;
        if(!strcmp(argv[1], "-"))
        {
            objfd = stdout;
            verbose = false;
        } else {
            objfd = fopen(argv[1], "w");
            if(objfd == NULL) {
                fprintf(stderr, "failed to open for writing the file %s\n", argv[1]);
                exit(1);
            }
        }
        argc -= 2;
        argv += 2;
        saveBk3d = false;
    }
    else IFCMD(-delmesh)
    {
        bk3dlibNeeded = true; // removing a mesh requires reorganizing the baked file
        cmd.cmdType = delmesh;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-delpg)
    {
        bk3dlibNeeded = true; // removing a pg requires reorganizing the baked file
        cmd.cmdType = delpg;
        if(argc < 3)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        argc -= 3;
        argv += 3;
        saveBk3d = true;
    }
    else IFCMD(-deltransf)
    {
        bk3dlibNeeded = true; // removing a transformation requires reorganizing the baked file
        cmd.cmdType = deltransf;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-delcv)
    {
        bk3dlibNeeded = true; // removing a curve requires reorganizing the baked file
        cmd.cmdType = delcv;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-delAttr)
    {
        bk3dlibNeeded = true; // removing some attribute array requires reorganizing the baked file
        cmd.cmdType = delAttr;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-delMat)
    {
        bk3dlibNeeded = true; // removing some material array requires reorganizing the baked file
        cmd.cmdType = delMat;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-switchcomp)
    {
        cmd.cmdType = switchcomp;
        if(argc < 2)
            return false;
        // gather XY, ZY, or XZ, XY...
        cmd.names.push_back(std::string(argv[1])); // attribute
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-negcomp)
    {
        cmd.cmdType = negcomp;
        if(argc < 2)
            return false;
        // gather XY, ZY, or XZ, XY...
        cmd.names.push_back(std::string(argv[1])); // attribute
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-oneminus)
    {
        cmd.cmdType = oneminus;
        if(argc < 2)
            return false;
        // gather XY, ZY, or XZ, XY...
        cmd.names.push_back(std::string(argv[1])); // attribute
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-edittransf)
    {
        cmd.cmdType = edittransf;
        if(argc < 5)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        int N = 0;
        if(!strcmp(argv[2], "mat"))
            N = 16;
        if(!strcmp(argv[2], "absmat"))
            N = 16;
        // TODO: the others ... TRANSFCOMP_rotationOrientation etc.
        else N = 3;
        for(int i=0; i<N; i++)
            cmd.vec[i] = (float)atof(argv[3+i]);
        argc -= 2+N;
        argv += 2+N;
        saveBk3d = true;
    }
    else IFCMD(-editmat)
    {
        cmd.cmdType = editmat;
        if(argc < (2+2))
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        int N = 0;
        if(    (!strcmp(argv[2], "diffusetex"))
            || (!strcmp(argv[2], "diffusetex"))
            || (!strcmp(argv[2], "diffusetex"))
            || (!strcmp(argv[2], "specexptex"))
            || (!strcmp(argv[2], "ambienttex"))
            || (!strcmp(argv[2], "reflectivitytex"))
            || (!strcmp(argv[2], "transparencytex"))
            || (!strcmp(argv[2], "transluencytex"))
            || (!strcmp(argv[2], "speculartex")) )
        {
            bk3dlibNeeded = true; // because name change == reallocation of string area...
            cmd.names.push_back(std::string(argv[3]));
            cmd.names.push_back(std::string(argv[4]));
            argc -= 2+2;
            argv += 2+2;
        } else {
            if(argc < (2+4))
                return false;
            for(int i=0; i<4; i++)
                cmd.vec[i] = (float)atof(argv[3+i]);
            argc -= 2+4;
            argv += 2+4;
        }
        saveBk3d = true;
    }
    /*else IFCMD(-hidemesh)
    {
        cmd.cmdType = hidemesh;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }
    else IFCMD(-showmesh)
    {
        cmd.cmdType = showmesh;
        if(argc < 2)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        argc -= 2;
        argv += 2;
        saveBk3d = true;
    }*/
    else IFCMD(-bake)
    {
        bk3dlibNeeded = true; // forces the whole to be baked to the newest version
        argc -= 1;
        argv += 1;
        saveBk3d = true;
        return true;
    }
    else IFCMD(-dumpvtx)// dumpvtx <mesh> <attr>
    {
        cmd.cmdType = dumpvtx;
        if(argc < 3)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        argc -= 3;
        argv += 3;
        saveBk3d = false;
        verbose = false;
    }
    else IFCMD(-dumpidx)// dumpidx <mesh> <pg>
    {
        cmd.cmdType = dumpidx;
        if(argc < 3)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        argc -= 3;
        argv += 3;
        saveBk3d = false;
        verbose = false;
    }
    else IFCMD(-setvtx)// dumpvtx <mesh> <attr> <file of vtx>
    {
        cmd.cmdType = setvtx;
        if(argc < 4)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        cmd.names.push_back(std::string(argv[3]));
        argc -= 4;
        argv += 4;
        saveBk3d = true;
        verbose = true;
    }
    else IFCMD(-setidx)// dumpidx <mesh> <pg> <file of idx>
    {
        cmd.cmdType = setidx;
        if(argc < 4)
            return false;
        cmd.names.push_back(std::string(argv[1]));
        cmd.names.push_back(std::string(argv[2]));
        cmd.names.push_back(std::string(argv[3]));
        argc -= 4;
        argv += 4;
        saveBk3d = true;
        verbose = true;
    }
    else
    {
        if(argc > 1)
        {
            fprintf(stderr, "Error : command %s not recognized\n", argv[0]);
            exit(1);
        }
        return false;
    }
    cmdList.push_back(cmd);
    return true;
}

//
// Note: if we edit a transformation that only has TRANSFCOMP_matrix or TRANSFCOMP_absmatrix, 
// we should try to find back the separate components from the matrix before changing specific components
// otherwise we will break the original transformation matrix
// TODO : ...
//
bool editMaterial(const Cmd &cmd)
{
    if(bk3dlibNeeded)
    {
        bk3dlib::PMaterial pMat = fileHeader->GetMaterial(cmd.names[0].c_str());
        if(pMat == NULL)
        {
            fprintf(stderr, "failed to find Material %s\n", cmd.names[0].c_str());
            return false;
        }
        if(cmd.names[1] == std::string("name"))
        {
            if(verbose) printf("Setting name of %s to %s\n", cmd.names[0].c_str(), cmd.names[1].c_str());
            pMat->SetName(cmd.names[1].c_str());
        }
        if(cmd.names[1] == std::string("diffuse"))
        {
            if(verbose) printf("Setting diffuse of %s to %f %f %f\n", cmd.names[0].c_str(), cmd.vec[0], cmd.vec[1], cmd.vec[2]);
            pMat->setDiffuse(cmd.vec[0], cmd.vec[1], cmd.vec[2]);
        }
        else if(cmd.names[1] == std::string("specular"))
        {
            if(verbose) printf("Setting specular of %s to %f %f %f\n", cmd.names[0].c_str(), cmd.vec[0], cmd.vec[1], cmd.vec[2]);
            pMat->setSpecular(cmd.vec[0], cmd.vec[1], cmd.vec[2]);
        }
        else if(cmd.names[1] == std::string("diffusetex"))
        {
            if(verbose) printf("Setting diffuse Texture of %s to %s / %s\n", cmd.names[0].c_str(), cmd.names[1].c_str(), cmd.names[2].c_str());
            pMat->setDiffuseTexture(cmd.names[2].c_str(), cmd.names[3].c_str());
        }
        else if(cmd.names[1] == std::string("speculartex"))
        {
            if(verbose) printf("Setting specular Texture of %s to %s / %s\n", cmd.names[0].c_str(), cmd.names[1].c_str(), cmd.names[2].c_str());
            pMat->setSpecularTexture(cmd.names[2].c_str(), cmd.names[3].c_str());
        }
        //TODO: do the rest...
        else
            fprintf(stderr, "Error : material item %s not supported\n", cmd.names[1].c_str() );
    } else {
        bk3d::Material* p;
        if(bk3dHeader->pMaterials)
        {
            int i = 0;
            for(int i=0; i< bk3dHeader->pMaterials->nMaterials; i++)
            {
                p = bk3dHeader->pMaterials->pMaterials[i];
                if(p && (cmd.names[0] == std::string(p->name)) )
                {
                    if(cmd.names[1] == std::string("name"))
                    {
                        if(verbose) printf("Setting name of %s to %s\n", cmd.names[0].c_str(), cmd.names[1].c_str());
                        strncpy(p->name, cmd.names[1].c_str(), NODENAMESZ);
                    }
                    else if(cmd.names[1] == std::string("diffuse"))
                    {
                        if(verbose) printf("Setting diffuse of %s to %f %f %f\n", cmd.names[0].c_str(), cmd.vec[0], cmd.vec[1], cmd.vec[2]);
                        memcpy(p->Diffuse(), cmd.vec, sizeof(float)*3);
                    }
                    else if(cmd.names[1] == std::string("specular"))
                    {
                        if(verbose) printf("Setting specular of %s to %f %f %f\n", cmd.names[0].c_str(), cmd.vec[0], cmd.vec[1], cmd.vec[2]);
                        memcpy(p->Specular(), cmd.vec, sizeof(float)*3);
                    }
                    //TODO: do the rest...
                    else
                        fprintf(stderr, "Error : material item %s not supported\n", cmd.names[1].c_str() );
                }
            }
        }
        else
            fprintf(stderr, "Error : no material in this file\n");
    }
    return false;
}
bool editTransf(const Cmd &cmd)
{
    if(bk3dlibNeeded)
    {
        bk3dlib::PBone pT = fileHeader->GetTransform(cmd.names[0].c_str());
        if(pT == NULL)
        {
            fprintf(stderr, "failed to find Transform %s\n", cmd.names[0].c_str());
            return false;
        }
        if(cmd.names[1] == std::string("name"))
        {
            if(verbose) printf("Setting name of %s to %s\n", cmd.names[0].c_str(), cmd.names[1].c_str());
            pT->SetName(cmd.names[1].c_str());
        }
        else if(cmd.names[1] == std::string("pos")) {
            pT->SetPos(cmd.vec[0], cmd.vec[1], cmd.vec[2]);
        } else if(cmd.names[1] == std::string("scale")) {
            bk3dlib::PTransformSimple pTS = pT->AsTransfSimple();
            if(pTS) pTS->SetScale(cmd.vec[0], cmd.vec[1], cmd.vec[2]);
        } else if(cmd.names[1] == std::string("rot")) {
            bk3dlib::PTransform pTS = pT->AsTransf();
            if(pTS) pTS->SetRotation(cmd.vec[0], cmd.vec[1], cmd.vec[2]);
        //TODO: write the rest
        } else
            fprintf(stderr, "unknown field : %s\n", cmd.names[2].c_str());
    } else
    {
        bk3d::Bone* pT;
        if(bk3dHeader->pTransforms)
        {
            int i = 0;
            for(int i=0; i< bk3dHeader->pTransforms->nBones; i++)
            {
                pT = bk3dHeader->pTransforms->pBones[i];
                if(pT && (cmd.names[0] == std::string(pT->name)) )
                {
                    if(cmd.names[1] == std::string("name"))
                    {
                        if(verbose) printf("Setting name of %s to %s\n", cmd.names[0].c_str(), cmd.names[1].c_str());
                        strncpy(pT->name, cmd.names[1].c_str(), NODENAMESZ);
                    }
                    else if(cmd.names[1] == std::string("pos"))
                    {
                        if((TRANSFCOMP_pos & pT->ValidComps()) == 0)
                            fprintf(stderr, "Warning : pos in transform %s was not used before...\n", pT->name);
                        pT->ValidComps() |= TRANSFCOMP_pos;
                        pT->setDirty(true);
                        memcpy(pT->Pos(), cmd.vec, sizeof(float)*3);
                    }
                    else if(cmd.names[1] == std::string("scale"))
                    {
                        if((pT->nodeType == NODE_TRANSFORM)||(pT->nodeType == NODE_TRANSFORMSIMPLE))
                        {
                            if((TRANSFCOMP_scale & pT->ValidComps()) == 0)
                                fprintf(stderr, "Warning : scale in transform %s was not used before...\n", pT->name);
                            pT->ValidComps() |= TRANSFCOMP_scale;
                            pT->setDirty(true);
                            memcpy(pT->asTransfSimple()->scale, cmd.vec, sizeof(float)*3);
                        }
                    }
                    else if(cmd.names[1] == std::string("rot"))
                    {
                        if(pT->nodeType == NODE_TRANSFORM)
                        {
                            if((TRANSFCOMP_rotation & pT->ValidComps()) == 0)
                                fprintf(stderr, "Warning : rotation in transform %s was not used before...\n", pT->name);
                            pT->ValidComps() |= TRANSFCOMP_rotation;
                            pT->setDirty(true);
                            memcpy(pT->asTransf()->Rotation(), cmd.vec, sizeof(float)*3);
                        }
                        // TODO: turn Euler rot as a quaternion and write it
                        else if(pT->nodeType == NODE_TRANSFORMSIMPLE)
                        {
                            DebugBreak();
                        }
                    }
                    // TODO: do the rest
                    else
                    {
                        fprintf(stderr, "transformation component %s doesn't exist\n", cmd.names[1].c_str());
                        return false;
                    }
                    break;
                }
            }
            if(i == bk3dHeader->pTransforms->nBones)
                fprintf(stderr, "transformation %s not found\n", cmd.names[0]);
        }
    }
    return false;
}

bk3d::Mesh *findbk3dMesh(const std::string &name)
{
    if(!bk3dHeader->pMeshes)
        return NULL;
    for(int i=0; i<bk3dHeader->pMeshes->n; i++)
    {
        bk3d::Mesh *pM = bk3dHeader->pMeshes->p[i];
        if(name == std::string(pM->name))
        {
            return pM;
        }
    }
    return NULL;
}
bk3d::Attribute* findbk3dAttribute(bk3d::Mesh *pM, std::string name)
{
    if(!pM->pAttributes)
        return NULL;
    for(int i=0; i<pM->pAttributes->n; i++)
    {
        bk3d::Attribute *pA = pM->pAttributes->p[i];
        if(name == std::string(pA->name))
        {
            return pA;
        }
    }
    return NULL;
}

void dumpVtx(const Cmd &cmd)
{
    if(asobj && (curprefix > 3))
    {
        fprintf(stderr, "Error : in OBJ mode, we can't output more than 'v', 'vt' and 'vn'... sorry\n");
        return;
    }
    if(bk3dlibNeeded)
    {
        assert(bk3dlibNeeded);
        bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
        if(pMesh == NULL)
        {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
            return;// false;
        }
        int N = pMesh->GetNumVtxBuffers(false);
        for(int i=0; i<N; i++)
        {
            bk3dlib::PBuffer vb = pMesh->GetVtxBuffer(i);
            if(cmd.names[1] == std::string(vb->GetName()) )
            {
                int nc = vb->GetNumComps();
                int ni = vb->GetNumItems();
                bk3dlib::DataType t = vb->GetDataType();
                for(int j=0; j<ni; j++)
                {
                    float v[4];
                    unsigned int   vi[4];
                    if(asobj)
                        fprintf(objfd, vtxobjprefix[curprefix]);
                    switch(t) {
                    case bk3dlib::FLOAT32:
                        vb->GetData(v, j*nc, nc);
                        for(int c=0; c<nc; c++)
                            fprintf(objfd, "%f ", v[c]);
                        fprintf(objfd, "\n");
                        break;
                    //case FLOAT16:// Format NOT IMPLEMENTED YET
                    case bk3dlib::UINT32:
                    case bk3dlib::UINT16:
                    case bk3dlib::UINT8:
                        vb->GetData(vi, j*nc, 1);
                        for(int c=0; c<nc; c++)
                            fprintf(objfd, "%d ", v[c]);
                        fprintf(objfd, "\n");
                        break;
                    default:
                        break;
                    }
                }
                curprefix++;
                return;
            }
        }
    } else {
        bk3d::Mesh* pM = findbk3dMesh(cmd.names[0]);
        if(pM)
        {
            bk3d::Attribute* pA = findbk3dAttribute(pM, cmd.names[1]);
            if(pA)
            {
                int nc = pA->numComp;
                int ni = pM->pSlots->p[0]->vertexCount; // all the slots should have the same #
                for(int i=0; i<ni; i++)
                {
                    float *pf = (float *)((char*)pA->pAttributeBufferData + (i*pA->strideBytes));
                    int *pi = (int *)((char*)pA->pAttributeBufferData + (i*pA->strideBytes));
                    if(asobj)
                        fprintf(objfd, vtxobjprefix[curprefix]);
                    for(int j=0; j<nc; j++)
                    {
                        switch(pA->formatGL)
                        {
                        case GL_FLOAT:
                            fprintf(objfd, "%f ", pf[j]);
                            break;
                        default:
                        case GL_INT:
                            fprintf(objfd, "%d ", pi[j]);
                            break;
                        }
                    }
                    fprintf(objfd, "\n");
                }
                curprefix++;
                return;
            }
        } else {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str() );
            return;
        }
    }
    fprintf(stderr, "failed to find buffer %s\n", cmd.names[1].c_str() );
}
void dumpIdx(const Cmd &cmd)
{
    if(asobj)
    {
        fprintf(objfd, "g %s\n", cmd.names[1].c_str());
        fprintf(objfd, "s 1\n");
    }
    if(bk3dlibNeeded)
    {
        assert(bk3dlibNeeded);
        bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
        if(pMesh == NULL)
        {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
            return;// false;
        }
        for(int i=0; i<pMesh->GetNumPrimGroups(); i++)
        {
            bk3dlib::PrimGroup pginfo;
            pMesh->GetPrimGroupInfo(i, pginfo);
            if(cmd.names[1] == std::string(pginfo.name) )
            {
                int nElmtsPerFace;
                switch(pginfo.topo)
                {
                case bk3dlib::TRIANGLES:
                    nElmtsPerFace = 3;
                    break;
                case bk3dlib::QUADS:
                    nElmtsPerFace = 4;
                    break;
                case bk3dlib::LINES:
                    nElmtsPerFace = 2;
                    break;
                default:
                    nElmtsPerFace = 3;
                    fprintf(stderr, "Error : topology not supported for obj creation, yet...\n");
                    if(asobj)
                        fprintf(objfd, "Error : topology not supported for obj creation, yet...\n");
                    break;
                }
                for(unsigned int j=0; j<pginfo.numElements; j++)
                {
                    if(asobj && ((j % nElmtsPerFace)==0))
                        fprintf(objfd, "\nf ");
                    if(!asobj && (pginfo.numIdxBuffers > 1))
                        printf("(");
                    if(asobj && (pginfo.numIdxBuffers > 1))
                    {
                        // TODO
                        fprintf(stderr, "Error : multi-idx for OBJ format is not ready, yet !!\n");
                        pginfo.numIdxBuffers = 1;
                    }
                    for(int k=0; k<pginfo.numIdxBuffers; k++)
                    {
                        bk3dlib::PBuffer ib = pginfo.idxBuffers[k];
                        unsigned int idx;
                        ib->GetData(&idx, j, 1);
                        if(asobj)
                            for(int l=0; l<curprefix-1; l++)
                                fprintf(objfd, "%d/", idx + (asobj ? 1 : 0));
                        fprintf(objfd, "%d ", idx + (asobj ? 1 : 0));
                    }
                    if(!asobj && (pginfo.numIdxBuffers > 1))
                        printf(") ");
                }
                if(asobj)
                    fprintf(objfd, "\n");
                return;
            }
        }
    } else {
        bk3d::Mesh* pM = findbk3dMesh(cmd.names[0]);
        if(pM)
        {
            if(pM->pPrimGroups)
                for(int i=0; i<pM->pPrimGroups->n; i++)
                {
                    bk3d::PrimGroup* pg = pM->pPrimGroups->p[i];
                    if(cmd.names[1] == std::string(pg->name) )
                    {
                        int nElmtsPerFace = pg->indexCount / pg->primitiveCount;
                        for(unsigned int j=0; j<pg->indexCount; j++)
                        {
                            if(asobj && ((j % nElmtsPerFace)==0))
                                fprintf(objfd, "\nf");
                            fprintf(objfd, " ");
                            // TODO: handle multi-index
                            // Note : not so trivial : multi index corresponds to various attributes
                            // so these multiple indices are tight to some vtx buffers.
                            // those must match when exporting data... to be continued
                            if(pg->indexPerVertex > 1)
                            {
                                fprintf(stderr, "multiple indices not supported, yet !!\n");
                            }
                            if(!asobj && (pg->indexPerVertex > 1))
                                printf("(");
                            switch(pg->indexFormatGL)
                            {
                            case GL_UNSIGNED_INT:
                                if(asobj)
                                    for(int l=0; l<curprefix-1; l++)
                                        fprintf(objfd, "%d/", ((unsigned int*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                fprintf(objfd, "%d ", ((unsigned int*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                break;
                            case GL_UNSIGNED_SHORT:
                                if(asobj)
                                    for(int l=0; l<curprefix-1; l++)
                                        fprintf(objfd, "%d/", ((unsigned short*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                fprintf(objfd, "%d ", ((unsigned short*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                break;
                            case GL_UNSIGNED_BYTE:
                                if(asobj)
                                    for(int l=0; l<curprefix-1; l++)
                                        fprintf(objfd, "%d/", ((unsigned char*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                fprintf(objfd, "%d ", ((unsigned char*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                break;
                            case GL_FLOAT:
                                if(asobj)
                                    for(int l=0; l<curprefix-1; l++)
                                        fprintf(objfd, "%f/", ((float*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                fprintf(objfd, "%f ", ((float*)pg->pIndexBufferData)[j+pg->indexOffset] + (asobj ? 1 : 0));
                                break;
                            default:
                                fprintf(objfd, "Err ");
                                break;
                            }
                            if(!asobj && (pg->indexPerVertex > 1))
                                printf("(");
                        }
                        if(asobj)
                            fprintf(objfd, "\n");
                        return;
                    }
                }
        } else {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str() );
            return;
        }
    }
    fprintf(stderr, "failed to find buffer %s\n", cmd.names[1].c_str() );
}

void setVtx(const Cmd &cmd)
{
    FILE *fin = stdin;
    if(cmd.names[2] != std::string("-"))
        fin = fopen(cmd.names[2].c_str(), "r");
    if(!fin)
    {
        fprintf(stderr, "Error : could not load file %s for vertex\n", cmd.names[2].c_str());
        return;
    }
    if(verbose) printf("reading vtx data for %s.%s from %s\n", cmd.names[0].c_str(), cmd.names[1].c_str(), cmd.names[2].c_str());
    if(bk3dlibNeeded)
    {
        assert(bk3dlibNeeded);
        bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
        if(pMesh == NULL)
        {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
            goto endsetVtx;
        }
        int N = pMesh->GetNumVtxBuffers(false);
        for(int i=0; i<N; i++)
        {
            bk3dlib::PBuffer vb = pMesh->GetVtxBuffer(i);
            if(cmd.names[1] == std::string(vb->GetName()) )
            {
                if(verbose) printf("editing the vertices of buffer %s of Mesh %s...\n", vb->GetName(), pMesh->GetName());
                int nc = vb->GetNumComps();
                int ni = vb->GetNumItems();
                bk3dlib::DataType t = vb->GetDataType();
                for(int j=0; j<ni; j++)
                {
                    float v[4];
                    unsigned int   vi[4];
                    switch(t) {
                    case bk3dlib::FLOAT32:
                        for(int c=0; c<nc; c++)
                        {
                            int res = fscanf(fin, "%f ", v+c);
                            if(res == EOF)
                            {
                                if(verbose)
                                {
                                    printf("\nReached the end of input data\n");
                                    printf("Warning : less data sent to the mesh (%d < %d)\n", j*nc + c, nc*ni);
                                }
                                goto endsetVtx;
                            }
                            bool bRes = vb->SetData(v+c, j*nc + c, 1);
                        }
                        break;
                    //case FLOAT16:// Format NOT IMPLEMENTED YET
                    case bk3dlib::UINT32:
                    case bk3dlib::UINT16:
                    case bk3dlib::UINT8:
                        for(int c=0; c<nc; c++)
                        {
                            int res = fscanf(fin, "%d ", vi+c);
                            if(res == EOF)
                            {
                                if(verbose)
                                {
                                    printf("\nReached the end of input data\n");
                                    printf("Warning : less data sent to the mesh (%d < %d)\n", j*nc + c, nc*ni-1);
                                }
                                goto endsetVtx;
                            }
                            bool bRes = vb->SetData(vi+c, j*nc + c, 1);
                        }
                        break;
                    default:
                        break;
                    }
                }
                if(verbose) printf("reached the end of the original buffer\n");
                // TODO: we could allow to add more than the original size...
                goto endsetVtx;
            }
        }
    } else {
        bk3d::Mesh* pM = findbk3dMesh(cmd.names[0]);
        if(pM)
        {
            bk3d::Attribute* pA = findbk3dAttribute(pM, cmd.names[1]);
            if(pA)
            {
                int nc = pA->numComp;
                int ni = pM->pSlots->p[0]->vertexCount; // all the slots should have the same #
                for(int i=0; i<ni; i++)
                {
                    float *pf = (float *)((char*)pA->pAttributeBufferData + (i*pA->strideBytes));
                    int *pi = (int *)((char*)pA->pAttributeBufferData + (i*pA->strideBytes));
                    for(int j=0; j<nc; j++)
                    {
                        int res;
                        switch(pA->formatGL)
                        {
                        case GL_FLOAT:
                            res = fscanf(fin, "%f ", pf+j);
                            break;
                        default:
                        case GL_INT:
                            res = fscanf(fin, "%d ", pi+j);
                            break;
                        }
                        if(res == EOF)
                        {
                            if(verbose)
                            {
                                printf("\nReached the end of input data\n");
                                printf("Warning : less data sent to the mesh (%d < %d)\n", i*nc + j, nc*ni);
                            }
                            goto endsetVtx;
                        }
                    }
                }
                if(verbose) printf("reached the end of the original buffer\n");
                goto endsetVtx;
            }
        } else {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str() );
            goto endsetVtx;
        }
    }
    fprintf(stderr, "failed to find buffer %s\n", cmd.names[1].c_str() );
endsetVtx:
    if(cmd.names[2] != std::string("-"))
        fclose(fin);
}
void setIdx(const Cmd &cmd)
{
    FILE *fin = stdin;
    if(cmd.names[2] != std::string("-"))
        fin = fopen(cmd.names[2].c_str(), "r");
    if(!fin)
    {
        fprintf(stderr, "Error : could not load file %s for vertex\n", cmd.names[2].c_str());
        return;
    }
    if(verbose) printf("reading idx data for %s.%s from %s\n", cmd.names[0].c_str(), cmd.names[1].c_str(), cmd.names[2].c_str());
    if(bk3dlibNeeded)
    {
        assert(bk3dlibNeeded);
        bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
        if(pMesh == NULL)
        {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
            goto endsetIdx;// false;
        }
        for(int i=0; i<pMesh->GetNumPrimGroups(); i++)
        {
            bk3dlib::PrimGroup pginfo;
            pMesh->GetPrimGroupInfo(i, pginfo);
            if(cmd.names[1] == std::string(pginfo.name) )
            {
                for(unsigned int j=0; j<pginfo.numElements; j++)
                {
                    if(pginfo.numIdxBuffers > 1)
                    {
                        int res = fscanf(fin, "(");
                    }
                    for(int k=0; k<pginfo.numIdxBuffers; k++)
                    {
                        bk3dlib::PBuffer ib = pginfo.idxBuffers[k];
                        unsigned int idx;
                        int res = fscanf(fin, "%d ", &idx);
                        if(res == EOF)
                        {
                            if(verbose)
                            {
                                printf("\nReached the end of input data\n");
                                printf("Warning : less data sent to the mesh (%d < %d)\n", j, pginfo.numElements-1);
                            }
                            goto endsetIdx;
                        }
                        bool bRes = ib->SetData(&idx, j, 1);
                    }
                    if(pginfo.numIdxBuffers > 1)
                    {
                        int res = fscanf(fin, ")");
                    }
                }
                goto endsetIdx;
            }
        }
    } else {
        bk3d::Mesh* pM = findbk3dMesh(cmd.names[0]);
        if(pM)
        {
            if(pM->pPrimGroups)
                for(int i=0; i<pM->pPrimGroups->n; i++)
                {
                    bk3d::PrimGroup* pg = pM->pPrimGroups->p[i];
                    if(cmd.names[1] == std::string(pg->name) )
                    {
                        for(unsigned int j=0; j<pg->indexCount; j++)
                        {
                            int res;
                            if(pg->indexPerVertex > 1)
                                fscanf(fin, "(");
                            switch(pg->indexFormatGL)
                            {
                            case GL_UNSIGNED_INT:
                                res = fscanf(fin, "%d ", (unsigned int*)pg->pIndexBufferData+j+pg->indexOffset);
                                break;
                            case GL_UNSIGNED_SHORT:
                                res = fscanf(fin, "%d ", (unsigned short*)pg->pIndexBufferData+j+pg->indexOffset);
                                break;
                            case GL_UNSIGNED_BYTE:
                                res = fscanf(fin, "%d ", (unsigned char*)pg->pIndexBufferData+j+pg->indexOffset);
                                break;
                            case GL_FLOAT:
                                res = fscanf(fin, "%d ", (float*)pg->pIndexBufferData+j+pg->indexOffset);
                                break;
                            default:
                                printf("Err ");
                                break;
                            }
                            if(res == EOF)
                            {
                                if(verbose)
                                {
                                    printf("\nReached the end of input data\n");
                                    printf("Warning : less data sent to the mesh (%d < %d)\n", j, pg->indexCount-1);
                                }
                                goto endsetIdx;
                            }
                            if(pg->indexPerVertex > 1)
                                fscanf(fin, ")");
                        }
                        goto endsetIdx;
                    }
                }
        } else {
            fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str() );
            goto endsetIdx;
        }
    }
    fprintf(stderr, "failed to find buffer %s\n", cmd.names[1].c_str() );
endsetIdx:
    if(cmd.names[2] != std::string("-"))
        fclose(fin);
}

//
// Execute the commands from the cmd line
//
bool executeCmd(const Cmd &cmd)
{
    switch(cmd.cmdType)
    {
    case print:
        if(bk3dHeader)
        {
            if(cmd.names[0] == std::string("all") )
                FileHeader_debugDumpAll(bk3dHeader, 1, NULL);
            else if(cmd.names[0] == std::string("ALL") )
                FileHeader_debugDumpAll(bk3dHeader, 2, NULL);
            else if(cmd.names[0] == std::string("brief") )
                FileHeader_debugDumpAll(bk3dHeader, 0, NULL);
            else
                FileHeader_debugDumpAll(bk3dHeader, 1, cmd.names[0].c_str() );
        }
        break;
    case delmesh:// "meshname" -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
            bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
            if(pMesh == NULL)
            {
                fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
                return false;
            }
            if(verbose) printf("Deleting Mesh %s...\n", cmd.names[0].c_str());
            pMesh->Destroy();
        }
        break;
    case delpg:// "meshname" "pgname"  -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
            bk3dlib::PMesh pMesh = fileHeader->FindMesh(cmd.names[0].c_str());
            if(pMesh == NULL)
            {
                fprintf(stderr, "failed to find Mesh %s\n", cmd.names[0].c_str());
                return false;
            }
            bool bRes = pMesh->DeletePrimGroupFromName(cmd.names[1].c_str());
            if(bRes)
                if(verbose) printf("Deleting Primitive Group %s...\n", cmd.names[1].c_str());
            else
                fprintf(stderr, "could not find/Delete Primitive Group %s...\n", cmd.names[1].c_str());
        }
        break;
    case deltransf:// "transfName"  -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
            bk3dlib::PBone pTransf = fileHeader->GetTransform(cmd.names[0].c_str());
            if(pTransf == NULL)
            {
                fprintf(stderr, "failed to find Transformation %s\n", cmd.names[0].c_str());
                return false;
            }
            if(verbose) printf("Deleting Transformation %s...\n", cmd.names[0].c_str());
            pTransf->Destroy();
        }
        break;
    case delcv:// "cvName"  -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
            bk3dlib::PCurveVec pCv = fileHeader->GetCurveVec(cmd.names[0].c_str());
            if(pCv == NULL)
            {
                fprintf(stderr, "failed to find Curve %s\n", cmd.names[0].c_str());
                return false;
            }
            if(verbose) printf("Deleting Curve %s...\n", cmd.names[0].c_str());
            pCv->Destroy();
        }
        break;
    case delAttr:// "[meshName.]attrName"  -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
        }
        break;
    case delMat:// "matName"  -o <ofile> <infile>
        {
            assert(bk3dlibNeeded);
            bk3dlib::PMaterial pMat = fileHeader->GetMaterial(cmd.names[0].c_str());
            if(pMat == NULL)
            {
                fprintf(stderr, "failed to find Material %s\n", cmd.names[0].c_str());
                return false;
            }
            if(verbose) printf("Deleting Material %s...\n", cmd.names[0].c_str());
            pMat->Destroy();
        }
        break;
    case switchcomp:// XZ "attrName" "otherAttr" ...;
        {
            assert(!"TODO");
        }
        break;
    case negcomp:// [X][Y][Z] "attrName" "otherAttr" ...;
        {
            assert(!"TODO");
        }
        break;
    case oneminus:// [X][Y][Z] "attrName" "otherAttr" ...;
        {
            assert(!"TODO");
        }
        break;
    case edittransf:// "transfName" pos|scale|rot <xval> <yval> <zval> <wval> OR "transfName" [abs]mat <v00> <v01> <v02> ... <v33>
        editTransf(cmd);
        break;
    case editmat:// "matName" diff|spec|... <xval> <yval> <zval> <wval>
        editMaterial(cmd);
        break;
    case hidemesh:// "meshname"  -o <ofile> <infile>
        {
            assert(!"TODO");
        }
        break;
    case showmesh:// "meshname" -o <ofile> <infile>
        {
            assert(!"TODO");
        }
        break;
    case dumpvtx:// dumpvtx <mesh> <attr>
        dumpVtx(cmd);
        break;
    case dumpidx:// dumpidx <mesh> <pg>
        dumpIdx(cmd);
        break;
    case setvtx:// setvtx <mesh> <attr> <file of vtx>
        setVtx(cmd);
        break;
    case setidx:// setidx <mesh> <pg> <file of idx>
        setIdx(cmd);
        break;
    }
    return true;
}

int main(int argc, char* argv[])
{
	bool bRes;
	if(argc < 2)
	{
		printHelp();
		return 1;
	}
	char* str = NULL;

    // skip the exe name
    argc--;
    argv++;
    bool bCont;
    do {
        bCont = parseCmd(argc, argv);
    } while(bCont);
    //
    // Check for the inputname
    //
    if(argc < 1)
	{
		fprintf(stderr, "no source file\n");
		return 1;
	}
    fullInStr = std::string(argv[0]);
    singleName = targetName;
    char* fullOutStr;
    if(targetName.empty())
    {
        fullOutStr = new char[fullInStr.size()+20];
        int offs = (int)fullInStr.find(".bk3d");
        if(offs < 0)
            offs = (int)fullInStr.find(".BK3D");
        std::string s = fullInStr.substr(0, offs);
	    sprintf(fullOutStr, "%s_v%x.bk3d", s.c_str(), bk3dlib::bk3dVersion() );
    } else {
        fullOutStr = new char[targetName.size()+1];
	    sprintf(fullOutStr, "%s", targetName.c_str() );
    }
    if(!bk3dlibNeeded)
    {
        // trivial case where we only load the binaries and poke into
        bk3dHeader = bk3d::load(fullInStr.c_str(), &bufferMemory, &bufferMemorySz);
        if(!bk3dHeader)
        {
            bk3dlibNeeded = true;
            saveBk3d = true;
            if(verbose) printf("Warning : %s may be an old version. Forcing to bake it to newer version\n", fullInStr.c_str());
        }
    }

    if(bk3dlibNeeded)
    {
        // complex case where we rebuild internal structures for re-organizing data and later bake them again
	    fileHeader = bk3dlib::FileHeader::Create(singleName.c_str());
	    if(!readOBJ(fullInStr.c_str(), singleName.c_str(), fileHeader))
	    {
            if(!readPMD(fullInStr.c_str(), singleName.c_str(), fileHeader))
	        {
		        if(verbose) printf( "reading %s...\n", fullInStr.c_str());
		        if(!fileHeader->LoadFromBk3dFile(fullInStr.c_str()) )
		        {
			        fprintf(stderr, "Failed loading file %s\n", fullInStr.c_str());
			        return 1;
		        }
            }
	    }
    }
    //
    // header for obj file if requested
    //
    if(objfd)
        fprintf(objfd, "# bk3d Util : from file %s\n", fullInStr.c_str());
    //
    // Execute commands
    //
//printf("WAiting 10s\n");
//Sleep(20000);
    std::vector<Cmd>::const_iterator iCmd = cmdList.begin();
    while(iCmd != cmdList.end())
    {
        /*bool bRes = */executeCmd(*iCmd);
        ++iCmd;
    }

    // finalize the obj file if needed
    if(objfd)
        fclose(objfd);

     if(saveBk3d)
    {
        if(bk3dlibNeeded)
        {
	        //
	        // cook the whole and save the result
	        //
	        unsigned int sz;
	        if(verbose) printf( "re-compute the BBoxes...\n");
            sz = fileHeader->GetNumMeshes();
            bk3dlib::Mesh *mesh = fileHeader->GetFirstMesh();
            while(mesh)
            {
                mesh->ComputeBoundingVolumes(mesh->GetVtxBuffer(0));
                mesh = fileHeader->GetNextMesh();
            }
	        if(verbose) printf( "baking...\n");
            bk3dHeader = (bk3d::FileHeader *)fileHeader->Cook(fullOutStr, NULL, &sz);
	        if(bk3dHeader == NULL)
            {
                fprintf(stderr, "Error : something terrible happened during cooking time\n");
                return 1;
            } else {
                bk3d::FileHeader_debugDumpAll(bk3dHeader, 0, NULL);
	            if(verbose) printf( "saving %s...\n", fullOutStr);
	            bRes = fileHeader->Save(fullOutStr);
                if(verbose)
                {
	                if(bRes)	printf( "done\n");
	                else		printf( "FAILED\n");
                }
            }
        } else {
            // we must recompute matrices if ever any of them got modified...
            extern void updateTransforms(bk3d::FileHeader * bk3dHeader, bool bComputeBindPose, bool forceDirty);
            updateTransforms(bk3dHeader, true, true);
            //
            // Case where we only need to re-compute the pointers to offsets and then
            // save the binary chunk that we loaded.
            // Note : we could also use the memory mapping (mmap equivalent)...
            //
            // then convert ptrs to offsets
            bk3dHeader->restorePointerOffsets(bufferMemory);
            //
            // Write to file
            //
            int i;
            FILE *f;
            fopen_s(&f, fullOutStr, "wb");
            if(verbose) printf("creating file %s (%d bytes)...\n", fullOutStr, bk3dHeader->nodeByteSize+bufferMemorySz);
            if(!f)
            {
                fprintf(stderr, "Could not open Vertex binary file\n");
                return false;
            }
            i = (int)fwrite(bk3dHeader, bk3dHeader->nodeByteSize, 1, f);
            if(i < 1)
            {
                fprintf(stderr,"Error while writing to the file\n");
                return false;
            }
            i = (int)fwrite(bufferMemory, bufferMemorySz, 1, f);
            if(i < 1)
            {
                fprintf(stderr, "Error while writing to the file\n");
                return false;
            }
            fclose(f);
        }
    }
	delete [] fullOutStr;
	if(str)
        delete [] str;
	return 0;
}

