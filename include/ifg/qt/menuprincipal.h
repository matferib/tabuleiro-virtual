#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

/** 
* @file include/ifg/qt/MenuPrincipal.h declaracao da bara de menu da janela principal.
* Compilar com moc para gerar o fonte do qt.
*/

#include <QMenuBar>
#include <QMenu>
#include <vector>

namespace ifg {
namespace qt {

	/** os modos que o menu aceita. */	
	enum modomenu_e { MM_INICIO, MM_MESTRE, MM_JOGADOR };

	/** A barra de menu principal contem os seguintes menus:
	* <li> Jogo: Iniciar Mestre, Conectar no mestre, Sair
	* <li> Jogadores: Salvar, Restaurar, Adicionar, Remover 
	* <li> Sobre: Tabuleiro virtual
	*/
	class MenuPrincipal : public QMenuBar {
		Q_OBJECT
	public:
		MenuPrincipal(QWidget* pai);
		~MenuPrincipal();

		/** poe o menu no modo passado como argumento.
		* @TODO o que cada modo habilita.
		*/
		void modo(modomenu_e modo);

	private slots:
		/** slot para tratar a acao QT de um item de menu localmente. */
		void trataAcaoItem(QAction*);

	private:
		// menus e acoes dos items
		std::vector<QMenu*> menus;
		std::vector< std::vector<QAction*> > acoes;
	};

} // namespace qt
} // namespace ifg

#endif
