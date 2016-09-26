#include "stdafx.h"

cl_program loadProgram(cl_context context, char *algorithm, int *err) {
    cl_program program;
    char path[1024];
    FILE *fp;
	join_path("src/kernel/", algorithm, path);
    join_path(path, ".cl", path);
	char *source_str;
	size_t source_size;

	fp = fopen(path, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n %s\n", path);
        (*err) = ERROR;
        return NULL;
	}
	source_str = (char*) malloc(2048);
	source_size = fread(source_str, 1, 2048, fp);
	fclose(fp);

    program = clCreateProgramWithSource(
        context,
        1,
        (const char **) &source_str,
        (const size_t *) &source_size,
        err
    );
    return program;
}

ERROR_T compose_images_opencl(
    char *base_path,
    char *algorithm,
    char *background,
    int compression,
    int filter
) {
    char path[1024];
    char name[1024];
    struct pcv_image bottom, top, final;
    char demultiply = is_multiplied(algorithm);

    read_png(join_path(base_path, "sole.png", path), demultiply, &bottom);
    read_png(join_path(base_path, "back.png", path), demultiply, &top);
    blend_images_opencl(&bottom, &top, algorithm); release_image(&top);
    read_png(join_path(base_path, "front.png", path), demultiply, &top);
    blend_images_opencl(&bottom, &top, algorithm); release_image(&top);
    read_png(join_path(base_path, "shoelace.png", path), demultiply, &top);
    blend_images_opencl(&bottom, &top, algorithm); release_image(&top);
    if(demultiply) { multiply_image(&bottom); }
    sprintf(name, "background_%s.png", background);
    read_png(join_path(base_path, name, path), FALSE, &final);
    blend_images_opencl(&final, &bottom, "alpha"); release_image(&bottom);
    sprintf(name, "result_%s_%s.png", algorithm, background);
    write_png_extra(&final, FALSE, join_path(base_path, name, path), compression, filter);
    release_image(&final);

    NORMAL;
}

ERROR_T blend_images_opencl(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    int y;

    unsigned char *bottom_array;
    unsigned char *top_array;

    int row_size = png_get_rowbytes(bottom->png_ptr, bottom->info_ptr);
    int buffer_size = row_size * bottom->height;
    bottom_array = malloc(buffer_size);
    top_array = malloc(buffer_size);

    unsigned char *bottom_p = &(bottom_array[0]);
    unsigned char *top_p = &(top_array[0]);

    for(y = 0; y < bottom->height; y++) {
        unsigned char *row_bottom = bottom->rows[y];
        unsigned char *row_top = top->rows[y];

        memcpy(bottom_p, row_bottom, row_size);
        memcpy(top_p, row_top, row_size);

        bottom_p += row_size;
        top_p += row_size;
    }

    blend_kernel(bottom_array, top_array, buffer_size, algorithm);

    bottom_p = &(bottom_array[0]);
    for (int y = 0; y < bottom->height; y++) {
        bottom->rows[y] = bottom_p;
		bottom_p += row_size;
	}

    NORMAL;
}

ERROR_T blend_kernel(unsigned char *bottom, unsigned char *top, int size, char *algorithm) {
    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
    cl_mem mem_bottom, mem_top;

    size_t global;
    size_t local; 

    int err;

    err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(err != CL_SUCCESS) { RAISE_S("Failed to create a device group: %d", err); }

    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if(!context) { RAISE_S("Failed to create a compute context: %d", err); }

    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if(!commands) { RAISE_S("Failed to create a command queue: %d", err); }

    program = loadProgram(context, algorithm, &err);
    if(err != CL_SUCCESS) { RAISE_S("Failed to create the program: %d", err); }

    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err != CL_SUCCESS) { 
        size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(
            program,
            device_id, 
            CL_PROGRAM_BUILD_LOG, 
            sizeof(buffer), 
            buffer, 
            &len
        );
        printf("%s\n", buffer);
        RAISE_S("Failed to build the program: %d", err); 
    }

    kernel = clCreateKernel(program, algorithm, &err);
    if(err != CL_SUCCESS) { RAISE_S("Failed to create kernel: %d", err); }

    mem_bottom = clCreateBuffer(context, CL_MEM_READ_WRITE,  size, NULL, NULL);
    mem_top = clCreateBuffer(context, CL_MEM_READ_ONLY,  size, NULL, NULL);

    err = clEnqueueWriteBuffer(commands, mem_bottom, CL_TRUE, 0, size, bottom, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(commands, mem_top, CL_TRUE, 0, size, top, 0, NULL, NULL);
    if(err != CL_SUCCESS){ RAISE_S("Error: Failed to write to source array"); }  

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_bottom);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_top);
    clSetKernelArg(kernel, 2, sizeof(int), &size);

    err = clGetKernelWorkGroupInfo(
        kernel,
        device_id, 
        CL_KERNEL_WORK_GROUP_SIZE, 
        sizeof(local), 
        &local, 
        NULL
    );
    
    int rem = size / local;
    if(local % rem != 0) { rem++; }
    global = local * rem;

    clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    clEnqueueReadBuffer(commands, mem_bottom, CL_TRUE, 0, size, bottom, 0, NULL, NULL ); 

    clFinish(commands);

    clReleaseMemObject(mem_bottom);
    clReleaseMemObject(mem_top);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}
