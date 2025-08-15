package com.game.tetris

import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var gameView: GameView
    private var paused = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        gameView = findViewById(R.id.gameView)

        val btnPause: Button = findViewById(R.id.btnPause)
        btnPause.setOnClickListener {
            paused = !paused
            gameView.setPaused(paused)
            btnPause.text = if (paused) "RESUME" else "PAUSE"
        }

        val btnDrop: Button = findViewById(R.id.btnDrop)
        btnDrop.setOnClickListener {
            if (!paused) NativeBridge.command(NativeBridge.HARD_DROP)
        }
    }
}
