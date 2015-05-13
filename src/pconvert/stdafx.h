#pragma once

#include "targetver.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <png.h>

#define PYTHON_26
#define PYTHON_THREADS

#include <Python.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")
#endif

#ifndef RUN_ABORT
#define RUN_ABORT exit(0)
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

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

void read_png(char *file_name, struct pcv_image *image);
void write_png(struct pcv_image *image, char *file_name);
void process_image(struct pcv_image *image);
void blend_images(struct pcv_image *bottom, struct pcv_image *top, char *algorithm);
void release_image(struct pcv_image *image);
void compose_images(char *base_path, char *algorithm, char *background);
char *join_path(char *base, char *extra, char *result);
void blend_multiplicative(
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
