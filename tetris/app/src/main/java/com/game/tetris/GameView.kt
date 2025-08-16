package com.game.tetris

import android.content.Context
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import kotlin.math.min

/**
 * ゲーム描画と入力を担当する SurfaceView。
 * - タップ：右回転
 * - ドラッグ（横）：左右移動（しきい値ごとに1マス）
 * - ドラッグ（下）：ソフトドロップ
 * - ポーズ中は入力無効＆落下停止（Native/C++側も停止）
 * - 画面左上に Score/Level、下に経過Timeを表示
 */
class GameView(ctx: Context, attrs: AttributeSet? = null) :
    SurfaceView(ctx, attrs), SurfaceHolder.Callback {

    // 盤サイズ（描画・入力のしきい値計算用）
    private val cols = 10
    private val rows = 22 // 上2段表示込み

    // ループ制御
    private var running = false
    private var lastNs = 0L

    // HUD/ブロック描画用
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val pieceColors = intArrayOf(
        Color.parseColor("#00BCD4"), // I
        Color.parseColor("#FFEB3B"), // O
        Color.parseColor("#4CAF50"), // S
        Color.parseColor("#F44336"), // Z
        Color.parseColor("#3F51B5"), // J
        Color.parseColor("#FF9800"), // L
        Color.parseColor("#9C27B0")  // T
    )

    // ポーズ＆タイマー
    private var paused = false
    private var elapsedSec = 0f

    // タッチ入力の累積
    private var downX = 0f; private var downY = 0f
    private var lastX = 0f; private var lastY = 0f
    private var accX = 0f;  private var accY = 0f
    private var moved = false

    init {
        holder.addCallback(this)
        isFocusable = true
    }

    // --- Surface lifecycle ----------------------------------------------------

    override fun surfaceCreated(h: SurfaceHolder) {
        // ★初手からランダムになるよう seed 付き初期化
        NativeBridge.initWithSeed((System.currentTimeMillis() and 0xFFFF_FFFFL).toInt())
        elapsedSec = 0f
        paused = false

        running = true
        lastNs = System.nanoTime()
        Thread { loop() }.start()
    }

    override fun surfaceDestroyed(h: SurfaceHolder) { running = false }
    override fun surfaceChanged(h: SurfaceHolder, f: Int, w: Int, hgt: Int) = Unit

    // --- 入力（タップ回転／ドラッグ移動・下落） -----------------------------

    override fun onTouchEvent(e: MotionEvent): Boolean {
        // ポーズ中・ゲームオーバー中は入力を食いつぶす
        if (NativeBridge.isGameOver() || paused) return true

        val cell = min(width / cols, height / rows).toFloat()
        when (e.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                downX = e.x; downY = e.y
                lastX = e.x; lastY = e.y
                accX = 0f; accY = 0f; moved = false
            }
            MotionEvent.ACTION_MOVE -> {
                val dx = e.x - lastX
                val dy = e.y - lastY
                lastX = e.x; lastY = e.y

                accX += dx
                accY += dy

                // しきい値（1/2セル分）ごとにコマンド送信
                val step = cell * 0.5f
                while (accX > step)  { NativeBridge.command(NativeBridge.RIGHT); accX -= step; moved = true }
                while (accX < -step) { NativeBridge.command(NativeBridge.LEFT);  accX += step; moved = true }
                while (accY > step)  { NativeBridge.command(NativeBridge.SOFT_DROP); accY -= step; moved = true }
            }
            MotionEvent.ACTION_UP -> {
                // ほぼ動いていない＝タップ：右回転
                val dx = e.x - downX
                val dy = e.y - downY
                val tapThresh = cell * 0.25f
                if (!moved && (dx*dx + dy*dy) < tapThresh*tapThresh) {
                    NativeBridge.command(NativeBridge.ROTATE_CW)
                }
            }
        }
        return true
    }

    // --- メインループ --------------------------------------------------------

    private fun loop() {
        while (running) {
            val now = System.nanoTime()
            val dt = (now - lastNs) / 1_000_000_000f
            lastNs = now

            if (!paused && !NativeBridge.isGameOver()) {
                elapsedSec += dt
            }

            // ネイティブ側の進行。フレーム落ちを防ぐため dt は上限をかける
            NativeBridge.update(dt.coerceAtMost(0.05f))
            drawFrame()

            try { Thread.sleep(16) } catch (_: InterruptedException) {}
        }
    }

    // --- 描画 ----------------------------------------------------------------

    private fun drawFrame() {
        val canvas = holder.lockCanvas() ?: return
        try {
            canvas.drawColor(Color.parseColor("#FFF8E1"))

            val cells = NativeBridge.readBoard()
            val ghost = NativeBridge.ghostPositions()
            val cell = min(width / cols, height / rows).toFloat()
            val offsetX = (width - cols * cell) / 2f
            val offsetY = (height - rows * cell) / 2f

            // 枠
            paint.style = Paint.Style.STROKE
            paint.strokeWidth = 4f
            paint.color = Color.LTGRAY
            canvas.drawRect(offsetX, offsetY, offsetX + cols * cell, offsetY + rows * cell, paint)

            // ゴースト
            paint.style = Paint.Style.FILL
            paint.color = Color.argb(80, 200, 200, 200)
            for (i in ghost.indices step 2) {
                val gx = ghost[i]
                val gy = ghost[i + 1]
                val l = offsetX + gx * cell
                val t = offsetY + gy * cell
                canvas.drawRect(l, t, l + cell, t + cell, paint)
            }

            // ブロック
            for (y in 0 until rows) {
                for (x in 0 until cols) {
                    val i = y * cols + x
                    val v = if (i < cells.size) cells[i] else 0
                    if (v != 0) {
                        val l = offsetX + x * cell
                        val t = offsetY + y * cell

                        paint.style = Paint.Style.FILL
                        paint.color = pieceColors[v - 1]
                        canvas.drawRect(l, t, l + cell, t + cell, paint)

                        paint.style = Paint.Style.STROKE
                        paint.color = Color.argb(255, 30, 30, 30)
                        canvas.drawRect(l, t, l + cell, t + cell, paint)
                    }
                }
            }

            // HUD（Score / Level / Time）
            paint.style = Paint.Style.FILL
            paint.color = Color.WHITE
            paint.textSize = 42f
            canvas.drawText("Score: ${NativeBridge.score()}", 16f, 50f, paint)
            canvas.drawText("Level: ${NativeBridge.level()}", 16f, 96f, paint)

            val mm = (elapsedSec / 60f).toInt()
            val ss = (elapsedSec % 60f).toInt()
            canvas.drawText(String.format("Time: %02d:%02d", mm, ss), 16f, 142f, paint)

            if (NativeBridge.isGameOver()) {
                paint.textSize = 64f
                canvas.drawText("GAME OVER", width / 2f - 180f, height / 2f, paint)
            } else if (paused) {
                paint.textSize = 64f
                canvas.drawText("PAUSED", width / 2f - 120f, height / 2f, paint)
            }
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }

    // --- 外部（Activity）からのポーズ切替 -----------------------------------

    fun setPaused(p: Boolean) {
        paused = p
        NativeBridge.setPaused(p) // C++ 側の進行も止める
    }

    fun isPaused(): Boolean = paused
}
