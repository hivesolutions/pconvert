#include "stdafx.h"

void abort_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void read_png(char *file_name, struct pcv_image *image) {
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
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    }
    fread(header, 1, 8, fp);
    if(png_sig_cmp((void *) header, 0, 8)) {
        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
    }

    /* initialize stuff, this is the structu that will be populated
    withe the complete stat of the png file reading */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!image->png_ptr) {
        abort_("[read_png_file] png_create_read_struct failed");
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if(!image->info_ptr) {
        abort_("[read_png_file] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(image->png_ptr))) {
        abort_("[read_png_file] Error during init_io");
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
        abort_("[read_png_file] Error during read_image");
    }

    image->rows = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for(y = 0; y < image->height; y++) {
        image->rows[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));
    }

    png_read_image(image->png_ptr, image->rows);
    fclose(fp);
}

void write_png(struct pcv_image *image, char *file_name) {
    /* allocates space for temporary pointer values to both the global
    png file tables and the (meta-)information tables */
    png_structp png_ptr;
    png_infop info_ptr;

    /* create file, that is going to be used as the target for the
    writting of the final file and the verifies it the open operation
    has been completed with the proper success */
    FILE *fp = fopen(file_name, "wb");
    if(!fp) {
        abort_("[write_png_file] File %s could not be opened for writing", file_name);
    }

    /* initialize stuff of the main structure, so that it may be used
    latter for the write operation */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        abort_("[write_png_file] png_create_write_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        abort_("[write_png_file] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fp);

    /* write header */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png_file] Error during writing header");
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
        abort_("[write_png_file] Error during writing bytes");
    }

    png_write_image(png_ptr, image->rows);

    /* end write */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png_file] Error during end of write");
    }

    png_write_end(png_ptr, NULL);
    fclose(fp);
}

void process_image(struct pcv_image *image) {
    int x;
    int y;

    if(png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB) {
        abort_(
            "[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
            "(lacks the alpha channel)"
        );
    }

    if(png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA) {
        abort_(
            "[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
            PNG_COLOR_TYPE_RGBA,
            png_get_color_type(image->png_ptr, image->info_ptr)
        );
    }

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

void blend_images(struct pcv_image *bottom, struct pcv_image *top) {
    int x, y;
	float atf;
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

            atf = 1.0f * (at / 255.0f);

            r = (png_byte) (rb * (1 - atf) + rt * atf);
            g = (png_byte) (gb * (1 - atf) + gt * atf);
            b = (png_byte) (bb * (1 - atf) + bt * atf);
            a = MAX(0, MIN(255, at + ab));

            r = MAX(0, MIN(255, r));
            g = MAX(0, MIN(255, g));
            b = MAX(0, MIN(255, b));

			*ptrBottom = r;
			*(ptrBottom + 1) = g;
			*(ptrBottom + 2) = b;
			*(ptrBottom + 3) = a;
        }
    }
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
}

void swear_demo() {
	struct pcv_image bottom, top;
	read_png("C:/repo.extra/swear/src/swear/static/demo/background_alpha.png", &bottom);
	read_png("C:/repo.extra/swear/src/swear/static/demo/front.png", &top);
	blend_images(&bottom, &top);
	read_png("C:/repo.extra/swear/src/swear/static/demo/top.png", &top);
	blend_images(&bottom, &top);
	read_png("C:/repo.extra/swear/src/swear/static/demo/shoelace.png", &top);
	blend_images(&bottom, &top);
	read_png("C:/repo.extra/swear/src/swear/static/demo/sole.png", &top);
	blend_images(&bottom, &top);
	write_png(&bottom, "C:/repo.extra/swear/src/swear/static/demo/result.png");
	release_image(&top);
	release_image(&bottom);
}

int main(int argc, char **argv) {
    struct pcv_image image;

	swear_demo();

  /*  if(argc != 3) {
        abort_("Usage: program_name <file_in> <file_out>");
    }

    read_png(argv[1], &image);
    process_image(&image);
    write_png(&image, argv[2]);
    release_image(&image);*/

    return 0;
}
