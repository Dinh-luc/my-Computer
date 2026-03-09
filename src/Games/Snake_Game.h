#ifndef _SNAKE_GAME_H_
#define _SNAKE_GAME_H_

#include <Arduino.h>
#include <Preferences.h>
#include <TFT_eSPI.h>

#include "Helpers/Keypad_4x4.h"
#include "Helpers/Color.h"

#include "AppInterface.h"

extern TFT_eSPI tft;

extern int width;
extern int height;

// ================= Nút bấm =================
#define BUTTON_LEFT     'L'
#define BUTTON_RIGHT    'R'
#define BUTTON_OK       'O'
#define BUTTON_DOWN     'D'
#define BUTTON_UP       'U'
#define BUTTON_STOP     'A'

// ================= Kích thước khung =================
#define nBlockSize      6

#define nFieldWidth      width - nBlockSize *2
#define nFieldHeight     height - 20 - nBlockSize

typedef struct {
    int x,y;
    int x0,y0;
} Point;

typedef struct {
    Point snake[100];    
    int len = 3;
} Snake;

class SnakeGame: public AppInterface {
    private:
        Preferences prefs;
        TFT_eSprite img = TFT_eSprite(&tft); 

        // --- State Management ---
        bool isRunning = false;
        bool inMenu = true;
        bool isStopped = false;

        enum DirectionState {
            LEFT,
            RIGHT,
            UP,
            DOWN
        } currentDirection = RIGHT;
        DirectionState lastMovedDirection = RIGHT;

        // --- Game logic ---
        Point food;
        Snake snake;
        const int DIRECTION = nBlockSize;   // ditance moved per step
        Point direction;                    // current direction vector (dx, dy)

        Snake aiSnake;
        Point aiDirection;
        uint16_t aiColor = BLUE;

        bool isOver = false;
        int score = 0;
        int speed = 200;
        unsigned long lastMoveTime = 0;

        // --- UI Constants ---
        const int MENU_START_Y = 50;
        const int MENU_GAP_Y = 20;

    public:
        SnakeGame() {}
        ~SnakeGame() {}

        const char* getName() override {
            return "SNAKE";
        }

        const char* getIconColor() override {
            return "SNAKE";
        }

        bool showStatusBar() override { 
            return false; 
        }


        // ---------------- Start game ----------------
        void start() override {
            img.setColorDepth(16);
            img.createSprite(width, height);
            
            isRunning = true;
            inMenu = true;

            // Khởi tạo bộ nhớ lưu trữ
            prefs.begin("tetris_data", false);
            loadHighScores();

            renderMenu();
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
            readInput(key);
        }

        // ---------------- Stop game ----------------
        void stop() override {
            // Dọn dẹp khi thoát game Snake
        }

    private:
        void renderMenu() {
            img.fillSprite(BLACK); 
            img.setTextFont(4);

            // title
            img.setTextColor(WHITE);
            img.setTextSize(2);
            img.setFreeFont();
            img.drawString("SNAKE", 51, 10);

            // High Score
            img.setTextSize(1);
            img.drawString("High Score: 0000", 33, 37);

            // Start button
            img.fillRoundRect(41, 62, 79, 39, 15, WHITE);
            img.setTextColor(0x0);
            img.setTextSize(2);
            img.drawString("START", 51, 74);

            // Đẩy ra màn hình
            img.pushSprite(0, 0);
        }

        void renderGame() {
            img.fillSprite(BLACK);

            img.drawRect(3, 18, width - 6, height - 23, WHITE);

            img.setTextColor(WHITE);
            img.setTextSize(1);
            img.setFreeFont();
            img.drawString("Score: " + String(score), 6, 5);
            
            // Vẽ Snake và Food
            drawSnake(snake, RED);   
            drawSnake(aiSnake, BLUE); 
            drawFood();

            img.pushSprite(0, 0);
        }   

        void drawSnake(Snake &s, uint16_t color) {
            for (int i = 0; i < s.len; i++) {
                if (i == 0) {
                    img.drawCircle(s.snake[i].x, s.snake[i].y, nBlockSize/2, color);
                }
                else 
                    img.fillCircle(s.snake[i].x, s.snake[i].y, nBlockSize/2, color);
            }
        }

        void spawnFood() {
            food.x = random(3 + nBlockSize/2, width - 6 - nBlockSize/2);
            food.y = random(18 + nBlockSize/2, height - 23 - nBlockSize/2);
        }

        void drawFood() {
            img.fillCircle(food.x, food.y, nBlockSize/2, GREEN);
        }

        void loadHighScores() {

        }

        void saveHighScore() {

        }

        void gameOver() {
            img.setFreeFont(&FreeMonoBold12pt7b);
            img.drawString("Game Over!", 10, 55);
            img.pushSprite(0,0);
        }

