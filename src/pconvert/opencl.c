#include "stdafx.h"

#ifdef PCONVERT_OPENCL

cl_program load_program(cl_context context, char *algorithm, int *error) {
    cl_program program;
    FILE *file;
    char path[1024];
    unsigned char *source_str;
    size_t source_size;

    join_path("src/kernel/", algorithm, path);
    join_path(path, ".cl", path);

    file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to load kernel.\n %s\n", path);
        (*error) = ERROR;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    source_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    source_str = (unsigned char *) malloc(source_size);
    source_size = fread(source_str, 1, source_size, file);
    fclose(file);

    program = clCreateProgramWithSource(
        context,
        1,
        (const char **) &source_str,
        (const size_t *) &source_size,
        error
    );
    return program;
}

ERROR_T blend_images_opencl(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    int y;
    unsigned char *bottom_p;
    unsigned char *top_p;
    unsigned char *bottom_array;
    unsigned char *top_array;

    int row_size = png_get_rowbytes(bottom->png_ptr, bottom->info_ptr);
    int buffer_size = row_size * bottom->height;
    bottom_array = malloc(buffer_size);
    top_array = malloc(buffer_size);

    bottom_p = &(bottom_array[0]);
    top_p = &(top_array[0]);

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
    for (y = 0; y < bottom->height; y++) {
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

    int rem;
    int error;

    error = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if(error != CL_SUCCESS) { RAISE_S("[blend_kernel] Failed to create a device group: %d", error); }

    context = clCreateContext(0, 1, &device_id, NULL, NULL, &error);
    if(!context) { RAISE_S("[blend_kernel] Failed to create a compute context: %d", error); }

    commands = clCreateCommandQueue(context, device_id, 0, &error);
    if(!commands) { RAISE_S("[blend_kernel] Failed to create a command queue: %d", error); }

    program = load_program(context, algorithm, &error);
    if(error != CL_SUCCESS) { RAISE_S("[blend_kernel] Failed to create the program: %d", error); }

    error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(error != CL_SUCCESS) {
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
        RAISE_S("[blend_kernel] Failed to build the program: %d", error);
    }

    kernel = clCreateKernel(program, algorithm, &error);
    if(error != CL_SUCCESS) { RAISE_S("[blend_kernel] Failed to create kernel: %d", error); }

    mem_bottom = clCreateBuffer(context, CL_MEM_READ_WRITE,  size, NULL, NULL);
    mem_top = clCreateBuffer(context, CL_MEM_READ_ONLY,  size, NULL, NULL);

    error = clEnqueueWriteBuffer(commands, mem_bottom, CL_TRUE, 0, size, bottom, 0, NULL, NULL);
    error = clEnqueueWriteBuffer(commands, mem_top, CL_TRUE, 0, size, top, 0, NULL, NULL);
    if(error != CL_SUCCESS){ RAISE_S("[blend_kernel] Failed to write to source array"); }

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_bottom);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_top);
    clSetKernelArg(kernel, 2, sizeof(int), &size);

    error = clGetKernelWorkGroupInfo(
        kernel,
        device_id,
        CL_KERNEL_WORK_GROUP_SIZE,
        sizeof(local),
        &local,
        NULL
    );
    if(error != CL_SUCCESS){ RAISE_S("[blend_kernel] Failed to retrieve work group info: %d", error); }

    rem = size / local;
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

    NORMAL;
}

#else

ERROR_T blend_images_opencl(struct pcv_image *bottom, struct pcv_image *top, char *algorithm) {
    RAISE_S("[blend_kernel] No OpenCL support available");
}

ERROR_T blend_kernel(unsigned char *bottom, unsigned char *top, int size, char *algorithm) {
    RAISE_S("[blend_kernel] No OpenCL support available");
}

#endif
