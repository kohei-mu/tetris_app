package com.game.tetris

object NativeBridge {
    init { System.loadLibrary("tetris") }

    external fun initWithSeed(seed: Int)
    external fun setPaused(p: Boolean)
    external fun isPaused(): Boolean

    external fun update(dt: Float)
    external fun command(cmd: Int)
    external fun readBoard(): IntArray
    external fun score(): Int
    external fun level(): Int
    external fun isGameOver(): Boolean

    const val LEFT = 0; const val RIGHT = 1; const val ROTATE_CW = 2
    const val SOFT_DROP = 3; const val HARD_DROP = 4
}
