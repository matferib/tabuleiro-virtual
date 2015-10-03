#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include "gltab/gl_interno.h"
#include "gltab/glues.h"
#include "arq/arquivo.h"
//#define VLOG_NIVEL 2
#include "log/log.h"
#include "matrix/matrices.h"

#if !USAR_OPENGL_ES && !WIN32
DEFINE_bool(luz_por_vertice, false, "Se verdadeiro, usa iluminacao por vertice.");
#endif

using gl::TSH_LUZ;
using gl::TSH_SIMPLES;
using gl::TSH_PROFUNDIDADE;
using gl::TSH_PRETO_BRANCO;

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

namespace interno {

void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  auto* c = BuscaContexto();
  unsigned int id_mapeado = c->proximo_id | (c->bit_pilha << 21);
  c->ids.insert(std::make_pair(id_mapeado, id));
  if (c->proximo_id == ((1 << 21) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    if (c->depurar_selecao_por_cor) {
      // Mais facil de ver.
      c->proximo_id += 5;
    } else {
      ++c->proximo_id;
    }
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

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

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical) {
  // Melhor deixar comentado assim para as letras ficarem sempre em primeiro plano.
  //gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  //gl::DesligaEscritaProfundidadeEscopo mascara_escopo;
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade(false);
  gl::Ortogonal(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, 1.0f);
  gl::MatrizEscopo salva_matriz_proj(GL_MODELVIEW);
  gl::CarregaIdentidade(false);

  int largura_fonte, altura_fonte, escala;
  TamanhoFonte(&largura_fonte, &altura_fonte, &escala);

  auto* contexto = BuscaContexto();
  float x2d = contexto->raster_x;
  float y2d = contexto->raster_y;
  gl::Translada(x2d, y2d, 0.0f, false);

  //LOG(INFO) << "x2d: " << x2d << " y2d: " << y2d;
  std::vector<std::string> str_linhas(interno::QuebraString(str, '\n'));
  gl::TamanhoPonto(escala);
  gl::Escala(escala, escala, 1.0f);
  for (const std::string& str_linha : str_linhas) {
    float translacao_x = 0;
    if (alinhamento == 1) {  // direita.
      translacao_x = -static_cast<float>(str_linha.size() * largura_fonte);
    } if (alinhamento == 0) {  // central.
      translacao_x = -static_cast<float>(str_linha.size() * largura_fonte) / 2.0f;
    }
    gl::Translada(translacao_x, 0.0f, 0.0f, false);
    for (const char c : str_linha) {
      gl::DesenhaCaractere(c);
      gl::Translada(largura_fonte, 0.0f, 0.0f, false);
    }
    // A translacao volta tudo que ela andou.
    gl::Translada(-static_cast<float>(str_linha.size() * largura_fonte) - translacao_x,
                  inverte_vertical ? altura_fonte : -altura_fonte,
                  0.0f,
                  false);
  }
}


bool ImprimeSeShaderErro(GLuint shader) {
#if USAR_SHADER
  GLint success = 0;
  ShaderLeParam(shader, GL_COMPILE_STATUS, &success);
  if (success) {
    return false;
  }
  GLint log_size = 0;
  ShaderLeParam(shader, GL_INFO_LOG_LENGTH, &log_size);
  std::string info_log;
  info_log.resize(log_size);
  ShaderInfoLog(shader, log_size, &log_size, &info_log[0]);
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
  {GL_SAMPLER_2D,                                "sampler2D" },
  {GL_SAMPLER_CUBE,                              "samplerCube" },
};
void print_uniforms(GLuint program) {
  GLint uniform_count;
  ProgramaLeParam(program, GL_ACTIVE_UNIFORMS, &uniform_count);

  GLchar name [256];

  for (GLint i = 0; i < uniform_count; i++) {
    memset (name, '\0', 256);
    GLint  size;
    GLenum type;

    LeUniformeAtivo(program, i, 255, NULL, &size, &type, name);

    GLint location = LocalUniforme (program, name);

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

namespace {
#define V_ERRO_SHADER(s) do { if (ImprimeSeShaderErro(s)) return false; } while (0)
bool IniciaShader(const char* nome_programa, const char* nome_vs, const char* nome_fs,
                  VarShader* shader) {
  shader->nome = nome_programa;
  GLuint v_shader = CriaShader(GL_VERTEX_SHADER);
  V_ERRO_RET("criando vertex shader");
  GLuint f_shader = CriaShader(GL_FRAGMENT_SHADER);
  V_ERRO_RET("criando fragment shader");
  std::string codigo_v_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_vs, &codigo_v_shader_str);
  const char* codigo_v_shader = codigo_v_shader_str.c_str();
  FonteShader(v_shader, 1, &codigo_v_shader, nullptr);
  V_ERRO_RET("shader source vertex");
  std::string codigo_f_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_fs, &codigo_f_shader_str);
  const char* codigo_f_shader = codigo_f_shader_str.c_str();
  FonteShader(f_shader, 1, &codigo_f_shader, nullptr);
  V_ERRO_RET("shader source fragment");
  CompilaShader(v_shader);
  V_ERRO_SHADER(v_shader);
  CompilaShader(f_shader);
  V_ERRO_SHADER(f_shader);
  GLuint p = CriaPrograma();
  V_ERRO_RET("criando programa shader");
  AnexaShader(p, v_shader);
  V_ERRO_RET("atachando vertex no programa shader");
  AnexaShader(p, f_shader);
  V_ERRO_RET("atachando fragment no programa shader");
  LinkaPrograma(p);
  V_ERRO_RET("linkando programa shader");
  shader->programa = p;
  shader->vs = v_shader;
  shader->fs = f_shader;
  return true;
}

bool IniciaVariaveis(VarShader* shader) {
  // Variaveis do shader.
  struct DadosVariavel {
    const char* nome;
    GLint* var;
  };
  // Variaveis uniformes.
  for (const auto& d : std::vector<DadosVariavel> {
          {"gltab_luz_ambiente", &shader->uni_gltab_luz_ambiente_cor },
          {"gltab_luz_direcional.cor", &shader->uni_gltab_luz_direcional_cor },
          {"gltab_luz_direcional.pos", &shader->uni_gltab_luz_direcional_pos },
          {"gltab_textura", &shader->uni_gltab_textura },
          {"gltab_unidade_textura", &shader->uni_gltab_unidade_textura },
          {"gltab_nevoa_dados", &shader->uni_gltab_nevoa_dados },
          {"gltab_nevoa_cor", &shader->uni_gltab_nevoa_cor},
          {"gltab_nevoa_referencia", &shader->uni_gltab_nevoa_referencia },
          {"gltab_mvm", &shader->uni_gltab_mvm },
          {"gltab_prm", &shader->uni_gltab_prm },
          {"gltab_nm", &shader->uni_gltab_nm },
          {"gltab_dados_raster", &shader->uni_gltab_dados_raster},
  }) {
    *d.var = LocalUniforme(shader->programa, d.nome);
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
      shader->uni_gltab_luzes[pos] = LocalUniforme(shader->programa, nome_var);
      if (shader->uni_gltab_luzes[pos] == -1) {
        LOG(ERROR) << "Erro lendo uniforme " << nome_var;
      }
      ++j;
    }
  }

  // Variaveis atributos.
  for (const auto& d : std::vector<DadosVariavel> {
          {"gltab_vertice", &shader->atr_gltab_vertice},
          {"gltab_normal", &shader->atr_gltab_normal},
          {"gltab_cor", &shader->atr_gltab_cor},
          {"gltab_texel", &shader->atr_gltab_texel},
  }) {
    *d.var = LeLocalAtributo(shader->programa, d.nome);
    if (*d.var == -1) {
      LOG(ERROR) << "Erro lendo atributo " << d.nome;
      continue;
    }
    LOG(INFO) << "Atributo " << d.nome << " na posicao " << *d.var;
  }
  return true;
}

void IniciaShaders(bool luz_por_vertice, interno::Contexto* contexto) {
  LOG(INFO) << "Tentando iniciar com shaders, luz por fragmento? " << !luz_por_vertice;

  V_ERRO("antes vertex shader");
  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  struct DadosShaders {
    std::string nome_programa;
    std::string nome_vs;
    std::string nome_fs;
    VarShader* shader;
  };
  std::vector<DadosShaders> dados_shaders = {
    { luz_por_vertice ? "programa_luz_vertice" : "programa_luz_pixel",
      luz_por_vertice ? "vert_luz_por_vertice.c" : "vert_luz.c",
      luz_por_vertice ? "frag_luz_por_vertice.c" : "frag_luz.c",
      &contexto->shaders[TSH_LUZ] },
    { "programa_simples", "vert_simples.c", "frag_simples.c", &contexto->shaders[TSH_SIMPLES] },
    { "programa_profundidade", "vert_simples.c", "frag_profundidade.c", &contexto->shaders[TSH_PROFUNDIDADE] },
    { "programa_preto_branco", "vert_preto_branco.c", "frag_preto_branco.c", &contexto->shaders[TSH_PRETO_BRANCO] },
  };

  for (auto& ds : dados_shaders) {
    LOG(INFO) << "Iniciando programa shaders: " << ds.nome_programa.c_str();
    if (!IniciaShader(ds.nome_programa.c_str(), ds.nome_vs.c_str(), ds.nome_fs.c_str(), ds.shader)) {
      LOG(ERROR) << "Erro carregando programa com " << ds.nome_vs.c_str() << " e " << ds.nome_fs.c_str();
      continue;
    }
    if (!IniciaVariaveis(ds.shader)) {
      LOG(ERROR) << "Erro carregando variaveis do programa " << ds.nome_programa.c_str();
      continue;
    }
    print_uniforms(ds.shader->programa);
    LOG(INFO) << "Programa shaders '" << ds.nome_programa.c_str() << "' iniciado com sucesso";
  }
  UsaShader(TSH_LUZ);
  V_ERRO("usando programa shader");
}

}  // namespace
#endif


void IniciaComum(bool luz_por_vertice, interno::Contexto* contexto) {
  contexto->pilha_mvm.push(Matrix4());
  contexto->pilha_prj.push(Matrix4());
  contexto->pilha_corrente = &contexto->pilha_mvm;
#if USAR_SHADER
  // Essa funcao pode dar excecao, entao eh melhor colocar depois das matrizes pra aplicacao nao crashar e mostrar
  // a mensagem de erro.
  IniciaShaders(luz_por_vertice, contexto);
#endif
}

void FinalizaShaders(const VarShader& shader) {
#if USAR_SHADER
  DesanexaShader(shader.programa, shader.vs);
  DesanexaShader(shader.programa, shader.fs);
  DestroiPrograma(shader.programa);
  DestroiShader(shader.vs);
  DestroiShader(shader.fs);
#endif
}

void HabilitaComShader(interno::Contexto* contexto, GLenum cap) {
#if USAR_SHADER
  const auto& shader = BuscaShader();
  if (cap == GL_LIGHTING) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(uniforme, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_LIGHT0) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_TEXTURE_2D) {
    Uniforme(shader.uni_gltab_textura, 1.0f);
    Uniforme(shader.uni_gltab_unidade_textura, 0);  // A unidade de textura usada sempre eh zero.
  } else if (cap == GL_FOG) {
    if (!UsandoShaderComNevoa()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_nevoa_cor, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_NORMALIZE) {
    // Shader ja normaliza tudo.
  } else {
    return glEnable(cap);
  }
#endif
}

void DesabilitaComShader(interno::Contexto* contexto, GLenum cap) {
#if USAR_SHADER
  const auto& shader = BuscaShader();
  if (cap == GL_LIGHTING) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(uniforme, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_LIGHT0) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    if (!UsandoShaderLuz()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_TEXTURE_2D) {
    Uniforme(shader.uni_gltab_textura, 0.0f);
  } else if (cap == GL_FOG) {
    if (!UsandoShaderComNevoa()) {
      return;
    }
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_nevoa_cor, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_NORMALIZE) {
    // Shader ja normaliza tudo.
  } else {
    glDisable(cap);
  }
#endif
}

bool LuzPorVertice(int argc, const char* const* argv) {
#if !USAR_OPENGL_ES && !WIN32
  return FLAGS_luz_por_vertice;
#else
  for (int i = 0; i < argc; ++i) {
    if (std::string(argv[i]) == "--luz_por_vertice") {
      return true;
    }
  }
  return false;
#endif
}

}  // namespace interno

