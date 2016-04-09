#ifndef M3D_M3D_H
#define M3D_M3D_H

#include <memory>
#include "ntf/notificacao.h"

namespace gl {
class VboNaoGravado;
}  // namespace gltab

namespace m3d {

class Modelos3d : public ntf::Receptor {
 public:
  Modelos3d(ntf::CentralNotificacoes* central);
  virtual ~Modelos3d();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  const gl::VboNaoGravado* Modelo(const std::string& id) const;

 private:
  // Recarrega os modelos 3d.
  void Recarrega();

  struct Interno;
  std::unique_ptr<Interno> interno_;
  ntf::CentralNotificacoes* central_;
};

}  // namespace m3d

#endif
