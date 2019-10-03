#include<stdlib.h>
#include<stdio.h>
#include <CL/cl.hpp>
#include"rppdefs.h"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

using namespace std;

void max_size(RppiSize *Sizes, int batch_size, int *max_height, int *max_width)
{
    int i;
    *max_height  = 0;
    *max_width =0;
    for (i=0; i<batch_size; i++){
        if(*max_height < Sizes[i].height)
            *max_height = Sizes[i].height;
        if(*max_width < Sizes[i].width)
            *max_width = Sizes[i].width;
    }
}

void max_roi_size(RppiROI *ROIs, int batch_size, int *max_roi_height, int *max_roi_width)
{
    int i;
    *max_roi_height  = 0;
    *max_roi_width = 0;
    for (i=0; i<batch_size; i++){
        if(*max_roi_height < ROIs[i].roiHeight)
            *max_roi_height = ROIs[i].roiHeight;
        if(*max_roi_width < ROIs[i].roiWidth)
            *max_roi_width = ROIs[i].roiWidth;
    }
}

void get_roi_dims(RppiROI *Rois, int batchsize, unsigned int *xroi_begin, unsigned int *xroi_end, 
                        unsigned int *yroi_begin, unsigned int *yroi_end){
    
    int i;
    for( i =0; i< batchsize; i++){
        xroi_begin[i] = Rois[i].x;
        yroi_begin[i] = Rois[i].y;
        xroi_end[i] = Rois[i].x + Rois[i].roiWidth - 1;
        xroi_end[i] = Rois[i].x + Rois[i].roiHeight -1;
    }
}

void get_size_params(RppiSize *Sizes, int batch_size, unsigned int *widths, 
                        unsigned int *heights, unsigned int *batch_index, int channel){
    int i;
    batch_index[0] = 0;
    for(i =0; i < batch_size -1 ; i++){
       widths[i]  =  Sizes[i].width;
       heights[i] =  Sizes[i].height;
       batch_index[i+1] = batch_index[i+1] + Sizes[i].height * Sizes[i].width * channel;
    }
    heights[batch_size -1] = Sizes[batch_size -1].height;
    widths[batch_size - 1] = Sizes[batch_size -1].width;  
}

int calculate_bytes(RppiSize *Sizes, int batchsize, int channel){
    int i;
    int bytes = 0;
    for(i =0; i< batchsize; i++){
        bytes += Sizes[i].height * Sizes[i].width * channel;
    }
    return bytes;
}

int read_file(unsigned char **output, size_t *size, const char *name) {
  FILE* fp = fopen(name, "rb");
  if (!fp) {
    return -1;
  }

  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(*size);
  if (!*output) {
    fclose(fp);
    return -1;
  }

  fread(*output, *size, 1, fp);
  fclose(fp);
  return 0;
}

void params_fill(float *alpha, float *beta, int batchsize){
    int i;
    for(i=0; i<batchsize; i++){
        alpha[i] = 1.0;
        beta[i] = 20.0;
    }
}

void roi_fill(RppiROI *ROIs, int batch_size){
    int i;
    for(i =0; i < batch_size; i++){
       ROIs[i].x = i; 
       ROIs[i].y = i;
       ROIs[i].roiHeight = 100;
       ROIs[i].roiWidth  = 100;
    }
}

void sizes_fill(RppiSize *Sizes, int batch_size){
    int i;
    for(i =0; i < batch_size-1; i++){
       Sizes[i].width = 500; 
       Sizes[i].height = 600;
    }   
    Sizes[batch_size-1].width  = 1000;
    Sizes[batch_size-1].height = 1000;
}

void image_fill(Rpp8u *images, int bytes){
    int i;
    for (i=0; i<bytes; i++)
        images[i] = i % 256;
}

