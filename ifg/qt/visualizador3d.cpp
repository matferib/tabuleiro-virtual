#include <stdexcept>
#include <QMouseEvent>
#include <cmath>
#include <GL/gl.h>
#include "ifg/qt/visualizador3d.h"
#include "ent/parametrosdesenho.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.pb.h"

using namespace ifg::qt;
using namespace std;

namespace {

ent::botao_e mapeiaBotao(Qt::MouseButton botao) {
  switch (botao) {
    case Qt::LeftButton: return ent::BOTAO_ESQUERDO;
    case Qt::RightButton: return ent::BOTAO_DIREITO;
    case Qt::MidButton: return ent::BOTAO_MEIO;
    default: return ent::BOTAO_NENHUM; 
  }
}

}  // namespace

Visualizador3d::Visualizador3d(ntf::CentralNotificacoes* central, QWidget* pai) : 
    QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai), central_(central) {
  central_->RegistraReceptor(this);
}

Visualizador3d::~Visualizador3d() {
}

// reimplementacoes
void Visualizador3d::initializeGL() {
  ent::Tabuleiro::InicializaGL();
}

void Visualizador3d::resizeGL(int width, int height) {
  if (ent::Tabuleiro::HaInstancia()) {
    ent::Tabuleiro::Instancia().TrataRedimensionaJanela(width, height);
  }
}

void Visualizador3d::paintGL() {
  if (ent::Tabuleiro::HaInstancia()) {
    ent::Tabuleiro::Instancia().Desenha();
  }
}

// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_INICIADO:
      // chama o resize pra iniciar a geometria e desenha a janela
      resizeGL(width(), height());
      glDraw();
      return true;
    default:
      return false;
  }
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  if (ent::Tabuleiro::HaInstancia()) {
    int altura = height();
    double aspecto = static_cast<double>(width()) / altura;
    ent::Tabuleiro::Instancia().TrataBotaoPressionado(
      mapeiaBotao(event->button()), 
      event->x(), altura - event->y(), aspecto
    );
    event->accept();
    glDraw();
  }
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  if (ent::Tabuleiro::HaInstancia()) {
    ent::Tabuleiro::Instancia().TrataBotaoLiberado();
    event->accept();
    glDraw();
  }
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  if (ent::Tabuleiro::HaInstancia()) {
    ent::Tabuleiro::Instancia().TrataMovimento(event->x(), (height() - event->y()));
    event->accept();
    glDraw();
  }
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  if (ent::Tabuleiro::HaInstancia()) {
    ent::Tabuleiro::Instancia().TrataRodela(event->delta());
    event->accept();
    glDraw();
  }
}
















