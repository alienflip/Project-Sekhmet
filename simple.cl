constant uint arrayWidth = 16;
constant uint arrayHeight = 4;

constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

__kernel void Add(__global int* A, __global int* B, __global int* C)
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
                if (surrounding[counter] < 0 || surrounding[counter] >= arrayWidth * arrayHeight) {
                    surrounding[counter] = -1;
                }
                counter++;
            }
        }
    }

    // layer 2
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

    uint M_A = A[x];
    uint M_B = B[x];

    C[x * 1024 + y] = M_A + M_B;
}

