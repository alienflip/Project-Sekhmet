// ~~~~ todo
constant int arrayWidth = 16;
__kernel void Add(__global int* A, __global float* averages, __global int* C){
    // kernel indexing
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    const int arrayHeight = arrayWidth / 4;
    
    // boids rules
    float ave_x = averages[0];
    float ave_y = averages[1];
    float ave_vx = averages[2];
    float ave_vy = averages[3];

    // steer based on global boid averages
    int steer[4] = { 0,0,0,0 };
    if (ave_x > 0.5) steer[0] += 1; 
    if (ave_x < -0.5) steer[0] -= 1;
    if (ave_y > 0.5) steer[1] += 1;
    if (ave_y < -0.5) steer[1] -= 1;
    if (ave_vx > 0.5) steer[2] += 1;
    if (ave_vx < -0.5) steer[2] -= 1;
    if (ave_vy > 0.5) steer[3] += 1;
    if (ave_vy < -0.5) steer[3] -= 1;
    
    // calculate next frame
    int idx = x + arrayWidth * y;
    if (idx % 4 == 0) {
        int currIdx, currRow, minRow, maxRow, currCol, minCol, maxCol, ax, ay, avx, avy;
        for (int j = -1; j <= 1; j++) {
            for (int i = -4; i <= 4; i = i + 4) {
                currIdx = idx + i + j * arrayWidth;
                currRow = j + (int) idx / arrayWidth;
                minRow = currRow * arrayWidth;
                maxRow = minRow + arrayWidth;
                currCol = (idx + i) % arrayWidth;
                minCol = currCol;
                maxCol = minCol + arrayHeight * arrayWidth;
                if(currIdx >= minRow && currIdx >= minCol && currIdx < maxRow && currIdx < maxCol){
                    for (int k = 0; k < 4; k++) C[currIdx + k] += A[currIdx + k] + steer[k];
                }
            }
        }
    }
}
