#include "stdafx.h"
#include "PyBk3d.h"


//////////////////////////////////////////////////////////////////////////////////////////////
//
// HEADER
//
//////////////////////////////////////////////////////////////////////////////////////////////
namespace bk3dMaterial
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
	// Method list
	//
	PyMethodDef methods[] = {
		{"getName",	(PyCFunction)getName, METH_NOARGS, "return the name of the Material"},
		{"setName",	(PyCFunction)setName, METH_VARARGS, "sets the name of the Material"},
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
		self->p = Material::Create(name);
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
		float x,y,z;
		char tname[200];
        Py_UNICODE *name;
        name = PyUnicode_AS_UNICODE(nameobj);
		PyObj *o = (PyObj *)self;
		bool bRes = false;
		//
		// Check properties we know
		//
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			return PySTRING_FromString(o->p->GetName());
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "diffuse") == 0)
		{
			o->p->getDiffuse(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "ambient") == 0)
		{
			o->p->getAmbient(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transparency") == 0)
		{
			o->p->getTransparency(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specular") == 0)
		{
			o->p->getSpecular(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "diffuse") == 0)
		{
			o->p->getDiffuse(x, y, z);
			return buildVector(&x,&y,&z);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specexp") == 0)
		{
			o->p->getSpecexp(x);
			return PyFloat_FromDouble((double)x);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "reflectivity") == 0)
		{
			o->p->getReflectivity(x);
			return PyFloat_FromDouble((double)x);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "translucency") == 0)
		{
			o->p->getTranslucency(x);
			return PyFloat_FromDouble((double)x);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "shader") == 0)
		{
			o->p->getShaderName(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "technique") == 0)
		{
			o->p->getShaderName(NULL, 0, tname, 199);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "textureDiffuse") == 0)
		{
			bRes = o->p->getDiffuseTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specexptexture") == 0)
		{
			bRes = o->p->getSpecExpTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "ambienttexture") == 0)
		{
			bRes = o->p->getAmbientTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "reflectivitytexture") == 0)
		{
			bRes = o->p->getReflectivityTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transparencytexture") == 0)
		{
			bRes = o->p->getTransparencyTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transluencytexture") == 0)
		{
			bRes = o->p->getTranslucencyTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "speculartexture") == 0)
		{
			bRes = o->p->getSpecularTexture(tname, 199, NULL, 0);
			return PySTRING_FromString(tname);
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
	bool parseVector(PyObject *srcobj, float *x, float *y, float *z=NULL, float *w=NULL)
	{
		if(!PyList_Check(srcobj))
		{
			// case of a Blender vector
			// We don't know the implementation so let's use attributes with names
			if(PyObject_HasAttrString(srcobj, "r"))
			{
				PyObject *oComp = PyObject_GetAttrString(srcobj, "r");
				float fVal = (float)PyFloat_AsDouble(oComp);
                if(x) *x = fVal;
				Py_DecRef(oComp);
				if(PyObject_HasAttrString(srcobj, "g"))
				{
					oComp = PyObject_GetAttrString(srcobj, "g");
					fVal = (float)PyFloat_AsDouble(oComp);
                    if(y) *y = fVal;
					Py_DecRef(oComp);
				}
				if(PyObject_HasAttrString(srcobj, "b"))
				{
					oComp = PyObject_GetAttrString(srcobj, "b");
					fVal = (float)PyFloat_AsDouble(oComp);
                    if(z) *z = fVal;
					Py_DecRef(oComp);
				}
				if(PyObject_HasAttrString(srcobj, "a"))
				{
					oComp = PyObject_GetAttrString(srcobj, "a");
					fVal = (float)PyFloat_AsDouble(oComp);
                    if(w) *w = fVal;
					Py_DecRef(oComp);
				}
                return true;
            } else {
			    PyErr_SetString(PyExc_AttributeError, "Bad parameter for vector");
			    //_asm { int 3 };
			    return false;
            }
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
		float x,y,z;
		PyObj *o = (PyObj *)self;
		//
		// Check properties we know
		//
		if(PyUnicode_CompareWithASCIIString(nameobj, "name") == 0)
		{
			o->p->SetName( PySTRING_AsString(*srcobj) );
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "diffuse") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->setDiffuse(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "ambient") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->setAmbient(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specular") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->setSpecular(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transparency") == 0)
		{
			parseVector(srcobj, &x,&y,&z);
			o->p->setTransparency(x, y, z);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specexp") == 0)
		{
			if(PyArg_ParseTuple(srcobj, "f", &x))
				o->p->setSpecexp(x);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "reflectivity") == 0)
		{
			if(PyArg_ParseTuple(srcobj, "f", &x))
				o->p->setReflectivity(x);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "translucency") == 0)
		{
			if(PyArg_ParseTuple(srcobj, "f", &x))
				o->p->setTranslucency(x);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "shader") == 0)
		{
			LPCSTR name,tech;
			if(PyArg_ParseTuple(srcobj, "s", &name))
				o->p->setShaderName(name,NULL);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "shader") == 0)
		{
			LPCSTR tech;
			if(PyArg_ParseTuple(srcobj, "s", &tech))
				o->p->setShaderName(NULL,tech);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "diffusetexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setDiffuseTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "specexptexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setSpecExpTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "ambienttexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setAmbientTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "reflectivitytexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setReflectivityTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transparencytexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setTransparencyTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "transluencytexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setTranslucencyTexture(texname, texname);
			return 0;
		}
		else if(PyUnicode_CompareWithASCIIString(nameobj, "speculartexture") == 0)
		{
			LPCSTR texname;
			if(PyArg_ParseTuple(srcobj, "s", &texname))
				o->p->setSpecularTexture(texname, texname);
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
		return PyUnicode_FromFormat("{Bk3d Material %s}", o->p->GetName());
#else
		return PyString_FromFormat("{Bk3d Material %s}", o->p->GetName());
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
			"Bk3d Material named %s\n"
			, o->p->GetName());
#else
		PyObject *strobj = PyUnicode_FromFormat(
			"Bk3d Material named %s\n"
			, o->p->GetName());
#endif
		return strobj;
	}

	// --------------------------------------------------
	//
	char doc [] =
	"----------------\n"
	"- MaterialType -\n"
	"----------------\n"
	"Properties:\n\n"
	"  - name (RW)\n"
	"  - diffuse (RW) : [r,g,b]\n"
	"  - ambient (RW) : [r,g,b]\n"
	"  - transparency (RW) : [r,g,b]\n"
	"  - reflectivity (RW) : float\n"
	"  - specular (RW) : [r,g,b]\n"
	"  - diffuse (RW) : [r,g,b]\n"
	"  - specexp (RW) : float\n"
	"  - translucency (RW) : float\n"
	"  - shader (RW) : 'name'\n"
	"  - technique (RW) : 'techname'\n"
	"  - textureDiffuse (RW) : 'texturename'\n"
	"  - specexptexture (RW) : 'texturename'\n"
	"  - ambienttexture (RW) : 'texturename'\n"
	"  - reflectivitytexture (RW) : 'texturename'\n"
	"  - transparencytexture (RW) : 'texturename'\n"
	"  - transluencytexture (RW) : 'texturename'\n"
	"  - speculartexture (RW) : 'texturename'\n"
	"\nFunctions\n\n"
	"  - getName() : return the name of the Material\n"
	"  - setName() : sets the name of the Material\n"
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
		"MaterialType",
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
