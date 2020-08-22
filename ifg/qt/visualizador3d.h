#ifndef IFG_QT_VISUALIZADOR3D_H
#define IFG_QT_VISUALIZADOR3D_H

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QWidget>
#include <list>
#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"

namespace ntf {
class Notificacao;
}  // namespace ntf

namespace ent {
class EntidadeProto;
class Tabelas;
class Tabuleiro;
class TabuleiroProto;
}  // namespace ent

namespace m3d {
class Modelos3d;
}  // namespace m3d

namespace tex {
class Texturas;
}  // namespace tex

namespace ifg {

class TratadorTecladoMouse;

namespace qt {

/** Widget responsavel por desenhar a area 3D. Recebe eventos de redimensionamento,
* mouse e repassa ao contexto 3D.
*/
class Visualizador3d :
  public QWidget, ntf::Receptor {
 public:
  /** constroi a widget do tabuleiro recebendo a widget pai.
  * Nao se torna dono de nada.
  */
  Visualizador3d(
      const ent::Tabelas& tabelas,
      m3d::Modelos3d* m3d,
      tex::Texturas* texturas,
      ifg::TratadorTecladoMouse* teclado_mouse,
      ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai);

  /** destroi as entidades do tabuleiro e libera os recursos. */
  virtual ~Visualizador3d();

  // As seguines funcoes chamam makeCurrent and doneCurrent, com contador de referencia.
  void PegaContexto();
  void LiberaContexto();

  // Interface QOpenGLWidget.
  /** inicializacao dos parametros GL. */
  void iniciaGL();
  /** redimensionamento da janela. */
  void resizeEvent(QResizeEvent *event) override;
  /** funcao de desenho da janela. Aqui comeca o desenho 3d. */
  void renderiza();

  // funcoes sobrecarregadas mouse e teclado.
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  // Interface ntf::Receptor.
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  const ent::Tabelas& tabelas() const { return tabelas_; }

  // Abre o dialogo de entidade para o subtipo TE_ENTIDADE. Retorna null se cancelado.
  // Parametro forma_em_uso indica que o dialogo se refere a forma que esta em uso. Por exemplo, clicar duas vezes no tabuleiro.
  // Ja quando se esta editando uma das formas alternativas, ele eh false (por exemplo, na UI, ao clicar duas vezes na forma alternativa ou criar uma nova).
  // Parametro pai eh usado para indicar o pai do dialogo. Se nullptr, usa this.
  std::unique_ptr<ent::EntidadeProto> AbreDialogoTipoEntidade(
      const ntf::Notificacao& notificacao, bool forma_em_uso = true, QWidget* pai = nullptr);

  struct DadosFramebuffer {
    ~DadosFramebuffer() {
      Apaga();
    }
    void Apaga();
    GLuint framebuffer = 0;
    GLuint textura = 0;
    GLuint renderbuffer = 0;
  };

 private:
  // Dialogos.
  // TODO fazer todos unique ou do tipo mesmo sem ser pointer.
  std::unique_ptr<ent::EntidadeProto> AbreDialogoEntidade(const ntf::Notificacao& notificacao);
  ent::EntidadeProto* AbreDialogoTipoForma(const ntf::Notificacao& notificacao);
  ent::TabuleiroProto* AbreDialogoCenario(const ntf::Notificacao& notificacao);
  ent::OpcoesProto* AbreDialogoOpcoes(const ntf::Notificacao& notificacao);
  ent::Bonus AbreDialogoBonus(const ent::Bonus& bonus);

 private:
  const ent::Tabelas& tabelas_;
  ent::OpcoesProto::TipoIluminacao tipo_iluminacao_;
  m3d::Modelos3d* m3d_ = nullptr;
  tex::Texturas* texturas_ = nullptr;
  ifg::TratadorTecladoMouse* teclado_mouse_;
  ntf::CentralNotificacoes* central_;
  ent::Tabuleiro* tabuleiro_;
  // Para prender mouse no lugar.
  int x_antes_ = 0;
  int y_antes_ = 0;
  float scale_ = 1.0f;
  int contexto_cref_ = 0;
  bool iniciado_ = false;
  QOffscreenSurface surface_;
  QOpenGLContext contexto_;
  //void* contexto_ = nullptr;
  DadosFramebuffer dfb_;
};

}  // namespace qt
}  // namespace ifg

#endif
