package com.game.tetris

import android.os.Bundle
import android.view.View
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var gameView: GameView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        gameView = findViewById(R.id.gameView)

        val btnResume: Button = findViewById(R.id.btnResume)
        btnResume.setOnClickListener {
            gameView.setPaused(false)
            btnResume.visibility = View.GONE
        }

        val btnDrop: Button = findViewById(R.id.btnDrop)
        btnDrop.setOnClickListener {
            if (!gameView.isPaused()) NativeBridge.command(NativeBridge.HARD_DROP)
        }
    }

    override fun onPause() {
        super.onPause()
        gameView.setPaused(true)
        findViewById<Button>(R.id.btnResume).visibility = View.VISIBLE
    }
}