        void resetGame() {
            score = 0;
            isStopped = false;
            isOver = false;

            currentDirection = RIGHT;
            lastMovedDirection = RIGHT;

            snake.len = 3;

            // khởi tạo hướng ban đầu
            direction.x = DIRECTION;
            direction.y = 0;

            // khởi tạo tọa độ rắn
            snake.snake[0].x = width/2;
            snake.snake[0].y = height/2;
            snake.snake[1].x = snake.snake[0].x - nBlockSize;
            snake.snake[1].y = snake.snake[0].y;
            snake.snake[2].x = snake.snake[1].x - nBlockSize;
            snake.snake[2].y = snake.snake[1].y;

            // Khởi tạo Rắn AI (Nằm ở góc dưới)
            aiSnake.len = 3;
            aiDirection.x = -nBlockSize; 
            aiDirection.y = 0;

            aiSnake.snake[0].x = width - 20;
            aiSnake.snake[0].y = height - 30;
            aiSnake.snake[1].x = aiSnake.snake[0].x + nBlockSize;
            aiSnake.snake[1].y = aiSnake.snake[0].y;
            aiSnake.snake[2].x = aiSnake.snake[1].x + nBlockSize;
            aiSnake.snake[2].y = aiSnake.snake[1].y;

            // khởi tạo thức ăn
            spawnFood();
        }

        void turnLeft() {
            if (isStopped || lastMovedDirection == RIGHT || lastMovedDirection == LEFT) return;
            direction.x = -DIRECTION;
            direction.y = 0;
            currentDirection = LEFT; 
        }

        void turnRight() {
            if (isStopped || lastMovedDirection == LEFT || lastMovedDirection == RIGHT) return;
            direction.x = DIRECTION;
            direction.y = 0;
            currentDirection = RIGHT;
        }

        void turnDown() {
            if (isStopped || lastMovedDirection == UP || lastMovedDirection == DOWN) return;
            direction.x = 0;
            direction.y = DIRECTION;
            currentDirection = DOWN;
        }

        void turnUp() {
            if (isStopped || lastMovedDirection == DOWN || lastMovedDirection == UP) return;
            direction.x = 0;
            direction.y = -DIRECTION;
            currentDirection = UP;
        }

        bool checkCollision() {
            // wall
            if (snake.snake[0].x <= 5 || snake.snake[0].x >= width - 5) return true;
            if (snake.snake[0].y <= 20 || snake.snake[0].y >= height - 8) return true;

            // self biting
            for (int i = 1; i < snake.len; i++) {
                if (snake.snake[0].x == snake.snake[i]. x && snake.snake[0].y == snake.snake[i].y) {
                    return true;
                }
            }

            // collosion with AI
            for (int i = 0; i < aiSnake.len; i++) {
                if (abs(snake.snake[0].x - aiSnake.snake[i].x) < nBlockSize && abs(snake.snake[0].y - aiSnake.snake[i].y) < nBlockSize) {
                    return true;
                }
            }

            // collosion with player
            for (int i = 0; i < snake.len; i++) {
                if (abs(aiSnake.snake[0].x - snake.snake[i].x) < nBlockSize && abs(aiSnake.snake[0].y - snake.snake[i].y) < nBlockSize) {
                    return true;
                }
            }

            return false;
        }

        bool checkFood() {
            if (abs(snake.snake[0].x - food.x) < nBlockSize && abs(snake.snake[0].y - food.y) < nBlockSize)
                return true;
            return false;
        }

        bool isSafe(int nextX, int nextY) {
            // check for wall collisions
            if (nextX <= 5 || nextX >= width - 5) return false;
            if (nextY <= 20 || nextY >= height - 8) return false;

            // self biting check
            for (int i = 0; i < aiSnake.len; i++) {
                if (aiSnake.snake[i].x == nextX && aiSnake.snake[i].y == nextY) return false;
            }

            // check collision with player 
            for (int i = 0; i < snake.len; i++) {
                if (snake.snake[i].x == nextX && snake.snake[i].y == nextY) return false;
            }

            return true; 
        }

