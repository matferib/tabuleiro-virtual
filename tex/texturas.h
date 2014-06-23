#ifndef TEX_TEXTURAS_H
#define TEX_TEXTURAS_H

#include <unordered_map>
#include "ent/entidade.h"
#include "ntf/notificacao.h"

#define DIR_TEXTURAS "texturas"

namespace tex {

/** Gerencia carregamento de texturas atraves de notificacoes. */
class Texturas : public ent::Texturas, public ntf::Receptor {
 public:
  Texturas(ntf::CentralNotificacoes* central);
  virtual ~Texturas();

  /** Trata as notificacoes do tipo de carregamento descarregamento de textura. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  /** Retorna uma textura. */
  virtual unsigned int Textura(const std::string& id) const override;

  /** Recarrega todas as texturas (em caso de perda do contexto OpenGL, no android por exemplo). */
  void Recarrega();

 private:
  struct InfoTexturaInterna;

  /** Auxiliar para retornar informacao de textura. @return nullptr se nao houver. */
  InfoTexturaInterna* InfoInterna(const std::string& id);
  const InfoTexturaInterna* InfoInterna(const std::string& id) const;

  /** Realiza o carregamento da textura ou referenciamento de uma textura ou referencia . */
  void CarregaTextura(const ent::InfoTextura& info);

  /** Descarrega ou dereferencia uma textura. */
  void DescarregaTextura(const ent::InfoTextura& info);

  /** Gera um identificador unico de textura.
  * @return -1 se alcancar o limite de texturas.
  */
  int GeraIdTextura();

  /** Le e decodifica uma imagem. */
  void LeDecodificaImagem(const std::string& caminho, ent::InfoTextura* info_textura);

  /** Realiza a leitura da imagem de um caminho, preenchendo dados com conteudo do arquivo no caminho.
  * Caso local, a textura sera local ao jogador. Caso contrario, eh uma textura global (da aplicacao).
  */
  virtual void LeImagem(const std::string& arquivo, std::vector<unsigned char>* dados);

 private:
  // Nao possui.
  ntf::CentralNotificacoes* central_;
  std::unordered_map<std::string, InfoTexturaInterna*> texturas_;
};

}  // namespace tex

#endif
