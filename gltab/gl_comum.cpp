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

bool ImprimeSeErro(const char* mais) {
  auto erro = glGetError();
  if (erro != GL_NO_ERROR) {
#if USAR_OPENGL_ES
    LOG(ERROR) << "OpenGL erro " << (mais == nullptr ? "" : mais) << ": " << erro;
#else
    LOG(ERROR) << "OpenGL erro " << (mais == nullptr ? "" : mais) << ": " << gluErrorString(erro);
#endif
    return true;
  }
  return false;
}
#define V_ERRO() do { if (ImprimeSeErro(nullptr)) return; } while (0)
#define V_ERRO_MAIS(X) do { if (ImprimeSeErro(X)) return; } while (0)

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

#if USAR_SHADER
struct glsl_type_set {
  GLenum      type;
  const char* name;
}
type_set [] = {
  {GL_INVALID_ENUM,                              "invalid" },
  {GL_FLOAT,                                     "float" },
  {GL_FLOAT_VEC2,                                "vec2" },
  {GL_FLOAT_VEC3,                                "vec3" },
  {GL_FLOAT_VEC4,                                "vec4" },
  {GL_INT,                                       "int" },
  {GL_INT_VEC2,                                  "ivec2" },
  {GL_INT_VEC3,                                  "ivec3" },
  {GL_INT_VEC4,                                  "ivec4" },
  {GL_UNSIGNED_INT,                              "unsigned int" },
  {GL_BOOL,                                      "bool" },
  {GL_BOOL_VEC2,                                 "bvec2" },
  {GL_BOOL_VEC3,                                 "bvec3" },
  {GL_BOOL_VEC4,                                 "bvec4" },
  {GL_FLOAT_MAT2,                                "mat2" },
  {GL_FLOAT_MAT3,                                "mat3" },
  {GL_FLOAT_MAT4,                                "mat4" },
#if !USAR_OPENGL_ES
  {GL_FLOAT_MAT2x3,                              "mat2x3" },
  {GL_FLOAT_MAT2x4,                              "mat2x4" },
  {GL_FLOAT_MAT3x2,                              "mat3x2" },
  {GL_FLOAT_MAT3x4,                              "mat3x4" },
  {GL_FLOAT_MAT4x2,                              "mat4x2" },
  {GL_FLOAT_MAT4x3,                              "mat4x3" },
  {GL_SAMPLER_1D,                                "sampler1D" },
#endif
  {GL_SAMPLER_2D,                                "sampler2D" },
#if !USAR_OPENGL_ES
  {GL_SAMPLER_3D,                                "sampler3D" },
#endif
  {GL_SAMPLER_CUBE,                              "samplerCube" },
#if !USAR_OPENGL_ES
  {GL_SAMPLER_1D_SHADOW,                         "sampler1DShadow" },
  {GL_SAMPLER_2D_SHADOW,                         "sampler2DShadow" },
#endif
};
void print_uniforms(GLuint program) {
  GLint uniform_count;
  glGetProgramiv (program, GL_ACTIVE_UNIFORMS, &uniform_count);

  GLchar name [256];

  for (GLint i = 0; i < uniform_count; i++) {
    memset (name, '\0', 256);
    GLint  size;
    GLenum type;

    glGetActiveUniform (program, i, 255, NULL, &size, &type, name);

    GLint location = glGetUniformLocation (program, name);

    for (int j = 0; j < sizeof (type_set) / sizeof (glsl_type_set); j++) {
      if (type_set [j].type != type)
        continue;

      const char* type_name = type_set [j].name;

      if (size > 1)
        printf ( "Uniform %d (loc=%d):\t%20s %-20s <Size: %d>\n",
                   i, location, type_name, name, size );
      else
        printf ( "Uniform %d (loc=%d):\t%20s %-20s\n",
                   i, location, type_name, name );

      break;
    }

    if (i == (uniform_count - 1))
      printf ("\n");
  }
}
#endif

#if USAR_SHADER
// Funcoes para buscar o indice do uniforme uni_gltab_luzes.
// id_luz [0, 7].
int IndiceLuzPos(int id_luz) {
  return id_luz * 3;
}

