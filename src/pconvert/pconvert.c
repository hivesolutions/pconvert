#include "stdafx.h"

char *last_error = NULL;
char last_error_b[MAX_ERROR_L] = "";

void print_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void abort_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    RUN_ABORT;
}

void set_last_error_f(char *message, ...) {
    va_list args;
    va_start(args, message);
    vsprintf(last_error_b, message, args);
    va_end(args);
    last_error = last_error_b;
}

ERROR_T libpng_version(char *buffer) {
    /* allocates space for the version number (as an integer)
    for the libpng library */
    png_uint_32 version;
    int major, medium, minor;
    char version_s[128], major_s[3], medium_s[3], minor_s[3];

    /* retrieves the libpng version value and then extracts
    the multiple parts from it */
    version = png_access_version_number();
    sprintf(version_s, "%06d", (int) version);

    memcpy(major_s, &version_s[0], 2);
    memcpy(medium_s, &version_s[2], 2);
    memcpy(minor_s, &version_s[4], 2);

    major_s[2] = '\0';
    medium_s[2] = '\0';
    minor_s[2] = '\0';

    major = atoi(major_s);
    medium = atoi(medium_s);
    minor = atoi(minor_s);

    sprintf((char *) buffer, "%d.%d.%d", major, medium, minor);

    NORMAL;
}

ERROR_T read_png(char *file_name, char demultiply, struct pcv_image *image) {
    /* allocates space for some of the simple values that are
    going to be used in the image processing */
    int y;

    /* allocates space for the header part of the image so that
    it must be possible to check for the correct PNG header */
    char header[8];

    /* allocates space for the pointer to the file that is going to
    be used in the reading process of the file */
    FILE *fp;

    /* allocates space for the counter that is going to be used for
    some of the reading operation (required) */
    size_t count;

    /* creates space for the buffer reference that is going to be used
    for the complete buffer of allocated data and for the number value
    of bytes that are going to be contained on that buffer */
    png_byte *buffer;
    png_uint_32 buffer_size;
    png_uint_32 row_size;

    /* opens the file and tests for proper opening, this is required
    to avoid possible problems while handling improper file reading */
#ifdef _MSC_VER
    wchar_t file_name_w[1024];
    swprintf(file_name_w, 1024, L"\\\\?\\%hs", file_name);
    fp = _wfopen(file_name_w, L"rb");
#else
    fp = fopen(file_name, "rb");
#endif
    if(!fp) {
        RAISE_F("[read_png] File %s could not be opened for reading", file_name);
    }

    /* tries to read the PNG signature from the file and in case it's
    not valid raises an error indicating so */
    count = fread(header, 1, 8, fp);
    if(png_sig_cmp((void *) header, 0, 8) || count != 8) {
        RAISE_F("[read_png] File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff, this is the structu that will be populated
    withe the complete stat of the PNG file reading */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!image->png_ptr) {
        RAISE_M("[read_png] png_create_read_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if(!image->info_ptr) {
        RAISE_M("[read_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(image->png_ptr))) {
        RAISE_M("[read_png] Error during init_io");
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);
    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    /* reads the complete file value in file, meaning that
    from this point on only decompression is remaining */
    if(setjmp(png_jmpbuf(image->png_ptr))) {
        RAISE_M("[read_png] Error during read_image");
    }

    /* allocates space in memory for the buffer that will
    be used for the "frame" buffer of the image data */
    row_size = (png_uint_32) png_get_rowbytes(image->png_ptr, image->info_ptr);
    buffer_size = row_size * image->height;
    buffer = (png_byte *) malloc(buffer_size);

    /* allocates space for the buffer of row pointers and
    iterates over the complete height to allocate sub-buffers */
    image->rows = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for(y = 0; y < image->height; y++) {
        image->rows[y] = buffer;
        buffer += row_size;
    }

    png_read_image(image->png_ptr, image->rows);
    if(demultiply) { demultiply_image(image); }

    /* closes the file pointer as no more reading is going
    to take place (as expected) avoiding leaks */
    fclose(fp);
    NORMAL;
}

ERROR_T write_png(struct pcv_image *image, char multiply, char *file_name) {
    return write_png_extra(image, multiply, file_name, Z_BEST_SPEED, PNG_FILTER_NONE);
}

ERROR_T write_png_extra(
    struct pcv_image *image,
    char multiply,
    char *file_name,
    int compression,
    int filter
) {
    /* allocates space for the pointer to the file that is going to
    be used in the writing process of the file */
    FILE *fp;

    /* allocates space for temporary pointer values to both the global
    PNG file tables and the (meta-)information tables */
    png_structp png_ptr;
    png_infop info_ptr;

    /* create file, that is going to be used as the target for the
    writting of the final file and the verifies it the open operation
    has been completed with the proper success */
#ifdef _MSC_VER
    wchar_t file_name_w[1024];
    swprintf(file_name_w, 1024, L"\\\\?\\%hs", file_name);
    fp = _wfopen(file_name_w, L"wb");
#else
    fp = fopen(file_name, "wb");
#endif
    if(!fp) {
        RAISE_F("[write_png] File %s could not be opened for writing", file_name);
    }

    /* initialize stuff of the main structure, so that it may be used
    latter for the write operation */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        RAISE_M("[write_png] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        RAISE_M("[write_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during init_io");
    }

    /* in case the multiply mode is defined the image structure
    is changed for proper multiplication */
    if(multiply) { multiply_image(image); }

    /* initializes the process of output of the file an runs
    the error checking verification process */
    png_init_io(png_ptr, fp);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during writing header");
    }

    png_set_IHDR(
        png_ptr,
        info_ptr,
        image->width,
        image->height,
        image->bit_depth,
        image->color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    /* writes the "header" information described by the
    info pointer into the PNG structure */
    png_write_info(png_ptr, info_ptr);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during writing bytes");
    }

    png_set_compression_level(png_ptr, compression);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during writing bytes");
    }

    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during writing bytes");
    }

    /* writes the complete set of bytes that are considered
    to be part of the image into PNG structure definition */
    png_write_image(png_ptr, image->rows);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_M("[write_png] Error during end of write");
    }

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    NORMAL;
}

