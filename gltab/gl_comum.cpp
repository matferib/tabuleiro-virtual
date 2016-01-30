#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <map>

#if USAR_GFLAGS
#include <google/gflags.h>
#endif

#include "gltab/gl_interno.h"
#include "gltab/glues.h"
#include "arq/arquivo.h"
//#define VLOG_NIVEL 2
#include "log/log.h"
#include "net/util.h"
#include "matrix/matrices.h"


#if USAR_GFLAGS
DEFINE_bool(luz_por_vertice, false, "Se verdadeiro, usa iluminacao por vertice.");
#endif

using gl::TSH_LUZ;
using gl::TSH_SIMPLES;
using gl::TSH_PICKING;
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

void UniformeSeValido(GLint location, GLint v0) {
  if (location == -1) {
    return;
  }
  Uniforme(location, v0);
}

void UniformeSeValido(GLint location, GLfloat v0) {
  if (location == -1) {
    return;
  }
  Uniforme(location, v0);
}

void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  auto* c = BuscaContexto();
  if (c->proximo_id > IdMaximoEntidade()) {
    throw std::logic_error(std::string("Limite de ids alcancado: ") + net::to_string(c->proximo_id));
  }
  unsigned int id_mapeado = c->proximo_id | (c->bit_pilha << DeslocamentoPilha() );
  c->ids.insert(std::make_pair(id_mapeado, id));
  if (c->depurar_selecao_por_cor) {
    // Mais facil de ver.
    c->proximo_id += 5;
  } else {
    ++c->proximo_id;
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  // Esse provavelmente sera ignorados ao usar 16 bits de profundidade.
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
  return true;
}

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
    for (unsigned int j = 0; j < sizeof (type_set) / sizeof (glsl_type_set); j++) {
      if (type_set [j].type != type)
        continue;

      const char* type_name = type_set [j].name;

      if (size > 1) {
        printf ("Uniform %d (loc=%d):\t%20s %-20s <Size: %d>\n", i, location, type_name, name, size );
      } else {
        printf("Uniform %d (loc=%d):\t%20s %-20s\n", i, location, type_name, name );
      }
      break;
    }
    if (i == (uniform_count - 1)) {
      printf ("\n");
    }
  }
}

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

// Realiza preprocessamento do fonte do shader. Altera o fonte.
void PreprocessaFonte(const std::string& nome, std::string* fonte) {
#define STRINGIFY_MACRO_VALUE(S) STRINGIFY(S)
#define STRINGIFY(S) #S
  std::map<std::string, std::string> mapa = {
    { "${USAR_FRAMEBUFFER}", STRINGIFY_MACRO_VALUE(USAR_FRAMEBUFFER) }
  };
  for (const auto& par : mapa) {
    auto pos = fonte->find(par.first);
    if (pos == std::string::npos) {
      continue;
    }
    LOG(INFO) << "Substituindo no shader " << nome << ": '" << par.first << "' -> '" << par.second << "'";
    fonte->replace(pos, par.first.size(), par.second);
  }
  //LOG(INFO) << "Fonte: " << *fonte;
#undef STRINGIFY
#undef STRINGIFY_MACRO_VALUE
}

