#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <list>

namespace ntf {
	class Notificacao;
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

	// notificacao
	void trataNotificacao(const ntf::Notificacao& notificacao);

	// funcoes sobrecarregadas mouse

	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event); 
	void mouseReleaseEvent(QMouseEvent* event); 
	void wheelEvent(QWheelEvent* event);

private:
	/** trata o clique do mouse (ja com Y convertido para opengl).
	* @return true se algum objeto tratou o evento.
	*/
	void trataClique(int x, int y);

private:
	class Dados;
	Dados* dv3d_;
};

}
}

#endif
