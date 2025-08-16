#pragma once
#include <array>
#include <cstdint>
#include <algorithm>

constexpr int W = 10;
constexpr int VISIBLE_H = 20;
constexpr int H = VISIBLE_H + 2;

enum Command : int32_t { LEFT=0, RIGHT=1, ROTATE_CW=2, SOFT_DROP=3, HARD_DROP=4 };
enum PieceType : int { I=0, O, S, Z, J, L, T };
using Shape = std::array<uint8_t, 16>;

struct Piece {
    int type;
    int rot;
    int x, y;
};

class Game {
public:
    Game();

    void reset();                 // ← これ1個だけ（引数なし）
    void reset(uint32_t seed);    // ← これ1個だけ（シード付き）

    void setPaused(bool p) { paused_ = p; }
    bool isPaused() const { return paused_; }

    void step(float dt);
    void command(Command c);

    const std::array<uint8_t, W*H>& board() const { return board_; }
    int score() const { return score_; }
    int level() const { return level_; }
    bool gameOver() const { return gameOver_; }
    std::array<int,8> ghostPositions() const;

private:
    const Shape& shape(int type, int rot) const;
    void spawn();
    bool collision(const Piece& p) const;
    void stampPiece(bool set);
    void lockPiece();
    int  clearLines();
    Piece rotatedCW(const Piece& p) const;
    Piece moved(const Piece& p, int dx, int dy) const;
    int  fallDistance(const Piece& p) const;
    int  nextPiece();

    std::array<uint8_t, W*H> board_{};
    Piece  cur_{};
    float  fallTimer_ = 0.f;
    float  fallInterval_ = 1.0f;
    int    score_ = 0;
    int    level_ = 1;
    bool   gameOver_ = false;
    bool   paused_ = false;

    int     bag_[7] = {0,1,2,3,4,5,6};
    int     bagIdx_ = 7;
    uint32_t rng_ = 0x12345678;
};
