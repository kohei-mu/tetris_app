package com.game.tetris

import android.os.Bundle
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    private lateinit var gameView: GameView
    private lateinit var pauseButton: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        gameView=findViewById(R.id.gameView); pauseButton=findViewById(R.id.btnPause)
        pauseButton.setOnClickListener { gameView.togglePause(); pauseButton.postDelayed(::refreshButton,50) }
        findViewById<Button>(R.id.btnDrop).setOnClickListener { gameView.hardDrop() }
        findViewById<Button>(R.id.btnRestart).setOnClickListener { gameView.restart(); pauseButton.text=getString(R.string.pause) }
    }

    override fun onPause() { super.onPause(); gameView.pause(); pauseButton.text=getString(R.string.resume) }
    private fun refreshButton() { pauseButton.text=getString(if(gameView.isPaused()) R.string.resume else R.string.pause) }
}
