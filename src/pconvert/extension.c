#include "stdafx.h"

PyObject *extension_register(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
};

PyObject *extension_unregister(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
};

PyObject *extension_blend_images(PyObject *self, PyObject *args) {
    char demultiply;
    char *bottom_path, *top_path, *target_path, *algorithm;
    struct pcv_image bottom, top;

    if(PyArg_ParseTuple(args, "sss|s", &bottom_path, &top_path, &target_path, &algorithm) == 0) { return NULL; }

    algorithm = algorithm == NULL ? "multiplicative" : algorithm;
    demultiply = is_multiplied(algorithm);

    read_png(bottom_path, demultiply, &bottom);
    read_png(top_path, demultiply, &top);
    blend_images(&bottom, &top, algorithm);
    write_png(&bottom, demultiply, target_path);
    release_image(&top);
    release_image(&bottom);

    Py_RETURN_NONE;
};

PyObject *extension_blend_multiple(PyObject *self, PyObject *args) {
    char demultiply;
    char *bottom_path, *top_path, *target_path, *algorithm;
    struct pcv_image bottom, top;
    PyObject *paths, *iterator, *element, *first, *second;
    Py_ssize_t size;

#if PY_MAJOR_VERSION >= 3
    PyObject *encoded;
#endif

    if(PyArg_ParseTuple(args, "Os|s", &paths, &target_path, &algorithm) == 0) { return NULL; }

    algorithm = algorithm == NULL ? "multiplicative" : algorithm;
    demultiply = is_multiplied(algorithm);

    size = PyList_Size(paths);
    if(size < 2) { Py_RETURN_NONE; }

    first = PyList_GetItem(paths, 0);
    second = PyList_GetItem(paths, 1);

#if PY_MAJOR_VERSION >= 3
    first = PyUnicode_EncodeFSDefault(first);
    second = PyUnicode_EncodeFSDefault(second);
    bottom_path = PyBytes_AsString(first);
    top_path = PyBytes_AsString(second);
#else
    bottom_path = PyString_AsString(first);
    top_path = PyString_AsString(second);
#endif

    read_png(bottom_path, demultiply, &bottom);
    read_png(top_path, demultiply, &top);
    blend_images(&bottom, &top, algorithm);
    release_image(&top);

#if PY_MAJOR_VERSION >= 3
    Py_DECREF(first);
    Py_DECREF(second);
#endif

    iterator = PyObject_GetIter(paths);

    PyIter_Next(iterator);
    PyIter_Next(iterator);

    while(TRUE) {
        element = PyIter_Next(iterator);
        if(element == NULL) { break; }
#if PY_MAJOR_VERSION >= 3
        encoded = PyUnicode_EncodeFSDefault(element);
        top_path = PyBytes_AsString(encoded);
#else
        top_path = PyString_AsString(element);
#endif
        read_png(top_path, demultiply, &top);
        blend_images(&bottom, &top, algorithm);
        release_image(&top);
        Py_DECREF(element);
#if PY_MAJOR_VERSION >= 3
        Py_DECREF(encoded);
#endif
    }

    Py_DECREF(iterator);

    write_png(&bottom, demultiply, target_path);
    release_image(&bottom);

    Py_RETURN_NONE;
};

PyMethodDef pconvert_functions[4] = {
    {
        "blend_images",
        extension_blend_images,
        METH_VARARGS,
        NULL
    },
    {
        "blend_multiple",
        extension_blend_multiple,
        METH_VARARGS,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        NULL
    }
};

#if PY_MAJOR_VERSION >= 3
    struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "pconvert",
        "PNG convertion module",
        -1,
        pconvert_functions,
        NULL,
        NULL,
        NULL,
        NULL,
    };
#endif

#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_pconvert(void) {
    // allocates space for the module object to hold the
    // module to be created
    PyObject *pconvert_module;

    // creates the pconvert extension module with the
    // functions defined in the previous array
    pconvert_module = PyModule_Create(&moduledef);
    if(pconvert_module == NULL) { return; }
}
#else
PyMODINIT_FUNC initpconvert(void) {
    // allocates space for the module object to hold the
    // module to be created
    PyObject *pconvert_module;

    // creates the pconvert extension module with the
    // functions defined in the previous array
    pconvert_module = Py_InitModule("pconvert", pconvert_functions);
    if(pconvert_module == NULL) { return; }
}
#endif
