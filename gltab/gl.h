#ifndef GLTAB_GL_H
#define GLTAB_GL_H

#include <cmath>
#include <memory>
#include <functional>
#include <string>
#include <stdexcept>
#include <vector>
#include <stack>
#include <unordered_map>
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
#include <GLES2/gl2ext.h>
#endif
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif WIN32
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
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
#include "net/util.h"
#define V_ERRO(X) do { auto e = glGetError(); if (e != GL_NO_ERROR) { LOG_EVERY_N(ERROR, 1) << "ERRO_GL: " << X << ", codigo: " << (void*)(unsigned long long)e << ", " << V_ERRO_STRING(e); return; } } while (0)
#define V_ERRO_RET(X) do { auto e = glGetError(); if (e != GL_NO_ERROR) { LOG_EVERY_N(ERROR, 1000) << "ERRO_GL: " << X << ", codigo: " << (void*)(unsigned long long)e << ", " << V_ERRO_STRING(e); return false; } } while (0)
#else
#define V_ERRO(X)
#define V_ERRO_RET(X)
#endif

namespace gl {

// Inicializacao e finalizacao da parte grafica. Da excecao em caso de erro.
enum TipoLuz {
  TL_POR_VERTICE = 0,
  TL_POR_PIXEL = 1,
  TL_POR_PIXEL_ESPECULAR = 2
};
void IniciaGl(TipoLuz tipo_luz, float escala = 1.0f);
void FinalizaGl();

// Atualiza as matrizes do shader de acordo com o modo. Apenas as necessarias serao atualizadas.
void AtualizaMatrizes();
/** Atualiza a matriz de projecao do shader, independente do modo. */
void AtualizaMatrizProjecao();
void AtualizaMatrizCamera();
// Atualiza todas as matrizes do shader.
void AtualizaTodasMatrizes();
void DebugaMatrizes();

// Operacoes de matriz. Melhor usar MatrizEscopo.
void EmpilhaMatriz();
void DesempilhaMatriz();
enum matriz_e {
  MATRIZ_MODELAGEM_CAMERA_NAO_USAR = GL_MODELVIEW,
  MATRIZ_PROJECAO = GL_PROJECTION,
  MATRIZ_AJUSTE_TEXTURA = GL_TEXTURE,
  MATRIZ_SOMBRA = GL_MODELVIEW + 3,
  MATRIZ_PROJECAO_SOMBRA = GL_MODELVIEW + 4,
  MATRIZ_OCLUSAO = GL_MODELVIEW + 5,
  MATRIZ_MODELAGEM = GL_MODELVIEW + 6,
  MATRIZ_CAMERA = GL_MODELVIEW + 7,
  MATRIZ_MODELAGEM_SOMBRA = GL_MODELVIEW + 8,
  MATRIZ_CAMERA_SOMBRA = GL_MODELVIEW + 9,
  MATRIZ_MODELAGEM_OCLUSAO = GL_MODELVIEW + 10,
  MATRIZ_CAMERA_OCLUSAO = GL_MODELVIEW + 11,
  MATRIZ_LUZ = GL_MODELVIEW + 12,

  MATRIZ_INVALIDA = GL_INVALID_ENUM
};
int ModoMatrizCorrente();
void MudaModoMatriz(int modo);

/** Salva a matriz corrente durante escopo da classe. Ou muda o modo de matriz e a salva, retornando ao modo anterior ao fim do escopo. */
class MatrizEscopo {
 public:
  /** Salva a matriz corrente pelo escopo. */
  MatrizEscopo()
      : modo_anterior_(GL_INVALID_ENUM), modo_(GL_INVALID_ENUM) { EmpilhaMatriz(); }

  /** Muda matriz para matriz do modo e salva pelo escopo. Ao terminar, retorna para o modo anterior a chamada. */
  explicit MatrizEscopo(matriz_e modo) : modo_(modo) {
    modo_anterior_ = ModoMatrizCorrente();
    MudaModoMatriz(modo_);
    EmpilhaMatriz();
  }

