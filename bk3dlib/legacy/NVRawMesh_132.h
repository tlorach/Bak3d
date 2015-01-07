#pragma once
//----------------------------------------------------------------------------------
// File:   nvrawmesh.h
// Author: Tristan Lorach
// Email:  tlorach@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
/**
 ** This version is used by each Nodes.
 ** v129 : added different userParams/Ptrs. Flags for Transforms.
 ** v128 : added ...
 ** v127 : added the ability to share index buffers between primgroups. Added the concept of ownership
 ** v131 : added user data slots to be able to temporarily keep data (OpenGL layer...)
 ** v132 : relocation table change to allow file mapping on memory; IK Handle (IK DOF are done : TransformDOF)
 **/
#define RAWMESHVERSION132 0x132

#pragma warning(disable: 4505)
#pragma warning(disable: 4311)
#pragma warning(disable: 4996)

#ifndef INLINE
#define INLINE inline
#endif

#ifndef __NVRAWMESH__
#define __NVRAWMESH__

//-------------------------------------------------
// 
// D3D9 definitions
// ----------------
// This part is needed when OpenGL/DX9-10 is not used :
// some enums & defines are needed anyways.
// Instead of including OpenGL only for that,
// this section will define them
// Furthermore : this can be used by any exporter/converter
// 
//-------------------------------------------------
#ifndef _d3d9TYPES_H_
#ifdef BK3DVERBOSE
#pragma message("defining D3DPRIMITIVETYPE here...")
#endif
  enum D3DPRIMITIVETYPE
  {
    D3DPT_UNDEFINED             = 0,
    D3DPT_POINTLIST             = 1,
    D3DPT_LINELIST              = 2,
    D3DPT_LINESTRIP             = 3,
    D3DPT_TRIANGLELIST          = 4,
    D3DPT_TRIANGLESTRIP         = 5,
    D3DPT_TRIANGLEFAN           = 6,
	D3DPT_END = -1
};
#ifdef BK3DVERBOSE
#pragma message("defining D3DFORMAT here...")
#endif
  enum D3DFORMAT
  {
    D3DFMT_INDEX16              = 101,
    D3DFMT_INDEX32              = 102,
	D3DFMT_END = -1
  };
#ifdef BK3DVERBOSE
#pragma message("defining D3DDECLTYPE here...")
#endif
  enum D3DDECLTYPE
  {
    D3DDECLTYPE_FLOAT1 = 0,
    D3DDECLTYPE_FLOAT2 = 1,
    D3DDECLTYPE_FLOAT3 = 2,
    D3DDECLTYPE_FLOAT4 = 3,
    D3DDECLTYPE_D3DCOLOR = 4,
    D3DDECLTYPE_UBYTE4 = 5,
    D3DDECLTYPE_SHORT2 = 6,
    D3DDECLTYPE_SHORT4 = 7,
    D3DDECLTYPE_UBYTE4N = 8,
    D3DDECLTYPE_SHORT2N = 9,
    D3DDECLTYPE_SHORT4N = 10,
    D3DDECLTYPE_USHORT2N = 11,
    D3DDECLTYPE_USHORT4N = 12,
    D3DDECLTYPE_UDEC3 = 13,
    D3DDECLTYPE_DEC3N = 14,
    D3DDECLTYPE_FLOAT16_2 = 15,
    D3DDECLTYPE_FLOAT16_4 = 16,
    D3DDECLTYPE_UNUSED = 17,

    D3DDECLTYPE_UNDEF     =  -1,
  };
#endif

//-------------------------------------------------
// 
// D3D10 definitions
// ----------------
// This part is needed when OpenGL is not used :
// some enums & defines are needed anyways.
// Instead of including OpenGL only for that,
// this section will define them
// Furthermore : this can be used by any exporter/converter
// 
//-------------------------------------------------
#ifndef __d3d10_h__
#ifdef BK3DVERBOSE
#pragma message("defining D3D10_PRIMITIVE_TOPOLOGY enum...")
#endif
  enum D3D10_PRIMITIVE_TOPOLOGY
  {	
      D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED	        = 0,
	  D3D10_PRIMITIVE_TOPOLOGY_POINTLIST	        = 1,
	  D3D10_PRIMITIVE_TOPOLOGY_LINELIST	            = 2,
	  D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP	        = 3,
	  D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST	        = 4,
	  D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	    = 5,
	  D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ	        = 10,
	  D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ	    = 11,
	  D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ	    = 12,
	  D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ	= 13,
	  //D3D10_PRIMITIVE_TOPOLOGY_FAN	            = 14                // Doesn't exist in DXGI...
	  D3D10_PT_END = -1
  };
#ifdef BK3DVERBOSE
#pragma message("defining DXGI_FORMAT enum...")
#endif
  enum DXGI_FORMAT // stick to DXGI values
  {
    DXGI_FORMAT_UNKNOWN                = 0,
    DXGI_FORMAT_R32_FLOAT	        = 41,
    DXGI_FORMAT_R32G32_FLOAT	        = 16,
    DXGI_FORMAT_R32G32B32_FLOAT	    = 6,
    DXGI_FORMAT_R32G32B32A32_FLOAT	= 2,
    DXGI_FORMAT_R16_UINT	            = 57,
    DXGI_FORMAT_R32_UINT	            = 42,
	DXGI_FORMAT_R32G32_UINT	        = 17,
	DXGI_FORMAT_R32G32B32_UINT	    = 7,
	DXGI_FORMAT_R32G32B32A32_UINT	= 3,
	DXGI_FORMAT_R8_UINT	        = 62,
	DXGI_FORMAT_R8G8_UINT	    = 50,
	DXGI_FORMAT_R8G8B8A8_UINT	= 30,
	DXGI_FORMAT_END = -1
  };
enum D3D10_INPUT_CLASSIFICATION
    {	D3D10_INPUT_PER_VERTEX_DATA	= 0,
	D3D10_INPUT_PER_INSTANCE_DATA	= 1
    };
#endif

#if !defined( __d3d11_h__ )
#if defined __d3dcommon_h__
#define D3D11_PRIMITIVE_TOPOLOGY D3D_PRIMITIVE_TOPOLOGY
#else
typedef 
enum D3D11_PRIMITIVE_TOPOLOGY
    {
    D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED	= 0,
    D3D11_PRIMITIVE_TOPOLOGY_POINTLIST	= 1,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST	= 2,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP	= 3,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST	= 4,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP	= 5,
    D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ	= 10,
    D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ	= 11,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ	= 12,
    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ	= 13,
    D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST	= 33,
    D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST	= 34,
    D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST	= 35,
    D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST	= 36,
    D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST	= 37,
    D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST	= 38,
    D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST	= 39,
    D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST	= 40,
    D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST	= 41,
    D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST	= 42,
    D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST	= 43,
    D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST	= 44,
    D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST	= 45,
    D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST	= 46,
    D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST	= 47,
    D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST	= 48,
    D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST	= 49,
    D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST	= 50,
    D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST	= 51,
    D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST	= 52,
    D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST	= 53,
    D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST	= 54,
    D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST	= 55,
    D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST	= 56,
    D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST	= 57,
    D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST	= 58,
    D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST	= 59,
    D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST	= 60,
    D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST	= 61,
    D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST	= 62,
    D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST	= 63,
    D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST	= 64
    } 	D3D11_PRIMITIVE_TOPOLOGY;
#endif // common
#endif //d3d11

