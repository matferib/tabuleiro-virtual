#ifndef ENT_ACOES_H
#define ENT_ACOES_H

#include "ent/acoes.pb.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf

namespace ent {

class Tabuleiro;

class Acao {
 public:
  Acao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro)
      : acao_proto_(acao_proto), tabuleiro_(tabuleiro) {
    delta_tempo_ = 0;
    velocidade_ = acao_proto.velocidade().inicial();
    dx_ = dy_ = dz_ = 0;
    dx_total_ = dy_total_ = dz_total_ = 0;
    disco_alvo_rad_ = 0;
    contador_atraso_ = acao_proto_.atraso();
  }
  virtual ~Acao() {}

  void Atualiza() {
    if (contador_atraso_ > 0) {
      --contador_atraso_;
      return;
    }
    AtualizaAposAtraso();
  }

  // Desenha a acao.
  void Desenha(ParametrosDesenho* pd);
  void DesenhaTranslucido(ParametrosDesenho* pd);

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

  const AcaoProto& Proto() const { return acao_proto_; }

 protected:
  virtual void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) {}
  virtual void DesenhaTranslucidoSeNaoFinalizada(ParametrosDesenho* pd) {} 
  virtual void AtualizaAposAtraso() = 0;

  // Pode ser chamada para atualizar a velocidade da acao de acordo com os parametros de velocidade.
  void AtualizaVelocidade();
  // Pode ser chamado para atualizar o alvo de uma acao. Retorna false quando terminar.
  bool AtualizaAlvo();

 protected:
  AcaoProto acao_proto_;
  Tabuleiro* tabuleiro_;
  int contador_atraso_;
  double delta_tempo_;
  double velocidade_;
  // Diferenca entre posicao da acao da origem e do destino.
  double dx_, dy_, dz_;
  // O alvo se move em uma senoide de 0 ate PI (usando cosseno, vai e volta).
  double disco_alvo_rad_;
  // Para controle de quanto o alvo se moveu.
  double dx_total_, dy_total_, dz_total_;
};

// Cria uma nova acao no tabuleiro.
Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro); 

}  // namespace ent

#endif  // ENT_ACOES_H
