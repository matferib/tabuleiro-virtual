#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include "ntf/notificacao.h"

namespace net {

class Cliente : public ntf::Receptor, public ntf::ReceptorRemoto {
 public:
  // Nao possui os parametros.
  explicit Cliente(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central);

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;
  virtual bool TrataNotificacaoRemota(const ntf::Notificacao& notificacao) override;

 private:
  // Conecta o cliente identificado por id ao servidor localizado em endereco, formato: <host:porta>.
  void Conecta(const std::string& id, const std::string& endereco_str);

  // Desconecta o cliente.
  void Desconecta();

  // Recebe dados da conexao continuamente.
  void RecebeDados();

  // Envia dados pela conexao continuamente.
  void EnviaDados(const std::string& dados);

  // Retorna se o cliente esta conectado ou nao.
  bool Ligado() const;

  ntf::CentralNotificacoes* central_;
  boost::asio::io_service* servico_io_;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  int a_receber_;  // bytes a receber.
  std::vector<char> buffer_;  // Buffer de recepcao.
  std::string buffer_notificacao_;  // Armazena o objeto lido.
};

}  // namespace net
