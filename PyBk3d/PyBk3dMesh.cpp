#include "stdafx.h"
#include "PyBk3d.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dMesh
{
	// --------------------------------------------------
	//
	static PyObject *getName(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
		LPCSTR name = p->p->GetName();
		PyObject *o = PySTRING_FromString(name);
		return o;
	}

	// --------------------------------------------------
	//
	static PyObject *setName(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		LPCSTR pName = NULL;
		PyArg_ParseTuple(args, "s", &pName);
		if(pName)
			p->p->SetName(pName);
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addVtxBuffer(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		LPCSTR pName = NULL;
		//_asm { int 3 };
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i+=2)
		{
			PyObject *po = PyTuple_GetItem(args, 2*i);
			if(po && (PyObject_TypeCheck(po, &bk3dBuffer::classType)))
			{
				int isBlendShape = 0;
				if(i+1 < sz)
				{
					PyObject *poBool = PyTuple_GetItem(args, 2*i+1);
					PyArg_Parse(poBool, "i", &isBlendShape);
				}
				bk3dBuffer::PyObj *pB = (bk3dBuffer::PyObj *)po;
				if(!p->p->AttachVtxBuffer(pB->p, isBlendShape?true:false))
				{
					PyErr_SetString(PyExc_AttributeError, "AddVtxBuffer() issue. Buffer of same name already attached to the Mesh ?");
					return NULL;
				}
				Py_INCREF(pB);
			}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addIdxBuffer(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		LPCSTR pName = NULL;
		//_asm { int 3 };
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i+=2)
		{
			PyObject *po = PyTuple_GetItem(args, 2*i);
			if(po && (PyObject_TypeCheck(po, &bk3dBuffer::classType)))
			{
				int primGroup = -1;
				if(i+1<sz)
				{
					PyObject *poPG = PyTuple_GetItem(args, 2*i+1);
					PyArg_Parse(poPG, "i", &primGroup);
				}
				bk3dBuffer::PyObj *pB = (bk3dBuffer::PyObj *)po;
				if(p->p->AttachIndexBuffer(pB->p, primGroup) >= 0) // if not -1, means that we added to a primitive group, too
          Py_INCREF(pB); // so incremembt the reference for the prim group
				Py_INCREF(pB);
			}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}
	//virtual int			CreatePrimGroup
	PyObject* createPrimGroup(PyObject *self, PyObject *args)
	{
		LPCSTR name;
		PyObject *idxBuffer = NULL;
		bk3dlib::Topology topo = bk3dlib::POINTS;
		PyObject *pMat = NULL; 
		int offsetElement=0, numElements=0;
		PyObj *p = (PyObj *)self;
		//_asm { int 3 };
		int n = PyArg_ParseTuple(args, "sOi|Oii", &name, &idxBuffer, &topo, &pMat, &offsetElement, &numElements);
		if(!PyObject_TypeCheck(idxBuffer, &bk3dBuffer::classType))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad index Buffer");
			return NULL;
		}
		if(pMat &&(!PyObject_TypeCheck(pMat, &bk3dMaterial::classType)))
		{
			//PyErr_SetString(PyExc_AttributeError, "Bad material");
			//return NULL;
      pMat = NULL;
		}
		bk3dBuffer::PyObj *bufobj = (bk3dBuffer::PyObj *)idxBuffer;
		bk3dMaterial::PyObj *matobj = pMat ? (bk3dMaterial::PyObj *)pMat : NULL;
    // For the reference counting trick, we must first check if this idx buffer is already there...
    int Nidx = p->p->GetNumIdxBuffers();
    int i;
    for(i=0; i<Nidx; i++)
      if(p->p->GetIdxBuffer(i) == bufobj->p)
        break; // already there
		int res = p->p->CreatePrimGroup(name, bufobj->p, topo, matobj ? matobj->p : NULL, offsetElement, numElements);
		if(res) {
      if(i == Nidx) // if we did not find this buffer before adding the primitive group, we need one more reference
			  Py_IncRef(idxBuffer);
			Py_IncRef(idxBuffer);
			if(pMat) 
				Py_IncRef(pMat);
		}
		return PyLI_FromLong(res);
	}
	PyObject* detachBuffer(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		bk3dBuffer::PyObj *bufobj;
		//_asm { int 3 };
		if((PyArg_ParseTuple(args, "O", &bufobj) == 0)||(!PyObject_TypeCheck(bufobj, &bk3dBuffer::classType)))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad parameter Buffer");
			return NULL;
		}
		p->p->DetachBuffer(bufobj->p);
		Py_IncRef(Py_None);
		return Py_None;
	}
	PyObject* computeBoundingVolumes(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		bk3dBuffer::PyObj *bufobj;
		//_asm { int 3 };
		if((PyArg_ParseTuple(args, "O", &bufobj) == 0)||(!PyObject_TypeCheck(bufobj, &bk3dBuffer::classType)))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad parameter Buffer");
			return NULL;
		}
		p->p->ComputeBoundingVolumes(bufobj->p);
		Py_IncRef(Py_None);
		return Py_None;
	}
	PyObject* addTransformReference(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		bk3dTransform::PyObj *transfobj = NULL;
		int primgroup = -1;
		if((PyArg_ParseTuple(args, "O|i", &transfobj, &primgroup) == 0)||(!PyObject_TypeCheck(transfobj, &bk3dTransform::classType)))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad Arguments : should have Transfomr obj and primgroup #");
			return NULL;
		}
		if(!p->p->AddTransformReference(transfobj->p, primgroup))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad Arguments : failed to add the Transform. Check primgroup index ?");
			return NULL;
		}
		Py_IncRef((PyObject*)transfobj);
		Py_IncRef(Py_None);
		return Py_None;
	}
	PyObject* clearTransformReference(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 };
		p->p->ClearTransformReferences();
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	// Method list
	//
	PyMethodDef methods[] = {
		{"getName",	(PyCFunction)getName, METH_NOARGS, "name"},
		{"setName",	(PyCFunction)setName, METH_VARARGS, "set the name"},
		{"addVtxBuffer", (PyCFunction)addVtxBuffer, METH_VARARGS, "addVtxBuffer(vtxbuffer [, bIsBlendshape] [,vtxbuffer2, bIsBlendshape2,...])"},
		{"addIdxBuffer", (PyCFunction)addIdxBuffer, METH_VARARGS, "addIdxBuffer(idxbuffer [, primgroup#] [,idxbuffer2 , primgroup2, ...]"},
		//virtual PBuffer		CreateVtxBuffer(LPCSTR name, int numcomp, int slot = 0, DataType type = FLOAT32, bool isBlendShape = false) = 0;
		//virtual PBuffer		CreateIdxBuffer(LPCSTR name, DataType type = UINT32, int numComp=1) = 0;
		{"createPrimGroup", (PyCFunction)createPrimGroup, METH_VARARGS, "createPrimGroup(name, idxBuf, Topology.xxx, material, offsetElement, numElements)"},
		{"detachBuffer", (PyCFunction)detachBuffer, METH_VARARGS, "deleteBuffer(buffer)"},
		{"computeBoundingVolumes", (PyCFunction)computeBoundingVolumes, METH_VARARGS, "doc"},
		{"addTransformReference", (PyCFunction)addTransformReference, METH_VARARGS, "addTransformReference(transform [,primgroup])"},
		{"clearTransformReference", (PyCFunction)clearTransformReference, METH_NOARGS, "clearTransformReference() : remove all references"},
		//virtual bool		ComputeNormalsAndTangents(PBuffer bufferIdx, PBuffer bufferVtxPos, PBuffer bufferTexcoords,
		//						PBuffer bufferPerVtxNormals, PBuffer bufferPerFaceNormals = NULL, 
		//						PBuffer bufferTangents = NULL, PBuffer bufferBitangents = NULL) = 0;
		{NULL,		NULL}		/* sentinel */
	};

	// --------------------------------------------------
	//
	int tp_init(PyObject *, PyObject *, PyObject *)
	{
		//_asm { int 3 };
		return 0;
	}

	// --------------------------------------------------
	//
	PyObject *tp_new(struct _typeobject *type, PyObject *args, PyObject *kwds)
	{
		PyObj *self=NULL;
		PyObject *objname=NULL;
		char *name = NULL;
		self = (PyObj *)type->tp_alloc(type, 0);

		static char *kwlist[] = {"name", NULL};
		//http://docs.python.org/c-api/arg.html#PyArg_ParseTupleAndKeywords
		PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &name);
		self->p = Mesh::Create(name);
		self->p->SetUserData(self);
		return (PyObject*)self;
	}

	// --------------------------------------------------
	//
	void dealloc(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		int n = o->p->GetNumVtxBuffers(false);
		for(int i=0; i<n; i++)
		{
			bk3dlib::PBuffer p = o->p->GetVtxBuffer(0, false); // 0 because we erase them during the loop...
			if(p) o->p->DetachBuffer(p); // need to delete it first because Python DecRef can delete it, too
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}
		n = o->p->GetNumVtxBuffers(true);
		for(int i=0; i<n; i++)
		{
			bk3dlib::PBuffer p = o->p->GetVtxBuffer(0, true);
			if(p) o->p->DetachBuffer(p); // need to delete it first because Python DecRef can delete it, too
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}
		n = o->p->GetNumPrimGroups();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PrimGroup pg;
			// TODO : Detach Material ?
			if(o->p->GetPrimGroupInfo(i, pg) && pg.pMat) 
				Py_DecRef((PyObject*)pg.pMat->GetUserData());
			// Release the index buffer of the prim group
      for(int j=0; j<pg.numIdxBuffers; j++)
  			if(pg.idxBuffers[j]) Py_DecRef((PyObject*)pg.idxBuffers[j]->GetUserData());
		}
		n = o->p->GetNumIdxBuffers();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PBuffer p = o->p->GetIdxBuffer(0);
			if(p) o->p->DetachBuffer(p); // need to delete it first because Python DecRef can delete it, too
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}
		n = o->p->GetNumTransformReferences();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PBone p = o->p->GetTransformReference(i);
			// TODO : Detach Transform ?
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}

		o->p->Destroy();
		o->p = NULL;
		PyObject_Del(self);
	}

	// --------------------------------------------------
	//
	PyObject *getattro(PyObject *self, PyObject *nameobj)
	{
		PyObj *o = (PyObj *)self;
        Py_UNICODE *name;
        name = PyUnicode_AS_UNICODE(nameobj);
		//
		// Check properties we know
		//
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			return PySTRING_FromString(o->p->GetName());
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "vtxbuffer") == 0)
		{
			//bk3dlib::PBuffer pBuf = GetVtxBuffer(PyInt_AsLong(), bool isBlendShape = false) = 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "idxbuffer") == 0)
		{
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "numvtxbuffers") == 0)
		{
			return PyLI_FromLong(o->p->GetNumVtxBuffers(false));
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "numvtxbuffersblendshape") == 0)
		{
			return PyLI_FromLong(o->p->GetNumVtxBuffers(true));
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "numidxbuffers") == 0)
		{
			return PyLI_FromLong(o->p->GetNumIdxBuffers());
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "numslots") == 0)
		{
			return PyLI_FromLong(o->p->GetNumSlots());
		}
        // should use tp_methods instead
        // http://stackoverflow.com/questions/8078197/py-findmethod-is-gone-in-python3-what-should-i-use-instead
		//PyObject *resobj = Py_FindMethod(methods, self, name);
		//if(resobj)
		//	return resobj;
		//
		// Default Generic Attr
		//
		return PyObject_GenericGetAttr(self, nameobj);
	}

	// --------------------------------------------------
	//
	int setattro(PyObject *self, PyObject *nameobj, PyObject *srcobj)
	{
		PyObj *o = (PyObj *)self;
		//
		// Check properties we know
		//
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			o->p->SetName( PySTRING_AsString(*srcobj) );
			return 0;
		}
		//
		// Default Generic Attr
		//
		return PyObject_GenericSetAttr(self, nameobj, srcobj);
	}

	// --------------------------------------------------
	//
	PyObject *tp_repr(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifndef IS_PY3K
		return PyString_FromFormat("{Bk3d Mesh %s}", o->p->GetName());
#else
		return PyUnicode_FromFormat("{Bk3d Mesh %s}", o->p->GetName());
#endif
	}

	// --------------------------------------------------
	//
	PyObject *tp_str(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifndef IS_PY3K
		return PyString_FromFormat("Bk3d Mesh named %s", o->p->GetName());
#else
		return PyUnicode_FromFormat("Bk3d Mesh named %s", o->p->GetName());
#endif
	}

	// --------------------------------------------------
	//
	char doc [] =
	"------------\n"
	"- MeshType -\n"
	"------------\n"
	"Properties\n\n"
	"  - name (RW)\n"
	"  - numvtxbuffers (R) : # vtx buffers\n"
	"  - numvtxbuffersblendshape (R) : # buffers for Blendshape\n"
	"  - numidxbuffers (R) : # used buffers\n"
	"  - numslots (R) : available slots in the mesh\n"
	"Properties\n\n"
	"  - getName() : name"
	"  - setName() : set the name"
	"  - addVtxBuffer() : addVtxBuffer(vtxbuffer [, bIsBlendshape] [,vtxbuffer2, bIsBlendshape2,...])"
	"  - addIdxBuffer() : addIdxBuffer(idxbuffer [, primgroup#] [,idxbuffer2 , primgroup2, ...]"
	//virtual PBuffer		CreateVtxBuffer(LPCSTR name, int numcomp, int slot = 0, DataType type = FLOAT32, bool isBlendShape = false) = 0;
	//virtual PBuffer		CreateIdxBuffer(LPCSTR name, DataType type = UINT32, int numComp=1) = 0;
	"  - createPrimGroup() : createPrimGroup(name, idxBuf, Topology.xxx, material, offsetElement, numElements)"
	"  - detachBuffer() : deleteBuffer(buffer)"
	"  - computeBoundingVolumes() : doc"
	"  - addTransformReference() : addTransformReference(transform [,primgroup])"
	"  - clearTransformReference() : clearTransformReference() : remove all references"
	//virtual bool		ComputeNormalsAndTangents(PBuffer bufferIdx, PBuffer bufferVtxPos, PBuffer bufferTexcoords,
	"\n";

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
		"MeshType",
		sizeof(PyObj), // taille de l'objet
		0,	// taille de l'Item ?
		dealloc,	//tp_dealloc
		0,//tp_print,		// PyObject_Print - Avoid : it may prevent tp_str / repr to be used
		0,	// tp_getattr - WARNING : cancels tp_methods
		0,				// tp_setattr
