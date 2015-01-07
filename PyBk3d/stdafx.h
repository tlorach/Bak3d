// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "bk3dlib.h"
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#else
#include "Python.h"
#endif
#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#pragma message("Using Python 3")
#define PySTRING_FromString PyUnicode_FromString
inline const char* PySTRING_AsString(PyObject srcobj)
{
    PyObject *exc_type = NULL, *exc_value = NULL, *exc_tb = NULL;
    PyErr_Fetch(&exc_type, &exc_value, &exc_tb);
    PyObject* str_exc_type = PyObject_Repr(exc_type); //Now a unicode
    PyObject* pyStr = PyUnicode_AsEncodedString(str_exc_type, "utf-8",
    "Error ~");
    const char *strExcType = PyBytes_AS_STRING(pyStr);
    Py_XDECREF(str_exc_type);
    Py_XDECREF(pyStr);

    Py_XDECREF(exc_type);
    Py_XDECREF(exc_value);
    Py_XDECREF(exc_tb);
    return strExcType;
}
#define PyLI_FromLong PyLong_FromLong
#else
#define PySTRING_FromString PyString_FromString
#define PyLI_FromLong PyInt_FromLong
#endif

#pragma warning(disable:4996)
