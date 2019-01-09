#ifndef PIECE_H
#define PIECE_H

enum piece_type {I, J, L, O, S, T, Z};

struct Piece {
    enum piece_type type;
    char orientation;
    int x, y, w, h;
    int real_x, real_y;
};

#endif
