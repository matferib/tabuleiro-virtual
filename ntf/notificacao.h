#ifndef NOTIFICACAO_H
#define NOTIFICACAO_H

#include <vector>
#include "ntf/notificacao.pb.h"

/** @file include/ifg/ntf/Notificacao.h declaracao da interface de notificacao. */

namespace ntf {

Notificacao* NovaNotificacao(ntf::Tipo tipo);

/** Interface para receber notificações. */
class Receptor {
 public:
  /** @return false se não tratar a notificação. */
  virtual bool TrataNotificacao(const Notificacao& notificacao) = 0;
};

/** Identico ao receptor, mas bom para diferenciar. */
class ReceptorRemoto {
 public:
  /** @return false se não tratar a notificação. */
  virtual bool TrataNotificacaoRemota(const Notificacao& notificacao) = 0;
};

class CentralNotificacoes {
 public:
  CentralNotificacoes();
  ~CentralNotificacoes();

  /** Adiciona uma notificacao a central, que sera a dona dela. */
  void AdicionaNotificacao(Notificacao* notificacao);

  /** Adiciona uma notificacao a ser processada apenas pelos receptores remotos. 
  * A central possuirá a notificação. */
  void AdicionaNotificacaoRemota(Notificacao* notificacao);

  /** Registra um receptor com a central, que nao sera dono dele. */
  void RegistraReceptor(Receptor* receptor);

  /** Registra um receptor remoto com a central, que nao sera dono dele. */
  void RegistraReceptorRemoto(ReceptorRemoto* receptor);

  /** Tira um receptor do registro. */
  void DesregistraReceptor(const Receptor* receptor);

  /** Tira um receptor remoto do registro. */
  void DesregistraReceptorRemoto(const ReceptorRemoto* receptor);

  /** Notifica todos os receptores registrados das notificacoes adicionadas. */
  void Notifica();

 private:
  std::vector<Notificacao*> notificacoes_;
  std::vector<Notificacao*> notificacoes_remotas_;
  std::vector<Receptor*> receptores_;
  std::vector<ReceptorRemoto*> receptores_remotos_;
};

} // namespace ntf


#endif
