// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PYBK3D_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PYBK3D_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PYBK3D_EXPORTS
#define PYBK3D_API __declspec(dllexport)
#else
#define PYBK3D_API __declspec(dllimport)
#endif

extern "C"
void initbk3d();

using namespace bk3dlib;

namespace bk3dHeader
{
	struct PyObj
	{
		PyObject_HEAD
		FileHeader	*p;
	};
	extern PyTypeObject classType;
	extern char doc [];
}

namespace bk3dMesh
{
	struct PyObj
	{
		PyObject_HEAD
		PMesh	p;
	};
	extern PyTypeObject classType;
	extern char doc [];
}
namespace bk3dBuffer
{
	struct PyObj
	{
		PyObject_HEAD
		PBuffer	p;
	};
	extern PyTypeObject classType;
	extern char doc [];
}

namespace bk3dTransform
{
	struct PyObj
	{
		PyObject_HEAD
		PTransform	p;
	};
	extern PyTypeObject classType;
	extern char doc [];
}

namespace bk3dMaterial
{
	struct PyObj
	{
		PyObject_HEAD
		PMaterial	p;
	};
	extern PyTypeObject classType;
	extern char doc [];
}

