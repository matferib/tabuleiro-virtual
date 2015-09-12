#ifndef GLTAB_GL_H
#define GLTAB_GL_H

#include <cmath>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>
#include <stack>
#if USAR_OPENGL_ES && !BENCHMARK
#if __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
    #include <OpenGLES/ES2/glext.h>
    #include <OpenGLES/ES2/gl.h>
   // iOS device
    #include <OpenGLES/ES1/glext.h>
    #include <OpenGLES/ES1/gl.h>
  #elif TARGET_OS_MAC
    // Other kinds of Mac OS
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
  #endif
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
//#include <GLES/egl.h>  Da problema com o simbolo None definido no X11/X.h, uma enum do Qt em qstyleoption.h usa None tambem.
#include <GLES/glplatform.h>
#include <GLES2/gl2.h>
#endif
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif WIN32
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "matrix/matrices.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define RAD_PARA_GRAUS (180.0f / M_PI)
#define GRAUS_PARA_RAD (M_PI / 180.0f)

#if USAR_OPENGL_ES
#define V_ERRO_STRING(e) ""
#else
#define V_ERRO_STRING(e) gluErrorString(e)
#endif
#if DEBUG
#define V_ERRO(X)\
  do {\
    auto e = glGetError();\
    if (e != GL_NO_ERROR) {\
      LOG_EVERY_N(ERROR, 1000) << "ERRO_GL: " << X << ", codigo: " << e << ", " << V_ERRO_STRING(e);\
      return;\
    }\
  } while (0)
#define V_ERRO_RET(X)\
  do {\
    auto e = glGetError();\
    if (e != GL_NO_ERROR) {\
      LOG_EVERY_N(ERROR, 1000) << "ERRO_GL: " << X << ", codigo: " << e << ", " << V_ERRO_STRING(e);\
      return false;\
    }\
  } while (0)
#else
#define V_ERRO(X)
#define V_ERRO_RET(X) do { if (glGetError() != GL_NO_ERROR) { return false; } } while (0);
#endif

namespace gl {

// Inicializacao e finalizacao da parte grafica. Inicializacao lanca excecao std::logic_error em caso de erro.
void IniciaGl(int* argcp, char** argv);
void FinalizaGl();


#if USAR_SHADER
#define ATUALIZA_MATRIZES_NOVO() AtualizaMatrizesNovo()
// Atualiza as matrizes do shader.
void AtualizaMatrizesNovo();
void DebugaMatrizes();
#else
#define ATUALIZA_MATRIZES_NOVO()
#endif

// Operacoes de matriz. Melhor usar MatrizEscopo.
void EmpilhaMatriz(bool atualizar = true);
void DesempilhaMatriz(bool atualizar = true);
GLenum ModoMatrizCorrente();
void MudarModoMatriz(GLenum modo);

/** Salva a matriz corrente durante escopo da classe. Ou muda o modo de matriz e a salva, retornando ao modo anterior ao fim do escopo. */
class MatrizEscopo {
 public:
  /** Salva a matriz corrente pelo escopo. */
  MatrizEscopo(bool atualizar = true)
      : atualizar_(atualizar), modo_anterior_(GL_INVALID_ENUM), modo_(GL_INVALID_ENUM) { EmpilhaMatriz(atualizar_); }

  /** Muda matriz para matriz do modo e salva pelo escopo. Ao terminar, retorna para o modo anterior a chamada. */
  explicit MatrizEscopo(int modo, bool atualizar = true) : atualizar_(atualizar), modo_(modo) {
    modo_anterior_ = ModoMatrizCorrente();
    MudarModoMatriz(modo_);
    EmpilhaMatriz(atualizar_);
  }

  /** Restaura matriz anterior ao escopo para o modo escolhido. */
  ~MatrizEscopo() {
    if (modo_ != GL_INVALID_ENUM) {
      MudarModoMatriz(modo_);
    }
    DesempilhaMatriz(atualizar_);
    if (modo_anterior_ != GL_INVALID_ENUM) {
      MudarModoMatriz(modo_anterior_);
    }
  }