bool IniciaShader(const char* nome_programa, const char* nome_vs, const char* nome_fs,
                  VarShader* shader) {
  shader->nome = nome_programa;
  GLuint v_shader = CriaShader(GL_VERTEX_SHADER);
  V_ERRO_RET("criando vertex shader");
  GLuint f_shader = CriaShader(GL_FRAGMENT_SHADER);
  V_ERRO_RET("criando fragment shader");
  std::string codigo_v_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_vs, &codigo_v_shader_str);
  PreprocessaFonte(nome_vs, &codigo_v_shader_str);
  const char* codigo_v_shader = codigo_v_shader_str.c_str();
  FonteShader(v_shader, 1, &codigo_v_shader, nullptr);
  V_ERRO_RET("shader source vertex");
  std::string codigo_f_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_fs, &codigo_f_shader_str);
  PreprocessaFonte(nome_fs, &codigo_f_shader_str);
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
          {"gltab_unidade_textura_sombra", &shader->uni_gltab_unidade_textura_sombra },
          {"gltab_nevoa_dados", &shader->uni_gltab_nevoa_dados },
          {"gltab_nevoa_cor", &shader->uni_gltab_nevoa_cor},
          {"gltab_nevoa_referencia", &shader->uni_gltab_nevoa_referencia },
          {"gltab_mvm", &shader->uni_gltab_mvm },
          {"gltab_mvm_sombra", &shader->uni_gltab_mvm_sombra },
          {"gltab_prm", &shader->uni_gltab_prm },
          {"gltab_prm_sombra", &shader->uni_gltab_prm_sombra },
          {"gltab_nm", &shader->uni_gltab_nm },
          {"gltab_dados_raster", &shader->uni_gltab_dados_raster},
  }) {
    *d.var = LocalUniforme(shader->programa, d.nome);
    if (*d.var == -1) {
      LOG(INFO) << "Shader nao possui uniforme " << d.nome;
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
        LOG(INFO) << "Shader nao possui uniforme array" << nome_var;
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
      LOG(INFO) << "Shader nao possui atributo " << d.nome;
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
    { "programa_picking", "vert_simples.c", "frag_picking.c", &contexto->shaders[TSH_PICKING] },
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


void IniciaComum(bool luz_por_vertice, interno::Contexto* contexto) {
  contexto->pilha_mvm.push(Matrix4());
  contexto->pilha_prj.push(Matrix4());
  contexto->pilha_mvm_sombra.push(Matrix4());
  contexto->pilha_prj_sombra.push(Matrix4());
  contexto->pilha_corrente = &contexto->pilha_mvm;
  // Essa funcao pode dar excecao, entao eh melhor colocar depois das matrizes pra aplicacao nao crashar e mostrar
  // a mensagem de erro.
  IniciaShaders(luz_por_vertice, contexto);
}

void FinalizaShaders(const VarShader& shader) {
  DesanexaShader(shader.programa, shader.vs);
  DesanexaShader(shader.programa, shader.fs);
  DestroiPrograma(shader.programa);
  DestroiShader(shader.vs);
  DestroiShader(shader.fs);
}

void HabilitaComShader(interno::Contexto* contexto, GLenum cap) {
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
    interno::UniformeSeValido(shader.uni_gltab_textura, 1.0f);
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
    glEnable(cap);
    return;
  }
}

void DesabilitaComShader(interno::Contexto* contexto, GLenum cap) {
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
    interno::UniformeSeValido(shader.uni_gltab_textura, 0.0f);
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
}

bool LuzPorVertice(int argc, const char* const* argv) {
#if USAR_GFLAGS
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

void AtualizaMatrizSombra(const Matrix4& m) {
#if USAR_FRAMEBUFFER
  if (ModoMatrizCorrente() != MATRIZ_MODELAGEM_CAMERA) {
    return;
  }
  auto* c = interno::BuscaContexto();
  c->pilha_mvm_sombra.top() *= m;
#endif
}

void IdentidadeMatrizSombra() {
#if USAR_FRAMEBUFFER
  if (ModoMatrizCorrente() != MATRIZ_MODELAGEM_CAMERA) {
    return;
  }
  auto* c = interno::BuscaContexto();
  c->pilha_mvm_sombra.top().identity();
#endif
}

void EmpilhaMatrizSombra() {
#if USAR_FRAMEBUFFER
  if (ModoMatrizCorrente() != MATRIZ_MODELAGEM_CAMERA) {
    return;
  }
  auto* c = interno::BuscaContexto();
  Matrix4 m(c->pilha_mvm_sombra.top());
  c->pilha_mvm_sombra.push(m);
#endif
}

void DesempilhaMatrizSombra() {
#if USAR_FRAMEBUFFER
  if (ModoMatrizCorrente() != MATRIZ_MODELAGEM_CAMERA) {
    return;
  }
  auto* c = interno::BuscaContexto();
  c->pilha_mvm_sombra.pop();
#endif
}

}  // namespace interno

void EmpilhaMatriz(bool atualizar) {
  auto* c = interno::BuscaContexto();
  Matrix4 m(c->pilha_corrente->top());
  c->pilha_corrente->push(m);
  interno::EmpilhaMatrizSombra();
  // Nao precisa porque a matriz empilhada eh igual.
  //if (atualizar) AtualizaMatrizes();
}

void DesempilhaMatriz(bool atualizar) {
  auto* c = interno::BuscaContexto();
#if DEBUG
  if (c->pilha_corrente->empty()) {
    LOG(ERROR) << "Pilha vazia";
    return;
  }
#endif
  c->pilha_corrente->pop();
  interno::DesempilhaMatrizSombra();
  if (atualizar) AtualizaMatrizes();
}

int ModoMatrizCorrente() {
  auto* c = interno::BuscaContexto();
  if (c->pilha_corrente == &c->pilha_mvm) { return MATRIZ_MODELAGEM_CAMERA; }
  else if (c->pilha_corrente == &c->pilha_prj) { return MATRIZ_PROJECAO; }
  else if (c->pilha_corrente == &c->pilha_prj_sombra) { return MATRIZ_PROJECAO_SOMBRA; }
  else { return MATRIZ_SOMBRA; }
}

void MudarModoMatriz(int modo) {
  auto* c = interno::BuscaContexto();
  if (modo == MATRIZ_MODELAGEM_CAMERA) {
    c->pilha_corrente = &c->pilha_mvm;
  } else if (modo == MATRIZ_PROJECAO) {
    c->pilha_corrente = &c->pilha_prj;
  } else if (modo == MATRIZ_PROJECAO_SOMBRA) {
    c->pilha_corrente = &c->pilha_prj_sombra;
  } else {
    c->pilha_corrente = &c->pilha_mvm_sombra;
  }
  //AtualizaMatrizes();
}

void CarregaIdentidade(bool atualizar) {
  interno::BuscaContexto()->pilha_corrente->top().identity();
  interno::IdentidadeMatrizSombra();
  if (atualizar) AtualizaMatrizes();
}

void MultiplicaMatriz(const GLfloat* matriz, bool atualizar) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  Matrix4 m4(matriz);
  topo *= m4;
  interno::AtualizaMatrizSombra(m4);
  if (atualizar) AtualizaMatrizes();
}

