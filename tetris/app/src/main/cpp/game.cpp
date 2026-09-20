#include "game.hpp"
#include <algorithm>
#include <chrono>

// 7種 * 4回転 の 4x4 形状（クラシック寄り）
// 4x4内で (x,y) -> idx = y*4 + x
static const Shape SHAPES[7][4] = {
        // I
        {
                // rot0
                {0,0,0,0,
                        1,1,1,1,
                        0,0,0,0,
                        0,0,0,0},
                // rot1
                {0,0,1,0,
                        0,0,1,0,
                        0,0,1,0,
                        0,0,1,0},
                // rot2
                {0,0,0,0,
                        0,0,0,0,
                        1,1,1,1,
                        0,0,0,0},
                // rot3
                {0,1,0,0,
                        0,1,0,0,
                        0,1,0,0,
                        0,1,0,0}
        },
        // O
        {
                {0,1,1,0,
                        0,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,1,0,
                        0,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,1,0,
                        0,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,1,0,
                        0,1,1,0,
                        0,0,0,0,
                        0,0,0,0}
        },
        // S
        {
                {0,1,1,0,
                        1,1,0,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,0,0,
                        0,1,1,0,
                        0,0,1,0,
                        0,0,0,0},
                {0,0,0,0,
                        0,1,1,0,
                        1,1,0,0,
                        0,0,0,0},
                {1,0,0,0,
                        1,1,0,0,
                        0,1,0,0,
                        0,0,0,0}
        },
        // Z
        {
                {1,1,0,0,
                        0,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,0,1,0,
                        0,1,1,0,
                        0,1,0,0,
                        0,0,0,0},
                {0,0,0,0,
                        1,1,0,0,
                        0,1,1,0,
                        0,0,0,0},
                {0,1,0,0,
                        1,1,0,0,
                        1,0,0,0,
                        0,0,0,0}
        },
        // J
        {
                {1,0,0,0,
                        1,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,1,0,
                        0,1,0,0,
                        0,1,0,0,
                        0,0,0,0},
                {0,0,0,0,
                        1,1,1,0,
                        0,0,1,0,
                        0,0,0,0},
                {0,1,0,0,
                        0,1,0,0,
                        1,1,0,0,
                        0,0,0,0}
        },
        // L
        {
                {0,0,1,0,
                        1,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,0,0,
                        0,1,0,0,
                        0,1,1,0,
                        0,0,0,0},
                {0,0,0,0,
                        1,1,1,0,
                        1,0,0,0,
                        0,0,0,0},
                {1,1,0,0,
                        0,1,0,0,
                        0,1,0,0,
                        0,0,0,0}
        },
        // T
        {
                {0,1,0,0,
                        1,1,1,0,
                        0,0,0,0,
                        0,0,0,0},
                {0,1,0,0,
                        0,1,1,0,
                        0,1,0,0,
                        0,0,0,0},
                {0,0,0,0,
                        1,1,1,0,
                        0,1,0,0,
                        0,0,0,0},
                {0,1,0,0,
                        1,1,0,0,
                        0,1,0,0,
                        0,0,0,0}
        }
};

static inline uint32_t lcg(uint32_t& s){ s = s*1664525u + 1013904223u; return s; }

const Shape& Game::shape(int type, int rot) const { return SHAPES[type][rot & 3]; }

Game::Game(){ reset(); }

void Game::reset(){
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    reset(static_cast<uint32_t>(now) ^ static_cast<uint32_t>(now >> 32));
}

void Game::reset(uint32_t seed){
    std::fill(board_.begin(), board_.end(), 0);
    score_=0; level_=1; fallInterval_=1.0f; fallTimer_=0.f; gameOver_=false; paused_=false;
    for(int i=0;i<7;++i) bag_[i]=i; bagIdx_=7;
    rng_ = seed;
    spawn();
}

// 7バッグ（簡易シャッフル）
int Game::nextPiece(){
    if(bagIdx_ >= 7){
        // Fisher-Yates
        for(int i=6;i>0;--i){
            int j = lcg(rng_) % (i+1);
            std::swap(bag_[i], bag_[j]);
        }
        bagIdx_ = 0;
    }
    return bag_[bagIdx_++];
}

void Game::spawn(){
    cur_.type = nextPiece();
    cur_.rot = 0;
    cur_.x = (W/2) - 2;  // 中央寄せ
    cur_.y = 0;          // 上の余白から開始
    // もし出現直後に衝突していたらゲームオーバー
    if(collision(cur_)){ gameOver_ = true; }
}

bool Game::collision(const Piece& p) const{
    const auto& s = shape(p.type, p.rot);
    for(int dy=0; dy<4; ++dy){
        for(int dx=0; dx<4; ++dx){
            if(!s[dy*4+dx]) continue;
            int x = p.x + dx;
            int y = p.y + dy;
            if(x < 0 || x >= W || y < 0 || y >= H) return true;
            if(board_[y*W + x] != 0) return true;
        }
    }
    return false;
}

