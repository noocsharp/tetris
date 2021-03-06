#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "piece.h"
#include "queue.h"

#define NO_STDIO_REDIRECT
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 800
#define PATH_LENGTH 50
#define CK_RED 0xFF
#define CK_GREEN 0xFF
#define CK_BLUE 0xFF
#define SQUARE_SIZE 16
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24
#define BOARD_X 0
#define BOARD_Y 0

#define QUEUE_WIDTH 6
#define QUEUE_HEIGHT 12 
const int QUEUE_X = BOARD_WIDTH*SQUARE_SIZE;
#define QUEUE_Y 0

#define QUEUE_CAPACITY 3

enum States {MENU, LOST, GAME, ABOUT} state = GAME;

char i[4][1] = {{I}, {I}, {I}, {I}};
char j[3][2] = {{J, -1}, {J, -1}, {J, J}};
char l[3][2] = {{L, L}, {L, -1}, {L, -1}};
char o[2][2] = {{O, O}, {O, O}};
char s[3][2] = {{-1, S}, {S, S}, {S, -1}};
char t[3][2] = {{T, -1}, {T, T}, {T, -1}};
char z[3][2] = {{Z, -1}, {Z, Z}, {-1, Z}};

char shape[4][4];

SDL_Window* window;
SDL_Renderer* renderer;

TTF_Font *font;

SDL_Texture* textures[7];
SDL_Texture* text_cache[5];

char board[BOARD_WIDTH][BOARD_HEIGHT];

struct Queue queue;
struct Piece queue_array[QUEUE_CAPACITY];

SDL_Texture* loadImageTexture(char* path);
SDL_Texture* loadTextTexture(char* text, SDL_Color fg, SDL_Color bg);

int initActivePiece(enum piece_type type, struct Piece* piece);
int initQueuePiece(struct Piece* piece);

void drawBoard();
void drawQueue();
int drawActivePiece(struct Piece* piece);

void getRealCoords(struct Piece* piece, int* x, int* y);
int moveLeft(struct Piece* piece);
int moveRight(struct Piece* piece);
int rotate(struct Piece* piece);
int drop(struct Piece* piece);

int rotateShape(struct Piece* piece, char shp[4][4]);
int getRotatedMap(struct Piece* piece, int* w, int* h);

int score = 0;
int level = 0;

void printShape();

void drawLostDialog();
void clearRows();

