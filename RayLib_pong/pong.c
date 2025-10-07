#include <stdio.h>
#include <raylib.h>
#include <stdbool.h>

typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER
} GamePhase;

GamePhase GP = STATE_MENU;

struct Ball{
    float posX, posY;
    float W, H;
    float velx, vely;
    Color color;
};

struct Player{
    float posX, posY;
    float W, H;
    Color color;
    int score;
};

struct GameState {
    bool Close;
    int winner;
    struct Ball ball;
    struct Player player1;
    struct Player player2;
};

struct GameState GS;

void InitGameState() {
    GS = (struct GameState){
        .Close = false,
        .winner = 0,
        .ball = {400.0f, 300.0f, 20.0f, 20.0f, 0.0f, 0.0f, WHITE},
        .player1 = {50.0f, 250.0f, 10.0f, 100.0f, WHITE, 0},
        .player2 = {740.0f, 250.0f, 10.0f, 100.0f, WHITE, 0}
    };
}

void reset() {
    GS.player1.score = 0;
    GS.player1.posX = 50.0f;
    GS.player1.posY = 250.0f;

    GS.player2.score = 0;
    GS.player2.posX = 740.0f;
    GS.player2.posY = 250.0f;
}

void render() {
    BeginDrawing();
    ClearBackground(BLACK);

    int textWidth = MeasureText(TextFormat("%d | %d", GS.player1.score, GS.player2.score), 20);

    switch (GP) {
        case STATE_MENU:

            // Draw Player1
            DrawRectangle((int)GS.player1.posX, (int)GS.player1.posY, (int)GS.player1.W, (int)GS.player1.H, GS.player1.color);

            // Draw Player2
            DrawRectangle((int)GS.player2.posX, (int)GS.player2.posY, (int)GS.player2.W, (int)GS.player2.H, GS.player2.color);

            // Draw Ball
            DrawRectangle((int)GS.ball.posX - 10, (int)GS.ball.posY, (int)GS.ball.W, (int)GS.ball.H, GS.ball.color);

            DrawText("PONG", GetScreenWidth() / 2 - MeasureText("PONG", 40) / 2, 20, 40, LIGHTGRAY);

            DrawText("Start", GetScreenWidth() / 2 - MeasureText("Start", 20) / 2, GetScreenHeight() / 2 + 25, 20, LIGHTGRAY);

            DrawText(TextFormat("%d | %d", GS.player1.score, GS.player2.score), GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 10, 20, LIGHTGRAY);
            break;

        case STATE_PLAYING:
            DrawRectangle((int)GS.player1.posX, (int)GS.player1.posY, (int)GS.player1.W, (int)GS.player1.H, GS.player1.color);
            DrawRectangle((int)GS.player2.posX, (int)GS.player2.posY, (int)GS.player2.W, (int)GS.player2.H, GS.player2.color);
            DrawRectangle((int)GS.ball.posX - 10, (int)GS.ball.posY, (int)GS.ball.W, (int)GS.ball.H, GS.ball.color);

            DrawText("PONG", GetScreenWidth() / 2 - MeasureText("PONG", 40) / 2, 20, 40, LIGHTGRAY);
            DrawText(TextFormat("%d | %d", GS.player1.score, GS.player2.score), GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 10, 20, LIGHTGRAY);
            break;
        case STATE_GAMEOVER:

            // Draw Player1
            DrawRectangle((int)GS.player1.posX, (int)GS.player1.posY, (int)GS.player1.W, (int)GS.player1.H, GS.player1.color);

            // Draw Player2
            DrawRectangle((int)GS.player2.posX, (int)GS.player2.posY, (int)GS.player2.W, (int)GS.player2.H, GS.player2.color);

            // Draw Ball
            DrawRectangle((int)GS.ball.posX - 10, (int)GS.ball.posY, (int)GS.ball.W, (int)GS.ball.H, GS.ball.color);

            DrawText("PONG", GetScreenWidth() / 2 - MeasureText("PONG", 40) / 2, 20, 40, LIGHTGRAY);

            DrawText(TextFormat("Player %d wins!", GS.winner), GetScreenWidth() / 2 - MeasureText(TextFormat("Player %d wins!", GS.winner), 40) / 2, GetScreenHeight() / 2 - 60, 40, WHITE);

            DrawText("Play again", GetScreenWidth() / 2 - MeasureText("Play again", 20) / 2, GetScreenHeight() / 2 + 25, 20, LIGHTGRAY);

            DrawText(TextFormat("%d | %d", GS.player1.score, GS.player2.score), GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 10, 20, LIGHTGRAY);
            break;
        default:
            break;
    }
    EndDrawing();
}

