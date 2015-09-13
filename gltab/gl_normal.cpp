#if WIN32
#include <Windows.h>
#include <Wingdi.h>
#endif

#include <cmath>
#include <vector>
#include <string>
#include "arq/arquivo.h"
#include "gltab/gl.h"
#include "log/log.h"

namespace gl {

#define INTERNO dynamic_cast<interno::ContextoDesktop*>(g_contexto.interno.get())

namespace interno {
struct ContextoDesktop : public ContextoDependente {
 public:
#if WIN32
  PROC pglGenBuffers;
  PROC pglBindBuffer;
  PROC pglDeleteBuffers;
  PROC pglBufferData;

  PROC pglGetShaderiv;
  PROC pglGetShaderInfoLog;
  PROC pglGetProgramiv;
  PROC pglGetActiveUniform;
  PROC pglGetUniformLocation;
  PROC pglVertexAttrib4f;
  PROC pglVertexAttrib3f;
  PROC pglDisableVertexAttribArray;
  PROC pglEnableVertexAttribArray;
  PROC pglUniform4f;
  PROC pglUniform3f;
  PROC pglUniform2f;
  PROC pglUniform1f;
  PROC pglUniform1i;
  PROC pglGetUniformfv;
  PROC pglGetUniformiv;
  PROC pglCreateShader;
  PROC pglDeleteShader;
  PROC pglCompileShader;
  PROC pglAttachShader;
  PROC pglDetachShader;
  PROC pglShaderSource;
  PROC pglLinkProgram;
  PROC pglUseProgram;
  PROC pglCreateProgram;
  PROC pglDeleteProgram;

  PROC pglGetAttribLocation;
  PROC pglVertexAttribPointer;
  PROC pglUniformMatrix4fv;
  PROC pglUniformMatrix3fv;
#endif
};
}  // namespace interno

namespace {
interno::Contexto g_contexto(new interno::ContextoDesktop);
}  // namespace

namespace interno {
Contexto* BuscaContexto() {
    return &g_contexto;
}
}  // namespace interno

bool ImprimeSeErro();
bool ImprimeSeShaderErro(GLuint shader);

#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return; } while (0)
void IniciaGl(int* argcp, char** argv) {
  glutInit(argcp, argv);
#if WIN32
#define PGL_ERRO(x) if (interno->x == nullptr) { erro = "null"#x; }
#define PGL(x) do { interno->p##x = wglGetProcAddress(#x); if (interno->p##x == nullptr) { erro = "null "#x; } } while (0)
  LOG(INFO) << "pegando ponteiros";
  auto* interno = dynamic_cast<interno::ContextoDesktop*>(g_contexto.interno.get());
  std::string erro;
  PGL(glGenBuffers);
  PGL(glDeleteBuffers);
  PGL(glBufferData);
  PGL(glBindBuffer);
  PGL(glGetAttribLocation);
  PGL(glVertexAttrib4f);
  PGL(glVertexAttrib3f);
  PGL(glVertexAttribPointer);
  PGL(glUniformMatrix4fv);
  PGL(glUniformMatrix3fv);
  PGL(glCreateShader);
  PGL(glDeleteShader);
  PGL(glCompileShader);
  PGL(glAttachShader);
  PGL(glDetachShader);
  PGL(glShaderSource);
  PGL(glLinkProgram);
  PGL(glUseProgram);
  PGL(glCreateProgram);
  PGL(glDeleteProgram);
  PGL(glGetUniformfv);
  PGL(glGetUniformiv);
  PGL(glUniform4f);
  PGL(glUniform3f);
  PGL(glUniform2f);
  PGL(glUniform1f);
  PGL(glUniform1i);
  PGL(glEnableVertexAttribArray);
  PGL(glDisableVertexAttribArray);
  PGL(glVertexAttrib4f);
  PGL(glVertexAttrib3f);
  PGL(glGetShaderiv);
  PGL(glGetShaderInfoLog);
  PGL(glGetProgramiv);
  PGL(glGetActiveUniform);
  PGL(glGetUniformLocation);

  if (!erro.empty()) {
    LOG(ERROR) << "Erro: " << erro;
    throw std::logic_error(erro);
  }
#endif
  interno::IniciaComum(interno::LuzPorVertice(*argcp, argv), &g_contexto);
}
//#undef V_ERRO

void FinalizaGl() {
#if WIN32
  // Apagar o contexto_interno
#endif
  interno::FinalizaShaders(g_contexto.programa_luz, g_contexto.vs, g_contexto.fs);
}

namespace interno {

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical) {
  GLboolean raster_valido;
  glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &raster_valido);
  if (!raster_valido) {
    return;
  }

