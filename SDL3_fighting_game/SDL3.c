#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_scancode.h>
#include <math.h>

typedef struct {
    int health;
    SDL_Texture* texture;

    float posx, posy;
    float velx, vely;

    int H; int W;

    int facingRight;
    int frame;

    int att_frame;

    int player_iframes;
    int player_dash_cooldown;

    int damage;
    int attack_cooldown;

    int stunned;
    int parrying;
    int parry_cooldown;
} Player;

void Init_player(Player* p, SDL_Texture* tex, int x, int y, int facingRight) {
    p->health = 100;
    p->texture = tex;

    p->posx = x;
    p->posy = y;
    p->velx = 0;
    p->vely = 0;

    p->W = 80;
    p->H = 80;

    p->facingRight = facingRight;
    p->frame = 0;
    printf("initialized player \n");

    p->player_iframes = 0;
    p->player_dash_cooldown = 0;

    p->damage = 20;
    p->attack_cooldown = 0;

    p->att_frame = 0;

    p->stunned = 0;
    p->parrying = 0;
    p->parry_cooldown = 0;
}

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("Failed to load BMP: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (!tex) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_DestroySurface(surface);
        return NULL;
    }
    SDL_DestroySurface(surface);
    printf("loaded texture \n");
    return tex;
}


void destroy(SDL_Renderer *ren, SDL_Window *win) {
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int game_end(SDL_Renderer *ren , SDL_Window *win, int winner) {
    SDL_FRect rect = {0, 0, 1280*2, 720*2};
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderFillRect(ren, &rect);

    // Winner text
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    if (winner == 1) {
        SDL_SetRenderScale(ren, 2, 2);
        SDL_RenderDebugTextFormat(ren, (1280/2 - 100)/2, 720 / 4, "Player 1 Wins!");
    } else {
        SDL_SetRenderScale(ren, 2, 2);
        SDL_RenderDebugTextFormat(ren, (1280/2 - 100)/2, 720 / 4, "Player 2 Wins!");
    }

    SDL_FRect againBtn = {1280/2 - 150, 400, 300, 80};
    SDL_FRect quitBtn  = {1280/2 - 150, 520, 300, 80};

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(ren, ((int)againBtn.x + 80)/2, ((int)againBtn.y + 30)/2, "Play Again");
    SDL_RenderDebugTextFormat(ren, ((int)quitBtn.x + 120)/2, ((int)quitBtn.y + 30)/2, "Quit");

    SDL_RenderPresent(ren);
    SDL_Event e;
    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                destroy(ren, win);
                return 0;
            }
            if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                int mx = e.button.x;
                int my = e.button.y;
                if (mx >= againBtn.x && mx <= againBtn.x + againBtn.w &&
                    my >= againBtn.y && my <= againBtn.y + againBtn.h) {
                    setup(ren, win);
                    return 1;
                }
                if (mx >= quitBtn.x && mx <= quitBtn.x + quitBtn.w &&
                    my >= quitBtn.y && my <= quitBtn.y + quitBtn.h) {
                    destroy(ren, win);
                    return 0;
                }
            }
        }
        SDL_Delay(16);
    }
}

