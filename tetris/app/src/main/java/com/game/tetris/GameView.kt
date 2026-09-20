package com.game.tetris

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.SurfaceHolder
import android.view.SurfaceView
import java.util.concurrent.ConcurrentLinkedQueue
import kotlin.math.min

class GameView(context: Context, attrs: AttributeSet? = null) :
    SurfaceView(context, attrs), SurfaceHolder.Callback {

    private val commands = ConcurrentLinkedQueue<Int>()
    @Volatile private var running = false
    @Volatile private var paused = false
    @Volatile private var gameOver = false
    private var gameThread: Thread? = null
    private var initialized = false
    private var lastNs = 0L
    private var elapsedSec = 0f
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val density = resources.displayMetrics.density
    private val scaledDensity = resources.displayMetrics.scaledDensity
    private val colors = intArrayOf(0xFF00BCD4.toInt(), 0xFFFFEB3B.toInt(), 0xFF4CAF50.toInt(),
        0xFFF44336.toInt(), 0xFF3F51B5.toInt(), 0xFFFF9800.toInt(), 0xFF9C27B0.toInt())

    private var downX = 0f; private var downY = 0f
    private var lastX = 0f; private var lastY = 0f
    private var accX = 0f; private var accY = 0f; private var moved = false

    init { holder.addCallback(this); isFocusable = true }

    override fun surfaceCreated(holder: SurfaceHolder) {
        synchronized(this) {
            if (gameThread?.isAlive == true) return
            running = true
            lastNs = System.nanoTime()
            gameThread = Thread(::loop, "TetrisGame").also { it.start() }
        }
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        running = false
        val thread = gameThread
        thread?.interrupt()
        try { thread?.join(2_000) } catch (_: InterruptedException) { Thread.currentThread().interrupt() }
        synchronized(this) { if (gameThread === thread) gameThread = null }
    }
    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) = Unit

    fun togglePause() { if (!gameOver) commands.add(if (paused) RESUME else PAUSE) }
    fun pause() { commands.add(PAUSE) }
    fun hardDrop() { if (!paused && !gameOver) commands.add(NativeBridge.HARD_DROP) }
    fun restart() { commands.clear(); commands.add(RESTART) }
    fun isPaused(): Boolean = paused
    fun isGameOver(): Boolean = gameOver

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (paused || gameOver) return true
        val cell = min(width / 10f, height / 20f)
        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> { downX=event.x; downY=event.y; lastX=event.x; lastY=event.y; accX=0f; accY=0f; moved=false }
            MotionEvent.ACTION_MOVE -> {
                accX += event.x-lastX; accY += event.y-lastY; lastX=event.x; lastY=event.y
                val step = cell * .5f
                while (accX > step) { commands.add(NativeBridge.RIGHT); accX-=step; moved=true }
                while (accX < -step) { commands.add(NativeBridge.LEFT); accX+=step; moved=true }
                while (accY > step) { commands.add(NativeBridge.SOFT_DROP); accY-=step; moved=true }
            }
            MotionEvent.ACTION_UP -> {
                val dx=event.x-downX; val dy=event.y-downY; val threshold=cell*.25f
                if (!moved && dx*dx+dy*dy < threshold*threshold) {
                    commands.add(NativeBridge.ROTATE_CW)
                    performClick()
                }
            }
        }
        return true
    }

    override fun performClick(): Boolean { super.performClick(); return true }

    private fun loop() {
        if (!initialized) { NativeBridge.restart(newSeed()); initialized = true }
        while (running) {
            val now=System.nanoTime(); val dt=((now-lastNs)/1_000_000_000f).coerceAtMost(.05f); lastNs=now
            drainCommands()
            if (!paused && !gameOver) elapsedSec += dt
            NativeBridge.update(dt)
            val snapshot=NativeBridge.renderSnapshot()
            paused=snapshot[238] != 0; gameOver=snapshot[239] != 0
            drawFrame(snapshot)
            try { Thread.sleep(16) } catch (_: InterruptedException) { if (!running) break }
        }
    }

    private fun drainCommands() {
        while (true) when (val command=commands.poll() ?: break) {
            PAUSE -> NativeBridge.setPaused(true)
            RESUME -> NativeBridge.setPaused(false)
            RESTART -> { NativeBridge.restart(newSeed()); elapsedSec=0f }
            else -> NativeBridge.command(command)
        }
    }

    private fun drawFrame(data: IntArray) {
        val canvas=holder.lockCanvas() ?: return
        try {
            canvas.drawColor(Color.BLACK)
            val hud=64f*density; val bottom=72f*density
            val cell=min(width/10f, (height-hud-bottom)/20f)
            val left=(width-cell*10)/2f; val top=hud+(height-hud-bottom-cell*20)/2f
            paint.style=Paint.Style.STROKE; paint.strokeWidth=2f*density; paint.color=Color.LTGRAY
            canvas.drawRect(left,top,left+cell*10,top+cell*20,paint)
            drawPositions(canvas,data,228,Color.argb(80,200,200,200),cell,left,top)
            for (y in 2 until 22) for (x in 0 until 10) {
                val value=data[y*10+x]; if (value != 0) drawCell(canvas,x,y-2,colors[value-1],cell,left,top)
            }
            // Active piece is deliberately separate from the locked board.
            drawPositions(canvas,data,220,Color.WHITE,cell,left,top)
            paint.style=Paint.Style.FILL; paint.color=Color.WHITE; paint.textSize=18f*scaledDensity
            canvas.drawText("Score ${data[236]}   Level ${data[237]}   Time ${formatTime()}",16f*density,38f*density,paint)
            if (gameOver) overlay(canvas,"GAME OVER\nScore: ${data[236]}") else if (paused) overlay(canvas,"PAUSED")
        } finally { holder.unlockCanvasAndPost(canvas) }
    }

    private fun drawPositions(c: Canvas,d:IntArray,start:Int,color:Int,cell:Float,left:Float,top:Float) {
        for(i in start until start+8 step 2) { val x=d[i]; val y=d[i+1]-2; if(y in 0..19) drawCell(c,x,y,color,cell,left,top) }
    }
    private fun drawCell(c:Canvas,x:Int,y:Int,color:Int,cell:Float,left:Float,top:Float) {
        paint.style=Paint.Style.FILL; paint.color=color; c.drawRect(left+x*cell,top+y*cell,left+(x+1)*cell,top+(y+1)*cell,paint)
        paint.style=Paint.Style.STROKE; paint.color=0xFF202020.toInt(); c.drawRect(left+x*cell,top+y*cell,left+(x+1)*cell,top+(y+1)*cell,paint)
    }
    private fun overlay(c:Canvas,text:String) { paint.textAlign=Paint.Align.CENTER; paint.textSize=28f*scaledDensity; text.lines().forEachIndexed{i,s->c.drawText(s,width/2f,height/2f+i*34f*density,paint)}; paint.textAlign=Paint.Align.LEFT }
    private fun formatTime()="%02d:%02d".format((elapsedSec/60).toInt(),(elapsedSec%60).toInt())
    private fun newSeed()=(System.nanoTime() xor System.currentTimeMillis()).toInt()

    private companion object { const val PAUSE=100; const val RESUME=101; const val RESTART=102 }
}