//-------------------------------------------------
// 
// OpenGL enums...
// This part is needed when OpenGL is not used :
// some enums & defines are needed anyways.
// Instead of including OpenGL only for that,
// this section will define them
// Furthermore : this can be used by any exporter/converter
// 
//-------------------------------------------------
#ifndef __gl_h_
  typedef unsigned int GLenum;
  typedef GLenum GLType;
  typedef GLenum GLTopology;
  //enum GLTopology // turn GL enums in real enums ?
  //{
#define    GL_POINTS                         0x0000
#define    GL_LINES                          0x0001
#define    GL_LINE_LOOP                      0x0002
#define    GL_LINE_STRIP                     0x0003
#define    GL_TRIANGLES                      0x0004
#define    GL_TRIANGLE_STRIP                 0x0005
#define    GL_TRIANGLE_FAN                   0x0006
#define    GL_QUADS                          0x0007
#define    GL_QUAD_STRIP                     0x0008
#define    GL_PATCHES                        0x000E
  //};
  //enum GLType
  //{
    // enums from OpenGL so that we are directly ready
#define    GL_BYTE                           0x1400
#define    GL_UNSIGNED_BYTE                  0x1401
#define    GL_SHORT                          0x1402
#define    GL_UNSIGNED_SHORT                 0x1403
#define    GL_INT                            0x1404
#define    GL_UNSIGNED_INT                   0x1405
#define    GL_FLOAT                          0x1406
#define    GL_2_BYTES                        0x1407
#define    GL_3_BYTES                        0x1408
#define    GL_4_BYTES                        0x1409
#define    GL_DOUBLE                         0x140A
  //};
#else
  typedef GLenum GLType;
  typedef GLenum GLTopology;
#endif
enum OGL_PATCH_VERTICES
{
	GL_PATCH_VERTICES_0	= 32,
	GL_PATCH_VERTICES_1	= 33,
	GL_PATCH_VERTICES_2	= 34,
	GL_PATCH_VERTICES_3	= 35,
	GL_PATCH_VERTICES_4	= 36,
	GL_PATCH_VERTICES_5	= 37,
	GL_PATCH_VERTICES_6	= 38,
	GL_PATCH_VERTICES_7	= 39,
	GL_PATCH_VERTICES_8	= 40,
	GL_PATCH_VERTICES_9	= 41,
	GL_PATCH_VERTICES_10	= 42,
	GL_PATCH_VERTICES_11	= 43,
	GL_PATCH_VERTICES_12	= 44,
	GL_PATCH_VERTICES_13	= 45,
	GL_PATCH_VERTICES_14	= 46,
	GL_PATCH_VERTICES_15	= 47,
	GL_PATCH_VERTICES_16	= 48,
	GL_PATCH_VERTICES_17	= 49,
	GL_PATCH_VERTICES_18	= 50,
	GL_PATCH_VERTICES_19	= 51,
	GL_PATCH_VERTICES_20	= 52,
	GL_PATCH_VERTICES_21	= 53,
	GL_PATCH_VERTICES_22	= 54,
	GL_PATCH_VERTICES_23	= 55,
	GL_PATCH_VERTICES_24	= 56,
	GL_PATCH_VERTICES_25	= 57,
	GL_PATCH_VERTICES_26	= 58,
	GL_PATCH_VERTICES_27	= 59,
	GL_PATCH_VERTICES_28	= 60,
	GL_PATCH_VERTICES_29	= 61,
	GL_PATCH_VERTICES_30	= 62,
	GL_PATCH_VERTICES_31	= 63,
	GL_PATCH_VERTICES_32	= 64
};


///
/// \brief These are the typical names of attributes that could be in the bk3d baked file
/// @{
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
/// @}

#endif //__NVRAWMESH__

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//// STRUCTURES STRUCTURES STRUCTURES STRUCTURES STRUCTURES STRUCTURES STRUCTURES STRUCTURES
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////


namespace bk3d132
{
/** 
 ** \name Node : for transforms, Meshes...
 ** @{
 **/
#define NODE132_HEADER     0 ///< \brief the very first node that you should find in the binary file
#define NODE132_MESH       1 ///< \brief the mesh node
#define NODE132_TRANSFORM  2
#define NODE132_JOINT      3
#define NODE132_TRANSFPOOL 4
#define NODE132_ATTRIBUTE  5
#define NODE132_SLOT		6
#define NODE132_PRIMGROUP  7
#define NODE132_RELOCTABLE 8
#define NODE132_MAYACURVE  9
#define NODE132_MAYACURVEVECTOR  10
#define NODE132_FLOATARRAY 11
#define NODE132_MATERIAL   12
#define NODE132_IKHANDLE   13
#define NODE132_DOF        14
//...
#define NODE132_END		15
/// @}
//
// Macro to reserve 64 bits in any case : x86 or x64
//
#ifdef NO64BITSCOMPAT // don't do any 32/64 bits compatibility... Doxygen will like it, for example
#define PTR64(a) a
#else
#define PTR64(a)\
	union {\
	a;\
    struct { int : 32; int : 32; };	}
#endif
///
/// \brief Node is a header for each important component.
///
/// The bk3d file is made of a list of nodes of different sizes. I should be possible to easily concatenate them or split a file Node by node.
/// 
struct Node
{
	short               nodeType; 		///< See \ref NODE132_HEADER
	short				version;		///< \brief Node version. Most of the time we expect all to be the same version. NODE132_HEADER's version is the most relevant.
	unsigned int        nodeByteSize; 	///< Size in bytes of this node : this includes all what is necessary to make this node consistent.
#	define NODENAMESZ (4*8)
	char                name[NODENAMESZ];	///< NOTE: the size of the string is so that Node is either good for 64 and 32 bits
	PTR64(Node			*nextNode); ///< pointer to the next node. 32/64 bits compliant pointer
    // TODO for later: shall we add a pointer to some optional comments ? Could be good to have Node self-documented
};

///
/// \brief connector NODE132_FLOATARRAY for vectors/matrices/scalars of Floats.
///
/// This structure will be referenced by Transforms or Meshes or others.
/// this is used when for example a curve is supposed to change something
/// on the receiver. IncomingValue is generic. Can be a curveVector or other...
///
struct FloatArray : public Node
{
	int		dim; ///< amount of components in f
	float	f[1]; ///< array of dim floats. f[1] will be in fact f[dim]
	/// Constructor of this Node
	FloatArray() {		
		memset((void*)this, 0, sizeof(FloatArray));
		nodeType = NODE132_FLOATARRAY;
		version = RAWMESHVERSION132;
		strcpy_s(name, 31, "FloatArray");
		}
};

/// \brief Pool of FloatArray's
///
/// \remark this structure doesn't require to be a Node.
///
/// This pool would gather for example all the input data that would also be connected to a curve.
///
/// in a Mesh : each Blendshape would have a curve...
struct FloatArrayPool
{
	FloatArrayPool() {		
		memset((void*)this, 0, sizeof(FloatArrayPool));
		}
	int			n; ///< amount of FloatArray
	int			: 32;
	struct Connection ///< associates the source and destination
	{
		char destName[32];		///< name of the destination
		PTR64(FloatArray *p);  ///<pointer to a FloatArray source
		PTR64(float *pTarget); ///<ptr to the destination where to write the data from FloatArray. Can be NULL if we want to rely only on destName...
	};
	Connection p[1]; ///< serie of 'n' connections
};

struct Slot;
struct Attribute;
struct PrimGroup;
struct Mesh;
struct Transform;
struct MaterialAttr;
struct TransformDOF;
struct IKHandlePool;
struct IKHandle;

/// \brief Ptr64 is a class template used to align pointers to 64 bits even when under 32 bits.
///
/// The issue is that the bk3d file binaries must be compatible with 32 and 64 bits systems. Meaning that we \b need to borrow 64 bits area
/// for each pointer, even if we are in a 32 bit system.
/// this template essentially allows to avoid a level of indirection when writing code
/// \code
/// Ptr64 myPtr;
/// myPtr.p->doSomething
/// //can be written :
/// myPtr->doSomething
/// \endcode
template <class T>
struct Ptr64 
{
	/// overloading '=' for ptr assignment
	inline T* operator =(T * _p) 
	{
		p = _p;
		return p;
	}
	/// overloading the pointer request
	inline operator T*(void) 
	{ 
		return p; 
	}
	/// overloading the cast to char
	inline operator char*(void) 
	{ 
		return (char*)p;
	}
	/// overloading the field access of the pointer
	inline T* operator->(void) 
	{ 
		return p; 
	}
    #ifdef NO64BITSCOMPAT // don't do any 32/64 bits compatibility... Doxygen will like it, for example
        T *p;			///< the pointer that we are interested in
    #else
	//PTR64(T *p); 
	union {
        T *p;			///< the pointer that we are interested in
        long long ll;	///< a long-long to borrow the bits...
    };
    #endif
};
/*--------------------------------
Bounding volumes
----------------------------------*/
/// Bounding Sphere that is supposed to encomparse the vertices of a Mesh
struct BSphere
{
    float pos[3]; ///< center of the sphere
    float radius; ///< radius
};
/// Axis-aligned bounding box
struct AABBox
{
    float min[3]; // min 3D position
    float max[3]; // max 3D position
};

/*--------------------------------
MAYA Animation Curves. We define them through
- vector of curve
	- contains curves
		- contains keys
----------------------------------*/
/// type of behavior for the curve at infinity
enum EtInfinityType 
{
	kInfinityConstant=0,
	kInfinityLinear=1,
	kInfinityCycle=3,
	kInfinityCycleRelative=4,
	kInfinityOscillate=5,
	kInfinityEnd = -1
};
/// tangent type of the curve
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

typedef enum EtTangentType EtTangentType;

/// maya curve key. You can refer to Maya documentation for details
struct MayaReadKey
{
	float			time;
	float			value;

