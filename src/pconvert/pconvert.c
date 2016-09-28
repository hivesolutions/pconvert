#include "stdafx.h"

void abort_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    RUN_ABORT;
}

ERROR_T read_png(char *file_name, char demultiply, struct pcv_image *image) {
    /* allocates space for some of the simple values that are
    going to be used in the image processing */
    int y;

    /* allocates space for the header part of the image so that
    it must be possible to check for the correct png header */
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

    /* opens the file and tests for it being a png, this is required
    to avoid possible problems while handling inproper files */
#ifdef _MSC_VER
    wchar_t file_name_w[1024];
    swprintf(file_name_w, 1024, L"\\\\?\\%hs", file_name);
    fp = _wfopen(file_name_w, L"rb");
#else
    fp = fopen(file_name, "rb");
#endif
    if(!fp) {
        RAISE_S("[read_png] File %s could not be opened for reading", file_name);
    }
    count = fread(header, 1, 8, fp);
    if(png_sig_cmp((void *) header, 0, 8) || count != 8) {
        RAISE_S("[read_png] File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff, this is the structu that will be populated
    withe the complete stat of the png file reading */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!image->png_ptr) {
        RAISE_S("[read_png] png_create_read_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if(!image->info_ptr) {
        RAISE_S("[read_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(image->png_ptr))) {
        RAISE_S("[read_png] Error during init_io");
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
        RAISE_S("[read_png] Error during read_image");
    }

    /* allocates space in memory for the buffer that will
    be used for the "frame" buffer of the image data */
    row_size = png_get_rowbytes(image->png_ptr, image->info_ptr);
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
    png file tables and the (meta-)information tables */
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
        RAISE_S("[write_png] File %s could not be opened for writing", file_name);
    }

    /* initialize stuff of the main structure, so that it may be used
    latter for the write operation */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        RAISE_S("[write_png] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        RAISE_S("[write_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during init_io");
    }

    /* in case the multiply mode is defined the image structure
    is changed for proper multiplication */
    if(multiply) { multiply_image(image); }

    /* initializes the process of output of the file an runs
    the error checking verification process */
    png_init_io(png_ptr, fp);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during writing header");
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
    info pointer into the png structure */
    png_write_info(png_ptr, info_ptr);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during writing bytes");
    }

    png_set_compression_level(png_ptr, compression);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during writing bytes");
    }

    png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during writing bytes");
    }

    /* writes the complete set of bytes that are considered
    to be part of the image into png structure definition */
    png_write_image(png_ptr, image->rows);
    if(setjmp(png_jmpbuf(png_ptr))) {
        RAISE_S("[write_png] Error during end of write");
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
        RAISE_S(
            "[process_image] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
            "(lacks the alpha channel)"
        );
    }

    if(png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA) {
        RAISE_S(
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

ERROR_T blend_images_extra(struct pcv_image *bottom, struct pcv_image *top, char *algorithm, char use_opencl) {
    ERROR_T err;
    if(use_opencl == TRUE) {
        err = blend_images_opencl(bottom, top, algorithm);
    } else {
        err = blend_images(bottom, top, algorithm);
    }
    return err;
}

ERROR_T blend_images(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;
    blend_algorithm *operation = get_blend_algorithm(algorithm);

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
                ptrBottom,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );
        }
    }
    NORMAL;
}

ERROR_T blend_images_i(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;

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
                ptrBottom,
                rb, gb, bb, ab,
                rt, gt, bt, at
            );
        }
    }
    NORMAL;
}