  /** Restaura matriz anterior ao escopo para o modo escolhido. */
  ~MatrizEscopo() {
    if (modo_ != GL_INVALID_ENUM) {
      MudaModoMatriz(static_cast<matriz_e>(modo_));
    }
    DesempilhaMatriz();
    // No caso da matriz de projecao e camera, eh sempre interessante atualizar a volta, ja que ela so eh configurada uma vez.
    if (modo_ == MATRIZ_PROJECAO) {
      AtualizaMatrizProjecao();
    }
    if (modo_ == MATRIZ_CAMERA) {
      AtualizaMatrizCamera();
    }
    if (modo_anterior_ != GL_INVALID_ENUM) {
      MudaModoMatriz(static_cast<matriz_e>(modo_anterior_));
    }
  }

 private:
  int modo_anterior_;
  // O valor GL_INVALID_ENUM indica que nao eh para restaurar a matriz.
  int modo_;
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
void InicioCena();

// Usado para indexar os shaders.
enum TipoShader {
  TSH_LUZ,           // shader completo.
  TSH_SIMPLES,       // bem simples, sem luz bom para 2d com textura.
  TSH_PICKING,       // bem simples, bom pra picking.
  TSH_PROFUNDIDADE,  // codifica profundidade com cor.
  TSH_PRETO_BRANCO,  // para visao escuro.
  TSH_CAIXA_CEU,     // para caixa do ceu.
  TSH_PONTUAL,       // para luz e oclusao pontual.
  TSH_TESTE,         // teste.
  TSH_NUM,  // numero de shaders.
};

void UsaShader(TipoShader ts);
TipoShader TipoShaderCorrente();

/** Funcoes gerais. */
bool EstaHabilitado(GLenum cap);
void Habilita(GLenum cap);
void Desabilita(GLenum cap);
void HabilitaEstadoCliente(GLenum cap);
void DesabilitaEstadoCliente(GLenum cap);

// Funcoes para habilitar e desabilitar mipmap e aniso para texturas.
void HabilitaMipmapAniso(GLenum alvo);
void DesabilitaMipmapAniso(GLenum alvo);

Matrix4 LeMatriz(matriz_e tipo_matriz);
inline void Le(GLenum nome_parametro, GLint* valor) { glGetIntegerv(nome_parametro, valor); }
void Le(GLenum nome_parametro, GLfloat* valor);
inline void Le(GLenum nome_parametro, GLboolean* valor) { glGetBooleanv(nome_parametro, valor); }
inline const GLubyte* Le(GLenum nome) { return glGetString(nome); }
inline void DesvioProfundidade(GLfloat fator, GLfloat unidades) { glPolygonOffset(fator, unidades);  }
bool TemExtensao(const std::string& nome_extensao);

void CarregaIdentidade();
void MultiplicaMatriz(const GLfloat* matriz);
#if !USAR_OPENGL_ES
inline void EmpilhaAtributo(GLbitfield mascara) { glPushAttrib(mascara); }
inline void DesempilhaAtributo() { glPopAttrib(); }
#else
#endif
void CorMisturaPreNevoa(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f);
void LeCorMisturaPreNevoa(GLfloat* rgb);
inline void FaceNula(GLenum modo) { glCullFace(modo); }
inline void FuncaoMistura(GLenum fator_s, GLenum fator_d) { glBlendFunc(fator_s, fator_d); }
inline void FuncaoProfundidade(GLenum funcao) { glDepthFunc(funcao); }
inline void Viewport(GLint x, GLint y, GLsizei largura, GLsizei altura) {
  glViewport(x, y, largura, altura);
}

// Texturas.
void UnidadeTextura(GLenum unidade);
void TexturaBump(bool estado);
inline void GeraTexturas(GLsizei n, GLuint* texturas) { glGenTextures(n, texturas); }
inline void ApagaTexturas(GLsizei n, const GLuint* texturas) { glDeleteTextures(n, texturas); }
inline void LigacaoComTextura(GLenum alvo, GLuint textura) { glBindTexture(alvo, textura); }
inline void ParametroTextura(GLenum alvo, GLenum nome_param, GLint valor_param) { glTexParameteri(alvo, nome_param, valor_param); }
inline void ImagemTextura2d(
    GLenum alvo, GLint nivel, GLint formato_interno, GLsizei largura, GLsizei altura, GLint borda,
    GLenum formato, GLenum tipo, const GLvoid* dados) {
  glTexImage2D(alvo, nivel, formato_interno, largura, altura, borda, formato, tipo, dados);
}

inline void BufferDesenho(GLenum modo) {
#if !USAR_OPENGL_ES
  glDrawBuffer(modo);
#endif
}
inline void BufferLeitura(GLenum modo) {
#if !USAR_OPENGL_ES
  glReadBuffer(modo);
#endif
}

// Funcoes OpenGL 1.2 e acima.
#if WIN32
void LocalAtributo(GLuint program, GLuint index, const GLchar *name);
GLenum VerificaFramebuffer(GLenum alvo);
void GeraFramebuffers(GLsizei num, GLuint *ids);
void ApagaFramebuffers(GLsizei num, const GLuint *ids);
void LigacaoComFramebuffer(GLenum alvo, GLuint framebuffer);
void GeraRenderbuffers(GLsizei n, GLuint* renderbuffers);
void ApagaRenderbuffers(GLsizei n, const GLuint* renderbuffers);
void LigacaoComRenderbuffer(GLenum target, GLuint buffer);
void ArmazenamentoRenderbuffer(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void RenderbufferDeFramebuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void TexturaFramebuffer(GLenum alvo, GLenum anexo, GLenum alvo_textura, GLuint textura, GLint nivel);
void BufferDesenho(GLenum modo);
void BufferLeitura(GLenum modo);
void GeraMipmap(GLenum alvo);
void CorMistura(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void GeraBuffers(GLsizei n, GLuint* buffers);
void GeraObjetosVertices(GLsizei n, GLuint *arrays);
void LigacaoComBuffer(GLenum target, GLuint buffer);
void LigacaoComObjetoVertices(GLuint buffer);
void ApagaBuffers(GLsizei n, const GLuint* buffers);
void ApagaObjetosVertices(GLsizei n, const GLuint *arrays);
void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void BufferizaSubDados(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
void ShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
void ShaderLeParam(GLuint shader, GLenum pname, GLint *params);
void ProgramaLeParam(GLuint program, GLenum pname, GLint *params);
GLint LocalUniforme(GLuint program, const GLchar *name);
void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
void HabilitaVetorAtributosVertice(GLuint index);
void DesabilitaVetorAtributosVertice(GLuint index);
void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void Uniforme(GLint location, GLfloat v0, GLfloat v1);
void Uniforme(GLint location, GLfloat v0);
void Uniforme(GLint location, GLint v0);
void LeUniforme(GLuint program, GLint location, GLfloat* params);
void LeUniforme(GLuint program, GLint location, GLint* params);
GLuint CriaShader(GLenum shaderType);
void DestroiShader(GLuint shader);
void CompilaShader(GLuint shader);
void AnexaShader(GLuint program, GLuint shader);
void DesanexaShader(GLuint program, GLuint shader);
void FonteShader(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
void LinkaPrograma(GLuint program);
void UsaPrograma(GLuint program);
GLuint CriaPrograma();
void DestroiPrograma(GLuint program);
void LeUniformeAtivo(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GLint LeLocalAtributo(GLuint program, const GLchar* name);
void PonteiroAtributosVertices(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
void Matriz3Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void Matriz4Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
#else
inline void LocalAtributo(GLuint program, GLuint index, const GLchar *name) { glBindAttribLocation(program, index, name); }
inline GLenum VerificaFramebuffer(GLenum alvo) { return glCheckFramebufferStatus(alvo); }
inline void GeraFramebuffers(GLsizei num, GLuint *ids) { glGenFramebuffers(num, ids); }
inline void ApagaFramebuffers(GLsizei num, const GLuint *ids) { glDeleteFramebuffers(num, ids); }
inline void LigacaoComFramebuffer(GLenum alvo, GLuint framebuffer) { glBindFramebuffer(alvo, framebuffer); }
inline void TexturaFramebuffer(GLenum alvo, GLenum anexo, GLenum alvo_textura, GLuint textura, GLint nivel) {
  glFramebufferTexture2D(alvo, anexo, alvo_textura, textura, nivel);
}
inline void GeraMipmap(GLenum alvo) { glGenerateMipmap(alvo); }
inline void CorMistura(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glBlendColor(r, g, b, a); }
inline void GeraBuffers(GLsizei n, GLuint* buffers) { glGenBuffers(n, buffers); }
inline void GeraObjetosVertices(GLsizei n, GLuint *arrays) {
#if __APPLE__
  glGenVertexArraysAPPLE(n, arrays);
#else
  glGenVertexArrays(n, arrays);
#endif
}
inline void LigacaoComBuffer(GLenum target, GLuint buffer) { glBindBuffer(target, buffer); }
inline void LigacaoComObjetoVertices(GLuint buffer) {
#if __APPLE__
  glBindVertexArrayAPPLE(buffer);
#else
  glBindVertexArray(buffer);
#endif
}
inline void LigacaoComRenderbuffer(GLenum target, GLuint buffer) { glBindRenderbuffer(target, buffer); }
inline void GeraRenderbuffers(GLsizei n, GLuint* renderbuffers) { glGenRenderbuffers(n, renderbuffers); }
inline void ApagaRenderbuffers(GLsizei n, const GLuint* renderbuffers) { glDeleteRenderbuffers(n, renderbuffers); }
inline void ArmazenamentoRenderbuffer(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) { glRenderbufferStorage(target, internalformat, width, height); }
inline void RenderbufferDeFramebuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
  glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}
inline void ApagaBuffers(GLsizei n, const GLuint* buffers) { glDeleteBuffers(n, buffers); }
inline void ApagaObjetosVertices(GLsizei n, const GLuint *arrays) {
#if __APPLE__
  glDeleteVertexArraysAPPLE(n, arrays);
#else
  glDeleteVertexArrays(n, arrays);
#endif
}
inline void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { glBufferData(target, size, data, usage); }
inline void BufferizaSubDados(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data) { glBufferSubData(target, offset, size, data); }
inline void ShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog) { glGetShaderInfoLog(shader, maxLength, length, infoLog); }
inline void ShaderLeParam(GLuint shader, GLenum pname, GLint *params) { glGetShaderiv(shader, pname, params); }
inline void ProgramaLeParam(GLuint program, GLenum pname, GLint *params) { glGetProgramiv(program, pname, params); }
inline GLint LocalUniforme(GLuint program, const GLchar *name) { return glGetUniformLocation(program, name); }
inline void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { glVertexAttrib4f(index, v0, v1, v2, v3); }
inline void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2) { glVertexAttrib3f(index, v0, v1, v2); }
inline void HabilitaVetorAtributosVertice(GLuint index) { glEnableVertexAttribArray(index); }
inline void DesabilitaVetorAtributosVertice(GLuint index) { glDisableVertexAttribArray(index); }
inline void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) { glUniform4f(location, v0, v1, v2, v3); }
inline void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) { glUniform3f(location, v0, v1, v2); }
inline void Uniforme(GLint location, GLfloat v0, GLfloat v1) { glUniform2f(location, v0, v1); }
inline void Uniforme(GLint location, GLfloat v0) { glUniform1f(location, v0); }
inline void Uniforme(GLint location, GLint v0) { glUniform1i(location, v0); }
inline void LeUniforme(GLuint program, GLint location, GLfloat* params) { glGetUniformfv(program, location, params); }
inline void LeUniforme(GLuint program, GLint location, GLint* params) { glGetUniformiv(program, location, params); }
inline GLuint CriaShader(GLenum shaderType) { return glCreateShader(shaderType); }
inline void DestroiShader(GLuint shader) { glDeleteShader(shader); }
inline void CompilaShader(GLuint shader) { glCompileShader(shader); }
inline void AnexaShader(GLuint program, GLuint shader) { glAttachShader(program, shader); }
inline void DesanexaShader(GLuint program, GLuint shader) { glDetachShader(program, shader); }
inline void FonteShader(GLuint shader, GLsizei count, const GLchar **s, const GLint *length) { glShaderSource(shader, count, s, length); }
inline void LinkaPrograma(GLuint program) { glLinkProgram(program); }
inline void UsaPrograma(GLuint program) { glUseProgram(program); }
inline GLuint CriaPrograma() { return glCreateProgram(); }
inline void DestroiPrograma(GLuint program) { glDeleteProgram(program); }
inline void LeUniformeAtivo(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
  glGetActiveUniform(program, index, bufSize, length, size, type, name);
}
inline GLint LeLocalAtributo(GLuint program, const GLchar* name) { return glGetAttribLocation(program, name); }
inline void PonteiroAtributosVertices(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer){
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}
inline void Matriz3Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix3fv(location, count, transpose, value);
}
inline void Matriz4Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix4fv(location, count, transpose, value);
}

#endif

/** Desenha o array habilitado de forma sequencial (sem indice). */
inline void DesenhaArrays(GLenum modo, GLint primeiro, GLsizei num) { glDrawArrays(modo, primeiro, num); }

/** Desenha elementos e afins. */
inline void DesenhaElementos(GLenum modo, GLsizei num_vertices, GLenum tipo, const GLvoid* indices) { glDrawElements(modo, num_vertices, tipo, indices); }
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
inline void PonteiroNormais(GLenum tipo, const GLvoid* normais) { PonteiroNormais(tipo, 0, normais); }
void PonteiroTangentes(GLenum tipo, GLsizei passo, const GLvoid* normais);
inline void PonteiroTangentes(GLenum tipo, const GLvoid* tangentes) { PonteiroTangentes(tipo, 0, tangentes); }

void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores);