int IndiceLuzCor(int id_luz) {
  return id_luz * 3 + 1;
}

int IndiceLuzAtributos(int id_luz) {
  return id_luz * 3 + 2;
}
#endif

#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return; } while (0)
void IniciaShaders(interno::Contexto* contexto) {
#if USAR_SHADER
  GLuint* programa_luz = &contexto->programa_luz;
  GLuint* vs = &contexto->vs;
  GLuint* fs = &contexto->fs;

  V_ERRO_MAIS("antes vertex shader");
  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  // Programa de luz.
  {
    GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
    V_ERRO_MAIS("criando vertex shader");
    GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    V_ERRO_MAIS("criando fragment shader");
    std::string codigo_v_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "vert_luz.c", &codigo_v_shader_str);
    const char* codigo_v_shader = codigo_v_shader_str.c_str();
    glShaderSource(v_shader, 1, &codigo_v_shader, nullptr);
    V_ERRO_MAIS("shader source vertex");
    std::string codigo_f_shader_str;
    arq::LeArquivo(arq::TIPO_SHADER, "frag_luz.c", &codigo_f_shader_str);
    const char* codigo_f_shader = codigo_f_shader_str.c_str();
    glShaderSource(f_shader, 1, &codigo_f_shader, nullptr);
    V_ERRO_MAIS("shader source fragment");
    glCompileShader(v_shader);
    V_ERRO_SHADER(v_shader);
    glCompileShader(f_shader);
    V_ERRO_SHADER(f_shader);
    GLuint p = glCreateProgram();
    V_ERRO_MAIS("criando programa shader");
    glAttachShader(p, v_shader);
    V_ERRO_MAIS("atachando vertex no programa shader");
    glAttachShader(p, f_shader);
    V_ERRO_MAIS("atachando fragment no programa shader");
    glLinkProgram(p);
    V_ERRO_MAIS("linkando programa shader");
    glUseProgram(p);
    V_ERRO_MAIS("usando programa shader");
    *programa_luz = p;
    *vs = v_shader;
    *fs = f_shader;
  }

  print_uniforms(*programa_luz);
  // Variaveis do shader.
  struct DadosVariavel {
    const char* nome;
    GLint* var;
  };

  // Variaveis uniformes.
  for (const auto& d : std::vector<DadosVariavel> {
          {"gltab_luz", &contexto->uni_gltab_luz },
          {"gltab_luz_ambiente", &contexto->uni_gltab_luz_ambiente_cor },
          {"gltab_luz_direcional.cor", &contexto->uni_gltab_luz_direcional_cor },
          {"gltab_luz_direcional.pos", &contexto->uni_gltab_luz_direcional_pos },
          {"gltab_textura", &contexto->uni_gltab_textura },
          {"gltab_unidade_textura", &contexto->uni_gltab_unidade_textura },
          {"gltab_nevoa_dados", &contexto->uni_gltab_nevoa_dados },
          {"gltab_nevoa_cor", &contexto->uni_gltab_nevoa_cor},
          {"gltab_nevoa_referencia", &contexto->uni_gltab_nevoa_referencia },
  }) {
    *d.var = glGetUniformLocation(*programa_luz, d.nome);
    if (*d.var == -1) {
      LOG(ERROR) << "Erro lendo uniforme " << d.nome;
    }
  }
  // Uniformes array.
  for (int i = 0; i < 7; ++i) {
    int j = 0;
    for (const char* sub_var : std::vector<const char*>{"pos", "cor", "atributos"}) {
      int pos = i * 3 + j;
      char nome_var[100];
      snprintf(nome_var, sizeof(nome_var), "gltab_luzes[%d].%s", i, sub_var);
      contexto->uni_gltab_luzes[pos] = glGetUniformLocation(*programa_luz, nome_var);
      if (contexto->uni_gltab_luzes[pos] == -1) {
        LOG(ERROR) << "Erro lendo uniforme " << nome_var;
      }
      ++j;
    }
  }

  // Variaveis atributos.
  for (const auto& d : std::vector<DadosVariavel> {
          {"gltab_vertice", &contexto->atr_gltab_vertice},
          {"gltab_normal", &contexto->atr_gltab_normal},
          {"gltab_cor", &contexto->atr_gltab_cor},
          {"gltab_textura", &contexto->atr_gltab_textura},
  }) {
    *d.var = glGetAttribLocation(*programa_luz, d.nome);
    if (*d.var == -1) {
      LOG(ERROR) << "Erro lendo atributo " << d.nome;
      continue;
    }
    LOG(INFO) << "Atributo " << d.nome << " na posicao " << *d.var;
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
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = contexto->uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = contexto->uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_TEXTURE_2D) {
    glUniform1i(contexto->uni_gltab_textura, 1);
    glUniform1i(contexto->uni_gltab_unidade_textura, 0);  // A unidade de textura usada sempre eh zero.
  } else if (cap == GL_FOG) {
    GLint uniforme = contexto->uni_gltab_nevoa_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_nevoa_cor, cor[0], cor[1], cor[2], 1.0f);
  } else {
    return glEnable(cap);
  }
