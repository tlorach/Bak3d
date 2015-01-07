#pragma once
#pragma warning(disable:4530)

#  pragma warning(disable:4786)
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <assert.h>

#ifndef NOGZLIB
#   include <zlib.h>
#endif
#include "bk3dEx.h"

#define EPF(n) printf n
#ifdef _DEBUG
#   include "bk3dDbgFuncs.inl"
#   define DPF(n) printf n
#else
#   define DPF(n)
#endif
#define PF(n) printf n
#define S__(x) #x
#define S_(x) S__(x)
#define S__LINE__ S_(__LINE__)
// use it as: #pragma message(__FILE__"("S__LINE__"): blah")

// This is where we can add older implementations, so that the library can still read them
// and help to convert older versions. In this case, those versions were older ones, before
// I published bk3d. I didn't put them. But later this is how we would do.
#define INCLUDE_OLD_BK3D_VERSIONS
#ifdef INCLUDE_OLD_BK3D_VERSIONS
#include "legacy/NVRawMesh_133.h"
#include "legacy/NVRawMesh_132.h"
//#include "NVRawMesh_131.h"
//#include "NVRawMesh_130.h"
//#include "NVRawMesh_128.h"
//#include "NVRawMesh_127.h"
//#include "NVRawMesh_126.h"
#endif

#include "Bk3dlib.h"
#include "PoolAllocator.h"
#include "MiscHelpers.h"
#include "nv_Math.h"
using namespace nv_math;

#define SETNAME(p,n) strncpy(p->name, n, 31);

//
/// Structures that redirect allocators to the pool
//
typedef Pool<bk3d::Attribute> TAttribute;
typedef Pool<bk3d::AttributePool> TAttributePool;
typedef Pool<bk3d::FileHeader> TFileHeader;
typedef Pool<bk3d::Mesh> TMesh;
typedef Pool<bk3d::MeshPool> TMeshPool;
typedef Pool<bk3d::PrimGroup> TPrimGroup;
typedef Pool<bk3d::PrimGroupPool> TPrimGroupPool;
typedef Pool<bk3d::RelocationTable> TRelocationTable;
typedef Pool<bk3d::Slot> TSlot;
typedef Pool<bk3d::SlotPool> TSlotPool;
typedef Pool<bk3d::Bone> TBone;
typedef Pool<bk3d::TransformSimple> TTransformSimple;
typedef Pool<bk3d::Transform> TTransform;
typedef Pool<bk3d::TransformDOF> TTransformDOF;
typedef Pool<bk3d::FloatPool> TFloatPool;
typedef Pool<bk3d::TransformPool> TTransformPool;
typedef Pool<bk3d::TransformPool2> TTransformPool2;
typedef Pool<bk3d::MatrixType> TMatrixType;
typedef Pool<bk3d::BoneDataType> TBoneDataType;
struct IntType { // for new operator to be overloaded... need a struct
    unsigned int    ui; };
typedef Pool<IntType> TUINT;
typedef Pool<bk3d::Vec4Type> TVec4Type;
typedef Pool<bk3d::MayaTransformData> TMayaTransformData;
typedef Pool<bk3d::TransformDOF> TTransformDOF;
typedef Pool<bk3d::IKHandleData> TIKHandleData;
typedef Pool<bk3d::EffectorTransformAndWeight> TEffectorTransformAndWeight;
typedef Pool<bk3d::TransformRefs> TTransformRefs;
typedef Pool<bk3d::MayaCurveVector> TMayaCurveVector;
typedef Pool<bk3d::MayaCurve> TMayaCurve;
typedef Pool<bk3d::MayaCurvePool> TMayaCurvePool;
typedef Pool<bk3d::QuatCurve> TQuatCurve;
typedef Pool<bk3d::QuatCurvePool> TQuatCurvePool;
typedef Pool<bk3d::FloatArrayPool> TFloatArrayPool;
typedef Pool<bk3d::FloatArray> TFloatArray;
typedef Pool<bk3d::MaterialData> TMaterialData;
typedef Pool<bk3d::Material> TMaterial;
typedef Pool<bk3d::MaterialPool> TMaterialPool;
typedef Pool<bk3d::IKHandle> TIKHandle;
typedef Pool<bk3d::IKHandlePool> TIKHandlePool;
typedef Pool<bk3d::RigidBody> TRigidBody;
typedef Pool<bk3d::RigidBodyPool> TRigidBodyPool;
typedef Pool<bk3d::Constraint> TConstraint;
typedef Pool<bk3d::ConstraintPool> TConstraintPool;
typedef Pool<char> TChar;
typedef Pool<unsigned short> TUShort;
typedef Pool<unsigned int> TUInt;

// Tested a way to automate the registration of pointers to further offset computation and relocation table
// sounds like this is not so useful
/*
template <class T>
struct PtrTrack
{
    T*    m_ptr;
    PtrTrack(T* ptr = NULL)
    {
        m_ptr = ptr;
    }
    T*    operator->()
    {
        return m_ptr;
    }
    PtrTrack& operator=(T* p) 
    {
        m_ptr = p; 
        return *this;
    }
    PtrTrack& operator=(PtrTrack const &t) 
    {
        m_ptr = t.m_ptr; 
        return *this;
    }
};

struct PAttribute
{
    TAttribute *p;

    PtrTrack<void> pAttributeBufferData;

    PAttribute(TAttribute *p_) :
        p(p_),
        pAttributeBufferData(p->pAttributeBufferData)
    {}
    PAttribute()    {}
    TAttribute*    operator->()
    {
        return p;
    }
    PAttribute& operator=(TAttribute* p_) 
    {
        p = p_; 
        pAttributeBufferData = p->pAttributeBufferData;
        return *this;
    }
};*/

/*------------------------------------------------------------------
    typedefs for maps
  ------------------------------------------------------------------*/
class CFileHeader;
class CPrimGroup;
class CMesh;
class CBone;
class CTransformSimple;
class CTransform;
class CTransformDOF;
class CBuffer;
class CMayaCurveVector;
class CQuatCurve;
class CMaterial;
class CIKHandle;
class CPhRigidBody;
class CPhConstraint;
typedef std::map<bk3dlib::PFileHeader, CFileHeader*> MapFileHeader; 
typedef std::map<bk3dlib::PMesh, CMesh*> MapMesh; 
typedef std::vector<CBone*> VecTransform; 
typedef std::map<bk3dlib::PBuffer, CBuffer*> MapBuffer;
typedef std::vector<CBuffer*> VecBuffer; 

// mapping used when reading a bk3d file, during creation
typedef std::map<bk3d::Material*, bk3dlib::PMaterial> Mapbk3dMat;
typedef std::map<bk3d::Bone*, bk3dlib::PBone> Mapbk3dTransf;

#ifdef INCLUDE_OLD_BK3D_VERSIONS
//typedef std::map<bk3d126::Material*, bk3dlib::PMaterial> Mapbk3dMat126;
//typedef std::map<bk3d126::Transform*, bk3dlib::PTransform> Mapbk3dTransf126;
//
//typedef std::map<bk3d127::Material*, bk3dlib::PMaterial> Mapbk3dMat127;
//typedef std::map<bk3d127::Transform*, bk3dlib::PTransform> Mapbk3dTransf127;
//
//typedef std::map<bk3d128::Material*, bk3dlib::PMaterial> Mapbk3dMat128;
//typedef std::map<bk3d128::Transform*, bk3dlib::PTransform> Mapbk3dTransf128;
//
//typedef std::map<bk3d130::Material*, bk3dlib::PMaterial> Mapbk3dMat130;
//typedef std::map<bk3d130::Transform*, bk3dlib::PTransform> Mapbk3dTransf130;
//
//typedef std::map<bk3d131::Material*, bk3dlib::PMaterial> Mapbk3dMat131;
//typedef std::map<bk3d131::Transform*, bk3dlib::PTransform> Mapbk3dTransf131;

typedef std::map<bk3d132::Material*, bk3dlib::PMaterial> Mapbk3dMat132;
typedef std::map<bk3d132::Transform*, bk3dlib::PBone> Mapbk3dTransf132;

typedef std::map<bk3d133::Material*, bk3dlib::PMaterial> Mapbk3dMat133;
typedef std::map<bk3d133::Transform*, bk3dlib::PBone> Mapbk3dTransf133;
#endif

typedef std::map<bk3dlib::PCurveVec, CMayaCurveVector*> MapCurveVec; 
typedef std::map<bk3dlib::PQuatCurve,CQuatCurve*> MapQuatCurve; 
typedef std::map<bk3dlib::PMaterial, CMaterial*> MapMaterial; 
typedef std::map<bk3dlib::PCurveVec, bk3d::MayaCurveVector*> MapMayaCVPtrs; // used to keep track of the bk3d allocation ptr while building 
typedef std::map<bk3dlib::PQuatCurve, bk3d::QuatCurve*> MapQuatCVPtrs; // used to keep track of the bk3d allocation ptr while building 
typedef std::map<bk3dlib::PBone, bk3d::Bone*> MapTransform2bk3d; // used to keep track of the bk3d allocation ptr while building 
typedef std::map<bk3dlib::PIKHandle, CIKHandle*> MapIKHandle;
typedef std::map<bk3dlib::PPhRigidBody, CPhRigidBody*> MapPhRigidBody;
typedef std::map<bk3dlib::PPhConstraint, CPhConstraint*> MapPhConstraint;

/*------------------------------------------------------------------
    :
  ------------------------------------------------------------------*/
struct lessCBuffer //: public binary_function<_Ty, _Ty, bool>
{
    bool operator()(const int & _Left, const int & _Right) const
    {
        return (_Left < _Right);
    }
};
/*------------------------------------------------------------------
    :
  ------------------------------------------------------------------*/
typedef std::multimap<int, CBuffer*, lessCBuffer> MMapBuffer; 

#define MAXATTRIBS 9
#define MAXSLOTS 16
/*------------------------------------------------------------------
    :
  ------------------------------------------------------------------*/