void EmpilhaMatriz(bool atualizar) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  Matrix4 m(c->pilha_corrente->top());
  c->pilha_corrente->push(m);
  // Nao precisa porque a matriz empilhada eh igual.
  //if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glPushMatrix();
#endif
}

void DesempilhaMatriz(bool atualizar) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
#if DEBUG
  if (c->pilha_corrente->empty()) {
    LOG(ERROR) << "Pilha vazia";
    return;
  }
#endif
  c->pilha_corrente->pop();
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glPopMatrix();
#endif
}

GLenum ModoMatrizCorrente() {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  return (c->pilha_corrente == &c->pilha_mvm) ? GL_MODELVIEW : GL_PROJECTION;
#else
  GLenum ret;
  glGetIntegerv(GL_MATRIX_MODE, (GLint*)&ret);
  return ret;
#endif
}

void MudarModoMatriz(GLenum modo) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  if (modo == GL_MODELVIEW) {
    c->pilha_corrente = &c->pilha_mvm;
  } else {
    c->pilha_corrente = &c->pilha_prj;
  }
  //ATUALIZA_MATRIZES_NOVO();
#else
  glMatrixMode(modo);
#endif
}

void CarregaIdentidade(bool atualizar) {
#if USAR_SHADER
  interno::BuscaContexto()->pilha_corrente->top().identity();
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glLoadIdentity();
#endif
}

