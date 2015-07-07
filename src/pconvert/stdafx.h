#pragma once

#include "targetver.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>

#define PYTHON_26
#define PYTHON_THREADS

#include <Python.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define PWINDOWS 1
#else
#define PUNIX
#endif

#ifdef PWINDOWS
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#define FINLINE __inline
#endif

#ifdef PUNIX
#define FINLINE inline
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

#define TRUE 1
#define FALSE 0

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define ROUND(x) floor(x + 0.5)

#define ERROR_T int
#define ERROR -1
#define NO_ERROR 0
#define RAISE return ERROR
#define RAISE_S(...) abort_(__VA_ARGS__); return ERROR
#define NORMAL return NO_ERROR
#define IS_ERROR(input) input != NO_ERROR
#define VALIDATE(input) if(IS_ERROR(input)) { RAISE; } while(FALSE)
#define VALIDATE_R(input, return_v) if(IS_ERROR(input)) { return return_v; } while(FALSE)
#define VALIDATE_A(input, action) if(IS_ERROR(input)) { action; } while(FALSE)

typedef struct pcv_image {
    int width;
    int height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *rows;
} pcv_image_t;

typedef void (blend_algorithm) (
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);

void abort_(const char *s, ...);
ERROR_T read_png(char *file_name, char demultiply, struct pcv_image *image);
ERROR_T write_png(struct pcv_image *image, char multiply, char *file_name);
ERROR_T demultiply_image(struct pcv_image *image);
ERROR_T multiply_image(struct pcv_image *image);
ERROR_T process_image(struct pcv_image *image);
ERROR_T blend_images(struct pcv_image *bottom, struct pcv_image *top, char *algorithm);
ERROR_T blend_images_i(struct pcv_image *bottom, struct pcv_image *top, char *algorithm);
ERROR_T blend_images_debug(struct pcv_image *bottom, struct pcv_image *top, char *algorithm, char *file_path);
ERROR_T release_image(struct pcv_image *image);
ERROR_T compose_images(char *base_path, char *algorithm, char *background);
char *join_path(char *base, char *extra, char *result);
blend_algorithm *get_blend_algorithm(char *algorithm);
char is_multiplied(char *algorithm);
void blend_alpha(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_multiplicative(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_source_over(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_under(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_over(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_debug(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);

static FINLINE void blend_source_over_i(
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
) {
    png_byte r, g, b, a;

    float abf = 1.0f * (ab / 255.0f);
    float atf = 1.0f * (at / 255.0f);
    float af = abf + atf * (1.0f - abf);

    r = af == 0.0f ? 0 : (png_byte) ((rb * abf + rt * atf * (1.0f - abf)) / af);
    g = af == 0.0f ? 0 : (png_byte) ((gb * abf + gt * atf * (1.0f - abf)) / af);
    b = af == 0.0f ? 0 : (png_byte) ((bb * abf + bt * atf * (1.0f - abf)) / af);
    a = MAX(0, MIN(255, (png_byte) (af * 255.0f)));

    r = MAX(0, MIN(255, r));
    g = MAX(0, MIN(255, g));
    b = MAX(0, MIN(255, b));

    *result = r;
    *(result + 1) = g;
    *(result + 2) = b;
    *(result + 3) = a;
}