 private:
  bool atualizar_;
  GLenum modo_anterior_;
  // O valor GL_INVALID_ENUM indica que nao eh para restaurar a matriz.
  GLenum modo_;
};

#if !USAR_OPENGL_ES
/* glPush/PopAttrib bits */
enum bits_atributos_e {
  BIT_POLIGONO = GL_POLYGON_BIT,
  BIT_LUZ = GL_LIGHTING_BIT,
  BIT_PROFUNDIDADE = GL_DEPTH_BUFFER_BIT,
  BIT_STENCIL = GL_STENCIL_BUFFER_BIT,
  BIT_HABILITAR = GL_ENABLE_BIT,
  BIT_COR = GL_COLOR_BUFFER_BIT,
};
/** Salva os atributos durante o escopo da variavel, restaurando no fim. */
class AtributosEscopo {
 public:
  AtributosEscopo(unsigned int mascara) { glPushAttrib(mascara); }
  ~AtributosEscopo() { glPopAttrib(); }
};
#endif

/** Funcao especial para depuracao. */
#if !USAR_OPENGL_ES
inline void InicioCena() {}
#else
void InicioCena();
#endif

#if USAR_SHADER
void ShaderLuz();
void ShaderSimples();
#endif

/** Funcoes gerais. */
bool EstaHabilitado(GLenum cap);
void Habilita(GLenum cap);
void Desabilita(GLenum cap);
void HabilitaEstadoCliente(GLenum cap);
void DesabilitaEstadoCliente(GLenum cap);

inline void Le(GLenum nome_parametro, GLint* valor) { glGetIntegerv(nome_parametro, valor); }
void Le(GLenum nome_parametro, GLfloat* valor);
inline void Le(GLenum nome_parametro, GLboolean* valor) { glGetBooleanv(nome_parametro, valor); }
inline void DesvioProfundidade(GLfloat fator, GLfloat unidades) { glPolygonOffset(fator, unidades);  }

void CarregaIdentidade(bool atualizar = true);
void MultiplicaMatriz(const GLfloat* matriz, bool atualizar = true);
#if !USAR_OPENGL_ES
inline void EmpilhaAtributo(GLbitfield mascara) { glPushAttrib(mascara); }
inline void DesempilhaAtributo() { glPopAttrib(); }
#else
#endif
inline void FaceNula(GLenum modo) { glCullFace(modo); }
inline void CorMistura(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glBlendColor(r, g, b, a); }
inline void FuncaoMistura(GLenum fator_s, GLenum fator_d) { glBlendFunc(fator_s, fator_d); }
inline void Viewport(GLint x, GLint y, GLsizei largura, GLsizei altura) {
  glViewport(x, y, largura, altura);
}

// Texturas.
inline void GeraTexturas(GLsizei n, GLuint* texturas) { glGenTextures(n, texturas); }
inline void ApagaTexturas(GLsizei n, const GLuint* texturas) { glDeleteTextures(n, texturas); }
inline void LigacaoComTextura(GLenum alvo, GLuint textura) { glBindTexture(alvo, textura); }
inline void ParametroTextura(GLenum alvo, GLenum nome_param, GLint valor_param) { glTexParameteri(alvo, nome_param, valor_param); }
inline void ImagemTextura2d(
    GLenum alvo, GLint nivel, GLint formato_interno, GLsizei largura, GLsizei altura, GLint borda,
    GLenum formato, GLenum tipo, const GLvoid* dados) {
  glTexImage2D(alvo, nivel, formato_interno, largura, altura, borda, formato, tipo, dados);
}

// Funcoes OpenGL 1.2 e acima.
#if WIN32
void GeraBuffers(GLsizei n, GLuint* buffers);
void LigacaoComBuffer(GLenum target, GLuint buffer);
void ApagaBuffers(GLsizei n, const GLuint* buffers);
void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
#else
inline void GeraBuffers(GLsizei n, GLuint* buffers) { glGenBuffers(n, buffers); }
inline void LigacaoComBuffer(GLenum target, GLuint buffer) { glBindBuffer(target, buffer); }
inline void ApagaBuffers(GLsizei n, const GLuint* buffers) { glDeleteBuffers(n, buffers); }
inline void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { glBufferData(target, size, data, usage); }
#endif


/** Desenha elementos e afins. */
inline void DesenhaElementos(GLenum modo, GLsizei num_vertices, GLenum tipo, const GLvoid* indices) {
  ATUALIZA_MATRIZES_NOVO();
  glDrawElements(modo, num_vertices, tipo, indices);
}
// Vertices.
void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices);
inline void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  PonteiroVertices(vertices_por_coordenada, tipo, 0, vertices);
}
// Vertices textura.
void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices);
inline void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, const GLvoid* vertices) {
  PonteiroVerticesTexturas(vertices_por_coordenada, tipo, 0, vertices);
}
void PonteiroNormais(GLenum tipo, GLsizei passo, const GLvoid* normais);
inline void PonteiroNormais(GLenum tipo, const GLvoid* normais) { PonteiroNormais(tipo, 0, normais);  }

