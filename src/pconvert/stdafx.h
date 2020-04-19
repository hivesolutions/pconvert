#pragma once

#include "targetver.h"

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>

#ifdef PCONVERT_EXTENSION
#define PYTHON_27
#define PYTHON_THREADS

#include <Python.h>
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define PWINDOWS 1
#else
#define PUNIX
#endif

#ifdef PWINDOWS
#ifndef NO_PRAGMA_LIB
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#endif
#define FINLINE __inline
#define EXTERNAL_PREFIX __declspec(dllexport)
#endif

#ifdef PUNIX
#define FINLINE inline
#define EXTERNAL_PREFIX extern
#endif

#ifdef PASS_ERROR
#ifndef RUN_ABORT
#define RUN_ABORT while(FALSE)
#endif
#else
#ifndef RUN_ABORT
#define RUN_ABORT exit(0)
#endif
#endif

#include "globals.h"
#include "structs.h"
#include "opencl.h"