	EtTangentType	inTangentType	: 32;
	EtTangentType	outTangentType	: 32;

	float			inAngle;
	float			inWeight;

	float			outAngle;
	float			outWeight;
};

/// maya curve object. This is also related to Maya documentation
struct MayaCurve : public Node
{
	MayaCurve()
	{
		memset((void*)this, 0, sizeof(MayaCurve));
		nodeType = NODE132_MAYACURVE;
		version = RAWMESHVERSION132;
		inputIsTime = true;
		outputIsAngular = false;
		isWeighted = false;
		preInfinity = kInfinityConstant;
		postInfinity = kInfinityConstant;
	}
	// Curve Settings
	EtInfinityType preInfinity	: 8;	///< how to evaluate pre-infinity			
	EtInfinityType postInfinity	: 8;	///< how to evaluate post-infinity		
	bool	inputIsTime			: 1;	///< if true, the input do not need Plugs to increase
	bool	outputIsAngular		: 1;
	bool	isWeighted			: 1;	///< whether or not this curve has weighted tangents 
	int							: 13;
	// IO
	float	fIn;		///< input value

	float	fOut;		///< result
	// KEYS
	int		nKeys;

	MayaReadKey key[1]; ///< array of keys
};

/// vector of maya curve. Most of the time we use vector curves (to connect to any 3D component...)
/// 
/// Note that we use FloatArray in order to allow these curve-vectors to be connected to something else (see FloatArrayPool )
struct MayaCurveVector : public Node
{
	int		nCurves	: 32;
	int				: 32;
	PTR64(FloatArray	*pFloatArray); ///< place to write the final result. It's a pointer because FloatArray's size can vary
	Ptr64<MayaCurve>	pCurve[1]; ///< nCurves that are used to define the curve-vector
	//-----------------------
	MayaCurveVector()
	{
		memset((void*)this, 0, sizeof(MayaCurveVector));
		nodeType = NODE132_MAYACURVEVECTOR;
		version = RAWMESHVERSION132;
	}
};

/// pool of curves
struct MayaCurvePool //: public Node
{
	int				n; ///< amount of curves
	int				: 32;
	Ptr64<MayaCurveVector> p[1]; ///< array of n curves
};

/// Pool of slots ('stream' in DX10)
struct SlotPool //: public Node
{
int       n; ///< amount of slots
int		  : 32;
  Ptr64<Slot>		p[1]; ///< array of n slots
};

/// Pool of vertex attributes
struct AttributePool //: public Node // == Layout : contains all the attributes in a list
{
	int       n; 			///< amount of attributes
	int		  : 32;
	Ptr64<Attribute> p[1];	///< array of attributes
};

/// Pool of Primitive groups
struct PrimGroupPool //: public Node
{
	int       n;
	int		  : 32;
	Ptr64<PrimGroup> p[1];
};

/// Pool of Meshes
struct MeshPool //: public Node
{
	int       n : 32;
	int			: 32;
	Ptr64<Mesh>		p[1];
};

/// Pool of transformations
struct TransformPool
{
	int       n : 32;
	int			: 32;
	Ptr64<Transform> p[1];    						///< transforms referenced by pointers
};

/// Pool of materials
struct MaterialAttrPool
{
	int			n;
	int		  : 32;
	Ptr64<MaterialAttr>	p[1];
};
/*--------------------------------
Transform, Transform Pool and references
This contains all the tranformations and children/parent infos
----------------------------------*/
  #define TRANSFCOMP_pos                    (1<<0)
  #define TRANSFCOMP_scale                  (1<<1)
  #define TRANSFCOMP_rotation               (1<<2)
  #define TRANSFCOMP_rotationQuat           (1<<3)
  #define TRANSFCOMP_rotationOrder          (1<<4)
  #define TRANSFCOMP_scalePivot             (1<<5)
  #define TRANSFCOMP_scalePivotTranslate    (1<<6)
  #define TRANSFCOMP_rotationPivot          (1<<7)
  #define TRANSFCOMP_rotationPivotTranslate (1<<8)
  #define TRANSFCOMP_rotationOrientation    (1<<9)
  #define TRANSFCOMP_jointOrientation       (1<<10)
  #define TRANSFCOMP_bindPose               (1<<11)
  #define TRANSFCOMP_isBone                 (1<<12)
  #define TRANSFCOMP_matrix                 (1<<13)
  #define TRANSFCOMP_abs_matrix             (1<<14)
  #define TRANSFCOMP_bindpose_matrix        (1<<15)
  #define TRANSFCOMP_invalidMatrix          (1<<31)