void game_loop(SDL_Renderer* ren, SDL_Window* win) {
    int running = 1;
    SDL_Event event;

    SDL_Texture* tex_red = loadTexture(ren, "assets/stickfigure_red.bmp");
    SDL_Texture* tex_white = loadTexture(ren, "assets/stickfigure_white.bmp");

    SDL_Texture* walking_red_tex_1 = loadTexture(ren, "assets/stickfigure_red_walking_1.bmp");
    SDL_Texture* walking_red_tex_2 = loadTexture(ren, "assets/stickfigure_red_walking_2.bmp");

    SDL_Texture* attacking_white_tex = loadTexture(ren, "assets/stickfigure_white_attack.bmp");
    SDL_Texture* attacking_red_tex = loadTexture(ren, "assets/stickfigure_red_attack.bmp");

    SDL_Texture* walking_white_tex_1 = loadTexture(ren, "assets/stickfigure_white_walking_1.bmp");
    SDL_Texture* walking_white_tex_2 = loadTexture(ren, "assets/stickfigure_white_walking_2.bmp");

    SDL_Texture* parry_icon = loadTexture(ren, "assets/parry_icon.bmp");

    Player p1 , p2;
    Init_player(&p1, tex_white, 100, 700, 1);
    Init_player(&p2, tex_red, 1280-100, 700, 0);

    float accel = 2.0f;
    float dashaccel = 60.0f;
    float maxSpeed = 6.0f;
    float friction = 0.8f;
    int dash_cooldown = 30;

    int p2_walking = 0;
    int red_walk_cycle = 0;

    int p1_walking = 0;
    int white_walk_cycle = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                destroy(ren, win);
                return;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.scancode) {
                    // player 1
                    case SDL_SCANCODE_W:
                        if (p1.posy >= 700) {
                            p1.vely = -15;
                        }
                        break;
                    //player 2
                    case SDL_SCANCODE_UP:
                        if (p2.posy >= 700) {
                            p2.vely = -15;
                        }
                        break;
                }
            }
        }

        const bool* state = SDL_GetKeyboardState(NULL);

        // player 1
        if (p1.stunned > 0) {
            p1.stunned--;
        } else {
            if ((state[SDL_SCANCODE_D] || state[SDL_SCANCODE_A]) && state[SDL_SCANCODE_LSHIFT] && p1.player_dash_cooldown == 0) {
                if (p1.player_iframes == 0) {
                    if (state[SDL_SCANCODE_D]) {
                        p1.facingRight = 1;
                        p1.velx += dashaccel;
                    } else if (state[SDL_SCANCODE_A]) {
                        p1.facingRight = 0;
                        p1.velx -= dashaccel;
                    }
                    p1.player_iframes = 10;
                    p1.player_dash_cooldown = dash_cooldown;
                }
            } else if (state[SDL_SCANCODE_D]) {
                p1.facingRight = 1;
                p1.velx += accel;
                p1_walking++;
                if (p1.velx > maxSpeed) p1.velx = maxSpeed;
            } else if (state[SDL_SCANCODE_A]) {
                p1.facingRight = 0;
                p1.velx -= accel;
                p1_walking++;
                if (p1.velx < -maxSpeed) p1.velx = -maxSpeed;
            } else if(state[SDL_SCANCODE_S]) {
                if (p1.posy < 700) {
                    p1.vely += 5;
                }
            } else {
                p1.velx *= friction;
                if (fabs(p1.velx) < 0.1f) p1.velx = 0;
                p1_walking = 0;
                p1.texture = tex_white;
                p1.frame = 0;             
            }
            if(state[SDL_SCANCODE_F]) {
                if (p1.attack_cooldown == 0) {
                    p1.texture = attacking_white_tex;
                    p1.att_frame = 5;
                    p1.attack_cooldown = 10;
                    if (p1.facingRight) {
                        if(p1.posx + p1.W >= p2.posx - 50 && p1.posx <= p2.posx + p2.W && p1.posy + p1.H >= p2.posy && p1.posy <= p2.posy + p2.H) {
                            if (p2.parrying > 0) {
                                p1.stunned = 30;
                                p1.velx = -p1.velx / 2;
                            } else if (p2.player_iframes == 0) {
                                p2.health -= p1.damage;
                                p2.player_iframes = 20;
                                if (p1.velx > 0) {
                                    p2.velx += 10;
                                } else {
                                    p2.velx += 5;
                                }
                                p2.vely = -5;
                            }
                        } 
                    } else {
                        if(p1.posx <= p2.posx + p2.W + 50 && p1.posx + p1.W >= p2.posx && p1.posy + p1.H >= p2.posy && p1.posy <= p2.posy + p2.H) {
                            if (p2.parrying > 0) {
                                p1.stunned = 30;
                                p1.velx = -p1.velx / 2;
                            } else if (p2.player_iframes == 0) {
                                p2.health -= p1.damage;
                                p2.player_iframes = 20;
                                if (p1.velx > 0) {
                                    p2.velx -= 10;
                                } else {
                                    p2.velx -= 5;
                                }
                                p2.vely = -5;
                            }
                        }
                    }
                }
            } else if (state[SDL_SCANCODE_E]) {
                if (p1.parry_cooldown == 0 && p1.parrying <= 0) {
                    if (p1.parrying == 0) {
                    p1.parrying = 5;
                    p1.parry_cooldown = 20;
                    //parry
                    }
                }
            }
        }
        p1.vely += 2;
        p1.posy += p1.vely;
        p1.posx += p1.velx;

        if (p1.posy >= 700) {
            p1.posy = 700;
            p1.vely = 0;
        }

        if (p1.posx < 0) {
            p1.posx = 0;
            p1.velx = 0;
        }
        if (p1.posx > 1280 - p1.W) {
            p1.posx = 1280 - p1.W;
            p1.velx = 0;
        }
        // player 2
        if (p2.stunned > 0) {
            p2.stunned--;
        } else {
            if ((state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_LEFT]) && state[SDL_SCANCODE_PAGEDOWN] && p2.player_dash_cooldown == 0) {
                if (p2.player_iframes == 0) {
                    if (state[SDL_SCANCODE_RIGHT]) {
                        p2.facingRight = 1;
                        p2.velx += dashaccel;
                    } else if (state[SDL_SCANCODE_LEFT]) {
                        p2.facingRight = 0;
                        p2.velx -= dashaccel;
                    }
                    p2.player_iframes = 10;
                    p2.player_dash_cooldown = dash_cooldown;
                }
            } else if (state[SDL_SCANCODE_RIGHT]) {
                p2.facingRight = 1;
                p2.velx += accel;
                p2_walking++;
                if (p2.velx > maxSpeed) p2.velx = maxSpeed;
            } else if (state[SDL_SCANCODE_LEFT]) {
                p2.facingRight = 0;
                p2.velx -= accel;
                p2_walking++;
                if (p2.velx < -maxSpeed) p2.velx = -maxSpeed;
            } else {
                p2.velx *= friction;
                if (fabs(p2.velx) < 0.1f) p2.velx = 0;
                p2_walking = 0;
                p2.texture = tex_red;
                p2.frame = 0;             
            }
            if(state[SDL_SCANCODE_PAGEUP]) {
                if (p2.attack_cooldown == 0) {
                    p2.texture = attacking_red_tex;
                    p2.att_frame = 5;
                    p2.attack_cooldown = 10;
                    if (p2.facingRight) {
                        if(p2.posx + p2.W >= p1.posx - 50 && p2.posx <= p1.posx + p1.W && p2.posy + p2.H >= p1.posy && p2.posy <= p1.posy + p1.H) {
                            if (p1.parrying > 0) {
                                p2.stunned = 30;
                                p2.velx = -p1.velx / 2;
                            } else if (p1.player_iframes == 0) {
                                p1.health -= p1.damage;
                                p1.player_iframes = 20;
                                if (p2.velx > 0) {
                                    p1.velx -= 10;
                                } else {
                                    p1.velx -= 5;
                                }
                                p1.vely = -5;
                            }
                        }
                    } else {
                        if(p2.posx <= p1.posx + p1.W + 50 && p2.posx + p2.W >= p1.posx && p2.posy + p2.H >= p1.posy && p2.posy <= p1.posy + p1.H) {
                            if (p1.parrying > 0) {
                                p2.stunned = 30;
                                p2.velx = -p1.velx / 2;
                            } else if (p1.player_iframes == 0) {
                                p1.health -= p1.damage;
                                p1.player_iframes = 20;
                                if (p2.velx > 0) {
                                    p1.velx -= 10;
                                } else {
                                    p1.velx -= 5;
                                }
                                p1.vely = -5;
                            }
                        }
                    }
                } 
            } else if (state[SDL_SCANCODE_RSHIFT]) {
                if (p2.parry_cooldown == 0 && p2.parrying <= 0) {
                    if (p2.parrying == 0) {
                        p2.parrying = 5;
                        p2.parry_cooldown = 20;
                        //parry
                    }
                }
            }
        }
        //player 1 walking and attacking:)
        if (p1.att_frame > 0) {
            p1.texture = attacking_white_tex;
            p1.att_frame--;
        } else {
        // walking / idle
        if (p1_walking == 3) {
            if (p1.frame == 0) {
                p1.texture = walking_white_tex_1;
                p1.frame = 2;
            } else if (p1.frame == 1) {
                p1.texture = walking_white_tex_2;
                p1.frame = 2;
            } else {
                p1.texture = tex_white;
                if (white_walk_cycle == 0) {
                    p1.frame = 1;
                    white_walk_cycle = 1;
                } else {
                    p1.frame = 0;
                    white_walk_cycle = 0;
                }
            }
            p1_walking = 0;
            }
        }
        //player 2 walking and attacking :3
        if (p2.att_frame > 0) {
        p2.texture = attacking_red_tex;
        p2.att_frame--;
        } else {
        // walking / idle
        if (p2_walking == 3) {
            if (p2.frame == 0) {
                p2.texture = walking_red_tex_1;
                p2.frame = 2;
            } else if (p2.frame == 1) {
                p2.texture = walking_red_tex_2;
                p2.frame = 2;
            } else {
                p2.texture = tex_red;
                if (red_walk_cycle == 0) {
                    p2.frame = 1;
                    red_walk_cycle = 1;
                } else {
                    p2.frame = 0;
                    red_walk_cycle = 0;
                }
            }
            p2_walking = 0;
            }
        }


        //decrement shtuff
        //player2
        if (p2.player_iframes > 0) {
            p2.player_iframes--;
        }
        if (p2.player_dash_cooldown > 0) {
            p2.player_dash_cooldown--;
        }
        if (p2.attack_cooldown > 0) {
            p2.attack_cooldown--;
        }
        if (p2.parrying > 0) {
            p2.parrying--;
        }
        if (p2.parry_cooldown > 0) {
            p2.parry_cooldown--;
        }
        //player1
        if (p1.player_iframes > 0) {
            p1.player_iframes--;
        }
        if (p1.player_dash_cooldown > 0) {
            p1.player_dash_cooldown--;
        }
        if(p1.attack_cooldown > 0) {
            p1.attack_cooldown--;
        }
        if (p1.parrying > 0) {
            p1.parrying--;
        }
        if(p1.parry_cooldown > 0) {
            p1.parry_cooldown--;
        }
        

        p2.vely += 2;
        p2.posy += p2.vely;
        p2.posx += p2.velx;

        if (p2.posy >= 700) {
            p2.posy = 700;
            p2.vely = 0;
        }
        if (p2.posx < 0) {
            p2.posx = 0;
            p2.velx = 0;
        }
        if (p2.posx > 1280 - p2.W) {
            p2.posx = 1280 - p2.W;
            p2.velx = 0;
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        int p1_texW = p1.W;
        int p1_texH = p1.H;
        SDL_FRect dst = {p1.posx, p1.posy, p1_texW, p1_texH};
        SDL_RenderTextureRotated(ren, p1.texture, NULL, &dst, 0, NULL, p1.facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        int p2_texW = p2.W;
        int p2_texH = p2.H;
        SDL_FRect dst2 = {p2.posx, p2.posy, p2_texW, p2_texH};
        SDL_RenderTextureRotated(ren, p2.texture, NULL, &dst2, 0, NULL, p2.facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

        SDL_SetRenderDrawColor(ren, 255, 255, 255 ,255);
        SDL_SetRenderScale(ren, 2, 2);
        SDL_RenderDebugTextFormat(ren, 50, 25, "player 1 health: %d", p1.health);
        SDL_RenderDebugTextFormat(ren, 50, 50, "dash cooldown: %d", p1.player_dash_cooldown);
        SDL_SetRenderScale(ren, 1, 1);

        SDL_SetRenderDrawColor(ren, 255, 0, 0 ,255);
        SDL_SetRenderScale(ren, 2, 2);
        SDL_RenderDebugTextFormat(ren, 400, 25, "player 2 health: %d", p2.health);
        SDL_RenderDebugTextFormat(ren, 400, 50, "dash cooldown: %d", p2.player_dash_cooldown);
        SDL_SetRenderScale(ren, 1, 1);

        SDL_FRect rect = {0, 700+p1.H, 1280, 960 - 700};
        SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
        SDL_RenderFillRect(ren, &rect);



        if (p1.health <= 0) {
            SDL_SetRenderScale(ren, 4, 4);
            SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
            SDL_RenderDebugTextFormat(ren, 1280/12, 80, "player 2 wins!");
            SDL_SetRenderScale(ren, 1, 1);
            SDL_RenderPresent(ren);
            SDL_Delay(2000);
            running = 0;
        } else if (p2.health <= 0) {
            SDL_SetRenderScale(ren, 4, 4);
            SDL_SetRenderDrawColor(ren, 255, 255, 255 ,255);
            SDL_RenderDebugTextFormat(ren, 1280/12, 80, "player 1 wins!");
            SDL_SetRenderScale(ren, 1, 1);
            SDL_RenderPresent(ren);
            SDL_Delay(2000);
            running = 0;
        }

        if (p1.parrying > 0) {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_FRect parry_dst = {p1.posx + p1.W/4, p1.posy - 40, 40, 40};
            SDL_RenderTexture(ren, parry_icon, NULL, &parry_dst);
        }
        if (p2.parrying > 0) {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
            SDL_FRect parry_dst = {p2.posx + p2.W/4, p2.posy - 40, 40, 40};
            SDL_RenderTexture(ren, parry_icon, NULL, &parry_dst);
        }


        SDL_RenderPresent(ren);

        SDL_Delay(16);
    }
    game_end(ren, win, (p1.health <= 0) ? 2 : 1);
}

void setup(SDL_Renderer *ren, SDL_Window *win) {
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
    SDL_RenderClear(ren);
	
    printf("cleared window \n");

    SDL_Texture* tex_red = loadTexture(ren, "stickfigure_red.bmp");
    SDL_Texture* tex_white = loadTexture(ren, "stickfigure_white.bmp");
    Player p1 , p2;
    Init_player(&p1, tex_white, 100, 700, 1);
    Init_player(&p2, tex_red, 1280-100, 700, 1);
    SDL_SetRenderScale(ren, 2, 2);

    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderDebugTextFormat(ren, 50, 25, "test");
    SDL_SetRenderScale(ren, 1, 1);

    int p1_texW = p1.W;
    int p1_texH = p1.H;

    SDL_FRect dst = {p1.posx, p1.posy, p1_texW, p1_texH};
    SDL_RenderTexture(ren, p1.texture, NULL, &dst);

    int p2_texW = p2.W;
    int p2_texH = p2.H;
    SDL_FRect dst2 = {p2.posx, p2.posy, p2_texW, p2_texH};
    SDL_RenderTexture(ren, p2.texture, NULL, &dst2);
    game_loop(ren, win);
    
    SDL_RenderPresent(ren);
}

int main(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	printf("initialized sdl \n");

    SDL_Window *win = SDL_CreateWindow("SDL3", 1280, 960, 0);
    printf("window created");
    SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);
    printf("renderer created \n");

    if (win == NULL){
    	printf("renderer not created%s\n",SDL_GetError());
	return 0;
    }
    if (ren == NULL){
    	printf("renderer not created%s\n",SDL_GetError());
	return 0;
    }

    setup(ren, win);

    printf("success \n");
    return 0;
}
