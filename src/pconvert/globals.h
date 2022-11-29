#pragma once

#include "structs.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define PCONVERT_VERSION "0.4.5"
#define PCONVERT_COMPILATION_DATE __DATE__
#define PCONVERT_COMPILATION_TIME __TIME__

#ifdef _MSC_VER
#define PCONVERT_PLATFORM_MSC true
#define PCONVERT_COMPILER "msvc"
#define PCONVERT_COMPILER_VERSION _MSC_VER
#define PCONVERT_COMPILER_VERSION_STRING TOSTRING(_MSC_VER)
#endif

#ifdef __GNUC__
#define PCONVERT_PLATFORM_GCC true
#define PCONVERT_COMPILER "gcc"
#define PCONVERT_COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#define PCONVERT_COMPILER_VERSION_STRING __VERSION__
#endif

#define PCONVERT_PLATFORM_CPU_BITS sizeof(void *) * 8

#ifdef PCONVERT_OPENCL
#define PCONVERT_OPENCL_V 1
#define PCONVERT_OPENCL_S " opencl"
#else
#define PCONVERT_OPENCL_V 0
#define PCONVERT_OPENCL_S ""
#endif

#ifdef PCONVERT_EXTENSION
#define PCONVERT_EXTENSION_V 1
#define PCONVERT_EXTENSION_S " python"
#else
#define PCONVERT_EXTENSION_V 0
#define PCONVERT_EXTENSION_S ""
#endif

#define PCONVERT_FEATURES "cpu" PCONVERT_OPENCL_S PCONVERT_EXTENSION_S

#define PCONVERT_ALGORITHMS "multiplicative", "source_over", "destination_over",\
    "mask_top", "first_top", "first_bottom", "disjoint_over", "disjoint_under", "disjoint_debug"

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
#define RAISE_M(message) set_last_error(message); return ERROR
#define RAISE_F(message, ...) set_last_error_f(message, __VA_ARGS__); return ERROR
#define EXCEPT_P(input) if(IS_ERROR(input)) {\
    print_(last_error == NULL ? "Unknown error" : last_error);\
} while(FALSE)
#define EXCEPT_S(input) if(IS_ERROR(input)) {\
    print_(last_error == NULL ? "Unknown error" : last_error);\
    return ERROR;\
} while(FALSE)
#define EXCEPT_M(input, ...) if(IS_ERROR(input)) {\
    __VA_ARGS__;\
    print_(last_error == NULL ? "Unknown error" : last_error);\
    return ERROR;\
} while(FALSE)
#define NORMAL return NO_ERROR
#define IS_ERROR(input) input != NO_ERROR
#define VALIDATE(input) if(IS_ERROR(input)) { RAISE; } while(FALSE)
#define VALIDATE_R(input, ...) if(IS_ERROR(input)) { return __VA_ARGS__; } while(FALSE)
#define VALIDATE_A(input, ...) if(IS_ERROR(input)) { __VA_ARGS__; } while(FALSE)
#define VALIDATE_PY(input, ...) if(IS_ERROR(input)) {\
    __VA_ARGS__;\
    PyErr_SetString(PyExc_TypeError, last_error == NULL ? "Unknown error" : last_error);\
    return NULL;\
} while(FALSE)

#define BENCHMARK(benchmark, start, target, ...)\
    start = (float) clock() / CLOCKS_PER_SEC;\
    __VA_ARGS__;\
    if (benchmark != NULL) { target += ((float) clock() / CLOCKS_PER_SEC) - start; }\
    while(FALSE)

#define Z_NO_COMPRESSION 0
#define Z_BEST_SPEED 1
#define Z_BEST_COMPRESSION 9
#define Z_DEFAULT_COMPRESSION (-1)

#define MAX_ERROR_L 1024

EXTERNAL_PREFIX char *last_error;
EXTERNAL_PREFIX char last_error_b[MAX_ERROR_L];

typedef struct pcv_image {
    int width;
    int height;
    png_byte color_type;
    png_byte bit_depth;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *rows;
} pcv_image_t;

typedef struct benchmark {
    float blend_time;
    float read_png_time;
    float write_png_time;
} benchmark_t;

typedef void (blend_algorithm) (
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);

void print_(const char *s, ...);
void abort_(const char *s, ...);
void set_last_error_f(char *message, ...);
ERROR_T libpng_version(char *buffer);
ERROR_T read_png(char *file_name, char demultiply, struct pcv_image *image);
ERROR_T write_png(struct pcv_image *image, char multiply, char *file_name);
ERROR_T write_png_extra(
    struct pcv_image *image,
    char multiply,
    char *file_name,
    int compression,
    int filter
);
ERROR_T demultiply_image(struct pcv_image *image);
ERROR_T multiply_image(struct pcv_image *image);
ERROR_T process_image(struct pcv_image *image);
ERROR_T blend_images(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
);
ERROR_T blend_images_i(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
);
ERROR_T blend_images_source_over_fast(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
);
ERROR_T blend_images_destination_over_fast(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
);
ERROR_T blend_images_debug(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params,
    char *file_path
);
ERROR_T blend_images_extra(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params,
    char use_opencl
);
ERROR_T release_image(struct pcv_image *image);
ERROR_T release_image_s(struct pcv_image *image, char destroy_struct);
ERROR_T copy_image(struct pcv_image *origin, struct pcv_image *target);
ERROR_T duplicate_image(struct pcv_image *origin, struct pcv_image *target);
ERROR_T compose_images(
    char *base_path,
    char *algorithm,
    params *params,
    char *background,
    struct benchmark *benchmark
);
ERROR_T compose_images_extra(
    char *base_path,
    char *algorithm,
    params *params,
    char *background,
    int compression,
    int filter,
    char use_opencl,
    struct benchmark *benchmark
);
char *join_path(char *base, char *extra, char *result);
blend_algorithm *get_blend_algorithm(char *algorithm);
char is_multiplied(char *algorithm);

void blend_alpha(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_multiplicative(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_source_over(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_destination_over(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_mask_top(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_first_top(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_first_bottom(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_under(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_over(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);
void blend_disjoint_debug(
    params *params,
    png_byte *result,
    png_byte rb, png_byte gb, png_byte bb, png_byte ab,
    png_byte rt, png_byte gt, png_byte bt, png_byte at
);

static FINLINE void blend_source_over_i(
    params *params,
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

static FINLINE void set_last_error(char *message, ...) {
    last_error = message;
}
