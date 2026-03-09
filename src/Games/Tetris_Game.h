#ifndef _TETRIS_GAME_H
#define _TETRIS_GAME_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Preferences.h>

#include "ShapeBlock.h"
#include "Helpers/Keypad_4x4.h"
#include "Helpers/Color.h" 

#include "AppInterface.h"

extern TFT_eSPI tft;

// ================= Thông số khung chơi =================
extern int width;
extern int height;

#define xBoard              20                   // tọa độ x góc trên bên trái của bảng chơi
#define yBoard              4                    // tọa độ y góc trên bên trái của bảng chơi
#define nBlockSize          6                    // Kích thước mỗi ô vuông

#define nFieldWidth         10*nBlockSize        // Chiều rộng lưới chơi - 10 blocks
#define nFieldHeight        20*nBlockSize        // Chiều cao lưới chơi - 20 blocks

#define nInforWidth         (xBoard + nFieldWidth + nBlockSize) // Chiều rộng vùng thông tin bên phải

// ================= Nút bấm =================
#define BUTTON_LEFT     'L'
#define BUTTON_RIGHT    'R'
#define BUTTON_ROTATE   'O'
#define BUTTON_DOWN     'D'
#define BUTTON_STOP     'A'

// ================= Các khối + board =================
typedef struct {
    int size;               // Kích thước khối (4x4 hoặc 3x3)
    uint16_t color;         // Màu sắc khối
    uint8_t blocks[4][4];   // Dữ liệu khối (1: có ô, 0: không có ô)
    int x,y;                // tọa độ khối trên lưới chơi (tính theo ô)
    int rotation;           // trạng thái xoay (0-3)
} Tetromino;

// Các khối
const uint8_t (*list_Block[7])[4] = {I_Block, J_Block, L_Block, O_Block, S_Block, T_Block, Z_Block};

uint16_t boardState[nFieldHeight/nBlockSize][nFieldWidth/nBlockSize] = {0};     // mảng lưu trạng thái lưới chơi

// ================= Constants =================
#define MODE_BASIC  0
#define MODE_MEDIUM 1
#define MODE_HARD   2

const int DEFAULT_SPEEDS[] = {500, 400, 300}; // Basic, Medium, Hard
const char* MODE_NAMES[]   = {"BASIC", "MEDIUM", "HARD"};

// ================= Garbage Settings =================
const unsigned long GARBAGE_INTERVAL_MEDIUM = 25000;  // 25 giây
const unsigned long GARBAGE_INTERVAL_HARD   = 20000;  // 20 giây

// ================= Class =================
class TetrisGame: public AppInterface {
    private:
        // --- Game Logic Variables ---
        Tetromino current;
        Tetromino next;
        unsigned long lastFallTime = 0;
        int fallDelay; 
        int score = 0;
        bool isOver = false;
        bool fastDrop = false;

        // --- State Management ---
        bool isRunning = false;
        bool inMenu = true;
        bool isStopped = false;

        // --- Menu Variables ---
        int currentMode = MODE_BASIC;
        int menuIndex = 0;                  // 0: Start, 1: Mode, 2: Speed
        int highScores[3] = {0, 0, 0};      // Lưu điểm cao cho 3 mode

        // --- Garbage Management ---
        unsigned long lastGarbageTime = 0;  
        unsigned long garbageInterval = 0;  
        int minGarbageLines = 0;
        int maxGarbageLines = 0;

        // --- UI Constants ---
        const int MENU_START_Y = 50;
        const int MENU_GAP_Y = 20;

        Preferences prefs;
        TFT_eSprite img = TFT_eSprite(&tft); // Sprite đối tượng

    public:
        TetrisGame() {}
        ~TetrisGame() {}

        const char* getName() override {
            return "TETRIS";
        }

        const char* getIconColor() override {
            return "TETRIS";
        }

        bool showStatusBar() override { 
            return false; 
        }