ERROR_T demultiply_image(struct pcv_image *image) {
    int x, y;
    float af;
    png_byte r, g, b, a;

    for(y = 0; y < image->height; y++) {
        png_byte *row = image->rows[y];
        for(x = 0; x < image->width; x++) {
            png_byte *pixel = &(row[x * 4]);

            r = *pixel;
            g = *(pixel + 1);
            b = *(pixel + 2);
            a = *(pixel + 3);
            af = 1.0f * (a / 255.0f);

            r = (png_byte) ROUND(r * af);
            g = (png_byte) ROUND(g * af);
            b = (png_byte) ROUND(b * af);

            *pixel = r;
            *(pixel + 1) = g;
            *(pixel + 2) = b;
        }
    }
    NORMAL;
}

ERROR_T multiply_image(struct pcv_image *image) {
    int x, y;
    float af;
    png_byte r, g, b, a;

    for(y = 0; y < image->height; y++) {
        png_byte *row = image->rows[y];
        for(x = 0; x < image->width; x++) {
            png_byte *pixel = &(row[x * 4]);

            r = *pixel;
            g = *(pixel + 1);
            b = *(pixel + 2);
            a = *(pixel + 3);
            af = 1.0f * (a / 255.0f);

            r = (png_byte) ROUND(r / af);
            g = (png_byte) ROUND(g / af);
            b = (png_byte) ROUND(b / af);

            *pixel = r;
            *(pixel + 1) = g;
            *(pixel + 2) = b;
        }
    }
    NORMAL;
}