  // Le o raster em coordenadas de janela.
  GLint raster_pos[4];
  glGetIntegerv(GL_CURRENT_RASTER_POSITION, raster_pos);
  // Le viewport.
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  int largura = viewport[2], altura = viewport[3];
  int altura_fonte, largura_fonte;
  TamanhoFonte(&largura_fonte, &altura_fonte);

  // Muda para projecao 2D.
  gl::MatrizEscopo salva_matriz_2(GL_PROJECTION);
  gl::CarregaIdentidade(false);
  gl::Ortogonal(0, largura, 0, altura, 0, 1);
  gl::MatrizEscopo salva_matriz_3(GL_MODELVIEW);
  gl::CarregaIdentidade();
  int x_original = raster_pos[0];
  int y = raster_pos[1];
  std::vector<std::string> str_linhas(interno::QuebraString(str, '\n'));
  for (const std::string& str_linha : str_linhas) {
    if (alinhamento < 0) {
      glRasterPos2i(x_original, y);
    } else if (alinhamento == 0) {
      glRasterPos2i(x_original - (str_linha.size() / 2.0f) * largura_fonte, y);
    } else {
      glRasterPos2i(x_original - (str_linha.size() * largura_fonte), y);
    }
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &raster_valido);
    if (!raster_valido) {
      break;
    }
    for (const char c : str_linha) {
      gl::DesenhaCaractere(c);
    }
    y -= (altura_fonte + 1) * (inverte_vertical ? -1 : 1);
  }
}

}  // namespace

#if WIN32
void GeraBuffers(GLsizei n, GLuint* buffers) {
  ((PFNGLGENBUFFERSPROC)INTERNO->pglGenBuffers)(n, buffers);
}

void LigacaoComBuffer(GLenum target, GLuint buffer) {
  ((PFNGLBINDBUFFERPROC)INTERNO->pglBindBuffer)(target, buffer);
}

void ApagaBuffers(GLsizei n, const GLuint* buffers) {
  ((PFNGLDELETEBUFFERSPROC)INTERNO->pglDeleteBuffers)(n, buffers);
}

void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
  ((PFNGLBUFFERDATAPROC)INTERNO->pglBufferData)(target, size, data, usage);
}

void ShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog) {
  ((PFNGLGETSHADERINFOLOGPROC)INTERNO->pglGetShaderInfoLog)(shader, maxLength, length, infoLog);
}

void ShaderLeParam(GLuint shader, GLenum pname, GLint *params) {
  ((PFNGLGETSHADERIVPROC)INTERNO->pglGetShaderiv)(shader, pname, params);
}

void ProgramaLeParam(GLuint program, GLenum pname, GLint *params) {
  ((PFNGLGETPROGRAMIVPROC)INTERNO->pglGetProgramiv)(program, pname, params);
}

GLint LocalUniforme(GLuint program, const GLchar *name) {
  return ((PFNGLGETUNIFORMLOCATIONPROC)INTERNO->pglGetUniformLocation)(program, name);
}

void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
  ((PFNGLVERTEXATTRIB4FPROC)INTERNO->pglVertexAttrib4f)(index, v0, v1, v2, v3);
}

void AtributoVertice(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2) {
  ((PFNGLVERTEXATTRIB3FPROC)INTERNO->pglVertexAttrib3f)(index, v0, v1, v2);
}

void HabilitaVetorAtributosVertice(GLuint index) {
  ((PFNGLENABLEVERTEXATTRIBARRAYPROC)INTERNO->pglEnableVertexAttribArray)(index);
}

void DesabilitaVetorAtributosVertice(GLuint index) {
  ((PFNGLDISABLEVERTEXATTRIBARRAYPROC)INTERNO->pglDisableVertexAttribArray)(index);
}

void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
  ((PFNGLUNIFORM4FPROC)INTERNO->pglUniform4f)(location, v0, v1, v2, v3);
}

void Uniforme(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
  ((PFNGLUNIFORM3FPROC)INTERNO->pglUniform3f)(location, v0, v1, v2);
}

void Uniforme(GLint location, GLfloat v0, GLfloat v1) {
  ((PFNGLUNIFORM2FPROC)INTERNO->pglUniform2f)(location, v0, v1);
}

