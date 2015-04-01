#ifndef NET_CLIENTE_H
#define NET_CLIENTE_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
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

  // Envia dados pela conexao continuamente.
  void EnviaDados(const std::string& dados);

  // Retorna se o cliente esta conectado ou nao.
  bool Ligado() const;

  ntf::CentralNotificacoes* central_;
  Sincronizador* sincronizador_;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  int a_receber_;  // bytes a receber.
  std::vector<char> buffer_;  // Buffer de recepcao.
  std::string buffer_notificacao_;  // Armazena o objeto lido.

  std::unique_ptr<boost::asio::ip::udp::socket> socket_descobrimento_;
  std::vector<char> buffer_descobrimento_;
  boost::asio::ip::udp::endpoint endereco_descoberto_;
  int timer_descobrimento_;
};

}  // namespace net

#endif
