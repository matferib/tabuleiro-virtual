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
  GLint uni_gltab_unidade_textura_sombra;
  GLint uni_gltab_nevoa_dados;          // Dados da nevoa: inicio, fim, escala.
  GLint uni_gltab_nevoa_cor;            // Cor da nevoa.
  GLint uni_gltab_nevoa_referencia;     // Ponto de referencia da nevoa.
  GLint uni_gltab_dados_raster;         // p = Tamanho do ponto.
  GLint uni_gltab_mvm;                  // Matrix modelview.
  GLint uni_gltab_mvm_sombra;           // Matrix modelview sombra.
  GLint uni_gltab_nm;                   // Matrix de normais.
  GLint uni_gltab_prm;                  // Matrix projecao.
  GLint uni_gltab_prm_sombra;           // Matrix projecao sombra.
  // Atributos do vertex shader.
  GLint atr_gltab_vertice;
  GLint atr_gltab_normal;
  GLint atr_gltab_cor;
  GLint atr_gltab_texel;
};

// Depende de plataforma.
struct ContextoDependente {
  virtual ~ContextoDependente() {}
};

// Mapeia um id para as cores rgb.
void MapeiaId(unsigned int id, GLubyte rgb[3]);

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
  std::stack<Matrix4> pilha_mvm_sombra;
  std::stack<Matrix4> pilha_prj_sombra;
  std::stack<Matrix4>* pilha_corrente = nullptr;
  Matrix3 matriz_normal;  // Computada da mvm corrente.

  std::unique_ptr<ContextoDependente> interno;

  // Mapeia um ID para a cor RGB em 21 bits (os dois mais significativos sao para a pilha).
  std::unordered_map<unsigned int, unsigned int> ids;
  modo_renderizacao_e modo_renderizacao = MR_RENDER;
  GLuint* buffer_selecao = nullptr;
  unsigned int proximo_id = 0;
  // O tipo de objeto tres bits (valor de [0 a 7]).
  unsigned int bit_pilha = 0;
  GLuint tam_buffer = 0;

  inline bool SelecaoPorCorHabilitada() const {
    return selecao_por_cor_habilitada_;
  }
  // So alterna se nao houver OpenGL ES, pois este nao tem as funcoes de selecao.
  inline void AlternaSelecaoPorCor() {
#if !USAR_OPENGL_ES
    selecao_por_cor_habilitada_ = !selecao_por_cor_habilitada_;
#endif
  }
  inline bool UsarSelecaoPorCor() const {
    return selecao_por_cor_habilitada_ && (modo_renderizacao == MR_SELECT || depurar_selecao_por_cor);
  }

 private:
  Contexto();
  // Selecao por cor.
  bool selecao_por_cor_habilitada_ = true;
};
Contexto* BuscaContexto();
inline const VarShader& BuscaShader(TipoShader ts) { return BuscaContexto()->shaders[ts]; }
inline const VarShader& BuscaShader() { return *BuscaContexto()->shader_corrente; }
inline bool UsandoShaderComNevoa() {
  auto* c = BuscaContexto();
  return c->shader_corrente == &c->shaders[TSH_LUZ] || c->shader_corrente == &c->shaders[TSH_PRETO_BRANCO];
}
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
