#include<vector>
#include<map>
#include<set>
#include<string>

//
// TODO: write this source code for each new revision of bk3d format : so we will have a serie of 
// loaders. This will always allow us to read older formats and thus convert them !
//
#include "builder.h"

#ifdef bk3dTransform
#   undef bk3dTransform
#   undef bk3dFileHeader
#   undef bk3dFloatArrayPoolConnection
#   undef bk3dFloatArray
#   undef bk3dFloatArrayPool
#   undef bk3dPrimGroup
#   undef bk3dMesh
#   undef bk3dAttribute
#   undef bk3dMaterial
#   undef bk3dMayaCurveVector
#   undef bk3dMayaCurve
#   undef Mapbk3dTransfTD
#   undef Mapbk3dMatTD
#endif
typedef bk3d::Bone               bk3dBone;
typedef bk3d::TransformSimple    bk3dTransformSimple;
typedef bk3d::Transform          bk3dTransform;
typedef bk3d::FileHeader         bk3dFileHeader;
typedef bk3d::FloatArrayPool::Connection bk3dFloatArrayPoolConnection;
typedef bk3d::FloatArray         bk3dFloatArray;
typedef bk3d::FloatArrayPool     bk3dFloatArrayPool;
typedef bk3d::PrimGroup          bk3dPrimGroup;
typedef bk3d::Mesh               bk3dMesh;
typedef bk3d::Attribute          bk3dAttribute;
typedef bk3d::Material           bk3dMaterial;
typedef bk3d::MayaCurveVector    bk3dMayaCurveVector;
typedef bk3d/*13x*/::MayaCurve          bk3dMayaCurve;
typedef Mapbk3dTransf/*13x*/            Mapbk3dTransfTD;
typedef Mapbk3dMat/*13x*/               Mapbk3dMatTD;

#define BK3DLOAD bk3d/*13x*/::load
#define READBK3DFILE ReadBk3dFile/*_13x*/
//#define GetDataTypeExtern
/*-------------------------------------------------------------------------
	helper to get back the destination of the plug
 */
