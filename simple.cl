constant int arrayWidth = 16;
constant int arrayHeight = 16;
__kernel void Add(__global int* A, __global int* B, __global int* C)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int surrounding[9] = {};

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

    //printf("(Ax: %d, Ay: %d) (Bx: %d, By: %d)\n",A[x], A[y], B[x], B[y]);
    //printf("\n");
    printf("A[%d] %d B[%d] %d\n", A[x] + arrayWidth * A[y], A[x] + arrayWidth * A[y], B[x] + arrayWidth * B[y], B[x] + arrayWidth * B[y]);

    C[A[x] + arrayWidth * A[y]] = A[x] + arrayWidth * A[y];
}
