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
#ifndef __D3D9RawMesh_h__
#define __D3D9RawMesh_h__
#include "meshfileheader.h"

#ifndef LOGMSG
#	define LOG_MSG 0
#	define LOG_WARN 1
#	define LOG_ERR 2
#define LOGMSG(a)
#endif
// structures

/*----------------------------------------------------------------------------------*/ /**


**/ //----------------------------------------------------------------------------------

class D3D9RawMesh
{
public:
	D3D9RawMesh() : 
		_pg(NULL),
		_mesh(NULL), 
		_valid(false),
		_pDeclaration(NULL),
		_pVB(NULL),
		_pNB(NULL),
		_pCB(NULL),
		_pTanB(NULL),
		_pBinB(NULL)
		{
			for(int i=0; i<8; i++) _pTB[i]=NULL;
		}
	~D3D9RawMesh() { Cleanup(); }
	void Cleanup()
	{ 
		SAFE_RELEASE(_pDeclaration);
		SAFE_RELEASE(_pVB);
		SAFE_RELEASE(_pNB);
		SAFE_RELEASE(_pCB);
		for(int i=0; i<8; i++) { SAFE_RELEASE(_pTB[i]); }
		SAFE_RELEASE(_pTanB);
		SAFE_RELEASE(_pBinB);
		if(_mesh)
			for(unsigned int i=0; i<_mesh->num_primgroup; i++)
			{
				if(!_pg)
					continue;
				SAFE_RELEASE(_pg[i]._pIB_tris);
				SAFE_RELEASE(_pg[i]._pIB_strips);
				SAFE_RELEASE(_pg[i]._pIB_fans);
			}
		if(_mesh) free(_mesh); _mesh = NULL; _valid = false; 
		if(_pg) delete [] _pg;
		_pg = NULL;
	}

