#include<vector>
#include<map>
#include<string>
#include<math.h>

#include "builder.h"

void (__stdcall *g_progressCallback)(LPCSTR s, int curItem, int maxItems) = NULL;
MapFileHeader   g_fileHeaders;
MapMesh         g_meshes;
MapBuffer       g_buffers;
MapMaterial     g_materials;
VecTransform    g_transforms;
MapIKHandle     g_ikHandles;
MapPhRigidBody  g_phRigidBodies;
MapPhConstraint g_phConstraints;
MapCurveVec     g_mayacurves;
MapQuatCurve    g_quatcurves;

namespace bk3dlib
{

/*=========================================================================
  FileHeader static's
 */

void setProgressCallBack(void (__stdcall *progressCallback)(LPCSTR s, int curItem, int maxItems))
{
    g_progressCallback = progressCallback;
}

void DestroyAllObjects(bool bKeepBakedData)
{
    MapMesh::iterator iM =            g_meshes.begin();
    while(iM != g_meshes.end())
    {
        iM->second->Destroy();
        iM = g_meshes.begin();
    }
    g_meshes.clear();
    MapBuffer::iterator iB =            g_buffers.begin();
    while(iB != g_buffers.end())
    {
        iB->second->Destroy();
        iB =            g_buffers.begin();
    }
    g_buffers.clear();
    MapMaterial::iterator iMat =    g_materials.begin();
    while(iMat != g_materials.end())
    {
        iMat->second->Destroy();
        iMat =    g_materials.begin();
    }
    g_materials.clear();
    VecTransform::iterator iTr =    g_transforms.begin();
    while(iTr != g_transforms.end())
    {
        (*iTr)->Destroy();
        iTr =    g_transforms.begin();
    }
    g_transforms.clear();
    MapIKHandle::iterator iIKH =        g_ikHandles.begin();
    while(iIKH != g_ikHandles.end())
    {
        iIKH->second->Destroy();
        iIKH =        g_ikHandles.begin();
    }
    g_ikHandles.clear();
    MapCurveVec::iterator iMCV =        g_mayacurves.begin();
    while(iMCV != g_mayacurves.end())
    {
        iMCV->second->Destroy();
        iMCV =        g_mayacurves.begin();
    }
    g_mayacurves.clear();
    MapFileHeader::iterator iFH =        g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->Destroy(bKeepBakedData);
        iFH =        g_fileHeaders.begin();
    }
    g_fileHeaders.clear();
}

PFileHeader FileHeader::Create(const char * name)
{
    CFileHeader* p = new CFileHeader(name);
    assert(p);
    g_fileHeaders[p] = p;
    return p;
}
PFileHeader FileHeader::CreateFromBk3dFile(LPCSTR file)
{
    bk3d::FileHeader *pData = static_cast<TFileHeader *>(bk3d::load(file));
    if(!pData)
        return NULL;
    CFileHeader* p = new CFileHeader(pData);
    assert(p);
    g_fileHeaders[p] = p;
    return p;
}


bool        FileHeader::FileHeaderDelete(PFileHeader fileheader)
{
    MapFileHeader::iterator iFH = g_fileHeaders.find(fileheader);
    if(iFH == g_fileHeaders.end())
        return false;
    g_fileHeaders.erase(iFH);
    return true;
}

PMesh        Mesh::Create(LPCSTR name)
{
    CMesh* p = new CMesh(name);
    assert(p);
    g_meshes[p] = p;
    return p;
}

PBuffer        Buffer::CreateVtxBuffer(LPCSTR name, int numcomp, int slot, bk3dlib::DataType type)
{
    CBuffer* p = new CBuffer();
    assert(p);
    p->m_name = std::string(name);
    p->m_numcomp = numcomp;
    p->m_slot = slot;
    p->m_type = type;
    p->m_usage = BufferForVtx;
    g_buffers[p] = p;
    return p;
}

PBuffer        Buffer::CreateIdxBuffer(LPCSTR name, bk3dlib::DataType type, int numComp)
{
    CBuffer* p = new CBuffer();
    assert(p);
    p->m_name = std::string(name);
    p->m_type = type;
    p->m_usage = BufferForIdx;
    // Note: this is a way to have multi-indexed buffer : multiple components...
    p->m_numcomp = numComp;
    g_buffers[p] = p;
    return p;
}

PBuffer        Buffer::Create(LPCSTR name)
{
    CBuffer* p = new CBuffer();
    assert(p);
    p->m_name = std::string(name ? name : "Buffer");
    g_buffers[p] = p;
    return p;
}

PTransform        Transform::Create(LPCSTR name)
{
    char* tname = name ? name : "Transform";
    CTransform* p = new CTransform(tname);
    assert(p);
    g_transforms.push_back(p);
    return p;
}

PTransformSimple  TransformSimple::Create(LPCSTR name)
{
    char* tname = name ? name : "TransformSimple";
    CTransformSimple* p = new CTransformSimple(tname);
    assert(p);
    g_transforms.push_back(p);
    return p;
}

PBone        Bone::Create(LPCSTR name)
{
    char* tname = name ? name : "Bone";
    CBone* p = new CBone(tname);
    assert(p);
    g_transforms.push_back(p);
    return p;
}

PMaterial        Material::Create(LPCSTR name)
{
    char* tname = name ? name : "Material";
    CMaterial* p = new CMaterial(tname);
    assert(p);
    g_materials[p] = p;
    return p;
}

PIKHandle IKHandle::Create(LPCSTR name)
{
    CIKHandle* p = new CIKHandle();
    assert(p);
    p->SetName(name);
    g_ikHandles[p] = p;
    g_transforms.push_back(p); // IK Handles are transforms, too
    return p;
}
PIKHandle   IKHandle::CreateRotateInfluence(LPCSTR name)
{
    PIKHandle p = IKHandle::Create(name);
    p->setMode(RotateInfluence);
    return p;
}
PIKHandle   IKHandle::CreateRollInfluence(LPCSTR name)
{
    PIKHandle p = IKHandle::Create(name);
    p->setMode(RollInfluence);
    return p;
}

PPhRigidBody PhRigidBody::Create(LPCSTR name)
{
    CPhRigidBody* p = new CPhRigidBody();
    assert(p);
    p->SetName(name);
    g_phRigidBodies[p] = p;
    g_transforms.push_back(p); // are transforms, too
    return p;
}

PPhConstraint PhConstraint::Create(LPCSTR name)
{
    CPhConstraint* p = new CPhConstraint();
    assert(p);
    p->SetName(name);
    g_phConstraints[p] = p;
    g_transforms.push_back(p); // are transforms, too
    return p;
}

}//namespace bk3dlib

short        bk3dlib::bk3dVersion()
{
    return RAWMESHVERSION;
}
/*=========================================================================
  FileHeader
 */
/*-------------------------------------------------------------------------
  
 */

//bk3dlib::PMesh        CFileHeader::CreateMesh(LPCSTR name)
//{
//    std::string strName(name);
//    MapMesh::iterator iM = m_meshes.begin();
//    while(iM != m_meshes.end())
//    {
//        if(iM->second->m_name == strName)
//        {
//            char tmp[6];
//            sprintf(tmp, "%d", m_meshes.size());
//            strName += tmp;
//            break;
//        }
//        ++iM;
//    }
//    CMesh* p = new CMesh(name);
//    assert(p);
//    m_meshes[p] = p;
//    g_meshes[p] = p;
//    return p;
//}