bk3dlib::TransfComponent GetDestinationComponent(bk3dBone* pbk3dTransf, bk3dFloatArrayPoolConnection *pC)
{
	if(pC->pfTarget == pbk3dTransf->Pos() )
		return bk3dlib::TRANSF_POS;
assert(!"TODO bk3dlib::TransfComponent GetDestinationComponent");
	//if(pC->pfTarget == pbk3dTransf->scale)
	//	return bk3dlib::TRANSF_SCALE;
	//if(pC->pfTarget == pbk3dTransf->rotation)
	//	return bk3dlib::TRANSF_EULERROT;
	//if(pC->pfTarget == pbk3dTransf->)
	//	return bk3dlib::TRANSF_VISIBILITY;

	EPF(("TODO : manage this component connection %s !\n", pC->destName));
	//_asm { int 3 }
	return bk3dlib::TRANSF_UNKNOWNCOMP;
}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::READBK3DFILE(bk3dFileHeader *pHeader)
{
	bk3dFileHeader* fileHeaderSrc = pHeader;
	//
	// Now let's inspect this file and re-build classes that we need to maintain
	// for further manipulations
	//
	//
	// transformpool
	//
	Mapbk3dTransfTD maptransf;
	std::map<bk3dlib::PBone, bk3dBone*> maptransf2;
	if(fileHeaderSrc->pTransforms)
	{
		for(int i=0; i<fileHeaderSrc->pTransforms->nBones; i++)
		{
			bk3dBone* pbk3dTransf = fileHeaderSrc->pTransforms->pBones[i];
            CBone* p = NULL;
            switch(pbk3dTransf->nodeType)
            {
            case NODE_BONE:
                p =new CBone();
                break;
            case NODE_TRANSFORMSIMPLE:
                p =new CTransformSimple();
                break;
            case NODE_TRANSFORM:
                p =new CTransform();
                break;
            }
            m_transforms.push_back(p);
            g_transforms.push_back(p);
            maptransf2[p] = pbk3dTransf;
            maptransf[pbk3dTransf] = p;
            // DOF if exists
            bk3d::TransformDOF* pDOF = pbk3dTransf->getTransformDOF();
            if(pDOF)
            {
                bk3dlib::PTransformDOF pd = p->CreateDOF();
                bk3dlib::TransformDOFMode m;
                m = (bk3dlib::TransformDOFMode)pDOF->mode;
                pd->SetDOFValues(m, &pDOF->DOFAlpha, &pDOF->AxisLimitStart, &pDOF->AxisLimitRange);
            }
		}
		// second paas to resolve children and parent pointers
		for(int i=0; i<fileHeaderSrc->pTransforms->nBones; i++)
		{
			bk3dBone* pbk3dTransf = fileHeaderSrc->pTransforms->pBones[i];
			bk3dlib::PBone p = maptransf[pbk3dTransf];
			Mapbk3dTransfTD::iterator iT;
			iT = maptransf.find(pbk3dTransf->getParent() );
			if(iT != maptransf.end())
			{
				p->SetParent(iT->second);
			}
			// No need to do children : SetParent will implicitly have updated this
			//for(int j=0; j<pbk3dTransf->getNumChildren(); j++)
			//{
			//	iT = maptransf.find(pbk3dTransf->getChild(j));
			//	assert(iT != maptransf.end());
			//	iT->second->SetParent(p);
			//}
		}
	}
	//
	// Materials
	//
	Mapbk3dMatTD mapmat; // to store temporary mapping
	if(fileHeaderSrc->pMaterials)
		for(int i=0; i<fileHeaderSrc->pMaterials->nMaterials; i++)
		{
			CMaterial* p = new CMaterial(fileHeaderSrc->pMaterials->pMaterials[i].p);
			assert(p);
			m_materials[p] = p;
            g_materials[p] = p;
			mapmat[fileHeaderSrc->pMaterials->pMaterials[i].p] = p;
		}

	//
	// IKHandles
	//
	if(fileHeaderSrc->pIKHandles)
		for(int i=0; i<fileHeaderSrc->pIKHandles->n; i++)
		{
			CIKHandle* p = new CIKHandle(maptransf, fileHeaderSrc->pIKHandles->p[i].p);
			assert(p);
			m_ikHandles[p] = p;
            g_ikHandles[p] = p;
		}

	//
	// Meshes
	//
	for(int i=0; i<fileHeaderSrc->pMeshes->n; i++)
	{
        if(g_progressCallback) g_progressCallback("Loading Meshes", i, fileHeaderSrc->pMeshes->n);
		CMesh* p = new CMesh(maptransf, mapmat, fileHeaderSrc->pMeshes->p[i].p);
		assert(p);
		m_meshes[p] = p;
        g_meshes[p] = p;
	}
	//
	// curve pool
	//
	if(fileHeaderSrc->pMayaCurves)
		for(int i=0; i<fileHeaderSrc->pMayaCurves->n; i++)
		{
			CMayaCurveVector* p = new CMayaCurveVector(fileHeaderSrc->pMayaCurves->p[i].p);
			assert(p);
			bk3dFloatArray	*pFArray = fileHeaderSrc->pMayaCurves->p[i]->pFloatArray;
			if(pFArray)
			{
				//
				// look for the FloatArray in transformations
				//
				Mapbk3dTransfTD::iterator iT = maptransf.begin();
				while(iT != maptransf.end())
				{
					bk3dBone* pbk3dTransf = iT->first;
					bk3dFloatArrayPool	*pFArrayPool = pbk3dTransf->pFloatArrays;
					//
					// Walk through connection list
					//
					if(pFArrayPool)
					  for(int j=0; j<pFArrayPool->n; j++)
					  {
						if(pFArrayPool->p[j].p == pFArray) // check if it matches the curve's pointer
						{
							p->Connect(iT->second, GetDestinationComponent(pbk3dTransf, pFArrayPool->p + j) ); 
						}
					  }
					++iT;
				}
			}
			m_mayacurves[p] = p;
            g_mayacurves[p] = p;
		}
	free(fileHeaderSrc);
	return true;
}
/*-------------------------------------------------------------------------
  
 */
