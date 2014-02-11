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
  Acao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro) : acao_proto_(acao_proto), tabuleiro_(tabuleiro) {}
  virtual ~Acao() {}

  // Atualiza a acao.
  virtual void Atualiza() = 0;

  // Desenha a acao.
  virtual void Desenha(ParametrosDesenho* pd) = 0;

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

 protected:
  AcaoProto acao_proto_;
  Tabuleiro* tabuleiro_;
};

// Cria uma nova acao no tabuleiro.
Acao* NovaAcao(const AcaoProto& acao_proto, Tabuleiro* tabuleiro); 

}  // namespace ent

#endif  // ENT_ACOES_H
