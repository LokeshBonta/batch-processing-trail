#include<iostream>
#include<stdlib.h>
#include <CL/cl.hpp>
#include"rppdefs.h"

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

using namespace std;
#define MAX_SIZE 8096

void max_size(RppiSize *Sizes, int batch_size, int *max_height, int *max_width)
{
    int i;
    *max_height  = 0;
    *max_width =0;
    for (i=0; i<batch_size; i++){
        if(max_height < Sizes[i].height)
            *max_height = Sizes[i].heigt;
        if(max_width < Sizes[i].width)
            *max_width = Sizes[i].width;
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

void get_roi_params(RppiROI *ROIs, int batch_size, unsigned int *x, unsigned int *y,
                                unsigned int *ROIwidths, unsigned int *ROIheights){
    x = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    y = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    ROIwidths  = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    ROIheights = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    for(i =0; i < batch_size; i++){
       x[i] = ROIs[i].x;  
       y[i] = ROIs[i].y ;
       ROIwidths[i]  = ROIs[i].roiHeight;
       ROIheights[i] = ROIs[i].roiWidth;
    }
}

void size_fill(RppiSize *Sizes, int batch_size){
    int i;
    for(i =0; i < batch_size-1; i++){
       Sizes[i].width = 500; 
       Sizes[i].height = 600;
    }   
    Sizes[batch_size-1].width  = 1000;
    Sizes[batch_size-1].height = 1000;
}



void get_size_params(RppiSize *Sizes, int batch_size, unsigned int *widths, unsigned int *heights){
    widths  = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    heights = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    for(i =0; i < batch_size; i++){
       widths[i]  =  Sizes[i].width ;
       heights[i] =  Sizes[i].height;
    }
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

int main(int argc, char** argv){
    
    // Host input vectors
    Rpp8u *h_a;
    Rpp8u *h_b;

    // Allocate memory for each vector on host
    int height, width, channel, batchsize;
    height = 1000;
    width  = 1000;
    batchsize = 100;
    channel = 3;

    int bytes = height * width * channel * batchsize;
    h_a = (Rpp8u *)malloc(bytes);
    //h_b = (doauble*)malloc(bytes);
    h_b = (Rpp8u *)malloc(bytes);
    
    cl_platform_id cpPlatform;        // OpenCL platform
    cl_device_id device_id;           // device ID
    cl_context context;               // context
    cl_command_queue queue;           // command queue
    // Device input bufers
    cl_mem d_a;
    // Device output buffer
    cl_mem d_b;

    cl_int err;
 
    
    // Create program
    unsigned char* program_file = NULL;
    size_t program_size = 0;
    read_file(&program_file, &program_size, "brightness.cl");


    // Bind to platform
    err = clGetPlatformIDs(1, &cpPlatform, NULL);
 
    // Get ID for the device
    err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
 
    // Create a context 
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
 
    // Create a command queue
    queue = clCreateCommandQueue(context, device_id, 0, &err);

    // Create the input and output arrays in device memory for our calculation
    d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    //d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    d_b = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);

    cl_program program =
      clCreateProgramWithSource(ctx, 1, (const char **)&program_file,
                                &program_size, &err);

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    free(program_file);
     
    ushort pln  = 0 // Packed Version
    float alpha = 1;
    float beta = 0;
    // Enqueue the kernel execution command
    cl_kernel kernel = clCreateKernel(program, "brighness_conntrast_ROI", &err);
    int ctr = 0;
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_a);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_b);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &b);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &height);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &width);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &channel);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &pln);

    
    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0,
                                   bytes, h_a, 0, NULL, NULL);

    // Read the results from the device
    clEnqueueReadBuffer(queue, d_b, CL_TRUE, 0,
                                bytes, h_b, 0, NULL, NULL );
    
    cout << h_a[35] << endl;
    //clReleaseMemObject(d_a);
    //clReleaseMemObject(d_b);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    //release host memory
    free(h_a);
    free(h_b);
    return 0;
}