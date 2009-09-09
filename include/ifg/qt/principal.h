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

	/** declaracao antecipada de eventos. */
	class Evento;

	/** Interface grafica principal. Singleton, acessivel de todos os pontos.
	* Todas as outras classes sao instanciadas direta ou indiretamente desta.
	* Responsavel pelo controle de eventos e tambem pela aplicacao QT. 
	*/
	class Principal : public QWidget { 
		Q_OBJECT
	public:
		/** retorna a instancia da classe principal. 
		* Recebe os parametros de linha de comando apenas se necessario
		* (primeira instancia). As chamadas seguintes podem utilizar os 
		* parametros padroes.
		*/
		static Principal& instancia(int argc = 0, char** argv = NULL);
		
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
	};

} // namespace qt
} // namespace ifg


#endif

