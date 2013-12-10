#include <stdexcept>
#include <QMouseEvent>
#include <cmath>
#include <GL/gl.h>
#include "ifg/qt/visualizador3d.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.pb.h"

using namespace ifg::qt;
using namespace std;

namespace {

ent::botao_e MapeiaBotao(Qt::MouseButton botao) {
  switch (botao) {
    case Qt::LeftButton: return ent::BOTAO_ESQUERDO;
    case Qt::RightButton: return ent::BOTAO_DIREITO;
    case Qt::MidButton: return ent::BOTAO_MEIO;
    default: return ent::BOTAO_NENHUM; 
  }
}

}  // namespace

Visualizador3d::Visualizador3d(ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai) : 
    QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai), central_(central), tabuleiro_(tabuleiro) {
  central_->RegistraReceptor(this);
}

Visualizador3d::~Visualizador3d() {
}

// reimplementacoes
void Visualizador3d::initializeGL() {
  ent::Tabuleiro::InicializaGL();
}

void Visualizador3d::resizeGL(int width, int height) {
  tabuleiro_->TrataRedimensionaJanela(width, height);
}

void Visualizador3d::paintGL() {
  tabuleiro_->Desenha();
}

// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_INICIADO:
      // chama o resize pra iniciar a geometria e desenha a janela
      resizeGL(width(), height());
      glDraw();
      return true;
    case ntf::TN_ENTIDADE_ADICIONADA:
    case ntf::TN_ENTIDADE_REMOVIDA:
      glDraw();
      return true;
    default:
      return false;
  }
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  int altura = height();
  double aspecto = static_cast<double>(width()) / altura;
  tabuleiro_->TrataBotaoPressionado(
    MapeiaBotao(event->button()), 
    event->x(), altura - event->y(), aspecto);
  event->accept();
  glDraw();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  tabuleiro_->TrataBotaoLiberado();
  event->accept();
  glDraw();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  tabuleiro_->TrataMovimento(event->x(), (height() - event->y()));
  event->accept();
  glDraw();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  tabuleiro_->TrataRodela(event->delta());
  event->accept();
  glDraw();
}
















