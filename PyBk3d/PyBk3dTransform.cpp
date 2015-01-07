#include "stdafx.h"
#include "PyBk3d.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dTransform
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
		case bk3dlib::UNKNOWN:	return "UNKNOWN";
		}
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
	static PyObject *getChild(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 };
		int childnum = 0;
		if((PyArg_ParseTuple(args, "|i", &childnum) == 0))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad index for child");
			return NULL;
		}
		bk3dlib::PBone pTr = p->p->GetChild(childnum);
		if(!pTr){
			Py_IncRef(Py_None);
			return Py_None;
		}
		return (PyObject*)pTr->GetUserData();
	}
	// --------------------------------------------------
	//
	static PyObject *getParent(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 };
		bk3dlib::PBone pTr = p->p->GetParent();
		if(!pTr)
		{
			Py_IncRef(Py_None);
			return Py_None;
		}
		return (PyObject*)pTr->GetUserData();
	}
	// --------------------------------------------------
	//
	static PyObject *setParent(PyObject *self, PyObject *args)
	{
		PyObj *p = (PyObj *)self;
		PyObj *parentobj = NULL;
		//_asm { int 3 };
		if((PyArg_ParseTuple(args, "O", &parentobj) == 0)||(!PyObject_TypeCheck(parentobj, &classType)))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad parameter Buffer");
			return NULL;
		}
		PBone pT = p->p->GetParent();
		if(pT)
		{
			Py_DecRef((PyObject*)pT->GetUserData());
			Py_DecRef((PyObject*)p);
		}
		p->p->SetParent(parentobj->p);
		Py_IncRef((PyObject*)parentobj);
		Py_IncRef((PyObject*)p);
		Py_IncRef(Py_None);
		return Py_None;
	}
	// --------------------------------------------------
	//
	static PyObject *computeMatrix(PyObject *self)
	{
		PyObj *p = (PyObj *)self;
		//_asm { int 3 };
		p->p->ComputeMatrix();
		Py_IncRef(Py_None);
		return Py_None;
	}
	// --------------------------------------------------
	// Method list
	//
	PyMethodDef methods[] = {
		{"getName",	(PyCFunction)getName, METH_NOARGS, "return the name of the Transform"},
		{"setName",	(PyCFunction)setName, METH_VARARGS, "sets the name of the Transform"},
		{"getChild",	(PyCFunction)getChild, METH_VARARGS, "sets the name of the Transform"},
		{"getParent",	(PyCFunction)getParent, METH_NOARGS, "sets the name of the Transform"},
		{"setParent",	(PyCFunction)setParent, METH_VARARGS, "sets the name of the Transform"},
		{"computeMatrix",	(PyCFunction)computeMatrix, METH_NOARGS, "sets the name of the Transform"},

		////virtual bool connectCurve(PCurveVec pCVec, TransfComponent comp, int compOffset=0) = 0;
		////virtual bool disconnectCurve(PCurveVec pCVec, TransfComponent comp=TRANSF_DEFCOMP, int compOffset=0) = 0;
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
		self->p = Transform::Create(name);
		self->p->SetUserData(self);
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

	PyObject *buildMatrix(float *m)
	{
		PyObject *matobj = PyList_New(4);
		int i=0;
		PyObject *o = PyList_New(4);
		PyList_SetItem(o,0,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,1,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,2,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,3,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(matobj,0, o);
		o = PyList_New(4);
		PyList_SetItem(o,0,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,1,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,2,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,3,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(matobj,1, o);
		o = PyList_New(4);
		PyList_SetItem(o,0,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,1,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,2,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,3,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(matobj,2, o);
		o = PyList_New(4);
		PyList_SetItem(o,0,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,1,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,2,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(o,3,PyFloat_FromDouble((double)m[i++]));
		PyList_SetItem(matobj,3, o);
		return matobj;
	}
	PyObject *buildVector(float *x, float *y, float *z=NULL, float *w=NULL)
	{
		PyObject *o = PyList_New(0);
		if(x) PyList_Append(o,PyFloat_FromDouble((double)*x));
		if(y) PyList_Append(o,PyFloat_FromDouble((double)*y));
		if(z) PyList_Append(o,PyFloat_FromDouble((double)*z));
		if(w) PyList_Append(o,PyFloat_FromDouble((double)*w));
		return o;
	}
	// --------------------------------------------------
	//
	PyObject *getattro(PyObject *self, PyObject *nameobj)
	{
		float x,y,z,w;
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
		else if(PyUnicode_CompareWithASCIIString(nameobj, "parent") == 0)
		{
			return ((PyObject*)o->p->GetUserData());
		}
        // WARNING : do strncmp equivalent !
		else if(PyUnicode_CompareWithASCIIString(nameobj, "pos") == 0)//,3) == 0)
		{
			o->p->GetPos(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scale") == 0)
		{
			o->p->GetScale(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "quat") == 0)//,4) == 0)
		{
			o->p->GetQuaternion(x, y, z, w);
			return buildVector(&x,&y,&z,&w);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotation") == 0)
		{
			o->p->GetRotation(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scalepivot") == 0)
		{
			o->p->GetScalePivot(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scalepivottranslate") == 0)
		{
			o->p->GetScalePivot(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationpivot") == 0)
		{
			o->p->GetRotationPivot(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationpivottranslate") == 0)
		{
			o->p->GetRotationPivotTranslate(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationorientation") == 0)
		{
			o->p->GetRotationOrientation(x, y, z, w);
			return buildVector(&x,&y,&z,&w);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "jointorientation") == 0)
		{
			o->p->GetRotationOrientation(x, y, z, w);
			return buildVector(&x,&y,&z,&w);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrix") == 0)
		{
			float m[16];
			//o->p->ComputeMatrix();
			o->p->GetMatrix(m);
			return buildMatrix(m);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrixbindpose") == 0)
		{
			float m[16];
			//o->p->ComputeMatrix();
			o->p->GetMatrix_Bindpose(m);
			return buildMatrix(m);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrixabs") == 0)
		{
			float m[16];
			//o->p->ComputeMatrix();
			o->p->GetMatrix_Abs(m);
			return buildMatrix(m);
		}
		//
		// methods : seems like if I overloaded the attribute management, I need to forward method search...
		//
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
	bool parseMatrix(PyObject *srcobj, float *m)
	{
		if(!PyList_Check(srcobj))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad parameter for vector");
			//_asm { int 3 };
			return false;
		}
		if(PyList_GET_SIZE(srcobj) != 4)
		{
			PyErr_SetString(PyExc_AttributeError, "Bad vector size in the matrix : should be be 4");
			//_asm { int 3 };
			return false;
		}
		for(int c=0; c<4; c++)
		{
			PyObject *o = PyList_GetItem(srcobj,c);
			if(!PyList_Check(o))
			{
				PyErr_SetString(PyExc_AttributeError, "Bad parameter for vector");
				//_asm { int 3 };
				return false;
			}
			if(PyList_GET_SIZE(o) != 4)
			{
				PyErr_SetString(PyExc_AttributeError, "Bad vector size in the matrix : should be be 4");
				//_asm { int 3 };
				return false;
			}
			for(int l=0; l<4; l++)
			{
				*m++ = (float)PyFloat_AsDouble(PyList_GetItem(o,l));
			}
		}
		return true;
	}

	// --------------------------------------------------
	//
	bool parseVector(PyObject *srcobj, float *x, float *y, float *z=NULL, float *w=NULL)
	{
		// case of a Blender vector
		// We don't know the implementation so let's use attributes with names
		if(PyObject_HasAttrString(srcobj, "x"))
		{
			PyObject *oComp = PyObject_GetAttrString(srcobj, "x");
			if(x) *x = (float)PyFloat_AsDouble(oComp);
			Py_DecRef(oComp);
			if(PyObject_HasAttrString(srcobj, "y"))
			{
				oComp = PyObject_GetAttrString(srcobj, "y");
				if(y) *y = (float)PyFloat_AsDouble(oComp);
				Py_DecRef(oComp);
			}
			if(PyObject_HasAttrString(srcobj, "z"))
			{
				oComp = PyObject_GetAttrString(srcobj, "z");
				if(z) *z = (float)PyFloat_AsDouble(oComp);
				Py_DecRef(oComp);
			}
			if(PyObject_HasAttrString(srcobj, "w"))
			{
				oComp = PyObject_GetAttrString(srcobj, "w");
				if(w) *w = (float)PyFloat_AsDouble(oComp);
				Py_DecRef(oComp);
			}
            return true;
		}
		// case of a list
		if(!PyList_Check(srcobj))
		{
			PyErr_SetString(PyExc_AttributeError, "Bad parameter for vector");
			//_asm { int 3 };
			return false;
		}
		int n  = (int)PyList_GET_SIZE(srcobj);
		if((n >= 1)&&x) *x = (float)PyFloat_AsDouble(PyList_GetItem(srcobj,0));
		if((n >= 2)&&y) *y = (float)PyFloat_AsDouble(PyList_GetItem(srcobj,1));
		if((n >= 3)&&z) *z = (float)PyFloat_AsDouble(PyList_GetItem(srcobj,2));
		if((n >= 4)&&w) *w = (float)PyFloat_AsDouble(PyList_GetItem(srcobj,3));
		return true;
	}

	// --------------------------------------------------
	//
	int setattro(PyObject *self, PyObject *nameobj, PyObject *srcobj)
	{
		float x,y,z,w;
		PyObj *o = (PyObj *)self;
		//
		// Check properties we know
		//
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			o->p->SetName( PySTRING_AsString(*srcobj) );
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "parent") == 0)
		{
			PyObj *p = (PyObj *)self;
			PyObj *parentobj = (PyObj *)srcobj;
			//_asm { int 3 };
			if(!PyObject_TypeCheck(parentobj, &classType))
			{
				PyErr_SetString(PyExc_AttributeError, "Bad parameter for parent");
				//_asm { int 3 };
				return 0;
			}
			PBone pT = p->p->GetParent();
			if(pT)
			{
				Py_DecRef((PyObject*)pT->GetUserData());
				Py_DecRef((PyObject*)p);
			}
			p->p->SetParent(parentobj->p);
			Py_IncRef((PyObject*)parentobj);
			Py_IncRef((PyObject*)p);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "pos") == 0)//,3) == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetPos(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scale") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetScale(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "quat") == 0)//,4) == 0)
		{
			parseVector(srcobj, &x,&y,&z,&w);
			o->p->SetQuaternion(x, y, z, w);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotation") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetRotation(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scalepivot") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetScalePivot(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "scalepivottranslate") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetScalePivot(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationpivot") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetRotationPivot(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationpivottranslate") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->SetRotationPivotTranslate(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "rotationorientation") == 0)
		{
			parseVector(srcobj, &x,&y,&z,&w);
			o->p->SetRotationOrientation(x, y, z, w);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "jointorientation") == 0)
		{
			parseVector(srcobj, &x,&y,&z,&w);
			o->p->SetRotationOrientation(x, y, z, w);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrix") == 0)
		{
			float m[16];
			parseMatrix(srcobj, m);
			o->p->SetMatrix(m);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrixabs") == 0)
		{
			float m[16];
			parseMatrix(srcobj, m);
			o->p->SetAbsMatrix(m);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "matrixbindpose") == 0)
		{
			float m[16];
			parseMatrix(srcobj, m);
			o->p->SetMatrixBindpose(m);
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
		return PyString_FromFormat("{Bk3d Transform %s}", o->p->GetName());
#else
		return PyUnicode_FromFormat("{Bk3d Transform %s}", o->p->GetName());
#endif
	}

	// --------------------------------------------------
	//
	PyObject *tp_str(PyObject *self)
	{
		PyObj *o = (PyObj *)self;
		assert(o->p);
#ifndef IS_PY3K
		PyObject *strobj = PyString_FromFormat(
			"Bk3d Transform named %s\n"
			, o->p->GetName());
#else
		PyObject *strobj = PyUnicode_FromFormat(
			"Bk3d Transform named %s\n"
			, o->p->GetName());
#endif
		return strobj;
	}

	// --------------------------------------------------
	//
	char doc [] =
	"------------------------------\n"
	"- TransformType (Maya style) -\n"
	"------------------------------\n"
	"Properties:\n\n"
	"  - name (RW): Transform Name\n"
	"  - parent (RW): parent transform\n"
	"  - pos (RW): [x,y,z] list for pos\n"
	"  - scale (RW): [x,y,z] list for scale\n"
	"  - quat (RW): [x,y,z,w] list for quaternion\n"
	"  - rotation (RW): [x,y,z] list for Eurler rotation\n"
	"  - scalepivot (RW): [x,y,z] list for pivot\n"
	"  - scalepivottranslate (RW): [x,y,z] list for pivot\n"
	"  - rotationpivot (RW): [x,y,z] list for pivot\n"
	"  - rotationpivottranslate (RW): [x,y,z] list for \n"
	"  - rotationorientation (RW): [x,y,z,w]\n"
	"  - jointorientation (RW): [x,y,z,w]\n"
	"  - matrix (RW): 4x4 list for local matrix\n"
	"  - matrixbindpose (R): 4x4 list for local matrix\n"
	"  - matrixabs (R): 4x4 list for abs matrix\n"
	"\nFunctions:\n\n"
	"  - getName() : return the name of the Transform\n"
	"  - setName() : sets the name of the Transform\n"
	"  - getChild(...) : return a child transform\n"
	"  - getParent() : sets the name of the Transform\n"
	"  - setParent(...) : sets the name of the Transform\n"
	"  - computeMatrix() : sets the name of the Transform\n"
	"\n"
	;

	// --------------------------------------------------
	//
	// bk3d Header Type
	// http://docs.python.org/extending/newtypes.html?highlight=pytypeobject
    //
    // http://docs.python.org/2.7/extending/newtypes.html?highlight=pytypeobject
    // vs.
    // http://docs.python.org/3.3/extending/newtypes.html?highlight=pytypeobject
	//
	PyTypeObject classType = 
	{
#ifdef IS_PY3K
		PyVarObject_HEAD_INIT(&PyType_Type, 0)
#else
		PyObject_HEAD_INIT(&PyType_Type)
		0,
#endif
		"TransformType",
		sizeof(PyObj),  // tp_basicsize
		0,              // tp_itemsize
		dealloc,        //tp_dealloc
		0,              //tp_print,   PyObject_Print - Avoid : it may prevent tp_str / repr to be used
		0,        // tp_getattr - WARNING : cancels tp_methods
		0,        // tp_setattr
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
