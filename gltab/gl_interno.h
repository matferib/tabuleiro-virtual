#ifndef GLTAB_INTERNO_H
#define GLTAB_INTERNO_H

#include "gltab/gl.h"

namespace gl {

// Namespace para utilidades internas, nem deveria estar aqui.
namespace interno {

// Variaveis de shaders.
struct VarShader {
  std::string nome;  // Nome para o programa, depuracao apenas.

  // Shader.
  GLuint programa;
  GLuint vs;
  GLuint fs;
  GLuint programa_simples;
  GLuint vs_simples;
  GLuint fs_simples;

  // Variaveis uniformes dos shaders.
  GLint uni_gltab_luz_ambiente_cor;     // Cor da luz ambiente. Alfa indica se iluminacao geral esta ligada.
  GLint uni_gltab_luz_direcional_cor;   // Cor da luz direcional.
  GLint uni_gltab_luz_direcional_pos;   // Posicao da luz direcional ().
  GLint uni_gltab_luzes[7 * 3];         // Luzes pontuais: 7 luzes InfoLuzPontual (3 vec4: pos, cor, atributos).
  GLint uni_gltab_textura;              // Ha textura: 1, nao ha: 0.
  GLint uni_gltab_unidade_textura;
  GLint uni_gltab_nevoa_dados;          // Dados da nevoa: inicio, fim, escala.
  GLint uni_gltab_nevoa_cor;            // Cor da nevoa.
  GLint uni_gltab_nevoa_referencia;     // Ponto de referencia da nevoa.
  GLint uni_gltab_dados_raster;         // p = Tamanho do ponto.
  GLint uni_gltab_mvm;                  // Matrix modelview.
  GLint uni_gltab_nm;                   // Matrix de normais.
  GLint uni_gltab_prm;                  // Matrix projecao.
  // Atributos do vertex shader.
  GLint atr_gltab_vertice;
  GLint atr_gltab_normal;
  GLint atr_gltab_cor;
  GLint atr_gltab_texel;
};

// Usado para indexar os shaders.
enum TipoShader {
  TSH_LUZ = 0,
  TSH_SIMPLES = 1,
  TSH_NUM,  // numero de shaders.
};

// Depende de plataforma.
struct ContextoDependente {
  virtual ~ContextoDependente() {}
};

// Contexto comum.
class Contexto {
 public:
  explicit Contexto(ContextoDependente* cd) : shaders(TSH_NUM), interno(cd) {}
  ~Contexto() {}

  bool depurar_selecao_por_cor = false;  // Mudar para true para depurar selecao por cor.
  float raster_x = 0.0f;
  float raster_y = 0.0f;

  std::vector<VarShader> shaders;
  VarShader* shader_corrente = nullptr;  // Aponta para o shader corrente.

  // Matrizes correntes. Ambas as pilhas sao iniciadas com a identidade.
  std::stack<Matrix4> pilha_mvm;
  std::stack<Matrix4> pilha_prj;
  std::stack<Matrix4>* pilha_corrente = nullptr;
  Matrix3 matriz_normal;  // Computada da mvm corrente.

  std::unique_ptr<ContextoDependente> interno;

  // Selecao por cor.
  // Mapeia um ID para a cor RGB em 21 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  modo_renderizacao_e modo_renderizacao = MR_RENDER;
  GLuint* buffer_selecao = nullptr;
  unsigned int proximo_id = 0;
  // O tipo de objeto tres bits (valor de [0 a 7]).
  unsigned int bit_pilha = 0;
  GLuint tam_buffer = 0;

  inline bool UsarSelecaoPorCor() const {
    return modo_renderizacao == MR_SELECT || depurar_selecao_por_cor;
  }

 private:
  Contexto();
};
Contexto* BuscaContexto();
inline const VarShader& BuscaShader(TipoShader ts) { return BuscaContexto()->shaders[ts]; }
inline const VarShader& BuscaShader() { return *BuscaContexto()->shader_corrente; }
inline bool UsandoShaderLuz() {
  auto* c = BuscaContexto();
  return c->shader_corrente == &c->shaders[TSH_LUZ];
}

bool LuzPorVertice(int argc, const char* const * argv);  // Retorna true se encontrar --luz_por_vertice.
void IniciaComum(bool luz_por_vertice, interno::Contexto* contexto);
void FinalizaShaders(const VarShader& shader);
void HabilitaComShader(interno::Contexto* contexto, GLenum cap);
void DesabilitaComShader(interno::Contexto* contexto, GLenum cap);

// Quebra uma string em varias.
const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra);

}  // namespace interno

}  // namespace gl


#endif
