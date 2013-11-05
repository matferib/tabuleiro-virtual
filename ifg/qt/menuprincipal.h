#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

/** 
* @file include/ifg/qt/MenuPrincipal.h declaracao da bara de menu da janela principal.
* Compilar com moc para gerar o fonte do qt.
*/

#include <QMenuBar>
#include <QMenu>
#include <vector>
#include "ntf/notificacao.h"

namespace ifg {
namespace qt {

/** os modos que o menu aceita. */  
enum modomenu_e { MM_COMECO, MM_MESTRE, MM_JOGADOR };

/** A barra de menu principal contem os seguintes menus:
* <li> Jogo: Iniciar Mestre, Conectar no mestre, Sair
* <li> Jogadores: Salvar, Restaurar, Adicionar, Remover 
* <li> Sobre: Tabuleiro virtual
*/
class MenuPrincipal : public QMenuBar, ntf::Receptor {
  Q_OBJECT
 public:
  MenuPrincipal(ntf::CentralNotificacoes* central, QWidget* pai);
  ~MenuPrincipal();

  /** Interface ntf::Receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private slots:
  /** slot para tratar a acao QT de um item de menu localmente. */
  void TrataAcaoItem(QAction*);

  /** poe o menu no modo passado como argumento.
  * @TODO o que cada modo habilita.
  */
  void Modo(modomenu_e modo);

 private:
  // menus e acoes dos items
  std::vector<QMenu*> menus_;
  std::vector<std::vector<QAction*>> acoes_;

  ntf::CentralNotificacoes* central_;
};

} // namespace qt
} // namespace ifg

#endif
