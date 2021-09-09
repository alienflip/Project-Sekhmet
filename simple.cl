constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void Add(__global int* A, __global int* B, __global int* C)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int surrounding[24] = {};

    int counter = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 1 || j == 1 || i == -1 || j == -1) {
                surrounding[counter] = ((y + i) - 1) * arrayWidth + (x + j) - 1;
                if (surrounding[counter] < 0 || surrounding[counter] >= arrayWidth * arrayHeight) {
                    surrounding[counter] = -1;
                }
                counter++;
            }
        }
    }

    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            if (i == 2 || j == 2 || i == -2 || j == -2) {
                surrounding[counter] = ((y + i) - 1) * arrayWidth + (x + j) - 1;
                if (surrounding[counter] < 0 || surrounding[counter] >= arrayWidth * arrayHeight) {
                    surrounding[counter] = -1;
                }
                counter++;
            }
        }
    }

    uint M_A = A[x * 1024 + y];
    uint M_B = B[x + y * 1024];

    C[x * 1024 + y] = M_A + M_B;
}

