#ifndef ENT_TEXTURAS_H
#define ENT_TEXTURAS_H

#include <unordered_map>
#include "ntf/notificacao.h"

namespace ent {

/** Dados de renderizacao para texturas. O formato é sempre RGBA. */ 
struct InfoTextura {
  int altura;
  int largura;
  void* dados;
};

/** Gerencia carregamento de texturas atraves de notificacoes. */
class Texturas : public ntf::Receptor {
 public:
  Texturas(ntf::CentralNotificacoes* central);
  ~Texturas();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Retorna uma textura. */
  const InfoTextura* Textura(const std::string& id) const;

 private:
  struct InfoTexturaInterna;

  /** Auxiliar para retornar informacao de textura. @return nullptr se nao houver. */
  InfoTexturaInterna* InfoInterna(const std::string& id);
  const InfoTexturaInterna* InfoInterna(const std::string& id) const;
  /** Realiza o carregamento da textura ou referenciamento de uma textura. */
  void CarregaTextura(const std::string& id);
  /** Descarrega uma textura ou desreferencia uma textura. */
  void DescarregaTextura(const std::string& id);

  // Nao possui.
  ntf::CentralNotificacoes* central_;
  std::unordered_map<std::string, InfoTexturaInterna*> texturas_;
};

}  // namespace ent

#endif
