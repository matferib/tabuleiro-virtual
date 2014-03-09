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
    mRenderer = new TabuleiroRenderer();
    setRenderer(mRenderer);
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

class TabuleiroRenderer implements GLSurfaceView.Renderer {
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    nativeInit();
  }

  public void onSurfaceChanged(GL10 gl, int w, int h) {
    //gl.glViewport(0, 0, w, h);
    nativeResize(w, h);
  }

  public void onDrawFrame(GL10 gl) {
    nativeRender();
  }

  private static native void nativeInit();
  private static native void nativeResize(int w, int h);
  private static native void nativeRender();
  private static native void nativeDone();
}