void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores);

/** Funcoes de nomes. */
#if !USAR_OPENGL_ES
inline void IniciaNomes() { glInitNames(); }
inline void EmpilhaNome(GLuint nome) { glPushName(nome); }
inline void CarregaNome(GLuint nome) { glLoadName(nome); }
inline void DesempilhaNome() { glPopName(); }
#else
void IniciaNomes();
void EmpilhaNome(GLuint nome);
void CarregaNome(GLuint nome);
void DesempilhaNome();
#endif

/** Configura o tipo de objeto para o escopo, retornando ao sair. */
class TipoEscopo {
 public:
  TipoEscopo(GLuint tipo, GLuint tipo_anterior = -1) {
    tipo_anterior_ = tipo_anterior;
    if (tipo_anterior != (GLuint)-1) {
      DesempilhaNome();
      DesempilhaNome();
    }
    EmpilhaNome(tipo);
    EmpilhaNome(tipo);  // no openglES vai ser util.
  }
  ~TipoEscopo() {
    DesempilhaNome();
    DesempilhaNome();
    if (tipo_anterior_ != (GLuint)-1) {
      EmpilhaNome(tipo_anterior_);
      EmpilhaNome(tipo_anterior_);
    }
  }

 private:
  GLuint tipo_anterior_;
};

/** Funcoes de escala, translacao e rotacao. */
void Escala(GLfloat x, GLfloat y, GLfloat z, bool atualizar = true);
void Translada(GLfloat x, GLfloat y, GLfloat z, bool atualizar = true);
void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z, bool atualizar = true);

/** Funcoes de iluminacao. */
#if !USAR_SHADER
inline void  Luz(GLenum luz, GLenum nome_param, GLfloat param) { glLightf(luz, nome_param, param); }
inline void Luz(GLenum luz, GLenum nome_param, const GLfloat* params) { glLightfv(luz, nome_param, params); }
#endif
void LuzAmbiente(float r, float g, float b);
void LuzDirecional(const GLfloat* pos, float r, float g, float b);
void LuzPontual(GLenum luz, GLfloat* pos, float r, float g, float b, float raio);

/** Funcoes de nevoa. */
void Nevoa(GLfloat inicio, GLfloat fim, float r, float g, float b, GLfloat* pos_referencia);

/** Funcoes de normais. */
void Normal(GLfloat x, GLfloat y, GLfloat z);

