/* array structure
0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 */
constant int arrayWidth = 16;
__kernel void Add(__global int* A, __global float* averages, __global int* C) {

    // kernel indexing
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    // array index from kernel index
    int idx = x + arrayWidth * y;
    if (idx % 4 != 0) return;

    // array height
    const int arrayHeight = arrayWidth / 4;

    // boids rules
    float ave_vx = averages[2];
    float ave_vy = averages[3];

    // steer based on global boid averages
    int steer_global[2] = { 0, 0, 0, 0 };
    if (ave_vx > 0.5) steer_global[2] = 1;
    else if (ave_vx < -0.5) steer_global[2] = -1;
    if (ave_vy > 0.5) steer_global[3] = 1;
    else if (ave_vy < -0.5) steer_global[3] = -1;

    // steer based on local velocities
    int steer_local[4] = { 0, 0, 0, 0 };
    int currIdx, currRow, minRow, maxRow, currCol, minCol, maxCol;
    int aliveCell = A[idx];
    int reboundCell = 0;
    int adjVelX, adjVelY, diffX, diffY, check;
    // ~~~ todo
    for (int j = -1; j <= 1; j++) {
        for (int i = -4; i <= 4; i = i + 4) {
            currIdx = idx + i + j * arrayWidth;
            currRow = j + (int)idx / arrayWidth;
            minRow = currRow * arrayWidth;
            maxRow = minRow + arrayWidth;
            currCol = (idx + i) % arrayWidth;
            minCol = currCol;
            maxCol = minCol + arrayHeight * arrayWidth;
            if (currIdx >= minRow && currIdx >= minCol && currIdx < maxRow && currIdx < maxCol && idx != currIdx) {
                adjVelX = A[currIdx + 2] * 4;
                adjVelY = A[currIdx + 3] * 4;
                diffX = currCol - adjVelX;
                diffY = currRow - adjVelY;
                check = (diffX == x) && (diffY == y);
                if (check) aliveCell = 1;
            }
        }
    }

    // weighted steering: relies on local data mainly
    int new_steer[4] = { aliveCell, reboundCell, (int)((steer_local[2] + steer_global[2]) / 2), (int)(( steer_local[3] + steer_global[3]) / 2) };

    for (int i = 0; i < 4; i++) C[idx + i] = new_steer[i];
}