void PonteiroMatriz(const GLvoid* matriz);
void PonteiroMatrizNormal(const GLvoid* matriz);

enum atributo_e {
  ATR_VERTEX_ARRAY        = 0,
  ATR_NORMAL_ARRAY        = 1,
  ATR_COLOR_ARRAY         = 2,
  ATR_TEXTURE_COORD_ARRAY = 3,
  ATR_MATRIX_ARRAY        = 4,
  ATR_TANGENT_ARRAY       = 5,
};
// Retorna true se o tipo de atributo possui indice (alguns shaders nao tem, ai ja evita muita coisa).
bool HabilitaVetorAtributosVerticePorTipo(atributo_e tipo);
void DesabilitaVetorAtributosVerticePorTipo(atributo_e tipo);

/** Funcoes de nomes. */
void CarregaNome(GLuint nome);
void IniciaNomes();
void EmpilhaNome(GLuint nome);
void DesempilhaNome();

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
void Escala(GLfloat x, GLfloat y, GLfloat z);
void Translada(GLfloat x, GLfloat y, GLfloat z);
void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z);

/** Funcoes de iluminacao. */
void LuzAmbiente(float r, float g, float b);
void LuzDirecional(const GLfloat* pos, float r, float g, float b);
void LuzPontual(GLenum luz, GLfloat* pos, float r, float g, float b, float raio);


