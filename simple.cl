// ~~~~ todo
constant int arrayWidth = 16;
__kernel void Add(__global int* A, __global float* averages, __global int* C){
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    const int arrayHeight = arrayWidth / 4;

    //printf("%f %f %f %f %f \n", averages[0], averages[1], averages[2], averages[3]);

    // boids rules
    float ave_x = averages[0];
    float ave_y = averages[1];
    float ave_vx = averages[2];
    float ave_vy = averages[3];

    int steer_x = 0;
    int steer_y = 0;
    
    if (ave_x > 0.5) steer_x += 1; 
    if (ave_x < -0.5) steer_x -= 1;

    if (ave_y > 0.5) steer_y += 1;
    if (ave_y < -0.5) steer_y -= 1;

    if (ave_vx > 0.5) steer_x += 1;
    if (ave_vx < -0.5) steer_x -= 1;

    if (ave_vy > 0.5) steer_y += 1;
    if (ave_vy < -0.5) steer_y -= 1; 
    
    // boids calculations
    
    /*
    (0  1  2  3)  (4  5  6  7)  (8  9  10 11) (12 13 14 15)
    (16 17 18 19) (20 21 22 23) (24 25 26 27) (28 29 30 31)
    (32 33 34 35) (36 37 38 39) (40 41 42 43) (44 45 46 47)
    (48 49 50 51) (52 53 54 55) (56 57 58 59) (60 61 62 63)
    */

    int idx = x + arrayWidth * y;

    if (idx == 28) {
        for (int j = -1; j <= 1; j++) {
            for (int i = -4; i <= 4; i = i + 4) {
                int currIdx = idx + 4 * j + i + j * arrayWidth;
                int currRow = (int) currIdx / 16;
                if(currIdx >= 0){
                    //printf("(curr index, row, lim: %d %d %d) ", currIdx, currRow, currRow * arrayWidth);
                    printf("(%d) ", currIdx);
                }
            }
            printf("\n");
        }
    }
    
    //prints x indicies % 4 == 0, and y indicies in rows above and below corresponding

    //printf("A[%d] + B[%d] = %d\n", x + arrayWidth * y, x + arrayWidth * y,  A[x + arrayWidth * y]  +  B[x + arrayWidth * y]);

    // boids output calcs
    /*
    for (i = 0; i < arrayHeight * arrayWidth; i++) {
        for (int j = 0; j < 4; j++) if ((i + j) % 4) C[j] = surrounding[j]; // calculate C values based on "surrounding"
    }
    */

    // placeholder
    C[x + arrayWidth * y] = 0;
}