void MultiplicaMatriz(const GLfloat* matriz, bool atualizar) {
#if USAR_SHADER
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4(matriz);
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glMultMatrixf(matriz);
#endif
}

void Escala(GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
#if USAR_SHADER
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4().scale(x, y, z);
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glScalef(x, y, z);
#endif
}

void Translada(GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
#if USAR_SHADER
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4().translate(x, y, z);
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glTranslatef(x, y, z);
#endif
}
void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
#if USAR_SHADER
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4().rotate(angulo_graus, x, y, z);
  if (atualizar) ATUALIZA_MATRIZES_NOVO();
#else
  glRotatef(angulo_graus, x, y, z);
#endif
}

void TamanhoPonto(float tam) {
#if USAR_SHADER
  const auto& shader = interno::BuscaShader();
  GLint uniforme = shader.uni_gltab_dados_raster;
  GLfloat dados_raster[4];
  LeUniforme(shader.programa, uniforme, dados_raster);
  // Tamanho do ponto eh o p, ou terceiro elemento.
  Uniforme(uniforme, dados_raster[0], dados_raster[1], tam, dados_raster[3]);
#else
  glPointSize(tam);
#endif
}

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
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_vertice, vertices_por_coordenada, tipo, GL_FALSE, passo, vertices);
#else
  glVertexPointer(vertices_por_coordenada, tipo, passo, vertices);