#ifdef IS_PY3K
        0,          // tp_reserved
#else
		PyObject_Compare,		// tp_compare : comparaison
#endif
		tp_repr,		// PyObject_Repr//tp_repr : utilisé par les funcs repr() et PyObject_Repr()
		0,				// tp_as_number : opérations typiques (+ - * etc)
		0,		// tp_as_sequence : concatenations etc.
		0,				// tp_as_mapping : pour les accès par indice (tableaux)
		0, 				// tp_hash : A VOIR...
		0,				// tp_call : appel de l'objet en tant que fonction ( a= obj() )
		tp_str,			// tp_str : like tp_repr. More fore human reading. If not used, tp_repr used instead...
		getattro,		// tp_getattro : version avec un objet au lieu d'un char*...
		setattro,		// tp_setattro
		0,				// tp_as_buffer
		0,				// tp_xxx4
		doc,			// tp_doc
		/* Assigned meaning in release 2.0 */
		/* call function for all accessible objects */
		NULL,//traverseproc tp_traverse;

		/* delete references to contained objects */
		NULL,//inquiry tp_clear;

		/* Assigned meaning in release 2.1 */
		/* rich comparisons */
		NULL,//richcmpfunc tp_richcompare;

		/* weak reference enabler */
		0,//Py_ssize_t tp_weaklistoffset;

		/* Added in release 2.2 */
		/* Iterators */
		NULL,//getiterfunc tp_iter;
		NULL,//iternextfunc tp_iternext;

		/* Attribute descriptor and subclassing stuff */
		methods,//struct PyMethodDef *tp_methods; WARNING: canceled if tp_getattr/tp_setattr used
		NULL,//struct PyMemberDef *tp_members;
		NULL,//struct PyGetSetDef *tp_getset;
		NULL,//struct _typeobject *tp_base;
		NULL,//PyObject *tp_dict;
		NULL,//descrgetfunc tp_descr_get;
		NULL,//descrsetfunc tp_descr_set;
		NULL,//Py_ssize_t tp_dictoffset;
		tp_init,//initproc tp_init;
		PyType_GenericAlloc,//allocfunc tp_alloc;
		tp_new,//PyType_GenericNew//newfunc tp_new;
		NULL,//freefunc tp_free; /* Low-level free-memory routine */
		0,//inquiry tp_is_gc; /* For PyObject_IS_GC */
		NULL,//PyObject *tp_bases;
		NULL,//PyObject *tp_mro; /* method resolution order */
		NULL,//PyObject *tp_cache;
		NULL,//PyObject *tp_subclasses;
		NULL,//PyObject *tp_weaklist;
		NULL,//destructor tp_del;

		/* Type attribute cache version tag. Added in version 2.6 */
		0//unsigned int tp_version_tag;
	};//PyTypeObject
}//namespace bk3dHeader