void Escala(GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  Matrix4 m4 = Matrix4().scale(x, y, z);
  topo *= m4;
  interno::AtualizaMatrizSombra(m4);
  if (atualizar) AtualizaMatrizes();
}

void Translada(GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  Matrix4 m4 = Matrix4().translate(x, y, z);
  topo *= m4;
  interno::AtualizaMatrizSombra(m4);
  if (atualizar) AtualizaMatrizes();
}
void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z, bool atualizar) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  Matrix4 m4 = Matrix4().rotate(angulo_graus, x, y, z);
  topo *= m4;
  interno::AtualizaMatrizSombra(m4);
  if (atualizar) AtualizaMatrizes();
}

void TamanhoPonto(float tam) {
  const auto& shader = interno::BuscaShader();
  GLint uniforme = shader.uni_gltab_dados_raster;
  GLfloat dados_raster[4];
  LeUniforme(shader.programa, uniforme, dados_raster);
  // Tamanho do ponto eh o p, ou terceiro elemento.
  Uniforme(uniforme, dados_raster[0], dados_raster[1], tam, dados_raster[3]);
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
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_vertice, vertices_por_coordenada, tipo, GL_FALSE, passo, vertices);
}

void PonteiroNormais(GLenum tipo, GLsizei passo, const GLvoid* normais) {
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_normal, 3  /**dimensoes*/, tipo, GL_FALSE, passo, normais);
}

