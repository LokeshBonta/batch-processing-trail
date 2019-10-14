#define saturate_8u(value) ( (value) > 255 ? 255 : ((value) < 0 ? 0 : (value) ))

unsigned int get_pln_index(unsigned int id_x, unsigned int id_y, unsigned int width){
 return (id_x + id_y * width);
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
 return (id_x * channel + id_y * width * channel);
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
                                    
                                    __global int *xroi_begin,
                                    __global int *yroi_begin,
                                    __global int *xroi_end,
                                    __global int *yroi_end,
                                    __global unsigned int *height,
                                    __global unsigned int *width,
                                    __global unsigned int *batch_index,
                                    unsigned int channel,
                                    const ushort pkd,
                                    const ushort roi
                                    )
{
    int id_x = get_global_id(0);
    int id_y = get_global_id(1);
    int id_z = get_global_id(2);
    unsigned char r, g, b;
    int pixIdx, inc;
    int condition = 0;

    if (pkd){
        if(roi){
            pixIdx = xroi_begin[id_z]*channel + yroi_begin[id_z] * width[id_z] * channel;
        }
        pixIdx += batch_index[id_z] + get_pkd_index(id_x, id_y, width[id_z], channel);
        inc    = 1;
    }
    else{
        if(roi){
            pixIdx = yroi_begin[id_z]*width[id_z] +  xroi_begin[id_z];
        }
        pixIdx += batch_index[id_z] + get_pln_index(id_x, id_y, width[id_z]);
        inc    = height[id_z]*width[id_z];
    }

    if(roi)
        condition =  (id_y >= 0) && (id_y <= (yroi_end[id_z] - yroi_end[id_z])) && (id_x >= 0) && (id_x <= (xroi_end[id_z] - xroi_begin[id_z])); 
    else
        condition =  (id_y >= yroi_begin[id_z]) && (id_y <= yroi_end[id_z]) && (id_x >= xroi_begin[id_z]) && (id_x <= xroi_end[id_z]);

    if(condition)
    {   
        int hfFiltSz = filterSize/2;
        if ( (id_x < hfFiltSz) || (id_y < hfFiltSz) ||
                (id_y >= (height-hfFiltSz)) || (id_x >= (width-hfFiltSz)) )
        {
            output[pixIdx] = input[pixIdx];
            return;
        }

        float sum = 0.0;
        for (int ri = (-1 * hfFiltSz) , rf = 0;
                (ri <= hfFiltSz) && (rf < filterSize);
                    ri++, rf++)
        {
            for (int ci = (-1 * hfFiltSz) , cf = 0;
                    (ci <= hfFiltSz) && (cf < filterSize);
                        ci++, cf++)
            {
                const int idxF = rf + cf * filterSize ;
                const int idxI = pixIdx + ri + ci * width;
                sum += filter[idxF]*input[idxI];
            }
        }
        int res = (int)sum;
        output[pixIdx] = saturate_8u(res);
    }
    else if(id_y < height[id_z] && id_x < width[id_z])
    {
        output[pixIdx]         = 0;
        if(channel == 3)
        {
            output[pixIdx + inc]   = 0;
            output[pixIdx + 2*inc] = 0;
            //printf("%d", id_x);
        }
       
    }
}