// Must match the order in the mel script !!
enum AttributeType 
{
  None = 0,
  Position,
  Color,
  Normal,
  Tangent,
  Binormal,
  VertexNormal,
  TexCoord0,
  TexCoord1,
  TexCoord2,
  TexCoord3,
  BonesOffsets,
  BonesWeights,
  VertexID,
  Blind0,
  Blind1,
  Blind2,
  Blind3
};


/*------------------------------------------------------------------
    :
  ------------------------------------------------------------------*/
struct VertexAttribute
{
    typedef std::vector< int > IntVector;
    typedef std::vector< float > FloatVector;
    std::string        m_Name;
    AttributeType    m_type;
    int                m_attributePos; // where in the list is this attribute
    IntVector        m_intVector;
    FloatVector        m_floatVector;
    int                m_numComponents;

    VertexAttribute& operator=( const VertexAttribute& rhs )
    {
        m_attributePos = rhs.m_attributePos;
        m_Name   = rhs.m_Name;
        m_intVector = rhs.m_intVector;
        m_floatVector = rhs.m_floatVector;
        m_numComponents = rhs.m_numComponents;
        m_type = rhs.m_type;
        return *this;
    }
    VertexAttribute(  ) : m_numComponents(0), m_attributePos(0), m_type(None) {}
    VertexAttribute( const char* pName ) : m_Name(pName), m_attributePos(0), m_type(None) {m_numComponents=1;}
    VertexAttribute( const char* pName, int nc ) : m_Name(pName), m_attributePos(None), m_type(None) {m_numComponents=nc;}
    VertexAttribute( const VertexAttribute& rhs )
    {
        *this = rhs;
    }
    bool operator==( const VertexAttribute& rhs )
    {
        return ( m_Name == rhs.m_Name );
    }
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
// Some internal/intermediate structures
// Index group, gathering index arrays for triangles, strips and fans
struct IdxGroup
{
    D3D11_PRIMITIVE_TOPOLOGY    primitiveTypeDXGI;
    D3DPRIMITIVETYPE            primitiveTypeDX9;
    GLTopology                  primitiveTypeGL;
    std::vector<unsigned int>   elements;
    bk3d::AABBox            aabb;
    bk3d::BSphere           bsphere;
    const char *                shaderName;
};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
struct MTransform
{
    bk3d::Bone              transform;  // the transform that we'll store in bk3d
    double                  bindpose[4][4]; // temporary bindpose data
    std::vector<MTransform> children;
    MTransform              *pParentTransform;
    //std::vector<MayaCurveVector2*> cvVectors;
};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
struct AttributeInfo
{
    AttributeInfo() : attribute(None), slot(0) {}
  AttributeType attribute;
  int           slot;
};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
struct BlindDataInfo
{
    BlindDataInfo() : formatDXGI(DXGI_FORMAT_UNKNOWN), formatDX9(D3DDECLTYPE_UNDEF) { name[0] = '\0'; }
  char          name[64];
  DXGI_FORMAT   formatDXGI;
  D3DDECLTYPE   formatDX9;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
struct ltstr
{
    bool operator()(const std::string s1, const std::string s2) const
    {
      return strcmp(s1.c_str(), s2.c_str()) < 0;
    }
};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
struct ltstr_inv
{
  bool operator()(const std::string s1, const std::string s2) const
  {
    return strcmp(s2.c_str(), s1.c_str()) < 0;
  }
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CSlot
{
private:
    CMesh*             m_parent;
    std::string        m_name;
    TSlot *            m_slot;
    int                m_sz;
    int                m_stride;
    int                m_numAttr;
    unsigned int    m_numVtx;
    bk3d::AttributePool *m_attributePool;
    std::pair<MMapBuffer::iterator, MMapBuffer::iterator> m_attrPair;
    std::vector<bool> m_validVtxTable; // temporary table to tell which vtx to take and which to remove, if asked
public:
    CSlot(CMesh* parent, std::pair<MMapBuffer::iterator, MMapBuffer::iterator> pair, bk3d::AttributePool *pAttr, LPCSTR slotName);
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Slot *p);
    virtual size_t getTotalSize(int &relocationSlots, bool doPruning=false);
    //2+1: pAttributes + pVtxBufferData + nextnode
    int        getNumRelocationSlots() {return 2+1; }
    TSlot    *buildBegin(bool doPruning=false);
    TSlot    *buildEnd();
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CAttribute
{
private:
    TAttribute *m_attribute; // the cooked struct
    CBuffer*    m_buffer;
public:
    CAttribute(CBuffer *buf);
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Attribute *p);
    //virtual size_t    getTotalSize(int &relocationSlots);
    // 1+1 : pAttributeBufferData + nextnode
    int        getNumRelocationSlots() {return 1+1; }
    TAttribute    *build(const char *prefix=NULL, int bsloop=0);
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CPrimGroup
{
private:
    std::string m_name;
    CMesh*        m_parent;
    bk3dlib::Topology m_topo;
    CMaterial    *m_pMaterial;
    int            m_offsetElement;
    int            m_numElements;

    size_t        m_sz;
    TPrimGroup*    primgroup; // the cooked struct
    IdxGroup    idxgroup;
    std::vector<CBuffer*>                m_idxBuffers;
    std::vector<bk3dlib::PBone>         m_transformRefs; // transforms can occasionally be local to the primitive group

    bk3d::AABBox    aabbox;
    bk3d::BSphere   bsphere;
public:
    CPrimGroup(CMesh* parent, LPCSTR name, CBuffer*    idxBuffer, bk3dlib::Topology topo, bk3dlib::PMaterial pMat, unsigned int offsetElement, unsigned int numElements);
    void           AddTransformReference(bk3dlib::PBone t);
    int            GetNumTransformReferences() { return (int)m_transformRefs.size(); }
    bk3dlib::PBone GetTransformReference(int i) { return i < (int)m_transformRefs.size() ? m_transformRefs[i] : NULL; }
    int            AddIndexBuffer(CBuffer* idxBuffer);
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::PrimGroup *p);
    virtual size_t getTotalSize(int &relocationSlots);
    // 1+4: pOwnerOfIB + pIndexBufferData + pMaterial + pTransforms + nextnode
    int            getNumRelocationSlots() {return 4+1; }
    TPrimGroup    *buildBegin(const MapTransform2bk3d &transform2bk3d);
    TPrimGroup    *buildEnd();

    friend CMesh;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CMesh : public bk3dlib::Mesh
{
private:
    TMesh *mesh; // the cooked struct
    TTransformPool*                    transformPool; // the transform pool in the header
    TMayaCurvePool*                    pMayaCurves; // the curve pool in the header
    bk3d::AABBox                    aabbox;
    bk3d::BSphere                    bsphere;

    void            *m_userData;
public:
    CMesh(Mapbk3dTransf &maptransf, Mapbk3dMat &mapmat, bk3d::Mesh *pMesh);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CMesh(Mapbk3dTransf126 &maptransf, Mapbk3dMat126 &mapmat, bk3d126::Mesh *pMesh);
    //CMesh(Mapbk3dTransf127 &maptransf, Mapbk3dMat127 &mapmat, bk3d127::Mesh *pMesh);
    //CMesh(Mapbk3dTransf128 &maptransf, Mapbk3dMat128 &mapmat, bk3d128::Mesh *pMesh);
    //CMesh(Mapbk3dTransf130 &maptransf, Mapbk3dMat130 &mapmat, bk3d130::Mesh *pMesh);
    //CMesh(Mapbk3dTransf131 &maptransf, Mapbk3dMat131 &mapmat, bk3d131::Mesh *pMesh);
    CMesh(Mapbk3dTransf132 &maptransf, Mapbk3dMat132 &mapmat, bk3d132::Mesh *pMesh);
    CMesh(Mapbk3dTransf133 &maptransf, Mapbk3dMat133 &mapmat, bk3d133::Mesh *pMesh);
#endif

    ~CMesh();
    CMesh(const char * name = NULL);
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Mesh *p);
    virtual size_t getTotalSize(int &relocationSlots);
    //      + pSlots 
    //      + pPrimGroups 
    //      + pAttributes 
    //      + pBSAttributes 
    //      + pBSSlots 
    //      + pBSWeights 
    //      + pTransforms 
    //      + pFloatArrays 
    //      + nextnode
    int        getNumRelocationSlots() {return 8+1; }
    TMesh    *build(const MapTransform2bk3d &transform2bk3d, const MapMayaCVPtrs &CVPtrs);

    bool Connect(bk3dlib::CurveVec *pCV, bk3dlib::MeshComponent comp);
    bool Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::MeshComponent comp);
    bool Connect(bk3dlib::QuatCurve *pCV, bk3dlib::MeshComponent comp);
    bool Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::MeshComponent comp);
    //
    // from bk3d
    //
private:
    std::string        m_name;
    MMapBuffer        m_vtxBuffers;
    MMapBuffer        m_vtxBuffersBS;
    std::vector< std::string >    m_SlotNames;
    std::vector< std::string >    m_BSSlotNames;
    std::vector<CBuffer*>    m_idxBuffers;
    std::vector<CPrimGroup*> m_primgroups;
    std::vector<bk3dlib::PBone> m_transformRefs;
    int                m_maxNumBones;

    //
    // connected curves
    //
    typedef std::pair<bk3dlib::CurveVec*, bk3dlib::MeshComponent> CurveVecConnection;
    std::set< CurveVecConnection > connectedCurveVectors;

