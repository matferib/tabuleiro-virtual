#ifndef M3D_M3D_H
#define M3D_M3D_H

#include <memory>
#include "ntf/notificacao.h"

namespace gl {
class VboNaoGravado;
}  // namespace gltab

namespace m3d {

struct Modelo3d {
  std::vector<gl::VboGravado> vbos;
  // Necessario para composicao.
  std::vector<gl::VboNaoGravado> vbos_nao_gravados;
  // Uso interno.
  int contador = 0;
  void Desenha() const;
  bool Valido() const { return !vbos.empty(); }
}; 

class Modelos3d : public ntf::Receptor {
 public:
  Modelos3d(ntf::CentralNotificacoes* central);
  virtual ~Modelos3d();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  const Modelo3d* Modelo(const std::string& id) const;

  // Recarrega os modelos 3d em caso de perda do contexto opengl.
  void Recarrega();

 private:
  // Realiza a carga de um modelo 3d. Os ids nao possuem a extensao .binproto.
  void CarregaModelo3d(const std::string& id_interno);
  void DescarregaModelo3d(const std::string& id_interno);

  struct Interno;
  std::unique_ptr<Interno> interno_;
  ntf::CentralNotificacoes* central_;
};

}  // namespace m3d

#endif
