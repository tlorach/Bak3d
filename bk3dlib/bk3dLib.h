#pragma once
#pragma warning(disable:4530)

#ifndef LPCSTR
typedef const char * LPCSTR;
#endif

//
// These are the typical names of attributes that could be in the baked file
//
#define MESH_POSITION        "position"
#define MESH_VERTEXID        "vertexid"
#define MESH_COLOR           "color"
#define MESH_FACENORMAL      "facenormal"
#define MESH_TANGENT         "tangent"
#define MESH_BINORMAL        "binormal"
#define MESH_NORMAL          "normal"
#define MESH_TEXCOORD0       "texcoord0"
#define MESH_TEXCOORD1       "texcoord1"
#define MESH_TEXCOORD2       "texcoord2"
#define MESH_TEXCOORD3       "texcoord3"
#define MESH_BLIND0          "blind0"
#define MESH_BLIND1          "blind1"
#define MESH_BLIND2          "blind2"
#define MESH_BLIND3          "blind3"
#define MESH_BONESOFFSETS    "bonesoffsets"
#define MESH_BONESWEIGHTS    "bonesweights"

namespace bk3d
{
    struct FileHeader;
}
namespace bk3dlib
{
    void setProgressCallBack(void (__stdcall *progressCallback)(LPCSTR s, int curItem, int maxItems));

    class FileHeader;
    class Mesh;
    class Buffer;
    class Bone;
    class TransformSimple;
    class Transform;
    class TransformDOF;
    class CurveVec;
    class Curve;
    class QuatCurve;
    class Material;
    class IKHandle;
    class PhRigidBody;
    class PhConstraint;

    typedef FileHeader* PFileHeader;
    typedef Mesh* PMesh;
    typedef Buffer* PBuffer;
    typedef Transform* PTransform;
    typedef TransformSimple* PTransformSimple;
    typedef Bone* PBone;
    typedef TransformDOF* PTransformDOF;
    typedef CurveVec* PCurveVec;
    typedef QuatCurve* PQuatCurve;
    typedef Material* PMaterial;
    typedef IKHandle* PIKHandle;
    typedef PhRigidBody* PPhRigidBody;
    typedef PhConstraint* PPhConstraint;


    enum Topology {
        UNKNOWN_TOPOLOGY =0,
        POINTS =1,
        LINES =2,
        LINE_LOOP =3,
        LINE_STRIP =4,
        TRIANGLES =5,
        TRIANGLE_STRIP =6,
        TRIANGLE_FAN =7,
        QUADS =8,
        QUAD_STRIP =9,
        FAN =10,
        LINES_ADJ =11,
        LINE_STRIP_ADJ =12,
        TRIANGLES_ADJ =13,
        TRIANGLE_STRIP_ADJ =14,
        // Use Topology as a way to tell how many vertices in the patch... avoid additional API func()
        PATCHES0 =20, // Only to be used as a base (say, PATCHES0 + i)
        PATCHES1 = 21,
        PATCHES2 = 22,
        PATCHES3 = 23,
        PATCHES4 = 24,
        PATCHES5 = 25,
        PATCHES6 = 26,
        PATCHES7 = 27,
        PATCHES8 = 28,
        PATCHES9 = 29,
        PATCHES10= 30,
        PATCHES11= 31,
        PATCHES12= 32,
        PATCHES13= 33,
        PATCHES14= 34,
        PATCHES15= 35,
        PATCHES16= 36,
        PATCHES17= 37,
        PATCHES18= 38,
        PATCHES19= 39,
        PATCHES20= 40,
        PATCHES21= 41,
        PATCHES22= 42,
        PATCHES23= 43,
        PATCHES24= 44,
        PATCHES25= 45,
        PATCHES26= 46,
        PATCHES27= 47,
        PATCHES28= 48,
        PATCHES29= 49,
        PATCHES30= 50,
        PATCHES31= 51,
        PATCHES32= 52,
        // Reserve up to 32
    };
    enum DataType
    {
        FLOAT32,
        FLOAT16,// Format NOT IMPLEMENTED YET
        UINT32,
        UINT16,
        UINT8,
        UNKNOWN
        //etc. see what we can add from OGL/DX
    };

    //
    // Returns the version of the bk3d version that the library can generate
    //
    extern short bk3dVersion();
    extern void DestroyAllObjects(bool bKeepBakedData=false);

    class FileHeader
    {
    public:
        //
        // Static methods for creation/destruction
        //
        static    PFileHeader Create(LPCSTR name);
        static    PFileHeader CreateFromBk3dFile(LPCSTR file);
        static    bool        FileHeaderDelete(PFileHeader fileheader);
        virtual void          Destroy(bool bKeepBakedData=false) = 0;
        //
        // typical actions on the instanced object
        //
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;