    //CFileHeader*    m_parent;
public:
    virtual void            Destroy();
    virtual LPCSTR            GetName() { return m_name.c_str(); };
    virtual void            SetName(LPCSTR name) { m_name = std::string(name); }
    //virtual bk3dlib::PBuffer    CreateVtxBuffer(LPCSTR name, int numcomp, int slot, bk3dlib::DataType type, bool isBlendShape);
    //virtual bk3dlib::PBuffer    CreateIdxBuffer(LPCSTR name, bk3dlib::DataType type, int numComp);
    virtual int             CreatePrimGroup(LPCSTR name, bk3dlib::PBuffer idxBuffer, bk3dlib::Topology topo, bk3dlib::PMaterial pMat, unsigned int offsetElement, unsigned int numElements);
    virtual bool            DeletePrimGroupFromName(LPCSTR name);
    virtual bool            DeletePrimGroupFromIndex(int id);
    virtual int             GetNumPrimGroups();
    virtual bool            GetPrimGroupInfo(int i, bk3dlib::PrimGroup &pginfo);
    virtual bool            AttachVtxBuffer(bk3dlib::PBuffer pBuf, bool isBlendShape);
    virtual int             AttachIndexBuffer(bk3dlib::PBuffer idxBuffer, int primGroupID);

    virtual bool            DetachBuffer(bk3dlib::PBuffer pbuffer);

    virtual bk3dlib::PBuffer    GetVtxBuffer(int n, bool isBlendShape);
    virtual bk3dlib::PBuffer    GetIdxBuffer(int n);

    virtual int                GetNumVtxBuffers(bool isBlendShape);
    virtual int                GetNumIdxBuffers();
    virtual int                GetNumSlots();
    virtual void               SetSlotName(int s, LPCSTR name);
    virtual const char*        GetSlotName(int s);
    virtual void               SetBSSlotName(int s, LPCSTR name);
    virtual const char*        GetBSSlotName(int s);

    virtual bool            ComputeBoundingVolumes(bk3dlib::PBuffer source);

    virtual bool            AddTransformReference(bk3dlib::PBone t, int primgroup);
    virtual int                GetNumTransformReferences(bool bMeshOnly);
    virtual bk3dlib::PBone  GetTransformReference(int n);
    virtual void            ClearTransformReferences(int primgroup);

    // NOT IMPLEMENTED YET:
    virtual bool            ComputeNormalsAndTangents(bk3dlib::PBuffer bufferIdx, bk3dlib::PBuffer bufferVtxPos, bk3dlib::PBuffer bufferTexcoords,
                                bk3dlib::PBuffer bufferPerVtxNormals, bk3dlib::PBuffer bufferPerFaceNormals, 
                                bk3dlib::PBuffer bufferTangents, bk3dlib::PBuffer bufferBitangents);
    virtual void            SetUserData(void *p);
    virtual void *            GetUserData();
    virtual void            SetMaxNumBones(int n) { m_maxNumBones = n;}
    virtual int             GetMaxNumBones() { return m_maxNumBones;}

    friend class CFileHeader;
    friend class CBuffer;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CBone : public bk3dlib::Bone
{
protected:
    std::string                 m_name;

    // one of these 3 implementations will be created
    // keeping them here because simpler
    TBone*                      m_bone; // the cooked struct
    TTransformSimple*           m_transformSimple; // the cooked struct
    TTransform*                 m_transform; // the cooked struct

    std::vector<CBone*>         m_childrenTransf;
    CBone*                      m_parentTransf;
    CTransformDOF*              m_TransfDOF;
    //double                        m_bindpose[4][4]; // temporary bindpose data
    //std::vector<MayaCurveVector2*> m_cvVectors;
    //
    // Matrix Data
    //
    mat4f    m_matrix;         // resulting matrix
    mat4f    m_abs_matrix;     // resulting matrix
    mat4f    m_bindpose_matrix;

    vec3f      m_pos;
    quatf      m_Quat;         //Quaternion
    quatf      m_abs_Quat;             //Quaternion in absolute mode
    vec3f      m_posBoneTail;   // tail of the bone. valid if TRANSFCOMP_isBone set

    unsigned int  m_validComps;
    bool      bDirty;

    //
    // connected objects
    //
    typedef std::pair<bk3dlib::CurveVec*, bk3dlib::TransfComponent> CurveVecConnection;
    std::set< CurveVecConnection > connectedCurveVectors;

    void            *m_userData;

public:
    ~CBone();
    CBone(bk3d::Bone *p);
    CBone(const char * name=NULL, CBone *p=NULL);

    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Bone *p);

