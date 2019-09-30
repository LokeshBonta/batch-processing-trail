#define saturate_8u(value) ( (value) > 255 ? 255 : ((value) < 0 ? 0 : (value) ))

unsigned int get_pln_index(unsigned int id_x, unsigned int id_y, unsigned int id_z, unsigned int width, 
                        unsigned int height, unsigned channel){
 retrun ( id_x + id_y * width + id_z * width * height);
}

/*struct pixel{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

pixel load_pix(unsigned char *image, int i, int j, int pln){
    pixel pix;
    unsigned int index = ;
    //depends on pln and pkd
    pix.r =;
    pix.g =;
    pix.b =;
    return pix;
}

void store_pix(unsigned char * image, int i, int j, int pln, pixel pix){
    usnigned int index =;
    //Depends on pln and pkd

}
unsigned int get_pkd_index(unsigned int id_x, unsigned int id_y, unsigned int id_z, unsigned int width, 
                        unsigned int height, unsigned channel){
 retrun (id_z + id_x * channel + id_y * width * channel);
}*/

__kernel void brightness_contrast(  __global unsigned char* input,
                                    __global unsigned char* output,
                                    const float alpha,
                                    const int beta,
                                    const unsigned int height,
                                    const unsigned int width,
                                    const unsigned int channel
)
{
    int id_x = get_global_id(0);
    int id_y = get_global_id(1);
    int id_z = get_global_id(2);
    if (id_x >= width || id_y >= height || id_z >= channel) return;

    int pixIdx = get_pln_index(id_x, id_y, id_z, width, height, channel);

    int res = input[pixIdx] * alpha + beta;
    output[pixIdx] = saturate_8u(res);
}


__kernel void brightness_contrast_ROI(  __global unsigned char* input,
                                    __global unsigned char* output,
                                    __global float *alpha,
                                    __global float *beta,
                                    __global int *xroi_begin,
                                    __global int *yroi_begin,
                                    __global int *roi_height,
                                    __global int *roi_width,
                                    __global unsigned int *height,
                                    __global unsigned int *width,
                                    unsigned int batchsize,
                                    unsigned int channel,
                                    const ushort pln
)
{
    int id_x = get_global_id(0);
    int id_y = get_global_id(1);
    int id_z = get_global_id(2);

    int xroi_end, yroi_end;
    xroi_end[id_z] = xroi_begin[id_z] + xroi_width[id_z] - 1;
    yroi_end[id_z] = yroi_begin[id_z] + yroi_height[id_z] - 1;

    int batch_index = 0;
    int i;
    for(i =0; i < id_z; i++)
        batch_index += height[i] * width[i] * channel; 
   
   // int pixIdx = batch_index+ id_x + id_y * width + id_z * width * height;
    if(pln)
        int pixIdx = batch_index + get_pln_index(id_x, id_y, id_z, width, height, channel);
    else
        int pixIdx = batch_index + get_pkd_index(id_x, id_y, id_z, width, height, channel);

    int res;
    //Index are in bounds in for batching
    if ((id_y >=y1) && (id_y <=y2) && (id_x >= x1) && (id_x <= x2 ))
    {
        res = input[pixIdx] * alpha + beta;
       
        output[pixIdx] = saturate_8u(res);
    }
    else
        output[pixIdx] = 0;
}