        virtual    bool     LoadFromBk3dFile(LPCSTR file) = 0;
        // removed: we are supposed to create things globally THEN Attach them here
        //virtual PMesh       CreateMesh(LPCSTR name) = 0;
        //virtual PTransform   CreateTransform(LPCSTR name) = 0;
        //virtual PTransformDOF CreateTransformDOF(LPCSTR name) = 0;
        //virtual PCurveVec   CreateCurveVec(LPCSTR name, int ncomps) = 0;
        //virtual PMaterial   CreateMaterial(LPCSTR name) = 0;
        //virtual PIKHandle   CreateIKHandle(LPCSTR name) = 0;

        virtual bool        AttachMesh(PMesh pMesh) = 0;
        virtual bool        DetachMesh(PMesh pMesh) = 0;
        virtual bool        AttachTransform(PBone pTr) = 0;
        virtual bool        DetachTransform(PBone pTr) = 0;
        virtual bool        AttachCurveVec(PCurveVec pCv) = 0;
        virtual bool        DetachCurveVec(PCurveVec pCv) = 0;
        virtual bool        AttachQuatCurve(PQuatCurve pCv) = 0;
        virtual bool        DetachQuatCurve(PQuatCurve pCv) = 0;
        virtual bool        AttachMaterial(PMaterial pMat) = 0;
        virtual bool        DetachMaterial(PMaterial pMat) = 0;
        virtual bool        AttachIKHandle(PIKHandle pMat) = 0;
        virtual bool        DetachIKHandle(PIKHandle pMat) = 0;
        virtual bool        AttachPhRigidBody(PPhRigidBody pB) = 0;
        virtual bool        DetachPhRigidBody(PPhRigidBody pB) = 0;
        virtual bool        AttachPhConstraint(PPhConstraint pC) = 0;
        virtual bool        DetachPhConstraint(PPhConstraint pC) = 0;
        virtual void        UpdateTransformations(bool bBindPose=true) = 0; // recursive update of all transformations

        // cook the data as a big area of memory. detachCookedData=true if we don't want this memory to stay associated with FileHeader
        virtual void*       Cook(LPCSTR file, void ** pBufferMemory=NULL, unsigned int* bufferMemorySz=NULL, bool detachCookedData=false) = 0;
        virtual bool        PtrToOffset() = 0;
        virtual bool        Save(LPCSTR file) = 0;

        //virtual bool       DeleteMesh(PMesh p) = 0;
        virtual PMesh        GetFirstMesh() = 0;
        virtual PMesh        GetNextMesh() = 0;
        virtual PMesh        FindMesh(int n) = 0; ///< \warning Avoid it to loop through the whole meshes. Use GetFirstMesh/GetNextMesh instead
        virtual PMesh        FindMesh(LPCSTR name) = 0;
        virtual int          GetNumMeshes() = 0;
        virtual int          GetNumTransforms() = 0;
        virtual int          GetNumCurves() = 0;
        virtual int          GetNumIKHandles() = 0;
        virtual int          GetNumPhRigidBodies() = 0;
        virtual int          GetNumPhConstraints() = 0;
        virtual PCurveVec    GetCurveVec(int n) = 0;
        virtual PCurveVec    GetCurveVec(LPCSTR name) = 0;
        virtual int          GetNumQuatCurve() = 0;
        virtual PQuatCurve   GetQuatCurve(int n) = 0;
        virtual PQuatCurve   GetQuatCurve(LPCSTR name) = 0;
        virtual int          GetNumMaterials() = 0;
        virtual PMaterial    GetMaterial(LPCSTR name) = 0;
        virtual PMaterial    GetMaterial(int n) = 0;
        virtual PBone        GetTransform(int n) = 0;
        virtual PBone        GetTransform(LPCSTR name, int *n = NULL) = 0; ///< returns the named transform, optionally it's array number (in the transform array)
        virtual PIKHandle    GetIKHandle(int n) = 0;
        virtual PIKHandle    GetIKHandle(LPCSTR name) = 0;
        virtual PPhRigidBody GetPhRigidBody(int n) = 0;
        virtual PPhRigidBody GetPhRigidBody(LPCSTR name) = 0;
        virtual PPhConstraint GetPhConstraint(int n) = 0;
        virtual PPhConstraint GetPhConstraint(LPCSTR name) = 0;
        virtual void         SetUserData(void *p) = 0;
        virtual void         *GetUserData() = 0;
    };

