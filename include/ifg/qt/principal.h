#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QApplication>
#include <QWidget> 

/** @file ifg/qt/principal.h declaracao da interface grafica principal baseada em QT. */

// declaracao antecipada das notificacoes.
namespace ntf {
	class Notificacao;
}

namespace ifg {
namespace qt {

	// fwd
	class Evento;
	class MenuPrincipal;
	class Visualizador3d;

	/** Interface grafica principal. Singleton, acessivel de todos os pontos.
	* Todas as outras classes sao instanciadas direta ou indiretamente desta.
	* Responsavel pelo controle de eventos e tambem pela aplicacao QT. 
	*/
	class Principal : public QWidget { 
		Q_OBJECT
	public:
		/** cria e retorna a instancia da classe principal. 
		* Recebe os parametros de linha de comando. 
		* As chamadas seguintes devem chamar instancia().
		*/
		static Principal& criaInstancia(int& argc, char** argv);

		/** @return a instancia unica. */
		static Principal& instancia();
		
		/** destroi a instancia unica da janela principal.
		* Chamado apos a execucao da janela.
		*/
		static void destroiInstancia();

		/** executa a classe principal ate que o evento de finalizacao seja executado.
		* Inicia a janela e o menu e aguarda eventos.
		*/
		void executa();

		/** ponto de entrada para todas as notificacoes geradas no sistema. */
		void trataNotificacao(ntf::Notificacao*);

	private:
		// o construtor e o destrutor nao fazem nada
		Principal(QApplication* qAp);
		~Principal();

	private:
		/** instancia unica da janela. */
		static Principal* inst;
		/** a aplicacao QT. */
		QApplication* qAp;
		/** barra de menu principal. */
		MenuPrincipal* menuPrincipal_;
		/** visualizador 3d da aplicacao. */
		Visualizador3d* v3d_;
	};

} // namespace qt
} // namespace ifg


#endif

