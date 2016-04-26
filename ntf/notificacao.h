#ifndef NOTIFICACAO_H
#define NOTIFICACAO_H

#include <memory>
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

/** Apenas classes de rede que enviam mensagens remotas devem implementar isso. */
class EmissorRemoto {
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

  /** Adiciona uma notificacao a ser processada apenas pelos emissores remotos.
  * A central possuirá a notificação. */
  void AdicionaNotificacaoRemota(Notificacao* notificacao);

  /** Registra um receptor com a central, que nao sera dono dele. */
  void RegistraReceptor(Receptor* receptor);

  /** Registra um receptor remoto com a central, que nao sera dono dele. */
  void RegistraEmissorRemoto(EmissorRemoto* receptor);

  /** Tira um receptor do registro.
  * Atencao: ao se desregistrar durante o loop, o objeto continuarar recebendo mensagens ate o termino do loop,
  * pois eh feita uma copia do loop por causa da iteracao.
  */
  void DesregistraReceptor(const Receptor* receptor);

  /** Tira um receptor remoto do registro. Ver warning acima. */
  void DesregistraEmissorRemoto(const EmissorRemoto* receptor);

  /** Notifica todos os receptores registrados das notificacoes adicionadas. */
  void Notifica();

 private:
  std::vector<std::unique_ptr<Notificacao>> notificacoes_;
  std::vector<std::unique_ptr<Notificacao>> notificacoes_remotas_;
  std::vector<Receptor*> receptores_;
  std::vector<EmissorRemoto*> emissores_remotos_;
};

} // namespace ntf


#endif
