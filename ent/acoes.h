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
  Acao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro);
  virtual ~Acao() {}

  void Atualiza(int intervalo_ms);

  // Desenha a acao.
  void Desenha(ParametrosDesenho* pd) const;
  void DesenhaTranslucido(ParametrosDesenho* pd) const;

  // Retorna se a acao ja atingiu alvo. A acao pode atingir o alvo antes de ser finalizada.
  // Algumas acoes usam o bit atingiu alvo para isso.
  virtual bool AtingiuAlvo() const { return Finalizada(); }
  // Indica que o alvo ja foi processado pela acao.
  void AlvoProcessado() { atingiu_alvo_ = false; }

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

  const AcaoProto& Proto() const { return acao_proto_; }

 protected:
  virtual void DesenhaSeNaoFinalizada(ParametrosDesenho* pd) const {}
  virtual void DesenhaTranslucidoSeNaoFinalizada(ParametrosDesenho* pd) const {}
  virtual void AtualizaAposAtraso(int intervalo_ms) = 0;

  // Pode ser chamada para atualizar a velocidade da acao de acordo com os parametros de velocidade.
  void AtualizaVelocidade();
  // Pode ser chamado para atualizar o alvo de uma acao. Retorna false quando terminar.
  // Utiliza os parametros dx_, dy_ e dz_ para calcular o deslocamento do alvo num movimento senoide
  // usando disco_alvo_rad_ para manter o estado.
  bool AtualizaAlvo(int intervalo_ms);

 protected:
  AcaoProto acao_proto_;
  Tabuleiro* tabuleiro_ = nullptr;
  float atraso_s_ = 0;
  double delta_tempo_ = 0;
  // Por atualizacao.
  double velocidade_ = 0;
  double aceleracao_ = 0;
  double delta_aceleracao_ = 0;
  // Diferenca entre posicao da acao da origem e do destino.
  double dx_ = 0, dy_ = 0, dz_ = 0;
  // O alvo se move em uma senoide de 0 ate PI (usando cosseno, vai e volta).
  double disco_alvo_rad_ = 0;
  // Para controle de quanto o alvo se moveu.
  double dx_total_ = 0, dy_total_ = 0, dz_total_ = 0;
  bool atingiu_alvo_ = false;
};

// Cria uma nova acao no tabuleiro.
Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro);

}  // namespace ent

#endif  // ENT_ACOES_H