    virtual size_t  getTotalSize(int &relocationSlots);
    // Can be more if many children or any parent or any curve
    // relocations for: 
    //      nextnode; 
    //      + parentPool 
    //      + pBoneData 
    //      + pFloatArrays
    //      + pParent
    //      + pChildren
    //      + pDOF
    //      + pIKHandles
    //      + pMatrixAbs
    //      + pMatrixAbsInvBindPoseMatrix
    //      + pMatrixInvBindPoseMatrix
    int             getNumRelocationSlots() {return 11; }
    virtual bk3d::Bone*   build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);
    virtual bool    setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d);

    bool Connect(bk3dlib::CurveVec *pCV, bk3dlib::TransfComponent comp);
    bool Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::TransfComponent comp);
    bool Connect(bk3dlib::QuatCurve *pCV, bk3dlib::TransfComponent comp);
    bool Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::TransfComponent comp);
    //
    // bk3d
    //
    virtual void            Destroy();
    virtual LPCSTR          GetName() { return m_name.c_str(); };
    virtual void            SetName(LPCSTR name) { m_name = std::string(name); }
    virtual bk3dlib::PBone  GetChild(int n);
    virtual int             GetNumChildren() { return (int)m_childrenTransf.size(); };
    virtual bk3dlib::PBone  GetParent();
    virtual void            SetParent(bk3dlib::PBone p);

    //virtual bool connectCurve(bk3dlib::PCurveVec pCVec, bk3dlib::TransfComponent comp, int compOffset);
    //virtual bool disconnectCurve(bk3dlib::PCurveVec pCVec, bk3dlib::TransfComponent comp, int compOffset);

    virtual void GetPos(float &x, float &y, float &z);
    virtual void GetQuaternion(float &x, float &y, float &z, float &w);
    virtual void GetMatrix(float *m);
    virtual void GetMatrix_Abs(float *m);
    virtual void GetMatrix_Bindpose(float *m);
    virtual const float * GetTailPos();

    virtual void CopyFrom(bk3dlib::PBone from);
    virtual void SetPos(float x, float y, float z);
    virtual void SetQuaternion(float x, float y, float z, float w);
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z);
    virtual void SetTailPos(float x, float y, float z);             //bone's taile pos relative to this transform

    virtual void SetMatrix(float *m);
    virtual void SetMatrixBindpose(float *m);
    virtual void SetAbsMatrix(float *m);
    //TODO:
    virtual bool ComputeMatrix(bool bBindPose);
    virtual void recTransfUpdate(CBone *parent=NULL, bool bBindPose=true);

    virtual void SetUserData(void *p);
    virtual void *GetUserData();

    virtual bk3dlib::PTransformDOF  GetDOF();
    virtual bk3dlib::PTransformDOF  CreateDOF();
    virtual void                    DeleteDOF();

    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return NULL; }
    virtual bk3dlib::PPhConstraint  AsPhConstraint()    { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return NULL; }
    virtual bk3dlib::PBone          AsBone()            { return this; }
    virtual bk3dlib::PTransformSimple AsTransfSimple()    { return NULL; }
    virtual bk3dlib::PTransform     AsTransf()          { return NULL; }

    friend class CFileHeader;
    friend class CTransform;
    friend class CTransformSimple;
    friend class CPhRigidBody;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CTransformSimple : public CBone, public bk3dlib::TransformSimple
{
protected:
    vec3f      m_scale;
public:
    ~CTransformSimple();
    CTransformSimple(bk3d::TransformSimple *pTransform);
    CTransformSimple(const char * name=NULL, CTransformSimple *p=NULL);

    static void     ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::TransformSimple *p);
    virtual size_t  getTotalSize(int &relocationSlots);
    virtual bk3d::Bone*   build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);

    virtual void GetScale(float &x, float &y, float &z);
    virtual void SetScale(float x, float y, float z);

    virtual bool ComputeMatrix(bool bBindPose);

    virtual bk3dlib::PTransformSimple AsTransfSimple()    { return this; }

    virtual void CopyFrom(bk3dlib::PBone from);
    virtual void Destroy();
    //
    // complimentary methods to pass to the class that implemented them
    //
    virtual LPCSTR GetName()                            { return CBone::GetName(); }
    virtual void SetName(LPCSTR name)                   { CBone::SetName(name); }
    virtual bk3dlib::PBone GetChild(int n)              { return CBone::GetChild(n); }
    virtual bk3dlib::PBone GetParent()                  { return CBone::GetParent(); }
    virtual void SetParent(bk3dlib::PBone p)            { CBone::SetParent(p); }
    virtual void GetPos(float &x, float &y, float &z)   { CBone::GetPos(x, y, z); }
    virtual void GetQuaternion(float &x, float &y, float &z, float &w) { CBone::GetQuaternion(x, y, z, w); }
    virtual void GetMatrix(float *m)                    { CBone::GetMatrix(m); }
    virtual void GetMatrix_Abs(float *m)                { CBone::GetMatrix_Abs(m); }
    virtual void GetMatrix_Bindpose(float *m)           { CBone::GetMatrix_Bindpose(m); }
    virtual const float * GetTailPos()                  { return CBone::GetTailPos(); }

    virtual void SetPos(float x, float y, float z)      { CBone::SetPos(x, y, z); }
    virtual void SetQuaternion(float x, float y, float z, float w) { CBone::SetQuaternion(x, y, z, w); }
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) { CBone::SetQuaternionFromEulerXYZ(x, y, z); }
    virtual void SetTailPos(float x, float y, float z)  { CBone::SetTailPos(x, y, z); }

    virtual void SetMatrix(float *m)                    { CBone::SetMatrix(m); }
    virtual void SetMatrixBindpose(float *m)            { CBone::SetMatrixBindpose(m); }
    virtual void SetAbsMatrix(float *m)                 { CBone::SetAbsMatrix(m); }

    virtual void SetUserData(void *p)                   { CBone::SetUserData(p); }
    virtual void *GetUserData()                         { return CBone::GetUserData(); }

    virtual bk3dlib::PTransformDOF  GetDOF()            { return CBone::GetDOF(); }
    virtual bk3dlib::PTransformDOF  CreateDOF()         { return CBone::CreateDOF(); }
    virtual void                    DeleteDOF()         { CBone::DeleteDOF(); }

    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return NULL; }
    virtual bk3dlib::PPhConstraint  AsPhConstraint()    { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return NULL; }
    virtual bk3dlib::PBone          AsBone()            { return CBone::AsBone(); }
    virtual bk3dlib::PTransform     AsTransf()          { return NULL; }

    friend class CFileHeader;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CTransform : public CTransformSimple, public bk3dlib::Transform
{
protected:
    //
    // Maya things
    //
    vec3f      m_rotation;                    // Euler Rotation in degres
    char      m_rotationOrder[3];                  // 3 chars for "xyz" or any other
    vec3f      m_scalePivot;
    vec3f      m_scalePivotTranslate;
    vec3f      m_rotationPivot;
    vec3f      m_rotationPivotTranslate;
    quatf      m_rotationOrientation;         //Quaternion
    quatf      m_jointOrientation;             //Quaternion
public:
    ~CTransform();
    CTransform(bk3d::Transform *pTransform);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CTransform(bk3d126::Transform *pTransform);
    //CTransform(bk3d127::Transform *pTransform);
    //CTransform(bk3d128::Transform *pTransform);
    //CTransform(bk3d130::Transform *pTransform);
    //CTransform(bk3d131::Transform *pTransform);
    CTransform(bk3d132::Transform *pTransform);
    CTransform(bk3d133::Transform *pTransform);
#endif

    CTransform(const char * name=NULL, CTransform *p=NULL);
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Transform *p);
    virtual size_t  getTotalSize(int &relocationSlots);

    virtual bk3d::Bone*   build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);

    virtual void GetRotation(float &x, float &y, float &z);
    virtual void GetScalePivot(float &x, float &y, float &z);
    virtual void GetScalePivotTranslate(float &x, float &y, float &z);
    virtual void GetRotationPivot(float &x, float &y, float &z);
    virtual void GetRotationPivotTranslate(float &x, float &y, float &z);
    virtual void GetRotationOrientation(float &x, float &y, float &z, float &w);
    virtual void GetJointOrientation(float &x, float &y, float &z, float &w);

    virtual void SetRotation(float a, float b, float g);                    // Euler Rotation in degres
    virtual void SetRotationOrder(char x, char y, char z);                  // 3 chars for "xyz" or any other
    virtual void SetScalePivot(float x, float y, float z);
    virtual void SetScalePivotTranslate(float x, float y, float z);
    virtual void SetRotationPivot(float x, float y, float z);
    virtual void SetRotationPivotTranslate(float x, float y, float z);
    virtual void SetJointOrientation(float x, float y, float z, float w);             //Quaternion
    virtual void SetRotationOrientation(float x, float y, float z, float w);         //Quaternion
    //TODO:
    virtual bool ComputeMatrix(bool bBindPose);

    virtual void CopyFrom(bk3dlib::PBone from);
    virtual void    Destroy();
    //
    // complimentary methods to pass to the class that implemented them
    //
    virtual LPCSTR    GetName()                         { return CBone::GetName(); }
    virtual void    SetName(LPCSTR name)                { CBone::SetName(name); }
    virtual bk3dlib::PBone GetChild(int n)              { return CBone::GetChild(n); }
    virtual bk3dlib::PBone GetParent()                  { return CBone::GetParent(); }
    virtual void SetParent(bk3dlib::PBone p)            { CBone::SetParent(p); }
    virtual void GetPos(float &x, float &y, float &z)   { CBone::GetPos(x, y, z); }
    virtual void GetQuaternion(float &x, float &y, float &z, float &w) { CBone::GetQuaternion(x, y, z, w); }
    virtual void GetMatrix(float *m)                    { CBone::GetMatrix(m); }
    virtual void GetMatrix_Abs(float *m)                { CBone::GetMatrix_Abs(m); }
    virtual void GetMatrix_Bindpose(float *m)           { CBone::GetMatrix_Bindpose(m); }
    virtual const float * GetTailPos()                  { return CBone::GetTailPos(); }

    virtual void SetPos(float x, float y, float z)      { CBone::SetPos(x, y, z); }
    virtual void SetQuaternion(float x, float y, float z, float w) { CBone::SetQuaternion(x, y, z, w); }
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) { CBone::SetQuaternionFromEulerXYZ(x, y, z); }
    virtual void SetTailPos(float x, float y, float z)  { CBone::SetTailPos(x, y, z); }

    virtual void SetMatrix(float *m)                    { CBone::SetMatrix(m); }
    virtual void SetMatrixBindpose(float *m)            { CBone::SetMatrixBindpose(m); }
    virtual void SetAbsMatrix(float *m)                 { CBone::SetAbsMatrix(m); }

    virtual void SetUserData(void *p)                   { CBone::SetUserData(p); }
    virtual void *GetUserData()                         { return CBone::GetUserData(); }

    virtual void GetScale(float &x, float &y, float &z) { CTransformSimple::GetScale(x, y, z); }
    virtual void SetScale(float x, float y, float z)    { CTransformSimple::SetScale(x, y, z); }

    virtual bk3dlib::PTransformDOF  GetDOF()            { return CBone::GetDOF(); }
    virtual bk3dlib::PTransformDOF  CreateDOF()         { return CBone::CreateDOF(); }
    virtual void                    DeleteDOF()         { CBone::DeleteDOF(); }

    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return NULL; }
    virtual bk3dlib::PPhConstraint  AsPhConstraint()    { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return CBone::AsIKHandle(); }
    virtual bk3dlib::PBone          AsBone()            { return CBone::AsBone(); }
    virtual bk3dlib::PTransformSimple AsTransfSimple()  { return CTransformSimple::AsTransfSimple(); }
    virtual bk3dlib::PTransform     AsTransf()          { return this; }

    friend class CFileHeader;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CTransformDOF : public bk3dlib::TransformDOF
{
    bk3dlib::TransformDOFMode   m_mode;
    float                       m_DOFAlpha;
    float                       m_AxisLimitStart;
    float                       m_AxisLimitRange;
    quatf                        m_quat;

    bk3d::TransformDOF	*build(bk3d::Bone* pTr);
    CTransformDOF(CTransformDOF *p=NULL);
    CTransformDOF(bk3d::TransformDOF *pTransform);
public:

    virtual void SetDOFValues(bk3dlib::TransformDOFMode  mode, float *DOFAlpha, float *AxisLimitStart, float *AxisLimitRange);
    virtual bk3dlib::TransformDOFMode GetDOFValues(float *DOFAlpha, float *AxisLimitStart, float *AxisLimitRange);
    virtual void GetQuaternion(float &x, float &y, float &z, float &w);
    virtual void SetQuaternion(float x, float y, float z, float w);
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z);

    friend class CBone;
};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
//
// structure for temporary storage so we can write them into the bk3d pool...
//
//struct MayaCurve2 : public bk3d::MayaCurve
//{
//    std::vector<bk3d::MayaReadKey> m_keys;
//};
//struct MayaCurveVector2 : public bk3d::MayaCurveVector
//{
//    int idx; // idx in the list...
//    std::string component;
//    std::vector<MayaCurve2> m_cvs;
//};
/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CMayaCurve
{
private:
    std::string            m_name;
    CMayaCurveVector*    m_parent;

    TMayaCurve*            m_curve; // the cooked struct

    bk3dlib::CurveVec::EtInfinityType    m_preInfinity;    // how to evaluate pre-infinity            
    bk3dlib::CurveVec::EtInfinityType    m_postInfinity;    // how to evaluate post-infinity        
    bool                    m_inputIsTime;    // if true, the input do not need Plugs to increase
    bool                    m_outputIsAngular;
    bool                    m_isWeighted;    // whether or not this curve has weighted tangents 

    std::vector<bk3d::MayaReadKey> m_keys;

    void            *m_userData;
public:
    ~CMayaCurve();
    CMayaCurve(CMayaCurveVector* parent);
    CMayaCurve(CMayaCurveVector* parent, bk3d::MayaCurve *pMayaCurve);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CMayaCurve(CMayaCurveVector* parent, bk3d126::MayaCurve *pMayaCurve);
    //CMayaCurve(CMayaCurveVector* parent, bk3d127::MayaCurve *pMayaCurve);
    //CMayaCurve(CMayaCurveVector* parent, bk3d128::MayaCurve *pMayaCurve);
    //CMayaCurve(CMayaCurveVector* parent, bk3d130::MayaCurve *pMayaCurve);
    //CMayaCurve(CMayaCurveVector* parent, bk3d131::MayaCurve *pMayaCurve);
    CMayaCurve(CMayaCurveVector* parent, bk3d132::MayaCurve *pMayaCurve);
    CMayaCurve(CMayaCurveVector* parent, bk3d133::MayaCurve *pMayaCurve);
#endif
    //CMayaCurve(CMayaCurveVector* parent, MayaCurve2 *_curve2) :
    //    curve2(_curve2),
    //    curve(NULL) {}
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::MayaCurve *p);
    virtual size_t getTotalSize(int &relocationSlots);
    int        getNumRelocationSlots() {return 1; } // 1: nextnode
    TMayaCurve    *build();

    friend class CMayaCurveVector;
    friend class CFileHeader;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CMayaCurveVector : public bk3dlib::CurveVec
{
private:
    std::string        m_name;
    //CFileHeader*    m_parent;

    TMayaCurveVector*    m_curveVector; // the cooked struct
//    MayaCurveVector2*    m_curveVector2;
    std::vector<CMayaCurve*> m_mayacurves;

    //
    // connected objects
    //
    typedef std::pair<bk3dlib::Bone*, bk3dlib::TransfComponent> TransformConnection;
    std::set< TransformConnection > connectedTransforms;
    typedef std::pair<bk3dlib::IKHandle*, bk3dlib::IKHandleComponent> IKHandleConnection;
    std::set< IKHandleConnection > connectedIKHandles;
    typedef std::pair<bk3dlib::Mesh*, bk3dlib::MeshComponent> MeshConnection;
    std::set< MeshConnection > connectedMeshes;

    bk3dlib::CurveVec::EtInfinityType    m_preInfinity;    // how to evaluate pre-infinity            
    bk3dlib::CurveVec::EtInfinityType    m_postInfinity;    // how to evaluate post-infinity        
    bool                    m_inputIsTime;    // if true, the input do not need Plugs to increase
    bool                    m_outputIsAngular;
    bool                    m_isWeighted;    // whether or not this curve has weighted tangents 

    void            *m_userData;
public:
    ~CMayaCurveVector();
    CMayaCurveVector();
    CMayaCurveVector(bk3d::MayaCurveVector *pMayaCurveVector);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CMayaCurveVector(bk3d126::MayaCurveVector *pMayaCurveVector);
    //CMayaCurveVector(bk3d127::MayaCurveVector *pMayaCurveVector);
    //CMayaCurveVector(bk3d128::MayaCurveVector *pMayaCurveVector);
    //CMayaCurveVector(bk3d130::MayaCurveVector *pMayaCurveVector);
    //CMayaCurveVector(bk3d131::MayaCurveVector *pMayaCurveVector);
    CMayaCurveVector(bk3d132::MayaCurveVector *pMayaCurveVector);
    CMayaCurveVector(bk3d133::MayaCurveVector *pMayaCurveVector);
#endif
    //CMayaCurveVector(CFileHeader* parent, MayaCurveVector2 *_curveVector2) :
    //    curveVector2(_curveVector2),
    //    curveVector(NULL) {}

    virtual void    Destroy();
    virtual LPCSTR    GetName() { return m_name.c_str(); };
    virtual void    SetName(LPCSTR name) { m_name = std::string(name); }
    virtual bool Connect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp);
    virtual bool Disconnect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp);
    virtual bool Connect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp);
    virtual bool Disconnect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp);
    virtual bool Connect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp);
    virtual bool Disconnect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp);

    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::MayaCurveVector *p);
    virtual size_t getTotalSize(int &relocationSlots);
    // 3: pCurve[1] + pFloatArray + nextnode
    int        getNumRelocationSlots() {return 3; } // + N Curves ptrs
    TMayaCurveVector *build();

    virtual void SetUserData(void *p);
    virtual void *GetUserData();

        /// setup curve characteristics
        virtual void        SetProps(EtInfinityType preInfinity,
                                EtInfinityType postInfinity,
                                bool    inputIsTime=true,
                                bool    outputIsAngular=true,
                                bool    isWeighted=true);
        virtual void        GetProps(EtInfinityType *preInfinity,
                                EtInfinityType *postInfinity,
                                bool    *inputIsTime,
                                bool    *outputIsAngular,
                                bool    *isWeighted);
        virtual void        SetPropsByComp(int comp, EtInfinityType preInfinity,
                                EtInfinityType postInfinity,
                                bool    inputIsTime,
                                bool    outputIsAngular,
                                bool    isWeighted);
        virtual void        GetPropsByComp(int comp, EtInfinityType *preInfinity,
                                EtInfinityType *postInfinity,
                                bool    *inputIsTime,
                                bool    *outputIsAngular,
                                bool    *isWeighted);
        virtual int         GetNumKeys(int comp);
        virtual int         DeleteKey(int comp, int keynum);
        /// separate assignment of Keys for each component (Maya does this way)
        virtual int         AddKey(int comp, float time, float value, 
                                    EtTangentType inTType, EtTangentType outTType,
                                    float inAngle, float inWeight,
                                    float outAngle, float outWeight
                                    );
        /// Add key for all components at the same time : N values for N components. The other params are shared
        virtual int         AddKey(float time, float *value, 
                                    EtTangentType inTType, EtTangentType outTType,
                                    float inAngle, float inWeight,
                                    float outAngle, float outWeight
                                    );
        virtual void        GetKey(int comp, int key, float *time, float *value, 
                                    EtTangentType *inTType, EtTangentType *outTType,
                                    float *inAngle, float *inWeight,
                                    float *outAngle, float *outWeight
                                    );
        friend class CFileHeader;
        friend class bk3dlib::CurveVec;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CQuatCurve : public bk3dlib::QuatCurve
{
private:
    std::string        m_name;

    TQuatCurve*        m_curve; // the cooked struct
    std::vector<bk3d::QuatReadKey> m_keys;
    //
    // connected objects
    //
    typedef std::pair<bk3dlib::Bone*, bk3dlib::TransfComponent>    TransformConnection;
    typedef std::pair<bk3dlib::IKHandle*, bk3dlib::IKHandleComponent>   IKHandleConnection;
    typedef std::pair<bk3dlib::Mesh*, bk3dlib::MeshComponent>           MeshConnection;
    std::set< TransformConnection > connectedTransforms;
    std::set< IKHandleConnection >  connectedIKHandles;
    std::set< MeshConnection >      connectedMeshes;

    void            *m_userData;
public:
    ~CQuatCurve();
    CQuatCurve();
    CQuatCurve(bk3d::MayaCurveVector *pMayaCurveVector);
public:
    virtual void        Destroy();
    virtual LPCSTR      GetName() { return m_name.c_str(); };
    virtual void        SetName(LPCSTR name) { m_name = std::string(name); }
    virtual bool        Connect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp);
    virtual bool        Disconnect(bk3dlib::Bone *pTransf, bk3dlib::TransfComponent comp);
    virtual bool        Connect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp);
    virtual bool        Disconnect(bk3dlib::Mesh *pMesh, bk3dlib::MeshComponent comp);
    virtual bool        Connect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp);
    virtual bool        Disconnect(bk3dlib::IKHandle *pIKHandle, bk3dlib::IKHandleComponent comp);
    virtual void        SetUserData(void *p);
    virtual void        *GetUserData();
    virtual int         GetNumKeys();
    virtual int         DeleteKey(int keynum);
    virtual int         AddKey(float time, float* quatVals);
    virtual void        GetKey(int key, float *time, float *quatVals);

    static void         ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::MayaCurve *p);
    virtual size_t      getTotalSize(int &relocationSlots);
    // 2: pFloatArray + nextnode
    int                 getNumRelocationSlots() {return 2; }
    TQuatCurve*         build();

    friend class CFileHeader;
};

