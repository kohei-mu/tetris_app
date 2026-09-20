#include <jni.h>
#include <array>
#include "game.hpp"

namespace { Game game; }

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_update(JNIEnv*, jobject, jfloat dt) { game.step(dt); }

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_command(JNIEnv*, jobject, jint command) {
    game.command(static_cast<Command>(command));
}

// Compact primitive snapshot: board, active coordinates, ghost coordinates,
// score, level, paused, gameOver. It is copied from one coherent native state.
extern "C" JNIEXPORT jintArray JNICALL
Java_com_game_tetris_NativeBridge_renderSnapshot(JNIEnv* env, jobject) {
    constexpr int size = W * H + 8 + 8 + 4;
    std::array<jint, size> values{};
    const RenderSnapshot snapshot = game.snapshot();
    int index = 0;
    for (uint8_t cell : snapshot.board) values[index++] = cell;
    for (int coordinate : snapshot.active) values[index++] = coordinate;
    for (int coordinate : snapshot.ghost) values[index++] = coordinate;
    values[index++] = snapshot.score;
    values[index++] = snapshot.level;
    values[index++] = snapshot.paused ? 1 : 0;
    values[index] = snapshot.gameOver ? 1 : 0;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, values.data());
    return result;
}

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_setPaused(JNIEnv*, jobject, jboolean paused) {
    game.setPaused(paused == JNI_TRUE);
}

extern "C" JNIEXPORT void JNICALL
Java_com_game_tetris_NativeBridge_restart(JNIEnv*, jobject, jint seed) {
    game.reset(static_cast<uint32_t>(seed));
}
