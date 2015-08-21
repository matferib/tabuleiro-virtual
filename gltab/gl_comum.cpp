#include <cmath>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "gltab/gl.h"
#include "arq/arquivo.h"
#include "log/log.h"
#include "matrix/matrices.h"

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
#if USAR_SHADER
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
#endif
  return true;
}

#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)
#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return; } while (0)
void IniciaShaders(interno::Contexto* contexto) {
#if USAR_SHADER
  GLuint* programa_luz = &contexto->programa_luz;
  GLuint* vs = &contexto->vs;
  GLuint* fs = &contexto->fs;

  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  // Programa de luz.
  {
    GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
    if (v_shader == 0) {
      LOG(ERROR) << "Erro criando vertex shader";
      V_ERRO();
      return;
    }
    //V_ERRO();
    GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (f_shader == 0) {
      LOG(ERROR) << "Erro criando fragment shader";
      V_ERRO();
      return;
    }
    //V_ERRO();
    std::string codigo_v_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "vert_luz.c", &codigo_v_shader_str);
    const char* codigo_v_shader = codigo_v_shader_str.c_str();
    glShaderSource(v_shader, 1, &codigo_v_shader, nullptr);
    //V_ERRO();
    std::string codigo_f_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "frag_luz.c", &codigo_f_shader_str);
    const char* codigo_f_shader = codigo_f_shader_str.c_str();
    glShaderSource(f_shader, 1, &codigo_f_shader, nullptr);
    //V_ERRO();
    glCompileShader(v_shader);
    //V_ERRO();
    V_ERRO_SHADER(v_shader);
    glCompileShader(f_shader);
    //V_ERRO();
    V_ERRO_SHADER(f_shader);
    GLuint p = glCreateProgram();
    if (p == 0) {
      LOG(ERROR) << "Erro criando programa de shader.";
      V_ERRO();
      return;
    }
    //V_ERRO();
    glAttachShader(p, v_shader);
    //V_ERRO();
    glAttachShader(p, f_shader);
    //V_ERRO();
    glLinkProgram(p);
    //V_ERRO();
    glUseProgram(p);
    //V_ERRO();
    *programa_luz = p;
    *vs = v_shader;
    *fs = f_shader;
  }

  // Variaveis uniformes.
  for (const auto& par : std::vector<std::pair<std::string, GLint*>> {
          {"gltab_luz", &contexto->uni_gltab_luz },
          {"gltab_textura", &contexto->uni_gltab_textura },
          {"gltab_unidade_textura", &contexto->uni_gltab_unidade_textura },
          {"gltab_nevoa", &contexto->uni_gltab_nevoa },
          {"gltab_stencil", &contexto->uni_gltab_stencil }}) {
    *par.second = glGetUniformLocation(*programa_luz, par.first.c_str());
    if (*par.second == -1) {
      LOG(ERROR) << "Erro lendo uniforme " << par.first;
    }
  }
  // Luzes.
  for (int i = 0; i < 8; ++i) {
    char nome_var[21];
    snprintf(nome_var, 20, "gltab_luzes[%d]", i);
    contexto->uni_gltab_luzes[i] = glGetUniformLocation(*programa_luz, nome_var);
    if (contexto->uni_gltab_luzes[i] == -1) {
      LOG(ERROR) << "Erro lendo uniforme " << nome_var;
    }
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

void HabilitaComShader(interno::Contexto* contexto, GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
     glUniform1i(contexto->uni_gltab_luz, 1);
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    glUniform1i(contexto->uni_gltab_luzes[cap - GL_LIGHT0], 1);
  } else if (cap == GL_TEXTURE_2D) {
    glUniform1i(contexto->uni_gltab_textura, 1);
    glUniform1i(contexto->uni_gltab_unidade_textura, 0);  // A unidade de textura usada sempre eh zero.
  } else if (cap == GL_FOG) {
    glUniform1i(contexto->uni_gltab_nevoa, 1);
  } else if (cap == GL_STENCIL_TEST) {
    glUniform1i(contexto->uni_gltab_stencil, 1);
  }
#endif
}

void DesabilitaComShader(interno::Contexto* contexto, GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
     glUniform1i(contexto->uni_gltab_luz, 0);
  } else if (cap >= GL_LIGHT0 && cap <= GL_LIGHT7) {
    glUniform1i(contexto->uni_gltab_luzes[cap - GL_LIGHT0], 0);
  } else if (cap == GL_TEXTURE_2D) {
    glUniform1i(contexto->uni_gltab_textura, 0);
  } else if (cap == GL_FOG) {
    glUniform1i(contexto->uni_gltab_nevoa, 0);
  } else if (cap == GL_STENCIL_TEST) {
    glUniform1i(contexto->uni_gltab_stencil, 0);
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

void AtualizaMatrizes() {
  GLenum modo;
  glGetIntegerv(GL_MATRIX_MODE, (GLint*)&modo);
  GLuint mloc = (modo == GL_MODELVIEW) ? gl::Uniforme("gltab_mvm") : gl::Uniforme("gltab_prm");
  if (mloc == -1) {
    return;
  }
  float m[16];
  gl::Le(modo == GL_MODELVIEW ? GL_MODELVIEW_MATRIX : GL_PROJECTION_MATRIX, m);
  glUniformMatrix4fv(mloc, 1, false, m);
  if (modo == GL_PROJECTION) {
#if 0
    static int done = 0;
    if (++done < 10) {
      LOG(INFO) << "mopengl: \n" <<
        m[0] << " "  << m[1] << " " << m[2]   << " " << m[3] << "\n" <<
        m[4] << " "  << m[5] << " " << m[6]   << " " << m[7] << "\n" <<
        m[8] << " "  << m[9] << " " << m[10]  << " " << m[11] << "\n" <<
        m[12] << " " << m[13] << " " << m[14] << " " << m[15] << "\n";

      /*
      LOG(INFO) << "mmatrices: \n" <<
        m[0] << " "  << m[1] << " " << m[2]   << " " m[3] << "\n" <<
        m[4] << " "  << m[5] << " " << m[6]   << " " m[7] << "\n" <<
        m[8] << " "  << m[9] << " " << m[10]  << " " m[11] << "\n" <<
        m[12] << " " << m[13] << " " << m[14] << " " m[15] << "\n";
        */
    }
#endif
    return;
  }

  // Normal matrix.
  //mat4 normalMatrix = transpose(inverse(modelView));
  Matrix3 normal(m[0], m[1], m[2],
                 m[4], m[5], m[6],
                 m[8], m[9], m[10]);
  normal.invert().transpose();
  GLuint nmloc = gl::Uniforme("gltab_nm");
  if (nmloc == -1) {
    return;
  }
  glUniformMatrix3fv(nmloc, 1, false, normal.get());
}

void DebugaMatrizes() {
  float mv[16];
  gl::Le(GL_MODELVIEW_MATRIX, mv);
  Matrix3 normal(mv[0], mv[1], mv[2],
                 mv[4], mv[5], mv[6],
                 mv[8], mv[9], mv[10]);
  normal.invert().transpose();
  LOG_EVERY_N(INFO, 300) << "MV: \n" << Matrix4(mv)
                          << ", NM: \n" << normal;
}

}  // namespace gl