/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachMesh(bk3dlib::PMesh pMesh)
{
    MapMesh::iterator iM = m_meshes.find(pMesh);
    if(iM == m_meshes.end())
        return false;
    m_meshes.erase(iM);
    return true;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachTransform(bk3dlib::PBone pTr)
{
    VecTransform::iterator iT = m_transforms.begin();
    while(iT != m_transforms.end())
    {
        if((*iT) == pTr->AsBone()) {
            m_transforms.erase(iT);
            return true;
        }
        ++iT;
    }
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachIKHandle(bk3dlib::PIKHandle pTr)
{
    MapIKHandle::iterator iT = m_ikHandles.begin();
    while(iT != m_ikHandles.end())
    {
        if(iT->second == pTr)
        {
            m_ikHandles.erase(iT);
            return false;
        }
        ++iT;
    }
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachPhConstraint(bk3dlib::PPhConstraint pTr)
{
    MapPhConstraint::iterator iT = m_phConstraints.begin();
    while(iT != m_phConstraints.end())
    {
        if(iT->second == pTr)
        {
            m_phConstraints.erase(iT);
            return false;
        }
        ++iT;
    }
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachPhRigidBody(bk3dlib::PPhRigidBody pTr)
{
    MapPhRigidBody::iterator iT = m_phRigidBodies.begin();
    while(iT != m_phRigidBodies.end())
    {
        if(iT->second == pTr)
        {
            m_phRigidBodies.erase(iT);
            return false;
        }
        ++iT;
    }
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachCurveVec(bk3dlib::PCurveVec pCv)
{
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - DETACH Code TODO !!")
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachQuatCurve(bk3dlib::PQuatCurve pCv)
{
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - DETACH Code TODO !!")
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool        CFileHeader::DetachMaterial(bk3dlib::PMaterial pMat)
{
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - DETACH Code TODO !!")
    return false;
}
/*-------------------------------------------------------------------------
  
 */

bool CFileHeader::AttachMesh(bk3dlib::PMesh pMesh)
{
    if(pMesh == NULL)
        return false;
    std::string strName(pMesh->GetName());
    MapMesh::iterator iM = m_meshes.begin();
    while(iM != m_meshes.end())
    {
        if(iM->second->m_name == strName)
        {
            char tmp[6];
            sprintf(tmp, "%d", m_meshes.size());
            strName += tmp;
            pMesh->SetName(strName.c_str());
            break;
        }
        ++iM;
    }
    CMesh* pcmesh = static_cast<CMesh*>(pMesh);
    m_meshes[pMesh] = pcmesh;
    //
    // We must also make sure that the referenced materials and transforms are also added to this Header
    //
    int n = pcmesh->GetNumTransformReferences(false);
    for(int i=0; i<n; i++)
    {
        bk3dlib::PBone p = pcmesh->GetTransformReference(i);
        VecTransform::iterator iT = m_transforms.begin();
        while(iT != m_transforms.end() )
        {
            if((*iT) == p) break;
            ++iT;
        }
        if(iT == m_transforms.end() )
            m_transforms.push_back(static_cast<CBone*>(p));
    }

    n = pcmesh->GetNumPrimGroups();
    for(int i=0; i<n; i++)
    {
        bk3dlib::PrimGroup pginfo;
        if(pcmesh->GetPrimGroupInfo(i, pginfo) && pginfo.pMat)
        {
            MapMaterial::iterator iT = m_materials.find(pginfo.pMat);
            if(iT == m_materials.end() )
                m_materials[pginfo.pMat] = static_cast<CMaterial*>(pginfo.pMat);
        }
    }
    return true;
}
/*-------------------------------------------------------------------------
  
 */
//bk3dlib::PMaterial        CFileHeader::CreateMaterial(LPCSTR name)
//{
//    std::string strName(name);
//    MapMaterial::iterator iM = m_materials.begin();
//    while(iM != m_materials.end())
//    {
//        if(iM->second->m_name == strName)
//        {
//            char tmp[6];
//            sprintf(tmp, "%d", m_materials.size());
//            strName += tmp;
//            break;
//        }
//        ++iM;
//    }
//    CMaterial* p = new CMaterial(strName.c_str());
//    assert(p);
//    m_materials[p] = p;
//    g_materials[p] = p;
//    return p;
//}
/*-------------------------------------------------------------------------
  
 */
//bk3dlib::PIKHandle CFileHeader::CreateIKHandle(LPCSTR name)
//{
//    std::string strName(name);
//    MapIKHandle::iterator iIK = m_ikHandles.begin();
//    while(iIK != m_ikHandles.end())
//    {
//        if(iIK->second->m_name == strName)
//        {
//            char tmp[6];
//            sprintf(tmp, "%d", m_ikHandles.size());
//            strName += tmp;
//            break;
//        }
//        ++iIK;
//    }
//    CIKHandle* p = new CIKHandle(strName.c_str());
//    assert(p);
//    m_ikHandles[p] = p;
//    g_ikHandles[p] = p;
//    return p;
//}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::AttachPhConstraint(bk3dlib::PPhConstraint pIKH)
{
    if(!pIKH)
        return false;
    CPhConstraint* p = static_cast<CPhConstraint*>(pIKH);
    MapPhConstraint::iterator iIK = m_phConstraints.begin();
    while(iIK != m_phConstraints.end())
    {
        if(iIK->second->m_name == p->m_name)
            return false;
        ++iIK;
    }
    m_phConstraints[p] = p;
    // Now attach the same object to the transform vector
    // because the IK Handle is now a tranform, too
    VecTransform::iterator i = m_transforms.begin();
    while(i != m_transforms.end())
    {
        if((*i)->m_name == p->m_name)
            return false;
        ++i;
    }
    m_transforms.push_back(p);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::AttachPhRigidBody(bk3dlib::PPhRigidBody pIKH)
{
    if(!pIKH)
        return false;
    CPhRigidBody* p = static_cast<CPhRigidBody*>(pIKH);
    MapPhRigidBody::iterator iIK = m_phRigidBodies.begin();
    while(iIK != m_phRigidBodies.end())
    {
        if(iIK->second->m_name == p->m_name)
            return false;
        ++iIK;
    }
    m_phRigidBodies[p] = p;
    // Now attach the same object to the transform vector
    // because the IK Handle is now a tranform, too
    VecTransform::iterator i = m_transforms.begin();
    while(i != m_transforms.end())
    {
        if((*i)->m_name == p->m_name)
            return false;
        ++i;
    }
    m_transforms.push_back(p);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::AttachIKHandle(bk3dlib::PIKHandle pIKH)
{
    if(!pIKH)
        return false;
    CIKHandle* p = static_cast<CIKHandle*>(pIKH);
    MapIKHandle::iterator iIK = m_ikHandles.begin();
    while(iIK != m_ikHandles.end())
    {
        if(iIK->second->m_name == p->m_name)
            return false;
        ++iIK;
    }
    m_ikHandles[p] = p;
    // Now attach the same object to the transform vector
    // because the IK Handle is now a tranform, too
    VecTransform::iterator i = m_transforms.begin();
    while(i != m_transforms.end())
    {
        if((*i)->m_name == p->m_name)
            return false;
        ++i;
    }
    m_transforms.push_back(p);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::AttachMaterial(bk3dlib::PMaterial pMat)
{
    if(pMat == NULL)
        return false;
    std::string strName(pMat->GetName());
    MapMaterial::iterator iM = m_materials.begin();
    while(iM != m_materials.end())
    {
        if(iM->second->m_name == strName)
        {
            char tmp[6];
            sprintf(tmp, "%d", m_meshes.size());
            strName += tmp;
            pMat->SetName(strName.c_str());
            break;
        }
        ++iM;
    }
    m_materials[pMat] = static_cast<CMaterial*>(pMat);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
void CFileHeader::UpdateTransformations(bool bBindPose)
{
    VecTransform::iterator iT = m_transforms.begin();
    while(iT != m_transforms.end())
    {
        if((*iT)->GetParent() == NULL)
            (*iT)->recTransfUpdate(NULL, bBindPose);
        ++iT;
    }
}

/*-------------------------------------------------------------------------
  
 */
void* CFileHeader::Cook(LPCSTR file, void ** pBufferMemory, unsigned int* bufferMemorySz, bool detachCookedData)
{
    if(m_bk3dPool)
        delete m_bk3dPool;
    if(m_bk3dPool2)
        delete m_bk3dPool2;
    m_bk3dPool = new Bk3dPool(file);
    m_bk3dPool2 = new Bk3dPool(file);

    // compute the matrices, first
    UpdateTransformations(true);

    try 
    {
        void *p = build(file, m_name);
        size_t sz = m_bk3dPool2->getUsedSize();
        if(pBufferMemory)  *pBufferMemory = m_bk3dPool2->getPool();
        if(bufferMemorySz) *bufferMemorySz = (unsigned int)sz;
        // Now let's calculate the relation table *without* changing pointers to offsets
        // This is necessary if ever the cooked structure is about to be used
        // and later converted to offsets before saving it (see the viewer example)
        ptrToOffset(true);
        if(detachCookedData) {
            if(m_bk3dPool)
            {
                m_bk3dPool->orphanPool();
                delete m_bk3dPool;
            }
            if(m_bk3dPool2)
            {
                m_bk3dPool2->orphanPool();
                delete m_bk3dPool2;
            }
            m_bk3dPool  = NULL;
            m_bk3dPool2 = NULL;
            m_fileHeader = NULL;
        }
        return p;
    } 
    catch(char *s)
    {
        printf(s);
        if(m_bk3dPool)
            delete m_bk3dPool;
        if(m_bk3dPool2)
            delete m_bk3dPool2;
        m_bk3dPool  = NULL;
        m_bk3dPool2 = NULL;
        m_fileHeader = NULL;
    }
    return NULL;
}
/*-------------------------------------------------------------------------
  
 */
bool CFileHeader::Save(LPCSTR file)
{
    DPF(("----------------------------------------------------------------\n"));
    //fileHeader->debugDumpAll();
    DPF(("----------------------------------------------------------------\n"));
    if(m_bk3dPool == NULL)
        Cook(file, NULL, NULL);
    ptrToOffset();
#ifndef USE_FILE_MAPPING // only if not using file mapping
    if(!m_fileHeader)
        return false;
    //
    // Write to file
    //
    int i;
    FILE *f;
    const char *lpcname = file;
    fprintf(stdout, "Creating file %s (%d+%d=%d bytes)...\n", lpcname, m_bk3dPool->getUsedSize(), m_bk3dPool2->getUsedSize(),
            m_bk3dPool->getUsedSize() + m_bk3dPool2->getUsedSize());
    f = fopen(lpcname, "wb");
    if(!f)
    {
        EPF(("Could not open Vertex binary file\n"));
        return false;
    }
    i = (int)fwrite(m_bk3dPool->getPool(), m_bk3dPool->getUsedSize(), 1, f);
    if(i < 1)
    {
        EPF(("Error while writing to the file\n"));
        fclose(f);
        return false;
    }
    if(m_bk3dPool2->getUsedSize()>0)
    {
        i = (int)fwrite(m_bk3dPool2->getPool(), m_bk3dPool2->getUsedSize(), 1, f);
        if(i < 1)
        {
            EPF(("Error while writing to the file\n"));
            fclose(f);
            return false;
        }
    }
    fclose(f);
#endif
    //
    // Back to pointers
    //
    OffsetToPtr();
    //
    // Note: commented-out: we may want to keep the data after saving
    // 
    //if(m_bk3dPool)
    //    delete m_bk3dPool;
    //m_bk3dPool = NULL;
    //if(m_bk3dPool2)
    //    delete m_bk3dPool2;
    //m_bk3dPool2 = NULL;
    return true;
}
/*-------------------------------------------------------------------------
  
 */
//bk3dlib::PBone    CFileHeader::CreateTransform(LPCSTR name)
//{
//    CBone* pTr = new CBone(name);
//    m_transforms.push_back(pTr);
//    g_transforms.push_back(pTr);
//    bk3dlib::PBone ppp = static_cast<bk3dlib::PBone>(pTr);
//    DPF(("Creating %s %p %p\n", name, pTr, ppp));
//    return ppp;
//}
bool CFileHeader::AttachTransform(bk3dlib::PBone pTr)
{
    if(!pTr)
        return false;
    if(pTr->AsIKHandle())
        return AttachIKHandle(pTr->AsIKHandle());
    CBone* p = static_cast<CBone*>(pTr->AsBone());
    VecTransform::iterator i = m_transforms.begin();
    while(i != m_transforms.end())
    {
        if((*i)->m_name == p->m_name)
            return false;
        ++i;
    }
    m_transforms.push_back(p);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
//bk3dlib::PTransformDOF CFileHeader::CreateTransformDOF(LPCSTR name)
//{
//    CTransformDOF* pTr = new CTransformDOF(name);
//    m_transforms.push_back(pTr);
//    g_transforms.push_back(pTr);
//    bk3dlib::PTransformDOF ppp = static_cast<bk3dlib::PTransformDOF>(pTr);
//    DPF(("Creating DOF Transform %s %p %p\n", name, pTr, ppp));
//    return ppp;
//}

/*-------------------------------------------------------------------------
  
 */
/*bk3dlib::PCurveVec    CFileHeader::CreateCurveVec(LPCSTR name, int ncomps)
{
    std::string strName(name);
    MapCurveVec::iterator iM = m_mayacurves.begin();
    while(iM != m_mayacurves.end())
    {
        if(iM->second->m_name == strName)
        {
            char tmp[6];
            sprintf(tmp, "%d", m_mayacurves.size());
            strName += tmp;
            break;
        }
        ++iM;
    }
    CMayaCurveVector* p = new CMayaCurveVector();
    assert(p);
    p->SetName(strName.c_str());
    //assert(ncomps < 10); // dumb safety check...
    for(int i=0; i<ncomps; i++)
    {
        CMayaCurve * pMCv = new CMayaCurve(p);
        p->m_mayacurves.push_back(pMCv);
    }
    m_mayacurves[p] = p;
    g_mayacurves[p] = p;
    return p;
}
*/
bool CFileHeader::AttachCurveVec(bk3dlib::PCurveVec pCv)
{
    if(!pCv)
        return false;
    CMayaCurveVector* p = static_cast<CMayaCurveVector*>(pCv);
    MapCurveVec::iterator iM = m_mayacurves.begin();
    while(iM != m_mayacurves.end())
    {
        if(iM->second->m_name == p->m_name)
            return false;
        ++iM;
    }
    m_mayacurves[p] = p;
    return true;
}

bool CFileHeader::AttachQuatCurve(bk3dlib::PQuatCurve pCv)
{
    if(!pCv)
        return false;
    CQuatCurve* p = static_cast<CQuatCurve*>(pCv);
    MapQuatCurve::iterator iM = m_quatcurves.begin();
    while(iM != m_quatcurves.end())
    {
        if(iM->second->m_name == p->m_name)
            return false;
        ++iM;
    }
    m_quatcurves[p] = p;
    return true;
}

/*-------------------------------------------------------------------------
  
 */
//virtual bool        CFileHeader::DeleteMesh(PMesh p)
//{
//}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMaterial    CFileHeader::GetMaterial(int n)
{
    MapMaterial::iterator i = m_materials.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_materials.end() )
            return NULL;
    }
    return i->second;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMaterial    CFileHeader::GetMaterial(LPCSTR name)
{
    MapMaterial::iterator i = m_materials.begin();
    while(i != m_materials.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PBone    CFileHeader::GetTransform(int n)
{
    VecTransform::iterator i = m_transforms.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_transforms.end() )
            return NULL;
    }
    return *i;
}
bk3dlib::PBone    CFileHeader::GetTransform(LPCSTR name, int *n)
{
    VecTransform::iterator i = m_transforms.begin();
    int j = 0;
    while(i != m_transforms.end())
    {
        if(!strcmp((*i)->GetName(), name))
        {
            if(n) *n = j;
            return *i;
        }
        ++i; j++;
    }
    if(n) *n = -1;
    return NULL;
}

/*-------------------------------------------------------------------------
  
 */
int                     CFileHeader::GetNumIKHandles()
{
    return (int)m_ikHandles.size();
}

bk3dlib::PIKHandle      CFileHeader::GetIKHandle(int n)
{
    MapIKHandle::iterator i = m_ikHandles.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_ikHandles.end() )
            return NULL;
    }
    return i->second;
}
bk3dlib::PIKHandle      CFileHeader::GetIKHandle(LPCSTR name)
{
    MapIKHandle::iterator i = m_ikHandles.begin();
    while(i != m_ikHandles.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}

/*-------------------------------------------------------------------------
  
 */
int                     CFileHeader::GetNumPhConstraints()
{
    return (int)m_phConstraints.size();
}

bk3dlib::PPhConstraint      CFileHeader::GetPhConstraint(int n)
{
    MapPhConstraint::iterator i = m_phConstraints.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_phConstraints.end() )
            return NULL;
    }
    return i->second;
}
bk3dlib::PPhConstraint      CFileHeader::GetPhConstraint(LPCSTR name)
{
    MapPhConstraint::iterator i = m_phConstraints.begin();
    while(i != m_phConstraints.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}

/*-------------------------------------------------------------------------
  
 */
int                     CFileHeader::GetNumPhRigidBodies()
{
    return (int)m_phRigidBodies.size();
}

bk3dlib::PPhRigidBody      CFileHeader::GetPhRigidBody(int n)
{
    MapPhRigidBody::iterator i = m_phRigidBodies.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_phRigidBodies.end() )
            return NULL;
    }
    return i->second;
}
bk3dlib::PPhRigidBody      CFileHeader::GetPhRigidBody(LPCSTR name)
{
    MapPhRigidBody::iterator i = m_phRigidBodies.begin();
    while(i != m_phRigidBodies.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}

/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMesh        CFileHeader::GetFirstMesh()
{
    m_iMesh = m_meshes.begin();
    if(m_iMesh == m_meshes.end() )
        return NULL;
    return m_iMesh->second;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMesh        CFileHeader::GetNextMesh()
{
    ++m_iMesh;
    if(m_iMesh == m_meshes.end() )
        return NULL;
    return m_iMesh->second;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMesh        CFileHeader::FindMesh(int n)
{
    MapMesh::iterator i = m_meshes.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_meshes.end() )
            return NULL;
    }
    return i->second;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PMesh        CFileHeader::FindMesh(LPCSTR name)
{
    MapMesh::iterator i = m_meshes.begin();
    while(i != m_meshes.end())
    {
        if(i->second->m_name == name)
            return i->second;
        ++i;
    }
    return NULL;
}
/*-------------------------------------------------------------------------
  
 */
int            CFileHeader::GetNumMeshes()
{
    return (int)m_meshes.size();
}
/*-------------------------------------------------------------------------
  
 */
int            CFileHeader::GetNumTransforms()
{
    return (int)m_transforms.size();
}

/*-------------------------------------------------------------------------
  
 */
int            CFileHeader::GetNumCurves()
{
    return (int)m_mayacurves.size();
}
bk3dlib::PCurveVec    CFileHeader::GetCurveVec(int n)
{
    MapCurveVec::iterator i = m_mayacurves.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_mayacurves.end() )
            return NULL;
    }
    return i->second;
}

bk3dlib::PCurveVec    CFileHeader::GetCurveVec(LPCSTR name)
{
    MapCurveVec::iterator i = m_mayacurves.begin();
    while(i != m_mayacurves.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}

/*-------------------------------------------------------------------------
  
 */
int            CFileHeader::GetNumQuatCurve()
{
    return (int)m_quatcurves.size();
}
bk3dlib::PQuatCurve    CFileHeader::GetQuatCurve(int n)
{
    MapQuatCurve::iterator i = m_quatcurves.begin();
    while(n-- >0)
    {
        ++i;
        if(i == m_quatcurves.end() )
            return NULL;
    }
    return i->second;
}

bk3dlib::PQuatCurve    CFileHeader::GetQuatCurve(LPCSTR name)
{
    MapQuatCurve::iterator i = m_quatcurves.begin();
    while(i != m_quatcurves.end())
    {
        if(!strcmp(i->second->GetName(), name))
            return i->second;
        ++i;
    }
    return NULL;
}

#if 1
/*=========================================================================
  MESH
 */

/*-------------------------------------------------------------------------
  
 */

//bk3dlib::PBuffer        CMesh::CreateVtxBuffer(LPCSTR name, int numcomp, int slot, bk3dlib::DataType type, bool isBlendShape)
//{
//    std::string strName(name);
//#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - TODO TODO TODO TODO TODO TODO : std::multimap<int, CBuffer*> MMapBuffer; needs to sort object according to primgroups and attrib number")
//    MMapBuffer::iterator iM = isBlendShape ? m_vtxBuffersBS.begin() : m_vtxBuffers.begin();
//    while(iM != (isBlendShape ? m_vtxBuffersBS.end() : m_vtxBuffers.end()))
//    {
//        if(iM->second->m_name == strName)
//        {
//            char tmp[6];
//            sprintf(tmp, "%d", m_vtxBuffers.size());
//            strName += tmp;
//        }
//        ++iM;
//    }
//    CBuffer* p = new CBuffer();
//    assert(p);
//    p->m_bufNum = (int)(isBlendShape ? m_vtxBuffersBS.size() : m_vtxBuffers.size());
//    p->m_name = std::string(name);
//    p->m_numcomp = numcomp;
//    p->m_slot = slot;
//    p->m_type = type;
//    p->m_usage = isBlendShape ? BufferForBS : BufferForVtx;
//    if(isBlendShape)
//        m_vtxBuffersBS.insert(std::make_pair(slot, p));
//    else
//        m_vtxBuffers.insert(std::make_pair(slot, p));
//    g_buffers[p] = p;
//    return p;
//}

/*-------------------------------------------------------------------------
  
 */
bool        CMesh::AttachVtxBuffer(bk3dlib::PBuffer pBuf, bool isBlendShape)
{
    if(pBuf == NULL)
        return false;
    CBuffer *p = static_cast<CBuffer*>(pBuf);
    p->m_usage = isBlendShape ? BufferForBS : BufferForVtx;
    if(isBlendShape)
    {
        p->m_bufNum = (int)m_vtxBuffersBS.size();
        m_vtxBuffersBS.insert(std::make_pair(p->m_slot, p));
    }
    else
    {
        p->m_bufNum = (int)m_vtxBuffers.size();
        m_vtxBuffers.insert(std::make_pair(p->m_slot, p));
    }
    return true;
}

/*-------------------------------------------------------------------------
  
 */
//bk3dlib::PBuffer        CMesh::CreateIdxBuffer(LPCSTR name, bk3dlib::DataType type, int numComp)
//{
//    std::string strName(name);
//    std::vector<CBuffer*>::iterator iM = m_idxBuffers.begin();
//    while(iM != m_idxBuffers.end())
//    {
//        if((*iM)->m_name == strName)
//        {
//            char tmp[6];
//            sprintf(tmp, "%d", m_idxBuffers.size());
//            strName += tmp;
//        }
//        ++iM;
//    }
//    CBuffer* p = new CBuffer();
//    assert(p);
//    p->m_name = std::string(name);
//    p->m_type = type;
//    p->m_bufNum = (int)m_idxBuffers.size();
//    p->m_usage = BufferForIdx;
//    m_idxBuffers.push_back(p);
//    // Note: this is a way to have multi-indexed buffer : multiple components...
//    p->m_numcomp = numComp;
//    g_buffers[p] = p;
//    return p;
//}
/*-------------------------------------------------------------------------
    Create a Primitive group
    If idxBuffer is Null, the primitive group is for a DrawArrays-style : no indices, but a range
    In this case, the range is expressed by offsetElement and numElements
    Returns the total size of the primitive groups in the mesh
 */
int CMesh::CreatePrimGroup(LPCSTR name, bk3dlib::PBuffer idxBuffer, bk3dlib::Topology topo, bk3dlib::PMaterial pMat, unsigned int offsetElement, unsigned int numElements)
{
    CBuffer* pBuffer = NULL;
    bool bAlreadyThere = false;
    std::string strName(name);
    std::vector<CBuffer*>::iterator iM = m_idxBuffers.begin();
    if(idxBuffer)
    {
        while(iM != m_idxBuffers.end())
        {
            if((*iM)->m_name == idxBuffer->GetName())
            {
                bAlreadyThere = true;
                break;
            }
            ++iM;
        }
        pBuffer = static_cast<CBuffer*>(idxBuffer);
        assert(pBuffer);
        if(!bAlreadyThere)
            m_idxBuffers.push_back(pBuffer);
        pBuffer->m_usage = BufferForIdx;
    }
    CPrimGroup* p = new CPrimGroup(this, name, pBuffer, topo, pMat, offsetElement, numElements);
    if(pBuffer && pBuffer->GetOwner() == NULL)
      pBuffer->SetOwner(p);
    assert(p);
    m_primgroups.push_back(p);
    return (int)m_primgroups.size();
}
bool CMesh::DeletePrimGroupFromName(LPCSTR name)
{
    std::vector<CPrimGroup*>::iterator iPG = m_primgroups.begin();
    for(; iPG != m_primgroups.end(); iPG++)
    {
        CPrimGroup* pg = *iPG;
        if(pg->m_name != std::string(name))
            continue;
        m_primgroups.erase(iPG);
        // TODO: shall we also delete idxBuffers + transforms + materials when not anymore used after this removal ?
        printf("TODO: shall we also delete idxBuffers + transforms + materials when not anymore used after this removal ?\n");
        delete pg;
        return true;
    }
    return false;
}
bool CMesh::DeletePrimGroupFromIndex(int id)
{
    return false;
}

int    CMesh::GetNumPrimGroups()
{
    return (int)m_primgroups.size();
}
bool CMesh::GetPrimGroupInfo(int i, bk3dlib::PrimGroup &pginfo)
{
    if(i >= (int)m_primgroups.size())
        return false;
    CPrimGroup* pg = m_primgroups[i];
    pginfo.name = pg->m_name.c_str();
    pginfo.numIdxBuffers = (int)pg->m_idxBuffers.size();
    if(pginfo.numIdxBuffers >= 10)
        return false;
    for(unsigned int j=0; j<10; j++)
    {
    if(j < pg->m_idxBuffers.size())
          pginfo.idxBuffers[j] = pg->m_idxBuffers[j];
    else
        pginfo.idxBuffers[j] = NULL;
    }
    pginfo.numElements = pg->m_numElements;
    pginfo.offsetElement = pg->m_offsetElement;
    pginfo.pMat = pg->m_pMaterial;
    pginfo.topo = pg->m_topo;
    return true;
}

int    CMesh::AttachIndexBuffer(bk3dlib::PBuffer idxBuffer, int primGroupID)
{
    if(!idxBuffer)
        return -1;
    CBuffer* p = static_cast<CBuffer*>(idxBuffer);
    m_idxBuffers.push_back(p);
    p->m_usage = BufferForIdx;
    if((primGroupID >= 0)&&(primGroupID < (int)m_primgroups.size()))
        return m_primgroups[primGroupID]->AddIndexBuffer(p);
    return (int)m_idxBuffers.size()-1;
}

/*-------------------------------------------------------------------------
  
 */
bool        CMesh::DetachBuffer(bk3dlib::PBuffer pbuf)
{
    bool bOk = false;
    int i = 0;
    std::vector<CBuffer*>::iterator iV;
    std::vector<CBuffer*>::iterator iVEnd;
    std::vector<CBuffer*>::iterator iV2;

    MMapBuffer::iterator iM;
    MMapBuffer::iterator iMEnd;
    MMapBuffer::iterator iM2;
    CBuffer *pb = static_cast<CBuffer*>(pbuf);
    switch(pb->m_usage)
    {
    case BufferForBS:
        iM = m_vtxBuffersBS.begin();
        iMEnd = m_vtxBuffersBS.end();
        break;
    case BufferForVtx:
        iM = m_vtxBuffers.begin();
        iMEnd = m_vtxBuffers.end();
        break;
    case BufferForIdx:
        iV = m_idxBuffers.begin();
        iVEnd = m_idxBuffers.end();
        while(iV != iVEnd)
        {
            if(*iV == pbuf)
            {
                m_idxBuffers.erase(iV);
                bOk = true;
                break;
            }
            else ++iV;
        }
        // recomputing the index... very annoying. TODO: remove it
        iV = m_idxBuffers.begin();
        iVEnd = m_idxBuffers.end();
        while(iV != iVEnd)
        {
            (*iV)->m_bufNum = i++;
            ++iV;
        }
        // removing the references in the primgroups
        for(int j=0; j<(int)m_primgroups.size(); j++)
        {
            iV = m_primgroups[j]->m_idxBuffers.begin();
            iVEnd = m_primgroups[j]->m_idxBuffers.end();
            while(iV != iVEnd)
            {
                iV2 = iV++;
                if(*iV2 == pbuf)
                {
                    m_primgroups[j]->m_idxBuffers.erase(iV2);
                    break;
                }
            }
        }
        return bOk;
    }
    iM2 = iM;
    while(iM != iMEnd)
    {
        if(iM->second == pbuf)
        {
            if(pb->m_usage == BufferForBS)
            {
                m_vtxBuffersBS.erase(iM);
                bOk = true;
                break;
            }
            if(pb->m_usage == BufferForVtx)
            {
                m_vtxBuffers.erase(iM);
                bOk = true;
                break;
            }
        }
        else ++iM;
    }
    // recomputing the index... very annoying. TODO: remove it
    switch(pb->m_usage)
    {
    case BufferForBS:
        iM = m_vtxBuffersBS.begin();
        iMEnd = m_vtxBuffersBS.end();
        break;
    case BufferForVtx:
        iM = m_vtxBuffers.begin();
        iMEnd = m_vtxBuffers.end();
        break;
    }
    while(iM != iMEnd)
    {
        iM->second->m_bufNum = i++;
        ++iM;
    }
    return bOk;
}
/*-------------------------------------------------------------------------
  
 */

bk3dlib::PBuffer        CMesh::GetVtxBuffer(int n, bool isBlendShape)
{
    MMapBuffer::iterator iB = isBlendShape ? m_vtxBuffersBS.begin() : m_vtxBuffers.begin();
    if(n >= (int)(isBlendShape ? m_vtxBuffersBS.size() : m_vtxBuffers.size()))
        return NULL;
    while(n > 0)
    {
        ++iB;
        n--;
    }
    return iB->second;
}

/*-------------------------------------------------------------------------
  
 */
bk3dlib::PBuffer        CMesh::GetIdxBuffer(int n)
{
    std::vector<CBuffer*>::iterator iB = m_idxBuffers.begin();
    if(n >= (int)m_idxBuffers.size())
        return NULL;
    while(n > 0)
    {
        ++iB;
        n--;
    }
    return *iB;
}
/*-------------------------------------------------------------------------
  
 */

int            CMesh::GetNumVtxBuffers(bool isBlendShape)
{
    if(isBlendShape)
        return (int)m_vtxBuffersBS.size();
    else
        return (int)m_vtxBuffers.size();
}
/*-------------------------------------------------------------------------
  
 */
int            CMesh::GetNumIdxBuffers()
{
    return (int)m_idxBuffers.size();
}
/*-------------------------------------------------------------------------
  
 */
int            CMesh::GetNumSlots()
{
    int n = 0;
    MMapBuffer::iterator iB = m_vtxBuffers.begin();
    while(iB != m_vtxBuffers.end())
    {
        if(iB->second->m_slot > n)
            n = iB->second->m_slot;
        ++iB;
    }
    return n;
}
/*-------------------------------------------------------------------------
  
 */
void CMesh::SetSlotName(int s, LPCSTR name)
{
    if(s >= m_SlotNames.size())
        m_SlotNames.resize(s+1);
    m_SlotNames[s] = std::string(name);
}
const char* CMesh::GetSlotName(int s)
{
    if(s >= m_SlotNames.size())
        return NULL;
    return m_SlotNames[s].c_str();
}
void CMesh::SetBSSlotName(int s, LPCSTR name)
{
    if(s >= m_BSSlotNames.size())
        m_BSSlotNames.resize(s+1);
    m_BSSlotNames[s] = std::string(name);
}
const char* CMesh::GetBSSlotName(int s)
{
    if(s >= m_BSSlotNames.size())
        return NULL;
    return m_BSSlotNames[s].c_str();
}

/*-------------------------------------------------------------------------
  Cmpute the B volumes by using source as the buffer of vertex positions
  TODO: walk through the Primitive groups index buffers and compute local
  bbox in each prim group
 */

bool CMesh::ComputeBoundingVolumes(bk3dlib::PBuffer source)
{
    if(!source)
        return false;
    CBuffer *pBuf = static_cast<CBuffer*>(source);
    for(int pg=0; pg<(int)m_primgroups.size(); pg++)
    {
        if(m_primgroups[pg]->m_idxBuffers.size() == 0)
            continue;
        // use the first one... maybe we should be more accurate in case of multi-idx, here
        pBuf->ComputeBVolume(m_primgroups[pg]->aabbox, m_primgroups[pg]->bsphere, m_primgroups[pg]->m_idxBuffers[0], m_primgroups[pg]->m_offsetElement, m_primgroups[pg]->m_numElements);
    }
    return pBuf->ComputeBVolume(aabbox, bsphere);
}

/*-------------------------------------------------------------------------
  
 */

bool        CMesh::AddTransformReference(bk3dlib::PBone t, int primgroup)
{
    if(primgroup == -1)
        m_transformRefs.push_back(t->AsBone());
    else
    {
        if(primgroup >= (int)m_primgroups.size() )
        {
            return false;
        }
        m_primgroups[primgroup]->AddTransformReference(t->AsBone());
    }
    return true;
}
/*-------------------------------------------------------------------------
  summing all together... TODO: see if this is good enough...
 */
int                CMesh::GetNumTransformReferences(bool bMeshOnly)
{
    int n = (int)m_transformRefs.size();
    if(bMeshOnly)
        return n;
    for(int i=0; i<(int)m_primgroups.size(); i++)
    {
        n += m_primgroups[i]->GetNumTransformReferences();
    }
    return n;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PBone        CMesh::GetTransformReference(int n)
{
    if(n < (int)m_transformRefs.size())
        return m_transformRefs[n];
    n -= (int)m_transformRefs.size();
    for(int i=0; i<(int)m_primgroups.size(); i++)
    {
        if(n < m_primgroups[i]->GetNumTransformReferences())
            return m_primgroups[i]->GetTransformReference(n);
    }
    return NULL;
}
/*-------------------------------------------------------------------------
  
 */
void        CMesh::ClearTransformReferences(int primgroup = -1)
{
    if(primgroup < 0)
    {
        m_transformRefs.clear();
        for(int i=0; i<(int)m_primgroups.size(); i++)
        {
            m_primgroups[i]->m_transformRefs.clear();
        }
    } else {
        if(primgroup < (int)m_primgroups.size())
            m_primgroups[primgroup]->m_transformRefs.clear();
    }
}
/*-------------------------------------------------------------------------
  
 */
bool        CMesh::ComputeNormalsAndTangents(bk3dlib::PBuffer bufferIdx, bk3dlib::PBuffer bufferVtxPos, bk3dlib::PBuffer bufferTexcoords,
                    bk3dlib::PBuffer bufferPerVtxNormals, bk3dlib::PBuffer bufferPerFaceNormals, 
                    bk3dlib::PBuffer bufferTangents, bk3dlib::PBuffer bufferBitangents)
{
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO -  method bool        CMesh::ComputeNormalsAndTangents()")
    return false;
}

/*=========================================================================
  Primgroup
 */
CPrimGroup::CPrimGroup(CMesh* parent, LPCSTR name, CBuffer*    idxBuffer, bk3dlib::Topology topo, bk3dlib::PMaterial pMat, unsigned int offsetElement, unsigned int numElements) :
    m_parent(parent),
    m_name(name),
    m_topo(topo),
    m_offsetElement(offsetElement),
    m_numElements(numElements)
{
    if(idxBuffer)
        m_idxBuffers.push_back(idxBuffer);
    m_pMaterial = static_cast<CMaterial*>(pMat);
}

/*--------------------------------------------------------------------------
  
 */
void CPrimGroup::AddTransformReference(bk3dlib::PBone t)
{
    m_transformRefs.push_back(t->AsBone());
}

/*--------------------------------------------------------------------------
  
 */
int CPrimGroup::AddIndexBuffer(CBuffer* idxBuffer)
{
    m_idxBuffers.push_back(idxBuffer);
    return (int)m_idxBuffers.size()-1;
}

/*=========================================================================
  BUFFER
 */
CBuffer::CBuffer()
{
    m_primitiveRestart = 0;
    m_bufNum = 0;
    m_numcomp = 1;
    m_slot = -1;
    m_type = bk3dlib::UNKNOWN;
    m_usage = BufferForVtx;
    m_pOwner = NULL;
    m_divisor = 1;
    m_bk3dptr = NULL;
    m_curItem = 0;
}
bool    CBuffer::Reserve(unsigned int numitems)
{
    m_FVals.reserve(numitems);
    m_UIVals.reserve(numitems);
    return true;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::ClearData()
{
    m_FVals.clear();
    m_UIVals.clear();
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(float f)
{
    assert(m_UIVals.empty());
    if((m_curItem+1) >= m_FVals.size())
        m_FVals.resize(m_curItem+1);
    m_FVals[m_curItem++] = f;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const float * p, unsigned int n)
{
    assert(m_UIVals.empty());
    if((m_curItem+n) >= m_FVals.size())
        m_FVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_FVals[m_curItem++] = *p++;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const unsigned long * p, unsigned int n)
{
    assert(m_FVals.empty());
    if((m_curItem+n) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_UIVals[m_curItem++] = (*p++)/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const unsigned int * p, unsigned int n)
{
    assert(m_FVals.empty());
    if((m_curItem+n) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_UIVals[m_curItem++] = (*p++)/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(unsigned int i)
{
    assert(m_FVals.empty());
    if((m_curItem+1) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+1);
    m_UIVals[m_curItem++] = i/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(int i)
{
    assert(m_FVals.empty());
    if((m_curItem+1) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+1);
    m_UIVals[m_curItem++] = (unsigned int)i/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const unsigned short * p, unsigned int n)
{
    assert(m_FVals.empty());
    if((m_curItem+n) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_UIVals[m_curItem++] = (*p++)/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const unsigned char * p, unsigned int n)
{
    assert(m_FVals.empty());
    if((m_curItem+n) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_UIVals[m_curItem++] = (*p++)/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::AddData(const long * p, unsigned int n)
{
    assert(m_FVals.empty());
    if((m_curItem+n) >= m_UIVals.size())
        m_UIVals.resize(m_curItem+n);
    //TODO: find a faster solution
    for(unsigned int i=0; i<n; i++)
        m_UIVals[m_curItem++] = (*p++)/m_divisor;
}
/*-------------------------------------------------------------------------
  
 */

void    CBuffer::SetDivisor(int divisor)
{
    m_divisor = divisor;
}

/*-------------------------------------------------------------------------
  
 */

void    CBuffer::GotoItem(unsigned int item)
{
    m_curItem = item;
}
/*-------------------------------------------------------------------------
  
 */
int      CBuffer::GetData(float * pDst, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            *pDst++ = m_FVals[i];
    }
    else // UInt
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            *pDst++ = (float)m_UIVals[i];
    }
    return last-offsitem;
}
/*-------------------------------------------------------------------------
  
 */
int      CBuffer::GetData(unsigned int * pDst, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            *pDst++ = (unsigned int)m_FVals[i];
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            *pDst++ = m_UIVals[i];
    }
    return last-offsitem;
}

bool CBuffer::SetData(const float * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_UIVals.empty())
            m_UIVals.resize(last);
        else
            m_FVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = *p++;
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}
bool CBuffer::SetData(const long * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_FVals.empty())
            m_FVals.resize(last);
        else
            m_UIVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = (float)(*p++);
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}
bool CBuffer::SetData(const unsigned long * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_FVals.empty())
            m_FVals.resize(last);
        else
            m_UIVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = (float)(*p++);
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}
bool CBuffer::SetData(const unsigned int * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_FVals.empty())
            m_FVals.resize(last);
        else
            m_UIVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = (float)(*p++);
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}
bool CBuffer::SetData(const unsigned short * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_FVals.empty())
            m_FVals.resize(last);
        else
            m_UIVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = (float)(*p++);
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}
bool CBuffer::SetData(const unsigned char * p, unsigned int offsitem, unsigned int numitems)
{
    int last = numitems + offsitem;
    if(!p)
    {
        if(!m_FVals.empty())
            m_FVals.resize(last);
        else
            m_UIVals.resize(last);
        return true;
    }
    if(!m_FVals.empty())
    {
        if((numitems == 0)||(last >= (int)m_FVals.size()))
            last = (int)m_FVals.size();
        for(int i=offsitem; i<last; i++)
            m_FVals[i] = (float)(*p++);
    }
    else
    {
        if((numitems == 0)||(last >= (int)m_UIVals.size()))
            last = (int)m_UIVals.size();
        for(int i=offsitem; i<last; i++)
            m_UIVals[i] = (unsigned int)(*p++);
    }
    return true;
}

/*-------------------------------------------------------------------------
  
/*-------------------------------------------------------------------------
  
 */
//int        GetNumVectors()
//{
//}
/*-------------------------------------------------------------------------
  
 */
int        CBuffer::GetNumItems()
{
    size_t n;
    n = m_FVals.size();
    if(n == 0)
        n = m_UIVals.size();
    //assert(m_numcomp*(n/m_numcomp) == (n));
    return (int)n / m_numcomp;
}

/*-------------------------------------------------------------------------
  
 */
unsigned int CBuffer::GetItemSizeBytes()
{
    switch(m_type)
    {
    case bk3dlib::UNKNOWN:
    case bk3dlib::UINT32:
    case bk3dlib::FLOAT32:
        return m_numcomp*4;
    case bk3dlib::UINT16:
    case bk3dlib::FLOAT16:
        return m_numcomp*2;
    }
    return 0;
}
/*-------------------------------------------------------------------------
  
 */
bool CBuffer::isFloatFormat()
{
    switch(m_type)
    {
    case bk3dlib::UNKNOWN:
        assert(!"Bad");
    case bk3dlib::FLOAT32:
    case bk3dlib::FLOAT16:
        return true;
    case bk3dlib::UINT32:
    case bk3dlib::UINT16:
    //case bk3dlib::UINT8:
        return false;
    }
    assert(!"Bad");
    return true;
}
/*-------------------------------------------------------------------------
  
 */
DXGI_FORMAT        CBuffer::FmtDXGI()
{
    switch(m_numcomp)
    {
    case 1:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return DXGI_FORMAT_R32_FLOAT;
        case bk3dlib::FLOAT16: // NOT IMPLEMENTED YET
            assert("FMT Undefined"); return DXGI_FORMAT_R16_FLOAT;
        case bk3dlib::UINT32:
            return DXGI_FORMAT_R32_UINT;
        case bk3dlib::UINT16:
            return DXGI_FORMAT_R16_UINT;
        case bk3dlib::UINT8:
            return DXGI_FORMAT_R8_UINT;
        }
    case 2:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return DXGI_FORMAT_R32G32_FLOAT;
        case bk3dlib::FLOAT16: // NOT IMPLEMENTED YET
            return DXGI_FORMAT_R16_FLOAT;
        case bk3dlib::UINT32:
            return DXGI_FORMAT_R32G32_UINT;
        case bk3dlib::UINT16:
            return DXGI_FORMAT_R16G16_UINT;
        case bk3dlib::UINT8:
            return DXGI_FORMAT_R8G8_UINT;
        }
    case 3:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case bk3dlib::FLOAT16:
            assert("FMT Undefined"); return DXGI_FORMAT_UNKNOWN;//DXGI_FORMAT_R16_FLOAT;
        case bk3dlib::UINT32:
            return DXGI_FORMAT_R32G32B32_UINT;
        case bk3dlib::UINT16:
            assert("FMT Undefined"); return DXGI_FORMAT_UNKNOWN;//DXGI_FORMAT_R16G16B16_UINT;
        case bk3dlib::UINT8:
            assert("FMT Undefined"); return DXGI_FORMAT_UNKNOWN;//TODO
        }
    case 4:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case bk3dlib::FLOAT16:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case bk3dlib::UINT32:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case bk3dlib::UINT16:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case bk3dlib::UINT8:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        }
    }
    assert(!"should not happen");
    return DXGI_FORMAT_UNKNOWN;
}
D3DDECLTYPE        CBuffer::FmtDX9()
{
    switch(m_numcomp)
    {
    case 1:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return D3DDECLTYPE_FLOAT1;
        case bk3dlib::FLOAT16:
            return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT32:
            return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT16:
            return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT8:
            return D3DDECLTYPE_UNDEF;
        }
    case 2:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return D3DDECLTYPE_FLOAT2;
        case bk3dlib::FLOAT16:
            return D3DDECLTYPE_FLOAT16_4;
        case bk3dlib::UINT32:
            return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT16:
            return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT8:
            return D3DDECLTYPE_UNDEF;
        }
    case 3:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            assert("FMT Undefined"); return D3DDECLTYPE_FLOAT3;
        case bk3dlib::FLOAT16:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT32:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT16:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT8:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        }
    case 4:
        switch(m_type)
        {
        case bk3dlib::FLOAT32:
            return D3DDECLTYPE_FLOAT4;
        case bk3dlib::FLOAT16:
            return D3DDECLTYPE_FLOAT16_4;
        case bk3dlib::UINT32:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT16:
            assert("FMT Undefined"); return D3DDECLTYPE_UNDEF;
        case bk3dlib::UINT8:
            return D3DDECLTYPE_UBYTE4;
        }
    }
    assert(!"should not happen");
    return D3DDECLTYPE_UNDEF;
}
GLenum            CBuffer::FmtOpenGL()
{
    switch(m_type)
    {
    case bk3dlib::FLOAT32:
        return GL_FLOAT;
    case bk3dlib::FLOAT16:
        assert("FMT Undefined"); return 0;//GL_HALF ??; TODO
    case bk3dlib::UINT32:
        return GL_UNSIGNED_INT;
    case bk3dlib::UINT16:
        return GL_UNSIGNED_SHORT;
    case bk3dlib::UINT8:
        return GL_UNSIGNED_BYTE;
    }
    assert(!"should not happen");
    return 0;
}

/*-------------------------------------------------------------------------
  
 */
float            CBuffer::GetValueAsFloat(int i, int c)
{
    unsigned int pos = i * m_numcomp + c;
    if(m_FVals.empty())
    {
        if(pos >= m_UIVals.size())
            return 0.0;
        return (float)m_UIVals[pos];
    }
    else
    {
        if(pos >= m_FVals.size())
            return 0.0;
        return m_FVals[pos];
    }
}
/*-------------------------------------------------------------------------
  
 */
unsigned int    CBuffer::GetValueAsUInt(int i, int c)
{
    unsigned int pos = i * m_numcomp + c;
    if(m_FVals.empty())
    {
        if(pos >= m_UIVals.size())
            return 0;
        return m_UIVals[pos];
    }
    else
    {
        if(pos >= m_FVals.size())
            return 0;
        return (unsigned int)m_FVals[pos];
    }
}

/*-------------------------------------------------------------------------
  
 */
bool CBuffer::ComputeBVolume(bk3d::AABBox &aabbox, bk3d::BSphere &bsphere, CBuffer * idxBuffer, int offset, int numelts)
{
    if(m_numcomp > 3)
        return false;
    int nvert = (int)m_FVals.size()/m_numcomp;

    aabbox.min[0] = aabbox.min[1] = aabbox.min[2] = 10000.0;
    aabbox.max[0] = aabbox.max[1] = aabbox.max[2] = -10000.0;

    if(m_numcomp < 3)
    {
        aabbox.min[2] = 0.0;
        aabbox.max[2] = 0.0;
    }
    if(m_numcomp < 2)
    {
        aabbox.min[1] = 0.0;
        aabbox.max[1] = 0.0;
    }

    float avg[3] = {0,0,0};

    if(idxBuffer)
    {
        //DataType dt = idxBuffer->GetDataType();
        //...if((dt == UINT32)||(dt == UINT16)||(dt == UINT8))
        if(numelts == -1)
            numelts = ((int)idxBuffer->m_UIVals.size() - offset);
        else
            numelts += offset;
        for(int j=offset; j<numelts; j++)
        {
            unsigned int i = idxBuffer->m_UIVals[j];
            if((i > m_FVals.size())||(m_primitiveRestart == i))
                continue; // can happen when we encounter the prim. restart ID
            unsigned int offset = m_numcomp*i;
            for(int j=0; j<m_numcomp; j++) {
                float f = m_FVals[offset + j];
                if(f > aabbox.max[j]) aabbox.max[j] = f;
                if(f < aabbox.min[j]) aabbox.min[j] = f;
                avg[j] += f;
            }
        }
    } else {
        for(int i=0; i<nvert; i++)
        {
            int offset = m_numcomp*i;
            for(int j=0; j<m_numcomp; j++) {
                float f = m_FVals[offset + j];
                if(f > aabbox.max[j]) aabbox.max[j] = f;
                if(f < aabbox.min[j]) aabbox.min[j] = f;
                avg[j] += f;
            }
        }
    }
    // TODO: better solution... 7.1.3 from Eric Lengyel
    bsphere.pos[0] = avg[0] / (float)nvert;
    bsphere.pos[1] = avg[1] / (float)nvert;
    bsphere.pos[2] = avg[2] / (float)nvert;

    for(int i=0; i<nvert; i++)
    {
        int offset = m_numcomp*i;
        float dd = 0;
        for(int j=0; j<m_numcomp; j++) {
            float f = m_FVals[offset + j];
            dd += (f-avg[j])*(f-avg[j]);
        }
        if(dd > bsphere.radius)
            bsphere.radius = dd;
    }
    bsphere.radius = sqrtf(bsphere.radius);    
    return true;
}

/*-------------------------------------------------------------------------
  
 */
CPrimGroup*    CBuffer::GetOwner()
{
    return m_pOwner;
}
/*-------------------------------------------------------------------------
  
 */
void    CBuffer::SetOwner(CPrimGroup *pg)
{
    m_pOwner = pg;
}

/*-------------------------------------------------------------------------
  
 */ // comps * numvecs

/*=========================================================================
  TRANSFORM
 */

//bool CTransform::connectCurve(bk3dlib::PCurveVec pCVec, bk3dlib::TransfComponent comp, int compOffset)
//{
//    return false;
//}
//bool CTransform::disconnectCurve(bk3dlib::PCurveVec pCVec, bk3dlib::TransfComponent comp, int compOffset)
//{
//    return false;
//}

/*-------------------------------------------------------------------------
  
 */
void CBone::SetParent(bk3dlib::PBone p)
{
    CBone* parent = static_cast<CBone*>(p->AsBone());
    if(m_parentTransf)
    {
        std::vector<CBone*>::iterator iT = m_parentTransf->m_childrenTransf.begin();
        while(iT != m_parentTransf->m_childrenTransf.end() )
        {
            if(*iT == this)
            {
                m_parentTransf->m_childrenTransf.erase(iT);
                break;
            }
            ++iT;
        }
    }
    m_parentTransf = parent;
  if(parent)
    parent->m_childrenTransf.push_back(this);
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PBone CBone::GetParent()
{
    return m_parentTransf;
}

/*-------------------------------------------------------------------------
  
 */
bk3dlib::PBone CBone::GetChild(int n)
{
    if(n >= (int)m_childrenTransf.size())
        return NULL;
    return m_childrenTransf[n];
}

/*-------------------------------------------------------------------------
  
 */
void CBone::GetPos(float &x, float &y, float &z)
{
    x = m_pos[0];
    y = m_pos[1];
    z = m_pos[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransformSimple::GetScale(float &x, float &y, float &z)
{
    x = m_scale[0];
    y = m_scale[1];
    z = m_scale[2];
}
/*-------------------------------------------------------------------------
  
 */
void CBone::GetQuaternion(float &x, float &y, float &z, float &w)
{
    x=y=z=w=0.0;
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetRotation(float &x, float &y, float &z)
{
    x = m_rotation[0];
    y = m_rotation[1];
    z = m_rotation[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetScalePivot(float &x, float &y, float &z)
{
    x = m_scalePivot[0];
    y = m_scalePivot[1];
    z = m_scalePivot[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetScalePivotTranslate(float &x, float &y, float &z)
{
    x = m_scalePivotTranslate[0];
    y = m_scalePivotTranslate[1];
    z = m_scalePivotTranslate[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetRotationPivot(float &x, float &y, float &z)
{
    x = m_rotationPivot[0];
    y = m_rotationPivot[1];
    z = m_rotationPivot[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetRotationPivotTranslate(float &x, float &y, float &z)
{
    x = m_rotationPivotTranslate[0];
    y = m_rotationPivotTranslate[1];
    z = m_rotationPivotTranslate[2];
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetRotationOrientation(float &x, float &y, float &z, float &w)
{
    x = m_rotationOrientation.x;
    y = m_rotationOrientation.y;
    z = m_rotationOrientation.z;
    w = m_rotationOrientation.w;
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::GetJointOrientation(float &x, float &y, float &z, float &w)
{
    x = m_jointOrientation[0];
    y = m_jointOrientation[1];
    z = m_jointOrientation[2];
    w = m_jointOrientation[3];
}
/*-------------------------------------------------------------------------
  
 */
void CBone::GetMatrix(float *m)
{
    memcpy(m, m_matrix.mat_array, sizeof(float)*16);
}
void CBone::GetMatrix_Bindpose(float *m)
{
    memcpy(m, m_bindpose_matrix.mat_array, sizeof(float)*16);
}
void CBone::GetMatrix_Abs(float *m)
{
    memcpy(m, m_abs_matrix.mat_array, sizeof(float)*16);
}

void CBone::CopyFrom(bk3dlib::PBone from)
{
    CBone * pFrom = static_cast<CBone*>(from->AsBone());
    SetParent(pFrom->m_parentTransf);
    m_TransfDOF = pFrom->m_TransfDOF;

    memcpy(m_matrix.mat_array, pFrom->m_matrix.mat_array, sizeof(float)*16);
    memcpy(m_bindpose_matrix.mat_array, pFrom->m_bindpose_matrix.mat_array, sizeof(float)*16);
    memcpy(m_abs_matrix.mat_array, pFrom->m_abs_matrix.mat_array, sizeof(float)*16);

    m_pos = pFrom->m_pos;
    m_Quat = pFrom->m_Quat;
    m_abs_Quat = pFrom->m_abs_Quat;
    m_validComps = pFrom->m_validComps;
    bDirty = pFrom->bDirty;

    m_userData = pFrom->m_userData;
    //std::set< CurveVecConnection > connectedCurveVectors;
}
void CTransformSimple::CopyFrom(bk3dlib::PBone from)
{
    CBone::CopyFrom(from);
    CTransformSimple * pFrom = static_cast<CTransformSimple*>(from->AsTransfSimple());
    if(pFrom == NULL)
        return;
    m_scale = pFrom->m_scale;

    // TODO: remove flags that TransformSimple cannot handle
    m_validComps = pFrom->m_validComps;
}
void CTransform::CopyFrom(bk3dlib::PBone from)
{
    CTransformSimple::CopyFrom(from);
    CTransform * pFrom = static_cast<CTransform*>(from->AsTransf());
    if(pFrom == NULL)
        return;
    m_rotation = pFrom->m_rotation;
    m_rotationOrder[0] = pFrom->m_rotationOrder[0];
    m_rotationOrder[1] = pFrom->m_rotationOrder[1];
    m_rotationOrder[2] = pFrom->m_rotationOrder[2];
    m_scalePivot = pFrom->m_scalePivot;
    m_scalePivotTranslate = pFrom->m_scalePivotTranslate;
    m_rotationPivot = pFrom->m_rotationPivot;
    m_rotationPivotTranslate = pFrom->m_rotationPivotTranslate;

    m_rotationOrientation = pFrom->m_rotationOrientation;
    m_jointOrientation = pFrom->m_jointOrientation;
}
/*-------------------------------------------------------------------------
  
 */
inline void setVec(bool &bDirty, float *x, float *y, float *z, float *w, vec3f &vec)
{
    if((x&&(*x != vec.x))
        ||(y&&(*y != vec.y))
        ||(z&&(*z != vec.z)) )
        bDirty = true;
    if(x) vec.x = *x;
    if(y) vec.y = *y;
    if(z) vec.z = *z;
}
inline void setVec(bool &bDirty, float *x, float *y, float *z, float *w, quatf &q)
{
    if((x&&(*x != q.x))
        ||(y&&(*y != q.y))
        ||(z&&(*z != q.z))
        ||(w&&(*w != q.w)) )
        bDirty = true;
    if(x) q.x = *x;
    if(y) q.y = *y;
    if(z) q.z = *z;
    if(w) q.w = *w;
}
#define EPSILON 0.000001f
void CBone::SetPos(float x, float y, float z)
{
    if((x==0.0f)&&(y==0.0f)&&(z==0.0f))
        m_validComps &= ~TRANSFCOMP_pos;
    else
        m_validComps |= TRANSFCOMP_pos;
    setVec(bDirty, &x, &y, &z, NULL, m_pos);
}
/*-------------------------------------------------------------------------
  
 */
void CTransformSimple::SetScale(float x, float y, float z)
{
    //if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
    //    m_validComps &= ~TRANSFCOMP_scale;
    //else
        m_validComps |= TRANSFCOMP_scale;
    setVec(bDirty, &x, &y, &z, NULL, m_scale);
}
/*-------------------------------------------------------------------------
  I commented the EPSILON condition so that we still have valid rotations even
  when close to none...
 */
void CTransform::SetRotation(float x, float y, float z)
{
    //if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
    //    m_validComps &= ~TRANSFCOMP_rotation;
    //else
        m_validComps |= TRANSFCOMP_rotation;
    setVec(bDirty, &x, &y, &z, NULL, m_rotation);
}
/*-------------------------------------------------------------------------
  
 */                    // Euler Rotation in degres
void CTransform::SetRotationOrder(char x, char y, char z)
{
    m_validComps |= TRANSFCOMP_rotationOrder;
    if((m_rotationOrder[0] != x)
        ||(m_rotationOrder[1] != y)
        ||(m_rotationOrder[2] != z))
        bDirty = true;
    m_rotationOrder[0] = x;
    m_rotationOrder[1] = y;
    m_rotationOrder[2] = z;
}
/*-------------------------------------------------------------------------
  
 */                  // 3 chars for "xyz" or any other
void CBone::SetQuaternion(float x, float y, float z, float w)
{
    //if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON)&&(fabs(w-1.0f)<=EPSILON))
    //    m_validComps &= ~TRANSFCOMP_Quat;
    //else
        m_validComps |= TRANSFCOMP_Quat;
    setVec(bDirty, &x, &y, &z, &w, m_Quat);
}

void CBone::SetQuaternionFromEulerXYZ(float x, float y, float z)
{
    quatf Q;
    Q.from_euler_xyz(vec3f(x,y,z));
    m_validComps |= TRANSFCOMP_Quat;
    setVec(bDirty, &Q.x, &Q.y, &Q.z, &Q.w, m_Quat);
}

/*-------------------------------------------------------------------------
  
 */                  // 3 chars for "xyz" or any other
// Do we really need all of them ?
void CTransform::SetScalePivot(float x, float y, float z)
{
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_scalePivot;
    else
        m_validComps |= TRANSFCOMP_scalePivot;
    setVec(bDirty, &x, &y, &z, NULL, m_scalePivot);
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::SetScalePivotTranslate(float x, float y, float z)
{
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_scalePivotTranslate;
    else
        m_validComps |= TRANSFCOMP_scalePivotTranslate;
    setVec(bDirty, &x, &y, &z, NULL, m_scalePivotTranslate);
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::SetRotationPivot(float x, float y, float z)
{
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_rotationPivot;
    else
        m_validComps |= TRANSFCOMP_rotationPivot;
    setVec(bDirty, &x, &y, &z, NULL, m_rotationPivot);
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::SetRotationPivotTranslate(float x, float y, float z)
{
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_rotationPivotTranslate;
    else
        m_validComps |= TRANSFCOMP_rotationPivotTranslate;
    setVec(bDirty, &x, &y, &z, NULL, m_rotationPivotTranslate);
}
/*-------------------------------------------------------------------------
  
 */

void CTransform::SetRotationOrientation(float x, float y, float z, float w)
{
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON)&&(fabs(w-1.0f)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_rotationOrientation;
    else
        m_validComps |= TRANSFCOMP_rotationOrientation;
    setVec(bDirty, &x, &y, &z, &w, m_rotationOrientation);
}
/*-------------------------------------------------------------------------
  
 */         //Quaternion
void CTransform::SetJointOrientation(float x, float y, float z, float w)
{
    m_validComps |= TRANSFCOMP_jointOrientation | TRANSFCOMP_isBone;
    if((fabs(x)<=EPSILON)&&(fabs(y)<=EPSILON)&&(fabs(z)<=EPSILON)&&(fabs(w-1.0f)<=EPSILON))
        m_validComps &= ~TRANSFCOMP_jointOrientation;
    setVec(bDirty, &x, &y, &z, &w, m_jointOrientation);
    if(m_posBoneTail.norm() == 0.0f) {
        m_posBoneTail.x = 0.0f;
        m_posBoneTail.y = 0.01f; // default orientation if not specified at all
        m_posBoneTail.z = 0.0f;
        bDirty = true;
    }
}

/*-------------------------------------------------------------------------
  
 */
void CBone::SetTailPos(float x, float y, float z)
{
    m_validComps |= TRANSFCOMP_isBone;
    setVec(bDirty, &x, &y, &z, NULL, m_posBoneTail);
}
/*-------------------------------------------------------------------------
  
 */
const float * CBone::GetTailPos()
{
    return m_posBoneTail.vec_array;
}

/*-------------------------------------------------------------------------
  
 */
void CBone::SetMatrix(float *m)
{
    bDirty = false; // not dirty when changing the matrix directly... no sub-components used
    m_validComps |= TRANSFCOMP_matrix|TRANSFCOMP_matrix_ready;
    m_validComps &= ~TRANSFCOMP_abs_matrix_ready|TRANSFCOMP_abs_matrix;
    memcpy(m_matrix.mat_array, m, sizeof(float)*16 );
}
/*-------------------------------------------------------------------------
  
 */
void CBone::SetAbsMatrix(float *m)
{
    bDirty = false; // not dirty when changing the matrix directly... no sub-components used
    m_validComps |= TRANSFCOMP_abs_matrix;
    m_validComps &= ~TRANSFCOMP_matrix_ready|TRANSFCOMP_matrix;
    memcpy(m_abs_matrix.mat_array, m, sizeof(float)*16 );
}

/*-------------------------------------------------------------------------
  
 */
void CBone::SetMatrixBindpose(float *m)
{
    if(m)
    {
        m_validComps |= TRANSFCOMP_bindpose_matrix;
        memcpy(m_bindpose_matrix.mat_array, m, sizeof(float)*16 );
    } else {
        // in this case we will rely on computation of matrices to create the bindpose
        memset(m_bindpose_matrix.mat_array, 0, sizeof(float)*16 );
        bDirty = true;
    }
}

/*-------------------------------------------------------------------------
  
 */
void CBone::recTransfUpdate(CBone *parent, bool bBindPose)
{
    ComputeMatrix(bBindPose);
    //
    // Children
    //
    std::vector<CBone*>::iterator iT = m_childrenTransf.begin();
    while(iT != m_childrenTransf.end() )
    {
        (*iT)->recTransfUpdate(this, bBindPose);
        ++iT;
    }

}

/*-------------------------------------------------------------------------
// Joint transformation order (OpenGL order) :
// P2 = Mtranslate * MjointOrient * Mrotation * Mrotorientation * Mscale * P
// basic Transformation order :
// P2 = MrotPivotTransl * MrotPivot * Mrotation * MrotOrient * MrotPivotInv * MscalePivotTransl 
//      * MscalePivot * Mscale * MscalePivotInv * P  
 */
bool CTransform::ComputeMatrix(bool bBindPose)
{
#if 0
    bool bParentChanged = false;
    bool bChanged = false;

    if(bDirty)
    {
        if(m_validComps & TRANSFCOMP_matrix)
        {
            m_matrix = mat4f(tr->matrix);
            bChanged = true; // to push abs matrix to compute
        } else
        {
            // because some components are now used, turn the matrix to Identity and re-compute it from scratch
            m_matrix.identity();
            // Joint transformation order (OpenGL order) :
            // P2 = Mtranslate * MparentScaleInverse * MjointOrient * Mrotation * Mrotorientation * Mscale * PosVec
            // P2 = [T]        * [IS]                * [JO]         * [R]       * [RO]            * [S]    * P
            // from maya doc : matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]
            // basic Transformation order :
            // P2 = Mtranslate * MrotPivotTransl * 
            //      MrotPivot * Mrotation * MrotOrient * MrotPivotInv * 
            //      MscalePivotTransl * MscalePivot * Mscale * MscalePivotInv * PosVec
            // In maya doc (DX matrix orientation) :
            //             -1                      -1
            // p' = p x [Sp]x[S]x[Sh]x[Sp]x[St]x[Rp]x[Ro]x[R]x[Rp]x[Rt]x[T]


            //translation (T)
            if(m_validComps & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(m_pos);
            }
            // parent scale inverse for JOINTs: TODO

            //rotation pivot translate (Rt)
            if(m_validComps & TRANSFCOMP_rotationPivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(m_rotationPivotTranslate);
            }
            //JOINT ONLY : orientation
            if(m_validComps & TRANSFCOMP_jointOrientation)
            {
                bChanged = true;
                m_matrix.rotate(quatf(m_jointOrientation[0], m_jointOrientation[1], m_jointOrientation[2], m_jointOrientation[3]));
            }
            //Rotation
            // rotation-pivot (Rp)
            if(m_validComps & TRANSFCOMP_rotationPivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_rotationPivot[0], m_rotationPivot[1], m_rotationPivot[2]));
            }
            {
                // TODO: take the order into account (R)
                if(m_validComps & TRANSFCOMP_rotation)
                {
                    bChanged = true;
                    quatf qz(vec3f(0,0,1), m_rotation[2]);// * nv_to_rad);
                    quatf qy(vec3f(0,1,0), m_rotation[1]);// * nv_to_rad);
                    quatf qx(vec3f(1,0,0), m_rotation[0]);// * nv_to_rad);
                    qz *= qy;
                    qz *= qx;
                    m_matrix.rotate(qz);
                    // make a copy of this rotation in the quaternion... to be consistent
                    memcpy(m_Quat.comp, qz.comp, sizeof(nv_scalar)*4);
                    //m_validComps |= TRANSFCOMP_Quat;
                }
                //rotation from a Quaternion (==R)
                else if(m_validComps & TRANSFCOMP_Quat)
                {
                    bChanged = true;
                    m_matrix.rotate(quatf(m_Quat[0], m_Quat[1], m_Quat[2], m_Quat[3]));
                }
                //rotation orientation (Ro)
                if(m_validComps & TRANSFCOMP_rotationOrientation)
                {
                    bChanged = true;
                    m_matrix.rotate(quatf(m_rotationOrientation[0], m_rotationOrientation[1], m_rotationOrientation[2], m_rotationOrientation[3]));
                }
            }
            // inverse rotation-pivot Inv(Rp)
            if(m_validComps & TRANSFCOMP_rotationPivot)
            {
                mat4f mrp, mrpI;
                mrp.identity();
                mrp.set_translation(vec3f(m_rotationPivot[0], m_rotationPivot[1], m_rotationPivot[2]));
                invert(mrpI, mrp);
                m_matrix *= mrpI;
            }
            // scale-pivot translation (St) introduced to preserve existing scale transformations when moving pivot. 
            // This is used to prevent the object from moving when the objects pivot point is not at the origin 
            // and a non-unit scale is applied to the object 
            if(m_validComps & TRANSFCOMP_scalePivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_scalePivotTranslate[0], m_scalePivotTranslate[1], m_scalePivotTranslate[2]));
            }

            // scale-pivot (Sp)
            if(m_validComps & TRANSFCOMP_scalePivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_scalePivot[0], m_scalePivot[1], m_scalePivot[2]));
            }
            {
                // According to maya doc, there should be a "Shearing" matrix, here... TODO?
                //scale (S)
                if(m_validComps & TRANSFCOMP_scale)
                {
                    bChanged = true;
                    m_matrix.scale(m_scale);
                }
            }
            // inverse scale-pivot Inv(Sp)
            if(m_validComps & TRANSFCOMP_scalePivot)
            {
                mat4f mscp, mscpI;
                mscp.identity();
                mscp.set_translation(vec3f(m_scalePivot[0], m_scalePivot[1], m_scalePivot[2]));
                invert(mscpI, mscp);
                m_matrix *= mscpI;
            }
        }
        bDirty = false;
    } //if(bDirty)
    // -----------------------------
    // Now work on matrices
    if(m_parentTransf)
        bParentChanged = m_parentTransf->ComputeMatrix(bBindPose);
    // re-compute absolute values (and bindpose if necessary)
    if((bParentChanged || bChanged))
    {
        bChanged = true;
        if(m_parentTransf)
            m_abs_matrix = mat4f(m_parentTransf->m_abs_matrix) * m_matrix;
        else
            m_abs_matrix = m_matrix;
    }
    // recompute the bindpose only if needed (i.e. the first time)
    if(bBindPose)
    {
        m_validComps |= TRANSFCOMP_bindpose_matrix;
        // TODO: The bindpose is more complex than this : we should compute it 
        // in the reference frame of the mesh's transform and not in world space
        invert(m_bindpose_matrix, m_abs_matrix);
    }
    m_validComps |= TRANSFCOMP_matrix|TRANSFCOMP_matrix_ready;
    m_validComps |= TRANSFCOMP_abs_matrix|TRANSFCOMP_abs_matrix_ready;
#else
    bool bChanged = false;
    if(bDirty)
    {
        if(m_validComps & TRANSFCOMP_matrix)
        {
            bChanged = true; // to push abs matrix to compute
        } else
        {
            //translation
            // the matrix was directly updated
            //if(!(((m_validComps & TRANSFCOMP_matrix) == m_validComps)
            //    || ((m_validComps & (TRANSFCOMP_matrix|TRANSFCOMP_abs_matrix)) == m_validComps)) )
            {
                m_matrix.identity();
            }
            // Joint transformation order (OpenGL order) :
            // P2 = Mtranslate * MparentScaleInverse * MjointOrient * Mrotation * Mrotorientation * Mscale * PosVec
            // P2 = [T]        * [IS]                * [JO]         * [R]       * [RO]            * [S]    * P
            // from maya doc : matrix = [S] * [RO] * [R] * [JO] * [IS] * [T]
            // basic Transformation order :
            // P2 = Mtranslate * MrotPivotTransl * 
            //      MrotPivot * Mrotation * MrotOrient * MrotPivotInv * 
            //      MscalePivotTransl * MscalePivot * Mscale * MscalePivotInv * PosVec
            // In maya doc (DX matrix orientation) :
            //             -1                      -1
            // p' = p x [Sp]x[S]x[Sh]x[Sp]x[St]x[Rp]x[Ro]x[R]x[Rp]x[Rt]x[T]

            //translation (T)
            if(m_validComps & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(m_pos);
            }
            // parent scale inverse for JOINTs: TODO

            //rotation pivot translate (Rt)
            if(m_validComps & TRANSFCOMP_rotationPivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(m_rotationPivotTranslate);
            }
            //joint orientation
            if(m_validComps & TRANSFCOMP_jointOrientation)
            {
                bChanged = true;
                m_matrix.rotate(m_jointOrientation);
            }
            // rotation-pivot (Rp)
            if(m_validComps & TRANSFCOMP_rotationPivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_rotationPivot[0], m_rotationPivot[1], m_rotationPivot[2]));
            }
            //Rotation
            // TODO: take the order into account
            if(m_validComps & TRANSFCOMP_rotation)
            {
                bChanged = true;
                // TODO: other than "xyz" !!
                // No need : use internal method from the quatf
                //quatf qz(vec3f(0,0,1), m_rotation.z * nv_to_rad);
                //quatf qy(vec3f(0,1,0), m_rotation.y * nv_to_rad);
                //quatf qx(vec3f(1,0,0), m_rotation.x * nv_to_rad);
                //qz *= qy;
                //qz *= qx;
                //m_matrix.rotate(qz);
                vec3f e(m_rotation);
                e *= nv_to_rad;
                quatf qz(e);
                m_matrix.rotate(qz);
                // make a copy of this rotation in the quaternion... to be consistent
                memcpy(m_Quat.comp, qz.comp, sizeof(nv_scalar)*4);
                m_validComps |= TRANSFCOMP_Quat_ready;
            }
            //rotation from a Quaternion (==R)
            else if(m_validComps & TRANSFCOMP_Quat)
            {
                bChanged = true;
                m_matrix.rotate(m_Quat);
            }
            //rotation orientation (Ro)
            if(m_validComps & TRANSFCOMP_rotationOrientation)
            {
                bChanged = true;
                m_matrix.rotate(m_rotationOrientation);
            }
            // inverse rotation-pivot Inv(Rp)
            if(m_validComps & TRANSFCOMP_rotationPivot)
            {
                bChanged = true;
                mat4f mrp, mrpI;
                mrp.identity();
                mrp.set_translation(vec3f(m_rotationPivot[0], m_rotationPivot[1], m_rotationPivot[2]));
                invert(mrpI, mrp);
                m_matrix *= mrpI;
            }
            // scale-pivot translation (St) introduced to preserve existing scale transformations when moving pivot. 
            // This is used to prevent the object from moving when the objects pivot point is not at the origin 
            // and a non-unit scale is applied to the object 
            if(m_validComps & TRANSFCOMP_scalePivotTranslate)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_scalePivotTranslate[0], m_scalePivotTranslate[1], m_scalePivotTranslate[2]));
            }

            // scale-pivot (Sp)
            if(m_validComps & TRANSFCOMP_scalePivot)
            {
                bChanged = true;
                m_matrix.translate(vec3f(m_scalePivot[0], m_scalePivot[1], m_scalePivot[2]));
            }
            //scale
            if(m_validComps & TRANSFCOMP_scale)
            {
                bChanged = true;
                m_matrix.scale(m_scale);
            }
            // inverse scale-pivot Inv(Sp)
            if(m_validComps & TRANSFCOMP_scalePivot)
            {
                bChanged = true;
                mat4f mscp, mscpI;
                mscp.identity();
                mscp.set_translation(vec3f(m_scalePivot[0], m_scalePivot[1], m_scalePivot[2]));
                invert(mscpI, mscp);
                m_matrix *= mscpI;
            }
            bDirty = false;
        }
    }
    bool bParentChanged = false;
    if(m_parentTransf)
        bParentChanged = m_parentTransf->ComputeMatrix(bBindPose);
    // re-compute absolute values (and bindpose if necessary)
    if(m_validComps & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
    {
        m_validComps |= TRANSFCOMP_abs_Quat_ready;
        quatf Q(m_Quat);
        if(m_parentTransf)
        {
            assert(m_validComps & TRANSFCOMP_abs_Quat_ready);
            quatf Qparent(m_parentTransf->m_abs_Quat);
            m_abs_Quat = Qparent * Q;
            m_abs_Quat.normalize(); // maybe we could this only fewer times
        } else {
            m_abs_Quat = Q;
        }
    }
    if(m_parentTransf && (bParentChanged || bChanged))
    {
        bChanged = true;
        m_abs_matrix = m_parentTransf->m_abs_matrix * m_matrix;
    } else if(bChanged) {
        m_abs_matrix = m_matrix;
    }
    m_validComps |= TRANSFCOMP_abs_matrix_ready | TRANSFCOMP_matrix_ready;
    // recompute the bindpose only if needed (i.e. the first time) or if requested
    if(bBindPose || (m_bindpose_matrix.mat_array[15] == 0.0))
    {
        m_validComps |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
    }
#endif
    return bChanged;
}
bool CTransformSimple::ComputeMatrix(bool bBindPose)
{
    bool bChanged = false;
    if(bDirty)
    {
        if(m_validComps & TRANSFCOMP_matrix)
        {
            bChanged = true; // to push abs matrix to compute
        } else
        {
            //translation
            // the matrix was directly updated
            //if(!(((m_validComps & TRANSFCOMP_matrix) == m_validComps)
            //    || ((m_validComps & (TRANSFCOMP_matrix|TRANSFCOMP_abs_matrix)) == m_validComps)) )
            {
                m_matrix.identity();
            }
            // Joint transformation order (OpenGL order) :
            // P2 = Mtranslate * Mrotation * Mscale * PosVec
            //translation (T)
            if(m_validComps & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(m_pos);
            }
            // parent scale inverse for JOINTs: TODO

            //Rotation
            // TODO: take the order into account
            //rotation from a Quaternion (==R)
            if(m_validComps & TRANSFCOMP_Quat)
            {
                bChanged = true;
                m_matrix.rotate(m_Quat);
            }
            //scale
            if(m_validComps & TRANSFCOMP_scale)
            {
                bChanged = true;
                m_matrix.scale(m_scale);
            }
            bDirty = false;
        }
    }
    bool bParentChanged = false;
    if(m_parentTransf)
        bParentChanged = m_parentTransf->ComputeMatrix(bBindPose);
    // re-compute absolute values (and bindpose if necessary)
    if(m_validComps & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
    {
        m_validComps |= TRANSFCOMP_abs_Quat_ready;
        quatf Q(m_Quat);
        if(m_parentTransf)
        {
            assert(m_validComps & TRANSFCOMP_abs_Quat_ready);
            quatf Qparent(m_parentTransf->m_abs_Quat);
            m_abs_Quat = Qparent * Q;
            m_abs_Quat.normalize(); // maybe we could this only fewer times
        } else {
            m_abs_Quat = Q;
        }
    }
    if(m_parentTransf && (bParentChanged || bChanged))
    {
        bChanged = true;
        m_abs_matrix = m_parentTransf->m_abs_matrix * m_matrix;
    } else if(bChanged) {
        m_abs_matrix = m_matrix;
    }
    m_validComps |= TRANSFCOMP_abs_matrix_ready | TRANSFCOMP_matrix_ready;
    // recompute the bindpose only if needed (i.e. the first time) or if requested
    if(bBindPose || (m_bindpose_matrix.mat_array[15] == 0.0))
    {
        m_validComps |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
    }
    return bChanged;
}
bool CBone::ComputeMatrix(bool bBindPose)
{
    bool bChanged = false;
    if(bDirty)
    {
        if(m_validComps & TRANSFCOMP_matrix)
        {
            bChanged = true; // to push abs matrix to compute
            // compute the quaternion rotation from it, for consistency
            // warning: scaling could be a problem
            m_Quat.from_matrix(m_matrix);
            m_validComps |= TRANSFCOMP_Quat;
            m_pos = vec3f(m_matrix.mat_array+12);
            m_validComps |= TRANSFCOMP_pos;
        } else {
            //translation
            // the matrix was directly updated
            //if(!(((m_validComps & TRANSFCOMP_matrix) == m_validComps)
            //    || ((m_validComps & (TRANSFCOMP_matrix|TRANSFCOMP_abs_matrix)) == m_validComps)) )
            {
                m_matrix.identity();
            }
            // Joint transformation order (OpenGL order) :
            // P2 = Mtranslate * Mrotation * Mscale * PosVec
            //translation (T)
            if(m_validComps & TRANSFCOMP_pos)
            {
                bChanged = true;
                m_matrix.translate(m_pos);
            }
            // parent scale inverse for JOINTs: TODO

            //Rotation
            // TODO: take the order into account
            //rotation from a Quaternion (==R)
            if(m_validComps & TRANSFCOMP_Quat)
            {
                bChanged = true;
                m_matrix.rotate(m_Quat);
            }
            bDirty = false;
        }
    }
    bool bParentChanged = false;
    if(m_parentTransf)
        bParentChanged = m_parentTransf->ComputeMatrix(bBindPose);
    // re-compute absolute values (and bindpose if necessary)
    if(m_validComps & (TRANSFCOMP_Quat|TRANSFCOMP_Quat_ready))
    {
        m_validComps |= TRANSFCOMP_abs_Quat_ready;
        quatf Q(m_Quat);
        if(m_parentTransf)
        {
            assert(m_validComps & TRANSFCOMP_abs_Quat_ready);
            quatf Qparent(m_parentTransf->m_abs_Quat);
            m_abs_Quat = Qparent * Q;
            m_abs_Quat.normalize(); // maybe we could this only fewer times
        } else {
            m_abs_Quat = Q;
        }
    }
    if(m_parentTransf && (bParentChanged || bChanged))
    {
        bChanged = true;
        m_abs_matrix = m_parentTransf->m_abs_matrix * m_matrix;
    } else if(bChanged) {
        m_abs_matrix = m_matrix;
    }
    m_validComps |= TRANSFCOMP_abs_matrix_ready | TRANSFCOMP_matrix_ready;
    // recompute the bindpose only if needed (i.e. the first time) or if requested
    if(bBindPose || (m_bindpose_matrix.mat_array[15] == 0.0))
    {
        m_validComps |= TRANSFCOMP_bindpose_matrix;
        invert(m_bindpose_matrix, m_abs_matrix);
    }
    return bChanged;
}
bool CBone::Connect(bk3dlib::QuatCurve *pCV, bk3dlib::TransfComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    connectedCurveVectors.insert(std::make_pair(pCV, comp) );
    //    return true;
    //}
    return true;
}

bool CBone::Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::TransfComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    return false;
    //}
    //connectedCurveVectors.erase(iT);
    return true;
}
bool CBone::Connect(bk3dlib::CurveVec *pCV, bk3dlib::TransfComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        connectedCurveVectors.insert(std::make_pair(pCV, comp) );
        return true;
    }
    return true;
}

bool CBone::Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::TransfComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        return false;
    }
    connectedCurveVectors.erase(iT);
    return true;
}
bool CIKHandle::Connect(bk3dlib::CurveVec *pCV, bk3dlib::IKHandleComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        connectedCurveVectors.insert(std::make_pair(pCV, comp) );
        return true;
    }
    return true;
}
bool CIKHandle::Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::IKHandleComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        return false;
    }
    connectedCurveVectors.erase(iT);
    return true;
}
bool CIKHandle::Connect(bk3dlib::QuatCurve *pCV, bk3dlib::IKHandleComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    connectedCurveVectors.insert(std::make_pair(pCV, comp) );
    //    return true;
    //}
    return true;
}
bool CIKHandle::Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::IKHandleComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    return false;
    //}
    //connectedCurveVectors.erase(iT);
    return true;
}

bool CMesh::Connect(bk3dlib::QuatCurve *pCV, bk3dlib::MeshComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    connectedCurveVectors.insert(std::make_pair(pCV, comp) );
    //    return true;
    //}
    return true;
}
bool CMesh::Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::MeshComponent comp)
{
    //if(!pCV)
    //    return false;
    //std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    //if(iT == connectedCurveVectors.end())
    //{
    //    return false;
    //}
    //connectedCurveVectors.erase(iT);
    return true;
}

bool CMesh::Connect(bk3dlib::CurveVec *pCV, bk3dlib::MeshComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        connectedCurveVectors.insert(std::make_pair(pCV, comp) );
        return true;
    }
    return true;
}
bool CMesh::Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::MeshComponent comp)
{
    if(!pCV)
        return false;
    std::set< CurveVecConnection >::iterator iT = connectedCurveVectors.find(std::make_pair(pCV, comp));
    if(iT == connectedCurveVectors.end())
    {
        return false;
    }
    connectedCurveVectors.erase(iT);
    return true;
}

