#include <Python.h>

#include "metrics.h"


PyObject* say_hello(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);

    Py_RETURN_NONE;
}

PyObject* rmse(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    return PyLong_FromLong(12);
}

static PyMethodDef PricingToolMethods[] =
{
     {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},
     {"rmse", rmse, METH_VARARGS, "Calculate the root mean squared error."},
     {NULL, NULL, 0, NULL}
};

static struct PyModuleDef pricingtoolmodule = {
    PyModuleDef_HEAD_INIT,
    "pricingtool",   /* name of module */
    "Pricing Tool module documentation", /* module documentation */
    1,
    PricingToolMethods
};

PyMODINIT_FUNC
PyInit_pricingtool(void) {
    PyObject* m = PyModule_Create(&pricingtoolmodule);
    if (m == NULL) {
        return NULL;
    }
    return m;
}
