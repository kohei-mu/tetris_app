package com.game.tetris

object NativeBridge {
    init { System.loadLibrary("tetris") }

    external fun restart(seed: Int)
    external fun setPaused(paused: Boolean)
    external fun update(dt: Float)
    external fun command(command: Int)
    external fun renderSnapshot(): IntArray

    const val LEFT = 0
    const val RIGHT = 1
    const val ROTATE_CW = 2
    const val SOFT_DROP = 3
    const val HARD_DROP = 4
}
