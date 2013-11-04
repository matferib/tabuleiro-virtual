/** @file ifg/qt/MenuPrincipal.cpp implementacao do menu principal. */

#include <QMenu>
#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "ifg/qt/menuprincipal.h"
#include "ifg/qt/principal.h"
#include "ntf/notificacao.h"

using namespace ifg::qt;

// enumeracao com os menus e seus items
namespace {

enum menu_e { ME_JOGO, ME_JOGADORES, ME_SOBRE, ME_NUM }; // menus da barra

unsigned int numItems[] = { 4, 5, 1 }; // numero de items em cada menu, incluindo sep

enum menuitem_e { // items de cada menu 
	MI_INICIAR, MI_CONECTAR, MI_SEP, MI_SAIR,
	MI_ADICIONAR = 0, MI_REMOVER, MI_SALVAR, MI_RESTAURAR,
	MI_TABVIRT = 0
};

}  // namespace

MenuPrincipal::MenuPrincipal(QWidget* pai) : QMenuBar(pai){
	// strs de cada menu
	const char* menuStrs[] = { "&Jogo", "J&ogadores", "&Sobre" };
	// strs dos items de cada menu
	const char* menuitemStrs[] = {
		// jogo
		("&Iniciar jogo mestre"), ("&Conectar no jogo mestre"), NULL, ("&Sair"),
		// jogadores
		("&Adicionar"), ("&Remover"), NULL, ("&Salvar"), ("R&estaurar"),
		// sobre
		("&Tabuleiro virtual")
	};
	// inicio das strings para o menu corrente
	unsigned int controleItemInicio = 0;
	for (
		unsigned int controleMenu = ME_JOGO; 
		controleMenu < ME_NUM; 
		++controleMenu
	) {
		QMenu* menu = new QMenu(tr(menuStrs[controleMenu]), this);
		menus_.push_back(menu);
		// para cada item no menu, cria os items (acoes)
		acoes_.push_back(std::vector<QAction*>());
		for (
			unsigned int controleItem = 0; 
			controleItem < numItems[controleMenu]; 
			++controleItem
		) {
			const char* menuItemStr = menuitemStrs[controleItemInicio + controleItem];
			if (menuItemStr != NULL) {
				// menuitem nao NULL, adiciona normalmente da lista de menuitems
				// incrementando para o proximo no final
				QAction* acao = new QAction(tr(menuItemStr), menu);
				acoes_[controleMenu].push_back(acao);
				menu->addAction(acao);
			}
			else {
				// menuitem NULL, adiciona separador e a acao NULL para manter contador
				acoes_[controleMenu].push_back(NULL);
				menu->addSeparator();
			}
		}
		controleItemInicio += numItems[controleMenu];
		// adiciona os menus ao menu principal
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(TrataAcaoItem(QAction*)));
		addMenu(menu);
	}

	Modo(MM_COMECO);
}

MenuPrincipal::~MenuPrincipal(){
}

void MenuPrincipal::TrataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.Tipo()) {
		case ntf::TN_INICIAR:
			Modo(MM_MESTRE);
		break;
		default:
			;
	}
}

void MenuPrincipal::Modo(modomenu_e modo){
	// jogo e sobre sempre habilitados
	menus_[ME_JOGO]->setEnabled(true);
	menus_[ME_SOBRE]->setEnabled(true);

	switch (modo){
	case MM_COMECO:
		// habilita todos do jogo
		for (
			std::vector<QAction*>::iterator it = acoes_[ME_JOGO].begin();
			it != acoes_[ME_JOGO].end();
			++it
		) {
			QAction* acao = *it;
			if (acao != NULL){
				acao->setEnabled(true);
			}
		}
		// desabilita jogadores
		menus_[ME_JOGADORES]->setEnabled(false);
		break;
	case MM_MESTRE:
	case MM_JOGADOR:
		// desabilia tudo menos sair no jogo
		for (
			std::vector<QAction*>::iterator it = acoes_[ME_JOGO].begin();
			it != acoes_[ME_JOGO].end();
			++it
		) {
			QAction* acao = *it;
			if (acao != NULL) {
				acao->setEnabled(false);
			}
		}
		acoes_[ME_JOGO][MI_SAIR]->setEnabled(true);

		// Jogadores habilitado so no modo mestre
		menus_[ME_JOGADORES]->setEnabled(modo == MM_MESTRE ? true : false);
		break;
	}
}

//#include <iostream>
//using namespace std;
void MenuPrincipal::TrataAcaoItem(QAction* acao){

	//cout << (const char*)acao->text().toAscii() << endl;
	ntf::Notificacao* nn = NULL;
	if (acao == acoes_[ME_JOGO][MI_INICIAR]) {
		nn = new ntf::Notificacao(ntf::TN_INICIAR);
	}
	// ..
	else if (acao == acoes_[ME_JOGO][MI_SAIR]) {
		nn = new ntf::Notificacao(ntf::TN_SAIR); 
	}
	else if (acao == acoes_[ME_JOGADORES][MI_ADICIONAR]) {
		// @todo abrir dialogo modal pedindo dados do jogador
		nn = new ntf::Notificacao(ntf::TN_ADICIONAR_ENTIDADE); 
	}
	else if (acao == acoes_[ME_JOGADORES][MI_REMOVER]) {
		// @todo abrir dialogo modal pedindo dados do jogador
		nn = new ntf::Notificacao(ntf::TN_REMOVER_ENTIDADE); 
	}
	// .. 
	else if (acao == acoes_[ME_SOBRE][MI_TABVIRT]) {
		// mostra a caixa de dialogo da versao
		QDialog* qd = new QDialog(qobject_cast<QWidget*>(parent()));
		qd->setModal(true);
		QLayout* ql = new QBoxLayout(QBoxLayout::TopToBottom, qd);
		ql->addWidget(new QLabel(tr("Tabuleiro virtual versao 0.1")));
		ql->addWidget(new QLabel(tr("Powered by QT and OpenGL")));
		ql->addWidget(new QLabel(tr("Autor: Matheus Ribeiro <mfribeiro@gmail.com>")));
		QPushButton* qpb = new QPushButton(tr("Fechar"));
		connect(qpb, SIGNAL(released()), qd, SLOT(accept()));
		ql->addWidget(qpb);
		qd->setWindowTitle(tr("Sobre o tabuleiro virtual"));
		qd->exec();
		delete qd;
	}

	if (nn != NULL) {
		Principal::Instancia()->TrataNotificacao(nn);
	}
}