Piece Game::moved(const Piece& p, int dx, int dy) const{
    Piece q = p; q.x += dx; q.y += dy; return q;
}

Piece Game::rotatedCW(const Piece& p) const{
    Piece q = p; q.rot = (q.rot + 1) & 3; return q;
}

void Game::stampLockedPiece(){
    const auto& s = shape(cur_.type, cur_.rot);
    for(int dy=0; dy<4; ++dy){
        for(int dx=0; dx<4; ++dx){
            if(!s[dy*4+dx]) continue;
            int x = cur_.x + dx;
            int y = cur_.y + dy;
            if(x<0||x>=W||y<0||y>=H) continue;
            board_[y*W + x] = static_cast<uint8_t>(cur_.type + 1);
        }
    }
}

int Game::fallDistance(const Piece& p) const{
    // 下方向へ何マス落ちられるか
    int dist = 0;
    Piece q = p;
    while(true){
        Piece r = moved(q, 0, 1);
        if(collision(r)) break;
        q = r; ++dist;
    }
    return dist;
}

std::array<int,8> Game::ghostPositions() const {
    Piece g = cur_;
    g.y += fallDistance(g);
    return positions(g);
}

std::array<int,8> Game::positions(const Piece& piece) const {
    std::array<int,8> out{};
    const auto& s = shape(piece.type, piece.rot);
    int idx = 0;
    for(int dy=0; dy<4; ++dy){
        for(int dx=0; dx<4; ++dx){
            if(!s[dy*4+dx]) continue;
            out[idx++] = piece.x + dx;
            out[idx++] = piece.y + dy;
        }
    }
    return out;
}

RenderSnapshot Game::snapshot() const {
    return {board_, positions(cur_), ghostPositions(), score_, level_, paused_, gameOver_};
}

void Game::lockPiece(){
    stampLockedPiece();
    // 固定後の行消去
    int cleared = clearLines();
    // スコア（クラシック風：line数で加点）
    if(cleared == 1) score_ += 100;
    else if(cleared == 2) score_ += 300;
    else if(cleared == 3) score_ += 500;
    else if(cleared == 4) score_ += 800;
    // レベル進行（お手軽：一定スコア毎に上がる）
    if(score_ / 1000 + 1 > level_){
        level_ = score_ / 1000 + 1;
        // 落下間隔を短縮（下限あり）
        fallInterval_ = std::max(0.1f, 1.0f - (level_-1)*0.08f);
    }
    spawn();
}

int Game::clearLines(){
    int cleared = 0;
    int y = H - 1;
    while (y >= 0) {
        bool full = true;
        for(int x=0; x<W; ++x){
            if(board_[y*W+x] == 0){ full = false; break; }
        }
        if(full){
            // 上から詰める
            for(int yy=y; yy>0; --yy){
                for(int x=0; x<W; ++x){
                    board_[yy*W+x] = board_[(yy-1)*W+x];
                }
            }
            for(int x=0; x<W; ++x) board_[0*W+x] = 0;
            ++cleared;
        } else {
            --y;
        }
    }
    return cleared;
}

void Game::step(float dt){
    if(gameOver_ || paused_) return;
    // 自然落下
    fallTimer_ += dt;
    if(fallTimer_ >= fallInterval_){
        fallTimer_ = 0.f;
        Piece down = moved(cur_, 0, 1);
        if(!collision(down)){
            cur_ = down;
        }else{
            lockPiece();
        }
    }
}

void Game::command(Command c){
    if(gameOver_ || paused_) return;

    if(c == LEFT){
        Piece q = moved(cur_, -1, 0);
        if(!collision(q)) cur_ = q;
    }else if(c == RIGHT){
        Piece q = moved(cur_, +1, 0);
        if(!collision(q)) cur_ = q;
    }else if(c == ROTATE_CW){
        // 右回転（壁キックは簡易：左右1マス試す）
        Piece r = rotatedCW(cur_);
        if(!collision(r)){
            cur_ = r;
        }else{
            Piece rL = r; rL.x -= 1;
            Piece rR = r; rR.x += 1;
            if(!collision(rL)) cur_ = rL;
            else if(!collision(rR)) cur_ = rR;
            // それでもダメなら回転不成立
        }
    }else if(c == SOFT_DROP){
        // 1マスだけ下げる（落下タイマも少し進める）
        Piece q = moved(cur_, 0, +1);
        if(!collision(q)) cur_ = q;
        fallTimer_ = 0.f; // 連打で早く落ちる感
    }else if(c == HARD_DROP){
        // 一気に落として固定
        int d = fallDistance(cur_);
        cur_.y += d;
        lockPiece();
        return; // lockPiece内でspawnされるためここで終了
    }
}