void Normal(GLfloat x, GLfloat y, GLfloat z) {
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  AtributoVertice(interno::BuscaShader().atr_gltab_normal, x, y, z);
}

void PonteiroCores(GLint num_componentes, GLsizei passo, const GLvoid* cores) {
  if (interno::BuscaShader().atr_gltab_cor != -1) {
    PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_cor, 4  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, cores);
  }
}

void PonteiroVerticesTexturas(GLint vertices_por_coordenada, GLenum tipo, GLsizei passo, const GLvoid* vertices) {
  if (interno::BuscaShader().atr_gltab_texel != -1) {
    PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_texel, 2  /**dimensoes*/, GL_FLOAT, GL_FALSE, passo, vertices);
  }
}

bool EstaHabilitado(GLenum cap) {
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
    if (shader.uni_gltab_textura == -1) {
      return false;
    }
    GLfloat fret;
    LeUniforme(shader.programa, shader.uni_gltab_textura, &fret);
    return fret > 0.5f;
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
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  Uniforme(interno::BuscaShader().uni_gltab_luz_ambiente_cor, r, g, b, 1.0f);
}

void LuzDirecional(const GLfloat* pos, float r, float g, float b) {
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
}

void LuzPontual(GLenum luz, GLfloat* pos, float r, float g, float b, float raio) {
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
}

void Nevoa(GLfloat inicio, GLfloat fim, float r, float g, float b, GLfloat* pos_referencia) {
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
  Matrix4& topo = interno::BuscaContexto()->pilha_corrente->top();
  topo *= Matrix4(&m[0][0]);
  AtualizaMatrizes();
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

  auto* c = interno::BuscaContexto();
  Matrix4& topo = c->pilha_corrente->top();
  topo *= Matrix4(&m[0][0]);
  topo *= Matrix4().translate(-eyex, -eyey, -eyez);
  AtualizaMatrizes();
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
  auto* c = interno::BuscaContexto();
  Matrix4 topo = c->pilha_corrente->top();
  Matrix4 m(glm);
  // Nao tenho certeza qual a ordem certa. No caso das ortogonais quase sempre a matriz corrente eh identidade, entao
  // da na mesma.
  //c->pilha_corrente->top() = m * topo;
  c->pilha_corrente->top() = topo * m;
  AtualizaMatrizes();
}

void MatrizPicking(float x, float y, float delta_x, float delta_y, GLint *viewport) {
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
  AtualizaMatrizes();
}

GLint Desprojeta(GLfloat winx, GLfloat winy, GLfloat winz,
                 const GLfloat modelMatrix[16],
                 const GLfloat projMatrix[16],
                 const GLint viewport[4],
                 GLfloat* objx, GLfloat* objy, GLfloat* objz) {
  auto ret = glu::Unproject(winx, winy, winz, modelMatrix, projMatrix, viewport, objx, objy, objz);
  return ret;
}

Matrix4 LeMatriz(matriz_e modo) {
  auto* c = interno::BuscaContexto();
  if (modo == MATRIZ_MODELAGEM_CAMERA) {
    return c->pilha_mvm.top();
  } else if (modo == MATRIZ_PROJECAO) {
    return c->pilha_prj.top();
  } else if (modo == MATRIZ_PROJECAO_SOMBRA) {
    return c->pilha_prj_sombra.top();
  } else {
    return c->pilha_mvm_sombra.top();
  }
}

