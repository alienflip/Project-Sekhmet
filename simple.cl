constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
constant uint arrayWidth = 16;

__kernel void Add(__global int *M_A, __global int *M_B, __global int *M_C)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // input matrixes
    int surrounding[24] = {};

    int counter = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 1 || j == 1 || i == -1 || j == -1) {
                surrounding[counter] = ((y + i) - 1) * arrayWidth + (x + j) - 1;
                counter++;
            }
        }
    }

    // layer 2
    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if (i == 2 || j == 2 || i == -2 || j == -2) {
                surrounding[counter] = ((y + i) - 1) * arrayWidth + (x + j) - 1;
                counter++;
            }
        }
    }

    /*
    for (int i = 0; i < 24; i++) {
        printf("%d ", surrounding[i]);
    }printf("\n");
    */

    // output matrix
    M_C[x * arrayWidth + y] = M_A[x] + M_B[y];
    printf("%d\n", M_C[x * arrayWidth + y]);
}