#endif
}

void PonteiroNormais(GLenum tipo, GLsizei passo, const GLvoid* normais) {
#if USAR_SHADER
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_normal, 3  /**dimensoes*/, tipo, GL_FALSE, passo, normais);
#else
  glNormalPointer(tipo, passo, normais);
#endif
}

void Normal(GLfloat x, GLfloat y, GLfloat z) {
#if USAR_SHADER
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  AtributoVertice(interno::BuscaShader().atr_gltab_normal, x, y, z);
#else
  glNormal3f(x, y, z);
#endif
}

void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores) {
#if USAR_SHADER
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_cor, 4  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, cores);
#else
  glColorPointer(num_componentes, GL_FLOAT, passo, cores);
#endif
}

void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
#if USAR_SHADER
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_texel, 2  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, vertices);
#else
  glTexCoordPointer(vertices_por_coordenada, tipo, passo, vertices);
#endif
}

bool EstaHabilitado(GLenum cap) {
#if USAR_SHADER
  const auto& shader = interno::BuscaShader();
  if (cap == GL_LIGHTING) {
    if (!interno::UsandoShaderLuz()) {
      return false;
    }
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    return cor[3] > 0.0f;
  } else if (cap == GL_LIGHT0) {
    if (!interno::UsandoShaderLuz()) {
      return false;
    }
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    return cor[3] > 0;
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    if (!interno::UsandoShaderLuz()) {
      return false;
    }
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    return cor[3] > 0;
  } else if (cap == GL_TEXTURE_2D) {
    GLfloat fret;
    LeUniforme(shader.programa, shader.uni_gltab_textura, &fret);
    return fret;
  } else if (cap == GL_FOG) {
    if (!interno::UsandoShaderComNevoa()) {
      return false;
    }
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
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
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  Uniforme(interno::BuscaShader().uni_gltab_luz_ambiente_cor, r, g, b, 1.0f);
#else
  GLfloat glparams[4] = { r, g, b, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, glparams);
#endif
}

void LuzDirecional(const GLfloat* pos, float r, float g, float b) {
#if USAR_SHADER
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  Uniforme(interno::BuscaShader().uni_gltab_luz_direcional_cor, r, g, b, 1.0f);
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
  Uniforme(interno::BuscaShader().uni_gltab_luz_direcional_pos, vp.x, vp.y, vp.z, 1.0f);
#else
  glLightfv(GL_LIGHT0, GL_POSITION, pos);
  GLfloat cor[] = { r, g, b, 1.0f };
  glLightfv(GL_LIGHT0, GL_DIFFUSE, cor);
#endif
}

void LuzPontual(GLenum luz, GLfloat* pos, float r, float g, float b, float raio) {
#if USAR_SHADER
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  if (luz <= 0 || luz > 8) {
    LOG(ERROR) << "Luz invalida: " << luz;
    return;
  }
  // Transforma a posicao em coordenadas da camera.
  float glm[16];
  gl::Le(GL_MODELVIEW_MATRIX, glm);
  Matrix4 m(glm);
  Vector4 vp(pos[0], pos[1], pos[2], 1.0f);
  vp = m * vp;
  const auto& shader = interno::BuscaShader();
  Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzPos(luz - 1)], vp.x, vp.y, vp.z, 1.0f);
  Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzCor(luz - 1)], r, g, b, 1.0f);
  Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzAtributos(luz - 1)], raio, 0, 0, 0);
