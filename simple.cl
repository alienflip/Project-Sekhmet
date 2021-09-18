// ~~~~ todo
constant int arrayWidth = 16;
__kernel void Add(__global int* A, __global float* averages, __global int* C) {
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

    /*
    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
    48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
    */

    // steer based on local velocities
    int idx = x + arrayWidth * y;
    int currIdx, currRow, minRow, maxRow, currCol, minCol, maxCol;
    for (int j = -1; j <= 1; j++) {
        for (int i = -4; i <= 4; i = i + 4) {
            currIdx = idx + i + j * arrayWidth;
            currRow = j + (int)idx / arrayWidth;
            minRow = currRow * arrayWidth;
            maxRow = minRow + arrayWidth;
            currCol = (idx + i) % arrayWidth;
            minCol = currCol;
            maxCol = minCol + arrayHeight * arrayWidth;
            if (currIdx >= minRow && currIdx >= minCol && currIdx < maxRow && currIdx < maxCol) {
                for (int k = 2; k < 4; k++) steer[k] -= (float)A[currIdx + k];
            }
        }
    }

    // calculate next frame
    switch (idx % 4) {
    case 0: // switch alive cell to dead cell
        if (steer[1] > 0 || steer[2] > 0) {
            switch (A[idx]) {
            case 0:
                C[idx] = 1;
                break;
            case 1:
                C[idx] = 0;
                break;
            }
        }
        break;
    case 1: // handle rebound velocity switch
        C[idx] = A[idx];
        break;
    case 2: // adjust next velocity
        C[idx] = steer[2];
        break;
    case 3: // adjust next velocity
        C[idx] = steer[3];
        break;
    }
}
