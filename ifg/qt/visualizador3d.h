#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <list>
#include "ntf/notificacao.h"

namespace ntf {
  class Notificacao;
}

namespace ifg {
namespace qt {

/** widget responsavel por desenhar a area 3D. 
*/
class Visualizador3d : public QGLWidget, ntf::Receptor {
public:
  /** constroi a widget do tabuleiro recebendo a widget pai. */
  explicit Visualizador3d(ntf::CentralNotificacoes* central, QWidget* pai);

  /** destroi as entidades do tabuleiro e libera os recursos. */
  virtual ~Visualizador3d();

  // Interface QGLWidget.
  /** inicializacao dos parametros GL. */
  virtual void initializeGL() override;
  /** redimensionamento da janela. */
  virtual void resizeGL(int width, int height) override;
  /** funcao de desenho da janela. No visualizador, cada unidade representa 1m. */
  virtual void paintGL() override;

  // funcoes sobrecarregadas mouse
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event); 
  void mouseReleaseEvent(QMouseEvent* event); 
  void wheelEvent(QWheelEvent* event);

  // Interface ntf::Receptor.
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

private:
  /** trata o clique do mouse (ja com Y convertido para opengl).
  * @return true se algum objeto tratou o evento.
  */
  void trataClique(int x, int y);

private:
  ntf::CentralNotificacoes* central_;
};

}  // namespace qt
}  // namespace ifg

#endif