#else
  glLightfv(GL_LIGHT0 + luz, GL_POSITION, pos);
  GLfloat cor_luz[] = { r, g, b, 1.0f };
  glLightfv(GL_LIGHT0 + luz, GL_DIFFUSE, cor_luz);
  // Equacao: y = 1 / (c + q * d ^ 2)
  // valores c = 0.2 e q = 0.02 dao 0.5 em y = 6 (4 quadrados), equivalente a tocha.
  // o modificador sera aplicado aos dois de forma inversa ao raio. Ou seja, um raio de 12 divide
  // os coeficientes por 2, o que reduz o decaimento de 50% pela metade. Eh apenas uma aproximacao.
  float mod = 6.0f / raio;
  glLightf(GL_LIGHT0 + luz, GL_CONSTANT_ATTENUATION, 0.2f * mod);
  glLightf(GL_LIGHT0 + luz, GL_QUADRATIC_ATTENUATION, 0.02f * mod);
#endif
}

void Nevoa(GLfloat inicio, GLfloat fim, float r, float g, float b, GLfloat* pos_referencia) {
#if USAR_SHADER
  if (!interno::UsandoShaderComNevoa()) {
    return;
  }
  GLfloat glm[16];
  gl::Le(GL_MODELVIEW_MATRIX, glm);
  Matrix4 m(glm);
  Vector4 v(pos_referencia[0], pos_referencia[1], pos_referencia[2], 1.0f);
  v = m * v;
  const auto& shader = interno::BuscaShader();
  Uniforme(shader.uni_gltab_nevoa_referencia, v.x, v.y, v.z, 1.0f);
  Uniforme(shader.uni_gltab_nevoa_dados, inicio, fim, 0.0f  /*nao usado*/, (1.0f / (fim - inicio))  /*escala*/);
  Uniforme(shader.uni_gltab_nevoa_cor, r, g, b, 1.0f);
#else
  glFogf(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, inicio);
  glFogf(GL_FOG_END, fim);
  GLfloat cor[] = { r, g, b, 1.0f };
  glFogfv(GL_FOG_COLOR, cor);
#endif
}

void Perspectiva(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
  // Copiado do glues.
  GLfloat m[4][4];
  GLfloat sine, cotangent, deltaZ;
  GLfloat radians = (GLfloat)(fovy / 2.0f * __glPi /180.0f);

  deltaZ= zFar - zNear;
  sine= (GLfloat)sinf(radians);
  if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f))
  {
      return;
  }
  cotangent= (GLfloat)(cos(radians)/sine);

  glu::PreencheIdentidade(&m[0][0]);
  m[0][0] = cotangent / aspect;
  m[1][1] = cotangent;
  m[2][2] = -(zFar + zNear) / deltaZ;
  m[2][3] = -1.0f;
  m[3][2] = -2.0f * zNear * zFar / deltaZ;
  m[3][3] = 0;
#if USAR_SHADER
  Matrix4& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4(&m[0][0]);
  ATUALIZA_MATRIZES_NOVO();
#else
  MultiplicaMatriz(&m[0][0]);
#endif
}

void OlharPara(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
               GLfloat centery, GLfloat centerz,
               GLfloat upx, GLfloat upy, GLfloat upz) {
  GLfloat forward[3], up[3];
  forward[0] = centerx - eyex;
  forward[1] = centery - eyey;
  forward[2] = centerz - eyez;
  up[0] = upx;
  up[1] = upy;
  up[2] = upz;

  glu::Normaliza(forward);

  /* Side = forward x up */
  GLfloat side[3];
  glu::ProdutoVetorial(forward, up, side);
  glu::Normaliza(side);

  /* Recompute up as: up = side x forward */
  glu::ProdutoVetorial(side, forward, up);

  GLfloat m[4][4];
  glu::PreencheIdentidade(&m[0][0]);
  m[0][0] = side[0];
  m[1][0] = side[1];
  m[2][0] = side[2];
  m[0][1] = up[0];
  m[1][1] = up[1];
  m[2][1] = up[2];
  m[0][2] = -forward[0];
  m[1][2] = -forward[1];
  m[2][2] = -forward[2];

#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  Matrix4& topo = c->pilha_corrente->top();
  topo *= Matrix4(&m[0][0]);
  topo *= Matrix4().translate(-eyex, -eyey, -eyez);
  ATUALIZA_MATRIZES_NOVO();
#else
  glMultMatrixf(&m[0][0]);
  glTranslatef(-eyex, -eyey, -eyez);
#endif
}

