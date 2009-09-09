#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <memory>
#include "ifg/gl/tabuleiro.h"
#include "ifg/gl/parametrosdesenho.h"

namespace ifg {
namespace gl {
	class Tabuleiro;
}
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


		// funcoes sobrecarregadas mouse

		void mouseMoveEvent(QMouseEvent* event);
		void mousePressEvent(QMouseEvent* event); 

	private:

		/** parametros de desenho da cena. */
		gl::ParametrosDesenho parametrosDesenho_;

		/** entidade principal de desenho. */
		gl::Tabuleiro tabuleiro_;

		// ultimo X do mouse
		int mouseUltimoX_;

		// angulo de rotacao da camera
		double theta_;
	};

}
}

#endif
