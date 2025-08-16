#include <jni.h>
#include <vector>
#include "game.hpp"

static Game g;


extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_update(JNIEnv* /*env*/, jobject /*thiz*/, jfloat dt) {
    g.step(dt);
}

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_command(JNIEnv* /*env*/, jobject /*thiz*/, jint cmd) {
    g.command((Command)cmd);
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_game_tetris_NativeBridge_readBoard(JNIEnv* env, jobject /*thiz*/) {
    const auto& b = g.board();
    jintArray arr = env->NewIntArray(W*H);
    std::vector<jint> tmp(W*H);
    for (int i = 0; i < W*H; ++i) tmp[i] = static_cast<jint>(b[i]);
    env->SetIntArrayRegion(arr, 0, W*H, tmp.data());
    return arr;
}

extern "C" JNIEXPORT jintArray JNICALL
Java_com_game_tetris_NativeBridge_ghostPositions(JNIEnv* env, jobject /*thiz*/) {
    auto gpos = g.ghostPositions();
    jintArray arr = env->NewIntArray(8);
    jint tmp[8];
    for(int i=0;i<8;++i) tmp[i] = gpos[i];
    env->SetIntArrayRegion(arr, 0, 8, tmp);
    return arr;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_game_tetris_NativeBridge_score(JNIEnv* /*env*/, jobject /*thiz*/) {
    return g.score();
}

extern "C" JNIEXPORT jint JNICALL
Java_com_game_tetris_NativeBridge_level(JNIEnv* /*env*/, jobject /*thiz*/) {
    return g.level();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_game_tetris_NativeBridge_isGameOver(JNIEnv* /*env*/, jobject /*thiz*/) {
    return g.gameOver() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_setPaused(JNIEnv*, jobject, jboolean p) {
    g.setPaused(p == JNI_TRUE);
}
extern "C" JNIEXPORT jboolean JNICALL
Java_com_game_tetris_NativeBridge_isPaused(JNIEnv*, jobject) {
    return g.isPaused() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_init(JNIEnv*, jobject) {
    g.reset(0); // 互換用（固定seed）
}
extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_initWithSeed(JNIEnv*, jobject, jint seed) {
    g.reset((uint32_t)seed); // ランダム初期化
}

