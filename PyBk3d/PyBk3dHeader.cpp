#include "stdafx.h"
#include "PyBk3d.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dHeader
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
	// TODO: when adding the mesh, check if referenced transforms/materials
	// got also added to the Header !
	//
	static PyObject *addMesh(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i++)
		{
			PyObject *po = PyTuple_GetItem(args, i);
			if(po && (PyObject_TypeCheck(po, &bk3dMesh::classType)))
			{
				bk3dMesh::PyObj *pMesh = (bk3dMesh::PyObj *)po;
				//
				// We must also make sure that the referenced materials and transforms are also added to this Header
				// This is already done in AddMesh() but we need to increase Pythong reference, too...
				//
				int n = pMesh->p->GetNumTransformReferences();
				for(int i=0; i<n; i++)
				{
					bk3dlib::PBone pT = pMesh->p->GetTransformReference(i);
					int nt = p->p->GetNumTransforms();
					int j;
					for(j=0; j<nt; j++)
					{
						bk3dlib::PBone pT2 = p->p->GetTransform(j);
						if(pT == pT2)
							break;
					}
					if(j==nt)
					{
						p->p->AttachTransform(pT);
						Py_IncRef((PyObject*)pT->GetUserData()); // this is why we do all of this... :-(
					}
				}

				n = pMesh->p->GetNumPrimGroups();
				for(int i=0; i<n; i++)
				{
					bk3dlib::PrimGroup pginfo;
					pMesh->p->GetPrimGroupInfo(i, pginfo);
					if(pginfo.pMat)
					{
						// First, check if the material was already stored
						bk3dlib::PMaterial pM = p->p->GetMaterial(pginfo.pMat->GetName());
						// Nope, so let's add it
						if(pM == NULL)
						{
							p->p->AttachMaterial(pginfo.pMat);
							Py_IncRef((PyObject*)pginfo.pMat->GetUserData()); // this is why we do all of this... :-(
						}
					}
				}
				//
				// Add the mesh
				//
				p->p->AttachMesh(pMesh->p);
				Py_INCREF(pMesh);
			}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addTransform(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i++)
		{
			PyObject *po = PyTuple_GetItem(args, i);
			if(po && (PyObject_TypeCheck(po, &bk3dTransform::classType)))
			{
				bk3dTransform::PyObj *pT = (bk3dTransform::PyObj *)po;
				p->p->AttachTransform(pT->p);
				Py_INCREF(pT);
			}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addCurveVec(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i++)
		{
			PyObject *po = PyTuple_GetItem(args, i);
			//if(po && (PyObject_TypeCheck(po, &bk3dCurveVec::classType)))
			//{
			//	bk3dCurveVec::PyObj *pCurveVec = (bk3dCurveVec::PyObj *)po;
			//	p->p->AddTransform(pCurveVec->p);
			//	Py_INCREF(pCurveVec);
			//}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addMaterial(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		int sz = (int)PyTuple_Size(args);
		for(int i=0; i<sz; i++)
		{
			PyObject *po = PyTuple_GetItem(args, i);
			if(po && (PyObject_TypeCheck(po, &bk3dMaterial::classType)))
			{
				bk3dMaterial::PyObj *pMaterial = (bk3dMaterial::PyObj *)po;
				p->p->AttachMaterial(pMaterial->p);
				Py_INCREF(pMaterial);
			}
		}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *cook(PyObject *self, PyObject *args)
	{
		unsigned int szBytes = 0;
		PyObj *p = (PyObj *)self;
		LPCSTR pName = NULL;
        char * name;
		PyArg_ParseTuple(args, "s", &name);
		//_asm { int 3 }
        if(name == NULL)
            name = "bk3dCache.bk3d";
		if(p->p->Cook(name, NULL, &szBytes) == NULL)
		{
			PyErr_SetString(PyExc_AttributeError, "Failed cooking");
			Py_IncRef(Py_None);
			return Py_None;
		}
		return PyLI_FromLong(szBytes);
	}

	// --------------------------------------------------
	//
	static PyObject *save(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		PyArg_ParseTuple(args, "s", &pName);
		if(pName)
			if(!p->p->Save(pName))
			{
				PyErr_SetString(PyExc_AttributeError, "Failed saving");
				Py_IncRef(Py_None);
				return Py_None;
			}
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *getTransform(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		PyArg_ParseTuple(args, "s", &pName);
		if(pName)
        {
            bk3dlib::PBone pTr = p->p->GetTransform(pName);
			if(!pTr)
			{
				Py_IncRef(Py_None);
				return Py_None;
			}
            PyObject* o = (PyObject*)pTr->GetUserData();
            Py_IncRef(o);
            return o;
        }
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *getMesh(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		PyArg_ParseTuple(args, "s", &pName);
		if(pName)
        {
            bk3dlib::PMesh pM = p->p->FindMesh(pName);
			if(!pM)
			{
				Py_IncRef(Py_None);
				return Py_None;
			}
            PyObject* o = (PyObject*)pM->GetUserData();
            Py_IncRef(o);
            return o;
        }
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *getMaterial(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 }
		LPCSTR pName = NULL;
		PyArg_ParseTuple(args, "s", &pName);
		if(pName)
        {
            bk3dlib::PMaterial pM = p->p->GetMaterial(pName);
			if(!pM)
			{
				Py_IncRef(Py_None);
				return Py_None;
			}
            PyObject* o = (PyObject*)pM->GetUserData();
            Py_IncRef(o);
            return o;
        }
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	// Method list
	//
	PyMethodDef methods[] = {
		{"getName",	(PyCFunction)getName, METH_NOARGS, "name of the header"},
		{"setName",	(PyCFunction)setName, METH_VARARGS, "sets the name"},
		{"addMesh", (PyCFunction)addMesh, METH_VARARGS, "addMesh(mesh) or addMesh(mesh1, mesh2, ...)"},
		{"addTransform", (PyCFunction)addTransform, METH_VARARGS, "addTransform(transf) or addTransform(transf1,transf2,...)"},
		{"addMaterial", (PyCFunction)addMaterial, METH_VARARGS, "addMaterial(m) or addMaterial(m1,m2,...)"},
		{"addCurveVec", (PyCFunction)addCurveVec, METH_VARARGS, "addCurveVec(cv) or addCurveVec(cv1,cv2,...)"},

			//virtual	bool		LoadFromBk3dFile(LPCSTR file) = 0;

			//virtual PMesh			CreateMesh(LPCSTR name) = 0;
			//virtual PBone	CreateTransform(LPCSTR name) = 0;
			//virtual PCurveVec		CreateCurveVec(LPCSTR name, int ncomps) = 0;
			//virtual PMaterial		CreateMaterial(LPCSTR name) = 0;

		{"cook", (PyCFunction)cook, METH_VARARGS, "cook(cacheName) : prepares the binary data"},
		{"save", (PyCFunction)save, METH_VARARGS, "saves the cooked data"},

			////virtual bool		DeleteMesh(PMesh p) = 0;
		{"getMesh", (PyCFunction)getMesh, METH_VARARGS, "returns a named or indexed Mesh, if exists"},
		{"getMaterial", (PyCFunction)getMaterial, METH_VARARGS, "returns a named or indexed Material, if exists"},
		{"getTransform", (PyCFunction)getTransform, METH_VARARGS, "returns a named or indexed Transform, if exists"},

		{NULL,		NULL}		/* sentinel */
	};

	// --------------------------------------------------
	//
	int tp_init(PyObject *, PyObject *, PyObject *)
	{
		////_asm { int 3 };
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
		self->p = FileHeader::Create(name);
		self->p->SetUserData(self);
		return (PyObject*)self;
	}

	// --------------------------------------------------
	//
	void dealloc(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		int n = o->p->GetNumMeshes();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PMesh p = o->p->FindMesh(i);
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}
		//n = o->p->GetNumCurves();
		//for(int i=0; i<n; i++)
		//{
		//	bk3dlib::PCurveVec p = o->p->GetCurveVec(i);
		//	if(p) Py_DecRef((PyObject*)p->GetUserData());
		//}
		n = o->p->GetNumTransforms();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PBone p = o->p->GetTransform(i);
			if(p) Py_DecRef((PyObject*)p->GetUserData());
		}

		n = o->p->GetNumMaterials();
		for(int i=0; i<n; i++)
		{
			bk3dlib::PMaterial p = o->p->GetMaterial(i);
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
        Py_UNICODE *name;
        name = PyUnicode_AS_UNICODE(nameobj);
		PyObj *o = (PyObj *)self;
		//
		// Check properties we know
		//
        if (PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			return PySTRING_FromString(o->p->GetName());
		}
		else if (PyUnicode_CompareWithASCIIString(nameobj, "numMeshes") == 0)
		{
			return PyLI_FromLong(o->p->GetNumMeshes());
		}
        else if (PyUnicode_CompareWithASCIIString(nameobj, "numTransforms") == 0)
		{
			return PyLI_FromLong(o->p->GetNumTransforms());
		}
		else if (PyUnicode_CompareWithASCIIString(nameobj, "numMaterials") == 0)
		{
			return PyLI_FromLong(o->p->GetNumMaterials());
		}
		else if (PyUnicode_CompareWithASCIIString(nameobj, "numCurves") == 0)
		{
			return PyLI_FromLong(o->p->GetNumCurves());
		}
		//
		// methods : seems like if I overloaded the attribute management, I need to forward method search...
		//
        // http://stackoverflow.com/questions/8078197/py-findmethod-is-gone-in-python3-what-should-i-use-instead
        // pior to 3.x. Now obsolete
		//PyObject *resobj = Py_FindMethod(methods, self, name);
		//if(resobj)
		//	return resobj;
		//
		// Default Generic Attr
		//
		return PyObject_GenericGetAttr(self, nameobj);
	}

	// --------------------------------------------------
	// check with http://hg.python.org/cpython/file/137e45f15c0b/Modules/pyexpat.c#l1227
	int setattro(PyObject *self, PyObject *pyname, PyObject *srcobj)
    {
        /* Set attribute 'name' to value 'v'. v==NULL means delete */
        if (srcobj == NULL) {
         PyErr_SetString(PyExc_RuntimeError, "Cannot delete attribute");
         return -1;
        }
        const char* name = PySTRING_AsString(*pyname);
		PyObj *o = (PyObj *)self;
		//
		// Check properties we know
		//
		if (PyUnicode_CompareWithASCIIString(pyname, "name") == 0)
		{
			o->p->SetName( PySTRING_AsString(*srcobj) );
			return 0;
		}
		//
		// Default Generic Attr
		//
		return PyObject_GenericSetAttr(self, pyname, srcobj);
	}

	// --------------------------------------------------
	//
	PyObject *tp_repr(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifndef IS_PY3K
		return PyString_FromFormat("{Bk3d Header %s}", o->p->GetName());
#else
		return PyUnicode_FromFormat("{Bk3d Header %s}", o->p->GetName());
#endif
	}

	// --------------------------------------------------
	//
	PyObject *tp_str(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifdef IS_PY3K
        return PyUnicode_FromFormat(
			"Bk3d Header named %s\n"
			"Curves: %d\n"
			"Materials: %d\n"
			"Meshes: %d\n"
			"Transforms: %d\n"
			, o->p->GetName(), o->p->GetNumCurves(), o->p->GetNumMaterials(), o->p->GetNumMeshes(), o->p->GetNumTransforms());
#else
        return PyString_FromFormat(
			"Bk3d Header named %s\n"
			"Curves: %d\n"
			"Materials: %d\n"
			"Meshes: %d\n"
			"Transforms: %d\n"
			, o->p->GetName(), o->p->GetNumCurves(), o->p->GetNumMaterials(), o->p->GetNumMeshes(), o->p->GetNumTransforms());
#endif
    }

	// --------------------------------------------------
	//
	char doc [] =
		"--------------\n"
		"- HeaderType -\n"
		"--------------\n"
		"Container for all the data that we are going to save in the binary format\n"
		"Properties:\n\n"
		"  - name (RW)\n"
		"  - numMeshes (R)\n"
		"  - numTransforms (R)\n"
		"  - numMaterials (R)\n"
		"  - numCurves (R)\n"
		"\nFunctions:\n\n"
		"  - getName() : name of the header\n"
		"  - setName() : sets the name\n"
		"  - addMesh() : addMesh(mesh) or addMesh(mesh1, mesh2, ...)\n"
		"  - addTransform() : addTransform(transf) or addTransform(transf1,transf2,...)\n"
		"  - addMaterial() : addMaterial(m) or addMaterial(m1,m2,...)\n"
		"  - addCurveVec() : addCurveVec(cv) or addCurveVec(cv1,cv2,...)\n"
		"  - getTransform(name) : finds back a transformation by name\n"
		"..-.GetMesh(LPCSTR name)\n"
		"..-.GetMaterial(LPCSTR name)\n"
		"..-.GetTransform(LPCSTR name)\n"
		"\nTODO:\n\n"
		"...LoadFromBk3dFile(LPCSTR file)\n"
		"...CreateMesh(LPCSTR name)\n"
		"...CreateTransform(LPCSTR name)\n"
		"...CreateCurveVec(LPCSTR name, int ncomps)\n"
		"...CreateMaterial(LPCSTR name)\n"
		"...DeleteMesh(PMesh p)\n"
		"...GetMesh(int n)\n"
		"...GetMaterial(int n)\n"
		"...GetTransform(int n)\n"
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
		"HeaderType",
		sizeof(PyObj),  // taille de l'objet
		0,              // taille de l'Item ?
		dealloc,        //tp_dealloc
		0,              //tp_print,		// PyObject_Print - Avoid : it may prevent tp_str / repr to be used
        0,              // tp_getattr (use attro instead)
        0,              // tp_setattr
        0,              // tp_reserved
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
