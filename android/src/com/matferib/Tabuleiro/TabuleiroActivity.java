/** Baseado no SanAngeles demo application. So uma casca Java para chamar o codigo nativo. */
package com.matferib.Tabuleiro;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;

public class TabuleiroActivity extends Activity {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    mGLView = new TabuleiroSurfaceView(this);
    setContentView(mGLView);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();
  }

  private GLSurfaceView mGLView;

  static {
    System.loadLibrary("tabuleiro");
  }
}

class TabuleiroSurfaceView extends GLSurfaceView {
  public TabuleiroSurfaceView(Context context) {
    super(context);
    mRenderer = new TabuleiroRenderer(this);
    setRenderer(mRenderer);
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
  }

  public boolean onTouchEvent(final MotionEvent event) {
    if (event.getAction() == MotionEvent.ACTION_DOWN) {
      nativeTogglePauseResume();
    }
    return true;
  }

  @Override
  public void onPause() {
    super.onPause();
    nativePause();
  }

  @Override
  public void onResume() {
    super.onResume();
    nativeResume();
  }

  TabuleiroRenderer mRenderer;

  private static native void nativePause();
  private static native void nativeResume();
  private static native void nativeTogglePauseResume();
}

class TabuleiroRenderer extends java.util.TimerTask implements GLSurfaceView.Renderer, Runnable {
  public TabuleiroRenderer(GLSurfaceView parent) {
    parent_ = parent;
  }

  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    nativeInit();
    // Liga o timer a cada 10 ms.
    java.util.Timer timer = new java.util.Timer();
    timer.scheduleAtFixedRate(this, 0, 10);
  }

  public void onSurfaceChanged(GL10 gl, int w, int h) {
    nativeResize(w, h);
  }

  public void onDrawFrame(GL10 gl) {
    nativeRender();
  }

  public void run() {
    nativeTimer();
    parent_.requestRender();
  }

  private static native void nativeInit();
  private static native void nativeResize(int w, int h);
  private static native void nativeRender();
  private static native void nativeDone();
  private static native void nativeTimer();

  private GLSurfaceView parent_;
}
