#define saturate_8u(value) ( (value) > 255 ? 255 : ((value) < 0 ? 0 : (value) ))

unsigned int get_pln_index(unsigned int id_x, unsigned int id_y, unsigned int width){
 retrun (id_x + id_y * width);
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

}*/
unsigned int get_pkd_index(unsigned int id_x, unsigned int id_y, unsigned int width,  unsigned channel){
 retrun (id_x * channel + id_y * width * channel);
}

/*__kernel void brightness_contrast(  __global unsigned char* input,
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
}*/


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
   
    unsigned char r, g, b;
    int res1, res2, res3;
    int pixIdx, inc;

    if (pln){
         pixIdx = batch_index + get_pln_index(id_x, id_y, width[id_z]);
         inc    = height[id_z]*width[id_z];
    }
    else{
         pixIdx = batch_index + get_pln_index(id_x, id_y, width[id_z]);
         inc    = 1;
    }
   
        
    if ((id_y >= yroi_begin[id_z]) && (id_y <= yroi_end[id_z]) && (id_x >= xroi_begin[id_z]) && (id_x <= xroi_end[id_z]))
    {   
        if(channel == 3)
        {
            r = input[pixIdx];
            g = input[pixIdx + inc];
            b = input[pixIdx + 2 * inc];

            res1 = r * alpha + beta;
            res2 = g * alpha + beta;
            res3 = b * alpha + beta;

            output[pixIdx] = saturate_8u(res1);
            output[pixIdx + inc] = saturate_8u(res2);
            output[pixIdx + 2*inc] = saturate_8u(res3);
        }
        else if (channel == 1){
            r = input[pixIdx];
            res1 = r * alpha + beta;
            output[pixIdx] = saturate_8u(res1);
        }
    }

    else
    {
        if(channel == 3)
        {
            output[pixIdx]         = 0;
            output[pixIdx + inc]   = 0;
            output[pixIdx + 2*inc] = 0;
        }
        else if(channel == 1)
            output[pixIdx] = 0;
    }
}