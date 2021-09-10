constant int arrayWidth = 2;
constant int arrayHeight = 2;
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

    C[x * arrayWidth + y] = A[x * arrayWidth + y] + B[y * arrayWidth + x];

    printf("x: %d Ax: %d Bx: %d\n", x,A[x], B[x]);
    printf("y: %d Ay: %d By: %d\n", y, A[y], B[y]);
    printf("%d Ax:%d By:%d\n", x * arrayWidth + y, A[x], B[y]);
    printf("%d Ay:%d Bx:%d\n", x * arrayWidth + y, A[y], B[x]);
}