    struct PrimGroup
    {
        LPCSTR              name;
        int                 numIdxBuffers;
        bk3dlib::Buffer*    idxBuffers[10];
        bk3dlib::Topology   topo;
        bk3dlib::PMaterial  pMat;
        unsigned int        offsetElement;
        unsigned int        numElements;
    };

    class Mesh
    {
    public:
        static    PMesh     Create(LPCSTR name);
        virtual void        Destroy() = 0;
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;
        // attributes. Slot allows to interleave or separate the attributes
        //virtual PBuffer     CreateVtxBuffer(LPCSTR name, int numcomp, int slot = 0, DataType type = FLOAT32, bool isBlendShape = false) = 0;
        virtual bool        AttachVtxBuffer(PBuffer pBuf, bool isBlendShape = false) = 0;

        // numcomp>1 would be for multi-indexed mode. Another interleaved solution, as opposed to AddIndexBuffer
        // TODO: check this numComp > 1 case !!
        //virtual PBuffer     CreateIdxBuffer(LPCSTR name, DataType type = UINT32, int numComp=1) = 0;
        //to handle the case of multi-index : allow one action on existing primgroup to add additional refs
        //TODO: create a Primgroup interface, better than referencing it by id... and add more methods to control primgroup...
        virtual int         AttachIndexBuffer(PBuffer idxBuffer, int primGroupID=-1) = 0;
        // Create primitive groups
        virtual int         CreatePrimGroup(LPCSTR name, PBuffer idxBuffer, bk3dlib::Topology topo = bk3dlib::POINTS, bk3dlib::PMaterial pMat=NULL, unsigned int offsetElement = 0, unsigned int numElements = 0) = 0;
        virtual bool        DeletePrimGroupFromName(LPCSTR name) = 0;
        virtual bool        DeletePrimGroupFromIndex(int id) = 0;
        virtual int         GetNumPrimGroups() = 0;
        virtual bool        GetPrimGroupInfo(int i, PrimGroup &pginfo) = 0;

        virtual bool        DetachBuffer(PBuffer pbuffer) = 0;

        virtual PBuffer     GetVtxBuffer(int n, bool isBlendShape = false) = 0;
        virtual PBuffer     GetIdxBuffer(int n) = 0;

        virtual int         GetNumVtxBuffers(bool isBlendShape = false) = 0;
        virtual int         GetNumIdxBuffers() = 0;
        virtual int         GetNumSlots() = 0;
        virtual void        SetSlotName(int s, LPCSTR name) = 0;
        virtual const char* GetSlotName(int s) = 0;
        virtual void        SetBSSlotName(int s, LPCSTR name) = 0;
        virtual const char* GetBSSlotName(int s) = 0;

        virtual bool        ComputeBoundingVolumes(PBuffer source) = 0;

        // Transformations can be used for the whole mesh; for separate primitive groups; or many for the whole mesh when doing some skinning (even many for each primgroup...).
        virtual bool        AddTransformReference(PBone t, int primgroup = -1) = 0;
        virtual int         GetNumTransformReferences(bool bMeshOnly = false) = 0;
        virtual PBone       GetTransformReference(int n) = 0;
        virtual void        ClearTransformReferences(int primgroup = -1) = 0;
        // Normal, Tangent and Bitangent computations : using an index buffer, vertex buffer and texture coords
        virtual bool        ComputeNormalsAndTangents(PBuffer bufferIdx, PBuffer bufferVtxPos, PBuffer bufferTexcoords,
                                PBuffer bufferPerVtxNormals, PBuffer bufferPerFaceNormals = NULL, 
                                PBuffer bufferTangents = NULL, PBuffer bufferBitangents = NULL) = 0;
        virtual void        SetUserData(void *p) = 0;
        virtual void        *GetUserData() = 0;

        virtual void        SetMaxNumBones(int n) = 0;
        virtual int         GetMaxNumBones() = 0;
    };

    class Buffer
    {
    public:
        static  PBuffer     Create(LPCSTR name);
        static  PBuffer     CreateVtxBuffer(LPCSTR name, int numcomp, int slot = 0, DataType type = FLOAT32);
        static  PBuffer     CreateIdxBuffer(LPCSTR name, DataType type = UINT32, int numComp=1);
        virtual void        Destroy() = 0;

        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;