void Ortogonal(float esquerda, float direita, float baixo, float cima, float proximo, float distante) {
  float tx = - ((direita + esquerda) / (direita - esquerda));
  float ty = - ((cima + baixo) / (cima - baixo));
  float tz = - ((distante + proximo) / (distante - proximo));
  GLfloat glm[16];
  glm[0] = 2.0f / (direita - esquerda); glm[4] = 0; glm[8] = 0; glm[12] = tx;
  glm[1] = 0; glm[5] = 2.0f / (cima - baixo); glm[9] = 0; glm[13] = ty;
  glm[2] = 0; glm[6] = 0; glm[10] = -2.0f / (distante - proximo); glm[14] = tz;
  glm[3] = 0; glm[7] = 0; glm[11] = 0; glm[15] = 1;
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  Matrix4 topo = c->pilha_corrente->top();
  Matrix4 m(glm);
  // Nao tenho certeza qual a ordem certa. No caso das ortogonais quase sempre a matriz corrente eh identidade, entao
  // da na mesma.
  //c->pilha_corrente->top() = m * topo;
  c->pilha_corrente->top() = topo * m;
  ATUALIZA_MATRIZES_NOVO();
#else
  glMultMatrixf(glm);
#endif
}

void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport) {
#if USAR_SHADER
  if (delta_x <= 0 || delta_y <= 0) {
    return;
  }

  Matrix4& topo = interno::BuscaContexto()->pilha_corrente->top();
  Matrix4 mt;
  mt.scale(viewport[2] / delta_x, viewport[3] / delta_y, 1.0).translate(
      (viewport[2] - 2 * (x - viewport[0])) / delta_x,
      (viewport[3] - 2 * (y - viewport[1])) / delta_y,
      0);
  topo *= mt;
  ATUALIZA_MATRIZES_NOVO();
#else
  if (delta_x <= 0 || delta_y <= 0) {
    return;
  }

  /* Translate and scale the picked region to the entire window */
  glTranslatef((viewport[2] - 2 * (x - viewport[0])) / delta_x,
               (viewport[3] - 2 * (y - viewport[1])) / delta_y, 0);
  glScalef(viewport[2] / delta_x, viewport[3] / delta_y, 1.0);
#endif
}

GLint Desprojeta(GLfloat winx, GLfloat winy, GLfloat winz,
                 const GLfloat modelMatrix[16],
                 const GLfloat projMatrix[16],
                 const GLint viewport[4],
                 GLfloat* objx, GLfloat* objy, GLfloat* objz) {
  auto ret = glu::Unproject(winx, winy, winz, modelMatrix, projMatrix, viewport, objx, objy, objz);
  return ret;
}

void Le(GLenum nome_parametro, GLfloat* valor) {
#if USAR_SHADER
  auto* c = interno::BuscaContexto();
  if (nome_parametro == GL_MODELVIEW_MATRIX) {
    memcpy(valor, c->pilha_mvm.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == GL_PROJECTION_MATRIX) {
    memcpy(valor, c->pilha_prj.top().get(), 16 * sizeof(float));
  } else {
    glGetFloatv(nome_parametro, valor);
  }
#else
  glGetFloatv(nome_parametro, valor);
#endif
}

void HabilitaEstadoCliente(GLenum cap) {
#if USAR_SHADER
  if (cap == GL_VERTEX_ARRAY) {
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_vertice);
  } else if (cap == GL_NORMAL_ARRAY) {
    if (!interno::UsandoShaderLuz()) {
      return;
    }
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_normal);
  } else if (cap == GL_COLOR_ARRAY) {
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_cor);
  } else if (cap == GL_TEXTURE_COORD_ARRAY) {
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_texel);
  } else {
    glEnableClientState(cap);
  }
#else
  glEnableClientState(cap);
#endif
}

void DesabilitaEstadoCliente(GLenum cap) {
#if USAR_SHADER
  if (cap == GL_VERTEX_ARRAY) {
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_vertice);
  } else if (cap == GL_NORMAL_ARRAY) {
    if (!interno::UsandoShaderLuz()) {
      return;
    }
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_normal);
  } else if (cap == GL_COLOR_ARRAY) {
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_cor);
  } else if (cap == GL_TEXTURE_COORD_ARRAY) {
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_texel);
  } else {
    glDisableClientState(cap);
  }
