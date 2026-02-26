#ifndef _APP_REGISTRY_H
#define _APP_REGISTRY_H

#include "Games/Tetris_Game.h"
#include "Games/Snake_Game.h" 

// 1. Khởi tạo các đối tượng game
TetrisGame tetris;
SnakeGame snake;

// 2. Đưa vào danh sách
AppInterface* myApps[] = {
    &tetris,
    &snake
};

// 3. Đếm số lượng
const int APP_COUNT = sizeof(myApps) / sizeof(myApps[0]);

#endif