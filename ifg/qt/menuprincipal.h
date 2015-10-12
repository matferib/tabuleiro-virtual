#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

/**
* @file include/ifg/qt/MenuPrincipal.h declaracao da bara de menu da janela principal.
* Compilar com moc para gerar o fonte do qt.
*/

#include <QMenuBar>
#include <QMenu>
#include <memory>
#include <vector>
#include <unordered_map>
#include "ntf/notificacao.h"

namespace ent {
class Tabuleiro;
}  // namespace ent

namespace ifg {
namespace qt {

/** A barra de menu principal e as acoes. */
class MenuPrincipal : public QMenuBar, ntf::Receptor {
  Q_OBJECT
 public:
  MenuPrincipal(ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central, QWidget* pai);
  ~MenuPrincipal();

  /** Interface ntf::Receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private:
  /** Menus da barra de menu. */
  enum menu_e { ME_JOGO, ME_TABULEIRO, ME_ENTIDADES, ME_ACOES, ME_DESENHO, ME_SOBRE, ME_NUM };
  /** Os items de cada menu. */
  enum menuitem_e { // items de cada menu
    MI_INICIAR = 0, MI_CONECTAR, MI_SAIR,  // ME_JOGO
    MI_DESFAZER = 0, MI_REFAZER, MI_OPCOES, MI_PROPRIEDADES, MI_REINICIAR, MI_SALVAR, MI_SALVAR_COMO,
        MI_RESTAURAR, MI_RESTAURAR_MANTENDO_ENTIDADES, MI_REMOVER_CENARIO, MI_SALVAR_CAMERA, MI_REINICIAR_CAMERA,  // ME_TABULEIRO
    MI_PROPRIEDADES_ENTIDADE = 0, MI_ADICIONAR, MI_REMOVER, MI_SALVAR_ENTIDADES, MI_RESTAURAR_ENTIDADES, // ME_ENTIDADES
    // ME_ACOES eh vazio.
    MI_CILINDRO = 0, MI_CIRCULO, MI_CONE, MI_CUBO, MI_ESFERA, MI_LIVRE, MI_PIRAMIDE, MI_RETANGULO,
        MI_SELECIONAR_COR, // ME_DESENHO
    MI_TABVIRT = 0,  // ME_SOBRE
  };
  /** os modos (estados) do menu aceita. */
  enum modomenu_e { MM_COMECO, MM_MESTRE, MM_JOGADOR };

 private slots:
  /** slot para tratar a acao QT de um item de menu localmente. */
  void TrataAcaoItem(QAction*);
  /** slot para tratar acoes de modelos de entidade. */
  void TrataAcaoModelo(QAction*);
  /** slot para tratar acoes de acoes de jogadores. */
  void TrataAcaoAcoes(QAction*);

  /** poe o menu no modo passado como argumento.
  * @TODO o que cada modo habilita.
  */
  void Modo(modomenu_e modo);

 private:
  /** Habilita/desabilita menus (e os items). */
  void EstadoMenu(bool estado, menu_e menu);

  /** Habilita/desabilita items de menu. */
  void EstadoItemMenu(bool estado, menu_e menu, const std::vector<menuitem_e>& items);

  // Tabuleiro de jogo, para modelos.
  ent::Tabuleiro* tabuleiro_;
  // menus e acoes dos items
  std::vector<QMenu*> menus_;
  std::vector<std::vector<QAction*>> acoes_;
  // Usar para armazenar as acoes de modelos, que sao desattivas em alguns casos.
  std::vector<QAction*> acoes_modelos_;

  ntf::CentralNotificacoes* central_;
};

} // namespace qt
} // namespace ifg

#endif
