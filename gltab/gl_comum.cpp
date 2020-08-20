#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <map>

#if __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
  #elif TARGET_OS_MAC
    #define MAC_OSX 1
  #endif
#endif

//#define VLOG_NIVEL 2
#include "log/log.h"

#include "gltab/gl_interno.h"
#include "gltab/glues.h"
#include "gltab/gl_vbo.h"
#include "arq/arquivo.h"
#include "goog/stringprintf.h"
#include "matrix/matrices.h"

// Comum.
namespace gl {

namespace {
using google::protobuf::StringPrintf;
}  // namespace

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

Matrix3 ExtraiMatrizNormal(const Matrix4& matriz_modelagem) {
  const float* mm = matriz_modelagem.get();
  return Matrix3(mm[0], mm[1], mm[2],
                 mm[4], mm[5], mm[6],
                 mm[8], mm[9], mm[10]).invert().transpose();
}

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

void UniformeSeValido(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
  if (location == -1) {
    return;
  }
  Uniforme(location, v0, v1, v2, v3);
}

void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  auto* c = BuscaContexto();
  if (c->proximo_id > IdMaximoEntidade()) {
    throw std::logic_error(StringPrintf("Limite de ids alcancado: %d", (int)c->proximo_id));
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
#if !USAR_OPENGL_ES
  // Multisampling pode causar problemas com pontos. Na radeon, os pontos somem pois o tamanho do ponto
  // eh considerado no sampling e nao em pixels.
  gl::DesabilitaEscopo salva_sampling(GL_MULTISAMPLE);
#endif

  // Melhor deixar comentado assim para as letras ficarem sempre em primeiro plano.
  //gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  //gl::DesligaEscritaProfundidadeEscopo mascara_escopo;
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  // Precisa salvar a projecao para a volta, senao ela fica bichada ja que nao eh alterada de novo.
  gl::MatrizEscopo salva_matriz(MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  gl::Ortogonal(0.0f, viewport[2], 0.0f, viewport[3], 0.0f, 1.0f);
  AtualizaMatrizes();

  gl::MatrizEscopo salva_matriz_camera(MATRIZ_CAMERA);
  gl::CarregaIdentidade();
  AtualizaMatrizes();
  gl::MatrizEscopo salva_matriz_modelagem(MATRIZ_MODELAGEM);
  gl::CarregaIdentidade();

  int largura_fonte, altura_fonte, escala;
  TamanhoFonte(&largura_fonte, &altura_fonte, &escala);

  auto* contexto = BuscaContexto();
  float x2d = contexto->raster_x;
  float y2d = contexto->raster_y;
  gl::Translada(x2d, y2d, 0.0f);

  //LOG(INFO) << "x2d: " << x2d << " y2d: " << y2d;
  std::vector<std::string> str_linhas(interno::QuebraString(str, '\n'));
  gl::TamanhoPonto(escala);
  std::vector<VboNaoGravado> vbos;
  for (int linha = 0; linha < (int)str_linhas.size(); ++linha) {
    const std::string& str_linha = str_linhas[linha];
    float translacao_x = 0;
    if (alinhamento == 1) {  // direita.
      translacao_x = -static_cast<float>(str_linha.size() * largura_fonte);
    } else if (alinhamento == 0) {  // central.
      translacao_x = -static_cast<float>(str_linha.size() * largura_fonte) / 2.0f;
    }
    for (unsigned int i = 0; i < str_linha.size(); ++i) {
      char c = str_linha[i];
      VboNaoGravado vbo = VboCaractere(c);
      vbo.Translada(translacao_x + i * largura_fonte, (inverte_vertical ? -1 : 1) * linha * altura_fonte, 0.0f);
      vbo.Escala(escala, escala, 1.0f);
      vbos.emplace_back(std::move(vbo));
    }
  }
  VbosNaoGravados vbos_ng(std::move(vbos));
  vbos_ng.Desenha(GL_POINTS);
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

namespace {

void CarregaExtensoes() {
  auto* contexto = BuscaContexto();
  const char* ext_cstr = ((const char*)gl::Le(GL_EXTENSIONS));
  std::string extstr(ext_cstr == nullptr ? "" : ext_cstr);
  std::string corrente;
  for (auto c : extstr) {
    if (!isspace(c)) {
      corrente.append(1, c);
    } else {
      contexto->extensoes.insert(corrente);
      corrente.clear();
    }
  }
  if (!corrente.empty()) {
    contexto->extensoes.insert(corrente);
  }
}

}  // namespace

void ImprimeExtensoes() {
  // Usa VLOG ao inves de info pra imprimir no android sempre.
  VLOG(0) << "Extensoes:";
  auto* contexto = BuscaContexto();
  for (const auto& e : contexto->extensoes) {
    VLOG(0) << e;
  }
}

void print_uniforms(GLuint program) {
  GLint uniform_count;
  ProgramaLeParam(program, GL_ACTIVE_UNIFORMS, &uniform_count);
  GLchar name [256];
  for (GLint i = 0; i < uniform_count; i++) {
    memset (name, '\0', 256);
    GLint  size;
    GLenum type;

    LeUniformeAtivo(program, i, 255, NULL, &size, &type, name);
    GLint location = LocalUniforme(program, name);
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
void PreprocessaFonte(const std::string& nome, const VarShader& shader, std::string* fonte) {
#define STRINGIFY_MACRO_VALUE(S) STRINGIFY(S)
#define STRINGIFY(S) #S
  std::map<std::string, std::string> mapa = {
#if USAR_OPENGL_ES
    { "${VERSAO}", "100" },
#elif __APPLE__
    { "${VERSAO}", "120" },
#else
    { "${VERSAO}", "130" },
#endif
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

bool IniciaShader(const char* nome_programa,
                  TipoShader tipo,
                  const char* nome_vs,
                  const char* nome_fs,
                  VarShader* shader) {
  shader->nome = nome_programa;
  shader->tipo = tipo;
  GLuint v_shader = CriaShader(GL_VERTEX_SHADER);
  V_ERRO_RET("criando vertex shader");
  GLuint f_shader = CriaShader(GL_FRAGMENT_SHADER);
  V_ERRO_RET("criando fragment shader");
  std::string codigo_v_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_vs, &codigo_v_shader_str);
  PreprocessaFonte(nome_vs, *shader, &codigo_v_shader_str);
  const char* codigo_v_shader = codigo_v_shader_str.c_str();
  FonteShader(v_shader, 1, &codigo_v_shader, nullptr);
  V_ERRO_RET("shader source vertex");
  std::string codigo_f_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, nome_fs, &codigo_f_shader_str);
  PreprocessaFonte(nome_fs, *shader, &codigo_f_shader_str);
  const char* codigo_f_shader = codigo_f_shader_str.c_str();
  FonteShader(f_shader, 1, &codigo_f_shader, nullptr);
  V_ERRO_RET("shader source fragment");
  CompilaShader(v_shader);
  V_ERRO_SHADER(v_shader);
  CompilaShader(f_shader);
  V_ERRO_SHADER(f_shader);
  GLuint p = CriaPrograma();
  V_ERRO_RET("criando programa shader");
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
  // Variaveis atributos.
  struct DadosAtributo {
    DadosVariavel dv;
    GLint indice;
  };
  // Forca as variaveis para indices > 0 (driver da ATI nao curte).
  // No caso da apple, tem que começar com vertice no indice 0... vai entender.
  constexpr int ii =
#if __APPLE__
    0
#else
    1
#endif
  ;
  std::vector<DadosAtributo> datribs {
          {{"gltab_vertice", &shader->atr_gltab_vertice}, ii + 0},
          {{"gltab_normal", &shader->atr_gltab_normal}, ii + 1},
          {{"gltab_cor", &shader->atr_gltab_cor}, ii + 2},
          {{"gltab_texel", &shader->atr_gltab_texel}, ii + 3},
          {{"gltab_tangent", &shader->atr_gltab_tangente}, ii + 4},
          {{"gltab_model_i", &shader->atr_gltab_matriz_modelagem}, ii + 5},  // 4 dimensoes.
          {{"gltab_nm_i", &shader->atr_gltab_matriz_normal}, ii + 9},  // 3 dimensoes
  };

  for (auto& [dv, indice_forcado] : datribs) {
    LocalAtributo(shader->programa, indice_forcado, dv.nome);
    V_ERRO_RET(StringPrintf("shader: %s atribuindo local de atributo %s", shader->nome.c_str(), dv.nome).c_str());
    LOG(INFO) << "Tentando atribuir" << dv.nome << " na posicao " << *dv.var;
  }

  // Variaveis de software, otimizacoes.
  shader->textura_ligada = false;

  AnexaShader(shader->programa, shader->vs);
  V_ERRO_RET("atachando vertex no programa shader");
  AnexaShader(shader->programa, shader->fs);
  V_ERRO_RET("atachando fragment no programa shader");
  LinkaPrograma(shader->programa);
  V_ERRO_RET("linkando programa shader");
  LOG(INFO) << "LINKADO!";

  // Confere os atributos apos linkagem.
  for (const auto& [dv, indice_forcado] : datribs) {
    // OpenGL do mac nao curte atribuir o local do atributo.
    *dv.var = LeLocalAtributo(shader->programa, dv.nome);
    if (*dv.var == -1) {
      LOG(INFO) << "Shader '" << shader->nome << "' nao possui atributo " << dv.nome;
    } else if (*dv.var == indice_forcado) {
      LOG(WARNING) << "Shader '" << shader->nome << "' tentou colocar " << dv.nome << " na posicao " << indice_forcado << " mas pos na " << *dv.var;
    } else {
      LOG(INFO) << "Atributo " << dv.nome << " na posicao " << *dv.var;
    }
  }

  // Variaveis uniformes.
  for (auto& [nome, p_local_atributo] : std::vector<DadosVariavel> {
          {"gltab_luz_ambiente", &shader->uni_gltab_luz_ambiente_cor },
          {"gltab_cor_mistura_pre_nevoa", &shader->uni_gltab_cor_mistura_pre_nevoa },
          {"gltab_luz_direcional.cor", &shader->uni_gltab_luz_direcional_cor },
          {"gltab_luz_direcional.pos", &shader->uni_gltab_luz_direcional_pos },
          {"gltab_textura", &shader->uni_gltab_textura },
          {"gltab_textura_bump", &shader->uni_gltab_textura_bump },
          {"gltab_textura_cubo", &shader->uni_gltab_textura_cubo },
          {"gltab_unidade_textura", &shader->uni_gltab_unidade_textura },
          //{"gltab_unidade_textura_bump", &shader->uni_gltab_unidade_textura_bump },
          {"gltab_unidade_textura_sombra", &shader->uni_gltab_unidade_textura_sombra },
          {"gltab_unidade_textura_cubo", &shader->uni_gltab_unidade_textura_cubo },
          {"gltab_unidade_textura_oclusao", &shader->uni_gltab_unidade_textura_oclusao },
          {"gltab_unidade_textura_luz", &shader->uni_gltab_unidade_textura_luz },
          {"gltab_nevoa_dados", &shader->uni_gltab_nevoa_dados },
          {"gltab_nevoa_cor", &shader->uni_gltab_nevoa_cor},
          {"gltab_nevoa_referencia", &shader->uni_gltab_nevoa_referencia },
          {"gltab_especularidade_ligada", &shader->uni_gltab_especularidade_ligada },
          {"gltab_view", &shader->uni_gltab_camera },
          {"gltab_mvm_sombra", &shader->uni_gltab_mvm_sombra },
          {"gltab_mvm_oclusao", &shader->uni_gltab_mvm_oclusao },
          {"gltab_mvm_luz", &shader->uni_gltab_mvm_luz },
          {"gltab_mvm_ajuste_textura", &shader->uni_gltab_mvm_ajuste_textura },
          {"gltab_prm", &shader->uni_gltab_prm },
          {"gltab_prm_sombra", &shader->uni_gltab_prm_sombra },
          {"gltab_dados_raster", &shader->uni_gltab_dados_raster },
          {"gltab_oclusao_ligada", &shader->uni_gltab_oclusao_ligada },
          {"gltab_plano_distante_oclusao", &shader->uni_gltab_plano_distante },
  }) {
    *p_local_atributo = LocalUniforme(shader->programa, nome);
    if (*p_local_atributo == -1) {
      LOG(INFO) << "Shader nao possui uniforme " << nome;
    } else {
      LOG(INFO) << "Uniforme " << nome << " na posicao " << *p_local_atributo;
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
        LOG(INFO) << "Shader '" << shader->nome << "' nao possui uniforme array " << nome_var;
      }
      ++j;
    }
  }

  return true;
}

void IniciaShaders(TipoLuz tipo_luz, interno::Contexto* contexto) {

  V_ERRO("antes vertex shader");
  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  struct DadosShaders {
    std::string nome_programa;
    TipoShader tipo;
    std::string nome_vs;
    std::string nome_fs;
    VarShader* shader;
  };
  std::string nome_programa_luz;
  // Isso aqui era para iniciar o shader de luz com outros tipos. Vou deixar que pode ser que use isso no futuro para novas features.
  // Mas por enquanto, hard coded para luz_especular.
  LOG(INFO) << "Tentando iniciar com shaders, luz especular HARDCODED";
  std::string nome_vert_luz;
  std::string nome_frag_luz;
  nome_programa_luz = "programa_luz_pixel_especular";
  nome_vert_luz = "vert_luz.c";
  nome_frag_luz = "frag_luz_espec.c";
  std::vector<DadosShaders> dados_shaders = {
    { nome_programa_luz, TSH_LUZ, nome_vert_luz, nome_frag_luz, &contexto->shaders[TSH_LUZ] },
    { "programa_simples", TSH_SIMPLES, "vert_simples.c", "frag_simples.c", &contexto->shaders[TSH_SIMPLES] },
    { "programa_caixa_ceu", TSH_CAIXA_CEU, "vert_caixa_ceu.c", "frag_caixa_ceu.c", &contexto->shaders[TSH_CAIXA_CEU] },
    { "programa_picking", TSH_PICKING, "vert_simples.c", "frag_picking.c", &contexto->shaders[TSH_PICKING] },
    { "programa_profundidade", TSH_PROFUNDIDADE, "vert_simples.c", "frag_profundidade.c", &contexto->shaders[TSH_PROFUNDIDADE] },
    { "programa_preto_branco", TSH_PRETO_BRANCO, "vert_preto_branco.c", "frag_preto_branco.c", &contexto->shaders[TSH_PRETO_BRANCO] },
    { "programa_pontual", TSH_PONTUAL, "vert_pontual.c", "frag_pontual.c", &contexto->shaders[TSH_PONTUAL] },
    { "programa_teste", TSH_TESTE, "vert_luz.c", "frag_simples.c", &contexto->shaders[TSH_TESTE] },
  };

  for (auto& ds : dados_shaders) {
    LOG(INFO) << "---------------------------------------------------------------------------";
    LOG(INFO) << "Iniciando programa shaders: " << ds.nome_programa.c_str();
    if (!IniciaShader(ds.nome_programa.c_str(), ds.tipo, ds.nome_vs.c_str(), ds.nome_fs.c_str(), ds.shader)) {
      LOG(ERROR) << "Erro carregando programa com " << ds.nome_vs.c_str() << " e " << ds.nome_fs.c_str();
      continue;
    }
    if (!IniciaVariaveis(ds.shader)) {
      LOG(ERROR) << "Erro carregando variaveis do programa " << ds.nome_programa.c_str();
      continue;
    }
    print_uniforms(ds.shader->programa);
    LOG(INFO) << "Programa shaders '" << ds.nome_programa.c_str() << "' iniciado com sucesso";
    LOG(INFO) << "---------------------------------------------------------------------------";
  }
  UsaShader(TSH_LUZ);
  V_ERRO("usando programa shader luz");
}

}  // namespace

void IniciaComum(TipoLuz tipo_luz, float escala, interno::Contexto* contexto) {
  contexto->pilha_model.push(Matrix4());
  contexto->pilha_camera.push(Matrix4());
  contexto->pilha_prj.push(Matrix4());
  contexto->pilha_mvm_sombra.push(Matrix4());
  contexto->pilha_prj_sombra.push(Matrix4());
  contexto->pilha_mvm_oclusao.push(Matrix4());
  contexto->pilha_mvm_luz.push(Matrix4());
  contexto->pilha_mvm_ajuste_textura.push(Matrix4());
  contexto->pilha_corrente = &contexto->pilha_camera;
  // Essa funcao pode dar excecao, entao eh melhor colocar depois das matrizes pra aplicacao nao crashar e mostrar
  // a mensagem de erro.
  CarregaExtensoes();
  ImprimeExtensoes();
  IniciaShaders(tipo_luz, contexto);
  IniciaVbos();
  IniciaChar();
}

void FinalizaShaders(const VarShader& shader) {
  DesanexaShader(shader.programa, shader.vs);
  DesanexaShader(shader.programa, shader.fs);
  DestroiPrograma(shader.programa);
  DestroiShader(shader.vs);
  DestroiShader(shader.fs);
}

void HabilitaComShader(interno::Contexto* contexto, GLenum cap) {
  auto& shader = BuscaShaderMutavel();
  if (cap == GL_LIGHTING) {
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(uniforme, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 1.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 1.0f);
  } else if (cap == GL_TEXTURE_2D) {
    if (!shader.textura_ligada) {
      interno::UniformeSeValido(shader.uni_gltab_textura, 1.0f);
      shader.textura_ligada = true;
    }
  } else if (cap == GL_TEXTURE_CUBE_MAP) {
    interno::UniformeSeValido(shader.uni_gltab_textura_cubo, 1.0f);
  } else if (cap == GL_FOG) {
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    if (uniforme == -1) {
      return;
    }
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
  auto& shader = BuscaShaderMutavel();
  if (cap == GL_LIGHTING) {
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(uniforme, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luz_direcional_cor, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)], cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_TEXTURE_2D) {
    if (shader.textura_ligada) {
      interno::UniformeSeValido(shader.uni_gltab_textura, 0.0f);
      shader.textura_ligada = false;
    }
  } else if (cap == GL_TEXTURE_CUBE_MAP) {
    interno::UniformeSeValido(shader.uni_gltab_textura_cubo, 0.0f);
  } else if (cap == GL_FOG) {
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    if (uniforme == -1) {
      return;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    Uniforme(shader.uni_gltab_nevoa_cor, cor[0], cor[1], cor[2], 0.0f);
  } else if (cap == GL_NORMALIZE) {
    // Shader ja normaliza tudo.
  } else {
    glDisable(cap);
  }
}

}  // namespace interno

void HabilitaMipmapAniso(GLenum alvo) {
  gl::ParametroTextura(alvo, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if WIN32 || MAC_OSX || (__linux__ && !ANDROID)
  GLfloat aniso = 0;
  gl::Le(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
  if (aniso <= 0) {
    // Trilinear.
    gl::ParametroTextura(alvo, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  } else {
    // Melhora muito as texturas. Mipmap + aniso.
    gl::ParametroTextura(alvo, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    gl::ParametroTextura(alvo, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
  }
#else
  gl::ParametroTextura(alvo, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif

  gl::ParametroTextura(alvo, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl::ParametroTextura(alvo, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if !USAR_OPENGL_ES
  // Nao sei se precisa disso...
  if (alvo == GL_TEXTURE_CUBE_MAP) {
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }
#endif
}

void DesabilitaMipmapAniso(GLenum alvo) {
  gl::ParametroTextura(alvo, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl::ParametroTextura(alvo, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#if WIN32 || MAC_OSX || (__linux__ && !ANDROID)
  GLfloat aniso = 0;
  gl::Le(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
  if (aniso > 0) {
    gl::ParametroTextura(alvo, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
  }
#endif
}

void EmpilhaMatriz() {
  auto* c = interno::BuscaContexto();
  c->pilha_corrente->emplace(c->pilha_corrente->top().get());
}

void DesempilhaMatriz() {
  auto* c = interno::BuscaContexto();
#if DEBUG
  if (c->pilha_corrente->empty()) {
    LOG(ERROR) << "Pilha vazia";
    return;
  }
#endif
  c->pilha_corrente->pop();
}

int ModoMatrizCorrente() {
  auto* c = interno::BuscaContexto();
  if (c->pilha_corrente == &c->pilha_model) { return MATRIZ_MODELAGEM; }
  else if (c->pilha_corrente == &c->pilha_camera) { return MATRIZ_CAMERA; }
  else if (c->pilha_corrente == &c->pilha_prj) { return MATRIZ_PROJECAO; }
  else if (c->pilha_corrente == &c->pilha_prj_sombra) { return MATRIZ_PROJECAO_SOMBRA; }
  else if (c->pilha_corrente == &c->pilha_mvm_sombra) { return MATRIZ_SOMBRA; }
  else if (c->pilha_corrente == &c->pilha_mvm_oclusao) { return MATRIZ_OCLUSAO; }
  else if (c->pilha_corrente == &c->pilha_mvm_luz) { return MATRIZ_LUZ; }
  else if (c->pilha_corrente == &c->pilha_mvm_ajuste_textura) { return MATRIZ_AJUSTE_TEXTURA; }
  else {
    LOG(ERROR) << "Nao ha matriz corrente!!";
    return MATRIZ_MODELAGEM;
  }
}

void MudaModoMatriz(int modo) {
  auto* c = interno::BuscaContexto();
  if (modo == MATRIZ_MODELAGEM) {
    c->pilha_corrente = &c->pilha_model;
  } else if (modo == MATRIZ_CAMERA) {
    c->pilha_corrente = &c->pilha_camera;
  } else if (modo == MATRIZ_PROJECAO) {
    c->pilha_corrente = &c->pilha_prj;
  } else if (modo == MATRIZ_PROJECAO_SOMBRA) {
    c->pilha_corrente = &c->pilha_prj_sombra;
  } else if (modo == MATRIZ_SOMBRA) {
    c->pilha_corrente = &c->pilha_mvm_sombra;
  } else if (modo == MATRIZ_OCLUSAO) {
    c->pilha_corrente = &c->pilha_mvm_oclusao;
  } else if (modo == MATRIZ_LUZ) {
    c->pilha_corrente = &c->pilha_mvm_luz;
  } else if (modo == MATRIZ_AJUSTE_TEXTURA) {
    c->pilha_corrente = &c->pilha_mvm_ajuste_textura;
  } else {
    LOG(ERROR) << "Modo invalido: " << (int)modo;
  }
  //AtualizaMatrizes();
}

void CarregaIdentidade() {
  interno::BuscaContexto()->pilha_corrente->top().identity();
}

void MultiplicaMatriz(const GLfloat* matriz) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  const Matrix4 m4(matriz);
  topo *= m4;
}

void Escala(GLfloat x, GLfloat y, GLfloat z) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  const Matrix4 m4 = Matrix4().scale(x, y, z);
  topo *= m4;
}

void Translada(GLfloat x, GLfloat y, GLfloat z) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  const auto m4 = Matrix4().translate(x, y, z);
  topo *= m4;
}

void Roda(GLfloat angulo_graus, GLfloat x, GLfloat y, GLfloat z) {
  auto& topo = interno::BuscaContexto()->pilha_corrente->top();
  const Matrix4 m4 = Matrix4().rotate(angulo_graus, x, y, z);
  topo *= m4;
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
  if (interno::BuscaShader().atr_gltab_normal != -1) {
    PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_normal, 3  /**dimensoes*/, tipo, GL_FALSE, passo, normais);
  }
}

void PonteiroTangentes(GLenum tipo, GLsizei passo, const GLvoid* tangentes) {
  if (interno::BuscaShader().atr_gltab_tangente != -1) {
    PonteiroAtributosVertices(interno::BuscaShader().atr_gltab_tangente, 3  /**dimensoes*/, tipo, GL_FALSE, passo, tangentes);
  }
}

void Normal(GLfloat x, GLfloat y, GLfloat z) {
  if (interno::BuscaShader().atr_gltab_normal != -1) {
    AtributoVertice(interno::BuscaShader().atr_gltab_normal, x, y, z);
  }
}

void Tangente(GLfloat x, GLfloat y, GLfloat z) {
  if (interno::BuscaShader().atr_gltab_tangente != -1) {
    AtributoVertice(interno::BuscaShader().atr_gltab_tangente, x, y, z);
  }
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
    GLint uniforme = shader.uni_gltab_luz_ambiente_cor;
    if (uniforme == -1) {
      return false;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    return cor[3] > 0.0f;
  } else if (cap == GL_LIGHT0) {
    GLint uniforme = shader.uni_gltab_luz_direcional_cor;
    if (uniforme == -1) {
      return false;
    }
    GLfloat cor[4];
    LeUniforme(shader.programa, uniforme, cor);
    return cor[3] > 0;
  } else if (cap >= GL_LIGHT1 && cap <= GL_LIGHT7) {
    GLint uniforme = shader.uni_gltab_luzes[interno::IndiceLuzCor(cap - GL_LIGHT1)];
    if (uniforme == -1) {
      return false;
    }
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
  } else if (cap == GL_TEXTURE_CUBE_MAP) {
    if (shader.uni_gltab_textura_cubo == -1) {
      return false;
    }
    GLfloat fret;
    LeUniforme(shader.programa, shader.uni_gltab_textura_cubo, &fret);
    return fret > 0.5f;
  } else if (cap == GL_FOG) {
    GLint uniforme = shader.uni_gltab_nevoa_cor;
    if (uniforme == -1) {
      return false;
    }
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
  interno::UniformeSeValido(interno::BuscaShader().uni_gltab_luz_ambiente_cor, r, g, b, 1.0f);
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
  vp.normalize();
  //LOG(ERROR) << "vp: " << vp.x << ", " << vp.y << ", " << vp.z;

//  float glm[16];
//  gl::Le(GL_MODELVIEW_MATRIX, glm);
//  Matrix4 m(glm);
//  Vector4 vp(pos[0], pos[1], pos[2], pos[3]);
//  vp = m * vp;
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

void Especularidade(bool ligado) {
  if (!interno::UsandoShaderLuz()) {
    return;
  }
  const auto& shader = interno::BuscaShader();
  Uniforme(shader.uni_gltab_especularidade_ligada, ligado);
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

void Oclusao(bool valor) {
  const auto& shader = interno::BuscaShader();
  interno::UniformeSeValido(shader.uni_gltab_oclusao_ligada, valor ? 1.0f : 0.0f);
}

bool OclusaoLigada() {
  const auto& shader = interno::BuscaShader();
  if (shader.uni_gltab_oclusao_ligada == -1) {
    return false;
  }
  GLint oclusao;
  LeUniforme(shader.programa, shader.uni_gltab_oclusao_ligada, &oclusao);
  return oclusao > 0.0f;
}

void PlanoDistanteOclusao(GLfloat distancia) {
  const auto& shader = interno::BuscaShader();
  interno::UniformeSeValido(shader.uni_gltab_plano_distante, distancia);
  auto* c = interno::BuscaContexto();
  c->plano_distante = distancia;  // salva no contexto, caso haja alguma mudanca de shaders.
}

void Perspectiva(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
  // Copiado do glues.
  GLfloat m[4][4];
  GLfloat sine, cotangent, deltaZ;
  GLfloat radians = (GLfloat)(fovy / 2.0f * __glPi /180.0f);

  deltaZ= zFar - zNear;
  sine= (GLfloat)sinf(radians);
  if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f)) {
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
  if (modo == MATRIZ_MODELAGEM) {
    return c->pilha_model.top();
  } else if (modo == MATRIZ_CAMERA) {
    return c->pilha_camera.top();
  } else if (modo == MATRIZ_PROJECAO) {
    return c->pilha_prj.top();
  } else if (modo == MATRIZ_SOMBRA) {
    return c->pilha_mvm_sombra.top();
  } else if (modo == MATRIZ_PROJECAO_SOMBRA) {
    return c->pilha_prj_sombra.top();
  } else if (modo == MATRIZ_OCLUSAO) {
    return c->pilha_mvm_oclusao.top();
  } else if (modo == MATRIZ_LUZ) {
    return c->pilha_mvm_luz.top();
  } else if (modo == MATRIZ_AJUSTE_TEXTURA) {
    return c->pilha_mvm_ajuste_textura.top();
  } else {
    LOG(ERROR) << "Tipo de matriz invalido: " << (int)modo;
    return c->pilha_mvm_sombra.top();
  }
}

bool TemExtensao(const std::string& nome_extensao) {
  const auto& extensoes = interno::BuscaContexto()->extensoes;
  return extensoes.find(nome_extensao) != extensoes.end();
}

void Le(GLenum nome_parametro, GLfloat* valor) {
  auto* c = interno::BuscaContexto();
  if (nome_parametro == GL_MODELVIEW_MATRIX) {
    Matrix4 mvm = c->pilha_camera.top() * c->pilha_model.top();
    memcpy(valor, mvm.get(), 16 * sizeof(float));
  } else if (nome_parametro == MATRIZ_MODELAGEM) {
    memcpy(valor, c->pilha_model.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == MATRIZ_CAMERA) {
    memcpy(valor, c->pilha_camera.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == GL_PROJECTION_MATRIX) {
    memcpy(valor, c->pilha_prj.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == MATRIZ_AJUSTE_TEXTURA) {
    memcpy(valor, c->pilha_mvm_ajuste_textura.top().get(), 16 * sizeof(float));
  } else if (nome_parametro == MATRIZ_SOMBRA) {
    memcpy(valor, c->pilha_mvm_sombra.top().get(), 16 * sizeof(float));
  } else {
    glGetFloatv(nome_parametro, valor);
  }
}

namespace {

GLuint TipoAtribParaIndice(atributo_e tipo) {
  GLuint ret = -1;
  switch (tipo) {
    case ATR_VERTEX_ARRAY: ret = interno::BuscaShader().atr_gltab_vertice; break;
    case ATR_NORMAL_ARRAY: ret = interno::BuscaShader().atr_gltab_normal; break;
    case ATR_TANGENT_ARRAY: ret = interno::BuscaShader().atr_gltab_tangente; break;
    case ATR_COLOR_ARRAY: ret = interno::BuscaShader().atr_gltab_cor; break;
    case ATR_TEXTURE_COORD_ARRAY: ret = interno::BuscaShader().atr_gltab_texel; break;
    case ATR_MODEL_MATRIX_ARRAY: ret = interno::BuscaShader().atr_gltab_matriz_modelagem; break;
    case ATR_NORMAL_MATRIX_ARRAY: ret = interno::BuscaShader().atr_gltab_matriz_normal; break;
    default:
      LOG(ERROR) << "tipo invalido: " << (int)tipo;
      break;
  }
  if (ret >= GL_MAX_VERTEX_ATTRIBS) {
    VLOG(3) << "valor invalido para tipo: " << tipo << ", ret: " << ret;
  }
  return ret;
}

}  // namespace

bool HabilitaVetorAtributosVerticePorTipo(atributo_e tipo) {
  GLuint indice = TipoAtribParaIndice(tipo);
  if (indice >= GL_MAX_VERTEX_ATTRIBS) return false;
  HabilitaVetorAtributosVertice(indice);
  switch (tipo) {
    case ATR_MODEL_MATRIX_ARRAY:
      if ((indice + 3) >= GL_MAX_VERTEX_ATTRIBS) return false;
      HabilitaVetorAtributosVertice(indice + 1);
      HabilitaVetorAtributosVertice(indice + 2);
      HabilitaVetorAtributosVertice(indice + 3);
      break;
    case ATR_NORMAL_MATRIX_ARRAY:
      if ((indice + 2) >= GL_MAX_VERTEX_ATTRIBS) return false;
      HabilitaVetorAtributosVertice(indice + 1);
      HabilitaVetorAtributosVertice(indice + 2);
      break;
    default:
      ;
  }
  return true;
}

void DesabilitaVetorAtributosVerticePorTipo(atributo_e tipo) {
  GLuint indice = TipoAtribParaIndice(tipo);
  if (indice >= GL_MAX_VERTEX_ATTRIBS) return;
  DesabilitaVetorAtributosVertice(indice);
  switch (tipo) {
    case ATR_MODEL_MATRIX_ARRAY:
      if ((indice + 3) >= GL_MAX_VERTEX_ATTRIBS) return;
      DesabilitaVetorAtributosVertice(indice + 1);
      DesabilitaVetorAtributosVertice(indice + 2);
      DesabilitaVetorAtributosVertice(indice + 3);
      break;
    case ATR_NORMAL_MATRIX_ARRAY:
      if ((indice + 2) >= GL_MAX_VERTEX_ATTRIBS) return;
      DesabilitaVetorAtributosVertice(indice + 1);
      DesabilitaVetorAtributosVertice(indice + 2);
      break;
    default:
      ;
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
  interno::UniformeSeValido(shader->uni_gltab_unidade_textura_cubo, 2);
  interno::UniformeSeValido(shader->uni_gltab_unidade_textura_oclusao, 3);
  interno::UniformeSeValido(shader->uni_gltab_unidade_textura_luz, 4);
  //interno::UniformeSeValido(shader->uni_gltab_unidade_textura_bump, 5);

  interno::UniformeSeValido(shader->uni_gltab_plano_distante, c->plano_distante);

  shader->cache_textura.clear();

  VLOG(3) << "Alternando para programa de shader: " << c->shader_corrente->nome;
}

TipoShader TipoShaderCorrente() {
  auto* c = interno::BuscaContexto();
  return c->shader_corrente->tipo;
}

namespace {
GLint IdMatrizCorrente(const interno::VarShader& shader) {
  switch (ModoMatrizCorrente()) {
    case MATRIZ_CAMERA:           return shader.uni_gltab_camera;
    case MATRIZ_AJUSTE_TEXTURA:   return shader.uni_gltab_mvm_ajuste_textura;
    case MATRIZ_PROJECAO:         return shader.uni_gltab_prm;
    case MATRIZ_PROJECAO_SOMBRA:  return shader.uni_gltab_prm_sombra;
    case MATRIZ_OCLUSAO:          return shader.uni_gltab_mvm_oclusao;
    case MATRIZ_LUZ:              return shader.uni_gltab_mvm_luz;
    case MATRIZ_SOMBRA:           return shader.uni_gltab_mvm_sombra;
    default:
      LOG(INFO) << "id corrente invalido: " << ModoMatrizCorrente();
      return shader.uni_gltab_mvm_sombra;
  }
}
}  // namespace

void AtualizaMatrizProjecao() {
  auto* c = interno::BuscaContexto();
  const interno::VarShader& shader = interno::BuscaShader();
  GLuint mloc = shader.uni_gltab_prm;
  Matriz4Uniforme(mloc, 1, false, c->pilha_prj.top().get());
}

void AtualizaMatrizCamera() {
  auto* c = interno::BuscaContexto();
  const interno::VarShader& shader = interno::BuscaShader();
  GLuint mloc = shader.uni_gltab_camera;
  Matriz4Uniforme(mloc, 1, false, c->pilha_camera.top().get());
}

void PonteiroMatrizModelagem(const void* matriz_modelagem) {
  const auto& shader = interno::BuscaShader();
  if (shader.atr_gltab_matriz_modelagem != -1) {
    // Pode ser que tenha que usar esse stride ao inves de 0.
    const int stride = 16 * sizeof(float);
    const float* pf = reinterpret_cast<const float*>(matriz_modelagem);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_modelagem,     4, GL_FLOAT, GL_FALSE, stride, pf);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_modelagem + 1, 4, GL_FLOAT, GL_FALSE, stride, pf + 4);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_modelagem + 2, 4, GL_FLOAT, GL_FALSE, stride, pf + 8);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_modelagem + 3, 4, GL_FLOAT, GL_FALSE, stride, pf + 12);
    DivisorAtributoVertice(shader.atr_gltab_matriz_modelagem, 1);
    DivisorAtributoVertice(shader.atr_gltab_matriz_modelagem + 1, 1);
    DivisorAtributoVertice(shader.atr_gltab_matriz_modelagem + 2, 1);
    DivisorAtributoVertice(shader.atr_gltab_matriz_modelagem + 3, 1);
  }
}

void PonteiroMatrizNormal(const void* matriz_normal) {
  const auto& shader = interno::BuscaShader();
  if (shader.atr_gltab_matriz_normal != -1) {
    // Pode ser que tenha que usar esse stride ao inves de 0.
    const int stride = 9 * sizeof(float);
    const float* pf = reinterpret_cast<const float*>(matriz_normal);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_normal,     3, GL_FLOAT, GL_FALSE, stride, pf);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_normal + 1, 3, GL_FLOAT, GL_FALSE, stride, pf + 3);
    PonteiroAtributosVertices(
        shader.atr_gltab_matriz_normal + 2, 3, GL_FLOAT, GL_FALSE, stride, pf + 6);

    DivisorAtributoVertice(shader.atr_gltab_matriz_normal, 1);
    DivisorAtributoVertice(shader.atr_gltab_matriz_normal + 1, 1);
    DivisorAtributoVertice(shader.atr_gltab_matriz_normal + 2, 1);
  }
}

namespace {
void AtualizaMatrizNormal() {
  auto shader = interno::BuscaShader();
  if (shader.atr_gltab_matriz_normal != -1) {
    // Matriz normal: inverso da transposta da MV ( V * M na verdade).
    // Porem, sabemos que V é ortognal, portanto seu inverso é sua transposta.
    // Alem disso, o inverso do produto (V * M)^1 = M^1 * V^1.
    // O tranposto disso (M^1 * V^1)^t = V^1^t * M^1^t.
    // Como V^1 = V^t, V^1^t = V.
    // Portanto, so vamos salvar a parte de model e fazer o resto no shader, assim nao precisamo recomputar
    // a normal quando a camera mexer.
    Matrix3 matriz_normal = interno::ExtraiMatrizNormal(interno::BuscaContexto()->pilha_model.top());
    const float* me = matriz_normal.get();
    AtributoVertice(shader.atr_gltab_matriz_normal,     me[0],  me[1],  me[2]);
    AtributoVertice(shader.atr_gltab_matriz_normal + 1, me[3],  me[4],  me[5]);
    AtributoVertice(shader.atr_gltab_matriz_normal + 2, me[6],  me[7],  me[8]);
  }
}

void AtualizaMatrizModelagemNormal() {
  auto shader = interno::BuscaShader();
  if (shader.atr_gltab_matriz_modelagem != -1) {
    auto* c = interno::BuscaContexto();
    const auto& m = c->pilha_model.top();
    const float* me = m.get();
    AtributoVertice(shader.atr_gltab_matriz_modelagem,     me[0],  me[1],  me[2],  me[3]);
    AtributoVertice(shader.atr_gltab_matriz_modelagem + 1, me[4],  me[5],  me[6],  me[7]);
    AtributoVertice(shader.atr_gltab_matriz_modelagem + 2, me[8],  me[9],  me[10], me[11]);
    AtributoVertice(shader.atr_gltab_matriz_modelagem + 3, me[12], me[13], me[14], me[15]);
  }
  AtualizaMatrizNormal();
}
}  // namespace

void AtualizaMatrizes() {
  auto* c = interno::BuscaContexto();
  int modo = ModoMatrizCorrente();
  if (modo == MATRIZ_MODELAGEM) {
    AtualizaMatrizModelagemNormal();
    return;
  }
  const interno::VarShader& shader = interno::BuscaShader();
  GLuint mloc = IdMatrizCorrente(shader);
  Matriz4Uniforme(mloc, 1, false, c->pilha_corrente->top().get());
}

void AtualizaTodasMatrizes() {
  auto* c = interno::BuscaContexto();
  const interno::VarShader& shader = interno::BuscaShader();
  struct DadosMatriz {
    GLint id;
    Matrix4* matriz;
  };
  std::vector<DadosMatriz> dados_matriz = {
    { shader.uni_gltab_prm, &c->pilha_prj.top() },
    { shader.uni_gltab_mvm_sombra, &c->pilha_mvm_sombra.top() },
    { shader.uni_gltab_prm_sombra, &c->pilha_prj_sombra.top() },
    { shader.uni_gltab_mvm_oclusao, &c->pilha_mvm_oclusao.top() },
    { shader.uni_gltab_mvm_luz, &c->pilha_mvm_luz.top() },
    { shader.uni_gltab_mvm_ajuste_textura, &c->pilha_mvm_ajuste_textura.top() },
    { shader.uni_gltab_camera, &c->pilha_camera.top() },
  };
  for (const auto& dm : dados_matriz) {
    if (dm.id != -1) {
      Matriz4Uniforme(dm.id, 1, false, dm.matriz->get());
    }
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
  //unsigned int media_tela = (largura_viewport + altura_viewport) / 2;
  //*escala = std::max(media_tela / 500, 1U);
  *escala = interno::BuscaContexto()->escala;
#elif __APPLE__
  *escala = interno::BuscaContexto()->escala;
  *largura_fonte = 8;
  *altura = 13;
  return;
#else
  *escala = 1;
  *largura_fonte = 8;
  *altura = 13;
  return;
#endif
  *largura_fonte = 8;
  *altura = 13;
}

bool PosicaoRaster(GLfloat x, GLfloat y, GLfloat z) {
  float matriz_mv[16];
  float matriz_pr[16];
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::Le(GL_MODELVIEW_MATRIX, matriz_mv);
  gl::Le(GL_PROJECTION_MATRIX, matriz_pr);
  float x2d, y2d, z2d;
  // z < 0 significa projecao atras do olho.
  if (!glu::Project(x, y, z, matriz_mv, matriz_pr, viewport, &x2d, &y2d, &z2d) || z2d < 0) {
    return false;
  }
  auto* contexto = interno::BuscaContexto();
  contexto->raster_x = x2d;
  contexto->raster_y = y2d;
  //LOG(INFO) << "raster_x: " << x2d << ", raster_y: " << y2d;
  return true;
}

bool PosicaoRasterAbsoluta(GLint x, GLint y) {
  auto* contexto = interno::BuscaContexto();
  contexto->raster_x = x;
  contexto->raster_y = y;
  return true;
}

bool PosicaoRaster(GLint x, GLint y) {
  return PosicaoRaster(static_cast<float>(x), static_cast<float>(y), 0.0f);
}

void AlternaModoDebug() {
  auto* c = interno::BuscaContexto();
  c->depurar_selecao_por_cor = !c->depurar_selecao_por_cor;
  //c->AlternaSelecaoPorCor();
  //LOG(INFO) << "selecao por cor: " << c->SelecaoPorCorHabilitada();
  LOG(INFO) << "selecao por cor: " << c->depurar_selecao_por_cor;
}

void CarregaNome(GLuint id) {
  auto* c = interno::BuscaContexto();
  if (c->UsarSelecaoPorCor()) {
    GLubyte rgb[3];
    interno::MapeiaId(id, rgb);
    VLOG(2) << "Mapeando " << id << ", tipo " << c->bit_pilha
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
    VLOG(1) << "Empilhando tipo: " << c->bit_pilha;
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
        //glFlush();
        //glFinish();
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
          // Em detecao de colisao, eh comum acontecer.
          VLOG(1) << "Id nao mapeado, tipo: " << tipo_objeto
                  << ", identificador: " << (id_mapeado & 0xFFFF)
                  << ", tudo: " << (void*)id_mapeado;
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
      LOG(WARNING) << "Luz nao deve ser usada com selecao";
      return;
    }
  }
  interno::HabilitaComShader(c, cap);
}

void Desabilita(GLenum cap) {
  interno::DesabilitaComShader(interno::BuscaContexto(), cap);
  V_ERRO((std::string("desabilitando es cap: ") + net::to_string((int)cap)).c_str());
}

void UnidadeTextura(GLenum unidade) {
#if WIN32
  interno::TexturaAtivaInterno(unidade);
  //glClientActiveTexture(unidade);
#else
  glActiveTexture(unidade);
  //glClientActiveTexture(unidade);
#endif
  interno::BuscaShaderMutavel().unidade_textura = unidade;
}

void TexturaBump(bool estado) {
  interno::UniformeSeValido(interno::BuscaShader().uni_gltab_textura_bump, estado ? 1.0f : 0.0f);
}

void LigacaoComTextura(GLenum alvo, GLuint textura) {
  auto& shader = interno::BuscaShaderMutavel();
  auto chave = std::make_pair(shader.unidade_textura, alvo);
  if (auto it = shader.cache_textura.find(chave);
      it == shader.cache_textura.end() || it->second != textura) {
    shader.cache_textura[chave] = textura;
    glBindTexture(alvo, textura);
    //LOG(INFO) << "miss, alvo: " << alvo << ", textura: " << textura;
  //} else {
    //LOG(INFO) << "hit, alvo: " << alvo << ", textura: " << textura;
  }
}

void CorMisturaPreNevoa(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
  interno::UniformeSeValido(interno::BuscaShader().uni_gltab_cor_mistura_pre_nevoa, r, g, b, a);
}

void LeCorMisturaPreNevoa(GLfloat* rgb) {
  if (interno::BuscaShader().uni_gltab_cor_mistura_pre_nevoa == -1) return;
  GLfloat cor[4];
  const auto& shader = interno::BuscaShader();
  LeUniforme(shader.programa, shader.uni_gltab_cor_mistura_pre_nevoa, cor);
  rgb[0] = cor[0];
  rgb[1] = cor[1];
  rgb[2] = cor[2];
  rgb[3] = cor[3];
}

void FinalizaGl() {
#if WIN32
  // Apagar o contexto_interno
#endif
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_LUZ]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_SIMPLES]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_PONTUAL]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_PRETO_BRANCO]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_PICKING]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_CAIXA_CEU]);
  interno::FinalizaShaders(interno::BuscaContexto()->shaders[TSH_TESTE]);
}

bool SelecaoPorCor() {
  return interno::BuscaContexto()->SelecaoPorCorHabilitada();
}

}  // namespace gl