void Le(GLenum nome_parametro, GLfloat* valor) {
  auto* c = interno::BuscaContexto();
  if (nome_parametro == GL_MODELVIEW_MATRIX) {
    memcpy(valor, c->pilha_mvm.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == GL_PROJECTION_MATRIX) {
    memcpy(valor, c->pilha_prj.top().get(), 16 * sizeof(float));
  } else {
    glGetFloatv(nome_parametro, valor);
  }
}

void HabilitaEstadoCliente(GLenum cap) {
  if (cap == GL_VERTEX_ARRAY) {
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_vertice);
  } else if (cap == GL_NORMAL_ARRAY) {
    if (!interno::UsandoShaderLuz()) {
      return;
    }
    HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_normal);
  } else if (cap == GL_COLOR_ARRAY) {
    if (interno::BuscaShader().atr_gltab_cor != -1) {
      HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_cor);
    }
  } else if (cap == GL_TEXTURE_COORD_ARRAY) {
    if (interno::BuscaShader().atr_gltab_texel != -1) {
      HabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_texel);
    }
  } else {
    glEnableClientState(cap);
  }
}

void DesabilitaEstadoCliente(GLenum cap) {
  if (cap == GL_VERTEX_ARRAY) {
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_vertice);
  } else if (cap == GL_NORMAL_ARRAY) {
    if (!interno::UsandoShaderLuz()) {
      return;
    }
    DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_normal);
  } else if (cap == GL_COLOR_ARRAY) {
    if (interno::BuscaShader().atr_gltab_cor != -1) {
      DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_cor);
    }
  } else if (cap == GL_TEXTURE_COORD_ARRAY) {
    if (interno::BuscaShader().atr_gltab_texel != -1) {
      DesabilitaVetorAtributosVertice(interno::BuscaShader().atr_gltab_texel);
    }
  } else {
    glDisableClientState(cap);
  }
}

void UsaShader(TipoShader ts) {
  auto* c = interno::BuscaContexto();
  UsaPrograma(c->shaders[ts].programa);
  auto* shader = c->shader_corrente = &c->shaders[ts];
  // Atualiza as variaveis de shader.
  AtualizaTodasMatrizes();
  interno::UniformeSeValido(shader->uni_gltab_unidade_textura, 0);
  interno::UniformeSeValido(shader->uni_gltab_unidade_textura_sombra, 1);

  VLOG(3) << "Alternando para programa de shader: " << c->shader_corrente->nome;
}

namespace {
GLint IdMatrizCorrente(const interno::VarShader& shader) {
  switch (ModoMatrizCorrente()) {
    case MATRIZ_MODELAGEM_CAMERA: return shader.uni_gltab_mvm;
    case MATRIZ_PROJECAO:         return shader.uni_gltab_prm;
    case MATRIZ_PROJECAO_SOMBRA:  return shader.uni_gltab_prm_sombra;
    case MATRIZ_SOMBRA:
    default:                      return shader.uni_gltab_mvm_sombra;
  }
}
}  // namespace