        // ---------------- Start game ----------------
        void start() override {
            // Khởi tạo Sprite full màn hình
            img.setColorDepth(16);
            img.createSprite(width, height);
            
            isRunning = true;
            inMenu = true;

            // Khởi tạo bộ nhớ lưu trữ
            prefs.begin("tetris_data", false);
            loadHighScores();

            fallDelay = DEFAULT_SPEEDS[MODE_BASIC];
            adjustSetting(0); // Cập nhật thông số theo mode
            lastGarbageTime = millis();

            renderMenu(); // Vẽ menu lần đầu
        }

        // ---------------- Stop game ----------------
        void stop() override {
            isRunning = false;
            prefs.end();
            img.deleteSprite(); // Giải phóng RAM khi thoát
        }

        // ---------------- Update game ----------------
        void update() override {
            if (!isRunning) return;

            if (inMenu) {
                
            } 
            else {
                updateLogic();
            }
        }

        // ---------------- Handle input ----------------
        void handleInput(char key) override {
            if (inMenu) {
                handleMenuInput(key);
            } else {
                readInput(key);
            }
        }

    private:
        // ==================== RENDERING ENGINE (SPRITE) ====================
        
        // Hàm vẽ Menu 
        void renderMenu() {
            img.fillSprite(BLACK); 

            // Title
            img.setTextSize(2);
            img.setCursor(40, 10);
            img.setTextColor(CYAN);
            img.print("TETRIS");

            // Vẽ các dòng menu
            for (int i = 0; i <= 2; i++) {
                int y = MENU_START_Y + (i * MENU_GAP_Y);
                bool isSelected = (i == menuIndex);
                
                // Con trỏ >
                if (isSelected) {
                    img.setTextColor(YELLOW);
                    img.setCursor(5, y);
                    img.setTextSize(1);
                    img.print(">");
                }

                img.setTextColor(isSelected ? YELLOW : WHITE);
                img.setCursor(15, y);
                img.setTextSize(1);

                if (i == 0) img.print("START GAME");
                else if (i == 1) {
                    img.print("MODE:");
                    img.setCursor(50, y);
                    img.setTextColor(isSelected ? YELLOW : WHITE);
                    img.print("< "); img.print(MODE_NAMES[currentMode]); img.print(" >");
                }
                else if (i == 2) {
                    img.print("SPEED:");
                    img.setCursor(50, y);
                    img.setTextColor(isSelected ? YELLOW : WHITE);
                    img.print("< "); img.print(fallDelay); img.print(" ms >");
                }
            }

            // High Score
            img.setCursor(30, 30);
            img.setTextColor(ORANGE);
            img.setTextSize(1);
            img.print("High Score: "); 
            img.print(highScores[currentMode]);

            // Footer
            img.setTextSize(1);
            img.setTextColor(MAGENTA);
            img.setCursor(10, 115);
            img.print("2/8:Move  4/6:Adj  5:OK");

            // Đẩy ra màn hình
            img.pushSprite(0, 0);
        }