bool		CFileHeader::LoadFromBk3dFile(LPCSTR file)
{
	bk3dFileHeader *pData = static_cast<bk3dFileHeader *>(BK3DLOAD(file));
    //pData->pMeshes->n = 1; // for debug...
	// another try with older formats
    // NOTE: this method MUST call the last old format. Which in turn would call the last old before itself...
    // The old format loader is in fact the original old file with minor modifications : 
    // prefix/suffix to params and methods so we don't have conflicts at linkage
#ifdef INCLUDE_OLD_BK3D_VERSIONS
	if(!pData)
		return LoadFromBk3dFile_133(file);
#else
	if(!pData)
		return false;
#endif
	return READBK3DFILE(pData);
}
/*-------------------------------------------------------------------------
  
 */
CFileHeader::CFileHeader(bk3dFileHeader *pHeader)
{
	READBK3DFILE(pHeader);
}

bk3dlib::Topology GetTopology(bk3dPrimGroup *pG)
{
	switch(pG->topologyGL)
	{
	case GL_POINTS:
		return bk3dlib::POINTS;
	case GL_LINES:
		return bk3dlib::LINES;
	case GL_LINE_LOOP:
		return bk3dlib::LINE_LOOP;
	case GL_LINE_STRIP:
		return bk3dlib::LINE_STRIP;
	case GL_TRIANGLES:
		return bk3dlib::TRIANGLES;
	case GL_TRIANGLE_STRIP:
		return bk3dlib::TRIANGLE_STRIP;
	case GL_TRIANGLE_FAN:
		return bk3dlib::TRIANGLE_FAN;
	case GL_QUADS:
		return bk3dlib::QUADS;
	case GL_QUAD_STRIP:
		return bk3dlib::QUAD_STRIP;
	}
	//TODO: switch with other DX fmts and add the rest
	return bk3dlib::UNKNOWN_TOPOLOGY;
}
#ifdef GetDataTypeExtern
extern bk3dlib::DataType GetDataType(GLType type);
#else
bk3dlib::DataType GetDataType(GLType type)
{
	switch(type)
	{
	case GL_BYTE:
		//return bk3dlib::UINT8;
	case GL_UNSIGNED_BYTE:
		return bk3dlib::UINT8;
	case GL_SHORT:
		return bk3dlib::UINT16;
	case GL_UNSIGNED_SHORT:
		return bk3dlib::UINT16;
	case GL_INT:
		//return bk3dlib::UINT32;
	case GL_UNSIGNED_INT:
		return bk3dlib::UINT32;
	case GL_FLOAT:
		return bk3dlib::FLOAT32;
	case GL_2_BYTES:
		//return bk3dlib::;
	case GL_3_BYTES:
		//return bk3dlib::;
	case GL_4_BYTES:
		//return bk3dlib::;
	case GL_DOUBLE:
		//return bk3dlib::;
	default:
		return bk3dlib::UNKNOWN;
	}
	return bk3dlib::UNKNOWN;
}
#endif
/*-------------------------------------------------------------------------
  
 */