int main(int argc, char* argv[]) {
    SDL_Event e;

    srand(time(0));

    initQueue(&queue, queue_array, QUEUE_CAPACITY);

    for (int n = 0; n < QUEUE_CAPACITY; n++) {
        struct Piece temp = {rand()%7, 0, 0, 0, 0, 0, 0};
        initQueuePiece(&temp);
        enqueue(&queue, temp);
    }

    SDL_Log("size: %d\n", queue.size);

    struct Piece active_piece;
    initActivePiece(L, &active_piece);

    memset(board, -1, sizeof(board));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Couldn't initialize SDL.\n");
        return -1;
    }

    int flags = IMG_INIT_PNG;
    if ((IMG_Init(flags) & flags) != flags) {
        SDL_Log("Failed to initialize PNG loading\n");
        return -1;
    }

    if (TTF_Init() < 0) {
        SDL_Log("Failed to initialize SDL_ttf.\n");
        return -1;
    }

    if ((window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        SDL_Log ("Couldn't create window.\n");
        return -1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
        SDL_Log("Couldn't initialize renderer.\n");
        return -1;
    }

    font = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 24);
    if (font == NULL) {
        SDL_Log("Failed to load font: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 48;
    rect.h = 32;

    textures[I] = loadImageTexture("img/i.png");
    textures[J] = loadImageTexture("img/j.png");
    textures[L] = loadImageTexture("img/l.png");
    textures[O] = loadImageTexture("img/o.png");
    textures[S] = loadImageTexture("img/s.png");
    textures[T] = loadImageTexture("img/t.png");
    textures[Z] = loadImageTexture("img/z.png");

    SDL_Color grey = {0x7f, 0x7f, 0x7f};
    SDL_Color white = {0, 0, 0};
    text_cache[0] = loadTextTexture("GAME OVER", white, grey);

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_LEFT:
                        switch (state) {
                            case GAME:
                                moveLeft(&active_piece);
                                break;
                        }
                        break;

                    case SDL_SCANCODE_RIGHT:
                        switch (state) {
                            case GAME:
                                moveRight(&active_piece);
                                break;
                        }
                        break;

                    case SDL_SCANCODE_UP:
                        switch (state) {
                            case GAME:
                                rotate(&active_piece);
                                break;
                        }
                        break;

                    case SDL_SCANCODE_DOWN:
                        switch (state) {
                            case GAME:
                                drop(&active_piece);
                                break;
                        }
                        break;

                    case SDL_SCANCODE_SPACE:
                        switch (state) {
                            case GAME:
                                while (drop(&active_piece) == 0);
                                break;
                        }
                        break;
                }
            }
        }

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, NULL);

        switch (state) {
            // leave break out, should still draw game in background
            case LOST:
                drawQueue();
                drawBoard();
                //drawActivePiece(&active_piece);
                drawLostDialog();
                break;
            case GAME:
                drawQueue();
                drawBoard();
                drawActivePiece(&active_piece);
                break;
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }

    for (int i = 0; i < sizeof(textures)/sizeof(textures); i++) {
        SDL_DestroyTexture(textures[i]);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

int drop(struct Piece* piece) {
    int board_x = (piece->real_x - BOARD_X)/SQUARE_SIZE;
    int board_y = (piece->real_y - BOARD_Y)/SQUARE_SIZE;

    int w = (piece->orientation & 1) ? piece->h/SQUARE_SIZE : piece->w/SQUARE_SIZE;
    int h = (piece->orientation & 1) ? piece->w/SQUARE_SIZE : piece->h/SQUARE_SIZE;
    if (piece->real_y + ((piece->orientation & 1) ? piece->w: piece->h) + SQUARE_SIZE <= BOARD_Y + BOARD_HEIGHT*SQUARE_SIZE) {

        for (int n = 0; n < w; n++) {
            for (int m = h-1; m >= 0; m--) {
                if (shape[n][m] >= 0) {
                    if (board[board_x+n][board_y+m+1] >= 0) {
                        goto after;
                    }
                }
            }
        }
        piece->y += SQUARE_SIZE;
        piece->real_y += SQUARE_SIZE;
        return 0;
    }
    after:
    
    if (piece->real_y <= 0) {
        state = LOST;
    }


    for (int n = 0; n < w; n++) {
        for (int m = 0; m < h; m++) {
            if (board[board_x+n][board_y+m] < shape[n][m]) {
                board[board_x+n][board_y+m] = shape[n][m];
            }
        }
    }
    clearRows();

    struct Piece* p = dequeue(&queue);

    if (p == NULL) {
        SDL_Log("dequeue pointer is null");
        SDL_Log("size: %d\n", queue.size);
        return -1;
    }
    memcpy(piece, p, sizeof(struct Piece));
    initActivePiece(piece->type, piece);
    struct Piece np = {rand()%7, 0, 0, 0, 0, 0, 0};
    initQueuePiece(&np);

    enqueue(&queue, np);

    return -1;
}

void drawLostDialog() {

    //draw box
    int d_w = 300;
    int d_h = 200;
    SDL_Rect dialog_rect = {SCREEN_WIDTH/2 - d_w/2, SCREEN_HEIGHT/2 - d_h/2, d_w, d_h};
    SDL_SetRenderDrawColor(renderer, 0x7F, 0x7F, 0x7F, 0xFF);
    SDL_RenderFillRect(renderer, &dialog_rect);

    // draw text
    int f_w, f_h;
    TTF_SizeText(font, "GAME OVER", &f_w, &f_h);
    SDL_Rect text_rect = {SCREEN_WIDTH/2 - f_w/2, SCREEN_HEIGHT/2 - f_h/2, f_w, f_h};
    SDL_RenderCopy(renderer, text_cache[0], NULL, &text_rect);

}

void printBoard() {
    for (int m = 0; m < BOARD_HEIGHT; m++) {
        SDL_Log("%02d %02d %02d %02d %02d %02d %02d %02d %02d %02d", board[0][m], board[1][m], board[2][m], board[3][m], board[4][m], board[5][m], board[6][m], board[7][m], board[8][m], board[9][m]);
    }
}

void clearRows() {
    int rows_cleared = 0;
    for (int m = BOARD_HEIGHT-1; m >= 0; m--) {
        char flag = 1;
        for (int n = 0; n < BOARD_WIDTH; n++) {
            if (board[n][m] == -1) {
                flag = 0;
                break;
            }
        }
        if (flag) {
            SDL_Log("Clearing rows\n");
            for (int p = m; p > 0; p--) {
                for (int n = 0; n < BOARD_WIDTH; n++) {
                    board[n][p] = board[n][p-1];
                }
            }
        }
    }
}

int rotate(struct Piece* piece) {
    int x, y;

    struct Piece temp_piece;
    memcpy(&temp_piece, piece, sizeof(struct Piece));
    temp_piece.orientation = (temp_piece.orientation+1)%4;

    getRealCoords(&temp_piece, &x, &y);
    int t_w = ((temp_piece.orientation & 1) ? temp_piece.w/SQUARE_SIZE : temp_piece.h/SQUARE_SIZE);
    int t_h = ((temp_piece.orientation & 1) ? temp_piece.h/SQUARE_SIZE : temp_piece.w/SQUARE_SIZE);
    SDL_Log("ROTATE");
    /*
    for (int n = 0; n < 4; n++) {
        SDL_Log("%02d %02d %02d %02d", shape[0][n], shape[1][n],shape[2][n],shape[3][n]);
    }
    */
    char l_shape[4][4];
    memcpy(l_shape, shape, 4*4*sizeof(char));
    rotateShape(&temp_piece, l_shape);

    for (int n = 0; n < 4; n++) {
        SDL_Log("%02d %02d %02d %02d", l_shape[0][n], l_shape[1][n], l_shape[2][n], l_shape[3][n]);
    }

    char flag;
    for (int n = 0; n < t_w; n++) {
        for (int m = 0; m < t_h; m++) {
            if (l_shape[m][n] != -1 && board[x/SQUARE_SIZE+m][y/SQUARE_SIZE+n] != -1) {
                return -1;
            }
        }
    }

    piece->orientation = (piece->orientation+1)%4;

    piece->real_x = x;
    piece->real_y = y;

    while (piece->real_y + ((piece->orientation & 1) ? piece->w : piece->h) > BOARD_Y + BOARD_HEIGHT*SQUARE_SIZE) {
        piece->y -= SQUARE_SIZE;
        piece->real_y -= SQUARE_SIZE;
    }

    while (piece->real_x + ((piece->orientation & 1) ? piece->h : piece->w) > BOARD_X + BOARD_WIDTH*SQUARE_SIZE) {
        piece->x -= SQUARE_SIZE;
        piece->real_x -= SQUARE_SIZE;
    }

    while (piece->real_x < 0) {
        piece->x += SQUARE_SIZE;
        piece->real_x += SQUARE_SIZE;
    }

    rotateShape(piece, shape);
}

int moveLeft(struct Piece* piece) {
    if (piece->real_x - SQUARE_SIZE >= 0) {
        piece->x -= SQUARE_SIZE;
        piece->real_x -= SQUARE_SIZE;
    }
}

int moveRight(struct Piece* piece) {
    if (piece->real_x + ((piece->orientation & 1) ? piece->h: piece->w) + SQUARE_SIZE <= BOARD_X + BOARD_WIDTH*SQUARE_SIZE) {
        piece->x += SQUARE_SIZE;
        piece->real_x += SQUARE_SIZE;
    }
}

void getRealCoords(struct Piece* piece, int* x, int* y) {
    switch (piece->type) {
        case I:
            switch (piece->orientation) {
                case 0:
                    *x = piece->x;
                    *y = piece->y;
                    break;
                case 1:
                    *x = piece->x + SQUARE_SIZE;
                    *y = piece->y - 2*SQUARE_SIZE;
                    break;
                case 2:
                    *x = piece->x;
                    *y = piece->y - SQUARE_SIZE;
                    break;
                case 3:
                    *x = piece->x + 2*SQUARE_SIZE;
                    *y = piece->y - 2*SQUARE_SIZE;
                    break;
            }
            break;
        
        case O:
            *x = piece->x;
            *y = piece->y;
            break;

        case J:
        case L:
        case S:
        case T:
        case Z:

            switch (piece->orientation) {
                case 0:
                    *x = piece->x;
                    *y = piece->y;
                    break;
                case 1:
                    *x = piece->x;
                    *y = piece->y;
                    break;
                case 2:
                    *x = piece->x - SQUARE_SIZE;
                    *y = piece->y;
                    break;
                case 3:
                    *x = piece->x;
                    *y = piece->y - SQUARE_SIZE;
                    break;
            }

    }
}

int initActivePiece(enum piece_type type, struct Piece* piece) {
    memset(shape, -1, 16);
    piece->type = type;
    piece->orientation = 0;
    piece->x = BOARD_X + (BOARD_WIDTH*SQUARE_SIZE)/2 - ((BOARD_WIDTH*SQUARE_SIZE)/2 % SQUARE_SIZE);
    piece->y = BOARD_Y;
    piece->real_x = piece->x;
    piece->real_y = piece->y;

    switch (type) {
        case I:
            {
            piece->w = 4*SQUARE_SIZE;
            piece->h = SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = i[n][m];
                }
            }

            break;
            }

        case J:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = j[n][m];
                }
            }

            break;

        case L:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = l[n][m];
                }
            }

            break;
        case O:

            piece->w = 2*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = o[n][m];
                }
            }

            break;

        case S:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = s[n][m];
                }
            }
            break;

        case T:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = t[n][m];
                }
            }
            break;

        case Z:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;

            for (int n = 0; n < piece->w/SQUARE_SIZE; n++) {
                for (int m = 0; m < piece->h/SQUARE_SIZE; m++) {
                    shape[n][m] = z[n][m];
                }
            }
            break;
        default:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;
    }

    for (int n = 0; n < 4; n++) {
        SDL_Log("%02d %02d %02d %02d", shape[0][n], shape[1][n],shape[2][n],shape[3][n]);
    }

}

