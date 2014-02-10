#include <cmath>
#include <stdexcept>
#include <string>

#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMessageBox>
#include <QMouseEvent>
#include <QString>
#if __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "ent/tabuleiro.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/texturas.h"
#include "ifg/qt/util.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/ui/iluminacao.h"
#include "ifg/qt/ui/opcoes.h"
#include "log/log.h"
#include "ntf/notificacao.pb.h"

using namespace ifg::qt;
using namespace std;

namespace {

ent::botao_e MapeiaBotao(const QMouseEvent& evento) {
  switch (evento.button()) {
    case Qt::LeftButton: {
      // Com shift pressionado, trata como botao do meio.
      return evento.modifiers() == Qt::ShiftModifier ?
        ent::BOTAO_MEIO :
        ent::BOTAO_ESQUERDO;
    }
    case Qt::RightButton: return ent::BOTAO_DIREITO;
    case Qt::MidButton: return ent::BOTAO_MEIO;
    default: return ent::BOTAO_NENHUM;
  }
}

int MapeiaTecla(const QKeyEvent& evento) {
  switch (evento.key()) {
    case Qt::Key_Delete:
      return evento.key();
    default:
      return -1;
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
  VLOG(3) << "Retornando estilo: " << estilo.toStdString();
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

// Carrega os dados de uma textura local pro proto 'info_textura'.
void PreencheProtoTextura(const QFileInfo& info_arquivo, ent::InfoTextura* info_textura) {
  QImageReader leitor_imagem(info_arquivo.absoluteFilePath());
  QImage imagem = leitor_imagem.read();
  if (imagem.isNull()) {
    LOG(ERROR) << "Textura inválida: " << info_textura->id();
    return;
  }
  info_textura->set_altura(imagem.height());
  info_textura->set_largura(imagem.width());
  info_textura->set_bits(imagem.constBits(), imagem.byteCount());
  info_textura->set_formato(imagem.format());
}

}  // namespace

Visualizador3d::Visualizador3d(
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QGLWidget(QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer), pai),
       central_(central), tabuleiro_(tabuleiro) {
  central_->RegistraReceptor(this);
  setFocusPolicy(Qt::StrongFocus);
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
      n->mutable_entidade()->Swap(entidade);
      central_->AdicionaNotificacao(n);
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO: {
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
      n->mutable_tabuleiro()->Swap(tabuleiro);
      central_->AdicionaNotificacao(n);
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_OPCOES: {
      if (!notificacao.has_opcoes()) {
        // O tabuleiro criara a mensagem completa.
        return false;
      }
      auto* opcoes = AbreDialogoOpcoes(notificacao);
      if (opcoes == nullptr) {
        VLOG(1) << "Alterações de opcoes descartadas";
        break;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_OPCOES);
      n->mutable_opcoes()->Swap(opcoes);
      central_->AdicionaNotificacao(n);
      break;
    }

    case ntf::TN_ERRO: {
      QMessageBox::warning(this, tr("Erro"), tr(notificacao.erro().c_str()));
    }
    default: ;
  }
  // Sempre redesenha para evitar qualquer problema de atualizacao.
  glDraw();
  return true;
}

// teclado.
void Visualizador3d::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Delete:
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE));
      return;
    default:
      event->ignore();
  }
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  if (event->modifiers() == Qt::ControlModifier) {
    // Control indica sinalizacao.
    tabuleiro_->TrataBotaoSinalizacaoPressionado(
        MapeiaBotao(*event), event->x(), height() - event->y());
  } else {
    tabuleiro_->TrataBotaoPressionado(
        MapeiaBotao(*event), event->x(), height() - event->y());
  }
  glDraw();
  event->accept();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  tabuleiro_->TrataBotaoLiberado(MapeiaBotao(*event));
  event->accept();
  glDraw();
}

