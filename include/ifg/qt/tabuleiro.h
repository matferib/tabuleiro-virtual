#ifndef IFG_QT_TABULEIRO_H
#define IFG_QT_TABULEIRO_H

#include <qgl.h>

namespace ifg {
namespace qt {

	/** widget responsavel por desenhar o tabuleiro na tela e tratar os eventos. */
	class Tabuleiro : public QGLWidget {
	public:
		/** constroi a widget do tabuleiro recebendo a widget pai. */
		explicit Tabuleiro(QWidget* pai);

		// funcoes sobrecarregadas
		/** inicializacao dos parametros GL. */
		void initializeGL();
		/** redimensionamento da janela. */
		void resizeGL(int width, int height);
		/** funcao de desenho da janela. */
		void paintGL();
	};

}
}

#endif
