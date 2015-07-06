// OpenGL ES.
// Varias funcoes copiadas do GLUES: https://code.google.com/p/glues/.

#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include "gltab/gl.h"
#include "log/log.h"

namespace gl {

#define __glPi 3.14159265358979323846
namespace {

void PreencheIdentidade(GLfloat m[16]) {
  m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
  m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
  m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
  m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

void Normaliza(GLfloat v[3]) {
  GLfloat r = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  if (r == 0.0f) {
    return;
  }
  v[0] /= r;
  v[1] /= r;
  v[2] /= r;
}

void ProdutoVetorial(GLfloat v1[3], GLfloat v2[3], GLfloat result[3]) {
  result[0] = v1[1] * v2[2] - v1[2] * v2[1];
  result[1] = v1[2] * v2[0] - v1[0] * v2[2];
  result[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
static int __gluInvertMatrixf(const GLfloat m[16], GLfloat invOut[16])
{
    GLfloat inv[16], det;
    int i;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det=1.0f/det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}

static void __gluMultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4],
                                GLfloat out[4])
{
    int i;

    for (i=0; i<4; i++)
    {
        out[i] = in[0] * matrix[0*4+i] +
                 in[1] * matrix[1*4+i] +
                 in[2] * matrix[2*4+i] +
                 in[3] * matrix[3*4+i];
    }
}

static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16], GLfloat r[16]) {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r[i*4+j] = a[i*4+0]*b[0*4+j] +
                       a[i*4+1]*b[1*4+j] +
                       a[i*4+2]*b[2*4+j] +
                       a[i*4+3]*b[3*4+j];
        }
    }
}

GLint gluProject(
    GLfloat objx,
    GLfloat objy,
    GLfloat objz,
    const GLfloat modelMatrix[16],
    const GLfloat projMatrix[16],
    const GLint viewport[4],
    GLfloat* winx,
    GLfloat* winy,
    GLfloat* winz) {
  GLfloat in[4];
  GLfloat out[4];

  in[0]=objx;
  in[1]=objy;
  in[2]=objz;
  in[3]=1.0;
  __gluMultMatrixVecf(modelMatrix, in, out);
  __gluMultMatrixVecf(projMatrix, out, in);

  /*
  int max;
  gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &max);
  LOG(INFO) << "Maximo pilha PJ: " << max;
  gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &max);
  LOG(INFO) << "Maximo pilha MV: " << max;
  for (int i = 0; i < 16; ++i) {
    LOG(INFO) << "proj[" << i << "]: " << projMatrix[i];
  }
  for (int i = 0; i < 16; ++i) {
    LOG(INFO) << "mv[" << i << "]: " << modelMatrix[i];
  }
  */
  if (in[3] == 0.0) {
    LOG(ERROR) << "Projecao falhou";
    //int max;
    //gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &max);
    //LOG(INFO) << "Maximo pilha PJ: " << max;
    //gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &max);
    //LOG(INFO) << "Maximo pilha MV: " << max;
    return GL_FALSE;
  }
  /*
  LOG(INFO) << "Projecao ok";
  */

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];
  /* Map x, y and z to range 0-1 */
  in[0]= in[0] * 0.5f + 0.5f;
  in[1]= in[1] * 0.5f + 0.5f;
  in[2]= in[2] * 0.5f + 0.5f;

  /* Map x,y to viewport */
  in[0] = in[0] * viewport[2] + viewport[0];
  in[1] = in[1] * viewport[3] + viewport[1];

  *winx = in[0];
  *winy = in[1];
  *winz = in[2];
  return GL_TRUE;
}
struct ContextoInterno {
  // Mapeia um ID para a cor RGB em 21 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  unsigned int proximo_id = 0;
  // O bit da pilha em tres bits (valor de [0 a 7]).
  unsigned int bit_pilha = 0;
  modo_renderizacao_e modo_renderizacao = MR_RENDER;
  bool depurar_selecao_por_cor = false;  // Mudar para true para depurar selecao por cor.
  GLuint* buffer_selecao = nullptr;
  GLuint tam_buffer = 0;
  float raster_x = 0.0f;
  float raster_y = 0.0f;
  int max_pilha_mv = 0.0f;
  int max_pilha_pj = 0.0f;