  #define TRANSFCOMP_MAYATRANSF (TRANSFCOMP_scale|TRANSFCOMP_pos|TRANSFCOMP_rotation|TRANSFCOMP_scalePivot|TRANSFCOMP_scalePivotTranslate\
                                |TRANSFCOMP_rotationPivot|TRANSFCOMP_rotationPivotTranslate|TRANSFCOMP_rotationOrientation)
  #define TRANSFCOMP_MAYAJOINT (TRANSFCOMP_scale|TRANSFCOMP_rotationOrientation|TRANSFCOMP_rotation|TRANSFCOMP_jointOrientation|TRANSFCOMP_pos\
                               |TRANSFCOMP_isBone/*|TRANSFCOMP_bindpose_matrix|TRANSFCOMP_bindPose*/)
  ///
  /// \brief Transform (NODE132_TRANSFORM) : a Maya-centric transformation system
  ///
  /// Joint transformation order (OpenGL order) :
  ///
  /// P2 = Mtranslate * MjointOrient * Mrotation * Mrotorientation * Mscale * P
  ///
  /// basic Transformation order :
  ///
  /// P2 = MrotPivotTransl * MrotPivot * Mrotation * MrotOrient * MrotPivotInv * MscalePivotTransl 
  ///      * MscalePivot * Mscale * MscalePivotInv * P
  /// 
  struct Transform : public Node
  {
      PTR64(FloatArrayPool *pFloatArrays);   ///< array of float coming from curves or anything else derived from FloatArray
      PTR64(Transform      *pParent);        ///< pointer to the parent
      PTR64(TransformPool  *pChildren);
      PTR64(TransformDOF   *pDOF);           ///< ptr to a DOF component if this transform should be limited in its movement...
      PTR64(IKHandlePool   *pIKHandles);     ///< Pool of IK Handles to which the tranform can be influenced by
      union {
        float     matrix4x4[4][4];           ///< resulting matrix ( TRANSFCOMP_matrix to 1 if a default one is available )
        float     matrix[16];                ///< resulting matrix ( TRANSFCOMP_matrix to 1 if a default one is available )
      };
      union { 
        float     abs_matrix4x4[4][4];       ///< resulting matrix ( TRANSFCOMP_abs_matrix to 1 if a default one is available )
        float     abs_matrix[16];            ///< resulting matrix ( TRANSFCOMP_abs_matrix to 1 if a default one is available )
      };
      union {   ///< initially contains the Inverse default world->transf so the vertex can be put in local-to-bone space.
        float     bindpose_matrix4x4[4][4];  ///< bindpos matrix ( TRANSFCOMP_bindpose_matrix to 1 if a default one is available )
        float     bindpose_matrix[16];     	 ///< bindpos matrix ( TRANSFCOMP_bindpose_matrix to 1 if a default one is available )
      };
      //
      // typical Maya things
      //
      float     pos[3];					///< TRANSFCOMP_pos
      float     scale[3];				///< TRANSFCOMP_scale
      float     rotationQuat[4];       	///< Euler Rotation in degres (TRANSFCOMP_rotation)
      float     rotation[3];           	///< Euler Rotation in degres (TRANSFCOMP_rotation)
      char      rotationOrder[3];      	///< 3 chars for "xyz" or any other (TRANSFCOMP_rotationOrder)
      // Do we really need all of them ?
      float     scalePivot[3];			///< TRANSFCOMP_scalePivot
      float     scalePivotTranslate[3];	///< TRANSFCOMP_scalePivotTranslate
      float     rotationPivot[3];		///< TRANSFCOMP_rotationPivot
      float     rotationPivotTranslate[3];///< TRANSFCOMP_rotationPivotTranslate

      float     rotationOrientation[4]; ///< TRANSFCOMP_rotationOrientation, Quaternion
      float     jointOrientation[4];    ///< TRANSFCOMP_jointOrientation, Quaternion

      /// when bDirty is true, you should recompute the matrices (relative, absolute, skin). The consequence is that
      /// some children of the dirty transformations should either re-compute their absolute matrices
      /// when bDirty is false, all the matrices are ready to use.
      bool          bDirty;
	  /// TRANSFCOMP_ flags. These bits are important to know which field is valid.
	  /// \note   TRANSFCOMP_matrix, TRANSFCOMP_abs_matrix, TRANSFCOMP_bindpose_matrix mean that
	  /// the matrices are there and ready to use from the baked export. This means that it is not needed to recompute
	  /// them prior to use them. Unless something changed (say, pos etc.).
	  /// If no other flags are valid, this means that nothing but the matrices are available.
	  ///
	  /// \note Inversely, if these matrix bits aren't set but only separate components are, then this means that
	  /// you must compte the matrices before using them.
	  /// 
	  /// \note Flag TRANSFCOMP_isBone : set to 1 when the transformation is considered as part of a skeleton
      /// this flag is useful for IK computation and to draw a line between a "scene-graph" hierarchy and a skeletong.
      /// The choice of mixing all together like a big hierarchy was made to simplify the tree of transformations.
	  ///
	  /// \note Flag TRANSFCOMP_invalidMatrix : Just tells if the resulting matrices are valid or not
	  ///
      unsigned int  validComps;
      //
      // 
      //
// TODO : connection with animation curves, if existing
      //CurveData  *cvPos[3];
      // scale[3] etc...
      // ? Shall I set a Pool ?
      //PTR64(CurvePool *pCurves);
      //
      //

      Transform() {init();}
      void init()
      {
        memset((void*)this, 0, sizeof(Transform));
        nodeType = NODE132_TRANSFORM;
	      version = RAWMESHVERSION132;
        //validComps = TRANSFCOMP_pos|TRANSFCOMP_scale|TRANSFCOMP_rotation|TRANSFCOMP_rotationOrder|TRANSFCOMP_scalePivot|TRANSFCOMP_scalePivotTranslate|TRANSFCOMP_rotationPivot|TRANSFCOMP_rotationPivotTranslate|TRANSFCOMP_rotationOrientation|TRANSFCOMP_jointOrientation|TRANSFCOMP_bindPose
      }
	  void debugDumpLayout(int l, bool brief, const char * nodeNameFilter);
  };

  /// \brief pool of transformations
  ///
  /// Here we get 3 choice :
  /// -# normal mesh will have a 1 Transformation reference. Or even none
  /// -# Skinned meshes will get N transforms for Bones. Skinning indices in Vertex attribs will refer to transforms in the array here.
  /// -# Simple instancing : N transforms for N instances of this same Mesh
  struct TransformRefs// : public Node
  {
	int				 n	: 32;        
	int					: 32;
	Ptr64<Transform> p[1]; 		///< first we get offsets here. Load will resolve as correct ptrs
  };

//--------------------------------
/**
\brief Degree Of Freedom object : allows to restrict the freedom of a transformation

A DOF Component inherits from bk3d::Transform, with additional information, so that we can orient the DOF.
Note that we could have made the DOF a separate component but it would have made things more complex.
The fact that it inherits from bk3d::Transform allows DOF to be part of the transform tree / list as a Transform very simply.

Its pointer will be referenced in a transformation: bk3d::Transform::pDof it needs to be applied.

The following comes from the DOF Locator plugin
- "DOFAlpha", "dofa" :  angle limit. Used to dot product...
- "SingleDOF", "sdof": boolean  to tell the DOF is just one axis along Z
- "theColor", "tc"
- "drawLast", "dL"
- "OxAxisLimit", "oxlim" : boolean
- "aOxLimitStart", "oxstart" : float
- "aOxLimitRange", "oxrange" : float
**/
struct TransformDOF : public Transform
{
	float	DOFAlpha;///< "dofa" :  angle limit. Used to dot product...
	/*bool*/char	SingleDOF;///< "sdof": boolean  to tell the DOF is just one axis along Z
	/*bool*/char	OxAxisLimit;///< "oxlim" : if we want a DOF along the Ox axis (Ox is the axis of the bone
    int : 16;

	float	OxLimitStart;///< "oxstart" : start for the limitation
	float	OxLimitRange;///< "oxrange" : range for the limitation
	//float	theColor[3];///< "tc" : Color to display the DOF...
	//bool	drawLast;///< "dL" : if we want the DOF to be rendered after the 3D objects...
    TransformDOF() {init();}
    void init()
    {
        memset((void*)this, 0, sizeof(TransformDOF));
        nodeType = NODE132_DOF;
          version = RAWMESHVERSION132;
    }
    //void debugDumpLayout(int l, bool brief, const char * nodeNameFilter);
};