void Uniforme(GLint location, GLfloat v0) {
  ((PFNGLUNIFORM1FPROC)INTERNO->pglUniform1f)(location, v0);
}

void Uniforme(GLint location, GLint v0) {
  ((PFNGLUNIFORM1IPROC)INTERNO->pglUniform1i)(location, v0);
}

void LeUniforme(GLuint program, GLint location, GLfloat* params) {
  ((PFNGLGETUNIFORMFVPROC)INTERNO->pglGetUniformfv)(program, location, params);
}

void LeUniforme(GLuint program, GLint location, GLint* params) {
  ((PFNGLGETUNIFORMIVPROC)INTERNO->pglGetUniformiv)(program, location, params);
}

GLuint CriaShader(GLenum shaderType) {
  return ((PFNGLCREATESHADERPROC)INTERNO->pglCreateShader)(shaderType);
}

void DestroiShader(GLuint shader) {
  ((PFNGLDELETESHADERPROC)INTERNO->pglDeleteShader)(shader);
}

void CompilaShader(GLuint shader) {
  ((PFNGLCOMPILESHADERPROC)INTERNO->pglCompileShader)(shader);
}

void AnexaShader(GLuint program, GLuint shader) {
  ((PFNGLATTACHSHADERPROC)INTERNO->pglAttachShader)(program, shader);
}

void DesanexaShader(GLuint program, GLuint shader) {
  ((PFNGLDETACHSHADERPROC)INTERNO->pglDetachShader)(program, shader);
}

void FonteShader(GLuint shader, GLsizei count, const GLchar **s, const GLint *length) {
  ((PFNGLSHADERSOURCEPROC)INTERNO->pglShaderSource)(shader, count, s, length);
}

void LinkaPrograma(GLuint program) {
  ((PFNGLLINKPROGRAMPROC)INTERNO->pglLinkProgram)(program);
}

void UsaPrograma(GLuint program) {
  ((PFNGLUSEPROGRAMPROC)INTERNO->pglUseProgram)(program);
}

GLuint CriaPrograma() {
  return ((PFNGLCREATEPROGRAMPROC)INTERNO->pglCreateProgram)();
}


void DestroiPrograma(GLuint program) {
  ((PFNGLDELETEPROGRAMPROC)INTERNO->pglDeleteProgram)(program);
}

void LeUniformeAtivo(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) {
  ((PFNGLGETACTIVEUNIFORMPROC)INTERNO->pglGetActiveUniform)(program, index, bufSize, length, size, type, name);
}

GLint LeLocalAtributo(GLuint program, const GLchar* name) {
  return ((PFNGLGETATTRIBLOCATIONPROC)INTERNO->pglGetAttribLocation)(program, name);
}

void PonteiroAtributosVertices(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) {
  ((PFNGLVERTEXATTRIBPOINTERPROC)INTERNO->pglVertexAttribPointer)(index, size, type, normalized, stride, pointer);
}

void Matriz3Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  ((PFNGLUNIFORMMATRIX3FVPROC)INTERNO->pglUniformMatrix3fv)(location, count, transpose, value);
}

void Matriz4Uniforme(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  ((PFNGLUNIFORMMATRIX4FVPROC)INTERNO->pglUniformMatrix4fv)(location, count, transpose, value);
}

#endif

void AlternaModoDebug() {
  g_contexto.depurar_selecao_por_cor = !g_contexto.depurar_selecao_por_cor;
}

void Habilita(GLenum cap) {
#if USAR_SHADER
  interno::HabilitaComShader(&g_contexto, cap);
#else
  glEnable(cap);
#endif
}

void Desabilita(GLenum cap) {
#if USAR_SHADER
  interno::DesabilitaComShader(&g_contexto, cap);
#else
  glDisable(cap);
#endif
}

void MudaCor(float r, float g, float b, float a) {
#if USAR_SHADER
  AtributoVertice(interno::BuscaContexto()->atr_gltab_cor, r, g, b, a);
#else
  glColor4f(r, g, b, a);
#endif
}

GLint Uniforme(const char* id) {
#if USAR_SHADER
  GLint ret = LocalUniforme(g_contexto.programa_luz, id);
  if (ret == -1) {
    LOG_EVERY_N(INFO, 100) << "Uniforme nao encontrada: " << id;
  }
  return ret;
#else
  return -1;
#endif
}

}  // namespace gl.
