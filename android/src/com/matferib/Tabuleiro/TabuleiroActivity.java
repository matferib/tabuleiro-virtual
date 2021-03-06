/** Baseado no SanAngeles demo application. So uma casca Java para chamar o codigo nativo. */
package com.matferib.Tabuleiro;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Matrix;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.Surface;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.NumberPicker;
import android.widget.Spinner;

import com.matferib.Tabuleiro.ent.EntidadeProto;
import com.matferib.Tabuleiro.ent.IluminacaoPontual;
import com.matferib.Tabuleiro.ent.TipoVisao;
import com.squareup.wire.Wire;

import java.util.Vector;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

// Atividade do tabuleiro que possui o view do OpenGL.
public class TabuleiroActivity extends Activity implements View.OnSystemUiVisibilityChangeListener {
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Log.d("TabuleiroActivity", "onCreate");
    view_ = new TabuleiroSurfaceView(this);
    view_.setOnSystemUiVisibilityChangeListener(this);
    if (!fixed_size_) {
      setContentView(view_);
    } else {
      view_.getHolder().setFixedSize(1024, 768);
      class AutoFrameLayout extends FrameLayout {
        AutoFrameLayout(TabuleiroActivity activity, View filha) {
          super(activity);
          addView(filha);
          filha_ = filha;
        }
        @Override
        public boolean onInterceptTouchEvent(MotionEvent event) {
          return true;
        }
        @Override
        public boolean onTouchEvent(MotionEvent event) {
          Log.d("TabuleiroSurfaceView", "antes: " + event.toString());
          Matrix m = new Matrix();
          m.preScale(1024.0f / getWidth(), 768.0f / getHeight());
          event.transform(m);
          Log.d("TabuleiroSurfaceView", "depois: " + event.toString());
          return filha_.onTouchEvent(event);
        }

        private View filha_;
      };
      AutoFrameLayout fl = new AutoFrameLayout(this, view_);
      setContentView(fl);
    }
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    nativeCreate(
        getIntent().getStringExtra(SelecaoActivity.SERVIDOR) != null,
        getIntent().getStringExtra(SelecaoActivity.NOME),
        getIntent().getStringExtra(SelecaoActivity.ENDERECO),
        getIntent().getBooleanExtra(SelecaoActivity.MAPEAMENTO_SOMBRAS, false),
        getIntent().getBooleanExtra(SelecaoActivity.LUZ_POR_PIXEL, false),
        getResources().getAssets(),
        ((android.content.Context)this).getFilesDir().getAbsolutePath());
    view_.requestFocus();
  }

  private void hideUi() {
    Log.d("TabuleiroActivity", "hideUi");
    if (Build.VERSION.SDK_INT >= 19) {
      Log.d("TabuleiroActivity", "hideUiInside");
      // Por causa de algum bug bizarro do android, tem que esperar um pouquinho para mudar a visibilidade
      // da barra do sistema.
      view_.postDelayed(new Runnable() {
        @Override
        public void run() {
          getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_FULLSCREEN
            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
      }, 500);
    }
  }

  @Override
  public void onSystemUiVisibilityChange(int visibility) {
    Log.d("TabuleiroActivity", "onSystemUiVisibilityChange: " + visibility/*, new Exception()*/);
  }

  @Override
  public void onWindowFocusChanged(boolean hasFocus) {
    Log.d("TabuleiroActivity", "onWindowsFocusChanged: " + hasFocus);
    super.onWindowFocusChanged(hasFocus);
    if (hasFocus) {
      hideUi();
    }
  }

  @Override
  public void onConfigurationChanged(Configuration newConfig) {
    Log.d("TabuleiroActivity", "onConfigurationChanged");
    super.onConfigurationChanged(newConfig);
  }

  @Override
  public void onContentChanged() {
    Log.d("TabuleiroActivity", "onContentChanged");
    super.onContentChanged();
  }

  @Override
  protected void onPause() {
    Log.d("TabuleiroActivity", "onPause");
    super.onPause();
    view_.onPause();
  }

  @Override
  protected void onResume() {
    Log.d("TabuleiroActivity", "onResume");
    super.onResume();
    view_.onResume();
  }

  @Override
  protected void onDestroy() {
    Log.d("TabuleiroActivity", "onDestroy");
    super.onStop();
    nativeDestroy();
  }

  public boolean fixed_size_ = false;
  private native void nativeCreate(boolean servidor, String nome, String endereco,
                                   boolean mapeamento_sombras, boolean luz_por_pixel, Object assets, String dir);
  private static native void nativeDestroy();

  private GLSurfaceView view_;
}

class TabvirtConfigChooser implements GLSurfaceView.EGLConfigChooser {
  @Override
  public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
    int attribs[] = {
      EGL10.EGL_LEVEL, 0,
      EGL10.EGL_RENDERABLE_TYPE, 4,  //EGL10.EGL_OPENGL_ES2_BIT,
      EGL10.EGL_COLOR_BUFFER_TYPE, EGL10.EGL_RGB_BUFFER,
      EGL10.EGL_RED_SIZE, 8,
      EGL10.EGL_GREEN_SIZE, 8,
      EGL10.EGL_BLUE_SIZE, 8,
      EGL10.EGL_ALPHA_SIZE, 8,
      EGL10.EGL_DEPTH_SIZE, 24,
      EGL10.EGL_STENCIL_SIZE, 1,
      // multisampling.
      //EGL10.EGL_SAMPLE_BUFFERS, 1,
      //EGL10.EGL_SAMPLES, 4,  // This is for 4x MSAA.
      EGL10.EGL_NONE
    };
    EGLConfig[] configs = new EGLConfig[1];
    int[] configCounts = new int[1];
    egl.eglChooseConfig(display, attribs, configs, 1, configCounts);

    if (configCounts[0] == 0) {
      // Failed! Error handling.
      return null;
    } else {
      return configs[0];
    }
  }
}

// View do OpenGL.
class TabuleiroSurfaceView extends GLSurfaceView {
  public static final String TAG = "TabuleiroSurfaceView";

  public TabuleiroSurfaceView(Activity activity) {
    super(activity);
    setEGLContextClientVersion(2);
    renderer_ = new TabuleiroRenderer(activity, this, getResources(), OrientacaoPadrao(activity));
    ultima_renderizacao_ms_ = 0;
    num_frames_pular_ = 0;
    detectorEventos_ = new GestureDetector(activity, renderer_);
    detectorEventos_.setOnDoubleTapListener(renderer_);
    detectorEventos_.setIsLongpressEnabled(false);
    detectorPressao_ = new PressureDetector(renderer_);
    detectorEscala_ = new ScaleGestureDetector(activity, renderer_);
    if (Build.VERSION.SDK_INT >= 19) {
      detectorEscala_.setQuickScaleEnabled(true);
    }
    detectorRotacao_ = new RotationGestureDetector(renderer_);
    detectorTranslacao_ = new TranslationGestureDetector(renderer_);
    //setEGLConfigChooser(8, 8, 8, 8, 16, 1);
    setEGLConfigChooser(new TabvirtConfigChooser());
    setRenderer(renderer_);
    setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    setFocusableInTouchMode(true);
    //setDebugFlags(DEBUG_CHECK_GL_ERROR  | DEBUG_LOG_GL_CALLS);
  }