///
/// pool of Inverse Kinematic Handles
///
struct IKHandlePool
{
	int       n;
	int		  : 32;
	Ptr64<IKHandle> p[1];
};
///
/// \brief Handle for IK
///
/// This handle is attached to a transform.
/// The IK process : try to make this handle reach a destPos
///
struct IKHandle : public Node
{
    float pos[3]; ///< position relative to the first transform : handle position
    float target[3]; ///< target position relative to the optional transform
    /// pool of transforms for pos : has all from pos's parent tranform to Transform's parents...
    /// used to resolve the IK : if the transform isn't here, it won't be used for IK
    /// the first in this list is the tranform to which we attached the handle (pos)
    /// the others are the active parents of this first transform
    PTR64(TransformPool     *pAttachedTransforms); ///< attached transforms for IK
    PTR64(Transform         *targetTransform); ///< optional transform for the target
    /// FloatArrays allow to connect to pos or target (animation...)
    PTR64(FloatArrayPool    *pFloatArrays);
	IKHandle() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(IKHandle));
      strcpy(name, "IKHANDLE");
      nodeType = NODE132_IKHANDLE;
	  version = RAWMESHVERSION132;
    }
};

/*--------------------------------
Mesh related structures
----------------------------------*/

///
/// \brief Attribute of a vertex (NODE132_ATTRIBUTE) : data required to setup vertex shader's input attributes
///
/// The idea is to provide an much information as possible so the application can easily setup data
/// (such as Input-Layout in DX; or such as OpenGL string and offsets for AttributePointer...)
///
  struct Attribute : public Node
  {
	D3DDECLTYPE         formatDX9	: 32;   ///< DX9 type for the attribute format
    DXGI_FORMAT         formatDXGI	: 32;   ///< DX10/11 type for the attribute format

    GLType              formatGL	: 32;   ///< OpenGL format. But it doesn't say how many components. See numComp.
    unsigned char       semanticIdx;        ///< semantic index is for example the Idx of the texcoord level
    unsigned char       numComp;            ///< amount of components (since formatGL doesn tell about this)
    unsigned char       strideBytes;        ///< stride in bytes of the attribute
    unsigned char       alignedByteOffset;  ///< DirectX10 needs it in the Input-Layout definition : offset in the layout.

    unsigned int        dataOffsetBytes;    ///< for example, use this for VertexPointer() in VBO or what we do with offsetof(VertexStructure, p)...
	unsigned int		slot;               ///<the Slot # to which it belongs. A Slot can have 1 or more attributes (interleaved)
	/// \todo add the Slot pointer ?
	PTR64(void           *pAttributeBufferData);  ///< This is where the data of the vertex-attribute are located. This pointer \b already has \b dataOffsetBytes added

    unsigned int        userData[2]; ///< free user data. You can add information for Attribute <> OpenGL setup, for example

	Attribute() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(Attribute));
      nodeType = NODE132_ATTRIBUTE;
	  version = RAWMESHVERSION132;
    }
  };


  /// 
  /// \brief slot/stream (NODE132_SLOT) : made of interleaved attributes. In the easiest case it has only one attribute.
  ///
  /// \remark a Slot COULD contain a subset of vertices instead of the whole : if the slot
  /// is being used on a smaller part of the mesh, no need to provide all.
  /// In this case, the Slot may contain
  /// - an attribute with index of the vertices (MESH_VERTEXID)
  /// - other attributes containing the values for these vertices index by the first attribute...
  /// Typical use : Blendshapes/Skinning computed locally onto the mesh. Maya/Blender work this way (Blender uses Vertex Groups)
  /// Can be good if we want to compute these modifiers from CPU; from a SPU or From Compute !
  /// TODO for 0x130: TEST THIS !!!!
  ///
  struct Slot : public Node
  {
    unsigned int        vtxBufferSizeBytes;     ///< size of the vertex buffer of this Slot, in bytes
    unsigned int        vtxBufferStrideBytes;   ///< stride in bytes of the vertex buffer

    unsigned int        vertexCount;            ///< amount of vertices in this slot
    int                 userData;               ///< arbitrary integer user-value
    Ptr64<void>         userPtr;                ///< arbitrary pointer for the user

    /// references to used attributes in this Slot. OpenGL setup essentially works with attributes, while DX10/11 works with Slots.
    /// A Slot can have 1 or more attributes. More than 1 attribute means that they are interleaved
	PTR64(AttributePool *pAttributes);
    PTR64(void          *pVtxBufferData); 		///< array of data : attibutes; interleaved if more than 1

	Slot() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(Slot));
      nodeType = NODE132_SLOT;
	  version = RAWMESHVERSION132;
    }
  };

///
/// \brief single Material attribute for arbitrary attributes.
///
/// A material can contain additional attributes on top of the known attributes.
/// Example : Maya allows you to dedine custome properties in a material...
///
struct MaterialAttr
{
	PTR64(const char*	name);
	union {
		float*	pFloat;
		int*	pInt;
		unsigned int* pUInt;
		void *	p;
		struct { int : 32; int : 32; };
	};
	DXGI_FORMAT	type	: 32; ///< DXGI_FORMAT is rather good to have a nice list of possible things
	int					: 32;
};

///
/// \brief Material (NODE132_MATERIAL) containing usual properties
///
struct Material : public Node
{
    /// \name basic material properties that almost all have
    /// @{
	float	diffuse[3];         ///< RGB diffuse color
	float	specexp;            ///< exponent of the specular lighting

	float	ambient[3];         ///< RGB ambient color
	float	reflectivity;       ///< intensity of the reflection

	float	transparency[3];    ///< RGB transparency
    float	translucency;       ///< translucency : 0, no transparency at all; 1 : object 100% transparent (transparency could be used)

	float	specular[3];        ///< specular RGB color
	int		: 32;
    /// @}
	/// \name Shader information : in Maya, this would come from Custom parameters "shader" and "technique"
	/// @{
	PTR64(char*	shaderName);    ///< shader name if exists (most of DCC apps don't have this but we can customize it in Maya as a custom property)
	PTR64(char*	techniqueName); ///< technique name if the shader is an effect.
	/// @}
	/// \brief Texture information
	struct Texture
	{
		PTR64(char*	name);       ///< name of the texture
		PTR64(char*	filename);   ///< filename of the texture. Or whatever helps to find it in whathever storage
		unsigned long userHandle; ///< when the application found the texture, one may want to keep track of resource references with this
		unsigned long userData; ///< when the application found the texture, one may want to keep track of resource references with this
	};
    /// \name textures for various property fields. Optional... would override the RGB/float values if defined
    /// @{
	Texture	diffuseTexture;
	Texture	specExpTexture;
	Texture	ambientTexture;
	Texture	reflectivityTexture;
	Texture	transparencyTexture;
	Texture translucencyTexture;
	Texture	specularTexture;
    /// @}
	/// can be used after load for storing handles... etc. (effect & technique, for example)
	unsigned long userData[2];
	/// Extra parameters : user-defined parameters. For example uniform values for the shader shaderName
	PTR64(MaterialAttrPool*	pAttrPool);
	/// Basic methods
    Material() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(Material));
	  nodeType = NODE132_MATERIAL;
	  version = RAWMESHVERSION132;
    }
};
///
/// \brief Pool of materials
///
struct MaterialPool //: public Node
{
	int			n;
	int			: 32;
	Ptr64<Material>	p[1];
};

