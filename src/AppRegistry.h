#ifndef _APP_REGISTRY_H
#define _APP_REGISTRY_H

#include "Games/Tetris_Game.h"
#include "Games/Snake_Game.h" 
#include "Games/App_WebSync.h"
#include "SettingsApp.h"

// 1. Khởi tạo các đối tượng game
TetrisGame tetris;
SnakeGame snake;
App_WebSync webSync;
SettingsApp settingsApp;

// 2. Đưa vào danh sách
AppInterface* myApps[] = {
    &tetris,
    &snake,
    &webSync,
    &settingsApp
};

// 3. Đếm số lượng
const int APP_COUNT = sizeof(myApps) / sizeof(myApps[0]);

#endif