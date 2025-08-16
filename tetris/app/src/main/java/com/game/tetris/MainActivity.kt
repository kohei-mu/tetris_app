package com.game.tetris

import android.os.Bundle
import android.view.View
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
        val btnResume: Button = findViewById(R.id.btnResume)

        btnPause.setOnClickListener {
            if (!paused) {
                paused = true
                gameView.setPaused(true)
                btnResume.visibility = View.VISIBLE
            }
        }

        btnResume.setOnClickListener {
            paused = false
            gameView.setPaused(false)
            btnResume.visibility = View.GONE
        }

        val btnDrop: Button = findViewById(R.id.btnDrop)
        btnDrop.setOnClickListener {
            if (!paused) NativeBridge.command(NativeBridge.HARD_DROP)
        }
    }
}
