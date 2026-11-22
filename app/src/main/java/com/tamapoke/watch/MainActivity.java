package com.tamapoke.watch;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.BatteryManager;
import android.os.Bundle;
import android.view.Choreographer;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

public class MainActivity extends Activity implements SurfaceHolder.Callback, Choreographer.FrameCallback {
    private static final int CANVAS_SIZE = 466;

    static {
        System.loadLibrary("tamapoke_wear");
    }

    private SurfaceView surfaceView;
    private Bitmap frameBitmap;
    private final Rect srcRect = new Rect(0, 0, CANVAS_SIZE, CANVAS_SIZE);
    private final Rect dstRect = new Rect();
    private final Paint drawPaint = new Paint(Paint.FILTER_BITMAP_FLAG);
    private final AudioSynthPlayer audioPlayer = new AudioSynthPlayer();
    private final Utf8TextRasterizer textRasterizer = new Utf8TextRasterizer();

    private boolean surfaceReady = false;
    private boolean running = false;
    private int viewWidth = CANVAS_SIZE;
    private int viewHeight = CANVAS_SIZE;

    private final BroadcastReceiver batteryReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            updateBatteryFromIntent(intent);
        }
    };

    private native void nativeInit(AssetManager assetManager, String filesDir);
    private native boolean nativeStep(Bitmap bitmap);
    private native void nativeTouch(int action, int x, int y);
    private native boolean nativeBackPressed();
    private native void nativeRotary(int delta);
    private native void nativeOnPause();
    private native void nativeOnResume();
    private native void nativeSetBattery(int percent, boolean charging);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        frameBitmap = Bitmap.createBitmap(CANVAS_SIZE, CANVAS_SIZE, Bitmap.Config.RGB_565);
        surfaceView = new SurfaceView(this);
        surfaceView.setFocusable(true);
        surfaceView.setFocusableInTouchMode(true);
        surfaceView.getHolder().addCallback(this);
        surfaceView.setOnTouchListener((v, event) -> {
            dispatchScaledTouch(event);
            return true;
        });
        surfaceView.setOnGenericMotionListener((v, event) -> {
            if ((event.getSource() & InputDevice.SOURCE_ROTARY_ENCODER) != 0 &&
                    event.getAction() == MotionEvent.ACTION_SCROLL) {
                float delta = event.getAxisValue(MotionEvent.AXIS_SCROLL);
                if (delta != 0f) {
                    nativeRotary(delta > 0f ? 1 : -1);
                    return true;
                }
            }
            return false;
        });
        setContentView(surfaceView);
        surfaceView.requestFocus();
        hideSystemBars();
        nativeInit(getAssets(), getFilesDir().getAbsolutePath());
    }

    private void hideSystemBars() {
        View decor = getWindow().getDecorView();
        WindowInsetsController ctrl = decor.getWindowInsetsController();
        if (ctrl != null) {
            ctrl.hide(WindowInsets.Type.statusBars() | WindowInsets.Type.navigationBars());
            ctrl.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        }
    }

    private void dispatchScaledTouch(MotionEvent event) {
        int side = Math.min(viewWidth, viewHeight);
        if (side <= 0) return;
        int left = (viewWidth - side) / 2;
        int top = (viewHeight - side) / 2;
        int tx = Math.round((event.getX() - left) * CANVAS_SIZE / (float) side);
        int ty = Math.round((event.getY() - top) * CANVAS_SIZE / (float) side);
        nativeTouch(event.getActionMasked(), tx, ty);
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemBars();
        IntentFilter batFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        if (android.os.Build.VERSION.SDK_INT >= 33) {
            registerReceiver(batteryReceiver, batFilter, Context.RECEIVER_NOT_EXPORTED);
        } else {
            registerReceiver(batteryReceiver, batFilter);
        }
        Intent sticky = registerReceiver(null, batFilter);
        if (sticky != null) updateBatteryFromIntent(sticky);
        nativeOnResume();
        running = true;
        Choreographer.getInstance().postFrameCallback(this);
    }

    @Override
    protected void onPause() {
        running = false;
        Choreographer.getInstance().removeFrameCallback(this);
        try {
            unregisterReceiver(batteryReceiver);
        } catch (IllegalArgumentException ignored) {
        }
        nativeOnPause();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        audioPlayer.release();
        super.onDestroy();
    }

    @Override
    @SuppressWarnings("deprecation")
    public void onBackPressed() {
        if (nativeBackPressed()) {
            if (nativeStep(frameBitmap) && surfaceReady) drawFrame();
            return;
        }
        super.onBackPressed();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        surfaceReady = true;
        if (nativeStep(frameBitmap)) drawFrame();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        viewWidth = width;
        viewHeight = height;
        int side = Math.min(width, height);
        int left = (width - side) / 2;
        int top = (height - side) / 2;
        dstRect.set(left, top, left + side, top + side);
        drawFrame();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        surfaceReady = false;
    }

    @Override
    public void doFrame(long frameTimeNanos) {
        if (!running) return;
        if (nativeStep(frameBitmap) && surfaceReady) {
            drawFrame();
        }
        Choreographer.getInstance().postFrameCallback(this);
    }

    private void drawFrame() {
        SurfaceHolder holder = surfaceView.getHolder();
        Canvas canvas = holder.lockCanvas();
        if (canvas == null) return;
        try {
            canvas.drawColor(Color.BLACK);
            canvas.drawBitmap(frameBitmap, srcRect, dstRect, drawPaint);
        } finally {
            holder.unlockCanvasAndPost(canvas);
        }
    }

    private void updateBatteryFromIntent(Intent intent) {
        int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
        int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        int pct = (level >= 0 && scale > 0) ? (level * 100 / scale) : -1;
        boolean charging = status == BatteryManager.BATTERY_STATUS_CHARGING ||
                status == BatteryManager.BATTERY_STATUS_FULL;
        nativeSetBattery(pct, charging);
    }

    public void playPcmAudio(short[] samples) {
        audioPlayer.play(samples);
    }

    public int[] rasterizeUtf8Text(String text, int size, boolean drawPixels) {
        return textRasterizer.rasterize(text, size, drawPixels);
    }
}
