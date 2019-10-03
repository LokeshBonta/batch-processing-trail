#include<iostream>
#include<stdlib.h>
#include<stdio.h>
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
            *max_height = Sizes[i].height;
        if(max_width < Sizes[i].width)
            *max_width = Sizes[i].width;
    }
}

void max_roi_size(RppiROI *ROIs, int batch_size, int *max_roi_height, int *max_roi_width)
{
    int i;
    *max_roi_height  = 0;
    *max_roi_width = 0;
    for (i=0; i<batch_size; i++){
        if(max_roi_height < ROIs[i].roiHeight)
            *max_roi_height = ROIs[i].roiHeight;
        if(max_roi_width < ROIs[i].roiWidth)
            *max_roi_width = ROIs[i].roiWidth;
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
    int i;
    for(i =0; i < batch_size; i++){
       x[i] = ROIs[i].x;  
       y[i] = ROIs[i].y ;
       ROIwidths[i]  = ROIs[i].roiHeight;
       ROIheights[i] = ROIs[i].roiWidth;
    }
}

void images_fill(Rpp8u **Images, RppiSize *Sizes, int batch_size, int channel){
    int i;
    Images = (Rpp8u **) malloc(batch_size * sizeof(Rpp8u));
    for(i =0; i < batch_size-1; i++){
       Sizes[i].width = 500; 
       Sizes[i].height = 600;
       Images[i] = (Rpp8u *) malloc (Sizes[i].height * Sizes[i].width * channel);
    }   
    Sizes[batch_size-1].width  = 1000;
    Sizes[batch_size-1].height = 1000;
    Images[batch_size - 1] = (Rpp8u *) malloc (Sizes[batch_size - 1].height * Sizes[batch_size - 1].width * channel);
    int j;
    for(j =0; j < batch_size; j++){
        for(i=0; i < Sizes[j].height * Sizes[i].width *channel; i++){
            Images[j][i] = i %256;
        }
    }
}

void params_fill(float *alpha, float *beta, int batchsize){
    int i;
    alpha = (float *)malloc(sizeof(float) * batchsize);
    beta  = (float *)malloc(sizeof(float) * batchsize);
    for(i=0; i<batchsize; i++){
        alpha[i] = 1.0;
        beta[i] = 20.0;
    }
}

int calculate_bytes(RppiSize *Sizes, int batchsize, int channel){
    int i;
    int bytes = 0;
    for(i =0; i< batchsize; i++){
        bytes += Sizes[i].height * Sizes[i].width * channel;
    }
    return bytes;
}

void get_size_params(RppiSize *Sizes, int batch_size, unsigned int *widths, unsigned int *heights){
    widths  = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    heights = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    int i;
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

void get_dims(RppiSize *Sizes, int batchsize, unsigned int** height, unsigned int** width, unsigned int** batch_index, int channel)
{
    *(height) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(width) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(batch_index) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(*batch_index) = 0;
    int i;
    for (i =0; i< batchsize -1 ; i++){
        (*height)[i]= Sizes[i].height;
        (*width)[i]= Sizes[i].width;
        (*batch_index)[i+1]= (*batch_index)[i+1] + Sizes[i].height * Sizes[i].width * channel;
    }

    (*height)[batchsize -1] = Sizes[batchsize -1].height;
    (*width)[batchsize - 1] = Sizes[batchsize -1].width;  
}

void get_roi_dims(RppiROI *Rois, int batchsize, unsigned int **xroi_begin, unsigned int **xroi_end, 
                        unsigned int **yroi_begin, unsigned int **yroi_end){
    *(xroi_begin) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(xroi_end) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(yroi_begin) = (unsigned int *)malloc(sizeof(int) * batchsize);
    *(yroi_end) = (unsigned int *)malloc(sizeof(int) * batchsize);
    int i;
    for( i =0; i< batchsize; i++){
        (*xroi_begin)[i] = Rois[i].x;
        (*yroi_begin)[i] = Rois[i].y;
        (*xroi_end)[i] = Rois[i].x + Rois[i].roiWidth - 1;
        (*xroi_end)[i] = Rois[i].x + Rois[i].roiHeight -1;
    }
}

int main(int argc, char** argv){
    
    // Host input vectors
    Rpp8u *h_a;
    Rpp8u *h_b;

    //Host input Images
    Rpp8u **input_images;
    RppiSize *Sizes;
    int batchsize = 100;
    int channel = 3;
    //Image filling
    images_fill(input_images, Sizes, batchsize, channel);
    
    //Setting up ROIs
    RppiROI *ROIs;
    roi_fill(ROIs, batchsize);

    //Caclulating max-height and max-width
    int *max_height, *max_width;
    max_size(Sizes, batchsize, max_height, max_width);

    //Parameters Filling
    float *alpha, *beta;
    params_fill(alpha, beta, batchsize);

    //Getting total number of bytes
    int bytes = calculate_bytes(Sizes, batchsize, channel);
    
    //Cl-mem creation for the buffer
    
    unsigned int *height,*width, *batch_index;
    unsigned int *xroi_begin, *xroi_end, *yroi_begin, *yroi_end;

    get_dims(Sizes, batchsize, &height, &width, &batch_index,channel);
    get_roi_dims(ROIs, batchsize, &xroi_begin, &xroi_end, &yroi_begin, &yroi_end);

    // Allocate memory for each vector on host
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

    cl_mem d_alpha, d_beta, d_xroi_begin, d_xroi_end,d_yroi_begin, d_yroi_end, d_height, d_width, d_batch_index;
 
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
     
    ushort pln  = 0 // Packed Versionl
    // Enqueue the kernel execution command
    //Params Setting
    cl_kernel kernel = clCreateKernel(program, "brightness_contrast_ROI", &err);

    // Setting the kernel arguments
    int ctr = 0;
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_a);
    err = clSetKernelArg(kernel, ctr++, sizeof(cl_mem), &d_b);
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

    
    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0,
                                   bytes, h_a, 0, NULL, NULL);

    // Read the results from the device
    clEnqueueReadBuffer(queue, d_b, CL_TRUE, 0,
                                bytes, h_b, 0, NULL, NULL );

    //Thread Spawning <max_width, max_heigt, batch_Size>
    
    cout << h_a[35] << endl;
    //clReleaseMemObject(d_a);
    //clReleaseMemObject(d_b);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    //release host memory
    free(h_a);
    free(h_b);
    //A lot of releasing to be done!!!!
    return 0;
}