int main(int argc, char** argv){
    int batchsize = 100;
    int channel   =   3;

    RppiSize *Sizes;
    Sizes = (RppiSize *)(batchsize * sizeof(RppiSize));
    sizes_fill(Sizes, batchsize);

    RppiROI *ROIs;
    ROIs = (RppiROI *)(batchsize * sizeof(RppiROI));
    roi_fill(ROIs, batchsize);

    //Getting Size arrays
    unsigned int *widths, *heights, *batch_index;
    widths  = (unsigned int *)malloc(sizeof(unsigned int)* batchsize);
    heights = (unsigned int *)malloc(sizeof(unsigned int)* batchsize);
    batch_index = (unsigned int *)malloc(sizeof(unsigned int)* batchsize);
    get_size_params(Sizes,batchsize,widths,heights,batch_index,channel);
    //

    // Getting ROI-Coordinates
    unsigned int *xroi_begin, *xroi_end, *yroi_begin, *yroi_end;
    xroi_begin = (unsigned int *)malloc(sizeof(int) * batchsize);
    xroi_end = (unsigned int *)malloc(sizeof(int) * batchsize);
    yroi_begin = (unsigned int *)malloc(sizeof(int) * batchsize);
    yroi_end = (unsigned int *)malloc(sizeof(int) * batchsize);
    get_roi_dims(ROIs, batchsize, xroi_begin, xroi_end, yroi_begin, yroi_end);
    //

    // Filling the params
    float *alpha, *beta;
    alpha = (float *)malloc(sizeof(float) * batchsize);
    beta  = (float *)malloc(sizeof(float) * batchsize);
    params_fill(alpha, beta, batchsize);
    //
    Rpp8u *images;
    int bytes;
    bytes = calculate_bytes(Sizes, batchsize, channel);
    images = (Rpp8u *)malloc(sizeof(Rpp8u) * bytes);
    image_fill(images, bytes);

    cl_mem d_input, d_output;
    cl_mem d_alpha, d_beta, d_xroi_begin, d_xroi_end,d_yroi_begin, d_yroi_end, d_height, 
            d_width, d_batch_index;
    cl_int err;

      // Allocate memory buffers (on the device)
    

    unsigned short pln = 0;
    /* Buffers to be enqueued as cl-mems */

    // OpenCL Program
    

    // Query platforms and devices
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    const cl_context_properties prop[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0
    };

    // Create context
    cl_context ctx = clCreateContext(prop, 1, &device, NULL, NULL, &err);

    // Create program
    unsigned char* program_file = NULL;
    size_t program_size = 0;
    read_file(&program_file, &program_size, "brightness.cl");

    cl_program program =
        clCreateProgramWithSource(ctx, 1, (const char **)&program_file,
                                    &program_size, &err);

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    free(program_file);
    int buffer_size = batchsize * sizeof(unsigned int);

    d_input  = clCreateBuffer(ctx, CL_MEM_READ_ONLY, bytes, NULL, &err);
    d_output = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bytes, NULL, &err);
    d_alpha = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_beta  = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_xroi_begin = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_xroi_end = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_yroi_begin = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_yroi_end = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_width = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_height = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);
    d_batch_index = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buffer_size, NULL, &err);

    // Create command queue
    cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, NULL);

    err = clEnqueueWriteBuffer(queue, d_input, CL_FALSE, 0, bytes, images, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_alpha, CL_FALSE, 0, buffer_size, alpha, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_beta, CL_FALSE, 0, buffer_size, beta, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_height, CL_FALSE, 0, buffer_size, heights, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_width, CL_FALSE, 0, buffer_size, widths, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_batch_index, CL_FALSE, 0, buffer_size, batch_index , 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_xroi_begin, CL_FALSE, 0, buffer_size, xroi_begin, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_xroi_end, CL_FALSE, 0, buffer_size, xroi_end, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_yroi_begin, CL_FALSE, 0, buffer_size, yroi_begin, 0,
                                NULL, NULL);
    err = clEnqueueWriteBuffer(queue, d_yroi_end, CL_FALSE, 0, buffer_size, yroi_end, 0,
                                NULL, NULL);

    // Enqueue the kernel execution command
    cl_kernel kernel = clCreateKernel(program, "brightness_contrast_ROI", &err);

    // Arguments Setting//
    int ctr = 0;
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_input);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_output);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_alpha);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_beta);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_xroi_begin);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_xroi_end);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_yroi_begin);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_yroi_end);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_height);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_width);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_batch_index);
    err = clSetKernelArg(kernel, ctr++, sizeof(unsigned int), &channel);
    err = clSetKernelArg(kernel, ctr++, sizeof(ushort), &pln);
    //
    


    return 0;
}