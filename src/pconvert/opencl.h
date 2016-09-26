#pragma once

#ifdef __APPLE__
    #include "OpenCL/opencl.h"
#else
    #include "CL/cl.h"
#endif

cl_program loadProgram(cl_context context, char *algorithm, int *err);

ERROR_T compose_images_opencl(
    char *base_path,
    char *algorithm,
    char *background,
    int compression,
    int filter
);

ERROR_T blend_images_opencl(struct pcv_image *bottom, struct pcv_image *top, char *algorithm);
ERROR_T blend_kernel(unsigned char *bottom, unsigned char *top, int size, char *algorithm);