#else
  glDisableClientState(cap);
#endif
}

#if USAR_SHADER
void UsaShader(TipoShader ts) {
  auto* c = interno::BuscaContexto();
  UsaPrograma(c->shaders[ts].programa);
  c->shader_corrente = &c->shaders[ts];
  VLOG(3) << "Alternando para programa de shader: " << c->shader_corrente->nome;
}

void AtualizaMatrizesNovo() {
  auto* c = interno::BuscaContexto();
  bool modo_mv = c->pilha_corrente == &c->pilha_mvm;
  const interno::VarShader& shader = interno::BuscaShader();
  GLuint mloc = modo_mv ? shader.uni_gltab_mvm : shader.uni_gltab_prm;
  Matriz4Uniforme(mloc, 1, false, c->pilha_corrente->top().get());
  if (!modo_mv || !interno::UsandoShaderLuz()) {
    return;
  }

  // Normal matrix, apenas para modelview.
  const auto& m = c->pilha_corrente->top();
  Matrix3 normal(m[0], m[1], m[2],
                 m[4], m[5], m[6],
                 m[8], m[9], m[10]);
  normal.invert().transpose();
  if (normal != c->matriz_normal) {
    c->matriz_normal = normal;
    Matriz3Uniforme(shader.uni_gltab_nm, 1, false, normal.get());
  }
}

void DebugaMatrizes() {
  float mv[16];
  gl::Le(GL_MODELVIEW_MATRIX, mv);
  Matrix3 normal(mv[0], mv[1], mv[2],
                 mv[4], mv[5], mv[6],
                 mv[8], mv[9], mv[10]);
  normal.invert().transpose();
  //LOG_EVERY_N(INFO, 300) << "MV: \n" << Matrix4(mv)
  //                       << ", NM: \n" << normal;
}
#endif

void TamanhoFonte(int* largura, int* altura, int* escala) {
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  TamanhoFonte(viewport[2], viewport[3], largura, altura, escala);
}

void TamanhoFonte(int largura_viewport, int altura_viewport, int* largura_fonte, int* altura, int* escala) {
#if 0
  *escala = 2;
#elif ANDROID || (__APPLE__ && (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR))
  unsigned int media_tela = (largura_viewport + altura_viewport) / 2;
  *escala = std::max(media_tela / 500, 1U);
#else
  *escala = 1;
#endif
  *largura_fonte = 8;
  *altura = 13;
}

void PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) {
  float matriz_mv[16];
  float matriz_pr[16];
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::Le(GL_MODELVIEW_MATRIX, matriz_mv);
  gl::Le(GL_PROJECTION_MATRIX, matriz_pr);
  float x2d, y2d, z2d;
  if (!glu::Project(x, y, z, matriz_mv, matriz_pr, viewport, &x2d, &y2d, &z2d)) {
    return;
  }
  auto* contexto = interno::BuscaContexto();
  contexto->raster_x = x2d;
  contexto->raster_y = y2d;
  //LOG(INFO) << "raster_x: " << x2d << ", raster_y: " << y2d;
}

void PosicaoRaster(GLint x, GLint y) {
  PosicaoRaster(static_cast<float>(x), static_cast<float>(y), 0.0f);
}

void AlternaModoDebug() {
  auto* c = interno::BuscaContexto();
  //c->depurar_selecao_por_cor = !c->depurar_selecao_por_cor;
  c->AlternaSelecaoPorCor();
  LOG(INFO) << "selecao por cor: " << c->SelecaoPorCorHabilitada();
}

void CarregaNome(GLuint id) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    GLubyte rgb[3];
    interno::MapeiaId(id, rgb);
    VLOG(2) << "Mapeando " << id << ", bit pilha " << c->bit_pilha
            << " para " << (int)rgb[0] << ", " << (int)rgb[1] << ", " << (int)rgb[2];
    // Muda a cor para a mapeada.
#if USAR_SHADER
    AtributoVertice(interno::BuscaShader().atr_gltab_cor, rgb[0] / 255.0f, rgb[1] / 255.0f, rgb[2] / 255.0f, 1.0f);
#else
    glColor4ub(rgb[0], rgb[1], rgb[2], 255);
#endif
  } else {
#if !USAR_OPENGL_ES
    glLoadName(id);
#endif
  }
}

// Nomes
void IniciaNomes() {
#if !USAR_OPENGL_ES
  auto* c = interno::BuscaContexto();
  if (!c->UsarSelecaoPorCor()) {
    glInitNames();
  }
#endif
}