	/**********************************************
	 * Load from file
	 **/
	bool loadModel(LPCSTR fname)
	{
		FILE *fd;
    LOGMSG(LOG_MSG, "D3D9RawMesh: loading %s...", fname);
		fd = fopen(fname, "rb");
		if(!fd)
    {
      LOGMSG(LOG_WARN, "D3D9RawMesh: couldn't load %s", fname);
			return false;
    }
		int offs = 0;
		char * memory = (char*)malloc(1024);
		int n;
		while(n=fread(memory + offs, 1024, 1, fd))
		{
			offs += 1024;
			memory = (char*)realloc(memory, 1024 + offs);
		}
		fclose(fd);
		if(strncmp(memory, "MESH", 4))
		{
			free(memory);
			return false;
		}
		_mesh = (FileHeader *)memory;
		_pg = new PrimGroup[_mesh->num_primgroup];
		_valid = true;
		return true;
	}
	/**********************************************
	 * Setup the mesh : vertex buffers etc.
	 **/
	HRESULT setupMesh(LPDIRECT3DDEVICE9 pd3dDevice)
	{
		HRESULT hr;
		void *data;
		if((!pd3dDevice)||(!_valid))
			return S_OK;
		_pd3dDevice = pd3dDevice;
		SAFE_RELEASE(_pVB);
		SAFE_RELEASE(_pNB);
		SAFE_RELEASE(_pCB);
		for(int i=0; i<8; i++) { SAFE_RELEASE(_pTB[i]); }
		if(_mesh)
			for(unsigned int i=0; i<_mesh->num_primgroup; i++)
			{
				if(!_pg)
					continue;
				SAFE_RELEASE(_pg[i]._pIB_tris);
				SAFE_RELEASE(_pg[i]._pIB_strips);
				SAFE_RELEASE(_pg[i]._pIB_fans);
			}
		//
		// Create vertex buffers and index buffers
		//
		{
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating Vtx buffer : %d elements", _mesh->vtx.num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->vtx.num_components*_mesh->vtx.num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pVB, NULL)))
				return hr;
			if (FAILED(hr = _pVB->Lock(0, 0, &data, 0))) 
				return hr;
			float *pul = (float *)data;
			float *pul2 = (float *)(_mesh->data + _mesh->vtx.ptr_offset);
			for(unsigned int i=0; i<_mesh->vtx.num_elements*_mesh->vtx.num_components; i++, pul++, pul2++)
				*pul = *pul2;
			//memcpy(data, (_mesh->data + _mesh->offs_vtx), sizeof(float)*3*_mesh->num_vtx);
			if (FAILED(hr = _pVB->Unlock()))
				return hr;
		}
		if(_mesh->normal.num_elements > 0)
		{
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating Normal buffer : %d elements", _mesh->normal.num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->normal.num_components*_mesh->normal.num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pNB, NULL)))
				return hr;
			if (FAILED(hr = _pNB->Lock(0, 0, &data, 0))) 
				return hr;
			memcpy(data, (_mesh->data + _mesh->normal.ptr_offset), sizeof(float)*_mesh->normal.num_components*_mesh->normal.num_elements);
			if (FAILED(hr = _pNB->Unlock()))
				return hr;
		}
		if(_mesh->color.num_elements > 0)
		{
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating Color buffer : %d elements", _mesh->color.num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->color.num_components*_mesh->color.num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pCB, NULL)))
				return hr;
			if (FAILED(hr = _pCB->Lock(0, 0, &data, 0))) 
				return hr;
			memcpy(data, (_mesh->data + _mesh->color.ptr_offset), sizeof(float)*_mesh->color.num_components*_mesh->color.num_elements);
			if (FAILED(hr = _pCB->Unlock()))
				return hr;
		}
		for(int i=0; i<4; i++)
		  if(_mesh->uv[i].num_elements > 0)
		  {
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating Texcoords[%d] buffer : %d elements", i, _mesh->uv[i].num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->uv[i].num_components*_mesh->uv[i].num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pTB[i], NULL)))
				return hr;
			if (FAILED(hr = _pTB[i]->Lock(0, 0, &data, 0))) 
				return hr;
			memcpy(data, (_mesh->data + _mesh->uv[i].ptr_offset), sizeof(float)*_mesh->uv[i].num_components*_mesh->uv[i].num_elements);
			if (FAILED(hr = _pTB[i]->Unlock()))
				return hr;
		  } 
		if(_mesh->tangent.num_elements > 0 /*and num of binormals*/)
		{
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating Tangent buffer : %d elements", _mesh->tangent.num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->tangent.num_components*_mesh->tangent.num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pTanB, NULL)))
				return hr;
			if (FAILED(hr = _pTanB->Lock(0, 0, &data, 0))) 
				return hr;
			memcpy(data, (_mesh->data + _mesh->tangent.ptr_offset), sizeof(float)*_mesh->tangent.num_components*_mesh->tangent.num_elements);
			if (FAILED(hr = _pTanB->Unlock()))
				return hr;
      LOGMSG(LOG_MSG, "D3D9RawMesh: creating binormal buffer : %d elements", _mesh->binormal.num_elements);
			if (FAILED(hr = pd3dDevice->CreateVertexBuffer(sizeof(float)*_mesh->binormal.num_components*_mesh->binormal.num_elements , D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &_pBinB, NULL)))
				return hr;
			if (FAILED(hr = _pBinB->Lock(0, 0, &data, 0))) 
				return hr;
			memcpy(data, (_mesh->data + _mesh->binormal.ptr_offset), sizeof(float)*_mesh->binormal.num_components*_mesh->binormal.num_elements);
			if (FAILED(hr = _pBinB->Unlock()))
				return hr;
		}
		//
		// 32-bit indices 
		// TODO : 16 bits
		//
		for(unsigned int i=0; i<_mesh->num_primgroup; i++)
		{
			PrimGroup *pg = _pg + i;
			FileHeader::PrimGroup *meshpg = _mesh->primgroup + i;
      LOGMSG(LOG_MSG, "D3D9RawMesh: processing primitive group %d...", i);
			if(meshpg->strips.num_elements)
			{
        LOGMSG(LOG_MSG, "D3D9RawMesh: create tri-strips index buffer : %d elements", meshpg->strips.num_elements);
				if (FAILED(hr = pd3dDevice->CreateIndexBuffer(4*meshpg->strips.num_elements , D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pg->_pIB_strips, NULL)))
					return hr;
				if(FAILED(hr = pg->_pIB_strips->Lock(0, 0, &data, 0)))
					return hr;
				unsigned long *pul = (unsigned long *)data;
				unsigned long *pul2 = (unsigned long *)(_mesh->data + meshpg->strips.offset);
				for(unsigned int i=0; i<meshpg->strips.num_elements; i++, pul++, pul2++)
					*pul = *pul2;
				//memcpy(data, (_mesh->data + meshpg->strips.offset), sizeof(unsigned long)*meshpg->strips.num_elements);
				if (FAILED(hr = pg->_pIB_strips->Unlock()))
					return hr;
			}
			if(meshpg->tris.num_elements)
			{
        LOGMSG(LOG_MSG, "D3D9RawMesh: create triangles index buffer : %d elements", meshpg->tris.num_elements);
				if (FAILED(hr = pd3dDevice->CreateIndexBuffer(4*meshpg->tris.num_elements , D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pg->_pIB_tris, NULL)))
					return hr;
				if(FAILED(hr = pg->_pIB_tris->Lock(0, 0, &data, 0)))
					return hr;
				memcpy(data, (_mesh->data + meshpg->tris.offset), sizeof(unsigned long)*meshpg->tris.num_elements);
				if (FAILED(hr = pg->_pIB_tris->Unlock()))
					return hr;
			}
			if(meshpg->fans.num_elements)
			{
        LOGMSG(LOG_MSG, "D3D9RawMesh: create fans index buffer : %d elements", meshpg->fans.num_elements);
				if (FAILED(hr = pd3dDevice->CreateIndexBuffer(4*meshpg->fans.num_elements , D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED, &pg->_pIB_fans, NULL)))
					return hr;
				if(FAILED(hr = pg->_pIB_fans->Lock(0, 0, &data, 0)))
					return hr;
				memcpy(data, (_mesh->data + meshpg->fans.offset), sizeof(unsigned long)*meshpg->fans.num_elements);
				if (FAILED(hr = pg->_pIB_fans->Unlock()))
					return hr;
			}
		}
		//
		// vertex declaration : depending on what we have
		//
		// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
		// impement the stride version
		bool bIsStream = true;
		// TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
		D3DVERTEXELEMENT9 declaration[20];
		D3DVERTEXELEMENT9 *p = declaration;
		int offset = 0;
		int stream = 0;
		#define ADD_ATTR(a,b,c,d,e,f) { p->Stream = a; p->Offset = b; p->Type = c; p->Method = d; p->Usage = e;p->UsageIndex = f; p++; }
		#define END_ATTR() {p->Stream=0xFF;p->Offset=0;p->Type=D3DDECLTYPE_UNUSED;p->Method=0;p->Usage=0;p->UsageIndex=0;}

		LOGMSG(LOG_MSG, "D3D9RawMesh: array Vtx : stream %d, Offset %d", stream, offset);
		ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 );
		if(bIsStream) stream++; else offset += 12;
		if(_mesh->normal.num_elements > 0)
		{
			LOGMSG(LOG_STATE, "D3D9RawMesh: array Normals : stream %d, Offset %d", stream, offset);
			ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 );  
			if(bIsStream) stream++; else offset += 12;
		}
		if(_mesh->color.num_elements > 0)
		{
			LOGMSG(LOG_STATE, "D3D9RawMesh: array Colors : stream %d, Offset %d", stream, offset);
			ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 );  
			if(bIsStream) stream++; else offset += 16;
		}
		for(int i=0; i<4; i++)
		  if(_mesh->uv[i].num_elements > 0)
		  {
		    // TODO : find a better way for this. Texcoord1&2 are for Tan & binorm
			int unit = i > 0 ? i+2 : i;
			LOGMSG(LOG_STATE, "D3D9RawMesh: array TexCoords[%d] : stream %d, Offset %d", i, stream, offset);
			ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, unit );
			if(bIsStream) stream++; else offset += 12;
		  }
		if(_mesh->tangent.num_elements > 0)
		{
			LOGMSG(LOG_STATE, "D3D9RawMesh: array Tangents : stream %d, Offset %d", stream, offset);
			ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD/*TANGENT ??*/, 1 );
			if(bIsStream) stream++; else offset += 12;
			LOGMSG(LOG_STATE, "D3D9RawMesh: array Binormals : stream %d, Offset %d", stream, offset);
			ADD_ATTR( stream, offset, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD/*BINORMAL ?? */, 2 );
			if(bIsStream) stream++; else offset += 12;
		}
		END_ATTR();
		if(FAILED(hr = pd3dDevice->CreateVertexDeclaration(declaration, &_pDeclaration)))
			return hr;
		return S_OK;
	}
	/**********************************************
	 * Draw the mesh : tris, fans, strips.
	 **/
	HRESULT drawMesh(int group=-1)
	{
		HRESULT hr;
		int stream =0;
		if(!_pd3dDevice) return S_OK;
		//
		// vertex declaration
		//
		if (FAILED(hr = _pd3dDevice->SetVertexDeclaration(_pDeclaration))) return hr;
		//
		// All the stream sources
		//
		if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pVB, 0, sizeof(float)*3))) return hr;
		if(_pNB)
		{
			if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pNB, 0, sizeof(float)*3))) return hr;
		}
		if(_pCB)
		{
			if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pCB, 0, sizeof(float)*4))) return hr;
		}
		for(int i=0; i<4; i++)
		{
			if(_pTB[i])
			{
				if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pTB[i], 0, sizeof(float)*3))) return hr;
			}
		}
		if(_pTanB && _pBinB)
		{
			if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pTanB, 0, sizeof(float)*3))) return hr;
			if (FAILED(hr = _pd3dDevice->SetStreamSource(stream++, _pBinB, 0, sizeof(float)*3))) return hr;
		}
		//
		// rendering strips
		//
		if(group == -1)
		{
			for(unsigned int i=0; i< _mesh->num_primgroup; i++)
				if (FAILED(hr = drawIndexedPrimitive(i)))
					return hr;
		}
		else
		{
			if(group >= _mesh->num_primgroup)
				return S_FALSE;
			if (FAILED(hr = drawIndexedPrimitive(group)))
				return hr;
		}
		return S_OK;
	}
