#ifndef _SHAPE_BLOCK_H
#define _SHAPE_BLOCK_H

#include <Arduino.h>

// ================= Hình khối =================
// All blocks are stored as 4x4 matrices (pad 3x3 shapes with zeros)

const uint8_t I_Block[4][4] = {{0,0,0,0},
                               {0,0,0,0},
                               {1,1,1,1},
                               {0,0,0,0} };

const uint8_t J_Block[4][4] = { {0,0,0,0},
                               {0,0,1,0},
                               {1,1,1,0},
                               {0,0,0,0} };

const uint8_t L_Block[4][4] = { {0,0,0,0},
                               {1,1,1,0},
                               {0,0,1,0},
                               {0,0,0,0} };

const uint8_t O_Block[4][4] = { {0,0,0,0},
                               {0,1,1,0},
                               {0,1,1,0},
                               {0,0,0,0} };

const uint8_t S_Block[4][4] = { {0,0,0,0},
                               {0,1,1,0},
                               {1,1,0,0},
                               {0,0,0,0} };

const uint8_t T_Block[4][4] = { {0,0,0,0},
                               {1,1,1,0},
                               {0,1,0,0},
                               {0,0,0,0} };

const uint8_t Z_Block[4][4] = { {0,0,0,0},
                               {1,1,0,0},
                               {0,1,1,0},
                               {0,0,0,0} };

#endif // _SHAPE_BLOCK_H