  private int OrientacaoPadrao(Context context) {
    // Encontra orientacao padrao do dispositivo, para inclinacao. Referencia:
    // http://stackoverflow.com/questions/4553650/how-to-check-device-natural-default-orientation-on-android-i-e-get-landscape
    WindowManager windowManager = (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);
    Configuration config = getResources().getConfiguration();
    int rotation = windowManager.getDefaultDisplay().getRotation();
    if (((rotation == Surface.ROTATION_0 || rotation == Surface.ROTATION_180) &&
          config.orientation == Configuration.ORIENTATION_LANDSCAPE)
        || ((rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270) &&
          config.orientation == Configuration.ORIENTATION_PORTRAIT)) {
      return Configuration.ORIENTATION_LANDSCAPE;
    } else {
      return Configuration.ORIENTATION_PORTRAIT;
    }
  }

  // A sequencia de eventos acontece da seguinte forma:
  // Primeiro toque: ON_ACTION_DOWN.
  // Segundo, terceiro... toques: ON_ACTION_POINTER_DOWN
  // Tirou terceiro, segundo: ON_ACTION_POINTER_UP
  // Tirou ultimo: ON_ACTION_UP.
  // Em resumo, ON_ACTION_POINTER_* serve para detectar ponteiros secundarios.
  @Override
  public boolean onTouchEvent(final MotionEvent event) {
    //Log.d("TabuleiroRenderer", event.toString());
    renderer_.habilitaSensores(event.getPointerCount() == 2);
    if (event.getActionMasked() == MotionEvent.ACTION_UP || event.getActionMasked() == MotionEvent.ACTION_POINTER_UP) {
      renderer_.onUp(event);
      detectorEventos_.onTouchEvent(event);
      if (event.getPointerCount() == 1) {
        // Voltou pro estado inicial.
        estado_ = ESTADO_OCIOSO;
      }
      return true;
    }

    // O detector de eventos tem que ficar ciente dos up e downs para poder confirmar o single tap e nao confirma-lo
    // quando houver duplo clique.
    if (event.getActionMasked() == MotionEvent.ACTION_DOWN || event.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN) {
      detectorEventos_.onTouchEvent(event);
    }

    if (event.getPointerCount() <= 1) {
      if (estado_ != ESTADO_OCIOSO) {
        return true;
      }
      if (event.getActionMasked() == MotionEvent.ACTION_MOVE) {
        // Scroll nao foi detectado acima.
        detectorEventos_.onTouchEvent(event);
      }
      // O detector de pressao esta causando algum tilt na pinca.
      //detectorPressao_.onTouch(event);
    } else if (event.getPointerCount() == 2) {
      if (estado_ != ESTADO_OCIOSO && estado_ != ESTADO_MULTITOQUE_2) {
        return true;
      }
      if (estado_ == ESTADO_OCIOSO) {
        // notifica os dois toques.
      }
      detectorRotacao_.onTouch(event);
      detectorEscala_.onTouchEvent(event);
      detectorTranslacao_.onTouch(event);
      estado_ = ESTADO_MULTITOQUE_2;
    } else if (event.getPointerCount() == 3) {
      if (estado_ != ESTADO_OCIOSO &&
          estado_ != ESTADO_MULTITOQUE_2 &&  // pode vir do estado de dois toques.
          estado_ != ESTADO_MULTITOQUE_3) {
        return true;
      }
      //renderer_.onActionTouch(event);
      estado_ = ESTADO_MULTITOQUE_3;
    }
    return true;
  }

  @Override
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    //Log.d("TabuleiroRenderer", "onKeyDown: " + keyCode);
    switch (keyCode) {
      case android.view.KeyEvent.KEYCODE_SHIFT_LEFT:
      case android.view.KeyEvent.KEYCODE_SHIFT_RIGHT:
      case android.view.KeyEvent.KEYCODE_ALT_LEFT:
      case android.view.KeyEvent.KEYCODE_ALT_RIGHT:
      case android.view.KeyEvent.KEYCODE_CTRL_LEFT:
      case android.view.KeyEvent.KEYCODE_CTRL_RIGHT:
        return renderer_.onKeyDown(keyCode, event);
      default:
        return false;
    }
  }

  @Override
  public boolean onKeyUp(int keyCode, KeyEvent event) {
    //Log.d("TabuleiroRenderer", "onKeyUp: " + keyCode);
    return renderer_.onKeyUp(keyCode, event);
  }

  @Override
  public void onPause() {
    super.onPause();
    timer_.cancel();
    timer_ = null;
    gerenteSensores_.unregisterListener(renderer_);
    nativePause();
  }

  @Override
  public void onResume() {
    Log.d("TabuleiroRenderer", "onResume");
    super.onResume();
    gerenteSensores_.registerListener(renderer_,
                                      gerenteSensores_.getDefaultSensor(Sensor.TYPE_GYROSCOPE),
                                      SensorManager.SENSOR_DELAY_NORMAL);
    nativeResume();
    // Liga o timer a cada 33 ms. TODO: funcao nativa para retornar o periodo do timer.
    timer_ = new java.util.Timer();
    timer_.scheduleAtFixedRate(new java.util.TimerTask() {
      public void run() {
        if (num_frames_pular_ > 0) {
          --num_frames_pular_;
          return;
        }
        queueEvent(new Runnable() {
          public void run() {
            renderer_.ChamaTimer();
          }
        });
        if ((num_frames_pular_ == 0) || desenha_proxima_) {
          //Log.d(TAG, "requestRender");
          //postInvalidate();
          requestRender();
          desenha_proxima_ = false;
        } else {
          Log.d(TAG, "num_frames_pular_: " + num_frames_pular_);
          --num_frames_pular_;
        }
      }
    }, 0, TEMPO_ENTRE_NOTIFICACOES_MS);
  }

  public void ReportaUltimaRenderizacao(int tempo_ms) {
    ultima_renderizacao_ms_ = tempo_ms;
    num_frames_pular_ = (tempo_ms / TEMPO_ENTRE_NOTIFICACOES_MS) - 1;
    if (num_frames_pular_ > 1) {
      desenha_proxima_ = true;
      Log.d(TAG, "Pulando: " + num_frames_pular_);
    } else {
      num_frames_pular_ = 0;
    }
  }

  private TabuleiroRenderer renderer_;
  private GestureDetector detectorEventos_;
  private PressureDetector detectorPressao_;
  private ScaleGestureDetector detectorEscala_;
  private RotationGestureDetector detectorRotacao_;
  private TranslationGestureDetector detectorTranslacao_;
  private SensorManager gerenteSensores_ = (SensorManager)getContext().getSystemService(Context.SENSOR_SERVICE);
  private Sensor sensor_ = gerenteSensores_.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
  private java.util.Timer timer_;
  private static int ESTADO_OCIOSO = 1, ESTADO_MULTITOQUE_2 = 2, ESTADO_MULTITOQUE_3 = 3;
  private int estado_ = ESTADO_OCIOSO;
  private int ultima_renderizacao_ms_ = 0;
  private int num_frames_pular_ = 0;
  private boolean desenha_proxima_ = true;
  private static final int TEMPO_ENTRE_NOTIFICACOES_MS = nativeTempoEntreNotificacoes();

  private static native int nativeTempoEntreNotificacoes();
  private static native void nativePause();
  private static native void nativeResume();
}

