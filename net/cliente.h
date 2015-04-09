#ifndef NET_CLIENTE_H
#define NET_CLIENTE_H

#include <memory>
#include <queue>
#include <string>
#include "net/socket.h"
#include "ntf/notificacao.h"

namespace net {

// Protocolo de conexao no servidor.
class Cliente : public ntf::Receptor, public ntf::ReceptorRemoto {
 public:
  // Nao possui os parametros.
  explicit Cliente(Sincronizador* sincronizador, ntf::CentralNotificacoes* central);

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;
  virtual bool TrataNotificacaoRemota(const ntf::Notificacao& notificacao) override;

 private:
  // Conecta o cliente identificado por id ao servidor localizado em endereco, formato: <host:porta>.
  void Conecta(const std::string& id, const std::string& endereco_str);

  // Descobre o endereco do servidor e conecta.
  void AutoConecta(const std::string& id);

  // Desconecta o cliente. Envia uma notificacao do tipo TN_DESCONECTADO. Se erro nao for vazio,
  // o campo erro da notificacao sera preenchido.
  void Desconecta(const std::string& erro);

  // Recebe dados da conexao continuamente.
  void RecebeDados();

  // Envia dados pela conexao de forma assincrona. Se sem dados for true, ignora dados e envia o primeiro da fifo_envio_.
  void EnviaDados(const std::string& dados, bool sem_dados = false);

  // Retorna se o cliente esta conectado ou nao.
  bool Ligado() const;

  ntf::CentralNotificacoes* central_;
  Sincronizador* sincronizador_;
  std::unique_ptr<Socket> socket_;
  std::string buffer_tamanho_;  // Buffer para receber tamanho dos dados.
  std::string buffer_;  // Buffer de recepcao.
  std::queue<std::string> fifo_envio_;  // FIFO para envio.

  std::unique_ptr<SocketUdp> socket_descobrimento_;
  std::string buffer_descobrimento_;
  std::string endereco_descoberto_;
  int timer_descobrimento_;
};

}  // namespace net

#endif
