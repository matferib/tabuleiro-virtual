#ifndef ENT_ACOES_H
#define ENT_ACOES_H

#include "ent/acoes.pb.h"
#include "ent/entidade.pb.h"

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf

namespace ent {

class Acao {
 public:
  Acao(const AcaoProto& acao_proto) : acao_proto_(acao_proto) {}
  virtual ~Acao() {}

  // Atualiza a acao.
  virtual void Atualiza() = 0;

  // Desenha a acao.
  virtual void Desenha(ParametrosDesenho* pd) = 0;

  // Indica que a acao ja terminou e pode ser descartada.
  virtual bool Finalizada() const = 0;

 protected:
  AcaoProto acao_proto_;
};

// Adiciona uma acao a ser realizada.
Acao* NovaAcao(const AcaoProto& acao_proto); 

}  // namespace ent

#endif  // ENT_ACOES_H