// Renderizador do tabuleiro. Responsavel pelo timer que atualiza o tabuleiro.
class TabuleiroRenderer
    implements GLSurfaceView.Renderer,
               GestureDetector.OnGestureListener,
               GestureDetector.OnDoubleTapListener,
               ScaleGestureDetector.OnScaleGestureListener,
               RotationGestureDetector.RotationListener,
               TranslationGestureDetector.TranslationListener,
               SensorEventListener,
               PressureDetector.PressureListener {

  public static final String TAG = "TabuleiroRenderer";

  public TabuleiroRenderer(Activity activity, GLSurfaceView view, Resources resources, int orientacao_padrao) {
    resources_ = resources;
    parent_ = view;
    orientacao_padrao_ = orientacao_padrao;
    activity_ = activity;
  }

  /** Manda uma mensagem para a thread de UI. Chamado do codigo nativo, qualquer mudanca aqui deve ser refletida la. */
  public void mensagem(final boolean erro, final String mensagem, final long dados_volta) {
    //Log.d(TAG, "mensagem: " + mensagem);
    activity_.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity_);
        builder.setTitle(erro ? "Erro" : "Info").setMessage(mensagem);
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int id) {
            dialog.dismiss();
            nativeMessage(dados_volta);
          }
        });
        AlertDialog caixa = builder.create();
        caixa.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
        caixa.show();
      }
    });
  }

  // @param dados_volta eh um ponteiro para void* passado no callback do ok de volta ao codigo nativo.
  public void abreDialogoSalvarTabuleiro(final long dados_volta) {
    //Log.d(TAG, "abreDialogoSalvarTabuleiro: ");
    activity_.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity_);
        builder.setTitle("Salvar Tabuleiro");
        LayoutInflater inflater = activity_.getLayoutInflater();
        View view = inflater.inflate(R.layout.dialogo_salvar_tabuleiro, null);
        final EditText edit_nome = (EditText)view.findViewById(R.id.nome_arquivo);
        // Termina a janela de dialogo.
        builder.setView(view)
          .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              if (edit_nome == null) {
                Log.e(TAG, "nome arquivo == null");
                return;
              }
              nativeSaveBoardName(dados_volta, edit_nome.getText().toString());
              dialog.dismiss();
            }
          })
          .setNegativeButton("Cancela", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              nativeSaveBoardName(dados_volta, "");
              dialog.dismiss();
            }
          }
        );
        AlertDialog caixa = builder.create();
        caixa.show();
      }
    });
  }

  // @param dados_volta eh um ponteiro para void* passado no callback do ok de volta ao codigo nativo.
  public void abreDialogoItemsLista(
      final String[] lista, final long dados_volta) {
    //Log.d(TAG, "abreDialogoItemsLista: ");
    activity_.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity_);
        builder.setTitle("Escolha");
        LayoutInflater inflater = activity_.getLayoutInflater();
        View view = inflater.inflate(R.layout.dialogo_abrir_tabuleiro, null);
        final Spinner spinner = (Spinner)view.findViewById(R.id.spinner_abrir);
        if (spinner == null) {
          Log.e(TAG, "spinner== null");
          return;
        }
        final ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(activity_, android.R.layout.simple_spinner_item) {
        };
        if (lista != null) {
          adapter.addAll(lista);
        }
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        // Termina a janela de dialogo.
        builder.setView(view)
          .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              int posicao = spinner.getSelectedItemPosition();
              if (posicao == spinner.INVALID_POSITION || posicao >= lista.length) {
                nativeOpenItemList(dados_volta, false, -1);
              }
              Log.e(TAG, "aqui: " + (String)spinner.getSelectedItem());
              nativeOpenItemList(dados_volta, true, posicao);
              dialog.dismiss();
            }
          })
          .setNegativeButton("Cancela", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              nativeOpenItemList(dados_volta, false, -1);
              dialog.dismiss();
            }
          }
        );
        AlertDialog caixa = builder.create();
        caixa.show();
      }
    });
  }

  // @param dados_volta eh um ponteiro para void* passado no callback do ok de volta ao codigo nativo.
  public void abreDialogoAbrirTabuleiro(
      final String[] tab_estaticos, final String[] tab_dinamicos, final long dados_volta) {
    //Log.d(TAG, "abreDialogoAbrirTabuleiro: ");
    activity_.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity_);
        builder.setTitle("Abrir");
        LayoutInflater inflater = activity_.getLayoutInflater();
        View view = inflater.inflate(R.layout.dialogo_abrir_tabuleiro, null);
        final Spinner spinner = (Spinner)view.findViewById(R.id.spinner_abrir);
        if (spinner == null) {
          Log.e(TAG, "spinner== null");
          return;
        }
        final ArrayAdapter<String> adapter =
            new ArrayAdapter<String>(activity_, android.R.layout.simple_spinner_item) {
          public boolean isEnabled(int position) {
            return !((position == 0) || (position == tab_estaticos.length + 1));
          }
          public View getView(int position, View convertView, ViewGroup parent) {
            View view = super.getView(position, convertView, parent);
            view.setEnabled(isEnabled(position));
            return view;
          }
        };
        if (tab_estaticos != null) {
          adapter.add("Estáticos");
          adapter.addAll(tab_estaticos);
        }
        //v.setSelectable(false);
        if (tab_dinamicos != null) {
          adapter.add("Salvos");
          adapter.addAll(tab_dinamicos);
        }
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        // Termina a janela de dialogo.
        builder.setView(view)
          .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              boolean tem_estaticos = tab_estaticos != null;
              boolean tem_dinamicos = tab_dinamicos != null;
              int tab_estatico_len = tem_estaticos ? tab_estaticos.length : -1;
              int tab_dinamico_len = tem_dinamicos ? tab_estaticos.length : -1;
              int posicao = spinner.getSelectedItemPosition();
              if (posicao == spinner.INVALID_POSITION || posicao == 0 ||
                  (posicao == tab_estatico_len + 1) ||
                  posicao >= (tab_estatico_len + tab_dinamico_len + 2)) {
                nativeOpenBoardName(dados_volta, "", false);
              }
              boolean estatico = posicao < tab_estatico_len + 1;
              nativeOpenBoardName(dados_volta, (String)spinner.getSelectedItem(), estatico);
              dialog.dismiss();
            }
          })
          .setNegativeButton("Cancela", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              nativeOpenBoardName(dados_volta, "", false);
              dialog.dismiss();
            }
          }
        );
        AlertDialog caixa = builder.create();
        caixa.show();
      }
    });
  }

  /** Abre uma janela de dialogo na thread de UI. Chamado do codigo nativo, qualquer mudanca aqui deve ser refletida la. */
  public void abreDialogoEntidade(final byte[] mensagem) {
    Log.d(TAG, "abreDialogoEntidade");
    Wire wire = new Wire();
    final EntidadeProto proto;
    try {
      proto = wire.parseFrom(mensagem, EntidadeProto.class);
    } catch (Exception e) {
      Log.e(TAG, "Falha deserializando mensagem: " + e.getMessage());
      return;
    }
    activity_.runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity_);
        builder.setTitle("Entidade");
        LayoutInflater inflater = activity_.getLayoutInflater();
        View view = inflater.inflate(R.layout.dialogo_entidade, null);
        // Preenche campos.
        final EditText max_pv = (EditText)view.findViewById(R.id.max_pontos_vida);
        if (max_pv == null) {
          Log.e(TAG, "max_pv == null");
          return;
        }
        final EditText pv = (EditText)view.findViewById(R.id.pontos_vida);
        if (pv == null) {
          Log.e(TAG, "pv == null");
          return;
        }
        final Spinner tv = (Spinner)view.findViewById(R.id.tipo_visao);
        if (tv == null) {
          Log.e(TAG, "tv == null");
          return;
        }
        final EditText av = (EditText)view.findViewById(R.id.raio_visao_escuro);
        if (av == null) {
          Log.e(TAG, "av == null");
          return;
        }
        final NumberPicker raio_luz = (NumberPicker)view.findViewById(R.id.raio_luz_picker);
        if (raio_luz == null) {
          Log.e(TAG, "raio_luz == null");
          return;
        }
        final EditText eventos = (EditText)view.findViewById(R.id.eventos);
        if (eventos == null) {
          Log.e(TAG, "eventos == null");
          return;
        }

        max_pv.setText(String.valueOf(proto.max_pontos_vida));
        pv.setText(String.valueOf(proto.pontos_vida));
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(
            activity_, R.array.tipo_visao_array, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        tv.setAdapter(adapter);
        tv.setSelection(proto.tipo_visao != null ? proto.tipo_visao.getValue() : TipoVisao.VISAO_NORMAL.getValue());
        final int num_valores_raios = 300;
        String[] valores_raio = new String[num_valores_raios];
        for (int i = 0; i < num_valores_raios; ++i) {
          valores_raio[i] = String.valueOf(i * 1.5f);
        }
        raio_luz.setMaxValue(num_valores_raios);
        raio_luz.setMinValue(0);
        raio_luz.setWrapSelectorWheel(false);
        raio_luz.setDisplayedValues(valores_raio);
        raio_luz.setDescendantFocusability(NumberPicker.FOCUS_BLOCK_DESCENDANTS);
        float val_luz = 0.0f;
        if (proto.luz != null) {
          if (proto.luz.raio_m == null) {
            val_luz = 6.0f;  // raio padrao de luz: 4 quadrados.
          } else {
            val_luz = proto.luz.raio_m;
          }
        }
        int indice_val_luz = (int)(val_luz / 1.5f);
        if (indice_val_luz > (num_valores_raios - 1)) {
          indice_val_luz = num_valores_raios - 1;
        }
        raio_luz.setValue(indice_val_luz);
        av.setText(String.valueOf(proto.alcance_visao == null ? 0 : proto.alcance_visao));
        String evento_str = new String();
        for (EntidadeProto.Evento e : proto.evento) {
          evento_str += e.descricao;
          if (e.complemento != null) {
            evento_str += " (" + String.valueOf(e.complemento) + ")";
          }
          evento_str += ": " + String.valueOf(e.rodadas) + "\n";
        }
        eventos.setText(evento_str);

        // Termina a janela de dialogo.
        builder.setView(view)
          .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              try {
                // Hack: eventos sera todos colocados em uma string e decodificados no codigo nativo.
                Vector<EntidadeProto.Evento> evento_hack = new Vector<EntidadeProto.Evento>();
                evento_hack.add(new EntidadeProto.Evento(0, eventos.getText().toString(), 0, 0));
                // Converte a visao para 18m se for no escuro e o valor for 0.
                float alcance = Float.parseFloat(av.getText().toString());
                if ((TipoVisao.values()[tv.getSelectedItemPosition()] == TipoVisao.VISAO_ESCURO) && alcance == 0) {
                  alcance = 18.0f;
                }
                EntidadeProto proto_modificado = new EntidadeProto.Builder()
                    .id(proto.id)
                    .max_pontos_vida(Integer.parseInt(max_pv.getText().toString()))
                    .pontos_vida(Integer.parseInt(pv.getText().toString()))
                    .tipo_visao(TipoVisao.values()[tv.getSelectedItemPosition()])
                    .luz(new IluminacaoPontual(null, new Float(raio_luz.getValue() * 1.5f), null))
                    .alcance_visao(new Float(alcance))
                    .evento(evento_hack)
                    .build();
                Log.d(TAG, "OK proto: " + proto_modificado.toString());
                nativeUpdateEntity(proto_modificado.toByteArray());
                dialog.dismiss();
              } catch (Exception e) {
              }
            }
          })
          .setNegativeButton("Cancela", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
              dialog.dismiss();
            }
          }
        );
        AlertDialog caixa = builder.create();
        caixa.show();
      }
    });
  }

  @Override
  public void onSurfaceCreated(GL10 unused, EGLConfig config) {
    Log.d(TAG, "===============onSurfaceCreated");
    nativeInitGl();
    contexto_criado_ = true;
  }

  @Override
  public void onSurfaceChanged(GL10 unused, int w, int h) {
    Log.d(TAG, "===============onSurfaceChanged " + w + ", " + h);
    nativeResize(w, h);
  }

  public void habilitaSensores(boolean hab) {
    lerGiroscopio_ = hab;
  }

  /** Remove os eventos duplicados de um tipo ate que Liberado seja encontrado. */
  private void removeEventosDuplicados(int tipo, Vector<Evento> eventos) {
    // Remove os eventos consecutivos de movimento.
    Vector<Evento> eventosSemDuplicados = new Vector<Evento>();
    Evento ultimo = null;
    for (Evento evento :  eventos) {
      if (evento.tipo() == tipo) {
        ultimo = evento;
      } else if (evento.tipo() == Evento.LIBERADO) {
        if (ultimo != null) {
          // Adiciona o ultimo movimento.
          eventosSemDuplicados.add(ultimo);
          ultimo = null;
        }
        eventosSemDuplicados.add(evento);
      } else {
        eventosSemDuplicados.add(evento);
      }
    }
    // Poe o ultimo movimento se ele foi o ultimo evento.
    if (ultimo != null) {
      // Adiciona o ultimo movimento.
      eventosSemDuplicados.add(ultimo);
      ultimo = null;
    }
    eventos.clear();
    eventos.addAll(eventosSemDuplicados);
  }

  /** Toda atualizacao eh feita daqui para acontecer na mesma thread que o grafico. */
  @Override
  public void onDrawFrame(GL10 unused) {
    //Log.d(TAG, "===============DrawFrame");
    ((TabuleiroSurfaceView)parent_).ReportaUltimaRenderizacao(nativeRender());
    //Log.d(TAG, "===============DrawFrame ended");
  }

  /** Deve ser chamado na UI thread. */
  public void ChamaTimer() {
    //Log.d(TAG, "===============ChamaTimer");
    if (!contexto_criado_) {
      return;
    }
    //Log.d(TAG, "ChamaTimer");
    nativeTimer();
    //Log.d(TAG, "Tam Evento Antes: " + eventos_.size());
    Vector<Evento> eventos;
    synchronized (eventos_) {
      eventos = new Vector<Evento>(eventos_);
      eventos_.clear();
    }
    removeEventosDuplicados(Evento.MOVIMENTO, eventos);
    removeEventosDuplicados(Evento.DETALHAMENTO, eventos);

    //Log.d(TAG, "Tam Evento Depois: " + eventos.size());
    for (Evento evento :  eventos) {
      //Log.d(TAG, "Evento: " + evento.toString());
      switch (evento.tipo()) {
        case Evento.TRANSLACAO:
          nativeTranslation(evento.x(), evento.y());
          break;
        case Evento.ESCALA:
          nativeScale(evento.escala());
          break;
        case Evento.ROTACAO:
          nativeRotation(evento.rotacao());
          break;
        case Evento.CLIQUE:
          nativeTouchPressed(false  /*toggle*/, evento.x(), evento.y());
          nativeTouchReleased();
          break;
        case Evento.CLIQUE_ALTERNANTE:
          nativeTouchPressed(true  /*toggle*/, evento.x(), evento.y());
          nativeTouchReleased();
          break;
        case Evento.CLIQUE_DUPLO:
          nativeDoubleClick(evento.x(), evento.y());
          break;
        case Evento.DETALHAMENTO:
          nativeHover(evento.x(), evento.y());
          break;
        case Evento.PRESSIONADO:
          nativeTouchPressed(false  /*toggle*/, evento.x(), evento.y());
          break;
        case Evento.LIBERADO:
          nativeTouchReleased();
          break;
        case Evento.MOVIMENTO:
          nativeTouchMoved(evento.x(), evento.y());
          break;
        case Evento.INCLINACAO:
          nativeTilt(-evento.delta());
          break;
        case Evento.TECLADO:
          nativeKeyboard(evento.tecla(), evento.modificadores());
          break;
        case Evento.ACAO:
          nativeAction(false, evento.x(), evento.y());
          break;
        case Evento.ACAO_SINALIZACAO:
          nativeAction(true, evento.x(), evento.y());
          break;
        case Evento.TOQUE_DUPLO:
          nativeDoubleTouchPressed(evento.x(), evento.y(), evento.x2(), evento.y2());
        default:
      }
    }

  }

  // Detector de eventos.
  @Override
  public boolean onDown(MotionEvent event) {
    //Log.d(TAG, "Down");
    return true;
  }

  // Algum ponteiro terminou.
  public boolean onUp(MotionEvent event) {
    //Log.d(TAG, "Up");
    eventos_.add(Evento.Liberado());
    carregando_ = false;
    return true;
  }

  @Override
  public boolean onFling(MotionEvent event1, MotionEvent event2,
      float velocityX, float velocityY) {
    //Log.d(TAG, "Fling");
    return true;
  }

  private int getParentHeight() {
    return ((TabuleiroActivity)activity_).fixed_size_ ? 768 : (int)parent_.getHeight();
  }

  @Override
  public void onLongPress(MotionEvent event) {
    //Log.d(TAG, "LongPress");
    eventos_.add(Evento.Detalhamento((int)event.getX(), (int)(getParentHeight() - event.getY())));
  }

  @Override
  public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
    //Log.d(TAG, "Scroll");
    if (!carregando_) {
      eventos_.add(Evento.Pressionado((int)e1.getX(), (int)(getParentHeight() - e1.getY())));
      carregando_ = true;
    }
    eventos_.add(Evento.Movimento((int)e2.getX(), (int)(getParentHeight() - e2.getY())));
    return true;
  }

  @Override
  public void onShowPress(MotionEvent event) {
    //Log.d(TAG, "ShowPress");
    eventos_.add(Evento.Toque((int)event.getX(), (int)(getParentHeight() - event.getY())));
  }

  @Override
  public boolean onSingleTapUp(MotionEvent event) {
    //Log.d(TAG, "TapUp");
    return true;
  }

  @Override
  public boolean onDoubleTap(MotionEvent event) {
    //Log.d(TAG, "DoubleTap");
    int x = (int)event.getX();
    int y = (int)(getParentHeight() - event.getY());
    if (((metaTeclas_ & META_SHIFT_ESQUERDO) != 0) || ((metaTeclas_ & META_SHIFT_DIREITO) != 0)) {
      eventos_.add(Evento.Detalhamento(x, y));
    } else {
      eventos_.add(Evento.DuploClique(x, y));
    }
    return true;
  }

  @Override
  public boolean onDoubleTapEvent(MotionEvent event) {
    //Log.d(TAG, "DoubleTapEvent");
    return true;
  }

  @Override
  public boolean onSingleTapConfirmed(MotionEvent event) {
    Log.d(TAG, "SingleTapConfirmed: " + event.toString());
    int x = (int)event.getX();
    int y = (int)(getParentHeight() - event.getY());
    if (metaTeclas_ == 0) {
      eventos_.add(Evento.Clique(x, y));
    } else if (((metaTeclas_ & META_CTRL_ESQUERDO) != 0) || ((metaTeclas_ & META_CTRL_DIREITO) != 0)) {
      eventos_.add(Evento.CliqueAlternante(x, y));
    } else if ((metaTeclas_ & META_ALT_ESQUERDO) != 0) {
      eventos_.add(Evento.Acao(x, y));
    } else if ((metaTeclas_ & META_ALT_DIREITO) != 0) {
      if ((metaTeclas_ & META_SHIFT_DIREITO) != 0) {
        // Hack para teclados sem alt esquerdo.
        eventos_.add(Evento.Acao(x, y));
      } else {
        eventos_.add(Evento.AcaoSinalizacao(x, y));
      }
    }
    return true;
  }

  // Detector de escala.
  @Override
  public boolean onScale(ScaleGestureDetector detector) {
    //Log.d(TAG, "Scale");
    eventos_.add(Evento.Escala(detector.getScaleFactor()));
    return true;
  }

  @Override
  public boolean onScaleBegin(ScaleGestureDetector detector) {
    //Log.d(TAG, "ScaleBegin");
    return true;
  }

  @Override
  public void onScaleEnd(ScaleGestureDetector detector) {
    //Log.d(TAG, "ScaleEnd");
  }

  // Detector rotacao.
  @Override
  public void onRotate(float delta) {
    if (delta == 0.0f) {
      return;
    }
    //Log.d(TAG, "Rotate");
    eventos_.add(Evento.Rotacao(delta));
  }

  // Inicio da pinca.
  @Override
  public void onDoubleTouchPressed(int x1, int y1, int x2, int y2) {
    eventos_.add(Evento.ToqueDuplo(x1, (int)(getParentHeight() - y1), x2, (int)(getParentHeight() - y2)));
  }

  // Detector de translacao.
  @Override
  public void onTranslateBegin(int x, int y) {
    //Log.d(TAG, "TranslationBegin");
    eventos_.add(Evento.Translacao(x, (int)(getParentHeight() - y)));
  }

  @Override
  public void onTranslate(int x, int y) {
    //Log.d(TAG, "Translation");
    eventos_.add(Evento.Movimento(x, (int)(getParentHeight() - y)));
  }

  @Override
  public void onTranslateEnd() {
    //Log.d(TAG, "TranslationEnd");
    eventos_.add(Evento.Liberado());
  }

  // Sensores.
  @Override
  public void onSensorChanged(SensorEvent se) {
    if (!lerGiroscopio_) {
      return;
    }
    //Log.d(TAG, "onSensorChanged: outras");
    // Detectar landscape ou retrato para saber se le o y ou x.
    float val;
    if (orientacao_padrao_ == android.content.res.Configuration.ORIENTATION_LANDSCAPE) {
      if (resources_.getConfiguration().orientation == android.content.res.Configuration.ORIENTATION_LANDSCAPE) {
        val = se.values[0];
      } else {
        val = se.values[1];
      }
    } else {
      // Orientacao padrao PORTRAIT.
      if (resources_.getConfiguration().orientation == android.content.res.Configuration.ORIENTATION_LANDSCAPE) {
        val = -se.values[1];
      } else {
        val = se.values[0];
      }
    }
    eventos_.add(Evento.Inclinacao(-val));
  }

  @Override
  public void onAccuracyChanged(Sensor sensor, int accuracy) {
  }

  // Detector de pressao.
  @Override
  public void onPressure(int x, int y) {
    //Log.d(TAG, "onPressure");
    eventos_.add(Evento.Detalhamento(x, (int)(getParentHeight() - y)));
  }

  // Tres toques: acao.
  public void onActionTouch(final MotionEvent e) {
    int somaX = 0, somaY = 0;
    for (int i = 0; i < e.getPointerCount(); ++i) {
      somaX += e.getX(i);
      somaY += e.getY(i);
    }
    somaX = somaX / e.getPointerCount();
    somaY = somaY / e.getPointerCount();

    eventos_.add(Evento.Acao(somaX, (int)(getParentHeight() - somaY)));
  }

  // Meta teclas.
  public boolean onKeyDown(int keyCode, KeyEvent event) {
    switch (keyCode) {
      case android.view.KeyEvent.KEYCODE_ALT_LEFT:
        metaTeclas_ |= META_ALT_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_ALT_RIGHT:
        metaTeclas_ |= META_ALT_DIREITO;
        return true;
      case android.view.KeyEvent.KEYCODE_CTRL_LEFT:
        metaTeclas_ |= META_CTRL_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_CTRL_RIGHT:
        metaTeclas_ |= META_CTRL_DIREITO;
        return true;
      case android.view.KeyEvent.KEYCODE_SHIFT_LEFT:
        metaTeclas_ |= META_SHIFT_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_SHIFT_RIGHT:
        metaTeclas_ |= META_SHIFT_DIREITO;
        return true;
      default:
        return false;
    }
  }

  public boolean onKeyUp(int keyCode, KeyEvent event) {
    //Log.d(TAG, "Teclado");
    switch (keyCode) {
      case android.view.KeyEvent.KEYCODE_ALT_LEFT:
        metaTeclas_ &= ~META_ALT_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_ALT_RIGHT:
        metaTeclas_ &= ~META_ALT_DIREITO;
        return true;
      case android.view.KeyEvent.KEYCODE_CTRL_LEFT:
        metaTeclas_ &= ~META_CTRL_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_CTRL_RIGHT:
        metaTeclas_ &= ~META_CTRL_DIREITO;
        return true;
      case android.view.KeyEvent.KEYCODE_SHIFT_LEFT:
        metaTeclas_ &= ~META_SHIFT_ESQUERDO;
        return true;
      case android.view.KeyEvent.KEYCODE_SHIFT_RIGHT:
        metaTeclas_ &= ~META_SHIFT_DIREITO;
        return true;
      default:
        // TODO modificadores.
        eventos_.add(Evento.Teclado(keyCode, event.isShiftPressed(), event.isCtrlPressed(), event.isAltPressed()));
        return true;
    }
  }

  private static native void nativeInitGl();
  private static native void nativeResize(int w, int h);
  private static native int nativeRender();
  private native void nativeTimer();
  private static native void nativeDoubleClick(int x, int y);
  private static native void nativeTouchPressed(boolean toggle, int x, int y);
  private static native void nativeDoubleTouchPressed(int x1, int y1, int x2, int y2);
  private static native void nativeTouchMoved(int x, int y);
  private static native void nativeAction(boolean signal, int x, int y);
  private static native void nativeTouchReleased();
  private static native void nativeHover(int x, int y);
  private static native void nativeScale(float s);
  private static native void nativeRotation(float r);
  private static native void nativeTranslation(int x, int y);
  private static native void nativeTilt(float delta);
  private static native void nativeKeyboard(int tecla, int modificadores);
  private static native void nativeMetaKeyboard(boolean pressionado, int tecla);
  private static native void nativeMessage(long dados_volta);
  private static native void nativeSaveBoardName(long dados_volta, String nome);
  private static native void nativeOpenBoardName(long dados_volta, String nome, boolean estatico);
  private static native void nativeOpenItemList(long dados_volta, boolean ok, int indice);
  private static native void nativeUpdateEntity(byte[] mensagem);

  private Activity activity_;
  private GLSurfaceView parent_;
  private Vector<Evento> eventos_ = new Vector<Evento>();
  private boolean carregando_ = false;
  private boolean contexto_criado_ = false;
  private Resources resources_;
  private boolean lerGiroscopio_ = false;
  private int metaTeclas_ = 0;
  private int orientacao_padrao_;  // ORIENTATION_PORTRAIT ou ORIENTATION_LANDSCAPE.
  private static final int META_ALT_ESQUERDO = 0x1;
  private static final int META_ALT_DIREITO  = 0x2;
  private static final int META_CTRL_ESQUERDO = 0x4;
  private static final int META_CTRL_DIREITO  = 0x8;
  private static final int META_SHIFT_ESQUERDO = 0x10;
  private static final int META_SHIFT_DIREITO  = 0x20;
}

