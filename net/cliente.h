#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include "ntf/notificacao.h"

namespace net {

class Cliente : public ntf::Receptor {
 public:
  explicit Cliente(ntf::CentralNotificacoes* central);

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private:
  // 
  void Conecta();
  //
  void Desconecta();

  ntf::CentralNotificacoes* central_;
  boost::asio::io_service servico_io_;
  std::vector<boost::asio::ip::tcp::socket*> clientes_;
  std::vector<char> buffer_;
};

}  // namespace net
