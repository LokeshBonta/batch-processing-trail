#include<iostream>
#include<stdlib.h>
#include <CL/cl.hpp>
#include"rppdefs.h"

#define C L_USE_DEPRECATED_OPENCL_1_2_APIS

using namespace std;
#define MAX_SIZE 8096

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