bool CheckCollision(struct Ball ball, struct Player player) {
    return (ball.posX < player.posX + player.W &&
            ball.posX + ball.W > player.posX &&
            ball.posY < player.posY + player.H &&
            ball.posY + ball.H > player.posY);
}

void Movement_and_collision() {
    if (GP == STATE_PLAYING) {
        float dt = GetFrameTime();
        GS.ball.posX += GS.ball.velx * dt;
        GS.ball.posY += GS.ball.vely * dt;

        //walls
        if (GS.ball.posY <= 0 || GS.ball.posY + GS.ball.H >= GetScreenHeight()) {
            GS.ball.vely *= -1.0f;
            if (GS.ball.posY <= 0) GS.ball.posY = 0;
            else GS.ball.posY = GetScreenHeight() - GS.ball.H;
        }
        // scoring
        if (GS.ball.posX <= 0) {
            GS.player2.score++;
            GS.ball = (struct Ball){400.0f, 300.0f, 20.0f, 20.0f, 150.0f, 75.0f, WHITE};
        } else if (GS.ball.posX + GS.ball.W >= GetScreenWidth()) {
            GS.player1.score++;
            GS.ball = (struct Ball){400.0f, 300.0f, 20.0f, 20.0f, -150.0f, 75.0f, WHITE};
        }

        if (GS.player1.score == 5) {
                GS.winner = 1;
                GP = STATE_GAMEOVER;
                reset();
        } else if (GS.player2.score == 5) {
                GS.winner = 2;
                GP = STATE_GAMEOVER;
                reset();
        }

        // player collision
        if (CheckCollision(GS.ball, GS.player1)) {
            GS.ball.velx *= -1.0f;
            GS.ball.posX = GS.player1.posX + GS.player1.W;
        } else if (CheckCollision(GS.ball, GS.player2)) {
            GS.ball.velx *= -1.0f;
            GS.ball.posX = GS.player2.posX - GS.ball.W;
        }   
    }
}

void HandleInput() {
    float dt = GetFrameTime();
    switch (GP) {
        case STATE_MENU:
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 mousePos = GetMousePosition();
                if (mousePos.x >= GetScreenWidth() / 2 - MeasureText("Start", 20) / 2 &&
                    mousePos.x <= GetScreenWidth() / 2 + MeasureText("Start", 20) / 2 &&
                    mousePos.y >= GetScreenHeight() / 2 + 25 &&
                    mousePos.y <= GetScreenHeight() / 2 + 45) {
                    GP = STATE_PLAYING;
                    GS.ball.velx = 150.0f;
                    GS.ball.vely = 150.0f;
                }
            }
            break;
        case STATE_PLAYING:
            if (IsKeyDown(KEY_W) && GS.player1.posY > 0) {
                GS.player1.posY -= 300.0f * dt;
            }
            if (IsKeyDown(KEY_S) && GS.player1.posY + GS.player1.H < GetScreenHeight()) {
                GS.player1.posY += 300.0f * dt;
            } 
            if (IsKeyDown(KEY_UP) && GS.player2.posY > 0) {
                GS.player2.posY -= 300.0f * dt;
            } 
            if (IsKeyDown(KEY_DOWN) && GS.player2.posY + GS.player2.H < GetScreenHeight()) {
                GS.player2.posY += 300.0f * dt;
            }
            break;
        case STATE_GAMEOVER:
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                
                if (
                    GetMousePosition().x >= GetScreenWidth() / 2 - MeasureText("Play again", 20) / 2 &&
                    GetMousePosition().x <= GetScreenWidth() / 2 + MeasureText("Play again", 20) / 2 &&
                    GetMousePosition().y >= GetScreenHeight() / 2 + 25 &&
                    GetMousePosition().y <= GetScreenHeight() / 2 + 45) {
                        GP = STATE_PLAYING;
                        GS.winner = 0;
                        GS.ball.velx = 150.0f;
                        GS.ball.vely = 150.0f;
                }
            }
            break;
    }
}

int main() {
    InitWindow(800, 600, "pong");

    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    InitGameState();

    while(!WindowShouldClose() && !GS.Close) {
        Movement_and_collision();
        HandleInput();
        render();
    }
    CloseWindow();
    return 0;
}