/// \brief Primitive group that is in a Mesh
///
/// one primitive group is really related to one \b Drawcall.
///
/// \note PrimGroup isn't a node... so related to a Mesh...
  struct PrimGroup : public Node
  {
    unsigned int                indexCount		: 32; ///< total number of elements. If multi-index: total # of multi-index elements
    unsigned int                indexOffset		: 32; ///< offset: where to start in the index table. interesting when sharing the same index buffer

	unsigned int                indexArrayByteOffset : 32;  ///< offset to reach the element array from &data[1]
    unsigned int                indexArrayByteSize	 : 32;  ///< total size in bytes

	unsigned int                primitiveCount	: 32;   ///< DX9 wants it
	unsigned int                indexPerVertex	: 32;   ///< usually 1. If >1 then we are in the case of multi-index mode...

    /// \name index formats for various API's
    /// @{
        D3DFORMAT                   indexFormatDX9	: 32;   ///< index format for DX9
        DXGI_FORMAT                 indexFormatDXGI : 32;   ///< index format for DX10

	    GLType                      indexFormatGL	: 32;   ///< index format for openGL
    /// @}
    /// \name topology (primitive type) for various API's
    /// @{
        D3DPRIMITIVETYPE            topologyDX9		: 32;

        /// Note: the DX11 topology can be used in OpenGL to find back the batch vertices. Good enough
	    union {
        D3D10_PRIMITIVE_TOPOLOGY    topologyDX10	: 32;
        D3D11_PRIMITIVE_TOPOLOGY    topologyDX11	: 32; ///< just extends DX10...
        OGL_PATCH_VERTICES           glPatchVertices : 32; ///< tells the # of vertices when topo is GL_PATCHES
	    };
        GLTopology                  topologyGL		: 32;
    ///@}
    PTR64(void                      *userPtr);            ///< a user pointer to store whatever additional data after load

    /// it is possible that the Index buffer doesn't belong to this structure, but references one from another (so they share it).
    /// useful when creating a OGL/DX buffer : to avoid redundancy
	PTR64(PrimGroup             *pOwnerOfIB);
    /// pIndexBufferData can be shared with other groups (if pOwnerOfIB points to another PrimGroup).
    /// this pointer directly points to the right place (no need to add indexOffset )
	PTR64(void *                pIndexBufferData);
	PTR64(Material              *pMaterial);        ///< Material in used for drawing this group
    /// Transforms used for this drawcall.
    /// it can be NULL is no need. It can have many for the skinning case
	PTR64(TransformRefs         *pTransforms);

    unsigned int                minIndex		: 32; ///< min element index (for OpenGL)
	unsigned int                maxIndex		: 32; ///< max element index (for OpenGL)
	unsigned int                primRestartIndex: 32; ///< primitive Restart Index (added in v130)
    BSphere                     bsphere;              ///< 3+1 floats
    AABBox                      aabbox;				  ///< 3*2 floats
	int : 32; int : 32; ///< to adjust AABox

	PrimGroup() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(PrimGroup));
	  nodeType = NODE132_PRIMGROUP;
	  version = RAWMESHVERSION132;
    }
  };
  //------------------------------------------------------------------------------------------
  /** \brief Mesh Node.
  
  In the case of instancing : many solutions can be taken
  - Use the same Mesh and aggregate transformations as explained below
  - create a brand new Mesh object *but* share the data asset
   - However we can duplicate what is different between meshes

     for example the transformation references could be totally different from one Mesh instance to another

     another example : skinning weight could be different for one instance... leading to another attribute table for skinning (Slot)
  \todo it is a known fact that rendering same Primitive groups of all instances is better than rendering Meshes one after another
  The question is how to do this...
  - Ok for the case where we aggregated all transforms in the Mesh
  - No Ok for the case where we duplicated the Mesh struct for each instance...
  \note In this design, the skeleton doesn't belong to the Mesh. The skeleton is really part of the transform tree...
  **/
  struct Mesh : public Node
  {
    //
    // SIMPLE ITEMS
    //
	PTR64(SlotPool      *pSlots);          					///< or called STREAM. Infos + Vertex Buffers for each slot/layer/stream
	PTR64(PrimGroupPool *pPrimGroups);       				///< primitive group structures is always at the begining of the data chunk.
    //
    // ADDITIONAL ITEMS
    //
	PTR64(AttributePool *pAttributes); ///<all attributes of all Slots together
	PTR64(AttributePool *pBSAttributes); ///<all Blendshapes attributes of all Slots together

    PTR64(SlotPool      *pBSSlots);             			///< to be resolved with blendShapesOffset+rawdata
    /// Transform offsets : the ones the mesh is using (many refs for the skinning case, 0 or 1 for the basic case)
    /// Note: if many transforms are regular transforms (not tagged as Bone), then it can mean that you have many instances of this mesh to render
    /// if the list is Regular-Transf + Bones-Transf's...; Regular-Transform + Bones transforms... : then we have many instances with different skeletons
    PTR64(TransformRefs *pTransforms);

	PTR64(FloatArrayPool *pFloatArrays);			///< array of float coming from curves or anything else derived from FloatArray. For Blendshape animation... for example
	PTR64(void           *userPtr);                 ///< arbitrary data could be pointed by this guy
    BSphere               bsphere;              			///< for the whole mesh, all primgroup included
    AABBox                aabbox;
	int                   numJointInfluence;     			///< # of weights for skinning. Weights are passed through attributes. For now only one attrib can be used (4 weights)
	int : 32; // alignment...
    //
    // Methods
    //
    Mesh() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(Mesh));
      nodeType = NODE132_MESH;
	  version = RAWMESHVERSION132;
    }
    void resolvePointers();
    void debugDumpLayout(unsigned int &nVtx, unsigned int &nElts, unsigned int &nPrims, bool brief, const char * nodeNameFilter);
  };

  ///
  /// \brief Pointer relocation table
  ///
  /// When stored as a file, pointers are turned to offsets. later, we use this relocation table to compute pointers.
  ///
  struct RelocationTable : public Node
  {
	long			numRelocationOffsets;      ///< data telling where to resolve pointers
	int				: 32;
    /// in the case of file mapping (mmap or windows equivalent), we may change values in the
    /// file at ptr locations and thus lost the offsets. Which is why we should keep them here, too
    struct Offsets {
        unsigned long ptrOffset; ///< offset where is located the ptr
        unsigned long offset; ///< offset used to recompute the ptr
    };
	PTR64(Offsets     *pRelocationOffsets);      ///< data telling where to resolve pointers

    RelocationTable() {init();}
	void init()
    {
      memset((void*)this, 0, sizeof(RelocationTable));
      nodeType = NODE132_RELOCTABLE;
	  version = RAWMESHVERSION132;
    }
  };
  //------------------------------------------------------------------------------------------
  /// \brief Header (NODE132_HEADER) of the Bk3d file. This where the main data are accessible
  ///
  /// \remark Keep the order of items if adding more : so we keep compatibility
  /// 
  //------------------------------------------------------------------------------------------
  struct FileHeader : public Node
  {
	// removed these 2 fields : version is now in the Node and magic is off...
    //unsigned int  magic;                // magic unsigned int  "MESH". Just to make sure this is a correct file. \todo version checking ?
    //unsigned int  version;              // version of this file

	PTR64(MeshPool      *pMeshes);               ///< first Mesh Node
    //
    // ADDITIONAL INFO
    //
	PTR64(TransformPool   *pTransforms);      ///< to be resolved at load time. Contains 0 to N transform offsets
	PTR64(MayaCurvePool	*pMayaCurves);
	PTR64(MaterialPool	*pMaterials);
    PTR64(IKHandlePool *pIKHandles);
	PTR64(RelocationTable *pRelocationTable);
    // TODO:
    // Cameras    *pCameras;
    // Modifiers  *pModifiers; // Lattice...

    //----------------------------
                  FileHeader();
    void          init();
    float*        findComponentf(const char *compname, bool **pDirty);
    void          resolvePointers();
    void          debugDumpAll(bool brief=false, const char * nodeNameFilter = NULL);
  };
} //namespace bk3d

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
//// METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS METHODS
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#define _CRT_SECURE_NO_WARNINGS
//
// ptr of the file contain offsets, relative to the class it belongs to
// 
#define RESOLVEPTR132(pstruct, ptr, type) ptr = (ptr ? (type*)(((char*)pstruct) + (size_t)(ptr)) : NULL)

