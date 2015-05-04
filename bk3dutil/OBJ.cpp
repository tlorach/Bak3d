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
#include "bk3dEx.h"
#include "nvModel.h"
#include "nv_math.h"
using namespace nv_math;

extern bool verbose;

//------------------------------------------------------------------------------
//
// read an OBJ file and make it bk3d
//
//------------------------------------------------------------------------------
bool readOBJ(const char* fullInStr, const char* singleName, bk3dlib::PFileHeader fileHeader)
{
	bool bRes;
	nv::Model *objModel = nv::Model::Create();
	if(verbose) printf( "loading obj file...\n");
	bRes = objModel->loadModelFromFile(fullInStr);
	if(!bRes)
	{
		return false;
	}
	if(verbose) printf( "converting...\n");

    bk3dlib::PMesh mesh = bk3dlib::Mesh::Create(singleName);
    fileHeader->AttachMesh(mesh);

    int matcount = objModel->getMaterialCount();
	if(matcount == 0)
		matcount = 1;
    struct Material
    {
        const char* name;
        int offset;
        bk3dlib::PBuffer bufferIdxVtx;
        bk3dlib::PBuffer bufferIdxNormals;
        bk3dlib::PBuffer bufferIdx;
        bk3dlib::PBuffer bufferVtx2;
        bk3dlib::PBuffer bufferNormals2;
    };
    Material *pMat = new Material[matcount];
    for(int m=0; m<matcount; m++)
    {
        if(!objModel->getMaterial(m, &pMat[m].offset, &pMat[m].name))
		{
			pMat[m].name = "None";
		}
        pMat[m].bufferIdxVtx = bk3dlib::Buffer::CreateIdxBuffer("idxvtx");
        mesh->AttachIndexBuffer(pMat[m].bufferIdxVtx);
        pMat[m].bufferIdxNormals = bk3dlib::Buffer::CreateIdxBuffer("idxnorm");
        mesh->AttachIndexBuffer(pMat[m].bufferIdxNormals);
	    //
	    // Final Buffers using a single index table
	    //
        pMat[m].bufferIdx = bk3dlib::Buffer::CreateIdxBuffer("primitiveGroup0");
        mesh->AttachIndexBuffer(pMat[m].bufferIdx);
    }
    bk3dlib::PBuffer bufferVtx = bk3dlib::Buffer::CreateVtxBuffer("position", 3);
    mesh->AttachVtxBuffer(bufferVtx);
    bk3dlib::PBuffer bufferNormals = bk3dlib::Buffer::CreateVtxBuffer("normal", 3);
    mesh->AttachVtxBuffer(bufferNormals);
	//
	// Final Buffers using a single index table
	//
    bk3dlib::PBuffer bufferVtx2 = bk3dlib::Buffer::CreateVtxBuffer("position", 3, 0); // Slot #0
    mesh->AttachVtxBuffer(bufferVtx2);
	bk3dlib::PBuffer bufferNormals2 = bk3dlib::Buffer::CreateVtxBuffer("normal", 3, 0); // Slot #0 so we interleave with Vtx pos
    mesh->AttachVtxBuffer(bufferNormals2);
	//
	// fill buffers. They are still made of separate index tables
	//
	bufferVtx->AddData(objModel->getPositions(), objModel->getPositionCount() * objModel->getPositionSize());
	if(objModel->getNormalCount() == 0)
	{
		// TODO: ask my lib to compute this...
		if(verbose) printf( "Computing normals of the obj file...\n");
		objModel->computeNormals();
	}
	bufferNormals->AddData(objModel->getNormals(), objModel->getNormalCount() * objModel->getNormalSize());

    for(int m=0; m<matcount; m++)
    {
	    pMat[m].bufferIdxVtx->AddData(objModel->getPositionIndices(), objModel->getIndexCount());
	    pMat[m].bufferIdxNormals->AddData(objModel->getNormalIndices(), objModel->getIndexCount());
	    //
	    // Now organize buffers for one single idx buffer
	    //
	    pMat[m].bufferIdx->SIB_ClearBuffers();
	    pMat[m].bufferIdx->SIB_AddBuffers(pMat[m].bufferIdxVtx, bufferVtx, bufferVtx2); // try SBMake_AddBuffers(bufferIdxVtx, bufferVtx)
	    pMat[m].bufferIdx->SIB_AddBuffers(pMat[m].bufferIdxNormals, bufferNormals, bufferNormals2);
    #if 0
	    bRes = bufferIdx->SIB_Compile(true);
    #else
	    if(verbose) printf( "turning into a single index...\n");
	    bRes = pMat[m].bufferIdx->SIB_Compile();
        mesh->DetachBuffer(pMat[m].bufferIdxVtx);
	    pMat[m].bufferIdxVtx->Destroy();
        mesh->DetachBuffer(bufferVtx);
        mesh->DetachBuffer(bufferNormals);
        mesh->DetachBuffer(pMat[m].bufferIdxNormals);
	    pMat[m].bufferIdxNormals->Destroy();
    #endif
    }
	bufferVtx->Destroy();
	bufferNormals->Destroy();

	if(verbose) printf( "computing bounding volumes...\n");
	mesh->ComputeBoundingVolumes(bufferVtx2);
	//
	// Now we're good to go... only one index buffer for all : it will become a "Primitive Group"
	//
    bk3dlib::PMaterial mat = NULL;
#if 1
	if(verbose) printf( "creating materials...\n");
	//
	// We can assign a material...
	//
	mat = bk3dlib::Material::Create("material");
    fileHeader->AttachMaterial(mat);
	mat->setDiffuse(1,1,1);
	mat->setAmbient(0,0,0);
    mat->setSpecular(1,1,1);
    mat->setTransparency(0,0,0);
    mat->setSpecexp(100.0f);
	mat->setShaderName("default", "default");
#endif
	// TODO: how do I know which topology we had in the obj file ??
    for(int m=0; m<matcount; m++)
    {
	    mesh->CreatePrimGroup(pMat[m].name, pMat[m].bufferIdx, bk3dlib::TRIANGLES, mat);
    }
	//
	// TODO: goodies like Tan/Binorm
	// MeshMender...
	//
	//bk3dlib::Buffer::computeNormals(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdxVtx);
	//bk3dlib::Buffer::computeTangents(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdxVtx);
	//bk3dlib::Buffer::computeBiTangents(bufferNormals, bufferIdxNormals, bufferVtx, bufferIdxVtx);

    //
    // Create a transformation
    //
    bk3dlib::TransformSimple* pTransf = bk3dlib::TransformSimple::Create("Base");
    if(fileHeader->AttachTransform(pTransf) )
    {
        pTransf->SetPos(0, 0, 0);
        pTransf->ComputeMatrix(true);
        mesh->AddTransformReference(pTransf);
    }
    delete [] pMat;
    delete objModel;
	return true;
}
//------------------------------------------------------------------------------
bk3d::Attribute *findAttr(const char* name, bk3d::Mesh *pMesh, bool isBS)
{
    bk3d::Attribute *pAttr = NULL;
    bk3d::AttributePool *pAttrPool = pMesh->pAttributes;
    if(isBS)
        pAttrPool = pMesh->pBSAttributes;
    for(int i=0; i<pAttrPool->n; i++)
    {
        pAttr = pAttrPool->p[i];
        if(!strcmp(pAttr->name, name))
            return pAttr;
    }
    return NULL;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vec4f applySkinning( int i, vec4f P, bk3d::Attribute *pAttrSkin2Bones2Weights
                    ,bk3d::Attribute *pAttrSkinBones
                    ,bk3d::Attribute *pAttrSkinWeights
                    ,mat4f *matArray
                    )
{
    float boneWeights[4];
    int   boneOffsets[4];
    int   numbones = 0;
    // TODO: check the format of the attributes pAttrSkin2Bones2Weights etc
    if(pAttrSkin2Bones2Weights)
    {
        boneOffsets[0] = ((unsigned short*)(((char*)pAttrSkin2Bones2Weights->pAttributeBufferData) + i*pAttrSkin2Bones2Weights->strideBytes))[0];
        boneOffsets[1] = ((unsigned short*)(((char*)pAttrSkin2Bones2Weights->pAttributeBufferData) + i*pAttrSkin2Bones2Weights->strideBytes))[1];
        boneWeights[0] = (float)(((unsigned short*)(((char*)pAttrSkin2Bones2Weights->pAttributeBufferData) + i*pAttrSkin2Bones2Weights->strideBytes))[2]) / 100.0;
        boneWeights[1] = 1.0 - boneWeights[0];
        numbones = 2;
    }
    else if(pAttrSkinBones && pAttrSkinWeights)
    {
        assert(!"TODO");
        for(int b=0; b<4; b++)
        {
            boneOffsets[b] = ((int*  )(((char*)pAttrSkinBones->pAttributeBufferData) + i*pAttrSkinBones->strideBytes))[b];
            boneWeights[b] = ((float*)(((char*)pAttrSkinWeights->pAttributeBufferData) + i*pAttrSkinWeights->strideBytes))[b];
        }
        numbones = 4;
    }
    vec3f Pos(0,0,0);
    for(int s=0; s<numbones; s++)
    {
        Pos += boneWeights[s] * (matArray[(int)boneOffsets[s]] * P);
    }
    return Pos;
}
//------------------------------------------------------------------------------
//
// read an OBJ file to put back the vertices/normals/TC in the buffer of the 
// SAME SIZE.
// Typically: used to export and re-import data for sculpting from ZBrush/Blender
//
//------------------------------------------------------------------------------
bool injectOBJ(const char* fullInStr, bk3d::Mesh *pMesh, bool applyTransforms, bool isBS=false)
{
	bool bRes = false;
	nv::Model *objModel = nv::Model::Create();
	if(verbose) printf( "loading obj file...\n");
	bRes = objModel->loadModelFromFile(fullInStr);
	if(!bRes)
	{
		return false;
	}
    //
    // Find the right attributes
    //
    bk3d::Attribute *pAttrPosition;
    bk3d::Attribute *pAttrNormal;
    bk3d::Attribute *pAttrTC;
    bk3d::Attribute *pAttrTan;
    bk3d::Attribute *pAttrBin;
    bk3d::Attribute *pAttrCol;
    bk3d::Attribute *pAttrSkinWeights = NULL;
    bk3d::Attribute *pAttrSkinBones = NULL;
    bk3d::Attribute *pAttrSkin2Bones2Weights = NULL;

    pAttrPosition   = findAttr(MESH_POSITION, pMesh, isBS);
    pAttrNormal     = findAttr(MESH_NORMAL, pMesh, isBS);
    pAttrTC         = findAttr(MESH_TEXCOORD0, pMesh, isBS);
    if(pAttrTC == NULL)
        pAttrTC     = findAttr("Texcoord", pMesh, isBS);
    pAttrTan        = findAttr(MESH_TANGENT, pMesh, isBS);
    pAttrBin        = findAttr(MESH_BINORMAL, pMesh, isBS);
    pAttrCol        = findAttr(MESH_COLOR, pMesh, isBS);
    pAttrSkinWeights= findAttr(MESH_BONESWEIGHTS, pMesh, isBS);
    pAttrSkinBones  = findAttr(MESH_BONESOFFSETS, pMesh, isBS);
    pAttrSkin2Bones2Weights= findAttr(MESH_2BONES2WEIGHTS, pMesh, isBS);
    int numbones = pAttrSkin2Bones2Weights ? 2:(pAttrSkinBones?4:0);//pMesh->numJointInfluence;
	//
	// check if the sizes are the same
	//
    unsigned int vtxCnt = pMesh->pSlots->p[0]->vertexCount;
    int vtxCntObj = objModel->getPositionCount();
    if(vtxCnt != vtxCntObj)
        goto end;
	if((objModel->getTangentCount() == 0) && (pAttrTC))
	{
		if(verbose) printf( "Computing tangents of the obj file...\n");
        objModel->computeTangents();
	}
    //
    // precompute the transformations for skinning
    //
    mat4f *matArray = NULL;
    if(applyTransforms)
    {
        if(!pMesh->pTransforms)
            return false;
        matArray = new mat4f[pMesh->pTransforms->n];
        for(int i=0; i<pMesh->pTransforms->n; i++)
        {
            bk3d::Bone* tr = pMesh->pTransforms->p[i];
            assert(tr->ValidComps() & TRANSFCOMP_bindpose_matrix);
            mat4f &bpM = (mat4f)tr->MatrixInvBindpose();
            mat4f &M = (mat4f)tr->MatrixAbs();
            // take the inverse because we want to get back to BindPose vertices
            matArray[i] = inverse(M * bpM);
        }
    }
    //
    // We must 'compile' the model in a way that only one index is used for vertices
    // this will lead to 
    //
    objModel->compileModel2();
    int stride, nOffset, tcOffset, sTanOffset, cOffset;
    const float *attrs = objModel->getCompiledAttributes(stride, nOffset, tcOffset, sTanOffset, cOffset);
	if(objModel->getNormalCount() == 0)
	{
		// TODO: ask my lib to compute this...
		if(verbose) printf( "Computing normals of the obj file...\n");
		objModel->computeNormals();
	}
    float* pDestP  = (float*)pAttrPosition->pAttributeBufferData;
    float* pDestN  = pAttrNormal ? (float*)pAttrNormal->pAttributeBufferData : NULL;
    float* pDestTC = pAttrTC  ? (float*)pAttrTC->pAttributeBufferData : NULL;
    float* pDestT  = pAttrTan ? (float*)pAttrTan->pAttributeBufferData : NULL;
    float* pDestC  = pAttrCol ? (float*)pAttrCol->pAttributeBufferData : NULL;
    for(int i=0; i<vtxCnt; i++)
    {
        // TODO Binormal...
        pDestP = (float*)(((char*)pAttrPosition->pAttributeBufferData) + i*pAttrPosition->strideBytes);
        if(pDestN) pDestN = (float*)(((char*)pAttrNormal->pAttributeBufferData) + i*pAttrNormal->strideBytes);
        if(pAttrTC)pDestTC= (float*)(((char*)pAttrTC->pAttributeBufferData) + i*pAttrTC->strideBytes);
        if(pDestT) pDestT = (float*)(((char*)pAttrTan->pAttributeBufferData) + i*pAttrTan->strideBytes);
        if(pDestC) pDestC = (float*)(((char*)pAttrCol->pAttributeBufferData) + i*pAttrCol->strideBytes);

        vec4f P(attrs[0], attrs[1], attrs[2], 1.0f);
        if(applyTransforms)
            P = applySkinning( i, P, pAttrSkin2Bones2Weights
                    ,pAttrSkinBones
                    ,pAttrSkinWeights
                    ,matArray);
        memcpy(pDestP, P.vec_array, sizeof(float)*objModel->getPositionSize());
        if((nOffset>=0) && pDestN)
        {
            vec4f N(attrs[nOffset], attrs[nOffset+1], attrs[nOffset+2], 0.0f);
            if(applyTransforms)
                N = applySkinning( i, N, pAttrSkin2Bones2Weights
                        ,pAttrSkinBones
                        ,pAttrSkinWeights
                        ,matArray);
            memcpy(pDestN, N.vec_array, sizeof(float)*objModel->getNormalSize());
        }
        if((tcOffset>=0) && pDestTC)
            memcpy(pDestTC,&attrs[tcOffset], sizeof(float)*objModel->getTexCoordSize());
        if((sTanOffset>=0) && pDestT)
            memcpy(pDestT, &attrs[sTanOffset], sizeof(float)*objModel->getTangentSize());
        if((cOffset>=0) && pDestC)
            memcpy(pDestC, &attrs[cOffset], sizeof(float)*objModel->getColorSize());

        attrs += stride;
    }
    bRes = true;
    //Then we must re-inject vertices to VBOs
end:
    delete objModel;
	return bRes;
}

#define FPRINTFINDEX(fd, i)\
{\
    unsigned int idx = indexInt ? indexInt[i+0]:indexShort[i+0];\
    fprintf(fd, "%d/", idx+1);\
    if(pAttrTC)\
        fprintf(fd, "%d", idx+1);\
    if(pAttrNormal)\
        fprintf(fd, "/%d ", idx+1);\
}

//------------------------------------------------------------------------------
//
// write out an OBJ file to put back the vertices/normals/TC
// Typically: used to export and re-import data for sculpting from ZBrush/Blender
//
//------------------------------------------------------------------------------
bool ejectOBJ(const char* fullInStr, bk3d::Mesh *pMesh, bool applyTransforms=false, bool isBS=false)
{
	bool bRes = false;
    FILE *fd = fopen(fullInStr, "w");
    if(!fd)
        return false;
    //
    // Find the right attributes
    //
    bk3d::Attribute *pAttrPosition = NULL;
    bk3d::Attribute *pAttrNormal = NULL;
    bk3d::Attribute *pAttrTC = NULL;
    bk3d::Attribute *pAttrTan = NULL;
    bk3d::Attribute *pAttrBin = NULL;
    bk3d::Attribute *pAttrCol = NULL;
    bk3d::Attribute *pAttrSkinWeights = NULL;
    bk3d::Attribute *pAttrSkinBones = NULL;
    bk3d::Attribute *pAttrSkin2Bones2Weights = NULL;

    pAttrPosition   = findAttr(MESH_POSITION, pMesh, isBS);
    pAttrNormal     = findAttr(MESH_NORMAL, pMesh, isBS);
    pAttrTC         = findAttr(MESH_TEXCOORD0, pMesh, isBS);
    if(pAttrTC == NULL)
        pAttrTC     = findAttr("Texcoord", pMesh, isBS);
    pAttrTan        = findAttr(MESH_TANGENT, pMesh, isBS);
    pAttrBin        = findAttr(MESH_BINORMAL, pMesh, isBS);
    pAttrCol        = findAttr(MESH_COLOR, pMesh, isBS);
    pAttrSkinWeights= findAttr(MESH_BONESWEIGHTS, pMesh, isBS);
    pAttrSkinBones  = findAttr(MESH_BONESOFFSETS, pMesh, isBS);
    pAttrSkin2Bones2Weights= findAttr(MESH_2BONES2WEIGHTS, pMesh, isBS);
    nv_math::vec3f P, N, T;
    float *pC;
    float *pTC;
    mat4f *matArray = NULL;
    int numbones = pAttrSkin2Bones2Weights ? 2:(pAttrSkinBones?4:0);//pMesh->numJointInfluence;
    fprintf(fd,"# exported from bk3d viewer\n");
    unsigned int vtxCnt = pMesh->pSlots->p[0]->vertexCount;
    //
    // precompute the transformations for skinning
    //
    if(applyTransforms)
    {
        if(!pMesh->pTransforms)
            return false;
        matArray = new mat4f[pMesh->pTransforms->n];
        for(int i=0; i<pMesh->pTransforms->n; i++)
        {
            bk3d::Bone* tr = pMesh->pTransforms->p[i];
            assert(tr->ValidComps() & TRANSFCOMP_bindpose_matrix);
            mat4f &bpM = (mat4f)tr->MatrixInvBindpose();
            mat4f &M = (mat4f)tr->MatrixAbs();
            matArray[i] = M * bpM;
        }
    }
    // Position
    for(int i=0; i<vtxCnt; i++)
    {
        P = *(vec3f*)(((char*)pAttrPosition->pAttributeBufferData) + i*pAttrPosition->strideBytes);
        // apply skinning
        if(applyTransforms)
            P = applySkinning( i, P, pAttrSkin2Bones2Weights
                    ,pAttrSkinBones
                    ,pAttrSkinWeights
                    ,matArray);
        fprintf(fd,"v %f %f %f\n", P.x, P.y, P.z);
    }
    if(pAttrNormal) for(int i=0; i<vtxCnt; i++)
    {
        N = *(vec3f*)(((char*)pAttrNormal->pAttributeBufferData) + i*pAttrNormal->strideBytes);
        // apply skinning
        if(applyTransforms)
            N = applySkinning( i, N, pAttrSkin2Bones2Weights
                    ,pAttrSkinBones
                    ,pAttrSkinWeights
                    ,matArray);
        fprintf(fd,"vn %f %f %f\n", N.x, N.y, N.z);
    }
    if(pAttrTC) for(int i=0; i<vtxCnt; i++)
    {
        pTC= (float*)(((char*)pAttrTC->pAttributeBufferData) + i*pAttrTC->strideBytes);
        fprintf(fd,"vt %f %f\n", pTC[0], pTC[1]);
    }
    //if(pAttrTan) for(int i=0; i<vtxCnt; i++)
    //{
    //    pT = (float*)(((char*)pAttrTan->pAttributeBufferData) + i*pAttrTan->strideBytes);
    //    fprintf(fd,"v %f %f\n", pTC[0], pTC[1]);
    //}
    if(pAttrCol) for(int i=0; i<vtxCnt; i++)
    {
        pC = (float*)(((char*)pAttrCol->pAttributeBufferData) + i*pAttrCol->strideBytes);
        // vc ?
        fprintf(fd,"vc %f %f %f\n", pC[0], pC[1], pC[2]);
    }
    fprintf(fd,"# faces\n");
    for(int pg=0; pg < pMesh->pPrimGroups->n; pg++)
    {
        bk3d::PrimGroup* pPG = pMesh->pPrimGroups->p[pg];
        fprintf(fd,"s %d\n", pg);
        char tmpstr[50];
        strncpy(tmpstr, pPG->pMaterial ? pPG->pMaterial->name : "default", 49);
        tmpstr[49] = '\0';
        for(int c=0; tmpstr[c]!='\0'; c++) {
            if(tmpstr[c]==' ') tmpstr[c]='_';
            if(tmpstr[c]=='.') tmpstr[c]='_';
        }
        fprintf(fd,"g grp_%s\n", tmpstr);
        fprintf(fd,"usemtl %s\n", tmpstr);
        fprintf(fd,"usemap %s_map\n", tmpstr);
        unsigned int* indexInt = NULL;
        unsigned short* indexShort = NULL;
        switch(pPG->indexFormatGL)
        {
        case GL_UNSIGNED_INT:
            indexInt = (unsigned int*)pPG->pIndexBufferData;
            indexInt += pPG->indexOffset;
            break;
        case GL_UNSIGNED_SHORT:
            indexShort = (unsigned short*)pPG->pIndexBufferData;
            indexShort += pPG->indexOffset;
            break;
        }
        // this mode is special: we kept multiple index per vertex...
        if(pPG->indexPerVertex > 1)
        {
            assert(!"TODO ejectOBJ indexPerVertex>1");
            // TODO: OBJ fmt is natively supporting it
        }
        else for(int i=0; i<pPG->indexCount;)
        {
            fprintf(fd, "f ");
            switch(pPG->topologyGL)
            {
            default:
                printf("WARNING: not handled, yet\n");
                break;
            case GL_TRIANGLES:
                FPRINTFINDEX(fd, i)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+2)
                i += 3;
                break;
            case GL_LINES:
                FPRINTFINDEX(fd, i)
                FPRINTFINDEX(fd, i+1)
                i += 2;
                break;
            case GL_TRIANGLE_STRIP:
                FPRINTFINDEX(fd, i)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+2)
                i += 1;
                break;
            case GL_QUADS:
                FPRINTFINDEX(fd, i)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+2)
                fprintf(fd, "\nf ");
                FPRINTFINDEX(fd, i+2)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+3)
                i += 4;
                break;
            case GL_QUAD_STRIP:
                FPRINTFINDEX(fd, i)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+2)
                fprintf(fd, "\nf ");
                FPRINTFINDEX(fd, i+2)
                FPRINTFINDEX(fd, i+1)
                FPRINTFINDEX(fd, i+3)
                i += 1;
                break;
            }
            fprintf(fd, "\n");
        }
    }
    bRes = true;
end:
    fclose(fd);
    if(matArray)
        delete [] matArray;
	return bRes;
}