void EmpilhaNome(GLuint id) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    if (id > 7) {
      LOG(ERROR) << "Bit da pilha passou do limite superior.";
      return;
    }
    c->bit_pilha = id;
    VLOG(1) << "Empilhando bit pilha: " << c->bit_pilha;
  } else {
#if !USAR_OPENGL_ES
    glPushName(id);
#endif
  }
}

void DesempilhaNome() {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    if (c->bit_pilha == 0) {
      // No jeito novo, isso nao eh mais erro.
      //LOG(ERROR) << "Bit da pilha passou do limite inferior.";
      return;
    }
    VLOG(1) << "Desempilhando bit pilha: " << c->bit_pilha;
    c->bit_pilha = 0;
  } else {
#if !USAR_OPENGL_ES
    glPopName();
#endif
  }
}

void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) {
  auto* c = interno::BuscaContexto();
  if (c->SelecaoPorCorHabilitada()) {
    c->buffer_selecao = buffer;
    c->tam_buffer = tam_buffer;
  } else {
#if !USAR_OPENGL_ES
    glSelectBuffer(tam_buffer, buffer);
#endif
  }
}

GLint ModoRenderizacao(modo_renderizacao_e modo) {
  auto* c = interno::BuscaContexto();
  // Esta verificacao esta certa. Pois esta funcao eh chamada para entrar no modo de selecao,
  // durante o qual UsarSelecaoPorCor retornaria false.
  if (c->SelecaoPorCorHabilitada()) {
    if (c->modo_renderizacao == modo) {
      VLOG(1) << "Nao houve mudanca no modo de renderizacao";
      return 0;
    }
    c->modo_renderizacao = modo;
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
        auto it = c->ids.find(id_mapeado);
        if (it == c->ids.end()) {
          LOG(ERROR) << "Id nao mapeado: " << (void*)id_mapeado;
          return 0;
        }
#pragma GCC diagnostic pop
        unsigned int id_original = it->second;
        VLOG(1) << "Id original: " << id_original;
        GLuint* ptr = c->buffer_selecao;
        ptr[0] = 2;  // Sempre 2: 1 para tipo, outro para id.
#if USAR_SHADER
        ptr[1] = static_cast<GLuint>((pixel[3] / static_cast<float>(0xFF)) * 0xFFFFFFFF);  // zmin.
#else
        ptr[1] = 0;  // zmin.
#endif
        ptr[2] = ptr[1];  // zmax
        ptr[3] = tipo_objeto;
        ptr[4] = id_original;
        c->buffer_selecao = nullptr;
        c->tam_buffer = 0;
        return 1;  // Numero de hits: so pode ser 0 ou 1.
      }
      default:
        return 0;
    }
  } else {
#if !USAR_OPENGL_ES
    return glRenderMode(modo);
#else
    LOG(ERROR) << "NUNCA";
    return 0;  // compilador feliz, nunca vai ser alcancado.
#endif
  }
}

void MudaCor(float r, float g, float b, float a) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
#if USAR_SHADER
  AtributoVertice(interno::BuscaShader().atr_gltab_cor, r, g, b, a);
#else
  // Segundo manual do OpenGL ES, nao se pode definir o material separadamente por face.
  GLfloat cor[4] = { r, g, b, a };
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor4f(r, g, b, a);
#endif
}

void Limpa(GLbitfield mascara) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    if ((mascara & GL_COLOR_BUFFER_BIT) != 0) {
      // Preto nao eh valido no color picking.
      glClearColor(0, 0, 0, 1.0f);
    }
  }
  glClear(mascara);
}

void InicioCena() {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    c->proximo_id = 0;
    c->bit_pilha = 0;
    c->ids.clear();
  }
}

void Habilita(GLenum cap) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    if (cap == GL_LIGHTING) {
      return;
    }
  } else {
#if USAR_SHADER
    interno::HabilitaComShader(c, cap);
#else
    glEnable(cap);
#endif
  }
}

void Desabilita(GLenum cap) {
#if USAR_SHADER
  interno::DesabilitaComShader(interno::BuscaContexto(), cap);
  V_ERRO((std::string("desabilitando es cap: ") + std::to_string((int)cap)).c_str());
#else
  glDisable(cap);
#endif
}

void FinalizaGl() {
#if WIN32
  // Apagar o contexto_interno
#endif
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_LUZ]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_SIMPLES]);
}

bool SelecaoPorCor() {
  return interno::BuscaContexto()->SelecaoPorCorHabilitada();
}

}  // namespace gl
