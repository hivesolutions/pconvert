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

    if(PyArg_ParseTuple(args, "ssss", &bottom_path, &top_path, &target_path, &algorithm) == 0) { return NULL; }

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

PyMethodDef pconvert_functions[4] = {
    {
        "blend_images",
        extension_blend_images,
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

PyMODINIT_FUNC initpconvert(void) {
    // allocates space for the module object to hold the
    // module to be created
    PyObject *pconvert_module;

    // creates the pconvert extension module with the
    // functions defined in the previous array
    pconvert_module = Py_InitModule("pconvert", pconvert_functions);
    if(pconvert_module == NULL) { return; }
}

PyMODINIT_FUNC PyInit_pconvert(void) {
	return initpconvert();
}