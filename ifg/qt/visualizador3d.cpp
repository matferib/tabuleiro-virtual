#include <stdexcept>
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMouseEvent>
#include <cmath>
#include <GL/gl.h>
#include "ent/tabuleiro.h"
#include "ifg/qt/util.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/qt/ui/entidade.h"
#include "log/log.h"
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
    QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai),
    central_(central), tabuleiro_(tabuleiro) {
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

namespace {

/** Abre um diálogo editável com as características da entidade. */ 
ent::EntidadeProto* AbreDialogoEntidade(const ntf::Notificacao& notificacao, QWidget* pai) {
  auto* proto = new ent::EntidadeProto(notificacao.entidade());
  ifg::qt::Ui::DialogoEntidade gerador;
  auto* dialogo = new QDialog(pai);
  gerador.setupUi(dialogo);
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(proto->id()));
  gerador.checkbox_cor->setCheckState(proto->has_cor() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_luz->setCheckState(proto->has_luz() ? Qt::Checked : Qt::Unchecked);
  lambda_connect(gerador.botoes, SIGNAL(accepted()), [dialogo, &gerador, &proto] {
    if (gerador.checkbox_cor->checkState() == Qt::Checked) {
      proto->mutable_cor();
    } else {
      proto->clear_cor();
    }
    if (gerador.checkbox_luz->checkState() == Qt::Checked) {
      proto->mutable_luz();
    } else {
      proto->clear_luz();
    }
    dialogo->accept();
  });
  lambda_connect(gerador.botoes, SIGNAL(rejected()), [&notificacao, &proto] {
      delete proto;
      proto = nullptr;
  });
  dialogo->exec();
  delete dialogo;
  return proto;
}

}  // namespace

// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_INICIADO:
      // chama o resize pra iniciar a geometria e desenha a janela
      resizeGL(width(), height());
      glDraw();
      break;
    case ntf::TN_ABRIR_DIALOGO_ENTIDADE: {
      auto* entidade = AbreDialogoEntidade(notificacao, this);
      if (entidade == nullptr) {
        VLOG(1) << "Alterações descartadas";
        break;
      }
      auto* n = new ntf::Notificacao;
      n->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
      n->mutable_entidade()->Swap(entidade);
      central_->AdicionaNotificacao(n);
      break;
    }
    default: ;
  }
  // Sempre redesenha para evitar qualquer problema de atualizacao.
  glDraw();
  return true;
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

void Visualizador3d::mouseDoubleClickEvent(QMouseEvent* event) {
  int altura = height();
  double aspecto = static_cast<double>(width()) / altura;
  tabuleiro_->TrataDuploClick(
    MapeiaBotao(event->button()), 
    event->x(), altura - event->y(), aspecto);
  event->accept();
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















