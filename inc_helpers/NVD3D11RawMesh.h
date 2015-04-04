//----------------------------------------------------------------------------------
// File:   nvrawmesh.h
// Author: Tristan Lorach
// Email:  lorachnroll@gmail.com
// 
// Copyright (c) 2013 Tristan Lorach. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND T.LORACH AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  T.LORACH OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF T.LORACH HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//

//#ifndef __D3D11RawMesh_h__
//#define __D3D11RawMesh_h__

#include "NVRawMesh.h"
#include <vector>
#include <map>
#ifndef NOCURVES
#include "../Curve/CurveReader.h"
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef PRINTF
#define PRINTF(a) printf a
#endif
#define DEG2RAD( a ) ( a * D3DX_PI / 180.f )
/*----------------------------------------------------------------------------------*/ /**


**/ //----------------------------------------------------------------------------------
class CD3D11MeshHelper
{
public:
    // each primitive group can have a different shader
    struct TILayout {
        TILayout() : pLayout(NULL) {}
        std::vector<int>                        usedAttribsInMesh;
        std::vector<D3D11_INPUT_ELEMENT_DESC>   attribDesc;
        ID3D11InputLayout                       *pLayout;
    };
    // each pass can have a different input layout, too
    typedef std::map< int, TILayout >       MapOfPasses; 
    // primitive groups can have a specific input layout for each pass
    typedef std::map< int, MapOfPasses >    MapPrimGroups;
    // all the layouts for primitive groups are in here:
    MapPrimGroups                           layoutsForPrimGroups;
    // raw means that RawMesh is the mesh data before any DX11-specific init : exported data
    // pRawMesh points to FileHeader::pMesh[xx], the one that got used here for DX11 things
    bk3d::Mesh *          pRawMesh;
    //D3D11_INPUT_ELEMENT_DESC  *pLayoutDesc;
    int                       numLayoutSlots;
    int                       numVertexBuffers;
    int                       numStreamOutBuffers;
    int                       numIndexBuffers;
    //
    // Various ways of exposing the buffers :
    // as a typical buffer
    // as a resource + its resource view : to bind it to a Buffer<> templated variable
    //
    struct BlendShapesResource{
      ID3D11Buffer              *pVertexResource;   // big chunk of memory with all the vtx buffers as resources
      ID3D11ShaderResourceView  *pVertexView;       // corresponding views
      ID3D11Buffer              *pOffsetsResource;  // offsets of Blendshapes to use
      ID3D11ShaderResourceView  *pOffsetsView;
      ID3D11Buffer              *pWeightsResource;  // weights
      ID3D11ShaderResourceView  *pWeightsView;
      int                       numUsedBS;          // amount of used BS in Offsets/Weights
    };
    BlendShapesResource       bsResource;

    ID3D11Buffer              **pVertexBuffers;   // array of ptr of basic Vtx buffers
    ID3D11Buffer              **pIndexBuffers;    // array of ptr
    ID3D11Buffer              **pStreamOutBuffers;// array of ptr
    UINT                      *pStreamOutStrides; // stride for the streamed out buffers
    UINT                      *pStrides;
    UINT                      *pOffsets;

    ~CD3D11MeshHelper();
    CD3D11MeshHelper(bk3d::Mesh *pRawMesh);

    int         getNumPrimGroups() { return pRawMesh->pPrimGroups->n; }
    bk3d::PrimGroup*  
                getPrimGroup(int n) { return pRawMesh->pPrimGroups->p[n]; }
    int         getNumJointInfluence() {    return pRawMesh->numJointInfluence; }
    int         getNumTransforms() {        return pRawMesh->pTransforms ? pRawMesh->pTransforms->n : 0; }
    float*      getTransformArray(float *matArrayToFill);

    HRESULT     bindBuffers(ID3D11DeviceContext* pd3dDeviceContext);
    HRESULT     bindInputLayout(ID3D11DeviceContext* pd3dDeviceContext, int pass, int primGroup=0);
    HRESULT     bindBuffersOverhideWithSO(ID3D11DeviceContext* pd3dDeviceContext, int SO, int bufferToReplace, int pass, int primGroup=0);
    HRESULT     bindIndexBufferAndDrawPrims(ID3D11DeviceContext* pd3dDeviceContext, int primGroupStart=0, int primGroupEnd=-1);

    HRESULT     unbindBuffers(ID3D11DeviceContext* pd3dDeviceContext);

    void        streamVerticesToSO(ID3D11DeviceContext* pd3dDeviceContext, int pass=0, int SO=0);

    HRESULT     initBuffers(ID3D11Device* pd3dDevice);
    HRESULT     createVertexBuffers(ID3D11Device* pd3dDevice, bool bAsVtxBuffer=true, bool bAsResource=false, bool bBSAsVtxBuffer=true, bool bBSAsResource=true, bool bVerbose = false);
    HRESULT     createIndexBuffers(ID3D11Device* pd3dDevice, bool bVerbose = false);
    HRESULT     createInputLayout(ID3D11Device* pd3dDevice, const void *shaderCode, size_t sz, int pass, int primGroup);

    HRESULT     createBSOffsetsAndWeightBuffers(ID3D11Device* pd3dDevice, bool bVerbose = false);
    HRESULT     createStreamOutBuffers(ID3D11Device* pd3dDevice, int numSOBuffers, int slot, bool bVerbose = false);

