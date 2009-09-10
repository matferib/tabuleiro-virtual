#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <list>
#include "ent/tabuleiro.h"

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
		/** funcao de desenho da janela. */
		void paintGL();

		// entidades

		/** adiciona uma entidade ao sistema. A responsabilidade de destruicao eh do visualizador. */
		void adicionaEntidade(ent::Entidade* entidade);

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

		/** parametros de desenho da cena. */
		ent::ParametrosDesenho parametrosDesenho_;

		/** elementos da cena: terreno. */
		ent::Tabuleiro tabuleiro_;

		/** elementos da cena: ceu. */
		//gl::Ceu ceu_;

		/** elementos da cena: entidades. */
		std::list<ent::Entidade*> entidades_;

		// arrastando
		bool arrastando_;

		// ultimo X do mouse
		int mouseUltimoY_;

		// angulo de rotacao da camera
		double theta_;
	};

}
}

#endif