void Visualizador3d::mouseDoubleClickEvent(QMouseEvent* event) {
  tabuleiro_->TrataDuploClique(
      MapeiaBotao(*event), event->x(), height() - event->y());
  event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  tabuleiro_->TrataMovimento(MapeiaBotao(*event), event->x(), (height() - event->y()));
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
  const auto& entidade = notificacao.entidade();
  auto* proto_retornado = new ent::EntidadeProto;
  proto_retornado->set_id(entidade.id());
  ifg::qt::Ui::DialogoEntidade gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  // ID.
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(entidade.id()));
  // TODO So habilita para mestre.
  gerador.checkbox_visibilidade->setCheckState(entidade.visivel() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_visibilidade->setEnabled(false);
  }
  // Tamanho.
  gerador.slider_tamanho->setSliderPosition(entidade.tamanho());
  gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
  lambda_connect(gerador.slider_tamanho, SIGNAL(valueChanged(int)), [&gerador] () {
    gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
  });
  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(entidade.cor());
  gerador.botao_cor->setStyleSheet(CorParaEstilo(entidade.cor()));
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [this, dialogo, &gerador, &ent_cor] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    ent_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
  });
  // Cor da luz.
  ent::EntidadeProto luz_cor;
  luz_cor.mutable_cor()->CopyFrom(entidade.luz().cor());
  gerador.botao_luz->setStyleSheet(CorParaEstilo(entidade.luz().cor()));
  if (entidade.has_luz()) {
    gerador.checkbox_luz->setCheckState(Qt::Checked);
  } else {
    gerador.checkbox_luz->setCheckState(Qt::Unchecked);
  }
  lambda_connect(gerador.botao_luz, SIGNAL(clicked()), [this, dialogo, &gerador, &luz_cor] {
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
  gerador.linha_textura->setText(entidade.info_textura().id().c_str());
  lambda_connect(gerador.botao_textura, SIGNAL(clicked()),
      [this, dialogo, &gerador, &luz_cor ] () {
    QString file_str = QFileDialog::getOpenFileName(this, tr("Abrir textura"), tr(DIR_TEXTURAS, FILTRO_IMAGENS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de leitura de textura cancelada.";
      return;
    }
    gerador.linha_textura->setText(file_str);
  });
  // Pontos de vida.
  gerador.spin_pontos_vida->setValue(entidade.pontos_vida());
  // Aura.
  gerador.spin_aura->setValue(entidade.aura());
  // Altura.
  gerador.checkbox_voadora->setCheckState(entidade.pos().z() > 0 ? Qt::Checked : Qt::Unchecked);
  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor] () {
    proto_retornado->set_tamanho(static_cast<ent::TamanhoEntidade>(gerador.slider_tamanho->sliderPosition()));
    proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
    proto_retornado->set_visivel(gerador.checkbox_visibilidade->checkState() == Qt::Checked);
    if (gerador.checkbox_luz->checkState() == Qt::Checked) {
      proto_retornado->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
    } else {
      proto_retornado->clear_luz();
    }
    if (!gerador.linha_textura->text().isEmpty()) {
      if (gerador.linha_textura->text().toStdString() == entidade.info_textura().id()) {
        // Textura igual a anterior.
        VLOG(2) << "Textura igual a anterior.";
        proto_retornado->mutable_info_textura()->set_id(entidade.info_textura().id());
      } else {
        VLOG(2) << "Textura diferente da anterior.";
        QFileInfo info(gerador.linha_textura->text());
        // TODO fazer uma comparacao melhor. Se o diretorio local terminar com o
        // mesmo nome isso vai falhar.
        if (info.dir().dirName() != DIR_TEXTURAS) {
          VLOG(2) << "Textura local, recarregando.";
          QString id = QString::number(notificacao.tabuleiro().id_cliente());
          id.append(":");
          id.append(info.fileName());
          proto_retornado->mutable_info_textura()->set_id(id.toStdString());
          // Usa o id para evitar conflito de textura local com texturas globais.
          // Enviar a textura toda.
          PreencheProtoTextura(info, proto_retornado->mutable_info_textura());
        } else {
          proto_retornado->mutable_info_textura()->set_id(info.fileName().toStdString());
        }
      }
      VLOG(2) << "Id textura: " << proto_retornado->info_textura().id();
    } else {
      proto_retornado->clear_info_textura();
    }
    proto_retornado->set_pontos_vida(gerador.spin_pontos_vida->value());
    int aura = gerador.spin_aura->value();
    if (aura > 0) {
      proto_retornado->set_aura(aura);
    } else {
      proto_retornado->clear_aura();
    }
    if (gerador.checkbox_voadora->checkState() == Qt::Checked) {
      proto_retornado->mutable_pos()->set_z(1.5f);  // TODO
    } else {
      proto_retornado->mutable_pos()->set_z(0.0f);
    }
  });
  // TODO: Ao aplicar as mudanças refresca e nao fecha.

  // Cancelar.
  lambda_connect(dialogo, SIGNAL(rejected()), [&notificacao, &proto_retornado] {
      delete proto_retornado;
      proto_retornado = nullptr;
  });
  dialogo->exec();
  delete dialogo;
  return proto_retornado;
}


