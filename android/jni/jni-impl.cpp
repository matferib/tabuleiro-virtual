/** Wrapper JNI para o tabuleiro baseado no Tabuleiro. */
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include "ent/constantes.h"
#include "ent/util.h"
#include "gl/gl.h"

namespace {
int x_ = 0, y_ = 0;

// Tamanho do tabuleiro em quadrados.
int TamanhoX() {
  return 30;
}

int TamanhoY() {
  return 30;
}

float Aspecto() {
  return x_ / y_;
}

}  // namespace

namespace ent {

/** campo de visao vertical em graus. */
const double CAMPO_VERTICAL_GRAUS = 60.0;

/** altura inicial do olho. */
const double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
const double OLHO_ALTURA_MAXIMA = 45.0;
/** altura minima do olho. */
const double OLHO_ALTURA_MINIMA = 1.5;

/** raio (distancia) inicial do olho. */
const double OLHO_RAIO_INICIAL = 20.0;
/** raio maximo do olho. */
const double OLHO_RAIO_MAXIMO = 40.0;
/** raio minimo do olho. */
const double OLHO_RAIO_MINIMO = 1.5;

/** sensibilidade da rodela do mouse. */
const double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
const double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** expessura da linha do tabuleiro. */
const float EXPESSURA_LINHA = 0.2f;
const float EXPESSURA_LINHA_2 = EXPESSURA_LINHA / 2.0f;
/** velocidade do olho. */
const float VELOCIDADE_POR_EIXO = 0.1f;  // deslocamento em cada eixo (x, y, z) por chamada de atualizacao.

/** tamanho maximo da lista de eventos para desfazer. */
const unsigned int TAMANHO_MAXIMO_LISTA = 10;

/** Distancia minima entre pontos no desenho livre. */
const float DELTA_MINIMO_DESENHO_LIVRE = 0.2;

/** A Translacao e a rotacao de objetos so ocorre depois que houver essa distancia de pixels percorrida pelo mouse. */
const int DELTA_MINIMO_TRANSLACAO_ROTACAO = 5;

/** Os clipping planes. */
const double DISTANCIA_PLANO_CORTE_PROXIMO = 0.5;
const double DISTANCIA_PLANO_CORTE_DISTANTE = 500.0f;

void DesenhaQuadrado(unsigned int id, int linha, int coluna) {
  gl::CarregaNome(id);
  GLfloat cinza[] = { 0.5f, 0.5f, 0.5f, 1.0f };
  MudaCor(cinza);

  float tamanho_texel_h = 1.0f / TamanhoX();
  float tamanho_texel_v = 1.0f / TamanhoY();
  float tamanho_y_linha = TamanhoY() - linha;
  const float vertices_texel_ladrilho[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  const unsigned short indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, };
  const float vertices[] = {
    0.0f, 0.0f,
    TAMANHO_LADO_QUADRADO, 0.0f,
    TAMANHO_LADO_QUADRADO, TAMANHO_LADO_QUADRADO,
    0.0f, TAMANHO_LADO_QUADRADO,
  };
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void DesenhaTabuleiro() {
  gl::MatrizEscopo salva_matriz;
  double deltaX = -TamanhoX() * TAMANHO_LADO_QUADRADO;
  double deltaY = -TamanhoY() * TAMANHO_LADO_QUADRADO;
  gl::Normal(0, 0, 1.0f);
  gl::Translada(deltaX / 2.0f,
               deltaY / 2.0f,
               0.0f);
  int id = 0;
  // Desenha o chao mais pro fundo.
  // TODO transformar offsets em constantes.
  gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(2.0f, 20.0f);
  for (int y = 0; y < TamanhoY(); ++y) {
    for (int x = 0; x < TamanhoX(); ++x) {
      // desenha quadrado
      DesenhaQuadrado(id, y, x);
      // anda 1 quadrado direita
      gl::Translada(TAMANHO_LADO_QUADRADO, 0, 0);
      ++id;
    }
    // volta tudo esquerda e sobe 1 quadrado
    gl::Translada(deltaX, TAMANHO_LADO_QUADRADO, 0);
  }
}

void DesenhaGrade() {
  MudaCor(COR_PRETA);
  // Linhas verticais (S-N).
  const float tamanho_y_2 = (TamanhoY() / 2.0f) * TAMANHO_LADO_QUADRADO;
  const float tamanho_x_2 = (TamanhoX() / 2.0f) * TAMANHO_LADO_QUADRADO;
  const int x_2 = TamanhoX()  / 2;
  const int y_2 = TamanhoY() / 2;
  for (int i = -x_2; i <= x_2; ++i) {
    float x = i * TAMANHO_LADO_QUADRADO;
    gl::Retangulo(x - EXPESSURA_LINHA_2, -tamanho_y_2, x + EXPESSURA_LINHA_2, tamanho_y_2);
  }
  // Linhas horizontais (W-E).
  for (int i = -y_2; i <= y_2; ++i) {
    float y = i * TAMANHO_LADO_QUADRADO;
    gl::Retangulo(-tamanho_x_2, y - EXPESSURA_LINHA_2, tamanho_x_2, y + EXPESSURA_LINHA_2);
  }
}

}  // namespace ent

extern "C" {

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeInit(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeInit");
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::Desabilita(GL_BLEND);
  gl::Habilita(GL_CULL_FACE);
  gl::FaceNula(GL_BACK);
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeResize(JNIEnv* env, jobject thiz, jint w, jint h) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "resize w=%d h=%d", w, h);
  gl::Viewport(0, 0, (GLint)w, (GLint)h);
  x_ = w;
  y_ = h;
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeDone(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeDone");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeTogglePauseResume(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeTogglePauseResume");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativePause(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");
}

void Java_com_matferib_Tabuleiro_TabuleiroSurfaceView_nativeResume(JNIEnv* env) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeResume");
}

void Java_com_matferib_Tabuleiro_TabuleiroRenderer_nativeRender(JNIEnv* env) {
  //__android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativeRender");
  gl::ModoRenderizacao(gl::MR_RENDER);
  gl::ModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Perspectiva(ent::CAMPO_VERTICAL_GRAUS, Aspecto(), ent::DISTANCIA_PLANO_CORTE_PROXIMO, ent::DISTANCIA_PLANO_CORTE_DISTANTE);

  gl::Habilita(GL_DEPTH_TEST);
  gl::CorLimpeza(1.0f, 1.0f, 1.0f, 1.0f);
  gl::Limpa(GL_COLOR_BUFFER_BIT);
  gl::Limpa(GL_DEPTH_BUFFER_BIT);
  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  gl::ModoMatriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  gl::OlharPara(
    // from.
    0, -20.0f, 10.0f,
    // to.
    0, 0, 0,
    // up
    0, 0, 1.0);
  gl::Desabilita(GL_LIGHTING);

  // desenha tabuleiro do sul para o norte.
  ent::DesenhaTabuleiro();

  gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  ent::DesenhaGrade();
}

}  // extern "C"
