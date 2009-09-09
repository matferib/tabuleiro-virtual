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
		MI_ADICIONAR, MI_REMOVER, MI_SALVAR, MI_RESTAURAR,
		MI_TABVIRT
	};
};

MenuPrincipal::MenuPrincipal(QWidget* pai) : QMenuBar(pai){
	// strs de cada menu
	const char* menuStrs[] = { "&Jogo", "J&ogadores", "&Sobre" };
	// strs dos items de cada menu
	const char* menuitemStrs[] = {
		// jogo
		"&Iniciar jogo mestre", "&Conectar no jogo mestre", NULL, "&Sair",
		// jogadores
		"&Adicionar", "&Remover", NULL, "&Salvar", "R&estaurar",
		// sobre
		"&Tabuleiro virtual"
	};
	// inicio das strings para o menu corrente
	unsigned int controleItemInicio = 0;
	for (
		unsigned int controleMenu = ME_JOGO; 
		controleMenu < ME_NUM; 
		++controleMenu
	) {
		QMenu* menu = new QMenu(tr(menuStrs[controleMenu]), this);
		menus.push_back(menu);
		// para cada item no menu, cria os items (acoes)
		acoes.push_back(std::vector<QAction*>());
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
				acoes[controleMenu].push_back(acao);
				menu->addAction(acao);
			}
			else {
				// menuitem NULL, adiciona separador e a acao NULL para manter contador
				acoes[controleMenu].push_back(NULL);
				menu->addSeparator();
			}
		}
		controleItemInicio += numItems[controleMenu];
		// adiciona os menus ao menu principal
		connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(trataAcaoItem(QAction*)));
		addMenu(menu);
	}

	modo(MM_INICIO);
}

MenuPrincipal::~MenuPrincipal(){
}

void MenuPrincipal::modo(modomenu_e modo){
	// jogo e sobre sempre habilitados
	menus[ME_JOGO]->setEnabled(true);
	menus[ME_SOBRE]->setEnabled(true);

	switch (modo){
	case MM_INICIO:
		// habilita todos do jogo
		for (
			std::vector<QAction*>::iterator it = acoes[ME_JOGO].begin();
			it != acoes[ME_JOGO].end();
			++it
		) {
			QAction* acao = *it;
			if (acao != NULL){
				acao->setEnabled(true);
			}
		}
		// desabilita jogadores
		menus[ME_JOGADORES]->setEnabled(false);
		break;
	case MM_MESTRE:
	case MM_JOGADOR:
		// desabilia tudo menos sair no jogo
		for (
			std::vector<QAction*>::iterator it = acoes[ME_JOGO].begin();
			it != acoes[ME_JOGO].end();
			++it
		) {
			QAction* acao = *it;
			if (acao != NULL) {
				acao->setEnabled(false);
			}
		}
		acoes[ME_JOGO][MI_SAIR]->setEnabled(true);

		// Jogadores habilitado so no modo mestre
		menus[ME_JOGADORES]->setEnabled(modo == MM_MESTRE ? true : false);
		break;
	}
}

//#include <iostream>
//using namespace std;
void MenuPrincipal::trataAcaoItem(QAction* acao){

	//cout << (const char*)acao->text().toAscii() << endl;
	if (acao == acoes[ME_JOGO][MI_INICIAR]) {

	}
	// ..
	else if (acao == acoes[ME_JOGO][MI_SAIR]) {
		ntf::Notificacao* nn = new ntf::Notificacao(ntf::TN_SAIR); 
		Principal::instancia().trataNotificacao(nn);
	}
	// .. 
	else if (acao == acoes[ME_SOBRE][MI_TABVIRT]) {
		// mostra a caixa de dialogo da versao
		QDialog* qd = new QDialog(qobject_cast<QWidget*>(parent()));
		qd->setModal(true);
		QLayout* ql = new QBoxLayout(QBoxLayout::TopToBottom, qd);
		ql->addWidget(new QLabel(tr("Tabuleiro virtual versao 0.1")));
		ql->addWidget(new QLabel(tr("Autor: Matheus Ribeiro <mfribeiro@gmail.com>")));
		QPushButton* qpb = new QPushButton(tr("Fechar"));
		connect(qpb, SIGNAL(released()), qd, SLOT(accept()));
		ql->addWidget(qpb);
		qd->setWindowTitle(tr("Sobre o tabuleiro virtual"));
		qd->exec();
		delete qd;
	}
}