void AtualizaMatrizes() {
  auto* c = interno::BuscaContexto();
  int modo = ModoMatrizCorrente();
  const interno::VarShader& shader = interno::BuscaShader();
  GLuint mloc = IdMatrizCorrente(shader);
  Matriz4Uniforme(mloc, 1, false, c->pilha_corrente->top().get());
  if (modo != MATRIZ_MODELAGEM_CAMERA || !interno::UsandoShaderLuz()) {
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

#if USAR_FRAMEBUFFER
  Matriz4Uniforme(shader.uni_gltab_mvm_sombra, 1, false, c->pilha_mvm_sombra.top().get());
#endif
}

void AtualizaTodasMatrizes() {
  auto* c = interno::BuscaContexto();
  const interno::VarShader& shader = interno::BuscaShader();
  struct DadosMatriz {
    GLint id;
    Matrix4* matriz;
  };
  std::vector<DadosMatriz> dados_matriz = {
    { shader.uni_gltab_mvm, &c->pilha_mvm.top() },
    { shader.uni_gltab_prm, &c->pilha_prj.top() },
    { shader.uni_gltab_mvm_sombra, &c->pilha_mvm_sombra.top() },
    { shader.uni_gltab_prm_sombra, &c->pilha_prj_sombra.top() },
  };
  for (const auto& dm : dados_matriz) {
    if (dm.id != -1) {
      Matriz4Uniforme(dm.id, 1, false, dm.matriz->get());
    }
  }
  if (shader.uni_gltab_nm != -1) {
    Matriz3Uniforme(shader.uni_gltab_nm, 1, false, c->matriz_normal.get());
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
    if (interno::BuscaShader().atr_gltab_cor != -1) {
      AtributoVertice(interno::BuscaShader().atr_gltab_cor, rgb[0] / 255.0f, rgb[1] / 255.0f, rgb[2] / 255.0f, 1.0f);
    }
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
        unsigned int id_mapeado = pixel[0] | (pixel[1] << 8);
        if (BitsProfundidade() == 8) {
          id_mapeado |= (pixel[2] << 16);
        }
        VLOG(1) << "Id mapeado: " << (void*)id_mapeado;
        unsigned int tipo_objeto = id_mapeado >> (32 - BitsProfundidade() - BitsPilha());
        VLOG(1) << "Tipo objeto: " << tipo_objeto;
        if (tipo_objeto > MaiorBitPilha()) {
          LOG(ERROR) << "Tipo objeto invalido: " << tipo_objeto;
          return 0;
        }
        auto it = c->ids.find(id_mapeado);
        if (it == c->ids.end()) {
          LOG(ERROR) << "Id nao mapeado: " << (void*)id_mapeado;
          return 0;
        }
        unsigned int id_original = it->second;
        VLOG(1) << "Id original: " << id_original;
        GLuint* ptr = c->buffer_selecao;
        ptr[0] = 2;  // Sempre 2: 1 para tipo, outro para id.
        //GLuint from_depth;
        //glReadPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, &from_depth);
        // Converte a profundidade para 32 bits.
        if (BitsProfundidade() == 8) {
          ptr[1] = static_cast<GLuint>((pixel[3] / static_cast<float>(0xFF)) * 0xFFFFFFFF);  // zmin.
        } else {
          float prof = (pixel[2] / static_cast<float>(0xFF)) + ((pixel[3] / static_cast<float>(0xFF)) / 256.0);
          //float teste = ((pixel[2] << 8) | pixel[3]) / static_cast<float>(0x10000);
          //LOG(INFO) << "from_depth: " << (void*)from_depth
          //          << ", teste: " << (void*)static_cast<GLuint>(teste * 0xFFFFFFFF)
          //          << ", prof: " << (void*)static_cast<GLuint>(prof * 0xFFFFFFFF);
          ptr[1] = static_cast<GLuint>(prof * 0xFFFFFFFF);
        }
#pragma GCC diagnostic pop
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
  if (interno::BuscaShader().atr_gltab_cor != -1) {
    AtributoVertice(interno::BuscaShader().atr_gltab_cor, r, g, b, a);
  }
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
    interno::HabilitaComShader(c, cap);
  }
}

void Desabilita(GLenum cap) {
  interno::DesabilitaComShader(interno::BuscaContexto(), cap);
  V_ERRO((std::string("desabilitando es cap: ") + net::to_string((int)cap)).c_str());
}

void UnidadeTextura(GLenum unidade) {
#if WIN32
  interno::TexturaAtivaInterno(unidade);
#else
  glActiveTexture(unidade);
#endif
#if 0
  // Passei pro usa shader.
  // Atualiza as variaveis de shader, se houver. Meio hacky mas ok.
  const auto& shader = interno::BuscaShader();
  if (unidade == GL_TEXTURE0) {
    Uniforme(shader.uni_gltab_unidade_textura, 0);
  } else if (unidade == GL_TEXTURE1) {
    Uniforme(shader.uni_gltab_unidade_textura_sombra, 1);
  }
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
