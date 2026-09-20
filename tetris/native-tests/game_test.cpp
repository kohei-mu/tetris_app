#include "game.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

struct GameTestAccess {
    static int next(Game& g) { return g.nextPiece(); }
    static int clear(Game& g) { return g.clearLines(); }
    static void cell(Game& g, int x, int y, int value) { g.board_[y * W + x] = value; }
    static void piece(Game& g, Piece p) { g.cur_ = p; }
    static void spawn(Game& g) { g.spawn(); }
    static void lock(Game& g) { g.lockPiece(); }
};

namespace {
int failures = 0;
#define CHECK(name, expression) do { if (!(expression)) { std::cerr << "FAIL: " << name << "\n"; ++failures; } } while (false)

void fillLines(Game& game, int count, int gap = -1) {
    for (int y = H - count; y < H; ++y)
        for (int x = 0; x < W; ++x) if (x != gap) GameTestAccess::cell(game, x, y, 1);
}

void lineClearTests() {
    for (int count = 1; count <= 4; ++count) {
        Game game; game.reset(10 + count); fillLines(game, count);
        CHECK(std::to_string(count) + " consecutive line clear", GameTestAccess::clear(game) == count);
        CHECK("cleared board is empty", std::all_of(game.board().begin(), game.board().end(), [](auto v){ return v == 0; }));
    }
}
}

int main() {
    Game a; a.reset(42); Game b; b.reset(42);
    CHECK("fixed seed initialization", a.activePiece().type == b.activePiece().type);

    Game bag; bag.reset(7); std::set<int> pieces{bag.activePiece().type};
    for (int i=0; i<6; ++i) pieces.insert(GameTestAccess::next(bag));
    CHECK("seven-bag behavior", pieces.size() == 7);

    Game movement; movement.reset(1); int startX=movement.activePiece().x;
    movement.command(LEFT); CHECK("move left", movement.activePiece().x == startX-1);
    movement.command(RIGHT); CHECK("move right", movement.activePiece().x == startX);
    for(int i=0;i<20;++i) movement.command(LEFT); int left=movement.activePiece().x;
    movement.command(LEFT); CHECK("left wall collision", movement.activePiece().x == left);
    for(int i=0;i<20;++i) movement.command(RIGHT); int right=movement.activePiece().x;
    movement.command(RIGHT); CHECK("right wall collision", movement.activePiece().x == right);

    Game drops; drops.reset(2); int y=drops.activePiece().y; drops.command(SOFT_DROP);
    CHECK("soft drop", drops.activePiece().y == y+1);
    drops.command(HARD_DROP); CHECK("hard drop", std::any_of(drops.board().begin(),drops.board().end(),[](auto v){return v!=0;}));

    Game rotation; rotation.reset(2); int rot=rotation.activePiece().rot; rotation.command(ROTATE_CW);
    CHECK("clockwise rotation", rotation.activePiece().rot == ((rot+1)&3));
    lineClearTests();

    Game scoring; scoring.reset(3); fillLines(scoring,4,5); GameTestAccess::piece(scoring,{I,1,3,H-4}); GameTestAccess::lock(scoring);
    CHECK("scoring", scoring.score() == 800);
    fillLines(scoring,2,5); GameTestAccess::piece(scoring,{I,1,3,H-4}); GameTestAccess::lock(scoring);
    CHECK("level progression", scoring.level() >= 2);

    Game over; over.reset(4); for(int x=0;x<W;++x) GameTestAccess::cell(over,x,0,1); GameTestAccess::spawn(over);
    CHECK("game-over detection", over.gameOver());

    Game ghost; ghost.reset(5); auto before=ghost.snapshot(); auto position=ghost.ghostPositions(); auto after=ghost.snapshot();
    CHECK("ghost calculation", position[1] > ghost.activePiece().y);
    CHECK("ghost calculation has no side effects", before.board == after.board && before.active == after.active && before.score == after.score);

    Game pause; pause.reset(6); auto pausedPiece=pause.activePiece(); pause.setPaused(true); pause.step(10.f);
    CHECK("pause behavior", pause.activePiece().y == pausedPiece.y);

    pause.setPaused(false); pause.command(HARD_DROP); pause.reset(99);
    CHECK("restart/reset behavior", pause.score()==0 && pause.level()==1 && !pause.gameOver() && !pause.isPaused() &&
          std::all_of(pause.board().begin(),pause.board().end(),[](auto v){return v==0;}));

    if (failures) return EXIT_FAILURE;
    std::cout << "All 20 native game-engine scenarios passed\n";
    return EXIT_SUCCESS;
}