/** Raster. */
#if !USAR_OPENGL_ES
inline void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) {
  ATUALIZA_MATRIZES_NOVO();
  glRasterPos3f(x, y, z);
}
inline void PosicaoRaster(GLint x, GLint y) { glRasterPos2i(x, y); }
inline void DesenhaCaractere(char c) { glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c); }
/** Retorna o tamanho da fonte. Sempre fixo. */
inline void TamanhoFonte(int* largura, int* altura) { *largura = 8; *altura = 13; }
inline void TamanhoFonte(int largura_vp, int altura_vp, int* largura, int* altura) { *largura = 8; *altura = 13; }
#else
void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z);
void PosicaoRaster(GLint x, GLint y);
void DesenhaCaractere(char c);
/** Retorna o tamanho da fonte. Variavel de acordo com viewport. Segunda versao eh um pouco mais barata. */
void TamanhoFonte(int* largura, int* altura);
void TamanhoFonte(int largura_vp, int altura_vp, int* largura, int* altura);
#endif
// Desenha a string str centralizada no ponto do raster.
// Se inverte_vertical for verdadeiro, linhas irao para cima ao inves de ir
// para baixo.
void DesenhaString(const std::string& str, bool inverte_vertical = false);
// Desenha a string str alinhada ao raster.
void DesenhaStringAlinhadoEsquerda(const std::string& str, bool inverte_vertical = false);
void DesenhaStringAlinhadoDireita(const std::string& str, bool inverte = false);

/** Transformacao de camera. */
void OlharPara(float olho_x, float olho_y, float olho_z,
               float centro_x, float centro_y, float centro_z,
               float cima_x, float cima_y, float cima_z);

/** Transformacao de projecao. */
void Perspectiva(float angulo_y, float aspecto, float z_perto, float z_longe);
void Ortogonal(float esquerda, float direita, float baixo, float cima, float proximo, float distante);
void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport);
GLint Desprojeta(float x_janela, float y_janela, float profundidade_3d,
                 const float* model, const float* proj, const GLint* view,
                 GLfloat* x3d, float* y3d, float* z3d);

/** Picking. */
#if !USAR_OPENGL_ES
enum modo_renderizacao_e {
  MR_RENDER = GL_RENDER,
  MR_SELECT = GL_SELECT
};
inline GLint ModoRenderizacao(modo_renderizacao_e modo) { return glRenderMode(modo); }
inline void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) { glSelectBuffer(tam_buffer, buffer); }
#else
enum modo_renderizacao_e {
  MR_RENDER = 0x1C00,
  MR_SELECT = 0x1C02
};
/* Render Mode */
GLint ModoRenderizacao(modo_renderizacao_e modo);
void BufferSelecao(GLsizei tam_buffer, GLuint* buffer);
#endif

/** Mudanca de cor. */
void MudaCor(float r, float g, float b, float a);
inline void MascaraCor(GLboolean mascara) { glColorMask(mascara, mascara, mascara, mascara); }

inline void CorLimpeza(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glClearColor(r, g, b, a); }
#if !USAR_OPENGL_ES
inline void Limpa(GLbitfield mascara) { glClear(mascara); }
#else
void Limpa(GLbitfield mascara);
#endif

inline void MascaraProfundidade(GLboolean valor) { glDepthMask(valor); }
class DesligaEscritaProfundidadeEscopo {
 public:
  DesligaEscritaProfundidadeEscopo();
  ~DesligaEscritaProfundidadeEscopo();

 private:
  GLboolean valor_anterior_;
};

/** Habilita uma caracteristica pelo escopo. Ver glEnable. */
class HabilitaEscopo {
 public:
  HabilitaEscopo(GLenum cap) : cap_(cap) { Habilita(cap_); }
  ~HabilitaEscopo() { Desabilita(cap_); }
 private:
  GLenum cap_;
};

/** Desabilita uma caracteristica pelo escopo. Ver glDisable. */
class DesabilitaEscopo {
 public:
  DesabilitaEscopo(GLenum cap) : cap_(cap) {
    //Le(cap, &valor_anterior_);
    valor_anterior_ = EstaHabilitado(cap);
    Desabilita(cap_);
  }
  ~DesabilitaEscopo() {
    if (valor_anterior_) {
      Habilita(cap_);
    }
  }
 private:
  GLenum cap_;
  GLboolean valor_anterior_;
};

#if !USAR_SHADER
class ModeloLuzEscopo {
 public:
  ModeloLuzEscopo(const GLfloat* luz) {
      glGetFloatv(GL_LIGHT_MODEL_AMBIENT, luz_antes);
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luz);
  }
  ~ModeloLuzEscopo() {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luz_antes);
  }

 private:
  GLfloat luz_antes[4];
};
#endif