/*------------------------------------------------------------------
    Material
    Now we just work with basic attributes...
  ------------------------------------------------------------------*/
class CMaterial : public bk3dlib::Material
{
private:
    std::string        m_name;

    //MObject        shd;
    TMaterial*    pMat; // the cooked struct

    float    diffuse[3];
    float    specexp;
    float    ambient[3];
    float    reflectivity;
    float    transparency[3];
    float    translucency;
    float    specular[3];

    std::string    shaderName;
    std::string    techniqueName;

    struct Texture
    {
        std::string    name;
        std::string    filename;
    };

    Texture    diffuseTexture;
    Texture    specExpTexture;
    Texture    ambientTexture;
    Texture    reflectivityTexture;
    Texture    transparencyTexture;
    Texture    translucencyTexture;
    Texture    specularTexture;
    void            *m_userData;
public:
    ~CMaterial();
    CMaterial(const char * name);
    CMaterial(bk3d::Material *pMaterial);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CMaterial(bk3d126::Material *pMaterial);
    //CMaterial(bk3d127::Material *pMaterial);
    //CMaterial(bk3d128::Material *pMaterial);
    //CMaterial(bk3d130::Material *pMaterial);
    //CMaterial(bk3d131::Material *pMaterial);
    CMaterial(bk3d132::Material *pMaterial);
    CMaterial(bk3d133::Material *pMaterial);
#endif
    static void    ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Material *p);
    virtual size_t getTotalSize(int &relocationSlots);
    // parentPool+pMaterialData+shaderName+techniqueName 7*(Texture::name+Texture::filename)
    int            getNumRelocationSlots() {return 4 + (7*2); }

    virtual void    Destroy();
    virtual LPCSTR    GetName() { return m_name.c_str(); };
    virtual void    SetName(LPCSTR name) { m_name = std::string(name); }
    virtual void setDiffuse(float r, float g, float b);
    virtual void setSpecexp(float s);
    virtual void setAmbient(float r, float g, float b);
    virtual void setReflectivity(float s);
    virtual void setTransparency(float r, float g, float b);
    virtual void setTranslucency(float s);
    virtual void setSpecular(float r, float g, float b);
    virtual void setShaderName(LPCSTR shdName, LPCSTR techName);
    virtual void setDiffuseTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setSpecExpTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setAmbientTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setReflectivityTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setTransparencyTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setTranslucencyTexture(LPCSTR    name, LPCSTR    filename);
    virtual void setSpecularTexture(LPCSTR    name, LPCSTR    filename);

    virtual void getDiffuse(float &r, float &g, float &b);
    virtual void getSpecexp(float &s);
    virtual void getAmbient(float &r, float &g, float &b);
    virtual void getReflectivity(float &s);
    virtual void getTransparency(float &r, float &g, float &b);
    virtual void getTranslucency(float &s);
    virtual void getSpecular(float &r, float &g, float &b);
    virtual void getShaderName(char* shdName, int shdNameSz, char* techName, int techNameSz);
    virtual bool getDiffuseTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getSpecExpTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getAmbientTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getReflectivityTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getTransparencyTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getTranslucencyTexture(char*    name, int nameSz, char*    filename, int filenameSz);
    virtual bool getSpecularTexture(char*    name, int nameSz, char*    filename, int filenameSz);

    virtual void SetUserData(void *p);
    virtual void *GetUserData();

    TMaterial    *build(bk3d::MaterialPool* pMaterialPool, int ID);

    friend CPrimGroup;
    friend CFileHeader;
};

/*------------------------------------------------------------------
    FileHeader
  ------------------------------------------------------------------*/