/** Abre um diálogo editável com as propriedades do tabuleiro. */
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
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [this, dialogo, &gerador, &cor_proto] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(cor_proto), dialogo, QObject::tr("Cor da luz ambiente"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    cor_proto.CopyFrom(CorParaProto(cor));
  });

  // Posicao na rosa dos ventos. No slider, o zero fica pra baixo enquanto no proto ele fica para direita.
  gerador.dial_posicao->setSliderPosition(tab_proto.luz().posicao_graus() + 90.0f);
  // Inclinacao: o zero do slider fica para baixo enquanto no proto ele fica para direita.
  gerador.dial_inclinacao->setSliderPosition(tab_proto.luz().inclinacao_graus() + 90.0f);
  // Textura do tabuleiro.
  gerador.linha_textura->setText(tab_proto.info_textura().id().c_str());
  lambda_connect(gerador.botao_textura, SIGNAL(clicked()),
      [this, dialogo, &gerador ] () {
    QString file_str = QFileDialog::getOpenFileName(this, tr("Abrir textura"), tr(DIR_TEXTURAS, FILTRO_IMAGENS));
    if (file_str.isEmpty()) {
      VLOG(1) << "Operação de leitura de textura cancelada.";
      return;
    }
    gerador.linha_textura->setText(file_str);
  });

  // Tamanho.
  gerador.linha_largura->setText(QString::number(tab_proto.largura()));
  gerador.linha_altura->setText(QString::number(tab_proto.altura()));

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(gerador.botoes, SIGNAL(accepted()), 
                 [this, tab_proto, dialogo, &gerador, &cor_proto, proto_retornado] {
    proto_retornado->mutable_luz()->set_posicao_graus(gerador.dial_posicao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz()->set_inclinacao_graus(gerador.dial_inclinacao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz()->mutable_cor()->Swap(&cor_proto);
    if (gerador.linha_textura->text().toStdString() == tab_proto.info_textura().id()) {
      // Textura igual a anterior.
      VLOG(2) << "Textura igual a anterior.";
      proto_retornado->mutable_info_textura()->set_id(tab_proto.info_textura().id());
    } else {
      VLOG(2) << "Textura diferente da anterior.";
      QFileInfo info(gerador.linha_textura->text());
      // TODO fazer uma comparacao melhor. Se o diretorio local terminar com o
      // mesmo nome isso vai falhar.
      if (info.dir().dirName() != DIR_TEXTURAS) {
        VLOG(2) << "Textura local, recarregando.";
        QString id = QString::number(tab_proto.id_cliente());
        id.append(":");
        id.append(info.fileName());
        proto_retornado->mutable_info_textura()->set_id(id.toStdString());
        // Usa o id para evitar conflito de textura local com texturas globais.
        // Enviar a textura toda.
        PreencheProtoTextura(info, proto_retornado->mutable_info_textura());
      } else {
        proto_retornado->mutable_info_textura()->set_id(info.fileName().toStdString());
      }
    }
    VLOG(2) << "Id textura: " << proto_retornado->info_textura().id();

    bool ok = true;
    int largura = gerador.linha_largura->text().toInt(&ok);
    if (!ok) {
      return;
    }
    int altura = gerador.linha_altura->text().toInt(&ok);
    if (!ok) {
      return;
    }
    proto_retornado->set_largura(largura);
    proto_retornado->set_altura(altura);
    dialogo->accept();
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

ent::OpcoesProto* Visualizador3d::AbreDialogoOpcoes(
    const ntf::Notificacao& notificacao) {
  auto* proto_retornado = new ent::OpcoesProto;
  ifg::qt::Ui::DialogoOpcoes gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  const auto& opcoes_proto = notificacao.opcoes();

  // fps.
  gerador.checkbox_mostrar_fps->setCheckState(opcoes_proto.mostrar_fps() ? Qt::Checked : Qt::Unchecked);
  // Texturas de frente.
  gerador.checkbox_texturas_sempre_de_frente->setCheckState(opcoes_proto.texturas_sempre_de_frente() ? Qt::Checked : Qt::Unchecked);
  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()), [dialogo, &gerador, proto_retornado] {
    proto_retornado->set_mostrar_fps(gerador.checkbox_mostrar_fps->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_texturas_sempre_de_frente(
        gerador.checkbox_texturas_sempre_de_frente->checkState() == Qt::Checked ? true : false);
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