/* Liga desliga especularidade. */
void Especularidade(bool ligado);
/** Funcoes de nevoa. */
void Nevoa(GLfloat inicio, GLfloat fim, float r, float g, float b, GLfloat* pos_referencia);
/** Liga e desliga oclusao. */
void Oclusao(bool valor);
bool OclusaoLigada();
/** Passa para o shader o valor do plano de corte distante durante a oclusao. */
void PlanoDistanteOclusao(GLfloat distancia);

/** Funcoes de normais. */
void Normal(GLfloat x, GLfloat y, GLfloat z);
void Tangente(GLfloat x, GLfloat y, GLfloat z);
void MatrizModelagem(const GLfloat* matriz);

void TamanhoPonto(float tam);

/** Retorna o tamanho base da fonte (hoje fixa em 8x13). A escala indica qual deve ser aplicada para ficar legal na tela.
* Ela sera usada por DesenhaString tanto como escala quanto no tamanho do ponto.
*/
void TamanhoFonte(int* largura, int* altura, int* escala);
inline void TamanhoFonteComEscala(int* largura, int* altura) {
  int escala;
  TamanhoFonte(largura, altura, &escala);
  *largura *= escala;
  *altura *= escala;
}
void TamanhoFonte(int largura_vp, int altura_vp, int* largura, int* altura, int* escala);