ERROR_T blend_images_fast(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    int x, y;
    float abf, atf, af;
    png_byte r, g, b, a;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;

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

ERROR_T blend_images_debug(struct pcv_image *bottom, struct pcv_image *top, char *algorithm, char *file_path) {
    int x, y;
    png_byte rb, gb, bb, ab;
    png_byte rt, gt, bt, at;
    blend_algorithm *operation = get_blend_algorithm(algorithm);
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
    png_uint_32 row_size = png_get_rowbytes(origin->png_ptr, origin->info_ptr);
    png_uint_32 buffer_size = row_size * origin->height;
    png_byte *buffer = (png_byte *) malloc(buffer_size);
    copy_image(origin, target);
    target->rows = (png_bytep *) malloc(rows_size);
    memcpy(buffer, *origin->rows, buffer_size);
    for(y = 0; y < origin->height; y++) {
        target->rows[y] = buffer;
        buffer += row_size;
    }
    NORMAL;
}

ERROR_T compose_images(char *base_path, char *algorithm, char *background) {
    return compose_images_extra(
        base_path,
        algorithm,
        background,
        Z_BEST_SPEED,
        PNG_FILTER_NONE,
        FALSE
    );
}

ERROR_T compose_images_extra(
    char *base_path,
    char *algorithm,
    char *background,
    int compression,
    int filter,
    char use_opencl
) {
    char path[1024];
    char name[1024];
    struct pcv_image bottom, top, final;
    char demultiply = is_multiplied(algorithm);
    read_png(join_path(base_path, "sole.png", path), demultiply, &bottom);
    read_png(join_path(base_path, "back.png", path), demultiply, &top);
    blend_images_extra(&bottom, &top, algorithm, use_opencl); release_image(&top);
    read_png(join_path(base_path, "front.png", path), demultiply, &top);
    blend_images_extra(&bottom, &top, algorithm, use_opencl); release_image(&top);
    read_png(join_path(base_path, "shoelace.png", path), demultiply, &top);
    blend_images_extra(&bottom, &top, algorithm, use_opencl); release_image(&top);
    if(demultiply) { multiply_image(&bottom); }
    sprintf(name, "background_%s.png", background);
    read_png(join_path(base_path, name, path), FALSE, &final);
    blend_images_extra(&final, &bottom, "alpha", use_opencl); release_image(&bottom);
    sprintf(name, "result_%s_%s.png", algorithm, background);
    write_png_extra(&final, FALSE, join_path(base_path, name, path), compression, filter);
    release_image(&final);

    NORMAL;
}

int pcompose(int argc, char **argv) {
    if(argc != 3) { abort_("Usage: pconvert compose <directory>"); }

    compose_images(argv[2], "alpha", "alpha");
    compose_images(argv[2], "alpha", "white");
    compose_images(argv[2], "alpha", "blue");
    compose_images(argv[2], "alpha", "texture");
    compose_images(argv[2], "multiplicative", "alpha");
    compose_images(argv[2], "multiplicative", "white");
    compose_images(argv[2], "multiplicative", "blue");
    compose_images(argv[2], "multiplicative", "texture");
    compose_images(argv[2], "source_over", "alpha");
    compose_images(argv[2], "source_over", "white");
    compose_images(argv[2], "source_over", "blue");
    compose_images(argv[2], "source_over", "texture");
    compose_images(argv[2], "disjoint_over", "alpha");
    compose_images(argv[2], "disjoint_over", "white");
    compose_images(argv[2], "disjoint_over", "blue");
    compose_images(argv[2], "disjoint_over", "texture");
    compose_images(argv[2], "disjoint_under", "alpha");
    compose_images(argv[2], "disjoint_under", "white");
    compose_images(argv[2], "disjoint_under", "blue");
    compose_images(argv[2], "disjoint_under", "texture");
    compose_images(argv[2], "disjoint_debug", "alpha");
    compose_images(argv[2], "disjoint_debug", "white");
    compose_images(argv[2], "disjoint_debug", "blue");
    compose_images(argv[2], "disjoint_debug", "texture");

    return 0;
}

int pconvert(int argc, char **argv) {
    struct pcv_image image;

    if(argc != 4) { abort_("Usage: pconvert convert <file_in> <file_out>"); }

    read_png(argv[2], FALSE, &image);
    process_image(&image);
    write_png(&image, FALSE, argv[3]);
    release_image(&image);

    return 0;
}

float pbenchmark_algorithm(
    char *base_path,
    char *algorithm,
    char *background,
    int compression,
    int filter
) {
    float start_time;
    float end_time;
    float time_elapsed;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    compose_images_extra(base_path, algorithm, background, compression, filter, FALSE);
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed = end_time - start_time;
    return time_elapsed;
}

int pbenchmark(int argc, char **argv) {
    float time;
    FILE *file;

    if(argc != 3) { abort_("Usage: pconvert benchmark <directory>"); }

    file = fopen("benchmark.txt", "wb");

    time = pbenchmark_algorithm(argv[2], "source_over", "alpha", Z_NO_COMPRESSION, 0);
    fprintf(file, "source_over Z_NO_COMPRESSION: %f\n", time);

    time = pbenchmark_algorithm(argv[2], "source_over", "alpha", Z_BEST_SPEED, 0);
    fprintf(file, "source_over Z_BEST_SPEED: %f\n", time);

    time = pbenchmark_algorithm(argv[2], "source_over", "alpha", Z_BEST_COMPRESSION, 0);
    fprintf(file, "source_over Z_BEST_COMPRESSION: %f\n", time);

    fprintf(file, "\n");

    time = pbenchmark_algorithm(argv[2], "multiplicative", "alpha", Z_NO_COMPRESSION, 0);
    fprintf(file, "multiplicative Z_NO_COMPRESSION: %f\n", time);

    time = pbenchmark_algorithm(argv[2], "multiplicative", "alpha", Z_BEST_SPEED, 0);
    fprintf(file, "multiplicative Z_BEST_SPEED: %f\n", time);

    time = pbenchmark_algorithm(argv[2], "multiplicative", "alpha", Z_BEST_COMPRESSION, 0);
    fprintf(file, "multiplicative Z_BEST_COMPRESSION: %f\n", time);

    fclose(file);

    return 0;
}

int popencl(int argc, char **argv) {
    float start_time;
    float end_time;
    float time_elapsed_cpu;
    float time_elapsed_gpu;

    if(argc != 3) { abort_("Usage: pconvert opencl <directory>"); }

    time_elapsed_cpu = 0;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    compose_images_extra(argv[2], "multiplicative", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE);
    compose_images_extra(argv[2], "source_over", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE);
    compose_images_extra(argv[2], "alpha", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE);
    compose_images_extra(argv[2], "disjoint_over", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE);
    compose_images_extra(argv[2], "disjoint_under", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, FALSE);
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed_cpu = end_time - start_time;

    time_elapsed_gpu = 0;
    start_time = (float) clock() / CLOCKS_PER_SEC;
    compose_images_extra(argv[2], "multiplicative", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE);
    compose_images_extra(argv[2], "source_over", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE);
    compose_images_extra(argv[2], "alpha", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE);
    compose_images_extra(argv[2], "disjoint_over", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE);
    compose_images_extra(argv[2], "disjoint_under", "alpha", Z_BEST_SPEED, PNG_FILTER_NONE, TRUE);
    end_time = (float) clock() / CLOCKS_PER_SEC;
    time_elapsed_gpu = end_time - start_time;

    printf("CPU: %f GPU: %f\n", time_elapsed_cpu, time_elapsed_gpu);

    NORMAL;
}

int main(int argc, char **argv) {
    if(argc < 2) { abort_("Usage: pconvert <command> [args...]"); }
    if(strcmp(argv[1], "compose") == 0) { return pcompose(argc, argv); }
    else if(strcmp(argv[1], "convert") == 0) { return pconvert(argc, argv); }
    else if(strcmp(argv[1], "benchmark") == 0) { return pbenchmark(argc, argv); }
    else if(strcmp(argv[1], "opencl") == 0) { return popencl(argc, argv); }
    else { abort_("Usage: pconvert <command> [args...]"); return 0; }
}