// Copiado de:
// https://code.google.com/p/osmdroid/source/browse/trunk/
// OpenStreetMapViewer/src/org/osmdroid/RotationGestureDetector.java?r=1186
class RotationGestureDetector {
  public interface RotationListener {
    // Angulo em radianos.
    public void onDoubleTouchPressed(int x1, int y1, int x2, int y2);
    public void onRotate(float deltaAngle);
  }

  protected float mRotation;
  private RotationListener mListener;

  public RotationGestureDetector(RotationListener listener) {
    mListener = listener;
  }

  private float rotation(MotionEvent event) {
    double delta_x = (event.getX(0) - event.getX(1));
    double delta_y = (event.getY(0) - event.getY(1));
    double radians = Math.atan2(delta_y, delta_x);
    return (float)radians;
  }

  public void onTouch(MotionEvent e) {
    if (e.getPointerCount() != 2) {
      return;
    }

    if (e.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN) {
      mRotation = rotation(e);
      mListener.onDoubleTouchPressed((int)e.getX(0), (int)e.getY(0), (int)e.getX(1), (int)e.getY(1));
    }

    float rotation = rotation(e);
    float delta = rotation - mRotation;
    mRotation += delta;
    mListener.onRotate(delta);
  }
}

// Lanca eventos quando a pressao passar de um determinado valor.
class PressureDetector {
  public interface PressureListener {
    public void onPressure(int x, int y);
  }
  public PressureDetector(PressureListener ouvinte) {
    ouvinte_ = ouvinte;
  }