    // use this before createInputLayout() when you want a specific mapping of attributes
    void        resetAttributeMapping();
    bool        addAttribute(LPCSTR shaderAttribName, LPCSTR meshAttribName, int pass=0, int primGroup=-1, int slotoffset=-1);
    // release allocations and DX11 interfaces:
    void        release();
};
/*----------------------------------------------------------------------------------*/ /**
CD3D11SceneHelper CD3D11SceneHelper CD3D11SceneHelper
**/ //----------------------------------------------------------------------------------
class CD3D11SceneHelper
{
protected:
public:
    bk3d::FileHeader            *pRawHeader;
#ifndef NOCURVES
    CurvePool                       cvPool;
#endif
    std::vector<CD3D11MeshHelper>   dx11meshes;

    CD3D11SceneHelper();
    bk3d::FileHeader*       load(LPCSTR name);
    float*                  findComponentf(const char *compname, bool **pDirty);
    void                    bindAnimationFile(LPCSTR animfile);
    void                    bindAnimationFromBk3d();
    void                    recTransfUpdate(D3DXMATRIX &Mparent, bk3d::Transform *ptr);
    void                    updateTransforms();
    void                    updateAnim(float time)  {    
#ifndef NOCURVES
        cvPool.updateAll(time); 
#endif
    }
    CD3D11MeshHelper*       getDX11Mesh(int meshNum = 0) { return meshNum < pRawHeader->pMeshes->n ? &(dx11meshes[meshNum]) : NULL; }
    int                     getNumDX11Meshes() { return pRawHeader->pMeshes->n; }
    void                    release();
};