int initQueuePiece(struct Piece* piece) {

    switch (piece->type) {
        case I:
            {
            piece->w = 4*SQUARE_SIZE;
            piece->h = SQUARE_SIZE;
            break;
            }
        case O:
            {
            piece->w = 2*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;
            break;
            }
        default:
            piece->w = 3*SQUARE_SIZE;
            piece->h = 2*SQUARE_SIZE;
    }
}

int drawActivePiece(struct Piece* piece) {
    int or = piece->orientation;
    int r_x = (piece->w)/2 - (((piece->w)/2) % SQUARE_SIZE);
    int r_y = (piece->h)/2 - (((piece->h)/2) % SQUARE_SIZE);
    SDL_Point p = {r_x, r_y};
    SDL_Rect rect = {piece->x, piece->y, piece->w, piece->h};
    SDL_RenderCopyEx(renderer, textures[piece->type], NULL, &rect, or*90, &p, SDL_FLIP_NONE);

    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 127);
    SDL_RenderDrawPoint(renderer, piece->real_x, piece->real_y);
}

void drawQueue() {
    for (int n = 0; n < QUEUE_CAPACITY; n++) {
        struct Piece* piece = &queue_array[(queue.front + n)%queue.capacity];
        SDL_Rect rect = {QUEUE_X + SQUARE_SIZE, QUEUE_Y + SQUARE_SIZE*(4*n+1), piece->w, piece->h};
        SDL_RenderCopy(renderer, textures[piece->type], NULL, &rect);
    }
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 127);
    SDL_Rect rect = {QUEUE_X, QUEUE_Y, QUEUE_WIDTH*SQUARE_SIZE, QUEUE_HEIGHT*SQUARE_SIZE};
    SDL_RenderDrawRect(renderer, &rect);
}