// Posiciona o raster, usando coordenadas 3d. Caso a projecao falhe ou fique atras, retorna false.
bool PosicaoRaster(GLfloat x, GLfloat y, GLfloat z);
bool PosicaoRaster(GLint x, GLint y);
// Ignora as matrizes para posicionar o raster.
bool PosicaoRasterAbsoluta(GLint x, GLint y);

/** Raster. */
void DesenhaCaractere(char c);
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
#else
enum modo_renderizacao_e {
  MR_RENDER = 0x1C00,
  MR_SELECT = 0x1C02
};
#endif
void BufferSelecao(GLsizei tam_buffer, GLuint* buffer);
GLint ModoRenderizacao(modo_renderizacao_e modo);

/** Mudanca de cor. */
void MudaCor(float r, float g, float b, float a);
inline void MascaraCor(GLboolean mascara) { glColorMask(mascara, mascara, mascara, mascara); }

inline void CorLimpeza(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { glClearColor(r, g, b, a); }
void Limpa(GLbitfield mascara);

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
  // Faz por funcao.
  DesabilitaEscopo(std::function<bool()> query_f, std::function<void(bool)> reset_f) : query_f_(query_f), reset_f_(reset_f) {
    //Le(cap, &valor_anterior_);
    valor_anterior_ = query_f_();
    reset_f_(false);
  }

  ~DesabilitaEscopo() {
    if (valor_anterior_) {
      if (reset_f_) {
        reset_f_(true);
      } else {
        Habilita(cap_);
      }
    }
  }
 private:
  GLenum cap_;
  std::function<bool()> query_f_;
  std::function<void(bool)> reset_f_;
  GLboolean valor_anterior_;
};

