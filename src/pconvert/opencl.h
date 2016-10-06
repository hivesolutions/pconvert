#pragma once

#ifdef PCONVERT_OPENCL

#ifdef __APPLE__
    #include "OpenCL/opencl.h"
#else
    #include "CL/cl.h"
#endif

cl_program load_program(cl_context context, char *algorithm, int *err);

#endif

ERROR_T blend_images_opencl(struct pcv_image *bottom, struct pcv_image *top, char *algorithm);
ERROR_T blend_kernel(unsigned char *bottom, unsigned char *top, int size, char *algorithm);