  public void onTouch(MotionEvent e) {
    //Log.d("TabuleiroRenderer", "Pressure: " + e.getPressure());
    if (e.getPressure() > 0.70f) {
      ouvinte_.onPressure((int)e.getX(), (int)e.getY());
    }
  }

  PressureListener ouvinte_;
}

// Translacao com 2+ dedos.
class TranslationGestureDetector {
  public interface TranslationListener {
    public void onTranslateBegin(int x, int y);
    public void onTranslate(int x, int y);
    public void onTranslateEnd();
  }

  public TranslationGestureDetector(TranslationListener ouvinte) {
    ouvinte_ = ouvinte;
  }

  public void onTouch(MotionEvent e) {
    if (e.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN) {
      ouvinte_.onTranslateBegin(X(e), Y(e));
      return;
    } else if (e.getActionMasked() == MotionEvent.ACTION_POINTER_UP) {
      ouvinte_.onTranslateEnd();
      return;
    }
    ouvinte_.onTranslate(X(e), Y(e));
  }

  private int X(MotionEvent e) {
    int soma = 0;
    for (int i = 0; i < e.getPointerCount(); ++i) {
      soma += e.getX(i);
    }
    return soma / e.getPointerCount();
  }

  private int Y(MotionEvent e) {
    int soma = 0;
    for (int i = 0; i < e.getPointerCount(); ++i) {
      soma += e.getY(i);
    }
    return soma / e.getPointerCount();
  }