struct FramebufferEscopo {
  FramebufferEscopo(GLint framebuffer) {
    Le(GL_FRAMEBUFFER_BINDING, &original_);
    LigacaoComFramebuffer(GL_FRAMEBUFFER, framebuffer);
  }
  ~FramebufferEscopo() {
    LigacaoComFramebuffer(GL_FRAMEBUFFER, original_);
  }
 private:
  GLint original_;
};

/** Stencil. */
inline void FuncaoStencil(GLenum func, GLint ref, GLuint mascara) { glStencilFunc(func, ref, mascara); }

inline void OperacaoStencil(GLenum falha_stencil, GLenum falha_profundidade, GLenum sucesso) {
  glStencilOp(falha_stencil, falha_profundidade, sucesso);
}

/** Retorna true se a selecao por cor estiver sendo usada. */
bool SelecaoPorCor();

constexpr unsigned int BitsPilha() { return 3; }
constexpr unsigned int MaiorBitPilha() { return (1 << BitsPilha()) - 1; }
// Tem que ser 16 ou 8. Ver o mapeamento inverso de cores em gl_comum.cc:
// unsigned int id_mapeado = ...
// Se mudar aqui, tem que mudar o shader de profundidade tambem.
constexpr unsigned int BitsProfundidade() { return 16; }
constexpr unsigned int DeslocamentoPilha() { return 32 - BitsProfundidade() - BitsPilha(); }
constexpr unsigned int NumeroMaximoEntidades() { return (1 << (32 - BitsPilha() - BitsProfundidade())); }
/** O id maximo de entidade que consegue ser mapeado pela selecao por cor. */
constexpr unsigned int IdMaximoEntidade() { return NumeroMaximoEntidades() - 1; }

/** debugging. */
void AlternaModoDebug();

}  // namespace gl

#endif  // GL_GL_H