        // Hàm vẽ Game Loop
        void renderGame() {
            // 1. Xóa Sprite
            img.fillSprite(BLACK);

            // 2. Vẽ khung board
            img.drawRect(xBoard-1, yBoard-1, nFieldWidth+2, nFieldHeight+2, WHITE);

            // 3. Vẽ UI (Score, Next, Text)
            img.setTextColor(WHITE); 
            img.setTextSize(1);
            
            // Best Score
            img.setCursor(nInforWidth, yBoard + 5); 
            img.print("Best:"); 
            img.print(highScores[currentMode]);
            
            // Current Score
            img.setCursor(nInforWidth, yBoard + 25);
            img.print("Score:");
            // img.setCursor(nInforWidth, yBoard + 35);
            img.print(score);

            // Speed
            img.setCursor(nInforWidth, yBoard + 45);
            img.print("Speed:"); 
            // img.setCursor(nInforWidth, yBoard + 55);
            img.print(fallDelay);

            // Next Text
            img.setCursor(nInforWidth, yBoard + 70);
            img.print("Next:");

            // 4. Vẽ Board (Các khối đã cố định)
            int rows = nFieldHeight / nBlockSize;
            int cols = nFieldWidth / nBlockSize;
            
            for (int y = 0; y < rows; y++) {
                for (int x = 0; x < cols; x++) {
                    uint16_t color = boardState[y][x];
                    if (color != 0) {
                        int px = xBoard + x * nBlockSize;
                        int py = yBoard + y * nBlockSize;
                        img.fillRect(px, py, nBlockSize, nBlockSize, color);
                    }
                }
            }

            // 5. Vẽ Khối đang rơi (Current Tetromino)
            if (!isOver) {
                for (int r = 0; r < current.size; r++) {
                    for (int c = 0; c < current.size; c++) {
                        if (current.blocks[r][c]) {
                            int bx = current.x + c;
                            int by = current.y + r;
                            // Chỉ vẽ nếu nằm trong vùng hiển thị (y >= 0)
                            if (by >= 0 && by < rows && bx >= 0 && bx < cols) {
                                int px = xBoard + bx * nBlockSize;
                                int py = yBoard + by * nBlockSize;
                                img.fillRect(px, py, nBlockSize, nBlockSize, current.color);
                            }
                        }
                    }
                }
            }

            // 6. Vẽ Next Tetromino (Bên phải)
            int nextOffsetX = nInforWidth;
            int nextOffsetY = yBoard + 85;
            for (int r = 0; r < next.size; r++) {
                for (int c = 0; c < next.size; c++) {
                    if (next.blocks[r][c]) {
                        int px = nextOffsetX + c * nBlockSize;
                        int py = nextOffsetY + r * nBlockSize;
                        img.fillRect(px, py, nBlockSize, nBlockSize, next.color);
                    }
                }
            }

            // 7. Đẩy Sprite ra màn hình
            img.pushSprite(0, 0);
        }

        // ==================== GAME LOGIC ====================

        // Sinh ngẫu nhiên khối mới
        void spawnRandomTetromino() {
            uint16_t colorList[7] = {CYAN, BLUE, ORANGE, YELLOW, GREEN, MAGENTA, RED};
            
            // Lần đầu tiên
            if (next.size == 0) {
                int firstIndex = random(0, 7);
                next.size = 4;
                next.color = colorList[firstIndex];
                next.x = 0; 
                next.y = 0; 
                next.rotation = 0;
                for (int r = 0; r < 4; r++)
                    for (int c = 0; c < 4; c++)
                        next.blocks[r][c] = list_Block[firstIndex][r][c];
            }

            // Chuyển next -> current
            current = next;
            current.x = 3;
            current.y = -2; // Xuất phát từ trên nóc
            current.rotation = 0;

            // Tạo next mới
            int newIndex = random(0, 7);
            next.size = 4;
            next.color = colorList[newIndex];
            next.x = 0; next.y = 0; next.rotation = 0;
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    next.blocks[r][c] = list_Block[newIndex][r][c];
        }

        bool checkCollision(const Tetromino *t) {
            int rows = nFieldHeight / nBlockSize;
            int cols = nFieldWidth / nBlockSize;

            for (int r = 0; r < t->size; r++) {
                for (int c = 0; c < t->size; c++) {
                    if (t->blocks[r][c]) {
                        int bx = t->x + c;
                        int by = t->y + r;
                        
                        if (bx < 0 || bx >= cols || by >= rows) return true;
                        if (by >= 0 && boardState[by][bx] != 0) return true;
                    }
                }
            }
            return false;
        }
        
        void clearFullLines() {
            for(int y = 0; y < nFieldHeight/nBlockSize; y++) {
                bool isFull = true;
                for(int x = 0; x < nFieldWidth/nBlockSize; x++) {
                    if(!boardState[y][x]) {
                        isFull = false;
                        break;
                    }
                }

                if(isFull) {
                    for(int k = y; k > 0; k--) {
                        for(int x = 0; x < nFieldWidth/nBlockSize; x++) {
                            boardState[k][x] = boardState[k-1][x];
                        }
                    }
                    score += 10;
                    // Tăng tốc độ game khi ăn điểm
                    if (score > 0 && score % 100 == 0 && fallDelay > 100) {
                        fallDelay -= 10;
                    }
                }
            }
        }

