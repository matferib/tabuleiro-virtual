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

namespace interno {
struct ContextoDesktop : public ContextoDependente {
 public:
#if WIN32
  PROC pglGenBuffers;
  PROC pglBindBuffer;
  PROC pglDeleteBuffers;
  PROC pglBufferData;
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

#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)
#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return; } while (0)
void IniciaGl(int* argcp, char** argv) {
  glutInit(argcp, argv);
#if WIN32
  LOG(INFO) << "pegando ponteiros";
  g_contexto.interno->pglGenBuffers = wglGetProcAddress("glGenBuffers");
  std::string erro;
  if (g_context.interno->pglGenBuffers == nullptr) {
    erro = "null glGenBuffers";
  }
  g_contexto.interno->pglDeleteBuffers = wglGetProcAddress("glDeleteBuffers");
  if (g_contexto.interno->pglDeleteBuffers == nullptr) {
    erro = "null glDeleteBuffers";
  }
  g_contexto.interno->pglBufferData = wglGetProcAddress("glBufferData");
  if (g_contexto.interno->pglBufferData == nullptr) {
    erro = "null glBufferData";
  }
  g_contexto.interno->pglBindBuffer = wglGetProcAddress("glBindBuffer");
  if (g_contexto.interno->pglBindBuffer == nullptr) {
    erro = "null glBindBuffer";
  }
  if (!erro.empty()) {
    throw std::logic_error(erro);
  }
#endif
  interno::IniciaShaders(&g_contexto);
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
  gl::CarregaIdentidade();
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
  ((PFNGLGENBUFFERSPROC)g_contexto.interno->pglGenBuffers)(n, buffers);
}

void LigacaoComBuffer(GLenum target, GLuint buffer) {
  ((PFNGLBINDBUFFERPROC)g_contexto.interno->pglBindBuffer)(target, buffer);
}

void ApagaBuffers(GLsizei n, const GLuint* buffers) {
  ((PFNGLDELETEBUFFERSPROC)g_contexto.interno->pglDeleteBuffers)(n, buffers);
}

void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
  ((PFNGLBUFFERDATAPROC)g_contexto.interno->pglBufferData)(target, size, data, usage);
}
#endif

void AlternaModoDebug() {
  g_contexto.depurar_selecao_por_cor = !g_contexto.depurar_selecao_por_cor;
}

void Habilita(GLenum cap) {
  interno::HabilitaComShader(&g_contexto, cap);
  glEnable(cap);
}

void Desabilita(GLenum cap) {
#if USAR_SHADER
  interno::DesabilitaComShader(&g_contexto, cap);
#endif
  glDisable(cap);
}

void MudaCor(float r, float g, float b, float a) {
#if USAR_SHADER
  glVertexAttrib4f(interno::BuscaContexto()->atr_gltab_cor, r, g, b, a);
#else
  glColor4f(r, g, b, a);
#endif
}

GLint Uniforme(const char* id) {
#if USAR_SHADER
  GLint ret = glGetUniformLocation(g_contexto.programa_luz, id);
  if (ret == -1) {
    LOG_EVERY_N(INFO, 100) << "Uniforme nao encontrada: " << id;
  }
  return ret;
#else
  return -1;
#endif
}

}  // namespace gl.
