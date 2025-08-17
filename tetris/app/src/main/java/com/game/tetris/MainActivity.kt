package com.game.tetris

import android.os.Bundle
import android.view.View
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var gameView: GameView
    private var paused = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        gameView = findViewById(R.id.gameView)

        val btnResume: Button = findViewById(R.id.btnResume)
        btnResume.visibility = View.VISIBLE

        // GameView が生成された後にポーズ状態を反映させる
        gameView.post { gameView.setPaused(true) }

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