ERROR_T process_image(struct pcv_image *image) {
    int x;
    int y;

    if(png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB) {
        RAISE_M(
            "[process_image] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
            "(lacks the alpha channel)"
        );
    }

    if(png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA) {
        RAISE_F(
            "[process_image] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
            PNG_COLOR_TYPE_RGBA,
            png_get_color_type(image->png_ptr, image->info_ptr)
        );
    }

    /* iterates over the complete buffer for the image pixels to be
    able to print some information about the provided image */
    for(y = 0; y < image->height; y++) {
        png_byte *row = image->rows[y];
        for(x = 0; x < image->width; x++) {
            png_byte *ptr = &(row[x * 4]);

            /* verifies if the current iteration is valid for debug
            information printing and if that's the case prints it */
            int is_valid = x % 100 == 0;
            if(is_valid) {
                printf(
                    "Pixel at position [ %d - %d ] has RGBA values: (%d,%d,%d,%d)\n",
                    x,
                    y,
                    ptr[0],
                    ptr[1],
                    ptr[2],
                    ptr[3]
                );
            }

            /* sets red value to 0 and green value to the blue one,
            this will create a special kind of effect making the image
            with a "blueish" effect on every valid pixel (blue filter) */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
    NORMAL;
}

ERROR_T blend_images_extra(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params,
    char use_opencl
) {
    ERROR_T err;

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images_extra] Inconsistent image sizes");
    }

    if(use_opencl == TRUE) {
        err = blend_images_opencl(bottom, top, algorithm, params);
    } else {
        err = blend_images(bottom, top, algorithm, params);
    }
    return err;
}

ERROR_T blend_images(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;
    blend_algorithm *operation = get_blend_algorithm(algorithm);

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images] Inconsistent image sizes");
    }

    for(y = 0; y < bottom->height; y++) {
        png_byte *rowBottom = bottom->rows[y];
        png_byte *rowTop = top->rows[y];
        for(x = 0; x < bottom->width; x++) {
            png_byte *ptrBottom = &(rowBottom[x * 4]);
            png_byte *ptrTop = &(rowTop[x * 4]);

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            rt = *ptrTop;
            gt = *(ptrTop + 1);
            bt = *(ptrTop + 2);
            at = *(ptrTop + 3);

            operation(
                params,
                ptrBottom,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );
        }
    }
    NORMAL;
}

ERROR_T blend_images_i(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images_i] Inconsistent image sizes");
    }

    for(y = 0; y < bottom->height; y++) {
        png_byte *rowBottom = bottom->rows[y];
        png_byte *rowTop = top->rows[y];
        for(x = 0; x < bottom->width; x++) {
            png_byte *ptrBottom = &(rowBottom[x * 4]);
            png_byte *ptrTop = &(rowTop[x * 4]);

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            rt = *ptrTop;
            gt = *(ptrTop + 1);
            bt = *(ptrTop + 2);
            at = *(ptrTop + 3);

            blend_source_over_i(
                params,
                ptrBottom,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );
        }
    }
    NORMAL;
}

ERROR_T blend_images_source_over_fast(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
) {
    int x, y;
    float abf, atf, af;
    png_byte r, g, b, a;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images_source_over_fast] Inconsistent image sizes");
    }

    for(y = 0; y < bottom->height; y++) {
        png_byte *rowBottom = bottom->rows[y];
        png_byte *rowTop = top->rows[y];
        for(x = 0; x < bottom->width; x++) {
            png_byte *ptrBottom = &(rowBottom[x * 4]);
            png_byte *ptrTop = &(rowTop[x * 4]);

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            rt = *ptrTop;
            gt = *(ptrTop + 1);
            bt = *(ptrTop + 2);
            at = *(ptrTop + 3);

            abf = 1.0f * (ab / 255.0f);
            atf = 1.0f * (at / 255.0f);
            af = abf + atf * (1.0f - abf);

            r = af == 0.0f ? 0 : (png_byte) ((rb * abf + rt * atf * (1.0f - abf)) / af);
            g = af == 0.0f ? 0 : (png_byte) ((gb * abf + gt * atf * (1.0f - abf)) / af);
            b = af == 0.0f ? 0 : (png_byte) ((bb * abf + bt * atf * (1.0f - abf)) / af);
            a = MAX(0, MIN(255, (png_byte) (af * 255.0f)));

            r = MAX(0, MIN(255, r));
            g = MAX(0, MIN(255, g));
            b = MAX(0, MIN(255, b));

            *ptrBottom = r;
            *(ptrBottom + 1) = g;
            *(ptrBottom + 2) = b;
            *(ptrBottom + 3) = a;
        }
    }
    NORMAL;
}

