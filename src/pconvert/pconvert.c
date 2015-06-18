#include "stdafx.h"

void abort_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    RUN_ABORT;
}

void read_png(char *file_name, char demultiply, struct pcv_image *image) {
    /* allocates space for some of the simple values that are
    going to be used in the image processing */
    int y;

    /* allocates space for the header part of the image so that
    it must be possible to check for the correct png header */
    char header[8];

    /* opens the file and tests for it being a png, this is required
    to avoid possible problems while handling inproper files */
    FILE *fp = fopen(file_name, "rb");
    if(!fp) {
        abort_("[read_png] File %s could not be opened for reading", file_name);
    }
    fread(header, 1, 8, fp);
    if(png_sig_cmp((void *) header, 0, 8)) {
        abort_("[read_png] File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff, this is the structu that will be populated
    withe the complete stat of the png file reading */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!image->png_ptr) {
        abort_("[read_png] png_create_read_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if(!image->info_ptr) {
        abort_("[read_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(image->png_ptr))) {
        abort_("[read_png] Error during init_io");
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
        abort_("[read_png] Error during read_image");
    }

    image->rows = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for(y = 0; y < image->height; y++) {
        image->rows[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
    }

    png_read_image(image->png_ptr, image->rows);
    if(demultiply) { demultiply_image(image); }

    fclose(fp);
}

void write_png(struct pcv_image *image, char multiply, char *file_name) {
    /* allocates space for temporary pointer values to both the global
    png file tables and the (meta-)information tables */
    png_structp png_ptr;
    png_infop info_ptr;

    /* create file, that is going to be used as the target for the
    writting of the final file and the verifies it the open operation
    has been completed with the proper success */
    FILE *fp = fopen(file_name, "wb");
    if(!fp) {
        abort_("[write_png] File %s could not be opened for writing", file_name);
    }

    /* initialize stuff of the main structure, so that it may be used
    latter for the write operation */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        abort_("[write_png] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        abort_("[write_png] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png] Error during init_io");
    }

    if(multiply) { multiply_image(image); }

    png_init_io(png_ptr, fp);

    /* write header */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png] Error during writing header");
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

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png] Error during writing bytes");
    }

    png_write_image(png_ptr, image->rows);

    /* ends the writing process of the png file */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png] Error during end of write");
    }

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

void demultiply_image(struct pcv_image *image) {
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
}

void multiply_image(struct pcv_image *image) {
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
}

void process_image(struct pcv_image *image) {
    int x;
    int y;

    if(png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB) {
        abort_(
            "[process_image] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
            "(lacks the alpha channel)"
        );
    }

    if(png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA) {
        abort_(
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
            this will create a special kind of effect */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
}

void blend_images(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
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
}

void blend_images_debug(struct pcv_image *bottom, struct pcv_image *top, char *algorithm, char *file_path) {
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
}

void release_image(struct pcv_image *image) {
    /* cleanup heap allocation, avoids memory leaks, note that
    the cleanup is performed first on row level and then at a
    row pointer level (two level of allocation) */
    int y;
    for(y = 0; y < image->height; y++) {
        free(image->rows[y]);
    }
    free(image->rows);
    png_destroy_read_struct(&image->png_ptr, &image->info_ptr, NULL);
}

void compose_images(char *base_path, char *algorithm, char *background) {
    char path[1024];
    char name[1024];
    struct pcv_image bottom, top, final;
    char demultiply = is_multiplied(algorithm);
    read_png(join_path(base_path, "sole.png", path), demultiply, &bottom);
    read_png(join_path(base_path, "back.png", path), demultiply, &top);
    blend_images(&bottom, &top, algorithm); release_image(&top);
    read_png(join_path(base_path, "front.png", path), demultiply, &top);
    blend_images(&bottom, &top, algorithm); release_image(&top);
    read_png(join_path(base_path, "shoelace.png", path), demultiply, &top);
    blend_images(&bottom, &top, algorithm); release_image(&top);
    if(demultiply) { multiply_image(&bottom); }
    sprintf(name, "background_%s.png", background);
    read_png(join_path(base_path, name, path), FALSE, &final);
    blend_images(&final, &bottom, "alpha"); release_image(&bottom);
    sprintf(name, "result_%s_%s.png", algorithm, background);
    write_png(&final, FALSE, join_path(base_path, name, path));
    release_image(&final);
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

    if(argc != 3) { abort_("Usage: pconvert convert <file_in> <file_out>"); }

    read_png(argv[1], FALSE, &image);
    process_image(&image);
    write_png(&image, FALSE, argv[2]);
    release_image(&image);

    return 0;
}

int main(int argc, char **argv) {
    if(argc < 2) { abort_("Usage: pconvert <command> [args...]"); }
    if(strcmp(argv[1], "compose") == 0) { return pcompose(argc, argv); }
    else if(strcmp(argv[1], "convert") == 0) { return pconvert(argc, argv); }
    else { abort_("Usage: pconvert <command> [args...]"); return 0; }
}
