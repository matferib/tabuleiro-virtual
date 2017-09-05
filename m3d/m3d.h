#ifndef M3D_M3D_H
#define M3D_M3D_H

#include <memory>
#include "ntf/notificacao.h"

namespace gl {
class VboNaoGravado;
}  // namespace gltab

namespace m3d {

struct Modelo3d {
  gl::VbosNaoGravados vbos_nao_gravados;
  gl::VbosGravados vbos_gravados;
  // Uso interno.
  int contador = 0;
  bool Valido() const { return !vbos_nao_gravados.Vazio(); }
};

// Para carregar um modelo 3d, mande uma mensagem TN_CARREGAR_MODELO_3D. Para descarregar, TN_DESCARREGAR_MODELO_3D. 
// O modelo deve estar em notificacao.entidade().modelo_3d().id().
class Modelos3d : public ntf::Receptor {
 public:
  Modelos3d(ntf::CentralNotificacoes* central);
  virtual ~Modelos3d();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** O id eh o nome do arquivo sem o binproto. */
  const Modelo3d* Modelo(const std::string& id) const;

  // Recarrega os modelos 3d em caso de perda do contexto opengl.
  void Recarrega();

  /** Retorna a lista de modelos 3d disponiveis. */
  static std::vector<std::string> ModelosDisponiveis(bool global);

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
