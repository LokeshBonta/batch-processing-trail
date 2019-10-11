#define saturate_8u(value) ( (value) > 255 ? 255 : ((value) < 0 ? 0 : (value) ))

unsigned int get_pln_index(unsigned int id_x, unsigned int id_y, unsigned int width){
 return (id_x + id_y * width);
}

unsigned int get_pkd_index(unsigned int id_x, unsigned int id_y, unsigned int width,  unsigned channel){
 return (id_x * channel + id_y * width * channel);
}

__kernel void min ( __global const unsigned char* input,
                    __global unsigned char *partial_min,
                    __global unsigned char *partial_min_location,
                    __global int *xroi_begin,
                    __global int *yroi_begin,
                    __global int *xroi_end,
                    __global int *yroi_end,
                    __global unsigned int *height,
                    __global unsigned int *width,
                    __global unsigned int *batch_index,
                    __global unsigned int batch_size,
                        unsigned int channel,
                        const ushort pln,
                        const ushort roi)
{
    uint local_id = get_local_id(0);
    uint group_size = get_local_size(0);
    local unsigned char localMins[256];
    local unsigned char localMinsLocation[256];

    /* Batch Index Calculation comes here */


    localMins[local_id] = input[get_global_id(0)];
    localMinsLocation[local_id] = get_global_id(0);

    for (uint stride = group_size/2; stride>0; stride /=2)
    {
        if (local_id < stride)
        {
            barrier(CLK_LOCAL_MEM_FENCE);
            if(localMins[local_id] > localMins[local_id + stride])
            {
                localMins[local_id] = localMins[local_id + stride];
                localMinsLocation[local_id] = get_global_id(0) + stride;
            }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (local_id == 0)
    {
        partial_min[get_group_id(0)] = localMins[0];
        partial_min_location[get_group_id(0)] = localMinsLocation[0];
    }
}