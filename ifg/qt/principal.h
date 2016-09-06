#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QApplication>
#include <QWidget>
#include "ent/tabuleiro.h"
#include "ifg/qt/texturas.h"
#include "ntf/notificacao.h"

/** @file ifg/qt/principal.h declaracao da interface grafica principal baseada em QT. */

namespace ent {
class Texturas;
}  // namespace tex

namespace ifg {

class TratadorTecladoMouse;
 
namespace qt {

// fwd
class Evento;
class MenuPrincipal;
class Visualizador3d;

/** Interface grafica principal. Responsável por manipular a centra de eventos. */
class Principal : public QWidget, ntf::Receptor {
   Q_OBJECT
 public:
  /** Realiza a inicializacao de algumas bibliotecas necessarias para o qt e opengl antes de
  * instanciar o objeto.
  */
  static Principal* Cria(int& argc, char** argv,
                         ent::Tabuleiro* tabuleiro,
                         ent::Texturas* texturas,
                         ifg::TratadorTecladoMouse* teclado_mouse,
                         ntf::CentralNotificacoes* central);
  ~Principal();

  /** executa a classe principal ate que o evento de finalizacao seja executado.
  * Inicia a janela e o menu e aguarda eventos.
  */
  void Executa();

  /** Interface ntf::Receptor. */
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 protected:
  /** Handler de fechamento. */
  void closeEvent(QCloseEvent * event) override;

 private slots:
  /** Trata o evento de temporização. */
  void Temporizador();

 private:
  Principal(ent::Tabuleiro* tabuleiro, ent::Texturas* texturas,
            ifg::TratadorTecladoMouse* teclado_mouse, ntf::CentralNotificacoes* central, QApplication* q_app);

  /** central de notificacoes da interface. */
  ntf::CentralNotificacoes* central_;
  /** a aplicacao QT. */
  QApplication* q_app_;
  /** Temporizador. */
  QTimer* q_timer_;
  /** O mundo virtual. */
  ent::Tabuleiro* tabuleiro_;
  /** barra de menu principal. */
  MenuPrincipal* menu_principal_;
  /** visualizador 3d da aplicacao. */
  Visualizador3d* v3d_;
};

} // namespace qt
} // namespace ifg


#endif
