#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#define EMPTY 0
#define X_VAL 1
#define O_VAL 2

void draw_x(SDL_Renderer *ren, int row, int col, int margin_x, int margin_y, int cell_x, int cell_y) {
    int x0 = margin_x + col * cell_x;
    int y0 = margin_y + row * cell_y;
    int x1 = x0 + cell_x;
    int y1 = y0 + cell_y;

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDrawLine(ren, x0 + 10, y0 + 10, x1 - 10, y1 - 10);
    SDL_RenderDrawLine(ren, x0 + 10, y1 - 10, x1 - 10, y0 + 10);
}

void draw_o(SDL_Renderer *ren, int row, int col, int margin_x, int margin_y, int cell_x, int cell_y) {
    int cx = margin_x + col * cell_x + cell_x / 2;
    int cy = margin_y + row * cell_y + cell_y / 2;
    int r = (cell_x < cell_y ? cell_x : cell_y) / 2 - 10;

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

    for (int w = -r; w <= r; w++) {
        for (int h = -r; h <= r; h++) {
            if (w*w + h*h <= r*r && w*w + h*h >= (r-2)*(r-2)) {
                SDL_RenderDrawPoint(ren, cx + w, cy + h);
            }
        }
    }
}
bool board_full(int board[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == EMPTY) { // empty spot found
                return false;
            }
        }
    }
    return true; // no empty spots
}

int check_win(int board[3][3], int p) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == p && board[i][1] == p && board[i][2] == p) {
            return p;
        }
    }
    for (int j = 0; j < 3; j++) {
        if (board[0][j] == p && board[1][j] == p && board[2][j] == p) {
            return p;
        }
    }

    if (board[0][0] == p && board[1][1] == p && board[2][2] == p) return p;
    if (board[0][2] == p && board[1][1] == p && board[2][0] == p) return p;

    if (board_full(board)) {
        return 5;
    }

    return 0;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win = SDL_CreateWindow("Tic Tac Toe",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        400, 450, 0);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    int margin_x = 50, margin_y = 50;
    int cell_x = 100, cell_y = 100;

    int board[3][3] = {0}; 
    int current_player = X_VAL;

    int running = 1;
    SDL_Event e;

    while (running) {
	check_win(board, 1);
        // clear background
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        // draw grid (white)
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        for (int i = 1; i <= 2; i++) {
            int x = margin_x + i * cell_x;
            SDL_RenderDrawLine(ren, x, margin_y, x, margin_y + 3 * cell_y);
        }
        for (int i = 1; i <= 2; i++) {
            int y = margin_y + i * cell_y;
            SDL_RenderDrawLine(ren, margin_x, y, margin_x + 3 * cell_x, y);
        }
        SDL_RenderDrawLine(ren, margin_x, margin_y, margin_x + 3*cell_x, margin_y);
        SDL_RenderDrawLine(ren, margin_x, margin_y, margin_x, margin_y + 3*cell_y);
        SDL_RenderDrawLine(ren, margin_x + 3*cell_x, margin_y, margin_x + 3*cell_x, margin_y + 3*cell_y);
        SDL_RenderDrawLine(ren, margin_x, margin_y + 3*cell_y, margin_x + 3*cell_x, margin_y + 3*cell_y);

        // draw all X and O from board state
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (board[r][c] == X_VAL) {
                    draw_x(ren, r, c, margin_x, margin_y, cell_x, cell_y);
                } else if (board[r][c] == O_VAL) {
                    draw_o(ren, r, c, margin_x, margin_y, cell_x, cell_y);
                }
            }
        }

        // handle events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (mx >= margin_x && mx < margin_x + 3*cell_x &&
                    my >= margin_y && my < margin_y + 3*cell_y) {
                    int col = (mx - margin_x) / cell_x;
                    int row = (my - margin_y) / cell_y;

                    if (board[row][col] == EMPTY) {
                        board[row][col] = current_player;
			if(check_win(board, current_player) == current_player) {
				printf("Player %c won! \n", (current_player == 1)? 'X': 'O');
				fflush(stdout);
				SDL_DestroyRenderer(ren);
				SDL_DestroyWindow(win);
				SDL_Quit();
				main();
				return 0;
			}
			else if (check_win(board, current_player) == 5) {
				printf("Draw! \n");
                                fflush(stdout);
                                SDL_DestroyRenderer(ren);
                                SDL_DestroyWindow(win);
                                SDL_Quit();
                                main();
                                return 0;
			}
                        current_player = (current_player == X_VAL) ? O_VAL : X_VAL;
                    }
                }
            }
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16); // ~60fps
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