  private TranslationListener ouvinte_;
}

/** Os tipos de eventos tratados pelo tabuleiro. */
class Evento {
  public static final int CLIQUE = 1;
  public static final int CLIQUE_ALTERNANTE = 2;
  public static final int CLIQUE_DUPLO = 3;
  public static final int ESCALA = 4;
  public static final int TOQUE = 5;
  public static final int CARREGAR = 6;
  public static final int DETALHAMENTO = 7;
  public static final int PRESSIONADO = 8;
  public static final int LIBERADO = 9;
  public static final int MOVIMENTO = 10;
  public static final int ROTACAO = 11;
  public static final int TRANSLACAO = 12;
  public static final int INCLINACAO = 13;
  public static final int TECLADO = 14;
  public static final int ACAO = 15;
  public static final int ACAO_SINALIZACAO = 16;
  public static final int TOQUE_DUPLO = 17;  // tipo pinca.

  public static Evento Teclado(int tecla, boolean shift, boolean ctrl, boolean alt) {
    Evento evento = new Evento(TECLADO);
    evento.tecla_ = teclaNativa(tecla);
    evento.modificadores_ = modificadoresNativos(shift, ctrl, alt);
    return evento;
  }

  public static Evento Liberado() {
    return new Evento(LIBERADO);
  }