bk3dlib::PTransformDOF  CBone::CreateDOF()
{
    if(!m_TransfDOF)
        m_TransfDOF = new CTransformDOF;
    return m_TransfDOF;
}
void                    CBone::DeleteDOF()
{
    if(m_TransfDOF)
        delete m_TransfDOF;
    m_TransfDOF = NULL;
}
bk3dlib::PTransformDOF CBone::GetDOF()
{
    return m_TransfDOF;
}
/*=========================================================================
  TransformDOF
 */
void CTransformDOF::SetDOFValues(bk3dlib::TransformDOFMode  mode, float *DOFAlpha, float *AxisLimitStart, float *AxisLimitRange)
{
    m_mode = mode;
    if(DOFAlpha) m_DOFAlpha = *DOFAlpha;
    if(AxisLimitStart) m_AxisLimitStart = *AxisLimitStart;
    if(AxisLimitRange) m_AxisLimitRange = *AxisLimitRange;
}
bk3dlib::TransformDOFMode CTransformDOF::GetDOFValues(float *DOFAlpha, float *AxisLimitStart, float *AxisLimitRange)
{
    if(DOFAlpha) *DOFAlpha = m_DOFAlpha;
    if(AxisLimitStart) *AxisLimitStart = m_AxisLimitStart;
    if(AxisLimitRange) *AxisLimitRange = m_AxisLimitRange;
    return m_mode;
}
void CTransformDOF::GetQuaternion(float &x, float &y, float &z, float &w)
{
    x = m_quat[0];
    y = m_quat[1];
    z = m_quat[2];
    w = m_quat[3];
}
void CTransformDOF::SetQuaternion(float x, float y, float z, float w)
{
    m_quat[0] = x;
    m_quat[1] = y;
    m_quat[2] = z;
    m_quat[3] = w;
}

