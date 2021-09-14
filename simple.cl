constant int arrayWidth = 8;
constant int arrayHeight = 4;
__kernel void Add(__global int* A, __global int* B, __global int* C, __global int* averages)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    /*
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
    */

    //printf("A[%d] + B[%d] = %d\n", x + arrayWidth * y, x + arrayWidth * y,  A[x + arrayWidth * y]  +  B[x + arrayWidth * y]);
    C[x + arrayWidth * y] = A[x + arrayWidth * y] + B[x + arrayWidth * y];
}