class CFileHeader : public bk3dlib::FileHeader
{
private:
    TFileHeader*    m_fileHeader; // the cooked struct
    Bk3dPool*       m_bk3dPool;
    Bk3dPool*       m_bk3dPool2; // a second pool allocator to store heavy data (vertices, indices...)
    //
    // for bk3dlib::FileHeader part
    //
    std::string        m_name;
    MapMesh            m_meshes;
    VecTransform       m_transforms;
    MapCurveVec        m_mayacurves;
    MapQuatCurve       m_quatcurves;
    MapMaterial        m_materials;
    MapIKHandle        m_ikHandles;
    MapPhRigidBody     m_phRigidBodies;
    MapPhConstraint    m_phConstraints;
    void            *m_userData;

    MapMesh::iterator m_iMesh; // used to iterate through Meshes (GetFirstMesh() GetNextMesh())

    void	CPrimGroup_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::PrimGroup *p, bool noPtrConv);
    void	CSlot_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Slot *p, bool noPtrConv);
    void	CTransform_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Bone *p, bool noPtrConv);
    void	CMaterial_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Material *p, bool noPtrConv);
    void	CAttribute_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Attribute *p, bool noPtrConv);
    void	CMayaCurve_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::MayaCurve *p, bool noPtrConv);
    void	CMayaCurveVector_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::MayaCurveVector *p, bool noPtrConv);
    void	CQuatCurve_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::QuatCurve *p, bool noPtrConv);
    void	CMesh_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Mesh *p, bool noPtrConv);
    void    CIKHandle_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::IKHandle *p, bool noPtrConv);
    void    CPhRigidBody_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::RigidBody *p, bool noPtrConv);
    void    CPhConstraint_ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::Constraint *p, bool noPtrConv);
    bool    ptrToOffset(bool noPtrConv=false);

public:
    CFileHeader(bk3d::FileHeader *pHeader);
#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //CFileHeader(bk3d126::FileHeader *pHeader);
    //CFileHeader(bk3d127::FileHeader *pHeader);
    //CFileHeader(bk3d128::FileHeader *pHeader);
    //CFileHeader(bk3d130::FileHeader *pHeader);
    //CFileHeader(bk3d131::FileHeader *pHeader);
    CFileHeader(bk3d132::FileHeader *pHeader);
    CFileHeader(bk3d133::FileHeader *pHeader);
#endif
    CFileHeader(LPCSTR name = NULL);
    ~CFileHeader();
    //
    // Internal things
    //
    void    OffsetToPtr();
    virtual size_t getTotalSize(int &relocationSlots);
    // 9+1: 
    //pMeshes
    //pTransforms
    //pMayaCurves
    //pMaterials
    //pIKHandles
    //pRelocationTable
    //pQuatCurves
    //pRigidBodies
    //pConstraints
    //nextnode
    int        getNumRelocationSlots() {return 9+1; }
    void    *build(LPCSTR file, std::string &headerName);
    //
    // from bk3dlib::FileHeader
    //
    virtual bool                    ReadBk3dFile(bk3d::FileHeader *pHeader);
    virtual bool                    LoadFromBk3dFile(LPCSTR file);

#ifdef INCLUDE_OLD_BK3D_VERSIONS
    //bool                            ReadBk3dFile_126(bk3d126::FileHeader *pHeader);
    //bool                            LoadFromBk3dFile_126(LPCSTR file);

    //bool                            ReadBk3dFile_127(bk3d127::FileHeader *pHeader);
    //bool                            LoadFromBk3dFile_127(LPCSTR file);

    //bool                            ReadBk3dFile_128(bk3d128::FileHeader *pHeader);
    //bool                            LoadFromBk3dFile_128(LPCSTR file);

    //bool                            ReadBk3dFile_130(bk3d130::FileHeader *pHeader);
    //bool                            LoadFromBk3dFile_130(LPCSTR file);

    //bool                            ReadBk3dFile_131(bk3d131::FileHeader *pHeader);
    //bool                            LoadFromBk3dFile_131(LPCSTR file);

    bool                            ReadBk3dFile_132(bk3d132::FileHeader *pHeader);
    bool                            LoadFromBk3dFile_132(LPCSTR file);

    bool                            ReadBk3dFile_133(bk3d133::FileHeader *pHeader);
    bool                            LoadFromBk3dFile_133(LPCSTR file);
#endif

    virtual void                    Destroy(bool bKeepBakedData);
    virtual LPCSTR                    GetName() { return m_name.c_str(); };
    virtual void                    SetName(LPCSTR name) { m_name = std::string(name); }
    //virtual bk3dlib::PMesh            CreateMesh(LPCSTR name);
    //virtual bk3dlib::PBone        CreateTransform(LPCSTR name);
    //virtual bk3dlib::PTransformDOF    CreateTransformDOF(LPCSTR name);
    //virtual bk3dlib::PCurveVec        CreateCurveVec(LPCSTR name, int ncomps); // NOT IMPLEMENTED YET
    //virtual bk3dlib::PMaterial        CreateMaterial(LPCSTR name);
    //virtual bk3dlib::PIKHandle      CreateIKHandle(LPCSTR name);
    virtual bool                    AttachMesh(bk3dlib::PMesh pMesh);
    virtual bool                    DetachMesh(bk3dlib::PMesh pMesh);
    virtual bool                    AttachTransform(bk3dlib::PBone pTr);
    virtual bool                    DetachTransform(bk3dlib::PBone pTr);
    virtual bool                    AttachCurveVec(bk3dlib::PCurveVec pCv);
    virtual bool                    DetachCurveVec(bk3dlib::PCurveVec pCv);
    virtual bool                    AttachQuatCurve(bk3dlib::PQuatCurve pCv);
    virtual bool                    DetachQuatCurve(bk3dlib::PQuatCurve pCv);
    virtual bool                    AttachMaterial(bk3dlib::PMaterial pMat);
    virtual bool                    DetachMaterial(bk3dlib::PMaterial pMat);
    virtual bool                    AttachIKHandle(bk3dlib::PIKHandle p);
    virtual bool                    DetachIKHandle(bk3dlib::PIKHandle pH);
    virtual bool                    AttachPhRigidBody(bk3dlib::PPhRigidBody p);
    virtual bool                    DetachPhRigidBody(bk3dlib::PPhRigidBody p);
    virtual bool                    AttachPhConstraint(bk3dlib::PPhConstraint p);
    virtual bool                    DetachPhConstraint(bk3dlib::PPhConstraint p);
    virtual void                    UpdateTransformations(bool bBindPose);
    virtual void*                   Cook(LPCSTR file, void ** pBufferMemory, unsigned int* bufferMemorySz, bool detachCookedData=false); // cook the data as a big area of memory
    virtual bool                    Save(LPCSTR file);
    virtual bool                    PtrToOffset();
    virtual bk3dlib::PMesh          GetFirstMesh();
    virtual bk3dlib::PMesh          GetNextMesh();
    virtual bk3dlib::PMesh          FindMesh(int n);
    virtual bk3dlib::PMesh          FindMesh(LPCSTR name);
    virtual int                     GetNumMeshes();
    virtual int                     GetNumTransforms();
    virtual int                     GetNumCurves();
    virtual int                     GetNumIKHandles();
    virtual int                     GetNumPhRigidBodies();
    virtual int                     GetNumPhConstraints();
    virtual bk3dlib::PCurveVec      GetCurveVec(int n);
    virtual bk3dlib::PCurveVec      GetCurveVec(LPCSTR name);
    virtual int                     GetNumQuatCurve();
    virtual bk3dlib::PQuatCurve     GetQuatCurve(int n);
    virtual bk3dlib::PQuatCurve     GetQuatCurve(LPCSTR name);
    virtual int                     GetNumMaterials() { return (int)m_materials.size(); };
    virtual bk3dlib::PMaterial      GetMaterial(LPCSTR name);
    virtual bk3dlib::PMaterial      GetMaterial(int n);
    virtual bk3dlib::PBone          GetTransform(LPCSTR name, int *n);
    virtual bk3dlib::PBone          GetTransform(int n);
    virtual bk3dlib::PIKHandle      GetIKHandle(int n);
    virtual bk3dlib::PIKHandle      GetIKHandle(LPCSTR name);
    virtual bk3dlib::PPhRigidBody   GetPhRigidBody(int n);
    virtual bk3dlib::PPhRigidBody   GetPhRigidBody(LPCSTR name);
    virtual bk3dlib::PPhConstraint  GetPhConstraint(int n);
    virtual bk3dlib::PPhConstraint  GetPhConstraint(LPCSTR name);
    virtual void                    SetUserData(void *p);
    virtual void *                  GetUserData();
};


/*------------------------------------------------------------------
    to order groups of indices
  ------------------------------------------------------------------*/

struct IdxSet {
    std::vector<unsigned int> index;

    void clearIndex()
    {
        index.clear();
    }
    void addIndex(unsigned int i)
    {
        index.push_back(i);
    }
    unsigned int getIndex(unsigned int idx)
    {
        if(idx >= index.size())
            return 0;
        return index[idx];
    }
    bool setIndex(int idx, unsigned int value)
    {
        if(idx >= (int)index.size())
            return false;
        index[idx] = value;
        return true;
    }

    bool operator< ( const IdxSet &rhs) const {
        assert(index.size() == rhs.index.size());
        for(unsigned int i=0; i<index.size(); i++)
        {
            if (index[i] < rhs.index[i])
                return true;
            if (index[i] > rhs.index[i])
                return false;
        }
        return false;
    }
};


/*------------------------------------------------------------------
    Buffers
  ------------------------------------------------------------------*/