void CTransformDOF::SetQuaternionFromEulerXYZ(float x, float y, float z)
{
    m_quat.from_euler_xyz(vec3f(x,y,z));
}

/*=========================================================================
  CURVES
 */

/*-------------------------------------------------------------------------
  
 */
bool CMayaCurveVector::Connect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp)
{
    if(!pTransf)
        return false;
    std::set< TransformConnection >::iterator iT = connectedTransforms.find(std::make_pair(pTransf, comp));
    if(iT == connectedTransforms.end())
    {
        connectedTransforms.insert(std::make_pair(pTransf, comp) );
        CBone *tr = static_cast<CBone *>(pTransf);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CMayaCurveVector::Disconnect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp)
{
    if(!pTransf)
        return false;
    std::set< TransformConnection >::iterator iT = connectedTransforms.find(std::make_pair(pTransf, comp));
    if(iT == connectedTransforms.end())
    {
        return false;
    }
    connectedTransforms.erase(iT);
    CBone *tr = static_cast<CBone *>(pTransf);
    tr->Disconnect(this, comp);
    return true;
}

bool CMayaCurveVector::Connect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp)
{
    if(!pMesh)
        return false;
    std::set< MeshConnection >::iterator iT = connectedMeshes.find(std::make_pair(pMesh, comp));
    if(iT == connectedMeshes.end())
    {
        connectedMeshes.insert(std::make_pair(pMesh, comp) );
        CMesh *tr = static_cast<CMesh *>(pMesh);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CMayaCurveVector::Disconnect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp)
{
    if(!pMesh)
        return false;
    std::set< MeshConnection >::iterator iT = connectedMeshes.find(std::make_pair(pMesh, comp));
    if(iT == connectedMeshes.end())
    {
        return false;
    }
    connectedMeshes.erase(iT);
    CMesh *tr = static_cast<CMesh *>(pMesh);
    tr->Disconnect(this, comp);
    return true;
}
bool CMayaCurveVector::Connect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp)
{
    if(!pIKHandle)
        return false;
    std::set< IKHandleConnection >::iterator iT = connectedIKHandles.find(std::make_pair(pIKHandle, comp));
    if(iT == connectedIKHandles.end())
    {
        connectedIKHandles.insert(std::make_pair(pIKHandle, comp) );
        CIKHandle *tr = static_cast<CIKHandle *>(pIKHandle);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CMayaCurveVector::Disconnect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp)
{
    if(!pIKHandle)
        return false;
    std::set< IKHandleConnection >::iterator iT = connectedIKHandles.find(std::make_pair(pIKHandle, comp));
    if(iT == connectedIKHandles.end())
    {
        return false;
    }
    connectedIKHandles.erase(iT);
    CIKHandle *tr = static_cast<CIKHandle *>(pIKHandle);
    tr->Disconnect(this, comp);
    return true;
}


void        CMayaCurveVector::SetProps(EtInfinityType preInfinity,
                    EtInfinityType postInfinity,
                    bool    inputIsTime,
                    bool    outputIsAngular,
                    bool    isWeighted)
{
    m_preInfinity = preInfinity;    // how to evaluate pre-infinity            
    m_postInfinity = postInfinity;    // how to evaluate post-infinity        
    m_inputIsTime = inputIsTime;    // if true, the input do not need Plugs to increase
    m_outputIsAngular = outputIsAngular;
    m_isWeighted = isWeighted;    // whether or not this curve has weighted tangents 
    for(int i=0; i<(int)m_mayacurves.size(); i++)
    {
        if(!m_mayacurves[i])
            continue;
        m_mayacurves[i]->m_preInfinity = preInfinity;    // how to evaluate pre-infinity            
        m_mayacurves[i]->m_postInfinity = postInfinity;    // how to evaluate post-infinity        
        m_mayacurves[i]->m_inputIsTime = inputIsTime;    // if true, the input do not need Plugs to increase
        m_mayacurves[i]->m_outputIsAngular = outputIsAngular;
        m_mayacurves[i]->m_isWeighted = isWeighted;    // whether or not this curve has weighted tangents 
    }
}
void        CMayaCurveVector::SetPropsByComp(int comp, EtInfinityType preInfinity,
                        EtInfinityType postInfinity,
                        bool    inputIsTime=true,
                        bool    outputIsAngular=true,
                        bool    isWeighted=true)
{
    if((comp < (int)m_mayacurves.size())&&(m_mayacurves[comp]))
    {
        m_mayacurves[comp]->m_preInfinity = preInfinity;    // how to evaluate pre-infinity            
        m_mayacurves[comp]->m_postInfinity = postInfinity;    // how to evaluate post-infinity        
        m_mayacurves[comp]->m_inputIsTime = inputIsTime;    // if true, the input do not need Plugs to increase
        m_mayacurves[comp]->m_outputIsAngular = outputIsAngular;
        m_mayacurves[comp]->m_isWeighted = isWeighted;    // whether or not this curve has weighted tangents 
    }
}
void        CMayaCurveVector::GetPropsByComp(int comp, EtInfinityType *preInfinity,
                        EtInfinityType *postInfinity,
                        bool    *inputIsTime,
                        bool    *outputIsAngular,
                        bool    *isWeighted)
{
    if((comp >= (int)m_mayacurves.size())||(!m_mayacurves[comp]))
        return;
    if(preInfinity) *preInfinity = m_mayacurves[comp]->m_preInfinity;    // how to evaluate pre-infinity            
    if(postInfinity) *postInfinity = m_mayacurves[comp]->m_postInfinity;    // how to evaluate post-infinity        
    if(inputIsTime) *inputIsTime = m_mayacurves[comp]->m_inputIsTime;    // if true, the input do not need Plugs to increase
    if(outputIsAngular) *outputIsAngular = m_mayacurves[comp]->m_outputIsAngular;
    if(isWeighted) *isWeighted = m_mayacurves[comp]->m_isWeighted;    // whether or not this curve has weighted tangents 
}
void        CMayaCurveVector::GetProps(EtInfinityType *preInfinity,
                    EtInfinityType *postInfinity,
                    bool    *inputIsTime,
                    bool    *outputIsAngular,
                    bool    *isWeighted)
{
    if(preInfinity) *preInfinity = m_preInfinity;    // how to evaluate pre-infinity            
    if(postInfinity) *postInfinity = m_postInfinity;    // how to evaluate post-infinity        
    if(inputIsTime) *inputIsTime = m_inputIsTime;    // if true, the input do not need Plugs to increase
    if(outputIsAngular) *outputIsAngular = m_outputIsAngular;
    if(isWeighted) *isWeighted = m_isWeighted;    // whether or not this curve has weighted tangents 
}
int         CMayaCurveVector::GetNumKeys(int comp)
{
    if((comp > (int)m_mayacurves.size())||(m_mayacurves[comp]==NULL))
        return 0;
    return (int)m_mayacurves[comp]->m_keys.size();
}
int         CMayaCurveVector::DeleteKey(int comp, int keynum)
{
    if((comp > (int)m_mayacurves.size())||(m_mayacurves[comp]==NULL))
        return 0;
    if(keynum >= (int)m_mayacurves[comp]->m_keys.size())
        return 0;
    std::vector<bk3d::MayaReadKey>::iterator iK = m_mayacurves[comp]->m_keys.begin();
    while(keynum >= 0)
        ++iK;
    m_mayacurves[comp]->m_keys.erase(iK);
    return 1;
}
/// separate assignment of Keys for each component (Maya does this way)
int         CMayaCurveVector::AddKey(int comp, float time, float value, 
                        EtTangentType inTType, EtTangentType outTType,
                        float inAngle, float inWeight,
                        float outAngle, float outWeight
                        )
{
    if((comp > (int)m_mayacurves.size())||(m_mayacurves[comp]==NULL))
        return 0;
    bk3d::MayaReadKey key;
    key.time = time;
    key.value = value;
    key.inTangentType = (bk3d::EtTangentType)inTType;
    key.outTangentType = (bk3d::EtTangentType)outTType;
    key.inAngle = inAngle;
    key.inWeight = inWeight;
    key.outAngle = outAngle;
    key.outWeight = outWeight;
    m_mayacurves[comp]->m_keys.push_back(key);
    return (int)m_mayacurves[comp]->m_keys.size();
}
/// Add key for all components at the same time : N values for N components. The other params are shared
int         CMayaCurveVector::AddKey(float time, float *value, 
                        EtTangentType inTType, EtTangentType outTType,
                        float inAngle, float inWeight,
                        float outAngle, float outWeight
                        )
{
    bk3d::MayaReadKey key;
    key.time = time;
    key.inTangentType = (bk3d::EtTangentType)inTType;
    key.outTangentType = (bk3d::EtTangentType)outTType;
    key.inAngle = inAngle;
    key.inWeight = inWeight;
    key.outAngle = outAngle;
    key.outWeight = outWeight;
    for(int comp=0 ; comp < (int)m_mayacurves.size(); comp++)
    {
        if(m_mayacurves[comp]==NULL)
            continue;
        key.value = value[comp];
        m_mayacurves[comp]->m_keys.push_back(key);
    }
    return 1;
}
void        CMayaCurveVector::GetKey(int comp, int key, float *time, float *value, 
                        EtTangentType *inTType, EtTangentType *outTType,
                        float *inAngle, float *inWeight,
                        float *outAngle, float *outWeight
                        )
{
    if((comp > (int)m_mayacurves.size())||(m_mayacurves[comp]==NULL)||(key >= (int)m_mayacurves[comp]->m_keys.size()))
        return /*false*/;
    bk3d::MayaReadKey & keydata = m_mayacurves[comp]->m_keys[key];
    if(time) *time = keydata.time;
    if(value) *value = keydata.value;
    if(inTType) *inTType = (bk3dlib::CurveVec::EtTangentType)keydata.inTangentType;
    if(outTType) *outTType = (bk3dlib::CurveVec::EtTangentType)keydata.outTangentType;
    if(inAngle) *inAngle  = keydata.inAngle;
    if(inWeight) *inWeight = keydata.inWeight;
    if(outAngle) *outAngle = keydata.outAngle;
    if(outWeight) *outWeight = keydata.outWeight;
}
/*-------------------------------------------------------------------------
  Quaternion 'curve'
 */
CQuatCurve::~CQuatCurve()
{ 
    MapQuatCurve::iterator it = g_quatcurves.find(static_cast<bk3dlib::PQuatCurve>(this));
    if(it != g_quatcurves.end() )
    {
        g_quatcurves.erase(it);
    }
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachQuatCurve(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CQuatCurve::~CQuatCurve() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
void CQuatCurve::Destroy()        
{ 
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
bool CQuatCurve::Connect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp)
{
    if(!pTransf)
        return false;
    std::set< TransformConnection >::iterator iT = connectedTransforms.find(std::make_pair(pTransf, comp));
    if(iT == connectedTransforms.end())
    {
        connectedTransforms.insert(std::make_pair(pTransf, comp) );
        CBone *tr = static_cast<CBone *>(pTransf);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CQuatCurve::Disconnect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp)
{
    if(!pTransf)
        return false;
    std::set< TransformConnection >::iterator iT = connectedTransforms.find(std::make_pair(pTransf, comp));
    if(iT == connectedTransforms.end())
    {
        return false;
    }
    connectedTransforms.erase(iT);
    CBone *tr = static_cast<CBone *>(pTransf);
    tr->Disconnect(this, comp);
    return true;
}

bool CQuatCurve::Connect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp)
{
    if(!pMesh)
        return false;
    std::set< MeshConnection >::iterator iT = connectedMeshes.find(std::make_pair(pMesh, comp));
    if(iT == connectedMeshes.end())
    {
        connectedMeshes.insert(std::make_pair(pMesh, comp) );
        CMesh *tr = static_cast<CMesh *>(pMesh);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CQuatCurve::Disconnect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp)
{
    if(!pMesh)
        return false;
    std::set< MeshConnection >::iterator iT = connectedMeshes.find(std::make_pair(pMesh, comp));
    if(iT == connectedMeshes.end())
    {
        return false;
    }
    connectedMeshes.erase(iT);
    CMesh *tr = static_cast<CMesh *>(pMesh);
    tr->Disconnect(this, comp);
    return true;
}
bool CQuatCurve::Connect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp)
{
    if(!pIKHandle)
        return false;
    std::set< IKHandleConnection >::iterator iT = connectedIKHandles.find(std::make_pair(pIKHandle, comp));
    if(iT == connectedIKHandles.end())
    {
        connectedIKHandles.insert(std::make_pair(pIKHandle, comp) );
        CIKHandle *tr = static_cast<CIKHandle *>(pIKHandle);
        tr->Connect(this, comp);
        return true;
    }
    return true;
}
bool CQuatCurve::Disconnect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp)
{
    if(!pIKHandle)
        return false;
    std::set< IKHandleConnection >::iterator iT = connectedIKHandles.find(std::make_pair(pIKHandle, comp));
    if(iT == connectedIKHandles.end())
    {
        return false;
    }
    connectedIKHandles.erase(iT);
    CIKHandle *tr = static_cast<CIKHandle *>(pIKHandle);
    tr->Disconnect(this, comp);
    return true;
}

int         CQuatCurve::GetNumKeys()
{
    return 0;//(int)m_quatcurves->m_keys.size();
}
int         CQuatCurve::DeleteKey(int keynum)
{
    //if(keynum >= (int)m_quatcurves->m_keys.size())
    //    return 0;
    //std::vector<bk3d::MayaReadKey>::iterator iK = m_quatcurves->m_keys.begin();
    //while(keynum >= 0)
    //    ++iK;
    //m_quatcurves->m_keys.erase(iK);
    return 1;
}

/// Add key for all components at the same time : N values for N components. The other params are shared
int         CQuatCurve::AddKey(float time, float *quatVals)
{
    bk3d::QuatReadKey key;
    key.time = time;
    memcpy(key.value, quatVals, sizeof(float)*4);
    m_keys.push_back(key);
    return (int)m_keys.size();
}
void        CQuatCurve::GetKey(int key, float *time, float *quatVals)
{
    if(key >= (int)m_keys.size())
        return /*false*/;
    bk3d::QuatReadKey & keydata = m_keys[key];
    if(time) *time = keydata.time;
    if(quatVals) memcpy(quatVals, keydata.value, sizeof(float)*4);
}

#endif

/*-------------------------------------------------------------------------
  User Data methods
 */
void  CFileHeader::SetUserData(void *p)    { m_userData = p; }
void *CFileHeader::GetUserData()        { return m_userData; }
void  CMesh::SetUserData(void *p)    { m_userData = p; }
void *CMesh::GetUserData()        { return m_userData; }
void  CBone::SetUserData(void *p)    { m_userData = p; }
void *CBone::GetUserData()        { return m_userData; }
void  CMayaCurveVector::SetUserData(void *p)    { m_userData = p; }
void *CMayaCurveVector::GetUserData()        { return m_userData; }
void  CQuatCurve::SetUserData(void *p)    { m_userData = p; }
void *CQuatCurve::GetUserData()        { return m_userData; }
void  CMaterial::SetUserData(void *p)    { m_userData = p; }
void *CMaterial::GetUserData()        { return m_userData; }
void  CBuffer::SetUserData(void *p)    { m_userData = p; }
void *CBuffer::GetUserData()        { return m_userData; }

/*-------------------------------------------------------------------------
  
 */
CFileHeader::~CFileHeader()
{
    MapFileHeader::iterator it = g_fileHeaders.find(static_cast<bk3dlib::PFileHeader>(this));
    if(it != g_fileHeaders.end() )
        g_fileHeaders.erase(it);

    if(m_bk3dPool)
        delete m_bk3dPool;
    if(m_bk3dPool2)
        delete m_bk3dPool2;
    m_bk3dPool  = NULL;
    m_bk3dPool2 = NULL;
    m_fileHeader = NULL;
}
void CFileHeader::Destroy(bool bKeepBakedData)
{
    if(bKeepBakedData)
    {
        // might be asked to not delete data generated by the library
        // orphan them: it will be the task of the user to release this memory!
        if(m_bk3dPool)
        {
            m_bk3dPool->orphanPool();
            delete m_bk3dPool;
        }
        if(m_bk3dPool2)
        {
            m_bk3dPool2->orphanPool();
            delete m_bk3dPool2;
        }
        m_bk3dPool  = NULL;
        m_bk3dPool2 = NULL;
        m_fileHeader = NULL;
    }
    MapFileHeader::iterator it = g_fileHeaders.find(static_cast<bk3dlib::PFileHeader>(this));
    if(it != g_fileHeaders.end() )
        g_fileHeaders.erase(it);
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
CMesh::~CMesh()
{
    MapMesh::iterator it = g_meshes.find(static_cast<bk3dlib::PMesh>(this));
    if(it != g_meshes.end() )
        g_meshes.erase(it);
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachMesh(this);
        ++iFH;
    }
}
/*-------------------------------------------------------------------------
  
 */
void CMesh::Destroy()        
{ 
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
CTransform::~CTransform()        
{ 
    if(m_parentTransf)
        SetParent(NULL);
    std::vector<CBone*>::iterator iT = m_childrenTransf.begin();
    while(iT != m_childrenTransf.end() )
    {
        (*iT)->m_parentTransf = NULL;
        ++iT;
    }

    VecTransform::iterator it = g_transforms.begin();
    while(it != g_transforms.end() )
    {
        if((*it)==this) {
            g_transforms.erase(it);
            break;
        }
        ++it;
    }
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachTransform(this->AsBone());
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CTransform::~CTransform() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
CTransformSimple::~CTransformSimple()        
{ 
    if(m_parentTransf)
        SetParent(NULL);
    std::vector<CBone*>::iterator iT = m_childrenTransf.begin();
    while(iT != m_childrenTransf.end() )
    {
        (*iT)->m_parentTransf = NULL;
        ++iT;
    }

    VecTransform::iterator it = g_transforms.begin();
    while(it != g_transforms.end() )
    {
        if((*it)==this) {
            g_transforms.erase(it);
            break;
        }
        ++it;
    }
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachTransform(this->AsBone());
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CTransformSimple::~CTransformSimple() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
CBone::~CBone()        
{ 
    DeleteDOF();
    if(m_parentTransf)
        SetParent(NULL);
    std::vector<CBone*>::iterator iT = m_childrenTransf.begin();
    while(iT != m_childrenTransf.end() )
    {
        (*iT)->m_parentTransf = NULL;
        ++iT;
    }

    VecTransform::iterator it = g_transforms.begin();
    while(it != g_transforms.end() )
    {
        if((*it)==this) {
            g_transforms.erase(it);
            break;
        }
        ++it;
    }
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachTransform(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CBone::~CBone() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
void CTransform::Destroy()
{ 
    delete this;
}
void CTransformSimple::Destroy()
{ 
    delete this;
}
void CBone::Destroy()
{ 
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
bk3dlib::PQuatCurve bk3dlib::QuatCurve::Create(LPCSTR name)
{
    MapQuatCurve::iterator iM = g_quatcurves.begin();
    while(iM != g_quatcurves.end())
    {
        if(!strcmp(iM->second->GetName(), name))
            return NULL;
        ++iM;
    }
    CQuatCurve* p = new CQuatCurve();
    assert(p);
    p->SetName(name);
    g_quatcurves[p] = p;
    return p;
}

/*-------------------------------------------------------------------------
  
 */
bk3dlib::PCurveVec bk3dlib::CurveVec::Create(LPCSTR name, int ncomps)
{
    if((!name)||(ncomps<1))
        return NULL;
    MapCurveVec::iterator iM = g_mayacurves.begin();
    while(iM != g_mayacurves.end())
    {
        if(!strcmp(iM->second->GetName(), name))
            return NULL;
        ++iM;
    }
    CMayaCurveVector* p = new CMayaCurveVector();
    assert(p);
    p->SetName(name);
    //assert(ncomps < 10); // dumb safety check...
    for(int i=0; i<ncomps; i++)
    {
        CMayaCurve * pMCv = new CMayaCurve(p);
        p->m_mayacurves.push_back(pMCv);
    }
    g_mayacurves[p] = p;
    return p;
}


CMayaCurveVector::~CMayaCurveVector()
{ 
    MapCurveVec::iterator it = g_mayacurves.find(static_cast<bk3dlib::PCurveVec>(this));
    if(it != g_mayacurves.end() )
    {
        g_mayacurves.erase(it);
    }
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachCurveVec(this);
        ++iFH;
    }
    for(int i=0; i<(int)m_mayacurves.size(); i++)
    {
        delete m_mayacurves[i];
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CMayaCurveVector::~CMayaCurveVector() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
void CMayaCurveVector::Destroy()        
{ 
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
CMaterial::~CMaterial()
{ 
    MapMaterial::iterator it = g_materials.find(static_cast<bk3dlib::PMaterial>(this));
    if(it != g_materials.end() )
        g_materials.erase(it);
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachMaterial(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CMaterial::~CMaterial() : DETACH from possible users")
}
void CMaterial::Destroy()
{ 
    delete this;
}
/*-------------------------------------------------------------------------
  
 */
CBuffer::~CBuffer()
{
    MapBuffer::iterator it = g_buffers.find(static_cast<bk3dlib::PBuffer>(this));
    if(it != g_buffers.end() )
        g_buffers.erase(it);

    MapMesh::iterator iM = g_meshes.begin();
    while(iM != g_meshes.end() )
    {
        CMesh * pM = iM->second;
        pM->DetachBuffer(this);
        ++iM;
    }

    m_name.clear();
    m_numcomp = 0;
    m_bufNum = 0;
    m_slot = 0;
    m_curItem = 0;
    m_usage = BufferForVtx;
    m_divisor=1;
    m_type = bk3dlib::UNKNOWN;
    m_pOwner = NULL;
    m_idxset.clearIndex();
    m_bk3dptr = NULL;
    m_FVals.clear();
    m_UIVals.clear();
    m_SIBBuffers.clear();
    m_userData = NULL;
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CBuffer::~CBuffer() : DETACH from possible users")
}
/*-------------------------------------------------------------------------
  
 */
void CBuffer::Destroy()
{
    delete this;
}

/*=========================================================================
  IKHANDLE
 */

CIKHandle::CIKHandle(LPCSTR name) : CBone(name)
{
    if(name)
        m_name = name;
    m_effectorTransformStart = NULL;
    m_effectorTransformEnd = NULL;
    //m_handleTransforms = NULL;
    m_bk3dHandle = NULL;
    //m_HandlePos = vec3f(0,0,0);
    //m_EffectorPos = vec3f(0,0,0);
    m_priority = 1;
    m_weight = 1.0;
    m_maxIter = 1;
    m_mode = Default;
}


CIKHandle::~CIKHandle()
{ 
    MapIKHandle::iterator it = g_ikHandles.find(static_cast<bk3dlib::PIKHandle>(this));
    if(it != g_ikHandles.end() )
        g_ikHandles.erase(it);
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachIKHandle(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CBone::~CBone() : DETACH from possible users")
}
void        CIKHandle::Destroy()
{
    delete this;
}

//virtual bool connectCurve(PCurveVec pCVec, IKHandleComponent comp, int compOffset=0) = 0;
//virtual bool disconnectCurve(PCurveVec pCVec, IKHandleComponent comp=IK_DEFCOMP, int compOffset=0) = 0;

void CIKHandle::setHandlePos(float x, float y, float z)
{
    //m_HandlePos = vec3f(x,y,z);
	// now directly using matrix we inherit from
	SetPos(x,y,z);
}
//void CIKHandle::setEffectorPos(float x, float y, float z)
//{
//    m_EffectorPos = vec3f(x,y,z);
//}
void CIKHandle::setEffectorTransformStart(bk3dlib::PBone pT)
{
    m_effectorTransforms.clear();
    m_weights.clear();
    m_effectorTransformStart = static_cast<CBone*>(pT->AsBone());
}
void CIKHandle::setEffectorTransformEnd(bk3dlib::PBone pT)
{
    m_effectorTransforms.clear();
    m_weights.clear();
    m_effectorTransformEnd = static_cast<CBone*>(pT->AsBone());
}
void CIKHandle::addEffectorTransform(bk3dlib::PBone pT, float weight)
{
    m_effectorTransforms.push_back(static_cast<CBone*>(pT->AsBone()) );
    m_weights.push_back(weight);
    m_effectorTransformStart = NULL;
    m_effectorTransformEnd = NULL;
}
void CIKHandle::clearEffectorTransforms()
{
    m_effectorTransforms.clear();
    m_weights.clear();
    m_effectorTransformStart = NULL;
    m_effectorTransformEnd = NULL;
}

/*=========================================================================
  PhConstraint
 */

CPhConstraint::CPhConstraint(LPCSTR name)
{
    if(name)
        m_name = name;
    m_pRigidBody1 = NULL;
    m_pRigidBody2 = NULL;
    memset(m_translation_limit_min,0, sizeof(float)*3);
    memset(m_translation_limit_max,0, sizeof(float)*3);
    memset(m_rotation_limit_min,0, sizeof(float)*3);
    memset(m_rotation_limit_max,0, sizeof(float)*3);
    memset(m_spring_constant_translation,0, sizeof(float)*3);
    memset(m_spring_constant_rotation,0, sizeof(float)*3);
    m_bk3dConstraint = NULL;
}
CPhConstraint::~CPhConstraint()
{
    MapPhConstraint::iterator it = g_phConstraints.find(static_cast<bk3dlib::PPhConstraint>(this));
    if(it != g_phConstraints.end() )
        g_phConstraints.erase(it);
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachPhConstraint(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CBone::~CBone() : DETACH from possible users")
}

void CPhConstraint::Destroy()
{
    delete this;
}


void CPhConstraint::linkRigidBodies(bk3dlib::PPhRigidBody pRigidBody1, bk3dlib::PPhRigidBody pRigidBody2)
{
    m_pRigidBody1 = dynamic_cast<CPhRigidBody*>(pRigidBody1);
    m_pRigidBody2 = dynamic_cast<CPhRigidBody*>(pRigidBody2);
}
void CPhConstraint::setTranslationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
    m_translation_limit_min[0] = minx;
    m_translation_limit_min[1] = miny;
    m_translation_limit_min[2] = minz;
    m_translation_limit_max[0] = maxx;
    m_translation_limit_max[1] = maxy;
    m_translation_limit_max[2] = maxz;
}
void CPhConstraint::setRotationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz)
{
    m_rotation_limit_min[0] = minx;
    m_rotation_limit_min[1] = miny;
    m_rotation_limit_min[2] = minz;
    m_rotation_limit_max[0] = maxx;
    m_rotation_limit_max[1] = maxy;
    m_rotation_limit_max[2] = maxz;
}
void CPhConstraint::setSpringConstantTranslation(float x, float y, float z)
{
    m_spring_constant_translation[0] = x;
    m_spring_constant_translation[1] = y;
    m_spring_constant_translation[2] = z;
}
void CPhConstraint::setSpringConstantRotation(float x, float y, float z)
{
    m_spring_constant_rotation[0] = x;
    m_spring_constant_rotation[1] = y;
    m_spring_constant_rotation[2] = z;
}

/*=========================================================================
  PhRigidBody
 */

CPhRigidBody::CPhRigidBody(LPCSTR name)
{
    if(name)
        m_name = name;
    m_type              = Sphere;
    m_grp               = 0;
    m_collision_group_mask = 0;
    m_mode              = 0;
    m_shape_size[0]     = 0.0f;
    m_shape_size[1]     = 0.0f;
    m_shape_size[2]     = 0.0f;
    m_mass              = 0.0f;
    m_linear_dampening  = 0.0f;
    m_angular_dampening = 0.0f;
    m_restitution       = 0.0f;
    m_friction          = 0.0f;
    m_bk3dRigidBody     = NULL;
}
CPhRigidBody::~CPhRigidBody()
{
    MapPhRigidBody::iterator it = g_phRigidBodies.find(static_cast<bk3dlib::PPhRigidBody>(this));
    if(it != g_phRigidBodies.end() )
        g_phRigidBodies.erase(it);
    MapFileHeader::iterator iFH = g_fileHeaders.begin();
    while(iFH != g_fileHeaders.end())
    {
        iFH->second->DetachPhRigidBody(this);
        ++iFH;
    }
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - CBone::~CBone() : DETACH from possible users")
}

void CPhRigidBody::Destroy()
{
    delete this;
}

void CPhRigidBody::setCollisionGroup(unsigned char grp, int collision_group_mask)
{
    m_grp               = grp;
    m_collision_group_mask = collision_group_mask;
}
void CPhRigidBody::setShapeType(ShapeType t)
{
    m_type = t;
}
void CPhRigidBody::setMode(unsigned char   mode)
{
    m_mode = mode;
}
void CPhRigidBody::setShapeDimensions(float shape_size[3])
{
    m_shape_size[0] = shape_size[0];
    m_shape_size[1] = shape_size[1];
    m_shape_size[2] = shape_size[2];
}
void CPhRigidBody::setAttributes(float mass, float linear_dampening, float angular_dampening, float restitution, float friction)
{
    m_mass              = mass;
    m_linear_dampening  = linear_dampening;
    m_angular_dampening = angular_dampening;
    m_restitution       = restitution;
    m_friction          = friction;
}
