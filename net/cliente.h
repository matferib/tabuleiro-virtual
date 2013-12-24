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
};

}  // namespace net
