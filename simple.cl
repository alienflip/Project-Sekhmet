constant int arrayWidth = 16;
__kernel void Add(__global int* A, __global int* B, __global int* averages, __global int* C)
{
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    int arrayHeight = arrayWidth / 4;

    //printf("%d %d %d %d %d \n", averages[0], averages[1], averages[2], averages[3]);
    int ave_x = averages[0];
    int ave_y = averages[1];
    int ave_vx = averages[2];
    int ave_vy = averages[3];

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

    C[x + arrayWidth * y] = (A[x + arrayWidth * y] + B[x + arrayWidth * y]) % 2;
}