void drawBoard() {
    for (int m = 0; m < BOARD_WIDTH; m++) {
        for (int n = 0; n < BOARD_HEIGHT; n++) {
            switch(board[m][n]) {
                case I:
                    {
                        SDL_Rect src_rect = {0, 0, 16, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[I], &src_rect, &dst_rect);
                        break;
                    }

                case J:
                    {
                        SDL_Rect src_rect = {0, 0, 16, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[J], &src_rect, &dst_rect);
                        break;
                    }

                case L:
                    {
                        SDL_Rect src_rect = {0, 0, 16, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[L], &src_rect, &dst_rect);
                        break;
                    }

                case O:
                    {
                        SDL_Rect src_rect = {0, 0, 16, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[O], &src_rect, &dst_rect);
                        break;
                    }

                case S:
                    {
                        SDL_Rect src_rect = {16, 0, 32, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[S], &src_rect, &dst_rect);
                        break;
                    }

                case T:
                    {
                        SDL_Rect src_rect = {16, 0, 32, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[T], &src_rect, &dst_rect);
                        break;
                    }

                case Z:
                    {
                        SDL_Rect src_rect = {0, 0, 16, 16};
                        SDL_Rect dst_rect = {m*SQUARE_SIZE, n*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE};
                        SDL_RenderCopy(renderer, textures[Z], &src_rect, &dst_rect);
                        break;
                    }
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 127);
    SDL_RenderDrawLine(renderer, 0, 0, 0, BOARD_HEIGHT*SQUARE_SIZE);
    SDL_RenderDrawLine(renderer, 0, 0, BOARD_WIDTH*SQUARE_SIZE, 0);
    SDL_RenderDrawLine(renderer, BOARD_WIDTH*SQUARE_SIZE, 0, BOARD_WIDTH*SQUARE_SIZE, BOARD_HEIGHT*SQUARE_SIZE);
    SDL_RenderDrawLine(renderer, 0, BOARD_HEIGHT*SQUARE_SIZE, BOARD_WIDTH*SQUARE_SIZE, BOARD_HEIGHT*SQUARE_SIZE);
}

SDL_Texture* loadImageTexture(char* path) {
    SDL_Texture* texture = NULL;

    SDL_Surface* loaded_surface = IMG_Load(path);

    if (loaded_surface == NULL) {
        SDL_Log("Couldn't load image at %s.\n", path);
        SDL_Log("%s\n", IMG_GetError());
        return texture;
    }

    SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, CK_RED, CK_GREEN, CK_BLUE));

    texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);

    if (texture == NULL) {
        SDL_Log("Couldn't convert image at %s to texture.\n", path);
    }

    SDL_FreeSurface(loaded_surface);

    return texture;
}

