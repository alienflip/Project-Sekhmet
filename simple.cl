constant int arrayWidth = 64;
__kernel void Add(__global int* A, __global int* B, __global float* averages, __global int* C){
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    const int arrayHeight = arrayWidth / 4;

    //printf("%f %f %f %f %f \n", averages[0], averages[1], averages[2], averages[3]);

    float ave_x = averages[0];
    float ave_y = averages[1];
    float ave_vx = averages[2];
    float ave_vy = averages[3];

    int steer_x = 0;
    int steer_y = 0;
    
    if (ave_x > 0.5) steer_x += 1; 
    if (ave_y > 0.5) steer_y += 1; 
    if (ave_vx > 0.5) steer_x += 1; 
    if (ave_vy > 0.5) steer_y += 1; 

    if (ave_x < -0.5) steer_x -= 1; 
    if (ave_y < -0.5) steer_y -= 1; 
    if (ave_vx < -0.5) steer_x -= 1; 
    if (ave_vy < -0.5) steer_y -= 1; 
    
    int surrounding[36] = {};

    int counter = 0;
    for (int i = -4; i <= 7; i++) {
        for (int j = -1; j <= 1; j++) {
            surrounding[counter] = ((y + i) - 1) * arrayWidth + (x + j) - 1;
            if (surrounding[counter] < 0 || surrounding[counter] >= arrayWidth * arrayHeight) {
                surrounding[counter] = -1;
            }
            counter++;
        }
    }

    //printf("A[%d] + B[%d] = %d\n", x + arrayWidth * y, x + arrayWidth * y,  A[x + arrayWidth * y]  +  B[x + arrayWidth * y]);
    C[x + arrayWidth * y] = (A[x + arrayWidth * y] + B[x + arrayWidth * y]);
}