  public static Evento Acao(int x,  int y) {
    Evento evento = new Evento(ACAO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento AcaoSinalizacao(int x,  int y) {
    Evento evento = new Evento(ACAO_SINALIZACAO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento Inclinacao(float delta) {
    Evento evento = new Evento(INCLINACAO);
    evento.delta_ = delta;
    return evento;
  }

  public static Evento Translacao(int x,  int y) {
    Evento evento = new Evento(TRANSLACAO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento Escala(float escala) {
    Evento evento = new Evento(ESCALA);
    evento.escala_ = escala;
    return evento;
  }

  public static Evento Rotacao(float rotacao) {
    Evento evento = new Evento(ROTACAO);
    evento.rotacao_ = rotacao;
    return evento;
  }

  public static Evento Pressionado(int x,  int y) {
    Evento evento = new Evento(PRESSIONADO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento ToqueDuplo(int x1,  int y1, int x2, int y2) {
    Evento evento = new Evento(TOQUE_DUPLO);
    evento.x_ = x1;
    evento.y_ = y1;
    evento.x2_ = x2;
    evento.y2_ = y2;

    return evento;
  }

  public static Evento Movimento(int x,  int y) {
    Evento evento = new Evento(MOVIMENTO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento Clique(int x,  int y) {
    Evento evento = new Evento(CLIQUE);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento CliqueAlternante(int x,  int y) {
    Evento evento = new Evento(CLIQUE_ALTERNANTE);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento DuploClique(int x,  int y) {
    Evento evento = new Evento(CLIQUE_DUPLO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento Toque(int x,  int y) {
    Evento evento = new Evento(TOQUE);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  public static Evento Detalhamento(int x,  int y) {
    Evento evento = new Evento(DETALHAMENTO);
    evento.x_ = x;
    evento.y_ = y;
    return evento;
  }

  private Evento(int tipo) {
    tipo_ = tipo;
  }

  public String toString() {
    return "Tipo: " + tipoString() + ", escala: " + escala_ + ", rotacao: " + rotacao_ +
                                     ", x:" + x_ + ", y: " + y_ + ", delta: " + delta_ +
                                     ", tecla: " + tecla_ + ", modificadores: " + modificadores_;
  }

  private String tipoString() {
    switch (tipo_) {
      case CLIQUE: return "CLIQUE";
      case CLIQUE_ALTERNANTE: return "CLIQUE_ALTERNANTE";
      case CLIQUE_DUPLO: return "CLIQUE_DUPLO";
      case ESCALA: return "ESCALA";
      case TOQUE: return "TOQUE";
      case CARREGAR: return "CARREGAR";
      case DETALHAMENTO: return "DETALHAMENTO";
      case PRESSIONADO: return "PRESSIONADO";
      case LIBERADO: return "LIBERADO";
      case MOVIMENTO: return "MOVIMENTO";
      case ROTACAO: return "ROTACAO";
      case TRANSLACAO: return "TRANSLACAO";
      case INCLINACAO: return "INCLINACAO";
      case TECLADO: return "TECLADO";
      case ACAO: return "ACAO";
      case ACAO_SINALIZACAO: return "ACAO_SINALIZACAO";
      case TOQUE_DUPLO: return "TOQUE_DUPLO";
      default: return "INVALIDO";
    }
  }

  // Transforma o keycode de java para nativo. Sao os mesmos do QT: http://qt-project.org/doc/qt-4.8/qt.html#Key-enum.
  public static int teclaNativa(int teclaJava) {
    switch (teclaJava) {
      case KeyEvent.KEYCODE_0:
      case KeyEvent.KEYCODE_1:
      case KeyEvent.KEYCODE_2:
      case KeyEvent.KEYCODE_3:
      case KeyEvent.KEYCODE_4:
      case KeyEvent.KEYCODE_5:
      case KeyEvent.KEYCODE_6:
      case KeyEvent.KEYCODE_7:
      case KeyEvent.KEYCODE_8:
      case KeyEvent.KEYCODE_9:
        return 0x30 + (teclaJava - KeyEvent.KEYCODE_0);
      case KeyEvent.KEYCODE_A:
      case KeyEvent.KEYCODE_B:
      case KeyEvent.KEYCODE_C:
      case KeyEvent.KEYCODE_D:
      case KeyEvent.KEYCODE_E:
      case KeyEvent.KEYCODE_F:
      case KeyEvent.KEYCODE_G:
      case KeyEvent.KEYCODE_H:
      case KeyEvent.KEYCODE_I:
      case KeyEvent.KEYCODE_J:
      case KeyEvent.KEYCODE_K:
      case KeyEvent.KEYCODE_L:
      case KeyEvent.KEYCODE_M:
      case KeyEvent.KEYCODE_N:
      case KeyEvent.KEYCODE_O:
      case KeyEvent.KEYCODE_P:
      case KeyEvent.KEYCODE_Q:
      case KeyEvent.KEYCODE_R:
      case KeyEvent.KEYCODE_S:
      case KeyEvent.KEYCODE_T:
      case KeyEvent.KEYCODE_U:
      case KeyEvent.KEYCODE_V:
      case KeyEvent.KEYCODE_W:
      case KeyEvent.KEYCODE_X:
      case KeyEvent.KEYCODE_Y:
      case KeyEvent.KEYCODE_Z:
        return 0x41 + (teclaJava - KeyEvent.KEYCODE_A);
      case KeyEvent.KEYCODE_SPACE: return 0x20;
      case KeyEvent.KEYCODE_ENTER: return 0x01000004;
      case KeyEvent.KEYCODE_ESCAPE: return 0x01000000;
      case KeyEvent.KEYCODE_DPAD_LEFT: return 0x01000012;
      case KeyEvent.KEYCODE_DPAD_UP: return 0x01000013;
      case KeyEvent.KEYCODE_DPAD_RIGHT: return 0x01000014;
      case KeyEvent.KEYCODE_DPAD_DOWN: return 0x01000015;
      case KeyEvent.KEYCODE_DEL: return 0x01000003;
      case KeyEvent.KEYCODE_TAB: return 0x01000001;
      case KeyEvent.KEYCODE_ALT_LEFT: return 0x01100007;
      case KeyEvent.KEYCODE_ALT_RIGHT: return 0x01100008;
      default: return -1;
    }
  }

  // Valores do QT: http://qt-project.org/doc/qt-4.8/qt.html#KeyboardModifier-enum.
  private static int modificadoresNativos(boolean shift, boolean ctrl, boolean alt) {
    int ret = 0;
    if (shift) {
      ret |= 0x02000000;
    }
    if (ctrl) {
      ret |= 0x04000000;
    }
    if (alt) {
      ret |= 0x08000000;
    }
    return ret;
  }

  public int tipo() { return tipo_; }
  public float escala() { return escala_; }
  public float rotacao() { return rotacao_; }
  public int x() { return x_; }
  public int y() { return y_; }
  public int x2() { return x2_; }
  public int y2() { return y2_; }
  public int nx() { return nx_; }
  public int ny() { return ny_; }
  public float delta() { return delta_; }
  public int tecla() { return tecla_; }
  public int modificadores() { return modificadores_; }

  private int tipo_;
  private float escala_;
  private float rotacao_;
  private int x_;
  private int y_;
  private int x2_;
  private int y2_;
  private int nx_;
  private int ny_;
  private float delta_;
  private int tecla_;  // modo nativo.
  private int modificadores_;  // modo nativo.
}
