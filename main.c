#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

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

enum piece_type {I, J, L, O, S, T, Z};

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Texture* textures[7];

char board[BOARD_WIDTH][BOARD_HEIGHT];

struct Piece {
    enum piece_type type;
    char orientation;
    int x, y, w, h;
    int real_x, real_y;
};

SDL_Texture* loadImageTexture(char* path);

int initActivePiece(enum piece_type type, struct Piece* piece);

void drawBoard();
int drawActivePiece(struct Piece* piece);

void updateRealCoords(struct Piece* piece);
int moveLeft(struct Piece* piece);
int moveRight(struct Piece* piece);
int rotate(struct Piece* piece);
int drop(struct Piece* piece);

int main(int argc, char* argv[]) {
    SDL_Event e;

    struct Piece active_piece;
    initActivePiece(I, &active_piece);

    memset(board, -1, sizeof(board));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Couldn't initialize SDL.\n");
        return -1;
    }

    int flags = IMG_INIT_PNG;
    if ((IMG_Init(flags) & flags) != flags) {
        printf("Failed to initialize PNG loading\n");
        return -1;
    }

    if ((window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0)) == NULL) {
        printf ("Couldn't create window.\n");
        return -1;
    }

    if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)) == NULL) {
        printf("Couldn't initialize renderer.\n");
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

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                return 0;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.scancode) {
                    case SDL_SCANCODE_LEFT:
                        moveLeft(&active_piece);
                        break;

                    case SDL_SCANCODE_RIGHT:
                        moveRight(&active_piece);
                        break;

                    case SDL_SCANCODE_UP:
                        rotate(&active_piece);
                        break;

                    case SDL_SCANCODE_DOWN:
                        drop(&active_piece);
                        break;
                }
            }
        }

        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, NULL);

        drawBoard();
        drawActivePiece(&active_piece);

        SDL_RenderPresent(renderer);

        SDL_Delay(50);
    }

    for (int i = 0; i < sizeof(textures)/sizeof(textures); i++) {
        SDL_DestroyTexture(textures[i]);
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

int drop(struct Piece* piece) {
    if (piece->real_y + ((piece->orientation & 1) ? piece->w: piece->h) + SQUARE_SIZE <= BOARD_Y + BOARD_HEIGHT*SQUARE_SIZE) {
        piece->y += SQUARE_SIZE;
        piece->real_y += SQUARE_SIZE;
        return 0;
    } else {
        int board_x = (piece->x - BOARD_X)/SQUARE_SIZE;
        int board_y = (piece->y - BOARD_Y)/SQUARE_SIZE;
    }
}

int rotate(struct Piece* piece) {
    piece->orientation += 1;
    piece->orientation %= 4;

    updateRealCoords(piece);

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

void updateRealCoords(struct Piece* piece) {
    switch (piece->type) {
        case I:
            switch (piece->orientation) {
                case 0:
                    piece->real_x = piece->x;
                    piece->real_y = piece->y;
                    break;
                case 1:
                    piece->real_x = piece->x + SQUARE_SIZE;
                    piece->real_y = piece->y - 2*SQUARE_SIZE;
                    break;
                case 2:
                    piece->real_x = piece->x;
                    piece->real_y = piece->y - SQUARE_SIZE;
                    break;
                case 3:
                    piece->real_x = piece->x + 2*SQUARE_SIZE;
                    piece->real_y = piece->y - 2*SQUARE_SIZE;
                    break;
            }
            break;
        
        case O:
            piece->real_x = piece->x;
            piece->real_y = piece->y;
            break;

        case J:

            switch (piece->orientation) {
                case 0:
                    piece->real_x = piece->x;
                    piece->real_y = piece->y;
                    break;
                case 1:
                    piece->real_x = piece->x;
                    piece->real_y = piece->y;
                    break;
                case 2:
                    piece->real_x = piece->x - SQUARE_SIZE;
                    piece->real_y = piece->y;
                    break;
                case 3:
                    piece->real_x = piece->x;
                    piece->real_y = piece->y - SQUARE_SIZE;
                    break;
            }

    }
}

int initActivePiece(enum piece_type type, struct Piece* piece) {
    piece->type = type;
    piece->orientation = 0;
    piece->x = BOARD_X + (BOARD_WIDTH*SQUARE_SIZE)/2 - ((BOARD_WIDTH*SQUARE_SIZE)/2 % SQUARE_SIZE);
    piece->y = BOARD_Y;
    piece->real_x = piece->x;
    piece->real_y = piece->y;

    switch (type) {
        case I:
            {
            piece->w = 64;
            piece->h = 16;
            break;
            }
        case O:
            {
            piece->w = 32;
            piece->h = 32;
            break;
            }
        default:
            piece->w = 48;
            piece->h = 32;
    }
}

int drawActivePiece(struct Piece* piece) {
    int or = piece->orientation;
    int r_x = (piece->w)/2 - (((piece->w)/2) % SQUARE_SIZE);
    int r_y = (piece->h)/2 - (((piece->h)/2) % SQUARE_SIZE);
    SDL_Point p = {r_x, r_y};
    SDL_Rect rect = {piece->x, piece->y, piece->w, piece->h};
    switch (piece->type) {
        case I:
            SDL_RenderCopyEx(renderer, textures[I], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case J:
            SDL_RenderCopyEx(renderer, textures[J], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case L:
            SDL_RenderCopyEx(renderer, textures[L], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case O:
            SDL_RenderCopyEx(renderer, textures[O], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case S:
            SDL_RenderCopyEx(renderer, textures[S], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case T:
            SDL_RenderCopyEx(renderer, textures[T], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;
        case Z:
            SDL_RenderCopyEx(renderer, textures[Z], NULL, &rect, or*90, &p, SDL_FLIP_NONE);
            break;

    }
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 127);
    SDL_RenderDrawPoint(renderer, piece->real_x, piece->real_y);
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
                        SDL_RenderCopy(renderer, textures[J], &src_rect, &dst_rect);
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
                        SDL_Rect src_rect = {16, 0, 32, 16};
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
        printf("Couldn't load image at %s.\n", path);
        printf("%s\n", IMG_GetError());
        return texture;
    }

    SDL_SetColorKey(loaded_surface, SDL_TRUE, SDL_MapRGB(loaded_surface->format, CK_RED, CK_GREEN, CK_BLUE));

    texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);

    if (texture == NULL) {
        printf("Couldn't convert image at %s to texture.\n", path);
    }

    return texture;
}
