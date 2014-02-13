#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <list>
#include <qgl.h>
#include <list>
#include "ntf/notificacao.h"

namespace ntf {
class Notificacao;
}  // namespace ntf

namespace ent {
class EntidadeProto;
class Tabuleiro;
class TabuleiroProto;
}  // namespace ent

namespace ifg {
namespace qt {

/** Widget responsavel por desenhar a area 3D. Recebe eventos de redimensionamento, 
* mouse e repassa ao contexto 3D.  
*/
class Visualizador3d : public QGLWidget, ntf::Receptor {
 public:
  /** constroi a widget do tabuleiro recebendo a widget pai.
  * Nao se torna dono de nada.
  */
  explicit Visualizador3d(ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai);

  /** destroi as entidades do tabuleiro e libera os recursos. */
  virtual ~Visualizador3d();

  // Interface QGLWidget.
  /** inicializacao dos parametros GL. */
  virtual void initializeGL() override;
  /** redimensionamento da janela. */
  virtual void resizeGL(int width, int height) override;
  /** funcao de desenho da janela. Aqui comeca o desenho 3d. */
  virtual void paintGL() override;

  // funcoes sobrecarregadas mouse e teclado.
  void keyPressEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override; 
  void mouseReleaseEvent(QMouseEvent* event) override; 
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  // Interface ntf::Receptor.
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private:
  // Dialogos.
  ent::EntidadeProto* AbreDialogoEntidade(const ntf::Notificacao& notificacao);
  ent::TabuleiroProto* AbreDialogoTabuleiro(const ntf::Notificacao& notificacao);
  ent::OpcoesProto* AbreDialogoOpcoes(const ntf::Notificacao& notificacao);
  void TrataAcaoTemporizadaTeclado();
  void TrataAcaoTemporizadaMouse();
  // Retorna o estado para ESTADO_OCIOSO.
  void EstadoOcioso();

 private:
  // Assumindo uma maquina de estados bem simples, que vai do ESTADO_OCIOSO pros outros e volta.
  enum estado_e {
    ESTADO_OCIOSO,
    ESTADO_TEMPORIZANDO_TECLADO,
    ESTADO_TEMPORIZANDO_MOUSE,
    ESTADO_CARREGANDO_COM_CLIQUE,
  };
  estado_e estado_;
  // Ultimas coordenadas do mouse (em OpenGL).
  int ultimo_x_;
  int ultimo_y_;
  // Temporizador para teclas em sequencia.
  int temporizador_teclado_;
  int temporizador_mouse_;
  // As teclas pressionadas ate o temporizador estourar ou enter ser pressionado.
  std::vector<int> teclas_;
  ntf::CentralNotificacoes* central_;
  ent::Tabuleiro* tabuleiro_;
};

}  // namespace qt
}  // namespace ifg

#endif