ERROR_T blend_images_destination_over_fast(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params
) {
    int x, y;
    float abf, atf, af;
    png_byte r, g, b, a;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images_destination_over_fast] Inconsistent image sizes");
    }

    for(y = 0; y < bottom->height; y++) {
        png_byte *rowBottom = bottom->rows[y];
        png_byte *rowTop = top->rows[y];
        for(x = 0; x < bottom->width; x++) {
            png_byte *ptrBottom = &(rowBottom[x * 4]);
            png_byte *ptrTop = &(rowTop[x * 4]);

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            rt = *ptrTop;
            gt = *(ptrTop + 1);
            bt = *(ptrTop + 2);
            at = *(ptrTop + 3);

            abf = 1.0f * (ab / 255.0f);
            atf = 1.0f * (at / 255.0f);
            af = atf + abf * (1.0f - atf);

            r = af == 0.0f ? 0 : (png_byte) ((rt * atf + rb * abf * (1.0f - atf)) / af);
            g = af == 0.0f ? 0 : (png_byte) ((gt * atf + gb * abf * (1.0f - atf)) / af);
            b = af == 0.0f ? 0 : (png_byte) ((bt * atf + bb * abf * (1.0f - atf)) / af);
            a = MAX(0, MIN(255, (png_byte) (af * 255.0f)));

            r = MAX(0, MIN(255, r));
            g = MAX(0, MIN(255, g));
            b = MAX(0, MIN(255, b));

            *ptrBottom = r;
            *(ptrBottom + 1) = g;
            *(ptrBottom + 2) = b;
            *(ptrBottom + 3) = a;
        }
    }
    NORMAL;
}

ERROR_T blend_images_debug(
    struct pcv_image *bottom,
    struct pcv_image *top,
    char *algorithm,
    params *params,
    char *file_path
) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;
    blend_algorithm *operation = get_blend_algorithm(algorithm);

    if(bottom->width != top->width || bottom->height != top->height) {
        RAISE_M("[blend_images_debug] Inconsistent image sizes");
    }

    FILE *file = fopen(file_path, "wb");

    for(y = 0; y < bottom->height; y++) {
        png_byte *rowBottom = bottom->rows[y];
        png_byte *rowTop = top->rows[y];
        for(x = 0; x < bottom->width; x++) {
            png_byte *ptrBottom = &(rowBottom[x * 4]);
            png_byte *ptrTop = &(rowTop[x * 4]);

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            rt = *ptrTop;
            gt = *(ptrTop + 1);
            bt = *(ptrTop + 2);
            at = *(ptrTop + 3);

            fprintf(
                file,
                "%04dx%04d - #%02x%02x%02x%02x + #%02x%02x%02x%02x = ",
                x, y,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );

            operation(
                NULL,
                ptrBottom,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );

            rb = *ptrBottom;
            gb = *(ptrBottom + 1);
            bb = *(ptrBottom + 2);
            ab = *(ptrBottom + 3);

            fprintf(
                file,
                "#%02x%02x%02x%02x\n",
                rb, gb, bb, ab
            );
        }
    }

    fclose(file);
    NORMAL;
}

ERROR_T release_image(struct pcv_image *image) {
    return release_image_s(image, TRUE);
}

ERROR_T release_image_s(struct pcv_image *image, char destroy_struct) {
    /* cleanup heap allocation, avoids memory leaks, note that
    the cleanup is performed first on row level and then at a
    row pointer level (two levels of allocation) */
    free(*image->rows);
    free(image->rows);
    if(destroy_struct) {
        png_destroy_read_struct(&image->png_ptr, &image->info_ptr, NULL);
    }
    NORMAL;
}

ERROR_T copy_image(struct pcv_image *origin, struct pcv_image *target) {
    target->width = origin->width;
    target->height = origin->height;
    target->color_type = origin->color_type;
    target->bit_depth = origin->bit_depth;
    target->png_ptr = origin->png_ptr;
    target->info_ptr = origin->info_ptr;
    target->rows = origin->rows;
    NORMAL;
}

