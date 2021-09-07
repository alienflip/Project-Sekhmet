constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void Add(__global int *M_A, __global int *M_B, __global int *M_C)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // input matrixes


    printf("%d %d\n", x, 4 * y);

    // output matrix

}
