/** Baseado no SanAngeles demo application. So uma casca Java para chamar o codigo nativo. */
package com.matferib.Tabuleiro;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;

// Atividade do tabuleiro que possui o view do OpenGL.
public class TabuleiroActivity extends Activity {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Intent intencao = getIntent();
    mGLView = new TabuleiroSurfaceView(this, getIntent().getStringExtra(SelecaoActivity.MENSAGEM_EXTRA));
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

// View do OpenGL.
class TabuleiroSurfaceView extends GLSurfaceView {
  public TabuleiroSurfaceView(Context context, String endereco) {
    super(context);
    renderer_ = new TabuleiroRenderer(this, endereco);
    detectorEventos_ = new GestureDetector(context, renderer_);
    detectorEventos_.setOnDoubleTapListener(renderer_);
    detectorEscala_ = new ScaleGestureDetector(context, renderer_);
    detectorEscala_.setQuickScaleEnabled(true);
    setRenderer(renderer_);
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
  }

  public boolean onTouchEvent(final MotionEvent event) {
    //detectorEventos_.onTouchEvent(event);
    detectorEscala_.onTouchEvent(event);
    return true;
    /* old
    MotionEvent event_copy = MotionEvent.obtain(event);
    event_copy.setLocation(event.getX(), getHeight() - event.getY());
    renderer_.pushBack(event_copy);
    return true;
    */
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

  private TabuleiroRenderer renderer_;
  private GestureDetector detectorEventos_;
  private ScaleGestureDetector detectorEscala_;
  private static native void nativePause();
  private static native void nativeResume();
}

// Renderizador do tabuleiro. Responsavel pelo timer que atualiza o tabuleiro.
class TabuleiroRenderer extends java.util.TimerTask
    implements GLSurfaceView.Renderer,
               Runnable,
               GestureDetector.OnGestureListener,
               GestureDetector.OnDoubleTapListener,
               ScaleGestureDetector.OnScaleGestureListener {

  public static final String TAG = "TabuleiroRenderer";

  public TabuleiroRenderer(GLSurfaceView parent, String endereco) {
    endereco_ = endereco;
    parent_ = parent;
    last_x_ = last_y_ = 0;
  }

  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    nativeInit(endereco_);
    // Liga o timer a cada 33 ms. TODO: funcao nativa para retornar o periodo do timer.
    java.util.Timer timer = new java.util.Timer();
    timer.scheduleAtFixedRate(this, 0, 33);
  }

  public void onSurfaceChanged(GL10 gl, int w, int h) {
    nativeResize(w, h);
  }

  /** Toda atualizacao eh feita daqui para acontecer na mesma thread que o grafico. */
  public void onDrawFrame(GL10 gl) {
    nativeTimer();
    synchronized (eventosEscala_) {
      for (Float fator : eventosEscala_) {
        nativeScale(fator.floatValue());
      }
      eventosEscala_.clear();
    }
    synchronized (eventos_) {
      /*
      for (int i = 0; i < events_.size(); ++i) {
        MotionEvent event = events_.get(i);
        if (event.getActionMasked() == MotionEvent.ACTION_MOVE &&
            i < (events_.size() - 1) &&
            events_.get(i + 1).getActionMasked() == MotionEvent.ACTION_MOVE) {
          // So pega um movimento por vez, descarta o resto.
          continue;
        }
        processTouchEvent(event);
      }
      events_.clear();
      */
    }
    nativeRender();
  }

  /** Sera chamada pelo timer a cada intervalo de atualizacao. Pode vir de qualquer thread. */
  public void run() {
    parent_.requestRender();
  }

  // Detector de eventos.
  @Override
  public boolean onDown(MotionEvent event) {
    return true;
  }

  @Override
  public boolean onFling(MotionEvent event1, MotionEvent event2,
      float velocityX, float velocityY) {
    return true;
  }

  @Override
  public void onLongPress(MotionEvent event) {
  }

  @Override
  public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
    return true;
  }

  @Override
  public void onShowPress(MotionEvent event) {
  }

  @Override
  public boolean onSingleTapUp(MotionEvent event) {
    return true;
  }

  @Override
  public boolean onDoubleTap(MotionEvent event) {
    return true;
  }

  @Override
  public boolean onDoubleTapEvent(MotionEvent event) {
    return true;
  }

  @Override
  public boolean onSingleTapConfirmed(MotionEvent event) {
    return true;
  }

  // Detector de escala.
  @Override
  public boolean onScale(ScaleGestureDetector detector) {
    //Log.d(TAG, "Escala");
    eventosEscala_.add(new Float(detector.getScaleFactor()));
    return true;
  }

  @Override
  public boolean onScaleBegin(ScaleGestureDetector detector) {
    return true;
  }

  @Override
  public void onScaleEnd(ScaleGestureDetector detector) {
  }

  /* Old
  public void pushBack(MotionEvent event) {
    synchronized (events_) {
      events_.add(event);
    }
  }

  private void processTouchEvent(final MotionEvent event) {
    int eventX = (int)event.getX();
    int eventY = (int)event.getY();
    if (event.getPointerCount() == 1) {
      //System.out.println("Evento: " + event.toString() + ", x: " + eventX() + ", y: " + eventY());
      if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
        System.out.println("TOUCH PRESS");
        nativeTouchPressed(eventX, eventY);
      } else if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
        if (eventX != last_x_ || eventY != last_y_) {
          System.out.println("TOUCH MOVE");
          nativeTouchMoved(eventX, eventY);
        }
      } else if (event.getActionMasked() == MotionEvent.ACTION_UP) {
        System.out.println("TOUCH RELEASE");
        nativeTouchReleased();
      }
      last_x_ = eventX;
      last_y_ = eventY;
      return;
    } else if (event.getPointerCount() == 2) {
      if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
        System.out.println("PINCH PRESS");
        nativeTouchReleased();
        nativePinchPressed(eventX, eventY);
      } else if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
        if (eventX != last_x_ || eventY != last_y_) {
          System.out.println("PINCH MOVE");
          nativeTouchMoved(eventX, eventY);
        }
      } else if (event.getActionMasked() == MotionEvent.ACTION_UP) {
        System.out.println("PINCH RELEASE");
        nativeTouchReleased();
      }
      last_x_ = eventX;
      last_y_ = eventY;
      return;

    }
  }
    */

  private static native void nativeInit(String endereco);
  private static native void nativeResize(int w, int h);
  private static native void nativeRender();
  private static native void nativeDone();
  private static native void nativeTimer();
  private static native void nativeTouchPressed(int x, int y);
  private static native void nativeTouchMoved(int x, int y);
  private static native void nativeTouchReleased();
  private static native void nativeScale(float s);

  private GLSurfaceView parent_;

  public static final int ESTADO_INICIAL = 1;
  public static final int ESTADO_TOCADO = 2;
  private int estado_ = ESTADO_INICIAL;
  private java.util.Vector<MotionEvent> eventos_ = new java.util.Vector<MotionEvent>();
  private java.util.Vector<Float> eventosEscala_ = new java.util.Vector<Float>();
  private int last_x_;
  private int last_y_;
  private String endereco_;
}
