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
  }
  virtual ~Acao() {}

  // Atualiza a acao.
  virtual void Atualiza() = 0;

  // Desenha a acao.
  void Desenha(ParametrosDesenho* pd);
  void DesenhaTranslucido(ParametrosDesenho* pd);

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

  const AcaoProto& Proto() const { return acao_proto_; }

 protected:
  virtual void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) {}
  virtual void DesenhaTranslucidoSeNaoFinalizada(ParametrosDesenho* pd) {} 

  void AtualizaVelocidade();

 protected:
  AcaoProto acao_proto_;
  Tabuleiro* tabuleiro_;
  double delta_tempo_;
  double velocidade_;
};

// Cria uma nova acao no tabuleiro.
Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro); 

}  // namespace ent

#endif  // ENT_ACOES_H
