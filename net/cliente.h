#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include "ntf/notificacao.h"

namespace net {

class Cliente : public ntf::Receptor {
 public:
  explicit Cliente(ntf::CentralNotificacoes* central);

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private:
  // Conecta o cliente ao servidor localizado em endereco, formato: <host:porta>.
  void Conecta(const std::string& endereco_str);

  // Desconecta o cliente.
  void Desconecta();

  // Recebe dados da conexao continuamente.
  void RecebeDados(); 

  // Envia dados pela conexao continuamente.
  void EnviaDados();

  // Retorna se o cliente esta conectado ou nao.
  bool Ligado() const;

  ntf::CentralNotificacoes* central_;
  boost::asio::io_service servico_io_;
  std::unique_ptr<boost::asio::ip::tcp::socket> socket_;
  std::vector<char> buffer_;
};

}  // namespace net
