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
    renderer_ = new TabuleiroRenderer(this);
    setRenderer(renderer_);
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
  }

  public boolean onTouchEvent(final MotionEvent event) {
    MotionEvent event_copy = MotionEvent.obtain(event);
    event_copy.setLocation(event.getX(), getHeight() - event.getY());
    renderer_.PushBack(event_copy);
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

  TabuleiroRenderer renderer_;
  private static native void nativePause();
  private static native void nativeResume();
}

class TabuleiroRenderer extends java.util.TimerTask implements GLSurfaceView.Renderer, Runnable {
  public TabuleiroRenderer(GLSurfaceView parent) {
    parent_ = parent;
    last_x_ = last_y_ = 0;
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
    nativeTimer();
    synchronized (events_) {
      for (int i = 0; i < events_.size(); ++i) {
        MotionEvent event = events_.get(i);
        if (event.getActionMasked() == MotionEvent.ACTION_MOVE &&
            i < (events_.size() - 1) &&
            events_.get(i + 1).getActionMasked() == MotionEvent.ACTION_MOVE) {
          // So pega um movimento por vez.
          continue;
        }
        processTouchEvent(event);
      }
      events_.clear();
    }
    nativeRender();
  }

  public void run() {
    parent_.requestRender();
  }

  public void PushBack(MotionEvent event) {
    synchronized (events_) {
      events_.add(event);
    }
  }

  private void processTouchEvent(final MotionEvent event) {
    if (event.getPointerCount() > 1) {
      return;
    }
    //System.out.println("Evento: " + event.toString() + ", x: " + event.getX() + ", y: " + event.getY());
    if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
      nativeTouchPressed(event.getX(), event.getY());
    } else if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
      if (event.getX() != last_x_ || event.getY() != last_y_) {
        nativeTouchMoved(event.getX(), event.getY());
      }
    } else if (event.getActionMasked() == MotionEvent.ACTION_UP) {
      nativeTouchReleased();
    }
    last_x_ = event.getX();
    last_y_ = event.getY();
    return;
  }

  private static native void nativeInit();
  private static native void nativeResize(int w, int h);
  private static native void nativeRender();
  private static native void nativeDone();
  private static native void nativeTimer();
  private static native void nativeTouchPressed(float x, float y);
  private static native void nativeTouchMoved(float x, float y);
  private static native void nativeTouchReleased();

  private GLSurfaceView parent_;

  public static final int ESTADO_INICIAL = 1;
  public static final int ESTADO_TOCADO = 2;
  private int estado_ = ESTADO_INICIAL;
  private java.util.Vector<MotionEvent> events_ = new java.util.Vector<MotionEvent>();
  private float last_x_;
  private float last_y_;
}