/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE CD3D11SceneHelper::CD3D11SceneHelper()
{
}
INLINE void CD3D11SceneHelper::release()
{
    for(int i=0; i<pRawHeader->pMeshes->n; i++)
    {
        dx11meshes[i].release();
    }
    dx11meshes.clear();
    free(pRawHeader);
    pRawHeader = NULL;
#ifndef NOCURVES
    cvPool.clear();
#endif
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
#ifndef NOCURVES
INLINE void CD3D11SceneHelper::bindAnimationFile(LPCSTR animfile)
{
    CurveVector *cv = cvPool.newCVFromFile(animfile);
    if(!cv)
    {
        PRINTF((TEXT("Warning: Failed to load %S"), animfile));
    }
    for(int i=0; i<cvPool.getNumCV(); i++)
    {
        CurveVector *pcv = cvPool.getCVByIndex(i);
        bool *pdirty;
        float *pf = pRawHeader->findComponentf(pcv->getName(), &pdirty);
        if(pf)
            pcv->bindPtrs(  pf, pf+1, pf+2, pf+3, pdirty);
        else
        {
            const char *n = pcv->getName();
            PRINTF((TEXT("Component not found : %s\n"), n));
        }
    }
}
void CD3D11SceneHelper::bindAnimationFromBk3d()
{
    for(int i=0; i<pRawHeader->pMayaCurves->n; i++)
    {
        bk3d::MayaCurveVector *pMCV = pRawHeader->pMayaCurves->p[i];
        // First, see if the curve is of any interest
        bool *pdirty;
        float *pf = pRawHeader->findComponentf(pMCV->name, &pdirty);
        if(!pf)
            continue;
        if(pMCV->nCurves == 0)
            continue;
        CurveVector *pcv = cvPool.newCV(pMCV->name, pMCV->nCurves);
        for(int j=0; j<pMCV->nCurves; j++)
        {
            CurveReader *pCurve = pcv->getCurve(j);
            bk3d::MayaCurve *pMC = pMCV->pCurve[j];
            pCurve->startKeySetup(
                pMC->inputIsTime,
                pMC->outputIsAngular,
                pMC->isWeighted,
                (::EtInfinityType)pMC->preInfinity,
                (::EtInfinityType)pMC->postInfinity);
            for(int k=0; k<pMC->nKeys; k++)
            {
                pCurve->addKey(
                    pMC->key[k].time,
                    pMC->key[k].value,
                    (::EtTangentType)pMC->key[k].inTangentType,
                    (::EtTangentType)pMC->key[k].outTangentType,
                    pMC->key[k].inAngle,
                    pMC->key[k].inWeight,
                    pMC->key[k].outAngle,
                    pMC->key[k].outWeight);
            }
            pCurve->endKeySetup();
        }
        pcv->bindPtrs(  pf, pf+1, pf+2, pf+3, pdirty);
    }
}
#endif
/*----------------------------------------------------------------------------------*/ /**
 Joint transformation order (OpenGL order) :
 P2 = Mtranslate * MjointOrient * Mrotation * Mrotorientation * Mscale * P
 basic Transformation order :
 P2 = MrotPivotTransl * MrotPivot * Mrotation * MrotOrient * MrotPivotInv * MscalePivotTransl 
      * MscalePivot * Mscale * MscalePivotInv * P
**/ //----------------------------------------------------------------------------------
INLINE void CD3D11SceneHelper::recTransfUpdate(D3DXMATRIX &Mparent, bk3d::Transform *ptr)
{
        D3DXMATRIX mat, mat2;
        if(ptr->bDirty)
        {
            D3DXMATRIX Mtemp;
            //Bindpose
            D3DXMATRIX Mbp(ptr->bindpose_matrix);
            //translation
            D3DXMATRIX Mtrans;
            D3DXMatrixTranslation(&Mtrans,   ptr->pos[0], ptr->pos[1], ptr->pos[2]);
            //joint orientation
            D3DXQUATERNION qjorient(ptr->jointOrientation[0], 
                                    ptr->jointOrientation[1],
                                    ptr->jointOrientation[2],
                                    ptr->jointOrientation[3]);
            D3DXMATRIX Mjorient;
            D3DXMatrixRotationQuaternion(&Mjorient, &qjorient);
            //Rotation
#if 0
            // Warning : cannot use this for other order than Z-X-Y !
            D3DXMATRIX Myawpitchroll;
            D3DXMatrixRotationYawPitchRoll(&Myawpitchroll,  DEG2RAD(ptr->rotation[0]), DEG2RAD(ptr->rotation[1]), DEG2RAD(ptr->rotation[2]));
#else
            D3DXVECTOR3 vx(1,0,0);
            D3DXMATRIX Mrotx;
            D3DXMatrixRotationAxis(&Mrotx, &vx, DEG2RAD(ptr->rotation[0]));
            D3DXVECTOR3 vy(0,1,0);
            D3DXMATRIX Mroty;
            D3DXMatrixRotationAxis(&Mroty, &vy, DEG2RAD(ptr->rotation[1]));
            D3DXVECTOR3 vz(0,0,1);
            D3DXMATRIX Mrotz;
            D3DXMatrixRotationAxis(&Mrotz, &vz, DEG2RAD(ptr->rotation[2]));
#endif
            //joint orientation
            D3DXQUATERNION qrorient(ptr->rotationOrientation[0],
                                    ptr->rotationOrientation[1],
                                    ptr->rotationOrientation[2],
                                    ptr->rotationOrientation[3]);
            D3DXMATRIX Mrorient;
            D3DXMatrixRotationQuaternion(&Mrorient, &qrorient);
            //scale
            D3DXMATRIX Mscale;
//WRONG...TO FIX
            D3DXMatrixScaling(&Mscale, ptr->scale[0], ptr->scale[1], ptr->scale[2]);
            //Multiply all together
#if 1
            mat = Mscale * Mrorient * Mrotx * Mroty * Mrotz * Mjorient * Mtrans * Mparent;
#else
            // Only for ZXY
            mat = Mscale * Mrorient * Myawpitchroll * Mjorient * Mtrans * Mparent;
#endif
            mat2 = Mbp * mat;
            memcpy(ptr->abs_matrix, mat2.m, sizeof(float)*16);
        } else {
            //Bindpose
            D3DXMATRIX Mbp(ptr->bindpose_matrix);
            D3DXMATRIX Mlocal(ptr->matrix);
            D3DXMatrixMultiply(&mat, &Mlocal, &Mparent);
            D3DXMatrixMultiply(&mat2, &Mbp, &mat);
            memcpy(ptr->abs_matrix, mat2.m, sizeof(float)*16);
        }
        //
        // Children
        //
        if(ptr->pChildren)
        for(int c=0; c<ptr->pChildren->n; c++)
        {
            recTransfUpdate(mat, ptr->pChildren->p[c]);
        }

}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE void CD3D11SceneHelper::updateTransforms()
{
    if(!pRawHeader->pTransforms)
        return;
    D3DXMATRIX Mid;
    D3DXMatrixIdentity(&Mid);
    for(int i=0; i<pRawHeader->pTransforms->n; i++)
    {
        bk3d::Transform *pT = pRawHeader->pTransforms->p[i];
        if(pT->pParent == NULL)
            recTransfUpdate(Mid, pT);
    }
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE CD3D11MeshHelper::~CD3D11MeshHelper()
{
}
INLINE CD3D11MeshHelper::CD3D11MeshHelper(bk3d::Mesh *pRawMesh) : 
        pRawMesh(pRawMesh) 
{
        numLayoutSlots = 0;
        numStreamOutBuffers = 0;
        pRawMesh = NULL;
        pVertexBuffers = NULL;
        pIndexBuffers = NULL;
        numIndexBuffers = 0;
        numVertexBuffers = 0;
        pStrides = 0;
        pStreamOutStrides = 0;
        pOffsets = 0;
        pStreamOutBuffers = NULL;
        bsResource.numUsedBS = 0;
        bsResource.pVertexResource = NULL;
        bsResource.pVertexView = NULL;    
        bsResource.pOffsetsResource = NULL;
        bsResource.pOffsetsView = NULL;
        bsResource.pWeightsResource = NULL;
        bsResource.pWeightsView = NULL;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE float*  CD3D11MeshHelper::getTransformArray(float *matArrayToFill)
{
    if(!pRawMesh->pTransforms)
        return NULL;
    for(int i=0; i<pRawMesh->pTransforms->n; i++)
    {
        memcpy(&(matArrayToFill[16*i]), pRawMesh->pTransforms->p[i]->abs_matrix , sizeof(float)*16);
    }
    return matArrayToFill;
}
/*----------------------------------------------------------------------------------*/ /**
    Basic create of the buffers.
    You may do this by yourself if different options are needed
**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::initBuffers(ID3D11Device* pd3dDevice)
{
    HRESULT hr;
    //
    // Release previous stuff
    //
    //
    // Create again
    //
    if(FAILED(hr = createVertexBuffers(pd3dDevice)))
        return hr;
    if(FAILED(hr = createBSOffsetsAndWeightBuffers(pd3dDevice)))
        return hr;
    if(FAILED(hr = createIndexBuffers(pd3dDevice)))
        return hr;
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::bindBuffers(ID3D11DeviceContext* pd3dDeviceContext)
{
    pd3dDeviceContext->IASetVertexBuffers(0, pRawMesh->pSlots->n, pVertexBuffers, pStrides, pOffsets);
    return S_OK;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::bindInputLayout(ID3D11DeviceContext* pd3dDeviceContext, int pass, int primGroup)
{
    MapOfPasses &mapofpasses = layoutsForPrimGroups[primGroup];
    TILayout &layout = mapofpasses[pass];
    if(!layout.pLayout)
    {
        PRINTF((TEXT("Warning: ptr NULL for IASetInputLayout() !") ));
        return E_FAIL;
    }
    pd3dDeviceContext->IASetInputLayout( layout.pLayout );
    return S_OK;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::bindBuffersOverhideWithSO(ID3D11DeviceContext* pd3dDeviceContext, int SO, int bufferToReplace, int pass, int primGroup)
{
    MapOfPasses &mapofpasses = layoutsForPrimGroups[primGroup];
    TILayout &layout = mapofpasses[pass];
    // TODO ? Depending on primGroup and , we may set only used vertex buffers
    // usedAttribsInMesh has the indices
    ID3D11Buffer *temp              = pVertexBuffers[bufferToReplace];
    UINT tempStride                 = pStrides[bufferToReplace];
    pStrides[bufferToReplace]       = pStreamOutStrides[SO];
    pVertexBuffers[bufferToReplace] = pStreamOutBuffers[SO];

    pd3dDeviceContext->IASetVertexBuffers(0, pRawMesh->pSlots->n, pVertexBuffers, pStrides, pOffsets);

    pVertexBuffers[bufferToReplace] = temp;
    pStrides[bufferToReplace]       = tempStride;
    if(!layout.pLayout)
    {
        PRINTF((TEXT("Warning: ptr NULL for IASetInputLayout() !")));
        return E_FAIL;
    }
    pd3dDeviceContext->IASetInputLayout( layout.pLayout );
    return S_OK;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::unbindBuffers(ID3D11DeviceContext* pd3dDeviceContext)
{
    HRESULT hr = S_OK;
    ID3D11Buffer *pNullBuffers[] = { NULL, NULL };
    UINT offsets[2] = {0,0};
    UINT strides[2] = {0,0};
    pd3dDeviceContext->IASetVertexBuffers(0, 2, pNullBuffers, strides, offsets);
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::bindIndexBufferAndDrawPrims(ID3D11DeviceContext* pd3dDeviceContext, int primGroupStart, int primGroupEnd)
{
    HRESULT hr = S_OK;
    if((primGroupEnd < 0)||(primGroupEnd >= numIndexBuffers))
        primGroupEnd = numIndexBuffers;
    for(int i=primGroupStart; i < primGroupEnd; i++)
    {
        pd3dDeviceContext->IASetIndexBuffer( pIndexBuffers[i], pRawMesh->pPrimGroups->p[i]->indexFormatDXGI, 0 );
        pd3dDeviceContext->IASetPrimitiveTopology( pRawMesh->pPrimGroups->p[i]->topologyDX11 );
        pd3dDeviceContext->DrawIndexed(pRawMesh->pPrimGroups->p[i]->indexCount, 0, 0);
    }
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
/*bool CD3D11MeshHelper::checkDX11LayoutCompatibility(bk3d::FileHeader *pFile, const char * layout)
{
    return true;
}*/
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE bk3d::FileHeader* CD3D11SceneHelper::load(LPCSTR fname)
{
    pRawHeader = bk3d::load(fname);
    if(!pRawHeader)
        return NULL;
    for(int i=0; i<pRawHeader->pMeshes->n; i++)
    {
        dx11meshes.push_back(CD3D11MeshHelper(pRawHeader->pMeshes->p[i]));
    }
    return pRawHeader;
}
/*----------------------------------------------------------------------------------*/ /**

**/ //----------------------------------------------------------------------------------
INLINE void CD3D11MeshHelper::release()
{
    resetAttributeMapping();
    
    for(int i=0; i<numStreamOutBuffers; i++)
      if(pStreamOutBuffers[i]) SAFE_RELEASE(pStreamOutBuffers[i]);
    if(pStreamOutBuffers) delete [] pStreamOutBuffers;
    pStreamOutBuffers = NULL;
    numStreamOutBuffers = 0;

    if(bsResource.pVertexView)      SAFE_RELEASE(bsResource.pVertexView);
    if(bsResource.pVertexResource)  SAFE_RELEASE(bsResource.pVertexResource);
    if(bsResource.pOffsetsView)     SAFE_RELEASE(bsResource.pOffsetsView);
    if(bsResource.pOffsetsResource) SAFE_RELEASE(bsResource.pOffsetsResource);
    if(bsResource.pWeightsView)     SAFE_RELEASE(bsResource.pWeightsView);
    if(bsResource.pWeightsResource) SAFE_RELEASE(bsResource.pWeightsResource);
    bsResource.numUsedBS = 0;
    if(pVertexBuffers) 
    {
        for(int i=0; i<numVertexBuffers; i++)
          if(pVertexBuffers[i]) SAFE_RELEASE(pVertexBuffers[i]);
        delete [] pVertexBuffers;
    }
    pVertexBuffers = NULL;
    numVertexBuffers = 0;

    for(int i=0; i<numIndexBuffers; i++)
      if(pIndexBuffers[i]) SAFE_RELEASE(pIndexBuffers[i]);
    if(pIndexBuffers) delete [] pIndexBuffers;
    pIndexBuffers = NULL;
    numIndexBuffers = 0;

    if(pStrides) delete [] pStrides;
    pStrides = NULL;
    if(pStreamOutStrides) delete [] pStreamOutStrides;
    pStreamOutStrides = NULL;
    numLayoutSlots = 0;
    if(pOffsets) delete [] pOffsets;
    pOffsets = NULL;

    //if(pLayoutDesc) delete [] pLayoutDesc;
    //pLayoutDesc = NULL;
    pRawMesh = NULL;
}
/*----------------------------------------------------------------------------------*/ /**
Create Misc vertex buffers

The buffers can be either used as 
- typical vertex buffers
- resource view so we can bind them to a Buffer<> templated variable
This choice is available either for the main mesh and the blendshapes
Note: DX11 doesn't want to use the same buffer for the 2 usages :(... It complains about incompatible formats...
So we have to create separate buffers.
**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::createVertexBuffers(ID3D11Device* pd3dDevice, 
      bool bAsVtxBuffer, 
      bool bAsResource, 
      bool bBSAsVtxBuffer, 
      bool bBSAsResource, 
      bool bVerbose)
{
    HRESULT hr = S_OK;
    //
    // Free data before reallocating them
    //
    if(bsResource.pVertexResource)  SAFE_RELEASE(bsResource.pVertexResource);
    if(bsResource.pVertexView)      SAFE_RELEASE(bsResource.pVertexView);
    if(pVertexBuffers) 
    {
        for(int i=0; i<numVertexBuffers; i++)
          if(pVertexBuffers[i]) SAFE_RELEASE(pVertexBuffers[i]);
        delete [] pVertexBuffers;
    }
    numVertexBuffers = pRawMesh->pSlots->n + (pRawMesh->pBSSlots ? pRawMesh->pBSSlots->n : 0);
    pVertexBuffers = new ID3D11Buffer*[numVertexBuffers];
    ZeroMemory(pVertexBuffers, sizeof(ID3D11Buffer*)*numVertexBuffers);
    if(pStrides) delete [] pStrides;
    pStrides = new UINT[numVertexBuffers];
    if(pOffsets) delete [] pOffsets;
    pOffsets = new UINT[numVertexBuffers];
    ZeroMemory(pOffsets, sizeof(UINT)*numVertexBuffers);

    //#pragma message("TODO: buffers for the main shape - Not needed for this sample")
    if(bAsResource) PRINTF((TEXT("Error: Main mesh 'as a resource' not working yet...") ));
    //
    // basic Shapes
    //
    for(int i=0; i<pRawMesh->pSlots->n; i++)
    {
        D3D11_SUBRESOURCE_DATA data;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
        data.pSysMem = pRawMesh->pSlots->p[i]->pVtxBufferData;
        //
        // Note : here we also use D3D11_BIND_SHADER_RESOURCE so we can
        // bind this buffer as a shader resource, too : To a Buffer<float3>, for example
        // This is used in the mode using buffers instead of Stream out.
        //
        D3D11_BUFFER_DESC bufferDescMesh =
        {
            pRawMesh->pSlots->p[i]->vtxBufferSizeBytes,
            D3D11_USAGE_DYNAMIC,
            D3D11_BIND_VERTEX_BUFFER,
            D3D11_CPU_ACCESS_WRITE,
            0
        };
        pStrides[i] = pRawMesh->pSlots->p[i]->vtxBufferStrideBytes;
        if(bAsVtxBuffer)
        {
            hr = pd3dDevice->CreateBuffer( &bufferDescMesh, &data, pVertexBuffers + i);
            if( FAILED( hr ) )
            {
                PRINTF((TEXT("Error: Failed creating vertex buffer" )) );
                return hr;
            }
            if(bVerbose) PRINTF((TEXT("Created vertex buffer %d of %d bytes"), i, pRawMesh->pSlots->p[i]->vtxBufferSizeBytes ));
        }
    }
    //
    // Blend-shapes
    //
    if(pRawMesh->pBSSlots)
    {
        for(int i=0; i<pRawMesh->pBSSlots->n; i++)
        {
            D3D11_SUBRESOURCE_DATA data;
            data.SysMemPitch = 0;
            data.SysMemSlicePitch = 0;
            data.pSysMem = pRawMesh->pBSSlots->p[i]->pVtxBufferData;
            D3D11_BUFFER_DESC bufferDescMesh =
            {
                pRawMesh->pBSSlots->p[i]->vtxBufferSizeBytes,
                D3D11_USAGE_DYNAMIC,
                D3D11_BIND_VERTEX_BUFFER,
                D3D11_CPU_ACCESS_WRITE,
                0
            };
            pStrides[i + pRawMesh->pSlots->n] = pRawMesh->pBSSlots->p[i]->vtxBufferStrideBytes;
            if(bBSAsVtxBuffer)
            {
                hr = pd3dDevice->CreateBuffer( &bufferDescMesh, &data, pVertexBuffers + i + pRawMesh->pSlots->n);
                if( FAILED( hr ) )
                {
                    PRINTF((TEXT("Error: Failed creating vertex buffer") ));
                    return hr;
                }
                if(bVerbose) PRINTF((TEXT("Created Blendshape vertex buffer %d of %d bytes"), i, pRawMesh->pBSSlots->p[i]->vtxBufferSizeBytes ));
            }
        }
        //
        // Create resource and view for the Blendshapes
        //
        if(bBSAsResource && (pRawMesh->pBSSlots->n > 0))
        {
            // all BS slots are the same (using bsSlots[0] for all)
            unsigned int sizeBytes = pRawMesh->pBSSlots->n * pRawMesh->pBSSlots->p[0]->vtxBufferSizeBytes;
            D3D11_BUFFER_DESC bufferDescMesh =
            {
                sizeBytes,
                D3D11_USAGE_IMMUTABLE,
                D3D11_BIND_SHADER_RESOURCE,
                0,
                0
            };
            D3D11_SUBRESOURCE_DATA data;
            data.SysMemPitch = 0;
            data.SysMemSlicePitch = 0;
            //
            // The mesh exposes the Blendshapes one after each other in memory.
            // So we know that all BS are available starting pRawMesh->bsSlots[0].pVtxBufferData
            //
            data.pSysMem = pRawMesh->pBSSlots->p[0]->pVtxBufferData;
            hr = pd3dDevice->CreateBuffer( &bufferDescMesh, &data, &(bsResource.pVertexResource) );
            if( FAILED( hr ) )
            {
                PRINTF((TEXT("Error: Failed creating BS Shader resource") ));
                return hr;
            }
            if(bVerbose) PRINTF((TEXT("Created BS shader resource of %d bytes"), sizeBytes ));
            //
            // WARNING: for now, attributes can only be float3
            // TODO: find a way to have a more complex description of the Buffer
            // in the shader, the buffer is Buffer<float3>... need Buffer<VSAttribsStruct>
            //
            D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
            ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
            SRVDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            SRVDesc.Buffer.ElementOffset = 0;
            SRVDesc.Buffer.ElementWidth = // all BS slots are the same (using bsSlots[0] for all)
                pRawMesh->pBSSlots->n * pRawMesh->pBSSlots->p[0]->vertexCount * (pRawMesh->pBSSlots->p[0]->vtxBufferStrideBytes/(3*sizeof(float)));
            hr = pd3dDevice->CreateShaderResourceView( bsResource.pVertexResource, &SRVDesc, &(bsResource.pVertexView) );
            if( FAILED( hr ) )
            {
                PRINTF((TEXT("Error: Failed creating resource view for BS") ));
                return hr;
            }
            else if(bVerbose) PRINTF((TEXT("Created BS resource view")));
        }
    }
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**
    Create buffers for Weights and offsets of Blendshapes
**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::createBSOffsetsAndWeightBuffers(ID3D11Device* pd3dDevice, bool bVerbose)
{
    HRESULT hr = S_OK;
    if( (!pRawMesh->pBSSlots) || pRawMesh->pBSSlots->n == 0)
        return S_OK;
    if(bsResource.pOffsetsResource) SAFE_RELEASE(bsResource.pOffsetsResource);
    if(bsResource.pOffsetsView)     SAFE_RELEASE(bsResource.pOffsetsView);
    if(bsResource.pWeightsResource) SAFE_RELEASE(bsResource.pWeightsResource);
    if(bsResource.pWeightsView)     SAFE_RELEASE(bsResource.pWeightsView);
    bsResource.numUsedBS = 0;
    //
    // Weights
    //
    D3D11_BUFFER_DESC bufferDescMesh =
    {
        pRawMesh->pBSSlots->n * sizeof(float),
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_SHADER_RESOURCE,
        D3D11_CPU_ACCESS_WRITE,
        0
    };
    hr = pd3dDevice->CreateBuffer( &bufferDescMesh, NULL, &(bsResource.pWeightsResource));
    if( FAILED( hr ) )
    {
        PRINTF((TEXT("Error: Failed creating BS Weight buffer resource") ));
        return hr;
    }
    if(bVerbose) PRINTF((TEXT("Created Weight buffer of %d bytes"), pRawMesh->pBSSlots->n * sizeof(float) ));
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
    SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset = 0;
    SRVDesc.Buffer.ElementWidth = pRawMesh->pBSSlots->n;
    hr = pd3dDevice->CreateShaderResourceView( bsResource.pWeightsResource, &SRVDesc, &(bsResource.pWeightsView) );
    if( FAILED( hr ) )
    {
        PRINTF((TEXT("Error: Failed creating BS Weight resource view") ));
        return hr;
    }
    else if(bVerbose) PRINTF((TEXT("Created Weight buffer resource view") ));
    //
    // Offsets
    //
    bufferDescMesh.ByteWidth = pRawMesh->pBSSlots->n * sizeof(UINT);
    SRVDesc.Format = DXGI_FORMAT_R32_UINT;
    hr = pd3dDevice->CreateBuffer( &bufferDescMesh, NULL, &(bsResource.pOffsetsResource));
    if( FAILED( hr ) )
    {
        PRINTF((TEXT("Error: Failed creating BS Weight buffer resource") ));
        return hr;
    }
    if(bVerbose) PRINTF((TEXT("Created Weight buffer of %d bytes"), pRawMesh->pBSSlots->n * sizeof(UINT) ));
    hr = pd3dDevice->CreateShaderResourceView( bsResource.pOffsetsResource, &SRVDesc, &(bsResource.pOffsetsView) );
    if( FAILED( hr ) )
    {
        PRINTF((TEXT("Error: Failed creating BS Weight resource view") ));
        return hr;
    }
    else if(bVerbose) PRINTF((TEXT("Created Weight buffer resource view") ));
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**
Create misc index buffers
**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::createIndexBuffers(ID3D11Device* pd3dDevice, bool bVerbose)
{
    HRESULT hr = S_OK;
    for(int i=0; i<numIndexBuffers; i++)
    {
      if(bVerbose) PRINTF((TEXT("releasing index buffer %d..."), i));
      if(pIndexBuffers[i]) SAFE_RELEASE(pIndexBuffers[i]);
    }
    if(pIndexBuffers) delete [] pIndexBuffers;
    numIndexBuffers = pRawMesh->pPrimGroups->n;
    pIndexBuffers = new ID3D11Buffer*[numIndexBuffers];

    for(int i=0; i<pRawMesh->pPrimGroups->n; i++)
    {
        D3D11_BUFFER_DESC bufferDescMesh =
        {
            pRawMesh->pPrimGroups->p[i]->indexArrayByteSize,
            D3D11_USAGE_DYNAMIC,
            D3D11_BIND_INDEX_BUFFER,
            D3D11_CPU_ACCESS_WRITE,
            0
        };
      D3D11_SUBRESOURCE_DATA data;
      data.SysMemPitch = 0;
      data.SysMemSlicePitch = 0;
      data.pSysMem = pRawMesh->pPrimGroups->p[i]->pIndexBufferData;
        if( FAILED( pd3dDevice->CreateBuffer( &bufferDescMesh, &data, pIndexBuffers + i ) ) )
        {
            PRINTF((TEXT("Error: Failed creating index buffer") ));
            return false;
        }
      if(bVerbose) PRINTF((TEXT("Created index buffer %S of %d bytes"), pRawMesh->pPrimGroups->p[i]->name, pRawMesh->pPrimGroups->p[i]->indexArrayByteSize ));
    }
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**

  Create Vertex Stream out buffers. These are for Blendshape passes : same size

**/ //----------------------------------------------------------------------------------
INLINE HRESULT CD3D11MeshHelper::createStreamOutBuffers(ID3D11Device* pd3dDevice, int numSOBuffers, int slot, bool bVerbose)
{
    HRESULT hr = S_OK;
    for(int i=0; i<numStreamOutBuffers; i++)
    {
      if(bVerbose) PRINTF((TEXT("releasing StreamOut buffer %d..."), i));
      if(pStreamOutBuffers[i]) SAFE_RELEASE(pStreamOutBuffers[i]);
    }
    if(pStreamOutBuffers) delete [] pStreamOutBuffers;
    numStreamOutBuffers = numSOBuffers;
    pStreamOutBuffers = new ID3D11Buffer*[numStreamOutBuffers];

    for(int i=0; i<numStreamOutBuffers; i++)
    {
      D3D11_BUFFER_DESC bufferDescMesh =
      {
          pRawMesh->pSlots->p[slot]->vtxBufferSizeBytes,
          D3D11_USAGE_DEFAULT,
          D3D11_BIND_VERTEX_BUFFER|D3D11_BIND_STREAM_OUTPUT,
          0,
          0
      };
      hr = pd3dDevice->CreateBuffer( &bufferDescMesh, NULL, pStreamOutBuffers + i );
      if( FAILED( hr ) )
      {
          PRINTF((TEXT("Error: Failed creating vertex buffer") ));
        return hr;
      }
    }
    return hr;
};
//
/// simplace case of passthrough : input points and them back to the SO
/// Good for Skinning on any per vertex pre-processing
//
INLINE void CD3D11MeshHelper::streamVerticesToSO(ID3D11DeviceContext* pd3dDeviceContext, int pass, int SO)
{
    ID3D11Buffer *pNullBuffer = NULL;
    UINT offsets = 0;
    pd3dDeviceContext->SOSetTargets( 1, &(pStreamOutBuffers[SO]), &offsets );
    pd3dDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    pd3dDeviceContext->Draw(pRawMesh->pSlots->p[0]->vertexCount , 0);
    pd3dDeviceContext->SOSetTargets( 1, &pNullBuffer, &offsets ); // Optional. Just to avoid the warning
}
//
// simple attribute mapping. Creates the set of D3D11_INPUT_ELEMENT_DESC for each primgroup
// TODO: Blendshape attributes
//
INLINE bool    CD3D11MeshHelper::addAttribute(LPCSTR shaderAttribName, LPCSTR meshAttribName, int pass, int primGroup, int slotoffset)
{
    int s,e;
    if(primGroup == -1)   { s=0;            e = pRawMesh->pPrimGroups->n;    }
    else   {                s = primGroup;  e = primGroup;   }
    for(int i=s; i<e; i++)
    {
        MapOfPasses &mapofpasses = layoutsForPrimGroups[i];
        TILayout &layout = mapofpasses[pass];
        std::vector<int> &usedBuffers = layout.usedAttribsInMesh;
        std::vector<D3D11_INPUT_ELEMENT_DESC> &attribsvec = layout.attribDesc;
        D3D11_INPUT_ELEMENT_DESC attrib;
        //std::map<int, int> &pair = primGroupAttribs[i];
        for(int j=0; j<pRawMesh->pAttributes->n; j++)
        {
            if(!strcmp(pRawMesh->pAttributes->p[j]->name, meshAttribName))
            {
                //pair[a2vAttrib] = j;
                size_t l = strlen(shaderAttribName)+1;//shaderAttribName) + 1;
                char *name = new char[l];
                strcpy_s(name, l, shaderAttribName);//shaderAttribName);
                int idx = 0;
                if(isdigit(name[l-2])) {
                    idx = (int)(name[l-2] - '0');
                    name[l-2] = '\0';
                }
                attrib.SemanticName = name;
                attrib.SemanticIndex = idx;
                attrib.Format = pRawMesh->pAttributes->p[j]->formatDXGI;
                // The slot in which the blendshape is located is chosen by the app and not by what is in the mesh definition
                //pRawMesh->pAttributes->p[j]->slot is just telling you which bs # it is...
                attrib.InputSlot = slotoffset >= 0 ? slotoffset : pRawMesh->pAttributes->p[j]->slot;
                attrib.AlignedByteOffset = pRawMesh->pAttributes->p[j]->alignedByteOffset;
                attrib.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                attrib.InstanceDataStepRate = 0;
                attribsvec.push_back(attrib);   // store this for later layout creation
                usedBuffers.push_back(j);       // store the index of the used buffer
                return true;
            }
        }
        // Look into Blendshapes attributes
        /*for(int j=0; j<pRawMesh->pBSAttributes->n; j++)
        {
            if(!strcmp(pRawMesh->pBSAttributes->p[j]->name, meshAttribName))
            {
                //pair[a2vAttrib] = j;
                size_t l = strlen(pRawMesh->pBSAttributes->p[j]->name)+1;//shaderAttribName) + 1;
                char *name = new char[l];
                strcpy_s(name, l, pRawMesh->pBSAttributes->p[j]->name);//shaderAttribName);
                int idx = 0;
                // TODO: Fix this for Bs
                // because a BS is "bs_normal321"
                // which doesn't make it for this, with texcoord, for example :
                // "bs_texcoord0321"... TO CHANGE
                // OR let's assume that for now we cannot work with >texcoord0 components in a BS...
                / *if(isdigit(name[l-2])) {
                    idx = (int)(name[l-2] - '0');
                    name[l-2] = '\0';
                }* /
                attrib.SemanticName = name;
                attrib.SemanticIndex = idx;
                attrib.Format = pRawMesh->pBSAttributes->p[j]->formatDX11;
                attrib.InputSlot = slotoffset + pRawMesh->pBSAttributes->p[j]->slot;
                attrib.AlignedByteOffset = pRawMesh->pBSAttributes->p[j]->alignedByteOffset;
                attrib.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                attrib.InstanceDataStepRate = 0;
                attribsvec.push_back(attrib);   // store this for later layout creation
                usedBuffers.push_back(j);       // store the index of the used buffer
                return true;
            }
        }*/
    }
    PRINTF((TEXT("Error: Attribute %S not available in this model (%S)"), meshAttribName, pRawMesh->name));
    return false;
}
INLINE HRESULT CD3D11MeshHelper::createInputLayout(ID3D11Device* pd3dDevice, const void *shaderCode, size_t sz, int pass, int primGroup)
{
    HRESULT hr;
    int s,e;
    if(primGroup == -1)   { s=0;            e = pRawMesh->pPrimGroups->n;    }
    else   {                s = primGroup;  e = primGroup;   }
    for(int i=s; i<e; i++)
    {
        MapOfPasses &mapofpasses = layoutsForPrimGroups[i];
        TILayout &layout = mapofpasses[pass];
        std::vector<D3D11_INPUT_ELEMENT_DESC> &attribsvec = layout.attribDesc;
        if(attribsvec.empty())
            continue;
        D3D11_INPUT_ELEMENT_DESC *pDesc = new D3D11_INPUT_ELEMENT_DESC[attribsvec.size()];
        for(unsigned int j=0; j<attribsvec.size(); j++)
            pDesc[j] = attribsvec[j];
        ID3D11InputLayout *pL;
        int n = (int)attribsvec.size();
        if( FAILED(hr = pd3dDevice->CreateInputLayout( pDesc, n, shaderCode, sz, &pL ) ) )
        {
          PRINTF((TEXT("Error: CreateInputLayout() failed.") ));
        }
        layout.pLayout = pL;
        delete [] pDesc;
    }
    return hr;
}
/*----------------------------------------------------------------------------------*/ /**
Create the Imput Layout interface, depending on the technique
// TODO : to give the Pass number...
**/ //----------------------------------------------------------------------------------
INLINE void CD3D11MeshHelper::resetAttributeMapping() 
{    
    MapPrimGroups::iterator iL = layoutsForPrimGroups.begin();
    while(iL != layoutsForPrimGroups.end())
    {
        MapOfPasses::iterator iL2 = iL->second.begin();
        while(iL2 != iL->second.end())
        {
            SAFE_RELEASE(iL2->second.pLayout);
            std::vector<D3D11_INPUT_ELEMENT_DESC> &idescvec = iL2->second.attribDesc;
            for(unsigned int i=0; i<idescvec.size(); i++)
            {
                delete [] idescvec[i].SemanticName;
                idescvec[i].SemanticName = NULL;
            }
            ++iL2;
        }
        ++iL;
    }
    layoutsForPrimGroups.clear();
}

//#endif
