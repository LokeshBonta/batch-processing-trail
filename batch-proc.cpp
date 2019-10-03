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
        (*xroi_begin)[i] = Rois[i].x;
        (*yroi_begin)[i] = Rois[i].y;
        (*xroi_end)[i] = Rois[i].x + Rois[i].roiWidth - 1;
        (*xroi_end)[i] = Rois[i].x + Rois[i].roiHeight -1;
    }
}

void get_size_params(RppiSize *Sizes, int batch_size, unsigned int *widths, 
                        unsigned int *heights, unsigned int *batch_width){
    int i;
    batch_index[0] = 0;
    for(i =0; i < batch_size -1 ; i++){
       widths[i]  =  Sizes[i].width;
       heights[i] =  Sizes[i].height;
       batch_index[i+1] = batch_index[i+1] + Sizes[i].height * Sizes[i].width * channel;
    }
    height[batchsize -1] = Sizes[batchsize -1].height;
    width[batchsize - 1] = Sizes[batchsize -1].width;  
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

void sizes_fill(RppiSize *Sizes, int batch_size, int channel){
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
    unsigned int *widths, *heights, *batchwidth;
    widths  = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    heights = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    batchwidth = (unsigned int *)malloc(sizeof(unsigned int)* batch_size);
    get_size_params();
    //

    // Getting ROI-Coordinates
    unsigned int *xroi_begin, *xroi_end, *yroi_begin, *yroi_end;
    xroi_begin = (unsigned int *)malloc(sizeof(int) * batchsize);
    xroi_end = (unsigned int *)malloc(sizeof(int) * batchsize);
    yroi_begin = (unsigned int *)malloc(sizeof(int) * batchsize);
    yroi_end = (unsigned int *)malloc(sizeof(int) * batchsize);
    get_roi_dims();
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
    images = (Rpp8u *)malloc(sizeofO(Rpp8u) * bytes);
    image_fill(images, bytes);

    cl_mem d_alpha, d_beta, d_xroi_begin, d_xroi_end,d_yroi_begin, d_yroi_end, d_height, 
            d_width, d_batch_index;
    




    return 0;
}