#ifndef NOTIFICACAO_H
#define NOTIFICACAO_H

#include <vector>

/** @file include/ifg/ntf/Notificacao.h declaracao da interface de notificacao. */

namespace ntf {

class Notificacao;

/** Interface para receber notificações. */
class Receptor {
 public:
  /** @return false se não tratar a notificação. */
  virtual bool TrataNotificacao(const Notificacao& notificacao) = 0;
};

class CentralNotificacoes {
 public:
  CentralNotificacoes();
  ~CentralNotificacoes();

  /** Adiciona uma notificacao a central, que sera a dona dela. */
  void AdicionaNotificacao(Notificacao* notificacao);

  /** Registra um receptor com a central, que nao sera dono dele. */
  void RegistraReceptor(Receptor* receptor);

  /** Registra um receptor remoto com a central, que nao sera dono dele. */
  void RegistraReceptorRemoto(Receptor* receptor);

  /** Tira um receptor do registro. */
  void DesregistraReceptor(const Receptor* receptor);

  /** Notifica todos os receptores registrados das notificacoes adicionadas. */
  void Notifica();

 private:
  std::vector<Notificacao*> notificacoes_;
  std::vector<Receptor*> receptores_;
  std::vector<Receptor*> receptores_remotos_;
};
	

} // namespace ntf


#endif