        virtual void     SetDataType(DataType type) = 0;
        virtual DataType GetDataType() = 0;
        virtual void     SetSlot(int slot) = 0;
        virtual int      GetSlot() = 0;
        virtual void     SetNumComps(int n) = 0;
        virtual int      GetNumComps() = 0;
        virtual void     ClearData() = 0;
        virtual void     AddData(const float * p, unsigned int n) = 0;
        virtual void     AddData(const long * p, unsigned int n) = 0;
        virtual void     AddData(const unsigned long * p, unsigned int n) = 0;
        virtual void     AddData(const unsigned int * p, unsigned int n) = 0;
        virtual void     AddData(const unsigned short * p, unsigned int n) = 0;
        virtual void     AddData(const unsigned char * p, unsigned int n) = 0;
        virtual void     AddData(unsigned int i) = 0;
        virtual void     AddData(int i) = 0;
        virtual void     AddData(float f) = 0;
        // TODO: add raw data
        //virtual void     AddData(void* p, int byteSz) = 0;
        virtual void     GotoItem(unsigned int item) = 0; // could be needed if multiple primgroups used the same idx buffer at different offsets...
        virtual void     SetDivisor(int divisor) = 0; // for dividing idx values. Needed for DS...

        // To copy the data in a buffer passed to the method
        virtual int      GetData(float * pDst, unsigned int offsitem=0, unsigned int numitems=0) = 0;
        virtual int      GetData(unsigned int * pDst, unsigned int offsitem=0, unsigned int numitems=0) = 0;

        // "replaces" the existing data, rather than appending. Could be used to "edit" existing values
        virtual bool     SetData(const float * p, unsigned int offsitem, unsigned int numitems) = 0;
        virtual bool     SetData(const long * p, unsigned int offsitem, unsigned int numitems) = 0;
        virtual bool     SetData(const unsigned long * p, unsigned int offsitem, unsigned int numitems) = 0;
        virtual bool     SetData(const unsigned int * p, unsigned int offsitem, unsigned int numitems) = 0;
        virtual bool     SetData(const unsigned short * p, unsigned int offsitem, unsigned int numitems) = 0;
        virtual bool     SetData(const unsigned char * p, unsigned int offsitem, unsigned int numitems) = 0;

        //int        GetNumVectors() = 0;
        virtual int      GetNumItems() = 0; // comps * numvecs
        virtual bool     Reserve(unsigned int numitems) = 0;
        
        /* Single Index Buffer. Example : 
        bk3dlib::PBuffer bufferIdxSingle = mesh->CreateIdxBuffer("primitiveGroup0");
        bufferIdxSingle->SIB_ClearBuffers();
        bufferIdxSingle->SIB_AddBuffers(bufferIdxVtx, bufferVtx, bufferVtxSingle); // try SBMake_AddBuffers(bufferIdxVtx, bufferVtx)
        bufferIdxSingle->SIB_AddBuffers(bufferIdxNormals, bufferNormals, bufferNormalsSingle);
        */
        virtual void SIB_ClearBuffers() = 0;
        virtual void SIB_AddBuffers(PBuffer bufferIdx, PBuffer bufferAttrSrc, PBuffer bufferAttrDst = NULL) = 0;
        virtual bool SIB_Compile() = 0;

        // Only valid for the buffers of indices
        virtual void        setPrimitiveRestartIndex(unsigned int) = 0;
        virtual unsigned int getPrimitiveRestartIndex() = 0;

        virtual void SetUserData(void *p) = 0;
        virtual void *GetUserData() = 0;

    };

    /**
    \brief simplest transformation: a Bone... quat + translation
     **/
    class Bone
    {
    public:
        static  PBone              Create(LPCSTR name);

        virtual void                Destroy() = 0;
        virtual LPCSTR              GetName() = 0;
        virtual void                SetName(LPCSTR name) = 0;
        virtual bk3dlib::PBone      GetChild(int n) = 0;
        virtual bk3dlib::PBone      GetParent() = 0;
        virtual void                SetParent(PBone p) = 0;
        //virtual bool connectCurve(PCurveVec pCVec, TransfComponent comp, int compOffset=0) = 0;
        //virtual bool disconnectCurve(PCurveVec pCVec, TransfComponent comp=TRANSF_DEFCOMP, int compOffset=0) = 0;
        virtual void GetPos(float &x, float &y, float &z) = 0;
        virtual void GetQuaternion(float &x, float &y, float &z, float &w) = 0;
        virtual void GetMatrix(float *m) = 0;
        virtual void GetMatrix_Bindpose(float *m) = 0;
        virtual void GetMatrix_Abs(float *m) = 0;
        virtual const float * GetTailPos() = 0;

        virtual void CopyFrom(PBone from) = 0; ///< Copy the data from an existing transform

