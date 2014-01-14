#include <cmath>
#include <stdexcept>
#include <string>

#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMouseEvent>
#include <QString>
#include <GL/gl.h>

#include "ent/tabuleiro.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/util.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/ui/iluminacao.h"
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

// Converte uma cor de float [0..1.0] para inteiro [0.255].
int ConverteCor(float cor_float) {
  int cor = static_cast<int>(255.0f * cor_float);
  if (cor < 0) {
    LOG(WARNING) << "Cor menor que zero!";
    cor = 0;
  } else if (cor > 255) {
    LOG(WARNING) << "Cor maior que 255.";
    cor = 255;
  }
  return cor;
}

// Converte uma cor de inteiro [0.255] para float [0..1.0].
float ConverteCor(int cor_int) {
  return cor_int / 255.0;
}

// Converte do formato ent::Proto para cor do QT.
const QColor ProtoParaCor(const ent::Cor& cor) {
  return QColor(ConverteCor(cor.r()), 
                ConverteCor(cor.g()),
                ConverteCor(cor.b()),
                ConverteCor(cor.a()));
}

// Converte cor do QT para ent::Cor.
const ent::Cor CorParaProto(const QColor& qcor) {
  ent::Cor cor;
  cor.set_r(ConverteCor(qcor.red()));
  cor.set_g(ConverteCor(qcor.green()));
  cor.set_b(ConverteCor(qcor.blue()));
  cor.set_a(ConverteCor(qcor.alpha()));
  return cor;
}

// Retorna uma string de estilo para background-color baseada na cor passada.
const QString CorParaEstilo(const QColor& cor) {
  QString estilo_fmt("background-color: rgb(%1, %2, %3);");
  QString estilo = estilo_fmt.arg(cor.red()).arg(cor.green()).arg(cor.blue());
  VLOG(1) << "Retornando estilo: " << estilo.toStdString();
  return estilo;
}

const QString CorParaEstilo(const ent::Cor& cor) {
  return CorParaEstilo(ProtoParaCor(cor));
}

// Converte um tamanho em string.
const QString TamanhoParaTexto(int tamanho) {
  string str;
  switch (tamanho) {
    case ent::TM_MINUSCULO: return QObject::tr("(minúsculo)");
    case ent::TM_DIMINUTO: return QObject::tr("(diminuto)");
    case ent::TM_MIUDO: return QObject::tr("(miudo)");
    case ent::TM_PEQUENO: return QObject::tr("(pequeno)");
    case ent::TM_MEDIO: return QObject::tr("(médio)");
    case ent::TM_GRANDE: return QObject::tr("(grande)");
    case ent::TM_ENORME: return QObject::tr("(enorme)");
    case ent::TM_IMENSO: return QObject::tr("(imenso)");
    case ent::TM_COLOSSAL: return QObject::tr("(colossal)");
  }
  LOG(ERROR) << "Tamanho inválido: " << tamanho;
  return QObject::tr("desconhecido");
}

}  // namespace

Visualizador3d::Visualizador3d(
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai) 
    :  QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai),
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

// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_INICIADO:
      // chama o resize pra iniciar a geometria e desenha a janela
      resizeGL(width(), height());
      glDraw();
      break;
    case ntf::TN_ABRIR_DIALOGO_ENTIDADE: {
      auto* entidade = AbreDialogoEntidade(notificacao);
      if (entidade == nullptr) {
        VLOG(1) << "Alterações descartadas";
        break;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_ENTIDADE);
      n->set_endereco("local");  // apenas para processar localmente.
      n->mutable_entidade()->Swap(entidade);
      central_->AdicionaNotificacao(n);
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_ILUMINACAO: {
      if (!notificacao.has_tabuleiro()) {
        // O tabuleiro criara a mensagem completa.
        return false;
      }
      auto* tabuleiro = AbreDialogoTabuleiro(notificacao);
      if (tabuleiro == nullptr) {
        VLOG(1) << "Alterações de iluminação descartadas";
        break;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_TABULEIRO);
      n->set_endereco("local");  // apenas para processar localmente.
      n->mutable_tabuleiro()->Swap(tabuleiro);
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
  tabuleiro_->TrataBotaoLiberado(MapeiaBotao(event->button()));
  event->accept();
  glDraw();
}

void Visualizador3d::mouseDoubleClickEvent(QMouseEvent* event) {
  int altura = height();
  double aspecto = static_cast<double>(width()) / altura;
  tabuleiro_->TrataDuploClique(
    MapeiaBotao(event->button()), 
    event->x(), altura - event->y(), aspecto);
  event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  double aspecto = static_cast<double>(width()) / height();
  tabuleiro_->TrataMovimento(MapeiaBotao(event->button()), event->x(), (height() - event->y()), aspecto);
  event->accept();
  glDraw();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  tabuleiro_->TrataRodela(event->delta());
  event->accept();
  glDraw();
}

ent::EntidadeProto* Visualizador3d::AbreDialogoEntidade(
    const ntf::Notificacao& notificacao) {
  auto* proto = new ent::EntidadeProto(notificacao.entidade());
  ifg::qt::Ui::DialogoEntidade gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  // ID.
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(proto->id()));
  // Tamanho.
  gerador.slider_tamanho->setSliderPosition(proto->tamanho());
  gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
  lambda_connect(gerador.slider_tamanho, SIGNAL(valueChanged(int)), [&gerador] () {
    gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
  });
  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(proto->cor());
  gerador.botao_cor->setStyleSheet(CorParaEstilo(proto->cor()));
  if (proto->has_cor()) {
    gerador.checkbox_cor->setCheckState(Qt::Checked);
  } else {
    gerador.checkbox_cor->setCheckState(Qt::Unchecked);
  }
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [dialogo, &gerador, &ent_cor] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.checkbox_cor->setCheckState(Qt::Checked);
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    ent_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
  });
  // Cor da luz.
  ent::EntidadeProto luz_cor;
  luz_cor.mutable_cor()->CopyFrom(proto->luz().cor());
  gerador.botao_luz->setStyleSheet(CorParaEstilo(proto->luz().cor()));
  if (proto->has_luz()) {
    gerador.checkbox_luz->setCheckState(Qt::Checked);
  } else {
    gerador.checkbox_luz->setCheckState(Qt::Unchecked);
  }
  lambda_connect(gerador.botao_luz, SIGNAL(clicked()), [dialogo, &gerador, &luz_cor] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(luz_cor.cor()), dialogo, QObject::tr("Cor da luz"));
    if (!cor.isValid()) {
      return;
    }
    luz_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
    gerador.botao_luz->setStyleSheet(CorParaEstilo(cor));
    gerador.checkbox_luz->setCheckState(Qt::Checked);
  });
  // Textura do objeto.
  gerador.linha_textura->setText(proto->textura().c_str());
  lambda_connect(gerador.botao_textura, SIGNAL(clicked()),
      [this, dialogo, &gerador, &luz_cor ] () {
    QString file_str = QFileDialog::getOpenFileName(this, tr("Abrir textura"), tr(DIR_TEXTURAS, FILTRO_IMAGENS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de leitura de textura cancelada.";
      return;
    }
    QFileInfo info(file_str);
    gerador.linha_textura->setText(info.fileName());
  });
  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, dialogo, &gerador, &proto, &ent_cor, &luz_cor] () {
    proto->set_tamanho(static_cast<ent::TamanhoEntidade>(gerador.slider_tamanho->sliderPosition()));
    if (gerador.checkbox_cor->checkState() == Qt::Checked) {
      proto->mutable_cor()->Swap(ent_cor.mutable_cor());
    } else {
      proto->clear_cor();
    }
    if (gerador.checkbox_luz->checkState() == Qt::Checked) {
      proto->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
    } else {
      proto->clear_luz();
    }
    if (!gerador.linha_textura->text().isEmpty()) {
      proto->set_textura(gerador.linha_textura->text().toStdString());
    } else {
      proto->clear_textura();
    }
  });
  // TODO: Ao aplicar as mudanças refresca e nao fecha.

  // Cancelar.
  lambda_connect(dialogo, SIGNAL(rejected()), [&notificacao, &proto] {
      delete proto;
      proto = nullptr;
  });
  dialogo->exec();
  delete dialogo;
  return proto;
}


/** Abre um diálogo editável com as características de iluminacao e textura do tabuleiro. */ 
ent::TabuleiroProto* Visualizador3d::AbreDialogoTabuleiro(
    const ntf::Notificacao& notificacao) {
  auto* proto_retornado = new ent::TabuleiroProto;
  ifg::qt::Ui::DialogoIluminacao gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  const auto& tab_proto = notificacao.tabuleiro();

  // Cor.
  ent::Cor cor_proto(tab_proto.luz().cor());
  gerador.botao_cor->setStyleSheet(CorParaEstilo(cor_proto));
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [dialogo, &gerador, &cor_proto] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(cor_proto), dialogo, QObject::tr("Cor da luz ambiente"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    cor_proto.CopyFrom(CorParaProto(cor));
  });

  // Posicao na rosa dos ventos. No slider, o zero fica pra baixo enquanto no proto ele fica para direita.
  gerador.dial_posicao->setSliderPosition(tab_proto.luz().posicao() + 90.0f);
  // Inclinacao: o zero do slider fica para baixo enquanto no proto ele fica para direita.
  gerador.dial_inclinacao->setSliderPosition(tab_proto.luz().inclinacao() + 90.0f);
  // Textura do tabuleiro.
  gerador.linha_textura->setText(tab_proto.textura().c_str());
  lambda_connect(gerador.botao_textura, SIGNAL(clicked()),
      [this, dialogo, &gerador ] () {
    QString file_str = QFileDialog::getOpenFileName(this, tr("Abrir textura"), tr(DIR_TEXTURAS, FILTRO_IMAGENS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de leitura de textura cancelada.";
      return;
    }
    QFileInfo info(file_str);
    gerador.linha_textura->setText(info.fileName());
  });

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()), [dialogo, &gerador, &cor_proto, proto_retornado] {
    proto_retornado->mutable_luz()->set_posicao(gerador.dial_posicao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz()->set_inclinacao(gerador.dial_inclinacao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz()->mutable_cor()->Swap(&cor_proto);
    if (!gerador.linha_textura->text().isEmpty()) {
      proto_retornado->set_textura(gerador.linha_textura->text().toStdString());
    } else {
      proto_retornado->clear_textura();
    }
  });
  // Cancelar.
  lambda_connect(dialogo, SIGNAL(rejected()), [&notificacao, &proto_retornado] {
    delete proto_retornado;
    proto_retornado = nullptr;
  });
  dialogo->exec();
  delete dialogo;
  return proto_retornado;
}
