#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include "ent/entidade.pb.h"
#include "gltab/gl.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf
namespace ent {

class Entidade;
class DesenhoBase;
class IluminacaoDirecional;

/** Informacao sobre vertices para VBO. */
struct InfoVertice {
  float x, y, z;     // Vertex.
  float nx, ny, nz;  // Normal.
  float s0, t0;      // Textura.
};

/** Interface de texturas para entidades. */
class Texturas {
 public:
  /** Retorna o id da textura ou GL_INVALID_VALUE. */
  virtual unsigned int Textura(const std::string& id) const = 0;
};

/** Constroi uma entidade de acordo com o proto passando, inicializando-a. */
Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas* texturas, ntf::CentralNotificacoes* central);

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico.
*/
class Entidade {
 public:
  /** Inicializa a entidade, recebendo seu proto diretamente. */
  void Inicializa(const EntidadeProto& proto);

  /** Atualiza a entidade usando apenas alguns campos do proto passado. Nao atualiza posicao. */
  void AtualizaProto(const EntidadeProto& novo_proto);

  /** Atualiza a posição da entidade em direção a seu destino. Ao alcançar o destino, o limpa. */
  void Atualiza();

  /** Destroi a entidade. */
  ~Entidade();

  /** @return o identificador da entidade que deve ser unico globalmente. */
  unsigned int Id() const { return proto_.id(); }

  TipoEntidade Tipo() const { return proto_.tipo(); }

  /** Retorna um VBO que representa a entidade (valido para FORMAS e COMPOSTAS). */
  static const gl::Vbo ExtraiVbo(const ent::EntidadeProto& proto);

  /** Move a entidade para o ponto especificado. Limpa destino. */
  void MovePara(float x, float y, float z = 0);
  void MovePara(const Posicao& proto);

  /** Move a entidade uma quantidade em cada eixo. Limpa destino. */
  void MoveDelta(float dx, float dy, float dz);

  /** Atribui um destino a entidade. A cada atualizacao ela se movera em direcao ao destino. */
  void Destino(const Posicao& pos);

  /** Altera a translacao em Z da entidade. */
  void AlteraTranslacaoZ(float delta_translacao);

  float TranslacaoZ() const { return proto_.translacao_z(); }

  /** Altera a rotacao em Z da entidade. */
  void AlteraRotacaoZ(float delta_rotacao_graus);

  float RotacaoZGraus() const { return proto_.rotacao_z_graus(); }

  /** @return o HP da unidade. */
  int PontosVida() const;

  /** @return a coordenada (x). */
  float X() const;

  /** @return a coordenada (y). */
  float Y() const;

  /** @return a coordenada (z). Se delta == true, aplica o delta de voo se a entidade estiver voando. */
  float Z() const;

  /** Retorna as coordenadas do objeto como posicao. */
  const Posicao& Pos() const { return proto_.pos(); }

  /** Adiciona um evento para acontecer em rodadas. */
  void AdicionaEvento(int rodadas, const std::string& descricao) { proto_.add_evento()->set_rodadas(rodadas); }

  /** Mata a entidade, ligando os bits de queda, morte e desligando voo e destino. */
  void MataEntidade();

  /** Atualiza os pontos de vida da entidade para a quantidade passada. */
  void AtualizaPontosVida(int pontos_vida);

  /** Atualiza apenas os campos presentes no proto para a entidade. */
  void AtualizaParcial(const EntidadeProto& proto_parcial);

  /** Atualiza a acao realizada pela entidade nos comandos de acao. */
  void AtualizaAcao(const std::string& id_acao);
  const std::string Acao() const { return proto_.ultima_acao(); }

  /** @return a posicao das acoes da entidade. */
  const Posicao PosicaoAcao() const;

  /** As luzes devem ser desenhadas primeiro, portanto há uma função separada para elas. */
  void DesenhaLuz(ParametrosDesenho* pd);

  /** desenha o objeto de forma solida. Pode alterar os parametros de desenho. */
  void Desenha(ParametrosDesenho* pd);

  /** Desenha a unidade de forma translucida. */
  void DesenhaTranslucido(ParametrosDesenho* pd);

  /** Desenha aura da entidade. */
  void DesenhaAura(ParametrosDesenho* pd);

  /** Monta a matriz de shear de acordo com posicao da luz, anula eixo Z e desenha o objeto com transparencia. */
  void DesenhaSombra(ParametrosDesenho* pd, const float* matriz_shear);

  /** Retorna o proto da entidade. */
  const EntidadeProto& Proto() const { return proto_; }

  /** Retorna se a entidade eh selecionavel para jogador. */
  bool SelecionavelParaJogador() const { return proto_.selecionavel_para_jogador(); }

  /** Retorna se a entidade eh fixa (ou seja, nem o mestre pode mover). */
  bool Fixa() const { return proto_.fixa(); }

  /** Retorna o multiplicador de tamanho para a entidade de acordo com seu tamanho. Por exemplo, retorna
  * 1.0f para entidades medias.
  */
  float MultiplicadorTamanho() const;