        virtual void SetPos(float x, float y, float z) = 0;
        virtual void SetQuaternion(float x, float y, float z, float w) = 0;
        virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) = 0;
        virtual void SetTailPos(float x, float y, float z) = 0;             //bone's taile pos relative to this transform
        virtual void SetMatrix(float *m) = 0;
        virtual void SetAbsMatrix(float *m) = 0;
        virtual void SetMatrixBindpose(float *m) = 0; ///< if m == NULL, matrix will be computed from current transformation
        virtual bool ComputeMatrix(bool bBindPose=true) = 0; ///< compute the matrix. return true if anything changed
        virtual void SetUserData(void *p) = 0;
        virtual void *GetUserData() = 0;
        virtual PTransformDOF   CreateDOF() = 0; ///< creates a Degree Of Freedom for this transformation
        virtual void            DeleteDOF() = 0; ///< removes the DOF from this transformation
        virtual PTransformDOF   GetDOF() = 0;
        virtual PIKHandle       AsIKHandle() = 0; ///< cast the transfor as a TransformDOF if possible
        virtual PPhRigidBody    AsPhRigidBody() = 0; ///< cast the transfor as a TransformDOF if possible
        virtual PPhConstraint   AsPhConstraint() = 0; ///< cast the transfor as a TransformDOF if possible
        virtual PTransform      AsTransf() = 0; ///< cast the transfor as a Transform
        virtual PTransformSimple AsTransfSimple() = 0; ///< cast the transfor as a Transform
        virtual PBone           AsBone() = 0; ///< cast the transfor as a Transform
    };

    /**
    \brief simple transformation... quat + scale + translation
     **/
    class TransformSimple : public Bone
    {
    public:
        static  PTransformSimple Create(LPCSTR name);

        virtual void GetScale(float &x, float &y, float &z) = 0;
        virtual void SetScale(float x, float y, float z) = 0;
    };

    /**
    \brief Maya-style transformation... complex, following Maya rules
     **/
    class Transform : public TransformSimple
    {
    public:
        static  PTransform Create(LPCSTR name);

        virtual void GetRotation(float &x, float &y, float &z) = 0;                    // Euler Rotation in degres

        virtual void GetScalePivot(float &x, float &y, float &z) = 0;
        virtual void GetScalePivotTranslate(float &x, float &y, float &z) = 0;
        virtual void GetRotationPivot(float &x, float &y, float &z) = 0;
        virtual void GetRotationPivotTranslate(float &x, float &y, float &z) = 0;
        virtual void GetRotationOrientation(float &x, float &y, float &z, float &w) = 0;         //Quaternion
        virtual void GetJointOrientation(float &x, float &y, float &z, float &w) = 0;             //Quaternion

        virtual void SetRotation(float a, float b, float g) = 0;                    // Euler Rotation in degres
        virtual void SetRotationOrder(char x, char y, char z) = 0;                  // 3 chars for "xyz" or any other

        virtual void SetScalePivot(float x, float y, float z) = 0;
        virtual void SetScalePivotTranslate(float x, float y, float z) = 0;
        virtual void SetRotationPivot(float x, float y, float z) = 0;
        virtual void SetRotationPivotTranslate(float x, float y, float z) = 0;

        // Bone/Joint(Maya bones) functions. Both funcs tunr the transformation to a Bone/Join special transform
        virtual void SetJointOrientation(float x, float y, float z, float w) = 0;             //Quaternion

        virtual void SetRotationOrientation(float x, float y, float z, float w) = 0;         //Quaternion
    };

    /**
    \brief Modes for TransformDOF
     **/
    enum TransformDOFMode
    {
        DOF_CONE = 0,
        DOF_SINGLE_AXIS_X = 1,
        //DOF_SINGLE_AXIS_Y = 2,
        //DOF_SINGLE_AXIS_Z = 3,
        DOF_TWIST_ALONG_BONE = 4,
        DOF_UNDEF = 0xFFFFFFFF
    };
    class TransformDOF
    {
    public:
        static  PTransformDOF Create(LPCSTR name);
        /// Set DOF Values :
        /// \arg "DOFAlpha" :  angle limit. Used to dot product...
        /// \arg "AxisStart" : start for the limitation
        /// \arg "AxisRange" : range for the limitation
        virtual void SetDOFValues(bk3dlib::TransformDOFMode  mode, float *DOFAlpha, float *AxisStart, float *AxisRange) = 0;
        virtual bk3dlib::TransformDOFMode GetDOFValues(float *DOFAlpha, float *AxisLimitStart, float *AxisLimitRange) = 0;
        virtual void GetQuaternion(float &x, float &y, float &z, float &w) = 0;
        virtual void SetQuaternion(float x, float y, float z, float w) = 0;
        virtual void SetQuaternionFromEulerXYZ(float x, float y, float z) = 0;
    };

    // Components for various objects : used to perform some connections with curves (or later, maybe more than curves)
    typedef enum
    {
        TRANSF_POS=0,
        TRANSF_SCALE,
        TRANSF_EULERROT,
        TRANSF_VISIBILITY,
        //TODO : Add mode in TransfComponent")
        // TODO: add more
        TRANSF_DEFCOMP,
        TRANSF_UNKNOWNCOMP,
    } TransfComponent;
    /// Enum for connection of components in a IK handle
    typedef enum
    {
        IKHANDLE_POS=0,
        IKHANDLE_WEIGHT,
        IKHANDLE_PRIORITY,
        IKHANDLE_UNKNOWNCOMP=0xFFFFFFFF,
    } IKHandleComponent;
    typedef enum
    {
        MESH_VISIBILITY=0,
        //TODO : Add mode in TransfComponent")
        // TODO: add more ?
        MESH_BLENDSHAPE, // this one is when we have a big vector for all the Blendshapes in one vector
        MESH_BLENDSHAPE0, // this one if when we have a vector for each Blendshape
        // Followed by other BS indices
        MESH_UNKNOWNCOMP = 0xFFFFFFFF
    } MeshComponent;

    class CurveVec
    {
    public:
        enum EtInfinityType 
        {
            kInfinityConstant=0,
            kInfinityLinear=1,
            kInfinityCycle=3,
            kInfinityCycleRelative=4,
            kInfinityOscillate=5,
            kInfinityEnd = -1
        };
        enum EtTangentType 
        {
            kTangentGlobal = 0,
            kTangentFixed,
            kTangentLinear,
            kTangentFlat,
            kTangentSmooth,
            kTangentStep,
            kTangentSlow,
            kTangentFast,
            kTangentClamped,
            kTangentPlateau,
            kTangentStepNext,
            kTangentEnd = -1
        };

        static  PCurveVec   Create(LPCSTR name, int ncomps);
        virtual void        Destroy() = 0;
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;
        virtual bool        Connect(Bone *pTransf, TransfComponent comp) = 0;
        virtual bool        Disconnect(Bone *pTransf, TransfComponent comp) = 0;
        virtual bool        Connect(Mesh *pMesh, MeshComponent comp) = 0;
        virtual bool        Disconnect(Mesh *pMesh, MeshComponent comp) = 0;
        virtual bool        Connect(IKHandle *pIKHandle, IKHandleComponent comp) = 0;
        virtual bool        Disconnect(IKHandle *pIKHandle, IKHandleComponent comp) = 0;
        virtual void        SetUserData(void *p) = 0;
        virtual void        *GetUserData() = 0;
        /// setup curve characteristics
        virtual void        SetProps(EtInfinityType preInfinity,
                                EtInfinityType postInfinity,
                                bool    inputIsTime=true,
                                bool    outputIsAngular=true,
                                bool    isWeighted=true) = 0;
        virtual void        GetProps(EtInfinityType *preInfinity,
                                EtInfinityType *postInfinity,
                                bool    *inputIsTime,
                                bool    *outputIsAngular,
                                bool    *isWeighted) = 0;
        virtual void        SetPropsByComp(int comp, EtInfinityType preInfinity,
                                EtInfinityType postInfinity,
                                bool    inputIsTime=true,
                                bool    outputIsAngular=true,
                                bool    isWeighted=true) = 0;
        virtual void        GetPropsByComp(int comp, EtInfinityType *preInfinity,
                                EtInfinityType *postInfinity,
                                bool    *inputIsTime,
                                bool    *outputIsAngular,
                                bool    *isWeighted) = 0;
        virtual int         GetNumKeys(int comp) = 0;
        virtual int         DeleteKey(int comp, int keynum) = 0;
        /// separate assignment of Keys for each component (Maya does this way)
        virtual int         AddKey(int comp, float time, float value, 
                                    EtTangentType inTType, EtTangentType outTType,
                                    float inAngle, float inWeight,
                                    float outAngle, float outWeight
                                    ) = 0;
        /// Add key for all components at the same time : N values for N components. The other params are shared
        virtual int         AddKey(float time, float *value, 
                                    EtTangentType inTType, EtTangentType outTType,
                                    float inAngle, float inWeight,
                                    float outAngle, float outWeight
                                    ) = 0;
        virtual void        GetKey(int comp, int key, float *time, float *value, 
                                    EtTangentType *inTType, EtTangentType *outTType,
                                    float *inAngle, float *inWeight,
                                    float *outAngle, float *outWeight
                                    ) = 0;
    };
    class QuatCurve
    {
    public:
        static  PQuatCurve  Create(LPCSTR name);
        virtual void        Destroy() = 0;
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;
        virtual bool        Connect(Bone *pTransf, TransfComponent comp) = 0;
        virtual bool        Disconnect(Bone *pTransf, TransfComponent comp) = 0;
        virtual bool        Connect(Mesh *pMesh, MeshComponent comp) = 0;
        virtual bool        Disconnect(Mesh *pMesh, MeshComponent comp) = 0;
        virtual bool        Connect(IKHandle *pIKHandle, IKHandleComponent comp) = 0;
        virtual bool        Disconnect(IKHandle *pIKHandle, IKHandleComponent comp) = 0;
        virtual void        SetUserData(void *p) = 0;
        virtual void        *GetUserData() = 0;
        virtual int         GetNumKeys() = 0;
        virtual int         DeleteKey(int keynum) = 0;
        virtual int         AddKey(float time, float* quatVals) = 0;
        virtual void        GetKey(int key, float *time, float *quatVals) = 0;
    };
    class Material
    {
    public:
        static  PMaterial   Create(LPCSTR name);
        virtual void        Destroy() = 0;
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;

        //virtual bool connectCurve(PCurveVec pCVec, TransfComponent comp, int compOffset=0) = 0;
        //virtual bool disconnectCurve(PCurveVec pCVec, TransfComponent comp=TRANSF_DEFCOMP, int compOffset=0) = 0;

        virtual void setDiffuse(float r, float g, float b) = 0;
        virtual void setSpecexp(float s) = 0;
        virtual void setAmbient(float r, float g, float b) = 0;
        virtual void setReflectivity(float s) = 0;
        virtual void setTransparency(float r, float g, float b) = 0;
        virtual void setTranslucency(float s) = 0;
        virtual void setSpecular(float r, float g, float b) = 0;
        virtual void setShaderName(LPCSTR shdName, LPCSTR techName) = 0;
        virtual void setDiffuseTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setSpecExpTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setAmbientTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setReflectivityTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setTransparencyTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setTranslucencyTexture(LPCSTR    name, LPCSTR    filename) = 0;
        virtual void setSpecularTexture(LPCSTR    name, LPCSTR    filename) = 0;

        virtual void getDiffuse(float &r, float &g, float &b) = 0;
        virtual void getSpecexp(float &s) = 0;
        virtual void getAmbient(float &r, float &g, float &b) = 0;
        virtual void getReflectivity(float &s) = 0;
        virtual void getTransparency(float &r, float &g, float &b) = 0;
        virtual void getTranslucency(float &s) = 0;
        virtual void getSpecular(float &r, float &g, float &b) = 0;
        virtual void getShaderName(char* shdName, int shdNameSz, char* techName, int techNameSz) = 0;
        virtual bool getDiffuseTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getSpecExpTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getAmbientTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getReflectivityTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getTransparencyTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getTranslucencyTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual bool getSpecularTexture(char*    name, int nameSz, char*    filename, int filenameSz) = 0;
        virtual void SetUserData(void *p) = 0;
        virtual void *GetUserData() = 0;
    };

    class IKHandle : public Bone
    {
    public:
        static  PIKHandle   Create(LPCSTR name);
        // creates a special IK Handle for influencing effectors with the rotation
        static  PIKHandle   CreateRotateInfluence(LPCSTR name);
        // creates a special IK Handle for influencing Roll rotation (along bone tail) with a percentage of Handle's Roll
        static  PIKHandle   CreateRollInfluence(LPCSTR name);
        enum Mode {
            Default,
            RotateInfluence,
            RollInfluence,
        };
        virtual void        setMode(Mode m) = 0;
        virtual Mode        getMode() = 0;
        virtual void        Destroy() = 0;
        virtual LPCSTR      GetName() = 0;
        virtual void        SetName(LPCSTR name) = 0;

        //virtual bool connectCurve(PCurveVec pCVec, IKHandleComponent comp, int compOffset=0) = 0;
        //virtual bool disconnectCurve(PCurveVec pCVec, IKHandleComponent comp=IK_DEFCOMP, int compOffset=0) = 0;

        virtual void setWeight(float w) = 0;
        virtual void setMaxIter(int i) = 0;
        virtual void setPriority(int pri) = 0;
        virtual void setHandlePos(float x, float y, float z) = 0;
        //virtual void setEffectorPos(float x, float y, float z) = 0;
        // when giving start/end transforms, we assume they are linked : start is a child and End is the parent where it stops
        virtual void setEffectorTransformStart(bk3dlib::PBone pT) = 0;
        virtual void setEffectorTransformEnd(bk3dlib::PBone pT) = 0;
        // Or we can add them all in the right order from end effector (child) to parent where it stops
        // this can be used for the rotate/roll-influence mode, too
        // if all weights == 1.0 the bk3d::IKHandle will avoid storing those values
        virtual void addEffectorTransform(bk3dlib::PBone pT, float weight=1.0) = 0;
        virtual void clearEffectorTransforms() = 0;
    };

    class PhRigidBody : public Bone
    {
    public:
        static  PPhRigidBody Create(LPCSTR name);
        virtual void        Destroy() = 0;

        virtual void setCollisionGroup(unsigned char grp, int collision_group_mask) = 0;
        enum ShapeType {
            Sphere  =0,
            Box     =1,
            Capsule =2,
        };
        virtual void setShapeType(ShapeType t) = 0;
        virtual void setMode(unsigned char   mode) = 0; // clarify this
        virtual void setShapeDimensions(float shape_size[3]) = 0; ///< width: for all; heigh: for sphere/capsules; depth: box shapes
        virtual void setAttributes(float mass, float linear_dampening, float angular_dampening, float restitution, float friction) = 0;
    };

    class PhConstraint : public Bone
    {
    public:
        static  PPhConstraint Create(LPCSTR name);
        virtual void        Destroy() = 0;

        virtual void linkRigidBodies(PPhRigidBody pRigidBody1, PPhRigidBody pRigidBody2) = 0;
        virtual void setTranslationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz) = 0;
        virtual void setRotationLimits(float minx, float miny, float minz, float maxx, float maxy, float maxz) = 0;
        virtual void setSpringConstantTranslation(float x, float y, float z) = 0;
        virtual void setSpringConstantRotation(float x, float y, float z) = 0;
    };

    //
    // Simple conversion of the types
    //
    inline DataType getType(unsigned int t, int *szof)
    {
        DataType type = UNKNOWN;
        switch(t)
        {
        case 0x1402://GL_SHORT:
        case 0x1403://GL_UNSIGNED_SHORT:
            type = UINT16;
            if(szof) *szof = 2;
            break;
        case 0x1404://GL_INT:
        case 0x1405://GL_UNSIGNED_INT:
            type = UINT32;
            if(szof) *szof = 4;
            break;
        case 0x1400://GL_BYTE:
        case 0x1401://GL_UNSIGNED_BYTE:
            type = UINT8;
            if(szof) *szof = 1;
            break;
        case 0x1406://GL_FLOAT:
            type = FLOAT32;
            if(szof) *szof = 4;
            break;
        case 0x1407://GL_2_BYTES:
        case 0x1408://GL_3_BYTES:
        case 0x1409://GL_4_BYTES:
        case 0x140A://GL_DOUBLE:
        default:
            //assert(!"Ooops");
            type = UNKNOWN;
            if(szof) *szof = 0;
            break;
        }
        return type;
    }

    inline Topology getTopology(unsigned int t)
    {
        Topology topology;
        switch(t)
        {
        case 0x0000://GL_POINTS:
            topology = POINTS;
            break;
        case 0x0001://GL_LINES:
            topology = LINES;
            break;
        case 0x0002://GL_LINE_LOOP:
            topology = LINE_LOOP;
            break;
        case 0x0003://GL_LINE_STRIP:
            topology = LINE_STRIP;
            break;
        case 0x0004://GL_TRIANGLES:
            topology = TRIANGLES;
            break;
        case 0x0005://GL_TRIANGLE_STRIP:
            topology = TRIANGLE_STRIP;
            break;
        case 0x0006://GL_TRIANGLE_FAN:
            topology = TRIANGLE_FAN;
            break;
        case 0x0007://GL_QUADS:
            topology = QUADS;
            break;
        case 0x0008://GL_QUAD_STRIP:
            topology = QUAD_STRIP;
            break;
        case 0x000E://GL_PATCHES:
            topology = PATCHES0; // returns the #0 and we assume you'll add the # of vertices
            break;
        /*case GL_LINES_ADJ: TODO
            topology = LINES_ADJ;
            break;
        case GL_LINE_STRIP_ADJ:
            topology = LINE_STRIP_ADJ;
            break;
        case GL_TRIANGLES_ADJ:
            topology = TRIANGLES_ADJ;
            break;
        case GL_TRIANGLE_STRIP_ADJ:
            topology = TRIANGLE_STRIP_ADJ;
            break;*/
        default:
            topology = UNKNOWN_TOPOLOGY;
            break;
        }
        return topology;
    }

} //namespace