  inline bool UsarSelecaoPorCor() const {
    return modo_renderizacao == MR_SELECT || depurar_selecao_por_cor ;
  }
};

ContextoInterno* g_contexto = nullptr;

// Gera um proximo ID.
void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  unsigned int id_mapeado = g_contexto->proximo_id | (g_contexto->bit_pilha << 21);
  g_contexto->ids.insert(std::make_pair(id_mapeado, id));
  if (g_contexto->proximo_id == ((1 << 21) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    if (g_contexto->depurar_selecao_por_cor) {
      // Mais facil de ver.
      g_contexto->proximo_id += 5;
    } else {
      ++g_contexto->proximo_id;
    }
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto = new ContextoInterno;
  gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &g_contexto->max_pilha_pj);
  gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &g_contexto->max_pilha_mv);
  LOG(INFO) << "Max pilha mv: " << g_contexto->max_pilha_mv;
  LOG(INFO) << "Max pilha pj: " << g_contexto->max_pilha_pj;
}

void FinalizaGl() {
  delete g_contexto;
}

void InicioCena() {
  if (g_contexto->UsarSelecaoPorCor()) {
    g_contexto->proximo_id = 0;
    g_contexto->bit_pilha = 0;
    g_contexto->ids.clear();
  }
}

void Habilita(GLenum cap) {
  if (g_contexto->UsarSelecaoPorCor() && (cap == GL_LIGHTING)) {
    // Sem luz no modo de selecao.
    glDisable(cap);
  } else {
    glEnable(cap);
  }
}

void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  const unsigned short indices[] = { 0, 1, 2, 3 };
  const float vertices[] = {
    x1, y1,
    x2, y1,
    x2, y2,
    x1, y2,
  };
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Perspectiva(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
  // Copiado do glues.
  GLfloat m[4][4];
  GLfloat sine, cotangent, deltaZ;
  GLfloat radians=(GLfloat)(fovy/2.0f*__glPi/180.0f);

  deltaZ=zFar-zNear;
  sine=(GLfloat)sinf(radians);
  if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f))
  {
      return;
  }
  cotangent=(GLfloat)(cos(radians)/sine);

  PreencheIdentidade(&m[0][0]);
  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1.0f;
  m[3][2] = -2.0f * zNear * zFar / deltaZ;
  m[3][3] = 0;
  glMultMatrixf(&m[0][0]);
}

void OlharPara(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz,
               GLfloat upx, GLfloat upy, GLfloat upz) {
  GLfloat forward[3], side[3], up[3];
  GLfloat m[4][4];

  forward[0] = centerx - eyex;
  forward[1] = centery - eyey;
  forward[2] = centerz - eyez;

  up[0] = upx;
  up[1] = upy;
  up[2] = upz;

  Normaliza(forward);

  /* Side = forward x up */
  ProdutoVetorial(forward, up, side);
  Normaliza(side);

  /* Recompute up as: up = side x forward */
  ProdutoVetorial(side, forward, up);

  PreencheIdentidade(&m[0][0]);
  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];

  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];

  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];

  glMultMatrixf(&m[0][0]);
  glTranslatef(-eyex, -eyey, -eyez);
}

