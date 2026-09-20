#pragma once

#include <array>
#include <cstdint>

constexpr int W = 10;
constexpr int VISIBLE_H = 20;
constexpr int H = VISIBLE_H + 2;
constexpr int HIDDEN_ROWS = H - VISIBLE_H;

enum Command : int32_t { LEFT=0, RIGHT=1, ROTATE_CW=2, SOFT_DROP=3, HARD_DROP=4 };
enum PieceType : int { I=0, O, S, Z, J, L, T };
using Shape = std::array<uint8_t, 16>;

struct Piece { int type; int rot; int x; int y; };

struct RenderSnapshot {
    std::array<uint8_t, W * H> board;
    std::array<int, 8> active;
    std::array<int, 8> ghost;
    int score;
    int level;
    bool paused;
    bool gameOver;
};

class Game {
public:
    Game();
    void reset();
    void reset(uint32_t seed);
    void setPaused(bool paused) { paused_ = paused; }
    bool isPaused() const { return paused_; }
    void step(float dt);
    void command(Command command);

    const std::array<uint8_t, W * H>& board() const { return board_; }
    Piece activePiece() const { return cur_; }
    int score() const { return score_; }
    int level() const { return level_; }
    bool gameOver() const { return gameOver_; }
    std::array<int, 8> ghostPositions() const;
    RenderSnapshot snapshot() const;

private:
    friend struct GameTestAccess;
    const Shape& shape(int type, int rot) const;
    void spawn();
    bool collision(const Piece& piece) const;
    void stampLockedPiece();
    void lockPiece();
    int clearLines();
    Piece rotatedCW(const Piece& piece) const;
    Piece moved(const Piece& piece, int dx, int dy) const;
    int fallDistance(const Piece& piece) const;
    int nextPiece();
    std::array<int, 8> positions(const Piece& piece) const;

    std::array<uint8_t, W * H> board_{}; // Locked blocks only.
    Piece cur_{};
    float fallTimer_ = 0.f;
    float fallInterval_ = 1.f;
    int score_ = 0;
    int level_ = 1;
    bool gameOver_ = false;
    bool paused_ = false;
    int bag_[7] = {0,1,2,3,4,5,6};
    int bagIdx_ = 7;
    uint32_t rng_ = 0x12345678;
};