#endif
}

void DesabilitaComShader(interno::Contexto* contexto, GLenum cap) {
#if USAR_SHADER
  if (cap == GL_LIGHTING) {
     glUniform1i(contexto->uni_gltab_luz, 0);
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = contexto->uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = contexto->uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_TEXTURE_2D) {
    glUniform1i(contexto->uni_gltab_textura, 0);
  } else if (cap == GL_FOG) {
    GLint uniforme = contexto->uni_gltab_nevoa_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    glUniform4f(contexto->uni_gltab_nevoa_cor, cor[0], cor[1], cor[2], 0.0f);
  } else {
    glDisable(cap);
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

void PonteiroVertices(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
#if USAR_SHADER
  glVertexAttribPointer(interno::BuscaContexto()->atr_gltab_vertice, vertices_por_coordenada, tipo, GL_FALSE, passo, vertices);
  //V_ERRO_MAIS("pointeiro vertices");
#else
  glVertexPointer(vertices_por_coordenada, tipo, passo, vertices);
#endif
}

void PonteiroNormais(GLenum tipo, GLsizei passo, const GLvoid* normais) {
#if USAR_SHADER
  glVertexAttribPointer(interno::BuscaContexto()->atr_gltab_normal, 3  /**dimensoes*/, tipo, GL_FALSE, passo, normais);
  //V_ERRO_MAIS("pointeiro normais");
#else
#endif
  glNormalPointer(tipo, passo, normais);
}

void Normal(GLfloat x, GLfloat y, GLfloat z) {
#if USAR_SHADER
  glVertexAttrib3f(interno::BuscaContexto()->atr_gltab_normal, x, y, z);
#else
  glNormal3f(x, y, z);
#endif
}

void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores) {
#if USAR_SHADER
  glVertexAttribPointer(interno::BuscaContexto()->atr_gltab_cor, 4  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, cores);
#else
  glColorPointer(num_componentes, GL_FLOAT, passo, cores);
#endif
}

void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
#if USAR_SHADER
  glVertexAttribPointer(interno::BuscaContexto()->atr_gltab_textura, 2  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, vertices);
#else
  glTexCoordPointer(vertices_por_coordenada, tipo, passo, vertices);
#endif
}

