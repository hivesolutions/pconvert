#include "stdafx.h"

int x;
int y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep *row_pointers;

void abort_(const char *s, ...) {
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void read_png_file(char* file_name) {
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
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr) {
        abort_("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) {
        abort_("[read_png_file] png_create_info_struct failed");
    }

    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[read_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    /* read file */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[read_png_file] Error during read_image");
    }

    row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
    for(y = 0; y < height; y++) {
        row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(png_ptr,info_ptr));
    }

    png_read_image(png_ptr, row_pointers);
    fclose(fp);
}

void write_png_file(char* file_name) {
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
        width,
        height,
        bit_depth,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);

    /* write bytes */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png_file] Error during writing bytes");
    }

    png_write_image(png_ptr, row_pointers);

    /* end write */
    if(setjmp(png_jmpbuf(png_ptr))) {
        abort_("[write_png_file] Error during end of write");
    }

    png_write_end(png_ptr, NULL);

    /* cleanup heap allocation, avoids memory leaks */
    for(y = 0; y < height; y++) {
        free(row_pointers[y]);
    }

    free(row_pointers);
    fclose(fp);
}

void process_file(void) {
    if(png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB) {
        abort_(
            "[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
            "(lacks the alpha channel)"
        );
    }

    if(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA) {
        abort_(
            "[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
            PNG_COLOR_TYPE_RGBA,
            png_get_color_type(png_ptr, info_ptr)
        );
    }

    for(y = 0; y < height; y++) {
        png_byte *row = row_pointers[y];
        for(x = 0; x < width; x++) {
            png_byte *ptr = &(row[x * 4]);
			int is_valid = x % 100 == 0;
            is_valid && printf(
                "Pixel at position [ %d - %d ] has RGBA values: (%d,%d,%d,%d)\n",
                x,
                y,
                ptr[0],
                ptr[1],
                ptr[2],
                ptr[3]
            );

            /* sets red value to 0 and green value to the blue one,
            this will create a special kind of effect */
            ptr[0] = 0;
            ptr[1] = ptr[2];
        }
    }
}

int main(int argc, char **argv) {
    if(argc != 3) {
        abort_("Usage: program_name <file_in> <file_out>");
    }

    read_png_file(argv[1]);
    process_file();
    write_png_file(argv[2]);

    return 0;
}