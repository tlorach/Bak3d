#include "stdafx.h"
#include "PyBk3d.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dBuffer
{
	static LPCSTR dataTypeAsString(bk3dlib::DataType dt)
	{
		switch(dt)
		{
		case bk3dlib::FLOAT32:	return "FLOAT32";
		case bk3dlib::FLOAT16:	return "FLOAT16";
		case bk3dlib::UINT32:	return "UINT32";
		case bk3dlib::UINT16:	return "UINT16";
		case bk3dlib::UINT8:	return "UINT8";
		case bk3dlib::UNKNOWN:	break;
		}
        return "UNKNOWN";
	}
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
	static PyObject *clearData(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
		p->p->ClearData();
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *goToItem(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		int i = 0;
		PyArg_ParseTuple(args, "i", &i);
		p->p->GotoItem(i);
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *setDivisor(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		int i = 0;
		PyArg_ParseTuple(args, "i", &i);
		p->p->SetDivisor(i);
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *SIB_ClearBuffers(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
		p->p->SIB_ClearBuffers();
		Py_IncRef(Py_None);
		return Py_None;
	}
	// --------------------------------------------------
	//
	static PyObject *SIB_AddBuffers(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		bk3dBuffer::PyObj *objIdx, *objSrc, *objDst;
		//PyArg_ParseTuple : http://docs.python.org/c-api/arg.html
		PyArg_ParseTuple(args, "O!O!O!", &bk3dBuffer::classType,&objIdx, &bk3dBuffer::classType,&objSrc, &bk3dBuffer::classType,&objDst);
		if((objSrc == NULL)||(objIdx == NULL))
		{
			PyErr_SetString(PyExc_AttributeError, "SIB problem");
			return NULL;
		}
		p->p->SIB_AddBuffers(objIdx->p, objSrc->p, objDst ? objDst->p : NULL);
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *SIB_Compile(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		int i = 0;
		//PyArg_ParseTuple(args, "i", &i);
		p->p->SIB_Compile();
		Py_IncRef(Py_None);
		return Py_None;
	}

	// --------------------------------------------------
	//
	static PyObject *addData(PyObject *self, PyObject *args)
	{
		float f;
		long l;
		int sz;
		PyObj *p = (PyObj *)self;
		sz = (int)PyTuple_Size(args);
		bk3dlib::DataType dtype = p->p->GetDataType();
		switch(dtype)
		{
		case bk3dlib::FLOAT32:
		case bk3dlib::FLOAT16:
		case bk3dlib::UNKNOWN:
		default:
			for(int i=0; i<sz; i++)
			{
				PyObject *po = PyTuple_GetItem(args, i);
				if(!po) 
				{
					PyErr_SetString(PyExc_AttributeError, "Bad item");
					return NULL;
				}
				// case of a Blender vector
				// We don't know the implementation so let's use attributes with names
				if(PyObject_HasAttrString(po, "x"))
				{
					PyObject *oComp = PyObject_GetAttrString(po, "x");
					float fVal = (float)PyFloat_AsDouble(oComp);
					p->p->AddData(&fVal, 1);
					Py_DecRef(oComp);
					if(PyObject_HasAttrString(po, "y"))
					{
						oComp = PyObject_GetAttrString(po, "y");
						fVal = (float)PyFloat_AsDouble(oComp);
						p->p->AddData(&fVal, 1);
						Py_DecRef(oComp);
					}
					if(PyObject_HasAttrString(po, "z"))
					{
						oComp = PyObject_GetAttrString(po, "z");
						fVal = (float)PyFloat_AsDouble(oComp);
						p->p->AddData(&fVal, 1);
						Py_DecRef(oComp);
					}
					if(PyObject_HasAttrString(po, "w"))
					{
						oComp = PyObject_GetAttrString(po, "w");
						fVal = (float)PyFloat_AsDouble(oComp);
						p->p->AddData(&fVal, 1);
						Py_DecRef(oComp);
					}
				}
				// case of a sub-list
				else if(PyList_Check(po))
				{
					int lsz = (int)PyList_Size(po);
					for(int j=0; j<lsz; j++)
					{
						PyObject *plo = PyList_GetItem(po, j);
						// case of a sub-list
						if(PyList_Check(plo))
						{
							int lsz2 = (int)PyList_Size(plo);
							for(int j=0; j<lsz2; j++)
							{
								PyObject *plo2 = PyList_GetItem(plo, j);
								int n = PyArg_Parse(plo2, "f", &f);
								if(n>0) p->p->AddData(&f, 1);
								else { 
									PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
									return NULL;
								}
							}
						}
						else
						{
							int n = PyArg_Parse(plo, "f", &f);
							if(n>0) p->p->AddData(&f, 1);
							else {
								PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
								return NULL;
							}
						}
					}
				}
				else
				{
					int n = PyArg_Parse(po, "f", &f);
					if(n>0) p->p->AddData(&f, 1);
					else {
						PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
						return NULL;
					}
				}
			}
			break;
		case bk3dlib::UINT32:
		case bk3dlib::UINT16:
		case bk3dlib::UINT8:
			for(int i=0; i<sz; i++)
			{
				PyObject *po = PyTuple_GetItem(args, i);
				if(!po) {
					PyErr_SetString(PyExc_AttributeError, "Problem with Item argument");
					return NULL;
				}
				assert(po);
				// case of a sub-list
				if(PyList_Check(po))
				{
					int lsz = (int)PyList_Size(po);
					for(int j=0; j<lsz; j++)
					{
						PyObject *plo = PyList_GetItem(po, j);
						// case of a sub-list
						if(PyList_Check(plo))
						{
							int lsz2 = (int)PyList_Size(plo);
							for(int j=0; j<lsz2; j++)
							{
								PyObject *plo2 = PyList_GetItem(plo, j);
								int n = PyArg_Parse(plo2, "l", &l);
								if(n>0) p->p->AddData(&l, 1);
								else {
									PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
									return NULL;
								}
							}
						}
						else
						{
							int n = PyArg_Parse(plo, "l", &l);
							if(n>0) p->p->AddData(&l, 1);
							else {
								PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
								return NULL;
							}
						}
					}
				}
				else
				{
					int n = PyArg_Parse(po, "l", &l);
					if(n>0) p->p->AddData(&l, 1);
					else {
						PyErr_SetString(PyExc_AttributeError, "Couldn't add arg to buffer");
						return NULL;
					}
				}
			}
			break;
		}
		Py_IncRef(Py_None);
		return Py_None;
	}
	// --------------------------------------------------
	//
	static PyObject *dataAsList(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
        int n = p->p->GetNumItems();
        int nc = p->p->GetNumComps();
        n *= nc; // for now lets create a flat list instead of a list of component lists
        //if(n == 0)
        //{
        //    Py_IncRef(Py_None);
        //    return Py_None;
        //}
        bk3dlib::DataType dtype = p->p->GetDataType();
        PyObject *olist = PyList_New(n);
        switch(dtype)
        {
        case bk3dlib::FLOAT32:
        case bk3dlib::FLOAT16:
            {
                float *farray = (float *)malloc(sizeof(float)*n);
                p->p->GetData(farray, 0, n);
                for(int i=0; i<n; i++)
                {
                    PyList_SetItem(olist, i, PyFloat_FromDouble((double)farray[i]) );
                }
                free(farray);
            }
            break;
        case bk3dlib::UINT32:
        case bk3dlib::UINT16:
        case bk3dlib::UINT8:
            {
                unsigned int *uiarray = (unsigned int *)malloc(sizeof(unsigned int)*n);
                p->p->GetData(uiarray, 0, n);
                for(int i=0; i<n; i++)
                {
                    PyList_SetItem(olist, i, PyLong_FromLong(uiarray[i]) );
                }
                free(uiarray);
            }
            break;
        }
        return olist;
	}
	// --------------------------------------------------
	// Method list
	//
	PyMethodDef methods[] = {
		{"getName",	(PyCFunction)getName, METH_NOARGS, "return the name of the buffer"},
		{"setName",	(PyCFunction)setName, METH_VARARGS, "sets the name of the buffer"},
		{"clearData",	(PyCFunction)clearData, METH_NOARGS, "clear the data : vertices of indices"},
		{"addData",	(PyCFunction)addData, METH_VARARGS, "adds vertex/index data"},
		{"goToItem",	(PyCFunction)goToItem, METH_VARARGS, "set the place where the add will occur"},
		{"setDivisor",	(PyCFunction)setDivisor, METH_VARARGS, "todo..."},
		{"dataAsList",	(PyCFunction)dataAsList, METH_NOARGS, "return all the data as a list"},

		{"SIB_ClearBuffers",	(PyCFunction)SIB_ClearBuffers, METH_NOARGS, "SIB_ClearBuffers() : clears the attached list of buffers used for Single index computation"},
		{"SIB_AddBuffers",	(PyCFunction)SIB_AddBuffers, METH_VARARGS, "SIB_AddBuffers(bufferIdx, bufferAttrSrc, bufferAttrDst) : Add a set of index buffer with related attribute buffer. the last argument is the target of the modified buffer resulting from the single index computation"},
		{"SIB_Compile",	(PyCFunction)SIB_Compile, METH_VARARGS, "SIB_Compile() : construct the single index buffer and generate related attribute buffers"},
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
		self->p = Buffer::Create(name);
		self->p->SetUserData(self);
		self->p->SetDataType(bk3dlib::FLOAT32);
		return (PyObject*)self;
	}

	// --------------------------------------------------
	//
	void dealloc(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
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
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			return PySTRING_FromString(o->p->GetName());
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "dataType") ==0)
		{
			return PyLong_FromLong((long)o->p->GetDataType()); // PyInt to be replaced with PyLong
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "slot") == 0)
		{
			return PyLong_FromLong((long)o->p->GetSlot());
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "components") == 0)
		{
			return PyLong_FromLong((long)o->p->GetNumComps());
		}
		//if(PyUnicode_CompareWithASCIIString(nameobj, "numVectors") == 0)
		//{
		//	return PyLong_FromLong((long)o->p->GetNumVectors());
		//}
		if(PyUnicode_CompareWithASCIIString(nameobj, "numItems") == 0)
		{
			return PyLong_FromLong((long)o->p->GetNumItems());
		}
		//
		// methods : seems like if I overloaded the attribute management, I need to forward method search...
		// NOTE: http://www.mail-archive.com/python-3000@python.org/msg15184.html
		//
        // http://stackoverflow.com/questions/8078197/py-findmethod-is-gone-in-python3-what-should-i-use-instead
		//PyObject *resobj = PyObject_GenericGetAttr(self, PyBytes_FromString(name));
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
        /* Set attribute 'name' to value 'v'. v==NULL means delete */
        if (srcobj == NULL) {
         PyErr_SetString(PyExc_RuntimeError, "Cannot delete attribute");
         return -1;
        }
		//
		// Check properties we know
		//
        if (PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			o->p->SetName( PySTRING_AsString(*srcobj) ); //http://stackoverflow.com/questions/8229597/embedding-python
			return 0;
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "dataType") == 0)
		{
			o->p->SetDataType((bk3dlib::DataType)PyLong_AsLong(srcobj));
			return 0;
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "slot") == 0)
		{
			o->p->SetSlot((bk3dlib::DataType)PyLong_AsLong(srcobj));
			return 0;
		}
		if(PyUnicode_CompareWithASCIIString(nameobj, "components") == 0)
		{
			o->p->SetNumComps((bk3dlib::DataType)PyLong_AsLong(srcobj));
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
#ifdef IS_PY3K
		return PyUnicode_FromFormat("{Bk3d Buffer %s}", o->p->GetName());
#else
		return PyString_FromFormat("{Bk3d Buffer %s}", o->p->GetName());
#endif
	}

	// --------------------------------------------------
	//
	PyObject *tp_str(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifdef IS_PY3K
		PyObject *strobj = PyUnicode_FromFormat(
#else
		PyObject *strobj = PyString_FromFormat(
#endif
			"Bk3d Buffer named %s\n"
			"	dataType = %s\n"
			"	slot = %d\n"
			"	components = %d\n"
			"	numItems = %d\n"
			, o->p->GetName(), dataTypeAsString(o->p->GetDataType()), o->p->GetSlot(),o->p->GetNumComps(), o->p->GetNumItems());
		return strobj;
	}

	// --------------------------------------------------
	//
	char doc [] =
	"--------------\n"
	"- BufferType -\n"
	"--------------\n"
	"Buffer object in which vertices or indices are stored. This object can then be attached to a Mesh\n"
	"Properties:\n\n"
	" - name (RW): Buffer name\n"
	" - dataType (RW): data type. Use DataType class to choose one(DataType.FLOAT32 etc.)\n"
	" - slot (RW): slot in which the buffer will be used if vertex attributes\n"
	" - components (RW): number of components to define a vector\n"
	" - numItems (R): number of items (number of vectors)\n\n"
	"Functions:\n\n"
	" - getName() : return the name of the buffer\n"
	" - setName() : sets the name of the buffer\n"
	" - clearData() : clear the data : vertices of indices\n"
	" - addData() : adds vertex/index data\n"
	" - goToItem() : set the place where the add will occur\n"
	" - setDivisor() : todo...\n"
	" - SIB_ClearBuffers() : SIB_ClearBuffers() : clears the buffers for Single index compute\n"
	" - SIB_AddBuffers() : SIB_AddBuffers(bufferIdx, bufferAttrSrc, bufferAttrDst) : add buffers...blah\n"
	" - SIB_Compile() : SIB_Compile(bool bDeleteBuffers) : construct the single index buffer\n"
	"\n";
	// --------------------------------------------------
	//
	// bk3d Header Type
	// http://docs.python.org/extending/newtypes.html?highlight=pytypeobject
	//
	PyTypeObject classType = 
	{
		PyVarObject_HEAD_INIT(&PyType_Type, 0)
		"BufferType", // const char *tp_name
		sizeof(PyObj), // Py_ssize_t tp_basicsize : taille de l'objet
		0,	// tp_itemsize :taille de l'Item ?
		dealloc,	//tp_dealloc
		0,//tp_print,		// PyObject_Print - Avoid : it may prevent tp_str / repr to be used
		0,	// tp_getattr - WARNING : cancels tp_methods
		0,				// tp_setattr
#ifdef IS_PY3K
        0,              // tp_reserved
#else
		PyObject_Compare,// tp_compare : comparaison
#endif
		tp_repr,		// PyObject_Repr//tp_repr : utilisé par les funcs repr() et PyObject_Repr()
		0,				// tp_as_number : opérations typiques (+ - * etc)
		0,		// tp_as_sequence : concatenations etc.
		0,				// tp_as_mapping : pour les accès par indice (tableaux)
		0, 				// tp_hash : A VOIR...
		0,				// tp_call : appel de l'objet en tant que fonction ( a= obj() )
		tp_str,			// tp_str : like tp_repr. More fore human reading. If not used, tp_repr used instead...
		getattro,				// tp_getattro : version avec un objet au lieu d'un char*...
		setattro,				// tp_setattro
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