CMesh::CMesh(Mapbk3dTransfTD &maptransf, Mapbk3dMatTD &mapmat, bk3dMesh *pMesh)
{
    CMesh();
	m_name = pMesh->name;
    //
    // Num Joint influences in the mesh
    //
	m_maxNumBones = pMesh->numJointInfluence; // max # of bones passed in the attributes
	//
	// Primitive Groups allocation == # of idx buffers
	//
	std::map<bk3dPrimGroup*, CBuffer *> bufferMap; // temporary storage to match buffers
	for(int i=0; i<pMesh->pPrimGroups->n; i++)
	{
		bk3dPrimGroup *pG = pMesh->pPrimGroups->p[i];
		Mapbk3dMatTD::iterator iM = mapmat.find(pG->pMaterial);
		bk3dlib::PMaterial pMat = NULL;
		if(iM != mapmat.end())
			pMat = iM->second;
		bk3dlib::DataType idxType = GetDataType(pG->indexFormatGL);
		int idxperVtx = pG->indexPerVertex > 0 ? pG->indexPerVertex : 1; // seems like it could be 0 by mistake
		int curPGID = 0;
		for(int j=0; j<idxperVtx; j++)
		{
			if(pG->pOwnerOfIB == NULL)
				pG->pOwnerOfIB = pG;
			std::map<bk3dPrimGroup*, CBuffer *>::iterator iT = bufferMap.find(pG->pOwnerOfIB);
			CBuffer *pIdx = NULL;
			if(iT == bufferMap.end() ) // first time we see it...
			{
                pIdx = static_cast<CBuffer*>(bk3dlib::Buffer::CreateIdxBuffer(pG->pOwnerOfIB->name, idxType));
                AttachIndexBuffer(pIdx,1);
                pIdx->setPrimitiveRestartIndex(pG->primRestartIndex);
                bufferMap[pG->pOwnerOfIB] = pIdx;
			} else {
				pIdx = iT->second;// bufferMap[pG->pOwnerOfIB]; // let's do it simple here : the item MUST exist... or its a bug
			}
			pIdx->GotoItem(pG->indexOffset);
            unsigned int N = pG->indexOffset + pG->indexCount;
			switch(idxType)
			{
			case bk3dlib::UINT32: //TODO: accelerate this !
				for(unsigned int k=pG->indexOffset; k<N; k++)
					pIdx->AddData(((unsigned int*)pG->pIndexBufferData)+(k*idxperVtx)+j, 1);
				
			break;
			case bk3dlib::UINT16:
				for(unsigned int k=pG->indexOffset; k<N; k++)
					pIdx->AddData(((unsigned short*)pG->pIndexBufferData)+(k*idxperVtx)+j, 1);
			break;
			default:
				// Err.
			break;
			}
			if(j==0)
			{
				curPGID = CreatePrimGroup(pG->name, pIdx, GetTopology(pG), pMat, pG->indexOffset, pG->indexCount) - 1;
                if(pG->pTransforms && (pG->pTransforms->n > 0))
                {
                    assert(pG->pTransforms->n == 1);
			        Mapbk3dTransfTD::iterator iT = maptransf.find(pG->pTransforms->p[0]);
			        assert(iT != maptransf.end() );
                    m_primgroups[curPGID]->AddTransformReference(iT->second);
                }
			} else {
				AttachIndexBuffer(pIdx, curPGID);// case of multi-index mode...
			}
		}
	}

    //
	// Attributes... all attributes of all Slots together
	// this will be enough to create all the buffers that we need : interleaved or not,
	// it doesn't matter : we get them all separated with the slot info
    //
	for(int i=0; i<pMesh->pAttributes->n; i++)
	{
		bk3dAttribute *pA = pMesh->pAttributes->p[i];
		bk3dlib::DataType attrType = GetDataType(pA->formatGL);
		unsigned int cnt = pMesh->pSlots->p[pA->slot]->vertexCount;
//pA->slot = i;
        CBuffer *pVtx = static_cast<CBuffer*>(bk3dlib::Buffer::CreateVtxBuffer(pA->name, pA->numComp, pA->slot, attrType));
        AttachVtxBuffer(pVtx, false);
		switch(attrType)
		{
		case bk3dlib::FLOAT32:
			for(unsigned int k=0; k<cnt; k++)
				pVtx->AddData((float*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
			break;
		case bk3dlib::UINT32:
			for(unsigned int k=0; k<cnt; k++)
				pVtx->AddData((unsigned int*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
			break;
		case bk3dlib::FLOAT16:
			//TODO
		//	for(unsigned int k=0; k<cnt; k++)
		//		pVtx->AddData(((half*)pA->pAttributeBufferData)+(k*pA->numComp), pA->numComp);
			break;
		case bk3dlib::UINT16:
			for(unsigned int k=0; k<cnt; k++)
				pVtx->AddData((unsigned short*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
			break;
		default:
			// Err.
			break;
		}
	}
	//
	// Slots : use multimap for counting
	//
	// attributes are enough. Just need to gather Slot names
	if(pMesh->pSlots)
		for(int i=0; i<pMesh->pSlots->n; i++)
		{
            SetSlotName(i, pMesh->pSlots->p[i]->name);
        }
	if(pMesh->pBSSlots)
		for(int i=0; i<pMesh->pBSSlots->n; i++)
		{
            SetBSSlotName(i, pMesh->pBSSlots->p[i]->name);
        }
	//
	// BlendShapes Attributes... all attributes of all Slots together
	// this will be enough to create all the buffers that we need : interleaved or not,
	// it doesn't matter : we get them all separated with the slot info
    //
	if(pMesh->pBSAttributes)
		for(int i=0; i<pMesh->pBSAttributes->n; i++)
		{
			bk3dAttribute *pA = pMesh->pBSAttributes->p[i];
			bk3dlib::DataType attrType = GetDataType(pA->formatGL);
			unsigned int cnt = pMesh->pSlots->p[pA->slot]->vertexCount;
            CBuffer *pVtx = static_cast<CBuffer*>(bk3dlib::Buffer::CreateVtxBuffer(pA->name, pA->numComp, pA->slot, attrType));
            AttachVtxBuffer(pVtx, true);
			switch(attrType)
			{
			case bk3dlib::FLOAT32:
				for(unsigned int k=0; k<cnt; k++)
					pVtx->AddData((float*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
				break;
			case bk3dlib::UINT32:
				for(unsigned int k=0; k<cnt; k++)
					pVtx->AddData((unsigned int*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
				break;
			case bk3dlib::FLOAT16:
				//TODO
			//	for(unsigned int k=0; k<cnt; k++)
			//		pVtx->AddData(((half*)pA->pAttributeBufferData)+(k*pA->numComp), pA->numComp);
				break;
			case bk3dlib::UINT16:
				for(unsigned int k=0; k<cnt; k++)
					pVtx->AddData((unsigned short*)(((char*)pA->pAttributeBufferData)+(k*pA->strideBytes)), pA->numComp);
				break;
			default:
				// Err.
				break;
			}
		}

	//
	// Transform references
	//
	if(pMesh->pTransforms)
		for(int i=0; i<pMesh->pTransforms->n; i++)
		{
			bk3dBone *pTransf = pMesh->pTransforms->p[i];
			Mapbk3dTransfTD::iterator iT = maptransf.find(pTransf);
			assert(iT != maptransf.end() );
			m_transformRefs.push_back(iT->second);
		}

	// BBox and BSphere
	memcpy(aabbox.min, pMesh->aabbox.min, sizeof(aabbox.min));
	memcpy(aabbox.max, pMesh->aabbox.max, sizeof(aabbox.max));
	memcpy(bsphere.pos, pMesh->bsphere.pos, sizeof(bsphere.pos));
	bsphere.radius = pMesh->bsphere.radius;
}


CMaterial::CMaterial(bk3dMaterial *pMaterial)
{
	m_name = pMaterial->name;

	pMat = NULL; // the cooked struct
    bk3d::MaterialData& matData = pMaterial->MaterialData();
	diffuse[0] = matData.diffuse[0];
	diffuse[1] = matData.diffuse[1];
	diffuse[2] = matData.diffuse[2];
	specexp = matData.specexp;
	ambient[0] = matData.ambient[0];
	ambient[1] = matData.ambient[1];
	ambient[2] = matData.ambient[2];
	reflectivity = matData.reflectivity;
	transparency[0] = matData.transparency[0];
	transparency[1] = matData.transparency[1];
	transparency[2] = matData.transparency[2];
	translucency = matData.translucency;
	specular[0] = matData.specular[0];
	specular[1] = matData.specular[1];
	specular[2] = matData.specular[2];

	if(pMaterial->shaderName)
		shaderName = std::string(pMaterial->shaderName);
	if(pMaterial->techniqueName)
		techniqueName = std::string(pMaterial->techniqueName);

	#define ASSIGNTEXTURE(n) if(pMaterial->n.name)\
	{ n.name = pMaterial->n.name;\
		n.filename = pMaterial->n.filename;	}
	ASSIGNTEXTURE(diffuseTexture);
	ASSIGNTEXTURE(specExpTexture);
	ASSIGNTEXTURE(ambientTexture);
	ASSIGNTEXTURE(reflectivityTexture);
	ASSIGNTEXTURE(transparencyTexture);
	ASSIGNTEXTURE(translucencyTexture);
	ASSIGNTEXTURE(specularTexture);
}

/*------------------------------------------------------------------
	Transforms
  ------------------------------------------------------------------*/
CTransform::CTransform(bk3dTransform *pTransform)
{
    m_validComps = pTransform->ValidComps();
	m_transform = NULL; // the cooked struct
	m_parentTransf = NULL;
    m_TransfDOF = NULL;

	m_name = pTransform->name;

	m_matrix = mat4f(pTransform->Matrix());     				// resulting matrix
	m_abs_matrix = mat4f(pTransform->MatrixAbs() );     				// resulting matrix
	m_bindpose_matrix = mat4f(pTransform->MatrixInvBindpose() );     				// resulting matrix
	m_pos = vec3f(pTransform->Pos() );
	m_scale = vec3f(pTransform->Scale() );

	m_rotation = vec3f(pTransform->Rotation());    				// Euler Rotation in degres
	m_rotationOrder[0] = pTransform->RotationOrder()[0];  				// 3 chars for "xyz" or any other
	m_rotationOrder[1] = pTransform->RotationOrder()[1];  				// 3 chars for "xyz" or any other
	m_rotationOrder[2] = pTransform->RotationOrder()[2];  				// 3 chars for "xyz" or any other
	m_scalePivot = vec3f(pTransform->ScalePivot());
	m_scalePivotTranslate = vec3f(pTransform->ScalePivotTranslate());
	m_rotationPivot = vec3f(pTransform->RotationPivot());
	m_rotationPivotTranslate = vec3f(pTransform->RotationPivotTranslate());
	m_rotationOrientation = quatf(pTransform->RotationOrientation()); 		//Quaternion
	m_jointOrientation = quatf(pTransform->JointOrientation()); 			//Quaternion

	bDirty = pTransform->getDirty();
}
CTransformSimple::CTransformSimple(bk3dTransformSimple *pTransform)
{
    m_validComps = pTransform->ValidComps();
	m_transform = NULL; // the cooked struct
	m_parentTransf = NULL;
    m_TransfDOF = NULL;

	m_name = pTransform->name;

	m_matrix = mat4f(pTransform->Matrix());     				// resulting matrix
	m_abs_matrix = mat4f(pTransform->MatrixAbs() );     				// resulting matrix
	m_bindpose_matrix = mat4f(pTransform->MatrixInvBindpose() );     				// resulting matrix
	m_pos = vec3f(pTransform->Pos() );
	m_scale = vec3f(pTransform->Scale() );

	bDirty = pTransform->getDirty();
}
CBone::CBone(bk3dBone *pTransform)
{
    bk3d::BoneDataType& boneData = pTransform->BoneData();

    m_validComps = boneData.validComps;
	m_transform = NULL; // the cooked struct
	m_parentTransf = NULL;
    m_TransfDOF = NULL;

	m_name = pTransform->name;

	m_matrix = mat4f(boneData.matrix);     				// resulting matrix
	m_abs_matrix = mat4f(pTransform->MatrixAbs() );     				// resulting matrix
	m_bindpose_matrix = mat4f(pTransform->MatrixInvBindpose() );     				// resulting matrix
	m_pos = vec3f(pTransform->Pos() );
    m_Quat = quatf(boneData.quat);

	bDirty = pTransform->getDirty();
}

CTransformDOF::CTransformDOF(bk3d::TransformDOF *pTransform)
{
    assert(!"TODO");
}

CQuatCurve::CQuatCurve()
{
    m_userData = NULL;
}

CMayaCurveVector::CMayaCurveVector()
{
    //m_parent;
    m_curveVector = NULL; // the cooked struct
    m_preInfinity = bk3dlib::CurveVec::kInfinityConstant;
    m_postInfinity = bk3dlib::CurveVec::kInfinityConstant;
    m_inputIsTime = true;
    m_outputIsAngular = false;
    m_isWeighted = false;
    m_userData = NULL;
}

CMayaCurveVector::CMayaCurveVector(bk3dMayaCurveVector *pMayaCurveVector)
{
	m_name = pMayaCurveVector->name;
	//m_parent = NULL;
	m_curveVector = NULL;
	for(int i=0; i<pMayaCurveVector->nCurves; i++)
	{
		CMayaCurve *pCV = new CMayaCurve(this, pMayaCurveVector->pCurve[i]);
		m_mayacurves.push_back(pCV);
	}
}

CMayaCurve::CMayaCurve(CMayaCurveVector* parent)
{
    m_parent = parent;
    m_curve = NULL;

    m_preInfinity = bk3dlib::CurveVec::kInfinityConstant;
    m_postInfinity = bk3dlib::CurveVec::kInfinityConstant;
    m_inputIsTime = true;
    m_outputIsAngular = false;
    m_isWeighted = false;
}
CMayaCurve::~CMayaCurve()
{
}
CMayaCurve::CMayaCurve(CMayaCurveVector* parent, bk3dMayaCurve *pMayaCurve)
{
	m_name = pMayaCurve->name;
	m_parent = parent;
	m_curve = NULL;

	m_preInfinity = (bk3dlib::CurveVec::EtInfinityType)pMayaCurve->preInfinity;
	m_postInfinity = (bk3dlib::CurveVec::EtInfinityType)pMayaCurve->postInfinity;
	m_inputIsTime = pMayaCurve->inputIsTime;
	m_outputIsAngular = pMayaCurve->outputIsAngular;
	m_isWeighted = pMayaCurve->isWeighted;
	for(int i=0; i<pMayaCurve->nKeys; i++)
	{
		bk3d::MayaReadKey k;
		k.time = pMayaCurve->key[i].time;
		k.value = pMayaCurve->key[i].value;
        k.inTangentType = (bk3d::EtTangentType)pMayaCurve->key[i].inTangentType;
        k.outTangentType = (bk3d::EtTangentType)pMayaCurve->key[i].outTangentType;
		k.inAngle = pMayaCurve->key[i].inAngle;
		k.inWeight = pMayaCurve->key[i].inWeight;
		k.outAngle = pMayaCurve->key[i].outAngle;
		k.outWeight = pMayaCurve->key[i].outWeight;
		m_keys.push_back(k);
	}
}

CIKHandle::CIKHandle(Mapbk3dTransfTD &maptransf, bk3d::IKHandle *pikh)
	: CBone(pikh)
{
    bk3d::IKHandleData& iKHData = pikh->IKHandleData();
    m_name = pikh->name;
    m_priority = iKHData.priority;
    m_weight = iKHData.weight;

    m_effectorTransformStart = NULL;
    m_effectorTransformEnd = NULL;
    //m_handleTransforms = NULL;
    Mapbk3dTransfTD::iterator iT;
    if(pikh->getNumEffectors() > 0)
    {
        iT = maptransf.find(pikh->getEffectorTransform(0) );
        assert(iT != maptransf.end() );
        m_effectorTransformStart = static_cast<CBone*>(iT->second);

        iT = maptransf.find(pikh->getEffectorTransform(pikh->getNumEffectors()-1));
        assert(iT != maptransf.end() );
        m_effectorTransformEnd = static_cast<CBone*>(iT->second);
    }
    //if(pikh->handleTransform)
    //{
    //    iT = maptransf.find(pikh->handleTransform);
    //    assert(iT != maptransf.end() );
    //    m_handleTransforms = static_cast<CTransform*>(iT->second);
    //}

    //m_HandlePos = vec3f(pikh->handlePos);
    //m_EffectorPos = vec3f(pikh->effectorPos);
    m_bk3dHandle = NULL;

	connectedCurveVectors.clear();
}

CPhConstraint::CPhConstraint(Mapbk3dTransfTD &maptransf, bk3d::Constraint *p)
	: CBone(p)
{
    m_name = p->name;
    // TODO
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CPhConstraint::CPhConstraint(Mapbk3dTransfTD &maptransf, bk3d::Constraint *p)")
}

CPhRigidBody::CPhRigidBody(Mapbk3dTransfTD &maptransf, bk3d::RigidBody *p)
	: CBone(p)
{
    m_name = p->name;
    // TODO
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CPhRigidBody::CPhRigidBody(Mapbk3dTransfTD &maptransf, bk3d::RigidBody *p)")
}
