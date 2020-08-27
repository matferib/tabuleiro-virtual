#ifndef PRINCIPAL_H
#define PRINCIPAL_H

#include <QApplication>
#include <QCursor>
#include <QWidget>
#include <QMainWindow>
#include "ntf/notificacao.h"

/** @file ifg/qt/principal.h declaracao da interface grafica principal baseada em QT. */

namespace ent {
class Tabelas;
class Tabuleiro;
}  // namespace tex

namespace m3d {
class Modelos3d;
}  // namespace m3d

namespace tex {
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
class Principal : public QMainWindow, ntf::Receptor {
   Q_OBJECT
 public:
  /** Realiza a inicializacao de algumas bibliotecas necessarias para o qt e opengl antes de
  * instanciar o objeto.
  */
  static Principal* Cria(QCoreApplication* qapp,
                         const ent::Tabelas& tabelas,
                         ent::Tabuleiro* tabuleiro,
                         m3d::Modelos3d* m3d,
                         tex::Texturas* texturas,
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
  /** Indica que a visibilidade do dock de log foi alternada. */
  void LogAlternado();

 private:
  Principal(const ent::Tabelas& tabelas, ent::Tabuleiro* tabuleiro, m3d::Modelos3d* m3d, tex::Texturas* texturas,
            ifg::TratadorTecladoMouse* teclado_mouse, ntf::CentralNotificacoes* central, QCoreApplication* q_app);

  /** central de notificacoes da interface. */
  ntf::CentralNotificacoes* central_ = nullptr;
  /** a aplicacao QT. Nao eh dono. */
  QCoreApplication* q_app_ = nullptr;
  /** O mundo virtual. */
  ent::Tabuleiro* tabuleiro_ = nullptr;
  /** visualizador 3d da aplicacao. */
  Visualizador3d* v3d_ = nullptr;
  /** barra de menu principal. */
  MenuPrincipal* menu_principal_ = nullptr;
  /** Dock com o log. */
  QDockWidget* dock_log_ = nullptr;
};

} // namespace qt
} // namespace ifg


#endif