        // FSM quyết định hướng đi
        void decideAIDirection() {
            int headX = aiSnake.snake[0].x;
            int headY = aiSnake.snake[0].y;
            
            Point bestDir = {0, 0};

            // === TRẠNG THÁI 1: HUNT (Tìm đường đến mồi) ===
            // Ưu tiên chạy theo trục X hoặc Y hướng về phía food, và đảm bảo hướng đó SAFE, đồng thời không quay đầu 180 độ
            if (food.x > headX && isSafe(headX + nBlockSize, headY) && aiDirection.x != -nBlockSize) {
                bestDir = {nBlockSize, 0};
            } else if (food.x < headX && isSafe(headX - nBlockSize, headY) && aiDirection.x != nBlockSize) {
                bestDir = {-nBlockSize, 0};
            } else if (food.y > headY && isSafe(headX, headY + nBlockSize) && aiDirection.y != -nBlockSize) {
                bestDir = {0, nBlockSize};
            } else if (food.y < headY && isSafe(headX, headY - nBlockSize) && aiDirection.y != nBlockSize) {
                bestDir = {0, -nBlockSize};
            }

            // === TRẠNG THÁI 2: SURVIVE (Sinh tồn) ===
            // Nếu hướng đến mồi bị kẹt, từ bỏ mồi và tìm BẤT KỲ hướng nào an toàn để không chết
            if (bestDir.x == 0 && bestDir.y == 0) {
                if (isSafe(headX + nBlockSize, headY) && aiDirection.x != -nBlockSize) 
                    bestDir = {nBlockSize, 0};
                else if (isSafe(headX - nBlockSize, headY) && aiDirection.x != nBlockSize) 
                    bestDir = {-nBlockSize, 0};
                else if (isSafe(headX, headY + nBlockSize) && aiDirection.y != -nBlockSize) 
                    bestDir = {0, nBlockSize};
                else if (isSafe(headX, headY - nBlockSize) && aiDirection.y != nBlockSize) 
                    bestDir = {0, -nBlockSize};
            }

            // Cập nhật hướng đi (Nếu cả 4 bề thọ địch, bestDir vẫn = 0, AI đành chấp nhận đâm tường/chết)
            if (bestDir.x != 0 || bestDir.y != 0) {
                aiDirection = bestDir;
            }
        }

        void readInput(char key) {
            if (key) {
                Serial.print("Snake input: ");
                Serial.println(key);

                if (isOver) {
                    if (key == BUTTON_OK) {
                        saveHighScore();
                        resetGame();
                        renderGame();
                        return;
                    }
                }

                if (inMenu) {
                    if (key == BUTTON_OK) {
                        inMenu = false;
                        resetGame();
                        renderGame();
                        return;
                    }
                }

                switch(key) {
                    case BUTTON_LEFT:
                        turnLeft();
                        break;
                    case BUTTON_RIGHT:
                        turnRight();
                        break;
                    case BUTTON_DOWN:
                        turnDown();
                        break;
                    case BUTTON_UP:
                        turnUp();
                        break;
                    case BUTTON_STOP:
                        isStopped = !isStopped;
                        break;
                }
            }
        }

        void updateLogic() {
            if (isStopped || isOver) return;

            unsigned long now = millis();

            if (now - lastMoveTime > speed) {
                decideAIDirection();
                lastMovedDirection = currentDirection;

                for (int i = 0; i < aiSnake.len; i++) {
                    aiSnake.snake[i].x0 = aiSnake.snake[i].x;
                    aiSnake.snake[i].y0 = aiSnake.snake[i].y;
                    if (i == 0) {
                        aiSnake.snake[i].x += aiDirection.x;
                        aiSnake.snake[i].y += aiDirection.y;
                    } else {
                        aiSnake.snake[i].x = aiSnake.snake[i-1].x0;
                        aiSnake.snake[i].y = aiSnake.snake[i-1].y0;
                    }
                }

                for (int i = 0; i < snake.len; i++) {
                    snake.snake[i].x0 = snake.snake[i].x;
                    snake.snake[i].y0 = snake.snake[i].y;

                    if ( i == 0) {
                        snake.snake[i].x += direction.x;
                        snake.snake[i].y += direction.y;
                    }
                    else {
                        snake.snake[i].x = snake.snake[i-1].x0;
                        snake.snake[i].y = snake.snake[i-1].y0;
                    }
                }

                if (checkCollision()) {
                    isOver = true;
                    gameOver();
                    return;
                }

                if (abs(aiSnake.snake[0].x - food.x) < nBlockSize && abs(aiSnake.snake[0].y - food.y) < nBlockSize) {
                    int lastIndex = aiSnake.len - 1;
                    aiSnake.snake[aiSnake.len].x = aiSnake.snake[lastIndex].x;
                    aiSnake.snake[aiSnake.len].y = aiSnake.snake[lastIndex].y;
                    aiSnake.len++;
                    spawnFood(); 
                }

                if (checkFood()) {
                    score += 10;

                    int lastIndex = snake.len - 1;
                    snake.snake[snake.len].x = snake.snake[lastIndex].x;
                    snake.snake[snake.len].y = snake.snake[lastIndex].y;

                    snake.len++;
                    spawnFood();
                }

                if (!isOver) renderGame();

                lastMoveTime = now;
            }
        }
};

#endif