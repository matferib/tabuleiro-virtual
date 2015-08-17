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

struct ContextoInterno {
 public:
#if WIN32
  PROC pglGenBuffers;
  PROC pglBindBuffer;
  PROC pglDeleteBuffers;
  PROC pglBufferData;
#endif
  bool depurar_selecao_por_cor = false;  // Mudar para true para depurar selecao por cor.
#if USAR_SHADER
  GLuint programa_luz;
  GLuint vs;
  GLuint fs;
#endif
} g_contexto;

bool ImprimeSeErro();
bool ImprimeSeShaderErro(GLuint shader) {
  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success) {
    return false;
  }
  GLint log_size = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
  std::string info_log;
  info_log.resize(log_size);
  glGetShaderInfoLog(shader, log_size, &log_size, &info_log[0]);
  LOG(ERROR) << "Erro de shader: " << info_log;
  return true;
}

#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)
#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return; } while (0)
void IniciaGl(int* argcp, char** argv) {
  glutInit(argcp, argv);
#if WIN32
  LOG(INFO) << "pegando ponteiros";
  g_contexto.pglGenBuffers = wglGetProcAddress("glGenBuffers");
  std::string erro;
  if (g_contexto.pglGenBuffers == nullptr) {
    erro = "null glGenBuffers";
  }
  g_contexto.pglDeleteBuffers = wglGetProcAddress("glDeleteBuffers");
  if (g_contexto.pglDeleteBuffers == nullptr) {
    erro = "null glDeleteBuffers";
  }
  g_contexto.pglBufferData = wglGetProcAddress("glBufferData");
  if (g_contexto.pglBufferData == nullptr) {
    erro = "null glBufferData";
  }
  g_contexto.pglBindBuffer = wglGetProcAddress("glBindBuffer");
  if (g_contexto.pglBindBuffer == nullptr) {
    erro = "null glBindBuffer";
  }
  if (!erro.empty()) {
    throw std::logic_error(erro);
  }
#endif
#if USAR_SHADER
  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  // Programa de luz.
  {
    GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
    V_ERRO();
    GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    V_ERRO();
    std::string codigo_v_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "vert_luz.c", &codigo_v_shader_str);
    const char* codigo_v_shader = codigo_v_shader_str.c_str();
    glShaderSource(v_shader, 1, &codigo_v_shader, nullptr);
    V_ERRO();
    std::string codigo_f_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "frag_luz.c", &codigo_f_shader_str);
    const char* codigo_f_shader = codigo_f_shader_str.c_str();
    glShaderSource(f_shader, 1, &codigo_f_shader, nullptr);
    V_ERRO();
    glCompileShader(v_shader);
    V_ERRO();
    V_ERRO_SHADER(v_shader);
    glCompileShader(f_shader);
    V_ERRO();
    V_ERRO_SHADER(f_shader);
    GLuint p = glCreateProgram();
    V_ERRO();
    glAttachShader(p, v_shader);
    V_ERRO();
    glAttachShader(p, f_shader);
    V_ERRO();
    glLinkProgram(p);
    V_ERRO();
    glUseProgram(p);
    V_ERRO();
    g_contexto.programa_luz = p;
    g_contexto.vs = v_shader;
    g_contexto.fs = f_shader;
  }
#endif
}
//#undef V_ERRO

void FinalizaGl() {
#if WIN32
  // Apagar o contexto_interno
#endif
#if USAR_SHADER
  glDetachShader(g_contexto.programa_luz, g_contexto.vs);
  glDetachShader(g_contexto.programa_luz, g_contexto.fs);
  glDeleteProgram(g_contexto.programa_luz);
  glDeleteShader(g_contexto.vs);
  glDeleteShader(g_contexto.fs);
#endif
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
  ((PFNGLGENBUFFERSPROC)g_contexto.pglGenBuffers)(n, buffers);
}

void LigacaoComBuffer(GLenum target, GLuint buffer) {
  ((PFNGLBINDBUFFERPROC)g_contexto.pglBindBuffer)(target, buffer);
}

void ApagaBuffers(GLsizei n, const GLuint* buffers) {
  ((PFNGLDELETEBUFFERSPROC)g_contexto.pglDeleteBuffers)(n, buffers);
}

void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
  ((PFNGLBUFFERDATAPROC)g_contexto.pglBufferData)(target, size, data, usage);
}
#endif

void AlternaModoDebug() {
  g_contexto.depurar_selecao_por_cor = !g_contexto.depurar_selecao_por_cor;
}

void Habilita(GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_luz");
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    char nome_var[21];
    snprintf(nome_var, 20, "gltab_luzes[%d]", cap - GL_LIGHT0);
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, nome_var);
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
  } else if (cap == GL_TEXTURE_2D) {
    //LOG_EVERY_N(INFO, 100) << "Ligando GL_TEXTURE_2D";
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_textura");
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
    // Apenas a unidade zero eh usada atualmente.
    loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_unidade_textura");
    if (loc != -1) {
      glUniform1i(loc, 0);
      V_ERRO();
    }
  } else if (cap == GL_FOG) {
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_nevoa");
    if (loc != -1) {
      glUniform1i(loc, 1);
      V_ERRO();
    }
  }
#endif
  glEnable(cap);
}

void Desabilita(GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_luz");
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
    // Nao pode retornar aqui senao a funcao EstaHabilitado falha.
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    char nome_var[21];
    snprintf(nome_var, 20, "gltab_luzes[%d]", cap - GL_LIGHT0);
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, nome_var);
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
  } else if (cap == GL_TEXTURE_2D) {
    //LOG_EVERY_N(INFO, 100) << "Desligando GL_TEXTURE_2D";
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_textura");
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
  } else if (cap == GL_FOG) {
    GLint loc = glGetUniformLocation(g_contexto.programa_luz, "gltab_nevoa");
    if (loc != -1) {
      glUniform1i(loc, 0);
      V_ERRO();
    }
  }
#endif
  glDisable(cap);
}

#if USAR_SHADER
GLint Uniforme(const char* id) {
  GLint ret = glGetUniformLocation(g_contexto.programa_luz, id);
  if (ret == -1) {
    LOG_EVERY_N(INFO, 100) << "Uniforme nao encontrada: " << id;
  }
  return ret;
}
#endif
}  // namespace gl.