        void adjustSetting(int dir) {
            if (menuIndex == 1) { // MODE
                currentMode += dir;
                if (currentMode < 0) currentMode = 2;
                if (currentMode > 2) currentMode = 0;
                
                fallDelay = DEFAULT_SPEEDS[currentMode];

                if(currentMode == MODE_BASIC) garbageInterval = 0;
                else if(currentMode == MODE_MEDIUM) {
                    garbageInterval = GARBAGE_INTERVAL_MEDIUM;
                    minGarbageLines = 1; maxGarbageLines = 2;
                }
                else if(currentMode == MODE_HARD) {
                    garbageInterval = GARBAGE_INTERVAL_HARD;
                    minGarbageLines = 1; maxGarbageLines = 3;
                }
            }
            else if (menuIndex == 2) { // SPEED
                int step = 10;
                fallDelay += (dir * step); 
                if (fallDelay < 100) fallDelay = 100;
                if (fallDelay > DEFAULT_SPEEDS[currentMode]) fallDelay = DEFAULT_SPEEDS[currentMode];
            }
        }

        void loadHighScores() {
            highScores[0] = prefs.getInt("hi_basic", 0);
            highScores[1] = prefs.getInt("hi_med", 0);
            highScores[2] = prefs.getInt("hi_hard", 0);
        }

        void saveHighScore() {
            if (score > highScores[currentMode]) {
                highScores[currentMode] = score;
                if (currentMode == 0) prefs.putInt("hi_basic", score);
                else if (currentMode == 1) prefs.putInt("hi_med", score);
                else if (currentMode == 2) prefs.putInt("hi_hard", score);
            }
        }

        void moveLeft() {
            if(isStopped) return;
            current.x--;
            if(checkCollision(&current)) current.x++;
        }

        void moveRight() {
            if(isStopped) return;
            current.x++;
            if(checkCollision(&current)) current.x--;
        }

        void moveDown() {
            if(isStopped) return;
            while (!checkCollision(&current)) {
                current.y++;
            }
            current.y--;
        }

        void rotate() {
            if(isStopped) return;
            uint16_t tmp[4][4];
            // Copy
            for(int i=0; i<4; i++) for(int j=0; j<4; j++) tmp[i][j] = current.blocks[i][j];
            
            // Rotate
            for (int r = 0; r < 4; r++)
                for (int c = 0; c < 4; c++)
                    current.blocks[r][c] = tmp[3 - c][r];

            // Check collision
            if (checkCollision(&current)) 
                for(int i=0; i<4; i++) for(int j=0; j<4; j++) current.blocks[i][j] = tmp[i][j];
        }

        void gameOver() {
            // Vẽ chữ Game Over lên Sprite hiện tại
            img.setCursor(xBoard + nBlockSize*2, yBoard + 8*nBlockSize);
            img.setTextColor(RED);
            img.setTextSize(2);
            img.print("Game Over");
            
            img.setCursor(xBoard + nBlockSize*1, yBoard + 12*nBlockSize);
            img.setTextColor(WHITE);
            img.setTextSize(1);
            img.print("Press 'A' to restart");
            
            img.pushSprite(0,0);
        }

        void resetGame() {
            int rows = nFieldHeight / nBlockSize;
            int cols = nFieldWidth / nBlockSize;
            for (int y = 0; y < rows; y++) {
                for(int x = 0; x < cols; x++) {
                    boardState[y][x] = 0;
                }
            }
            score = 0;
            isStopped = false;
            isOver = false;
        }