//
// Unicode and formatting is a bit tricky...
// 
#ifdef  UNICODE
#   define FSTR L"%S"
#	ifndef PRINTF
//#		pragma message("PRINTF undefined, defining it now")
#		define PRINTF(a) wprintf a
#	endif
#	ifndef EPRINTF
//#		pragma message("EPRINTF undefined, defining it now")
#		define EPRINTF(a) wprintf a
#	endif
#   ifndef TEXT
#       define TEXT(t) L t
#   endif
#   ifndef _WINNT_
        typedef const TCHAR * LPCSTR;
#   endif
#else
#   define FSTR "%s"
#	ifndef PRINTF
//#		pragma message("PRINTF undefined, defining it now")
#		define PRINTF(a) printf a
#	endif
#	ifndef EPRINTF
//#		pragma message("EPRINTF undefined, defining it now")
#		define EPRINTF(a) printf a
#	endif
#   ifndef TEXT
#       define TEXT(t) t
#   endif
#   ifndef LPCSTR
#		ifdef BK3DVERBOSE
//#		pragma message("!!! Defining our own LPCSTR !!!")
#		endif
        //typedef const char * LPCSTR;
#   endif
#endif

//
// Optional use of ZLIB
// 
#ifndef GFILE
#ifndef NOGZLIB
#pragma message("Using zlib to read bk3d models...")
#include "zlib/zlib.h"
#define GFILE gzFile
#define GOPEN gzopen
#define GREAD(a,b,c) gzread(a, b, (unsigned int)c)
#define GCLOSE gzclose
#else
#define GFILE FILE *
#define GOPEN fopen
#define GREAD(a,b,c) (int)fread(b, 1, c, a)
#define GCLOSE fclose
#endif
#endif