GLint Desprojeta(GLfloat winx, GLfloat winy, GLfloat winz,
                 const GLfloat modelMatrix[16],
                 const GLfloat projMatrix[16],
                 const GLint viewport[4],
                 GLfloat* objx, GLfloat* objy, GLfloat* objz) {
  GLfloat finalMatrix[16];
  GLfloat in[4];
  GLfloat out[4];

  __gluMultMatricesf(modelMatrix, projMatrix, finalMatrix);
  if (!__gluInvertMatrixf(finalMatrix, finalMatrix))
  {
      return(GL_FALSE);
  }

  in[0]=winx;
  in[1]=winy;
  in[2]=winz;
  in[3]=1.0;

  /* Map x and y from window coordinates */
  in[0] = (in[0] - viewport[0]) / viewport[2];
  in[1] = (in[1] - viewport[1]) / viewport[3];

  /* Map to range -1 to 1 */
  in[0] = in[0] * 2 - 1;
  in[1] = in[1] * 2 - 1;
  in[2] = in[2] * 2 - 1;

  __gluMultMatrixVecf(finalMatrix, in, out);
  if (out[3] == 0.0)
  {
      return(GL_FALSE);
  }

  out[0] /= out[3];
  out[1] /= out[3];
  out[2] /= out[3];
  *objx = out[0];
  *objy = out[1];
  *objz = out[2];

  return(GL_TRUE);
}

void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport) {
  if (delta_x <= 0 || delta_y <= 0) {
      return;
  }

  /* Translate and scale the picked region to the entire window */
  glTranslatef((viewport[2] - 2 * (x - viewport[0])) / delta_x,
               (viewport[3] - 2 * (y - viewport[1])) / delta_y, 0);
  glScalef(viewport[2] / delta_x, viewport[3] / delta_y, 1.0);
}

GLint ModoRenderizacao(modo_renderizacao_e modo) {
  if (g_contexto->modo_renderizacao == modo) {
    VLOG(1) << "Nao houve mudanca no modo de renderizacao";
    return 0;
  }
  g_contexto->modo_renderizacao = modo;
  switch (modo) {
    case MR_SELECT:
      return 0;
    case MR_RENDER: {
      glFlush();
      glFinish();
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GLubyte pixel[4] = { 0 };
      glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      int erro = glGetError();
      if (erro != 0) {
        VLOG(1) << "Erro pos glReadPixels: " << erro;
      }
      // Usar void* para imprimir em hexa.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      VLOG(2) << "Pixel: " << (void*)pixel[0] << " " << (void*)pixel[1] << " " << (void*)pixel[2] << " " << (void*)pixel[3];
      unsigned int id_mapeado = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      VLOG(1) << "Id mapeado: " << (void*)id_mapeado;
      unsigned int tipo_objeto = id_mapeado >> 21;
      VLOG(1) << "Tipo objeto: " << tipo_objeto;
      if (tipo_objeto > 7) {
        LOG(ERROR) << "Tipo objeto invalido: " << tipo_objeto;
        return 0;
      }
      auto it = g_contexto->ids.find(id_mapeado);
      if (it == g_contexto->ids.end()) {
        LOG(ERROR) << "Id nao mapeado: " << (void*)id_mapeado;
        return 0;
      }
#pragma GCC diagnostic pop
      unsigned int id_original = it->second;
      VLOG(1) << "Id original: " << id_original;
      GLuint* ptr = g_contexto->buffer_selecao;
      ptr[0] = 2;  // Sempre 2: 1 para tipo, outro para id.
      ptr[1] = 0;  // zmin.
      ptr[2] = 0;  // zmax
      ptr[3] = tipo_objeto;
      ptr[4] = id_original;
      g_contexto->buffer_selecao = nullptr;
      g_contexto->tam_buffer = 0;
      return 1;  // Numero de hits: so pode ser 0 ou 1.
    }
    default:
      return 0;
  }
}

void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) {
  g_contexto->buffer_selecao = buffer;
  g_contexto->tam_buffer = tam_buffer;
}

// Nomes
void IniciaNomes() {
}

void EmpilhaNome(GLuint id) {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (id > 7) {
    LOG(ERROR) << "Bit da pilha passou do limite superior.";
    return;
  }
  g_contexto->bit_pilha = id;
  VLOG(1) << "Empilhando bit pilha: " << g_contexto->bit_pilha;
}

void CarregaNome(GLuint id) {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  GLubyte rgb[3];
  MapeiaId(id, rgb);
  VLOG(2) << "Mapeando " << id << ", bit pilha " << g_contexto->bit_pilha
          << " para " << (int)rgb[0] << ", " << (int)rgb[1] << ", " << (int)rgb[2];
  // Muda a cor para a mapeada.
  glColor4ub(rgb[0], rgb[1], rgb[2], 255);
}