ERROR_T duplicate_image(struct pcv_image *origin, struct pcv_image *target) {
    int y;
    png_uint_32 rows_size = sizeof(png_bytep) * origin->height;
    png_uint_32 row_size = (png_uint_32) png_get_rowbytes(origin->png_ptr, origin->info_ptr);
    png_uint_32 buffer_size = row_size * origin->height;
    png_byte *buffer = (png_byte *) malloc(buffer_size);
    VALIDATE(copy_image(origin, target));
    target->rows = (png_bytep *) malloc(rows_size);
    memcpy(buffer, *origin->rows, buffer_size);
    for(y = 0; y < origin->height; y++) {
        target->rows[y] = buffer;
        buffer += row_size;
    }
    NORMAL;
}

ERROR_T compose_images(
    char *base_path,
    char *algorithm,
    params *params,
    char *background,
    struct benchmark *benchmark
) {
    return compose_images_extra(
        base_path,
        algorithm,
        params,
        background,
        Z_BEST_SPEED,
        PNG_FILTER_NONE,
        FALSE,
        benchmark
    );
}

ERROR_T compose_images_extra(
    char *base_path,
    char *algorithm,
    params *params,
    char *background,
    int compression,
    int filter,
    char use_opencl,
    struct benchmark *benchmark
) {
    char path[1024];
    char name[1024];
    float start_time;
    struct pcv_image bottom, top, final;
    char demultiply = is_multiplied(algorithm);

    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(read_png(join_path(base_path, "sole.png", path), demultiply, &bottom));
        VALIDATE(read_png(join_path(base_path, "back.png", path), demultiply, &top))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->blend_time,
        VALIDATE(blend_images_extra(&bottom, &top, algorithm, params, use_opencl))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(release_image(&top))
    );

    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(read_png(join_path(base_path, "front.png", path), demultiply, &top))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->blend_time,
        VALIDATE(blend_images_extra(&bottom, &top, algorithm, params, use_opencl))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(release_image(&top))
    );

    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(read_png(join_path(base_path, "shoelace.png", path), demultiply, &top))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->blend_time,
        VALIDATE(blend_images_extra(&bottom, &top, algorithm, params, use_opencl))
    );
    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(release_image(&top))
    );

    BENCHMARK(
        benchmark, start_time, benchmark->blend_time,
        if(demultiply) { multiply_image(&bottom); }
    );

    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        sprintf(name, "background_%s.png", background);
        VALIDATE(read_png(join_path(base_path, name, path), FALSE, &final));
    );
    BENCHMARK(
        benchmark, start_time, benchmark->blend_time,
        VALIDATE(blend_images_extra(&final, &bottom, algorithm, params, use_opencl));
    );
    BENCHMARK(
        benchmark, start_time, benchmark->read_png_time,
        VALIDATE(release_image(&bottom))
    );

    BENCHMARK(
        benchmark, start_time, benchmark->write_png_time,
        sprintf(name, "result_%s_%s_%s.png", algorithm, background, use_opencl ? "opencl" : "cpu");
        VALIDATE(write_png_extra(&final, FALSE, join_path(base_path, name, path), compression, filter));
        VALIDATE(release_image(&final))
    );

    NORMAL;
}

