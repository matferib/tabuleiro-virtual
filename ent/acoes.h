#ifndef ENT_ACOES_H
#define ENT_ACOES_H

#include <functional>
#include "ent/acoes.pb.h"
#include "ent/tabelas.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf

namespace tex {
class Texturas;
}  // namespace tex
namespace m3d {
class Modelos3d;
}  // namespace m3d

namespace ent {

class Entidade;
class EntidadeProto;
class Tabuleiro;

class Acao {
 public:
  Acao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central);
  virtual ~Acao();

  void Atualiza(int intervalo_ms);

  // Desenha a acao.
  void Desenha(ParametrosDesenho* pd) const;
  void DesenhaTranslucido(ParametrosDesenho* pd) const;
  // Retorna onde a acao sera desenhada.
  int IdCenario() const;

  enum estado_alvo_e {
    ALVO_NAO_ATINGIDO,          // acao ainda nao afetou o alvo.
    ALVO_A_SER_ATINGIDO,        // acao deve afetar o alvo agora. Deve ser seguida por AlvoProcessado.
    ALVO_ATINGIDO_E_PROCESSADO  // acao ja afetou o alvo.
  };
  // Retorna o estado do alvo da acao. O alvo pode ser afetado em qualquer momento.
  // Caso a acao tenha terminado e o alvo nao tenha sido atingido ainda, marca como ALVO_ATINGIDO
  // para a acao afetar os alvos.
  estado_alvo_e EstadoAlvo() const {
    return (Finalizada() && (estado_alvo_ == ALVO_NAO_ATINGIDO)) ? ALVO_A_SER_ATINGIDO : estado_alvo_;
  }
  // Indica que o alvo ja foi processado pela acao.
  void AlvoProcessado() { estado_alvo_ = ALVO_ATINGIDO_E_PROCESSADO; }

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

  const AcaoProto& Proto() const { return acao_proto_; }

  /** Retorna se o ponto eh afetado pela acao. Apenas acoes de area retornam true.
  * O ultimo parametro indica que o ponto testado e a origem da acao, pois algumas acoes nao afetam o proprio lancador.
  */
  static bool PontoAfetadoPorAcao(const Posicao& pos_ponto, const Posicao& pos_origem, const AcaoProto& proto, bool ponto_eh_origem);
  /** Retorna um ponto auxiliar para ver se tb eh afetado. */
  static Posicao AjustaPonto(
      const Posicao& pos_ponto, float multiplicador_tamanho, const Posicao& pos_origem, const AcaoProto& proto);

 protected:
  void DesenhaGeometriaAcao() const;
  virtual void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const {}
  virtual void DesenhaTranslucidoSeNaoFinalizada(ParametrosDesenho* pd) const { DesenhaSeNaoFinalizada(pd); }
  // Funcao auxiliar para atualizar alvo. Deve ser chamada manualmente por cada subclasse.
  virtual void AtualizaAposAtraso(int intervalo_ms) = 0;

  // Pode ser chamada para atualizar a velocidade da acao de acordo com os parametros de velocidade.
  void AtualizaVelocidade(int intervalo_ms);

  // Pode ser chamado para atualizar o alvo de uma acao. Retorna false quando terminar.
  // Utiliza os parametros dx_, dy_ e dz_ para calcular o deslocamento do alvo num movimento senoide
  // usando disco_alvo_rad_ para manter o estado.
  // Atualiza a direcao de queda do alvo.
  bool AtualizaAlvo(int intervalo_ms);
  // Pode ser chamado para atualizar a origem de uma acao.
  void AtualizaLuzOrigem(int intervalo_ms);

  // Funcoes auxiliares para atualizacao das direcoes. Baseado em dx_ e dy_.
  void AtualizaRotacaoZFonte(Entidade* entidade);
  void AtualizaDirecaoQuedaAlvo(Entidade* entidade);
  // Igual as anteriores, mas usa a posicao do alvo em relacao ao tabuleiro ao inves de dx_ dy_.
  void AtualizaRotacaoZFonteRelativoTabuleiro(Entidade* entidade);
  void AtualizaDirecaoQuedaAlvoRelativoTabuleiro(Entidade* entidade);

  // Retorna a entidade de origem, ou nullptr se nao houver.
  Entidade* EntidadeOrigem();
  // Retorna a entidade de destino se houver apenas 1, caso contrario, nullptr.
  Entidade* EntidadeDestino();

 protected:
  const Tabelas& tabelas_;
  AcaoProto acao_proto_;
  Tabuleiro* tabuleiro_ = nullptr;
  tex::Texturas* texturas_ = nullptr;
  const m3d::Modelos3d* m3d_ = nullptr;
  ntf::CentralNotificacoes* central_ = nullptr;
  float atraso_s_ = 0;
  // Por atualizacao.
  float velocidade_m_ms_ = 0;
  float aceleracao_m_ms_2_ = 0;
  // Vetor de deslocamento do alvo ao ser atingido. Este eh o valor total deslocado em cada eixo.
  float dx_ = 0, dy_ = 0, dz_ = 0;
  // O alvo se move em uma senoide de 0 ate PI (usando cosseno, vai e volta).
  float disco_alvo_rad_ = 0;
  // Para controle de quanto o alvo se moveu.
  float dx_total_ = 0, dy_total_ = 0, dz_total_ = 0;
  estado_alvo_e estado_alvo_ = ALVO_NAO_ATINGIDO;

 private:
  // Desenho comum.
  void DesenhaComum(ParametrosDesenho* pd, std::function<void(ParametrosDesenho*)> f_desenho) const;
};

// Cria uma nova acao no tabuleiro.
Acao* NovaAcao(const Tabelas& tabelas, const AcaoProto& acao_proto, Tabuleiro* tabuleiro, tex::Texturas* texturas, const m3d::Modelos3d* modelos3d, ntf::CentralNotificacoes* central);

// Helpers para pegar da acao. Tenta de por_entidade(0), senao pega da acao mesmo.
const std::string& TextoAcao(const AcaoProto& acao_proto);
int DeltaAcao(const AcaoProto& acao_proto);

// Combina os efeitos adicionais com combina_com.
void CombinaEfeitos(AcaoProto* acao);

// Retorna true se a acao for um efeito de area.
bool EfeitoArea(const AcaoProto& acao_proto);

// Dadas as entidades passadas, seleciona as que estao no mesmo cenario que entidade_origem e pega aquelas sao afetadas pela acao.
const std::vector<unsigned int> EntidadesAfetadasPorAcao(
    const AcaoProto& acao, const Entidade* entidade_origem, const std::vector<const Entidade*>& entidades_cenario);

// Retorna true se a entidade puder ser afetada pelo efeito.
bool EntidadeAfetadaPorEfeito(const Tabelas& tabelas, const AcaoProto::EfeitoAdicional& efeito, const EntidadeProto& alvo);

}  // namespace ent

#endif  // ENT_ACOES_H
