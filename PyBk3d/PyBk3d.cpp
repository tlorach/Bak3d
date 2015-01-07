#include "stdafx.h"
#include "PyBk3d.h"
// PyBk3d.cpp : Defines the exported functions for the DLL application.
//

#if 0
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Class for Enum Topology
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dTopology
{
	// --------------------------------------------------
	//
	PyObject *getattr(PyObject *self, char *name)
	{
		if(!strcmp(name, "points"))
			return PyLI_FromLong(Topology::POINTS);
		Py_IncRef(Py_None);
		return Py_None;
	}
	int setattr(PyObject *self, char *name, PyObject *srcobj)
	{
		return 0;
	}
	PyObject *getattro(PyObject *self, PyObject *name)
	{
		Py_IncRef(Py_None);
		return Py_None;
	}
	int setattro(PyObject *self, PyObject *name, PyObject *srcobj)
	{
		return 0;
	}
	// --------------------------------------------------
	//
	PyObject *tp_repr(PyObject *self)
	{
		return PyString_FromFormat("{Bk3d Toplogy Enum}");
	}

	// --------------------------------------------------
	//
	static char doc [] =
	"Enum for Topology...\n";

	// --------------------------------------------------
	//
	// bk3d Header Type
	// http://docs.python.org/extending/newtypes.html?highlight=pytypeobject
	//
	PyTypeObject classType = 
	{
#ifdef IS_PY3K
		PyVarObject_HEAD_INIT(&PyType_Type, 0)
#else
		PyObject_HEAD_INIT(&PyType_Type)
		0,
#endif
		"Topology",
		sizeof(PyObject), // taille de l'objet
		0,	// taille de l'Item ?
		0,	//tp_dealloc
		0,//tp_print,		// PyObject_Print - Avoid : it may prevent tp_str / repr to be used
		getattr,	// tp_getattr - WARNING : cancels tp_methods
		setattr,				// tp_setattr
		0,		// tp_compare : comparaison
		tp_repr,		// PyObject_Repr//tp_repr : utilisé par les funcs repr() et PyObject_Repr()
		0,				// tp_as_number : opérations typiques (+ - * etc)
		0,		// tp_as_sequence : concatenations etc.
		0,				// tp_as_mapping : pour les accès par indice (tableaux)
		0, 				// tp_hash : A VOIR...
		0,				// tp_call : appel de l'objet en tant que fonction ( a= obj() )
		0,			// tp_str : like tp_repr. More fore human reading. If not used, tp_repr used instead...
		getattro,				// tp_getattro : version avec un objet au lieu d'un char*...
		setattro,				// tp_setattro
		0,				// tp_as_buffer
		0,				// tp_xxx4
		doc,			// tp_doc
	};//PyTypeObject
}//namespace bk3dTopology
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
//
// General Methods
//
//////////////////////////////////////////////////////////////////////////////////////////////
static PyObject *createHeader(PyObject *self, PyObject *args)
{
	bk3dHeader::PyObj *obj;
	obj = PyObject_New(bk3dHeader::PyObj, &bk3dHeader::classType);
	obj->p = NULL;//todo
	return (PyObject *)obj;
}
static PyObject *createFromBk3dFile(PyObject *self, PyObject *args)
{
	return NULL;
}

static struct PyMethodDef bk3d_methods[] = {
	{"createHeader", createHeader, 1},
	{"createFromBk3dFile", createFromBk3dFile, 1},
	{NULL, NULL}
};

static char * doc = 
"bk3d baked-file creation module\n"
"-------------------------------\n"
"Available class objects (help() to get more on them):\n"
" HeaderType()\n"
" MeshType()\n"
" TransformType()\n"
" MaterialType()\n"
" BufferType()\n"
" (CurveVecType to come)\n\n"
"The basic use case is :\n\n"
"create a header (h = HeaderType() )\n"
"create meshes (m = MeshType() )\n"
"  add them to the header (h.addMesh(m) )\n"
"create Materials ( mat = MaterialType() )\n"
"  add them to the header ( h.addMaterial() )\n"
"create buffers ( b = BufferType() )\n"
"  - some are for index\n"
"  - some are for vertex attributes\n"
"  - some are for Blendshape vertex attributes\n"
"  - feed them with data ( addData(...) )\n"
"  add them to a mesh ( addIdxBuffer(...) or addVtxBuffer(...) )\n"
"create primitive groups in meshes (createPrimGroup)\n"
"  - this is where you can reference a material to a group ()\n"
"compute mesh bounding box...\n"
"create Transformations (t = TransformType() )\n"
"  - tranformations can be connected (parent/children)\n"
"  - reference transforms to meshes (m.addTransformReference(...)\n"
"    many transform references in a mesh : for skinning"
"\n"
"Then cook the header ( h.cook(cacheName) )\n"
"save the data : h.save(name)\n"
"\n"
"-------------------------------\n"
"Object Classes :\n\n"
;
//////////////////////////////////////////////////////////////////////////////////////////////
//
// INIT
// http://docs.python.org/extending/extending.html
//
//////////////////////////////////////////////////////////////////////////////////////////////

