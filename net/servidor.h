#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include "ntf/notificacao.h"

namespace net {

class Servidor : public ntf::Receptor, public ntf::ReceptorRemoto {
 public:
  // Nao possui os parametros.
  Servidor(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central);
  virtual ~Servidor();

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;
  virtual bool TrataNotificacaoRemota(const ntf::Notificacao& notificacao) override;

 private:
  struct Cliente {
    Cliente(boost::asio::ip::tcp::socket* socket) : socket(socket), a_receber_(0) {}
    Cliente() : Cliente(nullptr) {}
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
    std::string buffer_notificacao;
    int a_receber_;
  };

  // Liga o servidor e chama EsperaCliente.
  void Liga();
  // Desliga o servidor.
  void Desliga();

  // Retorna se o servidor esta ligado.
  bool Ligado() const;

  // Funcao que espera um cliente de forma assincrona e renova automaticamente sempre que um cliente aparece.
  // Para cada cliente, chama RecebeDadosCliente e Nao bloca.
  void EsperaCliente();
  // Chama a funcao de recepcao de dados de forma assincrona para o cliente.
  void RecebeDadosCliente(Cliente* cliente);
  // Envia dados para o cliente.
  void EnviaDadosCliente(boost::asio::ip::tcp::socket* cliente, const std::string& dados);
  // Desconecta um cliente do servidor, efetivamente destruindo sua estrutura e deletando o ponteiro.
  void DesconectaCliente(Cliente* cliente);

  ntf::CentralNotificacoes* central_;
  boost::asio::io_service* servico_io_;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador_;
  std::unique_ptr<Cliente> cliente_;
  std::vector<Cliente*> clientes_pendentes_;
  std::vector<Cliente*> clientes_;
  std::vector<char> buffer_;
};

}  // namespace net