SDL_Texture* loadTextTexture(char* text, SDL_Color fg, SDL_Color bg) {
    SDL_Surface* text_surface;
    SDL_Texture* text_texture;

    if (!(text_surface = TTF_RenderText_Shaded(font, text, fg, bg))) {
        SDL_Log("Failed to create surface: %s \n", SDL_GetError());
        return NULL;
    }

    if (!(text_texture = SDL_CreateTextureFromSurface(renderer, text_surface))) {
        SDL_Log("Failed to create texture from surface: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(text_surface);
    return text_texture;
}

int rotateShape(struct Piece* piece, char shp[4][4]) {
    int t_w = (piece->orientation & 1) ? piece->h/SQUARE_SIZE : piece->w/SQUARE_SIZE;
    int t_h = (piece->orientation & 1) ? piece->w/SQUARE_SIZE : piece->h/SQUARE_SIZE;

    for (int n = 0; n < 4; n++) {
        for (int m = 0; m < n; m++) {
            char temp = shp[n][m];
            shp[n][m] = shp[m][n];
            shp[m][n] = temp;
        }
    }

    for (int n = 0; n < (t_w/2); n++) {
        for (int m = 0; m < t_h; m++) {
            char temp = shp[n][m];
            shp[n][m] = shp[t_w-n-1][m];
            shp[t_w-n-1][m] = temp;
        }
    }
}