enum BufferUsage
{
    BufferForVtx,
    BufferForBS,
    BufferForIdx
};
class CBuffer : public bk3dlib::Buffer
{
private:
    std::string        m_name;
    int                m_numcomp;
    int                m_bufNum;
    int                m_slot;
    unsigned int    m_curItem;
    unsigned int    m_primitiveRestart;
    BufferUsage        m_usage;
    //int                m_primgroup;
    int                m_divisor;
    bk3dlib::DataType    m_type;
    CPrimGroup*        m_pOwner; // the owner of this buffer. To help avoid redundant allocation of the buffers in the final bk3d
    IdxSet            m_idxset;
    TPrimGroup*        m_bk3dptr; // temp ptr storage when building the bk3d file
    //
    // One of these arrays will be used depending on what is needed
    //
    std::vector<float> m_FVals;
    std::vector<unsigned int> m_UIVals;
    //
    // Used to compute a single idx buffer system
    //
    struct BufferGroup {
    CBuffer * idxBuffers;
    CBuffer * vtxBuffersSrc;
    CBuffer * vtxBuffersDst;
    };
    std::vector<BufferGroup> m_SIBBuffers;
    void            *m_userData;
public:
    CBuffer();
    ~CBuffer();
    float            GetValueAsFloat(int i, int c);
    unsigned int    GetValueAsUInt(int i, int c);
    unsigned int    GetItemSizeBytes();
    bool            isFloatFormat();
    //DXGI_FORMAT        FmtDX11(); ??
    DXGI_FORMAT        FmtDXGI();
    D3DDECLTYPE        FmtDX9();
    GLenum            FmtOpenGL();
    bool            ComputeBVolume(bk3d::AABBox &aabbox, bk3d::BSphere &bsphere, CBuffer * idxBuffer = NULL, int offset=0, int numelts=-1);
    // an Idx buffer can be used many times by many primitive groups. This will help to tell that only one instance is required. No need to duplicate this buffer
    CPrimGroup*        GetOwner();
    void            SetOwner(CPrimGroup *pg);
    TPrimGroup*        GetBk3dPointer() { return m_bk3dptr; }
    void            SetBk3dPointer(TPrimGroup*    p) { m_bk3dptr = p; }
    //
    // from bk3dlib::
    //
    virtual void    Destroy();
    virtual LPCSTR    GetName() { return m_name.c_str(); };
    virtual void    SetName(LPCSTR name) { m_name = std::string(name); }
    virtual void    SetDataType(bk3dlib::DataType type) { m_type = type; }
    virtual bk3dlib::DataType GetDataType() { return m_type; }
    virtual void    SetSlot(int slot) { m_slot = slot; }
    virtual int        GetSlot() { return m_slot; }
    virtual void    SetNumComps(int n) {m_numcomp = n; };
    virtual int        GetNumComps() { return m_numcomp; };
    virtual void    ClearData();
    virtual void    AddData(const float * p, unsigned int n);
    virtual void    AddData(const long * p, unsigned int n);
    virtual void    AddData(const unsigned long * p, unsigned int n);
    virtual void    AddData(const unsigned int * p, unsigned int n);
    virtual void    AddData(const unsigned short * p, unsigned int n);
    virtual void    AddData(const unsigned char * p, unsigned int n);
    virtual void    AddData(float f);
    virtual void    AddData(unsigned int i);
    virtual void    AddData(int i);
    virtual void    GotoItem(unsigned int item);
    virtual void    SetDivisor(int divisor); // for dividing idx values. Needed for DS...

    virtual int      GetData(float * pDst, unsigned int offsitem, unsigned int numitems);
    virtual int      GetData(unsigned int * pDst, unsigned int offsitem, unsigned int numitems);

    // "replaces" the existing data, rather than appending. Could be used to "edit" existing values
    virtual bool     SetData(const float * p, unsigned int offsitem, unsigned int numitems);
    virtual bool     SetData(const long * p, unsigned int offsitem, unsigned int numitems);
    virtual bool     SetData(const unsigned long * p, unsigned int offsitem, unsigned int numitems);
    virtual bool     SetData(const unsigned int * p, unsigned int offsitem, unsigned int numitems);
    virtual bool     SetData(const unsigned short * p, unsigned int offsitem, unsigned int numitems);
    virtual bool     SetData(const unsigned char * p, unsigned int offsitem, unsigned int numitems);

    //int        GetNumVectors();
    virtual int      GetNumItems();
    virtual bool     Reserve(unsigned int numitems);
    //Single Index Buffer
    virtual void SIB_ClearBuffers();
    virtual void SIB_AddBuffers(bk3dlib::PBuffer bufferIdx, bk3dlib::PBuffer bufferAttrSrc, bk3dlib::PBuffer bufferAttrDst = NULL);
    virtual bool SIB_Compile();

    virtual void        setPrimitiveRestartIndex(unsigned int i) { m_primitiveRestart = i; };
    virtual unsigned int getPrimitiveRestartIndex() { return m_primitiveRestart; };

    virtual void SetUserData(void *p);
    virtual void *GetUserData();

    friend CMesh;
    friend CPrimGroup;
    friend CSlot;
    friend bk3dlib::Buffer;
};

/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
class CIKHandle : public CBone, public bk3dlib::IKHandle
{
    //std::string     m_name;
    Mode            m_mode;
    int             m_priority;
    int             m_maxIter;
    float           m_weight;
    std::vector<float> m_weights; // weights used for transform effectors
    std::vector<CBone *> m_effectorTransforms; // other way to store transform effectors
    CBone *    m_effectorTransformStart; // the ending part of the IK chain : the one connected to the effector
    CBone *    m_effectorTransformEnd; // the side closest to the Root of the skeleton
    //CBone *    m_handleTransforms;// Removed: now IKHandle is the transform
    //vec3f            m_HandlePos;// Removed: now IKHandle is the transform
    //vec3f            m_EffectorPos;
    TIKHandle *     m_bk3dHandle;

    //
    // connected curves
    //
    typedef std::pair<bk3dlib::CurveVec*, bk3dlib::IKHandleComponent> CurveVecConnection;
    std::set< CurveVecConnection > connectedCurveVectors;

public:
    CIKHandle(LPCSTR name=NULL);
    CIKHandle(Mapbk3dTransf &maptransf, bk3d::IKHandle *pikh);
    ~CIKHandle();

    virtual void setMode(Mode m)    { m_mode = m; }
    virtual Mode getMode()          { return m_mode; }
    virtual void Destroy();
    // in CBone now
    //virtual LPCSTR GetName()        { return m_name.c_str(); };
    //virtual void SetName(LPCSTR name) { m_name = std::string(name); }

    //virtual bool connectCurve(PCurveVec pCVec, IKHandleComponent comp, int compOffset=0) = 0;
    //virtual bool disconnectCurve(PCurveVec pCVec, IKHandleComponent comp=IK_DEFCOMP, int compOffset=0) = 0;
    bool Connect(bk3dlib::CurveVec *pCV, bk3dlib::IKHandleComponent comp);
    bool Disconnect(bk3dlib::CurveVec *pCV, bk3dlib::IKHandleComponent comp);
    bool Connect(bk3dlib::QuatCurve *pCV, bk3dlib::IKHandleComponent comp);
    bool Disconnect(bk3dlib::QuatCurve *pCV, bk3dlib::IKHandleComponent comp);

    virtual void setWeight(float w) { m_weight = w; }
    virtual void setMaxIter(int i) { m_maxIter = i; }
    virtual void setPriority(int pri) { m_priority = pri; }
    virtual void setHandlePos(float x, float y, float z);
    //virtual void setEffectorPos(float x, float y, float z);
    virtual void setEffectorTransformStart(bk3dlib::PBone pT);
    virtual void setEffectorTransformEnd(bk3dlib::PBone pT);
    virtual void addEffectorTransform(bk3dlib::PBone pT, float weight);
    virtual void clearEffectorTransforms();

    static void ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Mesh *p);
    virtual size_t getTotalSize(int &relocationSlots);
    int            getNumRelocationSlots() {return CBone::getNumRelocationSlots()+4; } // pFloatArrays; pEffectorTransforms; pEffectorWeights; pIKHandleData; Bone's ptrs;
    virtual bk3d::Bone *build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);
    virtual bool    setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d);

    //
    // complimentary methods to pass to the class that implemented them
    //
    virtual LPCSTR GetName()                            { return CBone::GetName(); }
    virtual void SetName(LPCSTR name)                   { CBone::SetName(name); }
    virtual bk3dlib::PBone GetChild(int n)              { return CBone::GetChild(n); }
    virtual bk3dlib::PBone GetParent()                  { return CBone::GetParent(); }
    virtual void SetParent(bk3dlib::PBone p)            { CBone::SetParent(p); }
    virtual void GetPos(float &x, float &y, float &z)   { CBone::GetPos(x, y, z); }
    virtual void GetQuaternion(float &x, float &y, float &z, float &w) { CBone::GetQuaternion(x, y, z, w); }
    virtual void GetMatrix(float *m)                    { CBone::GetMatrix(m); }
    virtual void GetMatrix_Abs(float *m)                { CBone::GetMatrix_Abs(m); }
    virtual void GetMatrix_Bindpose(float *m)           { CBone::GetMatrix_Bindpose(m); }
    virtual const float * GetTailPos()                  { return CBone::GetTailPos(); }

    virtual void CopyFrom(bk3dlib::PBone from)          { CBone::CopyFrom(from); } // TODO: copy the DOF data, too
    virtual void SetPos(float x, float y, float z)      { CBone::SetPos(x, y, z); }
    virtual void SetQuaternion(float x, float y, float z, float w) { CBone::SetQuaternion(x, y, z, w); }
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) { CBone::SetQuaternionFromEulerXYZ(x, y, z); }
    virtual void SetTailPos(float x, float y, float z)  { CBone::SetTailPos(x, y, z); }

    virtual void SetMatrix(float *m)                    { CBone::SetMatrix(m); }
    virtual void SetMatrixBindpose(float *m)            { CBone::SetMatrixBindpose(m); }
    virtual void SetAbsMatrix(float *m)                 { CBone::SetAbsMatrix(m); }

    virtual bool ComputeMatrix(bool bBindPose)          { return CBone::ComputeMatrix(bBindPose); }

    virtual void SetUserData(void *p)                   { CBone::SetUserData(p); }
    virtual void *GetUserData()                         { return CBone::GetUserData(); }

    virtual bk3dlib::PTransformDOF  GetDOF()            { return CBone::GetDOF(); }
    virtual bk3dlib::PTransformDOF  CreateDOF()         { return CBone::CreateDOF(); }
    virtual void                    DeleteDOF()         { CBone::DeleteDOF(); }

    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return NULL; }
    virtual bk3dlib::PPhConstraint  AsPhConstraint()    { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return this; }
    virtual bk3dlib::PBone          AsBone()            { return CBone::AsBone(); }
    virtual bk3dlib::PTransform     AsTransf()          { return NULL; }
    virtual bk3dlib::PTransformSimple AsTransfSimple()  { return NULL; }

    friend class CFileHeader;
};