namespace bk3d132
{
//==========================================================================================
// 
// Transform methods
// 
//==========================================================================================

///
/// \brief dump the layout of a transform object
///
INLINE void Transform::debugDumpLayout(int l, bool brief, const char * nodeNameFilter)
{
	if(pParent && (l==0)) // if we have a parent : let's not display it when at the level 0
		return;
      if(nodeNameFilter && (strcmp(nodeNameFilter, name)))
          goto passThePrint;
	for(int i=0	; i<l; i++) PRINTF(( TEXT("\t") ));	
	PRINTF(( TEXT("TRANSFORM node : ") FSTR TEXT(" (parent: ") FSTR TEXT(" )\n"), name, pParent ? pParent->name : "None"));
    if(!brief)
    {
	    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	    PRINTF((TEXT("| pos : %f %f %f\n"), pos[0], pos[1], pos[2] ));
	    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	    PRINTF((TEXT("| scale : %f %f %f\n"), scale[0], scale[1], scale[2] ));
	    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
	    PRINTF((TEXT("| Rot : %f %f %f\n"), rotation[0], rotation[1], rotation[2]));
    		
	    if(pFloatArrays)
	    {
		    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
		    PRINTF(( TEXT("| sources connected to the transform: \n") ));
		    for(int j=0; j<pFloatArrays->n; j++)
		    {
			    for(int i=0; i<l; i++) PRINTF(( TEXT("\t") ));
			    PRINTF(( TEXT("| ") FSTR TEXT(" <== Curve ") FSTR TEXT("\n"), pFloatArrays->p[j].destName, pFloatArrays->p[j].p ? pFloatArrays->p[j].p->name : "NO SOURCE" ));
		    }
		    PRINTF(( TEXT("\n") ));
	    }
    }
passThePrint:
	if(pChildren)
	  for(int i=0; i<pChildren->n; i++)
		pChildren->p[i]->debugDumpLayout(l+1, brief, nodeNameFilter); // recusive display for children...
}
 
//==========================================================================================
// 
// FileHeader methods
// 
//==========================================================================================
INLINE FileHeader::FileHeader() 
{
	init();
}

INLINE void FileHeader::init()
{
  version = RAWMESHVERSION132;
  nodeByteSize = sizeof(FileHeader);
  pMeshes = NULL;
  pTransforms = NULL;
  nextNode = NULL;
  nodeType = NODE132_HEADER;
  nodeByteSize = sizeof(FileHeader);
}
///
/// Helper to find some components.
/// \todo : give more details
///
INLINE float* FileHeader::findComponentf(const char *compname, bool **pDirty)
{
    float *pComp = NULL;
    if(!compname)
        return NULL;
    char *name, *comp = NULL;
    int l = (int)strlen(compname)+1;
    name = new char[l];
#if     _MSC_VER > 1310
    strcpy_s(name, l, compname);
#else
    strcpy(name, compname);
#endif
    for(int i=l-1; i>=0; i--) if(name[i] == '_') 
    { 
        name[i] = '\0';
        comp = name + i + 1;
        break;
    }
    if(!comp) {
        delete [] name;
        return NULL;
    }
    //search in transforms
    for(int i=0; i<pTransforms->n; i++)
    {
        Transform *pt = pTransforms->p[i];
        if(strcmp(pt->name, name))
            continue;
        if(pDirty) *pDirty = &(pt->bDirty);
        if(!strcmp(comp, "translate")) {        
            pComp = pt->pos;
        } else if(!strcmp(comp, "scale")) {     
            pComp = pt->scale;
        } else if(!strcmp(comp, "rotation")) {  
            pComp = pt->rotation;
        } //some more to add...
        break;
    }
    //search in Mesh ? (TODO later)
    //...
    delete [] name;
    return pComp;
}
//------------------------------------------------------------------------------------------
// 
/// global resolution of pointers : this function uses RelocationTable to resolve pointers
// 
//------------------------------------------------------------------------------------------
INLINE void FileHeader::resolvePointers()
{
	RESOLVEPTR132(this, pRelocationTable, RelocationTable); // write the correct pointer now we are in memory
    RESOLVEPTR132(this, pRelocationTable->pRelocationOffsets, RelocationTable::Offsets); // write the correct pointer now we are in memory
	for(int i=0; i < pRelocationTable->numRelocationOffsets; i++)
	{
		char* ptr = (char*)this;
        unsigned long offs = pRelocationTable->pRelocationOffsets[i].ptrOffset;
		if(offs == 0)
			continue;
		ptr += offs;
		unsigned long *ptr2 = (unsigned long *)ptr;
		if(*ptr2)
			*ptr2 = (unsigned long)(((char*)this) + pRelocationTable->pRelocationOffsets[i].offset);
	}
}
//------------------------------------------------------------------------------------------
// 
/// Debug dumps all what is inside the Bk3d file
// 
//------------------------------------------------------------------------------------------
INLINE void FileHeader::debugDumpAll(bool brief, const char * nodeNameFilter)
{
	if(pTransforms) 
		for(int i=0; i< pTransforms->n; i++)
		{
			pTransforms->p[i]->debugDumpLayout(0, brief, nodeNameFilter);
		}
	PRINTF((TEXT("\n")));
	if(pMaterials)
		for(int i=0; i< pMaterials->n; i++)
		{
            if(nodeNameFilter && (strcmp(nodeNameFilter, name)))
              continue;
			Material *pM = pMaterials->p[i];
			PRINTF((TEXT("Material node : ") FSTR TEXT("\n"), pM->name));
            if(!brief)
            {
                PRINTF((TEXT("\tdiffuse : %f,%f,%f\n"), pM->diffuse[0], pM->diffuse[1], pM->diffuse[2] ));
                PRINTF((TEXT("\tspecexp : %f\n"), pM->specexp ));
                PRINTF((TEXT("\tambient : %f,%f,%f\n"), pM->ambient[0], pM->ambient[1], pM->ambient[2] ));
                PRINTF((TEXT("\treflectivity : %f\n"), pM->reflectivity ));
                PRINTF((TEXT("\ttransparency : %f,%f,%f\n"), pM->transparency[0], pM->transparency[1], pM->transparency[2] ));
                PRINTF((TEXT("\ttranslucency : %f\n"), pM->translucency ));
                PRINTF((TEXT("\tspecular : %f,%f,%f\n"), pM->specular[0], pM->specular[1], pM->specular[2] ));
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

	if(pMayaCurves)
		for(int i=0; i< pMayaCurves->n; i++)
		{
			Ptr64<MayaCurveVector> p = pMayaCurves->p[i];
			PRINTF((TEXT("Maya Curve vector : ") FSTR 
				TEXT("%d curves \n"), p->name, p->nCurves));
		}

	unsigned int nVtx = 0;
	unsigned int nElts = 0;
	unsigned int nPrims = 0;
	if(pMeshes)
		for(int i=0; i< pMeshes->n; i++)
		{
			pMeshes->p[i]->debugDumpLayout(nVtx, nElts, nPrims, brief, nodeNameFilter);
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
INLINE void Mesh::debugDumpLayout(unsigned int &nVtx, unsigned int &nElts, unsigned int &nPrims, bool brief, const char * nodeNameFilter)
{
  if(nodeNameFilter && (strcmp(nodeNameFilter, name)))
      return;
  if(brief)
  {
    PRINTF((TEXT("Mesh : ") FSTR, name));
    if(pTransforms)
    {
        PRINTF((TEXT(" ,Transf: ")));
        for(int i=0; i<pTransforms->n; i++)
        {
            PRINTF((TEXT(" ") FSTR, pTransforms->p[i]->name ));
        }
    }
    PRINTF((TEXT("\n")));
    if(pPrimGroups)
    {
        PrimGroup *pCurPrimGroup = pPrimGroups->p[0];
        for(int i=0; i<pPrimGroups->n; i++, pCurPrimGroup = pPrimGroups->p[i])
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
    PRINTF((TEXT("\n-------------------------------------\nMesh Infos : ") FSTR TEXT("\n\n"), name));
  if(pTransforms)
  {
	PRINTF((TEXT(" Attached to Transform(s) ( many transforms == use of skinning)\n") ));
	for(int i=0; i<pTransforms->n; i++)
	{
	  PRINTF((TEXT(" ") FSTR, pTransforms->p[i]->name ));
	}
  }
  if(pAttributes && pAttributes->n > 0)
  {
    PRINTF((TEXT("\n\n%d Attributes: \n"), pAttributes->n));
    Attribute *pCurAttr = pAttributes->p[0].p;
    for(int j=0; j<pAttributes->n; j++, pCurAttr = pAttributes->p[j].p)
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
  if(pSlots && pSlots->n > 0)
  {
    PRINTF((TEXT("\n%d Slots (Streams) : \n"), pSlots->n));
    Slot *pCurSlot = pSlots->p[0];
	nVtx += pCurSlot->vertexCount;
    for(int j=0; j<pSlots->n; j++, pCurSlot = pSlots->p[j])
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
			//float* pF = (float*)pCurAttr->pAttributeBufferData;
			//for(int j=0; j<100; j++)
			//{
			//	for(int k=0; k<pCurAttr->numComp; k++)
			//	{
			//	  PRINTF((TEXT("%f "), pF[k] ));
			//	}
			//	pF += pCurAttr->strideBytes / 4;
			//}
			//PRINTF((TEXT("...\n") ));
        }
		//float* pF = (float*)pCurSlot->pVtxBufferData;
		//for(int i=0; i<300; i++)
		//{
		//  PRINTF((TEXT("%f "), *pF++ ));
		//}
		//PRINTF((FSTR TEXT("...\n") ));
    }
  }
  if(pBSSlots && pBSSlots->n > 0)
  {
    PRINTF((TEXT("\n%d numBlendShapes (==Slots) : \n"), pBSSlots->n));
    Slot *pCurBSSlot = pBSSlots->p[0];
    for(int j=0; j<pBSSlots->n; j++, pCurBSSlot = pBSSlots->p[j])
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
 if(pPrimGroups)
 {
	 PrimGroup *pCurPrimGroup = pPrimGroups->p[0];
	 for(int i=0; i<pPrimGroups->n; i++, pCurPrimGroup = pPrimGroups->p[i])
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
		}
  }
  PRINTF((TEXT("\nBounding Sphere : p=(%f, %f %f), R= %f\n"),
    bsphere.pos[0], bsphere.pos[1], bsphere.pos[2], bsphere.radius));
  PRINTF((TEXT("AABB : min=(%f, %f %f), max=(%f, %f %f)\n"),
    aabbox.min[0], aabbox.min[1], aabbox.min[2], aabbox.max[0], aabbox.max[1], aabbox.max[2]));

  PRINTF((TEXT("\n--------------------------------\n")));
}
//--------------------------------
// 
/// LOAD function
/// 
/// Returns the baked structure of all the data you need to work
// 
//--------------------------------
INLINE static FileHeader * load(const char * fname)
{
#   define RAWMESHMINSZ (1024*1000)
	size_t size = RAWMESHMINSZ;
    GFILE fd = NULL;
    if(!fname)
        return NULL;
#ifndef NOGZLIB // pass to get the size of the file when using GZip fmt :-(
	FILE *fd2 = fopen(fname, "rb");
	if(!fd2)
    {
        PRINTF((TEXT("Error>> couldn't load ") FSTR TEXT("\n"), fname));
        return NULL;
    }
	fseek(fd2, 0, SEEK_END);
	fpos_t pos;
	fgetpos( fd2, &pos );
	size = 3*pos+1; //let's assume in most of the case compression is 3x
    fclose(fd2);
#endif
    fd = GOPEN(fname, "rb");
    if(!fd)
    {
      EPRINTF((TEXT("Error : couldn't load ") FSTR TEXT("\n"), fname));
        return NULL;
    }
#ifdef NOGZLIB
	fseek(fd, 0, SEEK_END);
	fpos_t pos;
	fgetpos( fd, &pos );
	rewind(fd);
	size = (size_t)pos+1;
#endif
    char * memory = (char*)malloc(size + sizeof(Node));
    size_t offs = 0;
    int n = 0;
    do {
      if(n > 0)
      {
	      offs += size;
	      memory = (char*)realloc(memory, size + offs);
      }
      if(fd)
        n= GREAD(fd, memory + offs, size);
    } while(n == size);
	memset(memory + offs + n, 0, sizeof(Node));
    /*if(n > 0)
    {
      offs -= RAWMESHMINSZ-n-4;
      memory = (char*)realloc(memory, RAWMESHMINSZ + offs);
    }*/
    if(fd)
        GCLOSE(fd);
    //if(strncmp((char*)&((FileHeader *)memory)->magic, "MESH", 4))
    //{
    //  EPRINTF((TEXT("Error : Not a mesh file\n")));
	   // free(memory);
	   // return false;
    //}
    if(((FileHeader *)memory)->version != RAWMESHVERSION132)
    {
      PRINTF((TEXT("Error>> Wrong version in Mesh description\n")));
      PRINTF((TEXT("needed %x and got %x\n"), RAWMESHVERSION132, ((FileHeader *)memory)->version));
      free(memory);
      return NULL;
    }
    ((FileHeader *)memory)->resolvePointers();
    //PRINTF((TEXT("Loaded ") FSTR TEXT(" (mesh version %x)\n"), fname, ((FileHeader *)memory)->version));
    return (FileHeader *)memory;
}

} //namespace bk3d