  /** Limpa a proxima salvacao para a entidade. */
  void AtualizaProximaSalvacao(ResultadoSalvacao rs);

  /** Retorna a proxima salvacao para a entidade. */
  ResultadoSalvacao ProximaSalvacao() const { return static_cast<ResultadoSalvacao>(proto_.proxima_salvacao()); }

  /** Atribui a direcao de queda da entidade. */
  void AtualizaDirecaoDeQueda(float x, float y, float z);

  /** Desenha um objeto a partir de seu proto. Usado para desenhar de forma simples objetos (por exemplo, formas sendo adicionadas).
  * Implementado em entidade_desenha.cpp.
  */
  static void DesenhaObjetoProto(
      const EntidadeProto& proto, ParametrosDesenho* pd, const float* matriz_shear = nullptr);

  /** Carrega modelos usados pelas entidades. */
  static void IniciaGl();

  // Id de entidade invalido.
  static constexpr unsigned int IdInvalido = 0xFFFFFFFF;

 protected:
  friend Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas*, ntf::CentralNotificacoes*);
  Entidade(const Texturas* texturas, ntf::CentralNotificacoes* central);

 private:
  // Nome dos buffers de VBO.
  constexpr static unsigned short NUM_VBOS = 11;
  constexpr static unsigned short VBO_PEAO = 0, VBO_TIJOLO_BASE = 1, VBO_TELA_TEXTURA = 2, VBO_CUBO = 3, VBO_ESFERA = 4, VBO_PIRAMIDE = 5, VBO_CILINDRO = 6, VBO_DISCO = 7, VBO_RETANGULO = 8, VBO_TRIANGULO = 9, VBO_CONE = 10;
  static std::vector<gl::Vbo> g_vbos;

  // Variaveis locais nao sao compartilhadas pela rede, pois sao computadas a partir de outras.
  struct VariaveisDerivadas {
    VariaveisDerivadas() { }
    // Como esse estado é local e não precisa ser salvo, fica aqui.
    float angulo_disco_selecao_graus = 0.0f;
    // Entidades em voo oscilam sobre a altura do voo. A oscilacao eh baseada no seno deste angulo.
    float angulo_disco_voo_rad = 0.0f;
    // Entidades em queda caem progressivamente ate 90 graus.
    float angulo_disco_queda_graus = 0.0f;
    // Oscilacao da luz.
    float angulo_disco_luz_rad = 0.0f;
    // As texturas da entidade.
    const Texturas* texturas = nullptr;
  };

  // Extracao de VBO por tipo.
  static const gl::Vbo ExtraiVboForma(const ent::EntidadeProto& proto);
  static const gl::Vbo ExtraiVboComposta(const ent::EntidadeProto& proto);

  /** Realiza as chamadas de notificacao para as texturas. */
  void AtualizaTexturas(const EntidadeProto& novo_proto);
  static void AtualizaTexturasProto(const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central);
  /** Para entidades compostas. */
  static void AtualizaTexturasEntidadesCompostasProto(
      const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central);

  /** A oscilacao de voo nao eh um movimento real (nao gera notificacoes). Esta funcao retorna o delta. */
  static float DeltaVoo(const VariaveisDerivadas& vd);

  /** Realiza o desenho do objeto com as decoracoes, como disco de selecao e barra de vida (de acordo com pd). */
  void DesenhaObjetoComDecoracoes(ParametrosDesenho* pd);

  /** Realiza o desenho do objeto. */
  void DesenhaObjeto(ParametrosDesenho* pd, const float* matriz_shear = nullptr);

  /** Desenha as decoracoes do objeto (pontos de vida, disco de selecao. */
  void DesenhaDecoracoes(ParametrosDesenho* pd);

  /** Auxiliar para montar a matriz de desenho do objeto.
  * @param em_voo se verdadeiro, posiciona matriz no ar, caso contrario no solo.
  * @param queda se verdeiro, roda o eixo para desenhar a entidade caida.
  * @param translacao_z se verdadeiro, considera a translacao no eixo z.
  */
  static void MontaMatriz(bool em_voo,
                          bool queda,
                          bool transladar_z,
                          const EntidadeProto& proto,
                          const VariaveisDerivadas& vd,
                          const ParametrosDesenho* pd = nullptr,
                          const float* matriz_shear = nullptr);

  static void DesenhaObjetoProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear = nullptr);
  static void DesenhaObjetoFormaProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear);
  static void DesenhaObjetoEntidadeProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear);
  static void DesenhaObjetoCompostoProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear);

  /** Calcula o multiplicador para um determinado tamanho. */
  static float CalculaMultiplicador(TamanhoEntidade tamanho);

 private:
  EntidadeProto proto_;
  VariaveisDerivadas vd_;

  // A central é usada apenas para enviar notificacoes de textura ja que as entidades nao sao receptoras.
  ntf::CentralNotificacoes* central_;
};

}  // namespace ent

#endif
