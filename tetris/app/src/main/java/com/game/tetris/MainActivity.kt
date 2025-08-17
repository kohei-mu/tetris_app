package com.game.tetris

import android.os.Bundle
import android.view.View
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var gameView: GameView
    private lateinit var btnResume: Button
    private var paused = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        gameView = findViewById(R.id.gameView)

        btnResume = findViewById(R.id.btnResume)
        btnResume.visibility = View.GONE

        btnResume.setOnClickListener {
            paused = false
            gameView.setPaused(false)
        }

        val btnDrop: Button = findViewById(R.id.btnDrop)
        btnDrop.setOnClickListener {
            if (!paused) NativeBridge.command(NativeBridge.HARD_DROP)
        }
    }

    override fun onPause() {
        super.onPause()
        paused = true
        gameView.setPaused(true)
        btnResume.visibility = View.VISIBLE
    }
}
