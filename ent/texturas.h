#ifndef ENT_TEXTURAS_H
#define ENT_TEXTURAS_H

#include <unordered_map>
#include "ntf/notificacao.h"

namespace ent {

/** Gerencia carregamento de texturas atraves de notificacoes. */
class Texturas : public ntf::Receptor {
 public:
  Texturas(ntf::CentralNotificacoes* central);
  ~Texturas();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Retorna uma textura. */
  const void* Textura(const std::string& id) const;

 private:
  struct InfoTextura;

  /** Auxiliar para retornar informacao de textura. @return nullptr se nao houver. */
  InfoTextura* Info(const std::string& id);
  const InfoTextura* Info(const std::string& id) const;
  /** Realiza o carregamento da textura ou referenciamento de uma textura. */
  void CarregaTextura(const std::string& id);
  /** Descarrega uma textura ou desreferencia uma textura. */
  void DescarregaTextura(const std::string& id);

  // Nao possui.
  ntf::CentralNotificacoes* central_;
  std::unordered_map<std::string, InfoTextura*> texturas_;
};

}  // namespace ent

#endif