protected:
	HRESULT drawIndexedPrimitive(int group)
	{
		HRESULT hr;
		PrimGroup *pg = _pg + group;
		FileHeader::PrimGroup *meshpg = _mesh->primgroup + group;
		if(pg->_pIB_strips)
		{
			if (FAILED(hr = _pd3dDevice->SetIndices(pg->_pIB_strips))) 
				return hr;
			if (FAILED(hr = _pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 
				0,								// offset index
				meshpg->strips.min_element, meshpg->strips.max_element - meshpg->strips.min_element,	// range [MinIndex, MinIndex+NumVertices]
				0, meshpg->strips.num_elements)))	// start index & prim count
				return hr;
		}
		if(pg->_pIB_tris)
		{
			if (FAILED(hr = _pd3dDevice->SetIndices(pg->_pIB_tris))) 
				return hr;
			if (FAILED(hr = _pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
				0,								// offset index
				meshpg->tris.min_element, meshpg->tris.max_element - meshpg->tris.min_element,	// range [MinIndex, MinIndex+NumVertices]
				0, meshpg->tris.num_elements)))	// start index & prim count
				return hr;
		}
		return S_OK;
	}
public:
	//
	// Members variables
	//
	bool							_valid;
	FileHeader *					_mesh;
	LPDIRECT3DDEVICE9				_pd3dDevice;
	IDirect3DVertexDeclaration9*	_pDeclaration;
	LPDIRECT3DVERTEXBUFFER9			_pVB;				// buffer for vertices
	LPDIRECT3DVERTEXBUFFER9			_pNB;				// buffer for normals
	LPDIRECT3DVERTEXBUFFER9			_pCB;				// buffer for colors
	LPDIRECT3DVERTEXBUFFER9			_pTB[8];			// buffers for texture
	LPDIRECT3DVERTEXBUFFER9			_pTanB;				// buffers for tan
	LPDIRECT3DVERTEXBUFFER9			_pBinB;				// buffers for binorm
	struct PrimGroup
	{
		PrimGroup() { memset(this, 0, sizeof(PrimGroup) ); }
		LPDIRECT3DINDEXBUFFER9			_pIB_tris;			// indices for triangles
		LPDIRECT3DINDEXBUFFER9			_pIB_strips;		// indices for strips
		LPDIRECT3DINDEXBUFFER9			_pIB_fans;			// indices for fans
	};
	PrimGroup *_pg;
};

#endif