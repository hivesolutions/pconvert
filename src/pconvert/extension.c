#include "stdafx.h"

PyObject *extension_register(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
};

PyObject *extension_unregister(PyObject *self, PyObject *args) {
    Py_RETURN_NONE;
};

PyObject *extension_blend_images(PyObject *self, PyObject *args) {
    int run_inline;
    char demultiply, source_over;
    char *bottom_path, *top_path, *target_path, *algorithm;
    PyObject *is_inline;
    struct pcv_image bottom, top;

    if(PyArg_ParseTuple(
        args,
        "sss|sO",
        &bottom_path,
        &top_path,
        &target_path,
        &algorithm,
        &is_inline
    ) == 0) { return NULL; }

    algorithm = algorithm == NULL ? "multiplicative" : algorithm;
    run_inline = is_inline == NULL ? 0 : PyBool_Check(is_inline);
    demultiply = is_multiplied(algorithm);
    source_over = strcmp(algorithm, "source_over") == 0;

    VALIDATE_A(read_png(bottom_path, demultiply, &bottom), Py_RETURN_NONE);
    VALIDATE_A(read_png(top_path, demultiply, &top), Py_RETURN_NONE);
    if(source_over == TRUE) {
        VALIDATE_A(blend_images_fast(&bottom, &top, algorithm), Py_RETURN_NONE);
    } else if(run_inline == TRUE) {
        VALIDATE_A(blend_images_i(&bottom, &top, algorithm), Py_RETURN_NONE);
    } else {
        VALIDATE_A(blend_images(&bottom, &top, algorithm), Py_RETURN_NONE);
    }

    VALIDATE_A(write_png(&bottom, demultiply, target_path), Py_RETURN_NONE);
    VALIDATE_A(release_image(&top), Py_RETURN_NONE);
    VALIDATE_A(release_image(&bottom), Py_RETURN_NONE);

    Py_RETURN_NONE;
};

PyObject *extension_blend_multiple(PyObject *self, PyObject *args) {
    int run_inline, cache, destroy_struct;
    char demultiply, source_over;
    char *bottom_path, *top_path, *target_path, *algorithm;
    struct pcv_image bottom, top;
    struct pcv_image *bottom_cache, *top_cache;
    PyObject *paths, *iterator, *element, *first, *second, *is_inline, *use_cache;
    Py_ssize_t size;

#if PY_MAJOR_VERSION >= 3
    PyObject *encoded;
#endif

    if(PyArg_ParseTuple(
        args,
        "Os|sOO",
        &paths,
        &target_path,
        &algorithm,
        &is_inline,
        &use_cache
    ) == 0) { return NULL; }

    algorithm = algorithm == NULL ? "multiplicative" : algorithm;
    run_inline = is_inline != NULL && PyObject_IsTrue(is_inline) == TRUE ? TRUE : FALSE;
    cache = use_cache != NULL && PyObject_IsTrue(use_cache) == TRUE ? TRUE : FALSE;
    demultiply = is_multiplied(algorithm);
    source_over = strcmp(algorithm, "source_over") == 0;
    destroy_struct = cache == TRUE ? FALSE : TRUE;

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

    bottom_cache = (struct pcv_image *) value_map(bottom_path);
    if(cache && bottom_cache != NULL) {
        VALIDATE_A(duplicate_image(bottom_cache, &bottom), Py_RETURN_NONE);
    } else {
        VALIDATE_A(read_png(bottom_path, demultiply, &bottom), Py_RETURN_NONE);
    }

    top_cache = (struct pcv_image *) value_map(top_path);
    if(cache && top_cache != NULL ) {
        VALIDATE_A(duplicate_image(top_cache, &top), Py_RETURN_NONE);
    } else {
        VALIDATE_A(read_png(top_path, demultiply, &top), Py_RETURN_NONE);
    }

    if(cache && bottom_cache == NULL) {
        bottom_cache = malloc(sizeof(struct pcv_image));
        duplicate_image(&bottom, bottom_cache);
        set_map(bottom_path, bottom_cache);
    }
    if(cache && top_cache == NULL) {
        top_cache = malloc(sizeof(struct pcv_image));
        duplicate_image(&top, top_cache);
        set_map(top_path, top_cache);
    }

    if(source_over == TRUE) {
        VALIDATE_A(blend_images_fast(&bottom, &top, algorithm), Py_RETURN_NONE);
    } else if(run_inline == TRUE) {
        VALIDATE_A(blend_images_i(&bottom, &top, algorithm), Py_RETURN_NONE);
    } else {
        VALIDATE_A(blend_images(&bottom, &top, algorithm), Py_RETURN_NONE);
    }
    VALIDATE_A(release_image_s(&top, destroy_struct), Py_RETURN_NONE);

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

        top_cache = (struct pcv_image *) value_map(top_path);
        if(cache && top_cache != NULL) {
            VALIDATE_A(duplicate_image(top_cache, &top), Py_RETURN_NONE);
        } else {
            VALIDATE_A(read_png(top_path, demultiply, &top), Py_RETURN_NONE);
        }
        VALIDATE_A(read_png(top_path, demultiply, &top), Py_RETURN_NONE);

        if(cache && top_cache == NULL) {
            top_cache = malloc(sizeof(struct pcv_image));
            duplicate_image(&top, top_cache);
            set_map(top_path, top_cache);
        }

        if(source_over == TRUE) {
            VALIDATE_A(blend_images_fast(&bottom, &top, algorithm), Py_RETURN_NONE);
        } else if(run_inline == TRUE) {
            VALIDATE_A(blend_images_i(&bottom, &top, algorithm), Py_RETURN_NONE);
        } else {
            VALIDATE_A(blend_images(&bottom, &top, algorithm), Py_RETURN_NONE);
        }
        VALIDATE_A(release_image_s(&top, destroy_struct), Py_RETURN_NONE);
        Py_DECREF(element);
#if PY_MAJOR_VERSION >= 3
        Py_DECREF(encoded);
#endif
    }

    Py_DECREF(iterator);

    VALIDATE_A(write_png(&bottom, demultiply, target_path), Py_RETURN_NONE);
    VALIDATE_A(release_image_s(&bottom, destroy_struct), Py_RETURN_NONE);

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
    // functions defined in the previous array, in case
    // no module is created the control flow is returned
    // immediately to the caller function
    pconvert_module = PyModule_Create(&moduledef);
    if(pconvert_module == NULL) { return NULL; }

    // returns the module that has just been created to
    // the caller method/function (to be used)
    return pconvert_module;
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