ERROR_T pcompose(int argc, char **argv) {
    if(argc != 3) { RAISE_M("Usage: pconvert compose <directory>"); }

    VALIDATE(compose_images(argv[2], "alpha", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "alpha", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "alpha", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "alpha", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "multiplicative", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "multiplicative", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "multiplicative", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "multiplicative", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "source_over", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "source_over", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "source_over", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "source_over", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "destination_over", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "destination_over", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "destination_over", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "destination_over", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "first_top", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "first_top", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "first_top", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "first_top", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "first_bottom", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "first_bottom", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "first_bottom", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "first_bottom", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_over", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_over", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_over", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_over", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_under", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_under", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_under", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_under", NULL, "texture", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_debug", NULL, "alpha", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_debug", NULL, "white", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_debug", NULL, "blue", NULL));
    VALIDATE(compose_images(argv[2], "disjoint_debug", NULL, "texture", NULL));

    NORMAL;
}

ERROR_T pconvert(int argc, char **argv) {
    struct pcv_image image;

    if(argc != 4) { RAISE_M("Usage: pconvert convert <file_in> <file_out>"); }

    VALIDATE(read_png(argv[2], FALSE, &image));
    VALIDATE(process_image(&image));
    VALIDATE(write_png(&image, FALSE, argv[3]));
    VALIDATE(release_image(&image));

    NORMAL;
}

float pbenchmark_algorithm(
    char *base_path,
    char *algorithm,
    params *params,
    char *background,
    int compression,
    int filter,
    char use_opencl,
    struct benchmark *benchmark
) {
    float start_time;
    float end_time;
    float time_elapsed;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    EXCEPT_S(compose_images_extra(
        base_path, algorithm, params, background, compression,
        filter, use_opencl, benchmark
    ));
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed = end_time - start_time;
    return time_elapsed;
}

ERROR_T pbenchmark(int argc, char **argv) {
    #define ALGORITHMS_SIZE 5
    #define COMPRESSION_SIZE 3

    struct benchmark benchmark, benchmark_total = { 0, 0, 0};
    size_t index, index_j, index_k, index_c;
    float time, result, total = 0.0f;
    char label[128];
    char time_s[64];
    char count = 1;
    char is_success = TRUE;
    char details = TRUE;

    char *algorithms[ALGORITHMS_SIZE] = {
        "multiplicative",
        "source_over",
        "alpha",
        "disjoint_over",
        "disjoint_under"
    };
    int compression[COMPRESSION_SIZE] = {
        Z_NO_COMPRESSION,
        Z_BEST_SPEED,
        Z_BEST_COMPRESSION
    };
    char *compression_s[COMPRESSION_SIZE] = {
        "Z_NO_COMPRESSION",
        "Z_BEST_SPEED",
        "Z_BEST_COMPRESSION"
    };

#ifdef PCONVERT_OPENCL
    #define OPENCL_SIZE 2
    char use_opencl[OPENCL_SIZE] = { FALSE, TRUE };
    char *use_opencl_s[OPENCL_SIZE] = { "CPU", "OPENCL" };
#else
    #define OPENCL_SIZE 1
    char use_opencl[OPENCL_SIZE] = { FALSE };
    char *use_opencl_s[OPENCL_SIZE] = { "CPU" };
#endif

    if(argc < 3) { RAISE_M("Usage: pconvert benchmark <directory> [count]"); }
    if(argc > 3) { count = atoi(argv[3]); }

    for(index = 0; index < ALGORITHMS_SIZE; index++) {
        for(index_j = 0; index_j < COMPRESSION_SIZE; index_j++) {
            for(index_k = 0; index_k < OPENCL_SIZE; index_k++) {
                time = 0.0f;
                benchmark.blend_time = 0;
                benchmark.read_png_time = 0;
                benchmark.write_png_time = 0;
                sprintf(label, "%s %s %s", algorithms[index], compression_s[index_j], use_opencl_s[index_k]);
                printf("%-42s ", label);
                fflush(stdout);
                for(index_c = 0; index_c < (size_t) count; index_c++) {
                    result = pbenchmark_algorithm(
                        argv[2], algorithms[index], NULL, "alpha",
                        compression[index_j], 0, use_opencl[index_k],
                        &benchmark
                    );
                    time += result;
                    total += result;
                    is_success = result >= 0.0f;
                    if (!is_success) break;
                }
                if (is_success) sprintf(time_s, "%0.2fms", time * 1000.0f);
                else sprintf(time_s, "ERROR!");
                printf("%s", time_s);
                if(details == TRUE && is_success == TRUE) {
                    printf(
                        " (blend %0.2fms, read %0.2fms, write %0.2fms)",
                        benchmark.blend_time * 1000.0f,
                        benchmark.read_png_time * 1000.0f,
                        benchmark.write_png_time * 1000.0f
                    );
                }
                benchmark_total.blend_time += benchmark.blend_time;
                benchmark_total.read_png_time += benchmark.read_png_time;
                benchmark_total.write_png_time += benchmark.read_png_time;
                printf("\n");
            }
        }
        printf("\n");
    }

    printf("%-42s %0.2fms", "total_execution", total * 1000.0f);
    if(details == TRUE) {
        printf(
            " (blend %0.2fms, read %0.2fms, write %0.2fms)",
            benchmark_total.blend_time * 1000.0f,
            benchmark_total.read_png_time * 1000.0f,
            benchmark_total.write_png_time * 1000.0f
        );
    }
    printf("\n");

    NORMAL;
}

ERROR_T popencl(int argc, char **argv) {
    float start_time;
    float end_time;
    float time_elapsed_cpu;
    float time_elapsed_gpu;
    struct benchmark benchmark_cpu = { 0, 0, 0 };
    struct benchmark benchmark_gpu = { 0, 0, 0 };

    if(argc != 3) { RAISE_M("Usage: pconvert opencl <directory>"); }

    time_elapsed_cpu = 0;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    VALIDATE(compose_images_extra(argv[2], "multiplicative", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE, &benchmark_cpu));
    VALIDATE(compose_images_extra(argv[2], "source_over", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE, &benchmark_cpu));
    VALIDATE(compose_images_extra(argv[2], "alpha", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE, &benchmark_cpu));
    VALIDATE(compose_images_extra(argv[2], "disjoint_over", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE, &benchmark_cpu));
    VALIDATE(compose_images_extra(argv[2], "disjoint_under", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE, &benchmark_cpu));
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed_cpu = end_time - start_time;

    time_elapsed_gpu = 0;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    VALIDATE(compose_images_extra(argv[2], "multiplicative", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE, &benchmark_gpu));
    VALIDATE(compose_images_extra(argv[2], "source_over", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE, &benchmark_gpu));
    VALIDATE(compose_images_extra(argv[2], "alpha", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE, &benchmark_gpu));
    VALIDATE(compose_images_extra(argv[2], "disjoint_over", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE, &benchmark_gpu));
    VALIDATE(compose_images_extra(argv[2], "disjoint_under", NULL, "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE, &benchmark_gpu));
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed_gpu = end_time - start_time;

    printf("CPU: %f GPU: %f\n", time_elapsed_cpu, time_elapsed_gpu);

    NORMAL;
}

ERROR_T pversion(int argc, char **argv) {
    char libpng_version_s[16];
    libpng_version(libpng_version_s);
    printf(
        "P(NG)Convert %s (%s %s) [%s %s %d bit] [libpng %s] [%s]\n",
        PCONVERT_VERSION,
        PCONVERT_COMPILATION_DATE,
        PCONVERT_COMPILATION_TIME,
        PCONVERT_COMPILER,
        PCONVERT_COMPILER_VERSION_STRING,
        (int) PCONVERT_PLATFORM_CPU_BITS,
        libpng_version_s,
        PCONVERT_FEATURES
    );
    printf("Copyright (c) 2008-2020 Hive Solutions Lda. All rights reserved.\n");
    NORMAL;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        abort_("Usage: pconvert <command> [args...]");
        return ERROR;
    }

    if(strcmp(argv[1], "compose") == 0) {
        EXCEPT_S(pcompose(argc, argv));
    } else if(strcmp(argv[1], "convert") == 0) {
        EXCEPT_S(pconvert(argc, argv));
    } else if(strcmp(argv[1], "benchmark") == 0) {
        EXCEPT_S(pbenchmark(argc, argv));
    } else if(strcmp(argv[1], "opencl") == 0) {
        EXCEPT_S(popencl(argc, argv));
    } else if(strcmp(argv[1], "version") == 0) {
        EXCEPT_S(pversion(argc, argv));
    } else {
        abort_("Usage: pconvert <command> [args...]");
        return ERROR;
    }

    return NO_ERROR;
}