#ifdef IS_PY3K
PyMODINIT_FUNC PyInit_bk3d(void)
#else
extern "C" void initbk3d()
#endif
{
	PyObject *m;

    if (PyType_Ready(&bk3dHeader::classType) < 0)
        return NULL;
    if (PyType_Ready(&bk3dMesh::classType) < 0)
        return NULL;

	char * tempDoc =  (char*)malloc(
		 strlen(doc)+1
		+strlen(bk3dHeader::doc)+1
		+strlen(bk3dMesh::doc)+1
		+strlen(bk3dMaterial::doc)+1
		+strlen(bk3dBuffer::doc)+1
		+strlen(bk3dTransform::doc)+1);
		//TODO: curves...
	strcpy(tempDoc, doc);
	strcat(tempDoc, bk3dHeader::doc);
	strcat(tempDoc, bk3dMesh::doc);
	strcat(tempDoc, bk3dMaterial::doc);
	strcat(tempDoc, bk3dBuffer::doc);
	strcat(tempDoc, bk3dTransform::doc);
	//TODO: curves...

#ifdef IS_PY3K
    static struct PyModuleDef bk3dModule = {
        PyModuleDef_HEAD_INIT,
        "bk3d",   /* name of module */
        doc, /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
                    or -1 if the module keeps state in global variables. */
        bk3d_methods,
        NULL,
        NULL,
        NULL,
        NULL
    };
    m = PyModule_Create(&bk3dModule);
#else
    m = Py_InitModule3("bk3d", bk3d_methods, tempDoc);
#endif
	free(tempDoc);
#if 0
	PyRun_SimpleString(
		"class Topology:\n"
		"	POINTS=0\n"
		"	LINES=1\n"
		"	LINE_LOOP=2\n"
		"	LINE_STRIP=3\n"
		"	TRIANGLES=4\n"
		"	TRIANGLE_STRIP=5\n"
		"	TRIANGLE_FAN=6\n"
		"	QUADS=7\n"
		"	QUAD_STRIP=8\n"
		"	FAN=9\n"
		"	LINES_ADJ=10\n"
		"	LINE_STRIP_ADJ=11\n"
		"	TRIANGLES_ADJ=12\n"
		"	TRIANGLE_STRIP_ADJ=13\n"
		);

	PyRun_SimpleString(
		"class DataType:\n"
		"	FLOAT32=0\n"
		"	FLOAT16=1\n"
		"	UINT32=2\n"
		"	UINT16=3\n"
		"	UINT8=4\n"
		"	UNKNOWN=5\n"
		);

	PyRun_SimpleString(
		"class Attribute:\n"
		"	POSITION =        \"position\"\n"
		"	VERTEXID =        \"vertexid\"\n"
		"	COLOR =           \"color\"\n"
		"	NORMAL =          \"normal\"\n"
		"	TANGENT =         \"tangent\"\n"
		"	BINORMAL =        \"binormal\"\n"
		"	VTXNORMAL =       \"vtxnormal\"\n"
		"	TEXCOORD0 =       \"texcoord0\"\n"
		"	TEXCOORD1 =       \"texcoord1\"\n"
		"	TEXCOORD2 =       \"texcoord2\"\n"
		"	TEXCOORD3 =       \"texcoord3\"\n"
		"	BLIND0 =          \"blind0\"\n"
		"	BLIND1 =          \"blind1\"\n"
		"	BLIND2 =          \"blind2\"\n"
		"	BLIND3 =          \"blind3\"\n"
		"	BONESOFFSETS =    \"bonesoffsets\"\n"
		"	BONESWEIGHTS =    \"bonesweights\"\n"
		);
#endif
    //Py_INCREF(&bk3dTopology::classType);
    //PyModule_AddObject(m, "Topology", (PyObject *)&bk3dTopology::classType);

    Py_INCREF(&bk3dHeader::classType);
    PyModule_AddObject(m, "HeaderType", (PyObject *)&bk3dHeader::classType);

	Py_INCREF(&bk3dMesh::classType);
    PyModule_AddObject(m, "MeshType", (PyObject *)&bk3dMesh::classType);

	Py_INCREF(&bk3dBuffer::classType);
    PyModule_AddObject(m, "BufferType", (PyObject *)&bk3dBuffer::classType);

	Py_INCREF(&bk3dMaterial::classType);
    PyModule_AddObject(m, "MaterialType", (PyObject *)&bk3dMaterial::classType);

	Py_INCREF(&bk3dTransform::classType);
    PyModule_AddObject(m, "TransformType", (PyObject *)&bk3dTransform::classType);

	//if(PyErr_Occurred())
	//	Py_FatalError("Can't initialize bk3d");
    return m;
}

