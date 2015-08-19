#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include "gltab/gl.h"
#include "arq/arquivo.h"
#include "log/log.h"

// Comum.
namespace gl {

bool ImprimeSeErro() {
  auto erro = glGetError();
  if (erro != GL_NO_ERROR) {
#if USAR_OPENGL_ES
    LOG(ERROR) << "OpenGL Erro: " << erro;
#else
    LOG(ERROR) << "OpenGL Erro: " << gluErrorString(erro);
#endif
    return true;
  }
  return false;
}
#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)

namespace interno {

const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra) {
  std::vector<std::string> ret;
  if (entrada.empty()) {
    return ret;
  }
  auto it_inicio = entrada.begin();
  auto it = it_inicio;
  while (it != entrada.end()) {
    if (*it == caractere_quebra) {
      ret.push_back(std::string(it_inicio, it));
      it_inicio = ++it;
    } else {
      ++it;
    }
  }
  ret.push_back(std::string(it_inicio, it));
  return ret;
}

// Implementado diferente em cada um.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical);

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
void IniciaShaders(GLuint* programa_luz, GLuint* vs, GLuint* fs) {
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
    *programa_luz = p;
    *vs = v_shader;
    *fs = f_shader;
  }
#endif
}

void FinalizaShaders(GLuint programa_luz, GLuint vs, GLuint fs) {
#if USAR_SHADER
  glDetachShader(programa_luz, vs);
  glDetachShader(programa_luz, fs);
  glDeleteProgram(programa_luz);
  glDeleteShader(vs);
  glDeleteShader(fs);
#endif
}

void HabilitaComShader(GLuint programa_luz, GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_luz");
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    char nome_var[21];
    snprintf(nome_var, 20, "gltab_luzes[%d]", cap - GL_LIGHT0);
    GLint loc = glGetUniformLocation(programa_luz, nome_var);
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
  } else if (cap == GL_TEXTURE_2D) {
    //LOG_EVERY_N(INFO, 100) << "Ligando GL_TEXTURE_2D";
    GLint loc = glGetUniformLocation(programa_luz, "gltab_textura");
    if (loc != -1) {
      glUniform1i(loc, 1);
    }
    // Apenas a unidade zero eh usada atualmente.
    loc = glGetUniformLocation(programa_luz, "gltab_unidade_textura");
    if (loc != -1) {
      glUniform1i(loc, 0);
      V_ERRO();
    }
  } else if (cap == GL_FOG) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_nevoa");
    if (loc != -1) {
      glUniform1i(loc, 1);
      V_ERRO();
    }
  } else if (cap == GL_STENCIL_TEST) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_stencil");
    if (loc != -1) {
      glUniform1i(loc, 1);
      V_ERRO();
    }
  }
#endif
}

void DesabilitaComShader(GLuint programa_luz, GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_luz");
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
    // Nao pode retornar aqui senao a funcao EstaHabilitado falha.
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    char nome_var[21];
    snprintf(nome_var, 20, "gltab_luzes[%d]", cap - GL_LIGHT0);
    GLint loc = glGetUniformLocation(programa_luz, nome_var);
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
  } else if (cap == GL_TEXTURE_2D) {
    //LOG_EVERY_N(INFO, 100) << "Desligando GL_TEXTURE_2D";
    GLint loc = glGetUniformLocation(programa_luz, "gltab_textura");
    if (loc != -1) {
      glUniform1i(loc, 0);
    }
  } else if (cap == GL_FOG) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_nevoa");
    if (loc != -1) {
      glUniform1i(loc, 0);
      V_ERRO();
    }
  } else if (cap == GL_STENCIL_TEST) {
    GLint loc = glGetUniformLocation(programa_luz, "gltab_stencil");
    if (loc != -1) {
      glUniform1i(loc, 0);
      V_ERRO();
    }
  }
#endif
}

}  // interno

// DesligaEscritaProfundidadeEscopo.
DesligaEscritaProfundidadeEscopo::DesligaEscritaProfundidadeEscopo() {
  // Nao funciona com glIsEnabled.
  Le(GL_DEPTH_WRITEMASK, &valor_anterior_);
  MascaraProfundidade(false);
  GLboolean valor_mudado;
  Le(GL_DEPTH_WRITEMASK, &valor_mudado);
}

DesligaEscritaProfundidadeEscopo::~DesligaEscritaProfundidadeEscopo() {
  MascaraProfundidade(valor_anterior_);
  GLboolean valor_mudado;
  Le(GL_DEPTH_WRITEMASK, &valor_mudado);
}

// Sao funcoes iguais dos dois lados que dependem de implementacoes diferentes.
void DesenhaString(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 0, inverte_vertical);
}

void DesenhaStringAlinhadoEsquerda(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, -1, inverte_vertical);
}

void DesenhaStringAlinhadoDireita(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 1, inverte_vertical);
}

}  // namespace gl