        void addGarbage() {
            int linesToAdd = random(minGarbageLines, maxGarbageLines + 1);
            int rows = nFieldHeight / nBlockSize;
            int cols = nFieldWidth / nBlockSize;
            uint16_t colors[] = {CYAN, BLUE, ORANGE, YELLOW, GREEN, MAGENTA, RED};

            for (int k = 0; k < linesToAdd; k++) {
                // Check game over ở hàng trên cùng
                for (int x = 0; x < cols; x++) {
                    if (boardState[0][x] != 0) {
                        isOver = true;
                        saveHighScore();
                        gameOver();
                        return;
                    }
                }
                // Shift up
                for (int y = 0; y < rows - 1; y++) {
                    for (int x = 0; x < cols; x++) {
                        boardState[y][x] = boardState[y + 1][x];
                    }
                }
                // Add garbage line
                int holePosition = random(0, cols);
                for (int x = 0; x < cols; x++) {
                    if (x == holePosition) boardState[rows - 1][x] = 0;
                    else boardState[rows - 1][x] = colors[random(0,7)];
                }
            }
            renderGame(); // Vẽ lại ngay sau khi thêm rác
        }

        void updateLogic() {
            if (isStopped || isOver) return;

            unsigned long now = millis();

            // Garbage Logic
            if (garbageInterval > 0 && now - lastGarbageTime > garbageInterval) {
                addGarbage();
                lastGarbageTime = now;
            }

            // Falling Logic
            if (now - lastFallTime > fallDelay) {
                current.y++;
                if (checkCollision(&current)) {
                    current.y--; // Back up
                    
                    // Lock block
                    int rows = nFieldHeight / nBlockSize;
                    int cols = nFieldWidth / nBlockSize;
                    for (int r = 0; r < current.size; r++) {
                        for (int c = 0; c < current.size; c++) {
                            if (current.blocks[r][c]) {
                                int bx = current.x + c;
                                int by = current.y + r;
                                if (by >= 0 && by < rows && bx >= 0 && bx < cols)
                                    boardState[by][bx] = current.color;
                            }
                        }
                    }

                    clearFullLines();
                    spawnRandomTetromino();

                    // Check Game Over ngay sau khi sinh khối mới
                    if (checkCollision(&current)) {
                        isOver = true;
                        saveHighScore();
                        Serial.println("Game Over!");
                        gameOver();
                        return; 
                    }
                }
                
                if(!isOver) renderGame();
                
                lastFallTime = now;
            }
        }

        void readInput(char key) {
            fastDrop = (key == BUTTON_DOWN);
            if (key) {
                if(isOver) {
                    if (key == BUTTON_STOP) {
                        saveHighScore();
                        inMenu = true;
                        renderGame();
                    }
                    return;
                }

                switch (key) {
                    case BUTTON_LEFT: 
                        moveLeft(); 
                        break;
                    case BUTTON_RIGHT: 
                        moveRight(); 
                        break;
                    case BUTTON_DOWN: 
                        moveDown(); 
                        break;
                    case BUTTON_ROTATE: 
                        rotate(); 
                        break;
                    case BUTTON_STOP: 
                        isStopped = !isStopped; 
                        break;
                }
                
                // Sau khi di chuyển, vẽ lại ngay lập tức để phản hồi nhanh
                if (!isStopped && !isOver) renderGame();
            }
        }

        void handleMenuInput(char key) {
            bool needRedraw = false;
            switch(key) {
                case 'U': // UP
                    menuIndex--; 
                    if (menuIndex < 0) menuIndex = 2;
                    needRedraw = true; 
                    break;
                case 'D': // DOWN
                    menuIndex++; 
                    if (menuIndex > 2) menuIndex = 0;
                    needRedraw = true; 
                    break;
                case 'L': // LEFT
                    adjustSetting(-1); 
                    needRedraw = true; 
                    break;
                case 'R': // RIGHT
                    adjustSetting(1); 
                    needRedraw = true; 
                    break;
                case 'O': // SELECT
                    if (menuIndex == 0) { 
                        inMenu = false;
                        resetGame();
                        spawnRandomTetromino();
                        renderGame(); // Vẽ frame game đầu tiên
                    }
                    return; 
            }
            if (needRedraw) renderMenu();
        }
};

#endif //_TETRIS_GAME_H