/** Stencil. */
inline void FuncaoStencil(GLenum func, GLint ref, GLuint mascara) { glStencilFunc(func, ref, mascara); }

inline void OperacaoStencil(GLenum falha_stencil, GLenum falha_profundidade, GLenum sucesso) {
  glStencilOp(falha_stencil, falha_profundidade, sucesso);
}

/** debugging. */
void AlternaModoDebug();

// Namespace para utilidades internas, nem deveria estar aqui.
namespace interno {

// Variaveis de shaders.
struct VarShader {
  std::string nome;  // Nome para o programa, depuracao apenas.

  // Shader.
  GLuint programa;
  GLuint vs;
  GLuint fs;
  GLuint programa_simples;
  GLuint vs_simples;
  GLuint fs_simples;

  // Variaveis uniformes dos shaders.
  GLint uni_gltab_luz_ambiente_cor;     // Cor da luz ambiente. Alfa indica se iluminacao geral esta ligada.
  GLint uni_gltab_luz_direcional_cor;   // Cor da luz direcional.
  GLint uni_gltab_luz_direcional_pos;   // Posicao da luz direcional ().
  GLint uni_gltab_luzes[7 * 3];         // Luzes pontuais: 7 luzes InfoLuzPontual (3 vec4: pos, cor, atributos).
  GLint uni_gltab_textura;              // Ha textura: 1, nao ha: 0.
  GLint uni_gltab_unidade_textura;
  GLint uni_gltab_nevoa_dados;          // Dados da nevoa: inicio, fim, escala.
  GLint uni_gltab_nevoa_cor;            // Cor da nevoa.
  GLint uni_gltab_nevoa_referencia;     // Ponto de referencia da nevoa.
  GLint uni_gltab_mvm;                  // Matrix modelview.
  GLint uni_gltab_nm;                   // Matrix de normais.
  GLint uni_gltab_prm;                  // Matrix projecao.
  // Atributos do vertex shader.
  GLint atr_gltab_vertice;
  GLint atr_gltab_normal;
  GLint atr_gltab_cor;
  GLint atr_gltab_texel;
};

// Usado para indexar os shaders.
enum TipoShader {
  TSH_LUZ = 0,
  TSH_SIMPLES = 1,
  TSH_NUM,  // numero de shaders.
};

// Depende de plataforma.
struct ContextoDependente {
  virtual ~ContextoDependente() {}
};

// Contexto comum.
class Contexto {
 public:
  Contexto(ContextoDependente* cd) : shaders(TSH_NUM), interno(cd) {}
  ~Contexto() {}

  bool depurar_selecao_por_cor = false;  // Mudar para true para depurar selecao por cor.

  std::vector<VarShader> shaders;
  VarShader* shader_corrente = nullptr;  // Aponta para o shader corrente.

  // Matrizes correntes. Ambas as pilhas sao iniciadas com a identidade.
  std::stack<Matrix4> pilha_mvm;
  std::stack<Matrix4> pilha_prj;
  std::stack<Matrix4>* pilha_corrente = nullptr;
  Matrix3 matriz_normal;  // Computada da mvm corrente.

  std::unique_ptr<ContextoDependente> interno;
};
Contexto* BuscaContexto();
inline const VarShader& BuscaShader(TipoShader ts) { return BuscaContexto()->shaders[ts]; }
inline const VarShader& BuscaShader() { return *BuscaContexto()->shader_corrente; }
inline bool UsandoShaderLuz() {
  auto* c = BuscaContexto();
  return c->shader_corrente == &c->shaders[TSH_LUZ];
}

bool LuzPorVertice(int argc, const char* const * argv);  // Retorna true se encontrar --luz_por_vertice.
void IniciaComum(bool luz_por_vertice, interno::Contexto* contexto);
void FinalizaShaders(const VarShader& shader);
void HabilitaComShader(interno::Contexto* contexto, GLenum cap);
void DesabilitaComShader(interno::Contexto* contexto, GLenum cap);

// Quebra uma string em varias.
const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra);

}  // namespace interno

}  // namespace gl

#endif  // GL_GL_H
