#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <list>

namespace ent {
	class Entidade;
}

namespace ifg {
namespace qt {

	/** widget responsavel por desenhar a area 3D. 
	* Eh uma ponte entre a interface QT e OpenGL, pois conhece ambos. 
	*/
	class Visualizador3d : public QGLWidget {
	public:
		/** constroi a widget do tabuleiro recebendo a widget pai. */
		explicit Visualizador3d(QWidget* pai);

		/** destroi as entidades do tabuleiro e libera os recursos. */
		~Visualizador3d();

		// funcoes sobrecarregadas OPENGL

		/** inicializacao dos parametros GL. */
		void initializeGL();
		/** redimensionamento da janela. */
		void resizeGL(int width, int height);
		/** funcao de desenho da janela. No visualizador, cada unidade representa 1m. */
		void paintGL();

		// entidades

		/** adiciona uma entidade ao sistema. A responsabilidade de destruicao eh do visualizador.
		* @param id do quadrado onde ela sera adicionada no tabuleiro.
		*/
		void adicionaEntidade(ent::Entidade* entidade, int id);

		// funcoes sobrecarregadas mouse

		void mouseMoveEvent(QMouseEvent* event);
		void mousePressEvent(QMouseEvent* event); 

	private:
		/** desenha os elementos da cena. */
		void desenhaCena();
		/** trata o clique do mouse (ja com Y convertido para opengl).
		* @return true se algum objeto tratou o evento.
		*/
		bool trataClique(int x, int y);

	private:
		class Dados;
		Dados* dv3d_;
	};

}
}

#endif