void DesempilhaNome() {
  if (!g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (g_contexto->bit_pilha == 0) {
    // No jeito novo, isso nao eh mais erro.
    //LOG(ERROR) << "Bit da pilha passou do limite inferior.";
    return;
  }
  VLOG(1) << "Desempilhando bit pilha: " << g_contexto->bit_pilha;
  g_contexto->bit_pilha = 0;
}

void MudaCor(float r, float g, float b, float a) {
  if (g_contexto->UsarSelecaoPorCor()) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
  //GLfloat cor[4] = { r, g, b, a };
  // Segundo manual do OpenGL ES, nao se pode definir o material separadamente por face.
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
}

void Limpa(GLbitfield mascara) {
  if (g_contexto->UsarSelecaoPorCor()) {
    if ((mascara & GL_COLOR_BUFFER_BIT) != 0) {
      // Preto nao eh valido no color picking.
      glClearColor(0, 0, 0, 1.0f);
    }
  }
  glClear(mascara);
}

void TamanhoFonte(int largura_viewport, int altura_viewport, int* largura_fonte, int* altura) {
  unsigned int media_tela = (largura_viewport + altura_viewport) / 2;
  *largura_fonte = media_tela / 64;
  *altura = static_cast<int>(*largura_fonte * (13.0f / 8.0f));
}

void TamanhoFonte(int* largura, int* altura) {
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  TamanhoFonte(viewport[2], viewport[3], largura, altura);
}

namespace interno {

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical) {
  // Melhor deixar comentado assim para as letras ficarem sempre em primeiro plano.
  //gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  //gl::DesligaEscritaProfundidadeEscopo mascara_escopo;
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, 1.0f);
  gl::MatrizEscopo salva_matriz_proj(GL_MODELVIEW);
  gl::CarregaIdentidade();

  int largura_fonte;
  int altura_fonte;
  TamanhoFonte(&largura_fonte, &altura_fonte);

  float x2d = g_contexto->raster_x;
  float y2d = g_contexto->raster_y;
  gl::Translada(x2d, y2d, 0.0f);

  //LOG(INFO) << "x2d: " << x2d << " y2d: " << y2d;
  gl::Escala(largura_fonte, altura_fonte, 1.0f);
  std::vector<std::string> str_linhas(interno::QuebraString(str, '\n'));
  for (const std::string& str_linha : str_linhas) {
    float translacao_x = -static_cast<float>(str_linha.size());
    if (alinhamento == 0) {
      translacao_x /= 2.0f;
    }
    gl::Translada(translacao_x, 0.0f, 0.0f);
    for (const char c : str_linha) {
      gl::DesenhaCaractere(c);
      gl::Translada(1.0f, 0.0f, 0.0f);
    }
    gl::Translada(-(translacao_x + static_cast<float>(str_linha.size())), inverte_vertical ? 1.0f : -1.0f, 0.0f);
  }
}

}  // namespace

void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) {
  float matriz_mv[16];
  float matriz_pr[16];
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::Le(GL_MODELVIEW_MATRIX, matriz_mv);
  gl::Le(GL_PROJECTION_MATRIX, matriz_pr);
  float x2d, y2d, z2d;
  if (!gluProject(x, y, z, matriz_mv, matriz_pr, viewport, &x2d, &y2d, &z2d)) {
    return;
  }
  g_contexto->raster_x = x2d;
  g_contexto->raster_y = y2d;
  //LOG(INFO) << "raster_x: " << x2d << ", raster_y: " << y2d;
}

void PosicaoRaster(GLint x, GLint y) {
  PosicaoRaster(static_cast<float>(x), static_cast<float>(y), 0.0f);
}

void AlternaModoDebug() {
  g_contexto->depurar_selecao_por_cor = !g_contexto->depurar_selecao_por_cor;
}

}  // namespace gl