class CPhRigidBody : public CBone, public bk3dlib::PhRigidBody
{
private:
    ShapeType       m_type;
    unsigned char   m_grp;
    int             m_collision_group_mask;
    unsigned char   m_mode;
    float           m_shape_size[3];
    float           m_mass;
    float           m_linear_dampening;
    float           m_angular_dampening;
    float           m_restitution;
    float           m_friction;
    TRigidBody *    m_bk3dRigidBody;
public:
    CPhRigidBody(LPCSTR name=NULL);
    CPhRigidBody(Mapbk3dTransf &maptransf, bk3d::RigidBody *p);
    ~CPhRigidBody();
    virtual void        Destroy();

    virtual void setCollisionGroup(unsigned char grp, int collision_group_mask);
    virtual void setShapeType(ShapeType t);
    virtual void setMode(unsigned char   mode);
    virtual void setShapeDimensions(float shape_size[3]);
    virtual void setAttributes(float mass, float linear_dampening, float angular_dampening, float restitution, float friction);

    static void ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Mesh *p);
    virtual size_t getTotalSize(int &relocationSlots);
    int            getNumRelocationSlots() {return CBone::getNumRelocationSlots(); }
    virtual bk3d::Bone*   build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);
    virtual bool    setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d);
    //
    // complimentary methods to pass to the class that implemented them
    //
    virtual LPCSTR GetName()                            { return CBone::GetName(); }
    virtual void SetName(LPCSTR name)                   { CBone::SetName(name); }
    virtual bk3dlib::PBone GetChild(int n)              { return CBone::GetChild(n); }
    virtual bk3dlib::PBone GetParent()                  { return CBone::GetParent(); }
    virtual void SetParent(bk3dlib::PBone p)            { CBone::SetParent(p); }
    virtual void GetPos(float &x, float &y, float &z)   { CBone::GetPos(x, y, z); }
    virtual void GetQuaternion(float &x, float &y, float &z, float &w) { CBone::GetQuaternion(x, y, z, w); }
    virtual void GetMatrix(float *m)                    { CBone::GetMatrix(m); }
    virtual void GetMatrix_Abs(float *m)                { CBone::GetMatrix_Abs(m); }
    virtual void GetMatrix_Bindpose(float *m)           { CBone::GetMatrix_Bindpose(m); }
    virtual const float * GetTailPos()                  { return CBone::GetTailPos(); }

    virtual void CopyFrom(bk3dlib::PBone from)          { CBone::CopyFrom(from); } // TODO: copy the DOF data, too
    virtual void SetPos(float x, float y, float z)      { CBone::SetPos(x, y, z); }
    virtual void SetQuaternion(float x, float y, float z, float w) { CBone::SetQuaternion(x, y, z, w); }
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) { CBone::SetQuaternionFromEulerXYZ(x, y, z); }
    virtual void SetTailPos(float x, float y, float z)  { CBone::SetTailPos(x, y, z); }

    virtual void SetMatrix(float *m)                    { CBone::SetMatrix(m); }
    virtual void SetMatrixBindpose(float *m)            { CBone::SetMatrixBindpose(m); }
    virtual void SetAbsMatrix(float *m)                 { CBone::SetAbsMatrix(m); }

    virtual bool ComputeMatrix(bool bBindPose)          { return CBone::ComputeMatrix(bBindPose); }

    virtual void SetUserData(void *p)                   { CBone::SetUserData(p); }
    virtual void *GetUserData()                         { return CBone::GetUserData(); }

    virtual bk3dlib::PTransformDOF  GetDOF()            { return CBone::GetDOF(); }
    virtual bk3dlib::PTransformDOF  CreateDOF()         { return CBone::CreateDOF(); }
    virtual void                    DeleteDOF()         { CBone::DeleteDOF(); }

    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return this; }
    virtual bk3dlib::PPhConstraint  AsPhConstraint()    { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return NULL; }
    virtual bk3dlib::PBone          AsBone()            { return CBone::AsBone(); }
    virtual bk3dlib::PTransform     AsTransf()          { return NULL; }
    virtual bk3dlib::PTransformSimple AsTransfSimple()  { return NULL; }

    friend class CFileHeader;
    friend class CPhConstraint;
};

class CPhConstraint : public CBone, public bk3dlib::PhConstraint
{
private:
    CPhRigidBody*  m_pRigidBody1;
    CPhRigidBody*  m_pRigidBody2;
    float   m_translation_limit_min[3];
    float   m_translation_limit_max[3];
    float   m_rotation_limit_min[3];
    float   m_rotation_limit_max[3];
    float   m_spring_constant_translation[3];
    float   m_spring_constant_rotation[3];
    TConstraint * m_bk3dConstraint;
public:
    CPhConstraint(LPCSTR name=NULL);
    CPhConstraint(Mapbk3dTransf &maptransf, bk3d::Constraint *p);
    ~CPhConstraint();

    virtual void        Destroy();

    virtual void linkRigidBodies(bk3dlib::PPhRigidBody pRigidBody1, bk3dlib::PPhRigidBody pRigidBody2);
    virtual void setTranslationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz);
    virtual void setRotationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz);
    virtual void setSpringConstantTranslation(float x, float y, float z);
    virtual void setSpringConstantRotation(float x, float y, float z);

    static void ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Mesh *p);
    virtual size_t getTotalSize(int &relocationSlots);
    int            getNumRelocationSlots() {return CBone::getNumRelocationSlots()+2; }
    virtual bk3d::Bone*   build(TTransformPool *transformPool, const MapMayaCVPtrs &CVPtrs, MapTransform2bk3d &transform2bk3d, bk3d::Bone * pTr, int &childrenOffset, int &effectorOffset);
    virtual bool    setupDOFAndChildrenLinks(TTransformPool *transformPool, MapTransform2bk3d &transform2bk3d);
    //
    // complimentary methods to pass to the class that implemented them
    //
    virtual LPCSTR GetName()                            { return CBone::GetName(); }
    virtual void SetName(LPCSTR name)                   { CBone::SetName(name); }
    virtual bk3dlib::PBone GetChild(int n)              { return CBone::GetChild(n); }
    virtual bk3dlib::PBone GetParent()                  { return CBone::GetParent(); }
    virtual void SetParent(bk3dlib::PBone p)            { CBone::SetParent(p); }
    virtual void GetPos(float &x, float &y, float &z)   { CBone::GetPos(x, y, z); }
    virtual void GetQuaternion(float &x, float &y, float &z, float &w) { CBone::GetQuaternion(x, y, z, w); }
    virtual void GetMatrix(float *m)                    { CBone::GetMatrix(m); }
    virtual void GetMatrix_Abs(float *m)                { CBone::GetMatrix_Abs(m); }
    virtual void GetMatrix_Bindpose(float *m)           { CBone::GetMatrix_Bindpose(m); }
    virtual const float * GetTailPos()                  { return CBone::GetTailPos(); }

    virtual void CopyFrom(bk3dlib::PBone from)          { CBone::CopyFrom(from); } // TODO: copy the DOF data, too
    virtual void SetPos(float x, float y, float z)      { CBone::SetPos(x, y, z); }
    virtual void SetQuaternion(float x, float y, float z, float w) { CBone::SetQuaternion(x, y, z, w); }
    virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) { CBone::SetQuaternionFromEulerXYZ(x, y, z); }
    virtual void SetTailPos(float x, float y, float z)  { CBone::SetTailPos(x, y, z); }

    virtual void SetMatrix(float *m)                    { CBone::SetMatrix(m); }
    virtual void SetMatrixBindpose(float *m)            { CBone::SetMatrixBindpose(m); }
    virtual void SetAbsMatrix(float *m)                 { CBone::SetAbsMatrix(m); }

    virtual bool ComputeMatrix(bool bBindPose)          { return CBone::ComputeMatrix(bBindPose); }

    virtual void SetUserData(void *p)                   { CBone::SetUserData(p); }
    virtual void *GetUserData()                         { return CBone::GetUserData(); }

    virtual bk3dlib::PTransformDOF  GetDOF()            { return CBone::GetDOF(); }
    virtual bk3dlib::PTransformDOF  CreateDOF()         { return CBone::CreateDOF(); }
    virtual void                    DeleteDOF()         { CBone::DeleteDOF(); }

    virtual bk3dlib::PPhConstraint   AsPhConstraint()   { return this; }
    virtual bk3dlib::PPhRigidBody   AsPhRigidBody()     { return NULL; }
    virtual bk3dlib::PIKHandle      AsIKHandle()        { return NULL; }
    virtual bk3dlib::PBone          AsBone()            { return CBone::AsBone(); }
    virtual bk3dlib::PTransform     AsTransf()          { return NULL; }
    virtual bk3dlib::PTransformSimple AsTransfSimple()  { return NULL; }

    friend class CFileHeader;
};


/*------------------------------------------------------------------
  
  ------------------------------------------------------------------*/
extern void (__stdcall *g_progressCallback)(LPCSTR s, int curItem, int maxItems);

extern MapFileHeader    g_fileHeaders;
extern MapMesh          g_meshes;
extern MapBuffer        g_buffers;
extern MapMaterial      g_materials;
// Transformation list is important to keep : later when using skinning, items
// will be referenced by vertices
// TODO: we might want to expose some way to sort transformations.
extern VecTransform     g_transforms;
extern MapIKHandle      g_ikHandles;
extern MapCurveVec      g_mayacurves;
extern MapQuatCurve     g_quatcurves;