bool EstaHabilitado(GLenum cap) {
#if USAR_SHADER
  auto* contexto = interno::BuscaContexto();
  GLint ret = 0;
  if (cap == GL_LIGHTING) {
    glGetUniformiv(contexto->programa_luz, contexto->uni_gltab_luz, &ret);
    return ret;
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = contexto->uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    return cor[3] > 0;
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = contexto->uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    return cor[3] > 0;
  } else if (cap == GL_TEXTURE_2D) {
    glGetUniformiv(contexto->programa_luz, contexto->uni_gltab_textura, &ret);
    return ret;
  } else if (cap == GL_FOG) {
    GLint uniforme = contexto->uni_gltab_nevoa_cor;
    GLfloat cor[4];
    glGetUniformfv(contexto->programa_luz, uniforme, cor);
    return cor[3] > 0;
  }
  return glIsEnabled(cap);
#else
  return glIsEnabled(cap);
#endif
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

void LuzAmbiente(float r, float g, float b) {
#if USAR_SHADER
  glUniform4f(interno::BuscaContexto()->uni_gltab_luz_ambiente_cor, r, g, b, 1.0f);
#else
  GLfloat glparams[4] = { r, g, b, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glparams);
#endif
}

void LuzDirecional(const GLfloat* pos, float r, float g, float b) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  glUniform4f(c->uni_gltab_luz_direcional_cor, r, g, b, 1.0f);
  // Transforma o vetor em coordenadas da camera.
  float m[16];
  gl::Le(GL_MODELVIEW_MATRIX, m);
  Matrix3 nm(m[0], m[1], m[2],
             m[4], m[5], m[6],
             m[8], m[9], m[10]);
  nm.invert().transpose();
  Vector3 vp(pos[0], pos[1], pos[2]);
  vp = nm * vp;
  //LOG(ERROR) << "vp: " << vp.x << ", " << vp.y << ", " << vp.z;
  glUniform4f(c->uni_gltab_luz_direcional_pos, vp.x, vp.y, vp.z, 1.0f);
#else
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  GLfloat cor[] = { r, g, b, 1.0f };
  glLightfv(GL_LIGHT0, GL_DIFFUSE, cor);
#endif
}

void LuzPontual(GLenum luz, GLfloat* pos, float r, float g, float b, float atenuacao_constante, float atenuacao_quadratica) {
#if USAR_SHADER
  if (luz <= 0 || luz > 8) {
    LOG(ERROR) << "Luz invalida: " << luz;
    return;
  }
  auto* c = interno::BuscaContexto();
  // Transforma a posicao em coordenadas da camera.
  float glm[16];
  gl::Le(GL_MODELVIEW_MATRIX, glm);
  Matrix4 m(glm);
  Vector4 vp(pos[0], pos[1], pos[2], 1.0f);
  vp = m * vp;
  glUniform4f(c->uni_gltab_luzes[interno::IndiceLuzPos(luz - 1)], vp.x, vp.y, vp.z, 1.0f);
  glUniform4f(c->uni_gltab_luzes[interno::IndiceLuzCor(luz - 1)], r, g, b, 1.0f);
  glUniform4f(c->uni_gltab_luzes[interno::IndiceLuzAtributos(luz - 1)], 6.0f  /*raio*/, 0, 0, 0);
#else
  gl::Luz(GL_LIGHT0 + luz, GL_POSITION, pos);
  GLfloat cor_luz[] = { r, g, b, 1.0f };
  gl::Luz(GL_LIGHT0 + luz, GL_DIFFUSE, cor_luz);
  gl::Luz(GL_LIGHT0 + luz, GL_CONSTANT_ATTENUATION, atenuacao_constante);
  gl::Luz(GL_LIGHT0 + luz, GL_QUADRATIC_ATTENUATION, atenuacao_quadratica);
#endif
}

void Nevoa(GLfloat inicio, GLfloat fim, float r, float g, float b, GLfloat* pos_referencia) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  GLfloat glm[16];
  gl::Le(GL_MODELVIEW_MATRIX, glm);
  Matrix4 m(glm);
  Vector4 v(pos_referencia[0], pos_referencia[1], pos_referencia[2], 1.0f);
  v = m * v;
  glUniform4f(c->uni_gltab_nevoa_referencia, v.x, v.y, v.z, 1.0f);
  glUniform4f(c->uni_gltab_nevoa_dados, inicio, fim, 0.0f  /*nao usado*/, (1.0f / (fim - inicio))  /*escala*/);
  glUniform4f(c->uni_gltab_nevoa_cor, r, g, b, 1.0f);
#else
  glFogf(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, inicio);
  glFogf(GL_FOG_END, fim);
  GLfloat cor[] = { r, g, b, 1.0f };
  glFogfv(GL_FOG_COLOR, cor);
#endif
}

#if USAR_SHADER
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
#endif

}  // namespace gl
