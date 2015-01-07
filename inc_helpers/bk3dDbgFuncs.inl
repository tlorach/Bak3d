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
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//// METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __BK3DMETHODS__
#define __BK3DMETHODS__


namespace bk3d
{
//==========================================================================================
// 
// Transform methods
// 
//==========================================================================================

///
/// \brief dump the layout of a transform object
///
INLINE void Transform_debugDumpLayout(Bone* pT, int l, int level, const char * nodeNameFilter)
{
	if(pT->getParent() && (l==0)) // if we have a parent : let's not display it when at the level 0
		return;
      if(nodeNameFilter && (strcmp(nodeNameFilter, pT->name)))
          goto passThePrint;
	for(int i=0	; i<l; i++) PRINTF(( TEXT("\t") ));	
	PRINTF(( TEXT("TRANSFORM node : ") FSTR TEXT(" (parent: ") FSTR TEXT(" )\n"), pT->name, pT->getParent() ? pT->getParent()->name : "None"));
    if(level>0)
    {
	    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	    PRINTF((TEXT("| pos : %f %f %f\n"), pT->Pos()[0], pT->Pos()[1], pT->Pos()[2] ));
        if(pT->nodeType == NODE_TRANSFORMSIMPLE)
	        for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	            PRINTF((TEXT("| scale : %f %f %f\n"), pT->asTransfSimple()->Scale()[0], pT->asTransfSimple()->Scale()[1], pT->asTransfSimple()->Scale()[2] ));
        if(pT->nodeType == NODE_TRANSFORM)
    	    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	            PRINTF((TEXT("| Rot : %f %f %f\n"), pT->asTransf()->TransformData().rotation[0], pT->asTransf()->TransformData().rotation[1], pT->asTransf()->TransformData().rotation[2]));
    		
	    if(pT->pFloatArrays)
	    {
		    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
		    PRINTF(( TEXT("| sources connected to the transform: \n") ));
		    for(int j=0; j<pT->pFloatArrays->n; j++)
		    {
			    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
			    PRINTF(( TEXT("| ") FSTR TEXT(" <== Curve ") FSTR TEXT("\n"), pT->pFloatArrays->p[j].destName, pT->pFloatArrays->p[j].p ? pT->pFloatArrays->p[j].p->name : "NO SOURCE" ));
		    }
		    PRINTF(( TEXT("\n") ));
	    }
    }
passThePrint:
	if(pT->getNumChildren()>0)
	  for(int i=0; i<(int)pT->getNumChildren(); i++)
		Transform_debugDumpLayout(pT->getChild(i), l+1, level, nodeNameFilter); // recusive display for children...
}
 
//------------------------------------------------------------------------------------------
// 
/// Debug dumps all what is inside the Bk3d file
// 
//------------------------------------------------------------------------------------------
INLINE void FileHeader_debugDumpAll(FileHeader* pH, int level, const char * nodeNameFilter)
{
	if(pH->pTransforms) 
		for(int i=0; i< pH->pTransforms->nBones; i++)
		{
			Transform_debugDumpLayout(pH->pTransforms->pBones[i], 0, level, nodeNameFilter);
		}
	PRINTF((TEXT("\n")));
	if(pH->pMaterials)
		for(int i=0; i< pH->pMaterials->nMaterials; i++)
		{
            if(nodeNameFilter && (strcmp(nodeNameFilter, pH->name)))
              continue;
			Material *pM = pH->pMaterials->pMaterials[i];
			PRINTF((TEXT("Material node : ") FSTR TEXT("\n"), pM->name));
            if(level>0)
            {
                MaterialData &matData = pM->MaterialData();
                PRINTF((TEXT("\tdiffuse : %f,%f,%f\n"), matData.diffuse[0], matData.diffuse[1], matData.diffuse[2] ));
                PRINTF((TEXT("\tspecexp : %f\n"), matData.specexp ));
                PRINTF((TEXT("\tambient : %f,%f,%f\n"), matData.ambient[0], matData.ambient[1], matData.ambient[2] ));
                PRINTF((TEXT("\treflectivity : %f\n"), matData.reflectivity ));
                PRINTF((TEXT("\ttransparency : %f,%f,%f\n"), matData.transparency[0], matData.transparency[1], matData.transparency[2] ));
                PRINTF((TEXT("\ttranslucency : %f\n"), matData.translucency ));
                PRINTF((TEXT("\tspecular : %f,%f,%f\n"), matData.specular[0], matData.specular[1], matData.specular[2] ));
                if(pM->shaderName)
                { PRINTF((TEXT("\tShader Name : %s\n"), pM->shaderName )); }
                if(pM->techniqueName)
                { PRINTF((TEXT("\tTechnique Name : %s\n"), pM->techniqueName )); }
                #define _PRINTTEXTURE(t) if(pM->t.name) {\
                    PRINTF((TEXT("\t") TEXT(#t) TEXT(": ") FSTR TEXT(" : ") FSTR TEXT("\n"), pM->t.name, pM->t.filename)); }
                _PRINTTEXTURE(diffuseTexture)
                _PRINTTEXTURE(specExpTexture);
                _PRINTTEXTURE(ambientTexture);
                _PRINTTEXTURE(reflectivityTexture);
                _PRINTTEXTURE(transparencyTexture);
                _PRINTTEXTURE(translucencyTexture);
                _PRINTTEXTURE(specularTexture);
            }
        }

	if(pH->pMayaCurves)
		for(int i=0; i< pH->pMayaCurves->n; i++)
		{
			Ptr64<MayaCurveVector> p = pH->pMayaCurves->p[i];
			PRINTF((TEXT("Maya Curve vector : ") FSTR 
				TEXT("%d curves \n"), p->name, p->nCurves));
		}

	unsigned int nVtx = 0;
	unsigned int nElts = 0;
	unsigned int nPrims = 0;
	if(pH->pMeshes)
		for(int i=0; i< pH->pMeshes->n; i++)
		{
			Mesh_debugDumpLayout(pH->pMeshes->p[i], nVtx, nElts, nPrims, level, nodeNameFilter);
		}
	PRINTF((TEXT("\n Vertices= %d ; Elements= %d ; Prims= %d\n\n"), nVtx, nElts, nPrims));
}

//------------------------------------------------------------------------------------------
// 
/// returns Strings for various formats used in the file
// 
//------------------------------------------------------------------------------------------
INLINE const char * formatAsString(DXGI_FORMAT fmt)
{
    switch(fmt)
    {
    default:
        return "Bad format code";
    case DXGI_FORMAT_UNKNOWN:
        return "FORMAT_UNKNOWN";
    case DXGI_FORMAT_R32_FLOAT:
        return "FORMAT_R32_FLOAT";
    case DXGI_FORMAT_R32G32_FLOAT:
        return "FORMAT_R32G32_FLOAT";
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return "FORMAT_R32G32B32_FLOAT";
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return "FORMAT_R32G32B32A32_FLOAT";
    case DXGI_FORMAT_R16_UINT:
        return "FORMAT_R16_UINT";
    case DXGI_FORMAT_R32_UINT:
        return "FORMAT_R32_UINT";
    case DXGI_FORMAT_R32G32_UINT:
        return "FORMAT_R32G32_UINT";
    case DXGI_FORMAT_R32G32B32_UINT:
        return "FORMAT_R32G32B32_UINT";
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return "FORMAT_R32G32B32A32_UINT";
    case DXGI_FORMAT_R8_UINT:
        return "DXGI_FORMAT_R8_UINT";
    case DXGI_FORMAT_R8G8_UINT:
        return "DXGI_FORMAT_R8G8_UINT";
    case DXGI_FORMAT_R8G8B8A8_UINT:
        return "DXGI_FORMAT_R8G8B8A8_UINT";
    }
}
//------------------------------------------------------------------------------------------
// 
/// debug dump : part handled by the Mesh
// 
//------------------------------------------------------------------------------------------
INLINE void Mesh_debugDumpLayout(Mesh *pM, unsigned int &nVtx, unsigned int &nElts, unsigned int &nPrims, int level, const char * nodeNameFilter)
{
  if(nodeNameFilter && (strcmp(nodeNameFilter, pM->name)))
      return;
  if(level==0)
  {
    PRINTF((TEXT("Mesh : ") FSTR, pM->name));
    if(pM->pTransforms)
    {
        PRINTF((TEXT(" ,Transf: ")));
        for(int i=0; i<pM->pTransforms->n; i++)
        {
            PRINTF((TEXT(" ") FSTR, pM->pTransforms->p[i]->name ));
        }
    }
    PRINTF((TEXT("\n")));
    if(pM->pPrimGroups)
    {
        PrimGroup *pCurPrimGroup = pM->pPrimGroups->p[0];
        for(int i=0; i<pM->pPrimGroups->n; i++, pCurPrimGroup = pM->pPrimGroups->p[i])
        {
            PRINTF((TEXT("    PG %d : ") FSTR TEXT("("), i, pCurPrimGroup->name));
            if(pCurPrimGroup->pMaterial)
            {
              PRINTF((TEXT(" mat: ") FSTR, pCurPrimGroup->pMaterial->name));
            }
            for(int i=0; i<pCurPrimGroup->pTransforms->n; i++)
            {
                PRINTF((TEXT(" Transf: ") FSTR, pCurPrimGroup->pTransforms->p[i]->name ));
            }
            PRINTF((TEXT(")\n")));
        }
    }
    return;
  }
    PRINTF((TEXT("\n-------------------------------------\nMesh Infos : ") FSTR TEXT("\n\n"), pM->name));
  if(pM->pTransforms)
  {
	PRINTF((TEXT(" Attached to Transform(s) ( many transforms == use of skinning)\n") ));
	for(int i=0; i<pM->pTransforms->n; i++)
	{
	  PRINTF((TEXT(" ") FSTR, pM->pTransforms->p[i]->name ));
	}
  }
  if(pM->pAttributes && pM->pAttributes->n > 0)
  {
    PRINTF((TEXT("\n\n%d Attributes: \n"), pM->pAttributes->n));
    Attribute *pCurAttr = pM->pAttributes->p[0].p;
    for(int j=0; j<pM->pAttributes->n; j++, pCurAttr = pM->pAttributes->p[j].p)
    {
       PRINTF((FSTR TEXT(" : strideBytes %d, format %s, "), 
		   pCurAttr->name, 
		   pCurAttr->strideBytes, 
		   formatAsString(pCurAttr->formatDXGI) ));
	   PRINTF((TEXT("alignedByteOffset:%d dataOffsetBytes:%d formatGL:%d numComp:%d semanticIdx:%d slot:%d strideBytes:%d \n"), 
		pCurAttr->alignedByteOffset, pCurAttr->dataOffsetBytes,pCurAttr->formatGL,pCurAttr->numComp,pCurAttr->semanticIdx,
		pCurAttr->slot,pCurAttr->strideBytes ));
    }
  }
  if(pM->pSlots && pM->pSlots->n > 0)
  {
    PRINTF((TEXT("\n%d Slots (Streams) : \n"), pM->pSlots->n));
    Slot *pCurSlot = pM->pSlots->p[0];
	nVtx += pCurSlot->vertexCount;
    for(int j=0; j<pM->pSlots->n; j++, pCurSlot = pM->pSlots->p[j])
    {
        PRINTF((TEXT("Slot %d : ") FSTR TEXT(", "), j, pCurSlot->name));
		PRINTF((TEXT("vertexCount:%d vtxBufferSizeBytes:%d vtxBufferStrideBytes:%d\n"), 
			pCurSlot->vertexCount, pCurSlot->vtxBufferSizeBytes, pCurSlot->vtxBufferStrideBytes ));
		PRINTF((TEXT("Vertex count : %d\n"), pCurSlot->vertexCount));
        PRINTF((TEXT("Attributes in the Slot : \n"), pCurSlot->pAttributes->n));
        Attribute *pCurAttr = pCurSlot->pAttributes->p[0].p;
        for(int i=0; i<pCurSlot->pAttributes->n; i++, pCurAttr = pCurSlot->pAttributes->p[i].p)
        {
            PRINTF((FSTR TEXT(" : strideBytes %d, format %s\n"), pCurAttr->name, pCurAttr->strideBytes, formatAsString(pCurAttr->formatDXGI) ));
            float* pF = NULL;
            double* pD = NULL;
            short * pS = NULL;
            unsigned short * pUS = NULL;
            int * pI = NULL;
            unsigned int * pUI = NULL;
            switch(pCurAttr->formatGL)
            {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
                pS = (short*)pCurAttr->pAttributeBufferData;
                break;
            case GL_UNSIGNED_SHORT:
                pUS = (unsigned short*)pCurAttr->pAttributeBufferData;
                break;
            case GL_INT:
                pI = (int*)pCurAttr->pAttributeBufferData;
                break;
            case GL_UNSIGNED_INT:
                pUI = (unsigned int*)pCurAttr->pAttributeBufferData;
                break;
            case GL_2_BYTES:
            case GL_3_BYTES:
            case GL_4_BYTES:
			    PRINTF((TEXT("Debug print of this format not supported, yet\n")));
                break;
            case GL_DOUBLE:
                pD = (double*)pCurAttr->pAttributeBufferData;
                break;
            case GL_FLOAT:
                pF = (float*)pCurAttr->pAttributeBufferData;
                break;
            }
			if(level > 1) for(int j=0; j<(int)pCurSlot->vertexCount; j++)
			{
			    PRINTF((TEXT("(")));
				for(int k=0; k<pCurAttr->numComp; k++)
				{
                    if(pF) { PRINTF((TEXT("%f "), pF[k] )); }
                    if(pD) { PRINTF((TEXT("%f "), pD[k] )); }
                    if(pI) { PRINTF((TEXT("%d "), pI[k] )); }
                    if(pUI) { PRINTF((TEXT("%d "), pUI[k] )); }
                    if(pS) { PRINTF((TEXT("%d "), pS[k] )); }
                    if(pUS) { PRINTF((TEXT("%d "), pUS[k] )); }
				}
			    PRINTF((TEXT(")")));
                if((j % 4) == 0) {
                  PRINTF((TEXT("\n") ));
                }
				if(pF) pF = (float*)((char*)pF + pCurAttr->strideBytes);
				if(pD) pD = (double*)((char*)pD + pCurAttr->strideBytes);
				if(pS) pS = (short*)((char*)pS + pCurAttr->strideBytes);
				if(pUS) pUS = (unsigned short*)((char*)pUS + pCurAttr->strideBytes);
				if(pI) pI = (int*)((char*)pI + pCurAttr->strideBytes);
				if(pUI) pUI = (unsigned int*)((char*)pUI + pCurAttr->strideBytes);
			}
			PRINTF((TEXT("\n") ));
        }
    }
  }
  if(pM->pBSSlots && pM->pBSSlots->n > 0)
  {
    PRINTF((TEXT("\n%d numBlendShapes (==Slots) : \n"), pM->pBSSlots->n));
    Slot *pCurBSSlot = pM->pBSSlots->p[0];
    for(int j=0; j<pM->pBSSlots->n; j++, pCurBSSlot = pM->pBSSlots->p[j])
    {
        PRINTF((TEXT("BlendShape %d : ") FSTR TEXT("\n"), j, pCurBSSlot->name));
		PRINTF((TEXT("Vertex count : %d\n"), pCurBSSlot->vertexCount));
        PRINTF((TEXT("Attributes in the BlendShape : \n"), pCurBSSlot->pAttributes->n));
        Attribute *pCurAttr = pCurBSSlot->pAttributes->p[0].p;
        for(int i=0; i<pCurBSSlot->pAttributes->n; i++, pCurAttr = pCurBSSlot->pAttributes->p[i].p)
        {
          PRINTF((FSTR TEXT(" : strideBytes %d, format %s\n"), pCurAttr->name, pCurAttr->strideBytes, formatAsString(pCurAttr->formatDXGI) ));
        }
    }
  }
 if(pM->pPrimGroups)
 {
	 PrimGroup *pCurPrimGroup = pM->pPrimGroups->p[0];
	 for(int i=0; i<pM->pPrimGroups->n; i++, pCurPrimGroup = pM->pPrimGroups->p[i])
	  {
		  PRINTF((TEXT("Prim group %d : ") FSTR TEXT("\n"), i, pCurPrimGroup->name));
		  if(pCurPrimGroup->pMaterial)
		  {
			  PRINTF((TEXT("\tPrimitive material : ") FSTR  TEXT("\n"), pCurPrimGroup->pMaterial->name));
		  }
		  nElts += pCurPrimGroup->indexCount;
		  nPrims += pCurPrimGroup->primitiveCount;
		  PRINTF((TEXT("\tnumelements = %d\n"), pCurPrimGroup->indexCount));
		  PRINTF((TEXT("\tindexPerVertex = %d\n"), pCurPrimGroup->indexPerVertex));
		  PRINTF((TEXT("\tminelement = %d\n"), pCurPrimGroup->minIndex));
		  PRINTF((TEXT("\tmaxelement = %d\n"), pCurPrimGroup->maxIndex));
		  PRINTF((TEXT("\tprimitiveCount = %d\n"), pCurPrimGroup->primitiveCount));
		  PRINTF((TEXT("\tPrimitive type = ")));
		  // special case of Quad - OGL Only
		  if(pCurPrimGroup->topologyGL == GL_QUADS)
		  {
			  PRINTF((TEXT("QUAD LIST (Warning : OpenGL ONLY)")));
		  }
		  else if(pCurPrimGroup->topologyGL == GL_QUAD_STRIP)
		  {
			  PRINTF((TEXT("QUAD STRIP (Warning : OpenGL ONLY)")));
		  }
		  else if(pCurPrimGroup->topologyGL == GL_TRIANGLE_FAN)
		  {
			  PRINTF((TEXT("TRIANGLE FAN (Warning : OpenGL ONLY)")));
		  }
		  else switch(pCurPrimGroup->topologyDX11)
		  {
		  case D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED: PRINTF((TEXT("UNDEFINED"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_POINTLIST:PRINTF((TEXT("POINT LIST"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_LINELIST:PRINTF((TEXT("LINE LIST"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP: PRINTF((TEXT("LINE STRIP"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST: PRINTF((TEXT("TRI LIST"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: PRINTF((TEXT("TRI STRIP"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ: PRINTF((TEXT("ADJ LINE LIST"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ: PRINTF((TEXT("ADJ LINE STRIP"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ: PRINTF((TEXT("ADJ TRIANGLE LIST"))); break;
		  case D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ: PRINTF((TEXT("ADJ TRI STRIP"))); break;
		  }
		  PRINTF((TEXT("\n")));
          if((pCurPrimGroup->pOwnerOfIB!=NULL) && (pCurPrimGroup->pOwnerOfIB != pCurPrimGroup))
          {
              PRINTF((TEXT("\tNOTE: Index buffer owned by ") FSTR TEXT("\n"), pCurPrimGroup->pOwnerOfIB->name));
          }
          PRINTF((TEXT("\tIndex start Offset = %d\n"), pCurPrimGroup->indexOffset));
          if((level > 1))// && ((pCurPrimGroup->pOwnerOfIB==NULL) || (pCurPrimGroup->pOwnerOfIB == pCurPrimGroup)))
          {
            short * pS = NULL;
            unsigned short * pUS = NULL;
            int * pI = NULL;
            unsigned int * pUI = NULL;
            switch(pCurPrimGroup->indexFormatGL)
            {
            case GL_SHORT:
                pS = (short*)pCurPrimGroup->pIndexBufferData;
                break;
            case GL_UNSIGNED_SHORT:
                pUS = (unsigned short*)pCurPrimGroup->pIndexBufferData;
                break;
            case GL_INT:
                pI = (int*)pCurPrimGroup->pIndexBufferData;
                break;
            case GL_UNSIGNED_INT:
                pUI = (unsigned int*)pCurPrimGroup->pIndexBufferData;
                break;
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_2_BYTES:
            case GL_3_BYTES:
            case GL_4_BYTES:
            case GL_DOUBLE:
            case GL_FLOAT:
                PRINTF((TEXT("Debug print of this format not supported, yet\n")));
                break;
            }
            int N = pCurPrimGroup->indexOffset + pCurPrimGroup->indexCount;
            for(int j=pCurPrimGroup->indexOffset; j<N; j++)
            {
                if(pI) { PRINTF((TEXT("%d "), pI[j] )); }
                if(pUI) { PRINTF((TEXT("%d "), pUI[j] )); }
                if(pS) { PRINTF((TEXT("%d "), pS[j] )); }
                if(pUS) { PRINTF((TEXT("%d "), pUS[j] )); }
                if(j&&((j % 8) == 0)) {
                  PRINTF((TEXT("\n") ));
                }
            }
            PRINTF((TEXT("\n") ));
          }
      }// for pPrimGroups->n
  }
  PRINTF((TEXT("\nBounding Sphere : p=(%f, %f %f), R= %f\n"),
   pM->bsphere.pos[0], pM->bsphere.pos[1], pM->bsphere.pos[2], pM->bsphere.radius));
  PRINTF((TEXT("AABB : min=(%f, %f %f), max=(%f, %f %f)\n"),
    pM->aabbox.min[0], pM->aabbox.min[1], pM->aabbox.min[2], pM->aabbox.max[0], pM->aabbox.max[1], pM->aabbox.max[2]));

  PRINTF((TEXT("\n--------------------------------\n")));
}

} //namespace bk3d
#endif //__BK3DMETHODS__
