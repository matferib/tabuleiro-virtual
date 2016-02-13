#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

#include <QBoxLayout>
#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>

#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/texturas.h"
#include "ifg/qt/util.h"
#include "ifg/qt/ui/forma.h"
#include "ifg/qt/ui/cenario.h"
#include "ifg/qt/ui/opcoes.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/tecladomouse.h"
#include "log/log.h"
#include "net/util.h"
#include "ntf/notificacao.pb.h"
#include "tex/texturas.h"

using namespace std;

namespace ifg {
namespace qt {
namespace {

class DesativadorWatchdogEscopo {
 public:
  DesativadorWatchdogEscopo(ent::Tabuleiro* tabuleiro) : tabuleiro_(tabuleiro) {
    tabuleiro_->DesativaWatchdog();
  }
  ~DesativadorWatchdogEscopo() {
    tabuleiro_->ReativaWatchdog();
  }

 private:
  ent::Tabuleiro* tabuleiro_;
};

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

// Carrega os dados de uma textura pro proto 'info_textura' e tambem preenche plargura e paltura.
bool PreencheInfoTextura(
    const std::string& nome, arq::tipo_e tipo, ent::InfoTextura* info_textura,
    unsigned int* plargura = nullptr, unsigned int* paltura = nullptr) {
  unsigned int largura = 0, altura = 0;
  if (plargura == nullptr) {
    plargura = &largura;
  }
  if (paltura == nullptr) {
    paltura = &altura;
  }
  try {
    tex::Texturas::LeDecodificaImagem(tipo, nome, info_textura, plargura, paltura);
    return true;
  } catch (...) {
    LOG(ERROR) << "Textura inválida: " << info_textura->id();
    return false;
  }
}

// Mapeia a tecla do QT para do TratadorTecladoMouse.
teclas_e TeclaQtParaTratadorTecladoMouse(int tecla_qt) {
  return static_cast<teclas_e>(tecla_qt);
}

modificadores_e ModificadoresQtParaTratadorTecladoMouse(int modificadores_qt) {
  return static_cast<modificadores_e>(modificadores_qt);
}

botoesmouse_e BotaoMouseQtParaTratadorTecladoMouse(int botao_qt) {
  return static_cast<botoesmouse_e>(botao_qt);
}

// Formato da janela.
QGLFormat Formato() {
  return QGLFormat(QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer | QGL::AlphaChannel | QGL::SampleBuffers);
}

void AdicionaSeparador(const QString& rotulo, QComboBox* combo_textura) {
  QStandardItem* item = new QStandardItem(rotulo);
  item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
  QFont font = item->font();
  font.setBold(true);
  item->setFont(font);
  ((QStandardItemModel*)combo_textura->model())->appendRow(item);
}

// Preenche combo de textura. Cada item tera o id e o tipo de textura. Para texturas locais,
// o nome sera prefixado por id.
void PreencheComboTextura(const std::string& id_corrente, int id_cliente, QComboBox* combo_textura) {
  combo_textura->addItem(combo_textura->tr("Nenhuma"), QVariant(-1));
  auto NaoEhSkybox = [] (const std::string& nome_arquivo) {
    bool ret = nome_arquivo.find("skybox_") != 0;
    return ret;
  };
  auto FiltraOrdena = [NaoEhSkybox] (std::vector<std::string> texturas) -> std::vector<std::string> {
    texturas.erase(std::remove_if(texturas.begin(), texturas.end(), NaoEhSkybox), texturas.end());
    std::sort(texturas.begin(), texturas.end());
    return texturas;
  };
  std::vector<std::string> texturas = std::move(FiltraOrdena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA)));
  std::vector<std::string> texturas_baixadas = std::move(FiltraOrdena((arq::ConteudoDiretorio(arq::TIPO_TEXTURA_BAIXADA))));
  std::vector<std::string> texturas_locais = std::move(FiltraOrdena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA_LOCAL)));

  AdicionaSeparador(combo_textura->tr("Globais"), combo_textura);
  for (const std::string& textura : texturas) {
    combo_textura->addItem(QString(textura.c_str()), QVariant(arq::TIPO_TEXTURA));
  }
  AdicionaSeparador(combo_textura->tr("Baixadas"), combo_textura);
  for (const std::string& textura : texturas_baixadas) {
    combo_textura->addItem(textura.c_str(), QVariant(arq::TIPO_TEXTURA_BAIXADA));
  }
  AdicionaSeparador(combo_textura->tr("Locais"), combo_textura);
  QString prefixo = QString::number(id_cliente).append(":");
  for (const std::string& textura : texturas_locais) {
    combo_textura->addItem(QString(prefixo).append(textura.c_str()), QVariant(arq::TIPO_TEXTURA_LOCAL));
  }
  if (id_corrente.empty()) {
    combo_textura->setCurrentIndex(0);
  } else {
    int index = combo_textura->findText(QString(id_corrente.c_str()));
    if (index == -1) {
      index = 0;
    }
    combo_textura->setCurrentIndex(index);
  }
}

// Preenche proto_retornado usando entidade e o combo como base.
void PreencheTexturaProtoRetornado(const ent::InfoTextura& info_antes, const QComboBox* combo_textura,
                                   ent::InfoTextura* info_textura) {
  if (combo_textura->currentIndex() != 0) {
    if (combo_textura->currentText().toStdString() == info_antes.id()) {
      // Textura igual a anterior.
      VLOG(2) << "Textura igual a anterior.";
      info_textura->set_id(info_antes.id());
    } else {
      VLOG(2) << "Textura diferente da anterior.";
      QString nome(combo_textura->currentText());
      QVariant dados(combo_textura->itemData(combo_textura->currentIndex()));
      if (dados.toInt() == arq::TIPO_TEXTURA_LOCAL) {
        VLOG(2) << "Textura local, recarregando.";
        info_textura->set_id(nome.toStdString());
        PreencheInfoTextura(nome.split(":")[1].toStdString(),
            arq::TIPO_TEXTURA_LOCAL, info_textura);
      } else {
        info_textura->set_id(nome.toStdString());
      }
    }
    VLOG(2) << "Id textura: " << info_textura->id();
  } else {
    info_textura->Clear();
  }
}

}  // namespace

Visualizador3d::Visualizador3d(
    int* argcp, char** argv, TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QGLWidget(Formato(), pai),
       argcp_(argcp), argv_(argv),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro) {
  central_->RegistraReceptor(this);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
}

Visualizador3d::~Visualizador3d() {
  gl::FinalizaGl();
}

// reimplementacoes
void Visualizador3d::initializeGL() {
  static bool once = false;
  try {
    if (!once) {
      once = true;
      gl::IniciaGl(argcp_, argv_);
    }
    tabuleiro_->IniciaGL();
  } catch (const std::logic_error& erro) {
    // Este log de erro eh pro caso da aplicacao morrer e nao conseguir mostrar a mensagem.
    LOG(ERROR) << "Erro na inicializacao GL " << erro.what();
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro(erro.what());
    central_->AdicionaNotificacao(n);
  }
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
      break;
#if 0
    case ntf::TN_ABRIR_DIALOGO_ABRIR_TABULEIRO: {
      QString file_str = QFileDialog::getOpenFileName(qobject_cast<QWidget*>(parent()),
          tr("Abrir tabuleiro"),
          arq::Diretorio(arq::TIPO_TABULEIRO).c_str());
      if (file_str.isEmpty()) {
        VLOG(1) << "Operação de restaurar cancelada.";
        break;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
      if (notificacao.tabuleiro().manter_entidades()) {
        n->mutable_tabuleiro()->set_manter_entidades(true);
      }
      n->set_endereco(file_str.toStdString());
      central_->AdicionaNotificacao(n);
      break;
    }
#endif
    /*
    case ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO: {
      // Abre dialogo de arquivo.
      QString file_str = QFileDialog::getSaveFileName(
          qobject_cast<QWidget*>(parent()),
          tr("Salvar tabuleiro"),
          tr(arq::Diretorio(arq::TIPO_TABULEIRO).c_str()));
      if (file_str.isEmpty()) {
        VLOG(1) << "Operação de salvar cancelada.";
        return false;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
      n->set_endereco(file_str.toStdString());
      central_->AdicionaNotificacao(n);
      break;
    }
    */
    case ntf::TN_ABRIR_DIALOGO_ENTIDADE: {
      if (!notificacao.has_entidade()) {
        return false;
      }
      DesativadorWatchdogEscopo dw(tabuleiro_);
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
      DesativadorWatchdogEscopo dw(tabuleiro_);
      auto* tabuleiro = AbreDialogoCenario(notificacao);
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
      DesativadorWatchdogEscopo dw(tabuleiro_);
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
    /*
    case ntf::TN_INFO: {
      DesativadorWatchdogEscopo dw(tabuleiro_);
      QMessageBox::information(this, tr("Informação"), tr(notificacao.erro().c_str()));
      break;
    }
    case ntf::TN_ERRO: {
      DesativadorWatchdogEscopo dw(tabuleiro_);
      QMessageBox::warning(this, tr("Erro"), tr(notificacao.erro().c_str()));
      break;
    }*/
    case ntf::TN_TEMPORIZADOR:
      glDraw();
      break;
    default: ;
  }
  return true;
}

// teclado.
void Visualizador3d::keyPressEvent(QKeyEvent* event) {
  teclado_mouse_->TrataTeclaPressionada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
}

void Visualizador3d::keyReleaseEvent(QKeyEvent* event) {
  teclado_mouse_->TrataTeclaLiberada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  teclado_mouse_->TrataBotaoMousePressionado(
       BotaoMouseQtParaTratadorTecladoMouse(event->button()),
       ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
       event->x(),
       height() - event->y());
  event->accept();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  teclado_mouse_->TrataBotaoMouseLiberado();
  event->accept();
}

void Visualizador3d::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->modifiers() != 0) {
    // Com modificadores chama o mouse press duas vezes.
    auto* event2 = new QMouseEvent(*event);
    mousePressEvent(event);
    mousePressEvent(event2);
    return;
  }
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      event->x(), height() - event->y());
  event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  teclado_mouse_->TrataMovimentoMouse(event->x(), height() - event->y());
  event->accept();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  teclado_mouse_->TrataRodela(event->delta());
  event->accept();
}

ent::EntidadeProto* Visualizador3d::AbreDialogoTipoForma(
    const ntf::Notificacao& notificacao) {
  const auto& entidade = notificacao.entidade();
  auto* proto_retornado = new ent::EntidadeProto(entidade);
  ifg::qt::Ui::DialogoForma gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  // ID.
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(entidade.id()));
  // Pontos de vida.
  gerador.spin_max_pontos_vida->setValue(entidade.max_pontos_vida());
  gerador.spin_pontos_vida->setValue(entidade.pontos_vida());
  // Rotulos especiais.
  std::string rotulos_especiais;
  for (const std::string& rotulo_especial : entidade.rotulo_especial()) {
    rotulos_especiais += rotulo_especial + "\n";
  }
  gerador.lista_rotulos->appendPlainText(rotulos_especiais.c_str());
  // Visibilidade.
  gerador.checkbox_visibilidade->setCheckState(entidade.visivel() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_visibilidade->setEnabled(false);
  }
  // Fixa.
  gerador.checkbox_fixa->setCheckState(entidade.fixa() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_selecionavel->setEnabled(false);
  }
  lambda_connect(gerador.checkbox_fixa, SIGNAL(clicked()),
      [this, &gerador, &notificacao] () {
    gerador.checkbox_selecionavel->setEnabled(notificacao.modo_mestre() &&
                                              (gerador.checkbox_fixa->checkState() != Qt::Checked));
  });
  // Selecionavel para jogadores.
  gerador.checkbox_selecionavel->setCheckState(entidade.selecionavel_para_jogador() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre() || entidade.fixa()) {
    gerador.checkbox_selecionavel->setEnabled(false);
  }
  // Textura do objeto.
  PreencheComboTextura(entidade.info_textura().id(), notificacao.tabuleiro().id_cliente(), gerador.combo_textura);
  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(entidade.cor());
  gerador.botao_cor->setStyleSheet(CorParaEstilo(entidade.cor()));
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [this, dialogo, &gerador, &ent_cor] {
    QColor cor = QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    ent_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
  });
  gerador.slider_alfa->setValue(static_cast<int>(ent_cor.cor().a() * 100.0f));
  // Luz.
  ent::EntidadeProto luz_cor;
  if (entidade.has_luz()) {
    luz_cor.mutable_cor()->CopyFrom(entidade.luz().cor());
    gerador.botao_luz->setStyleSheet(CorParaEstilo(entidade.luz().cor()));
  } else {
    ent::Cor branco;
    branco.set_r(1.0f);
    branco.set_g(1.0f);
    branco.set_b(1.0f);
    luz_cor.mutable_cor()->CopyFrom(branco);
    gerador.botao_luz->setStyleSheet(CorParaEstilo(branco));
  }
  gerador.checkbox_luz->setCheckState(entidade.has_luz() ? Qt::Checked : Qt::Unchecked);
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

  // Rotacao em Z.
  gerador.dial_rotacao->setSliderPosition(entidade.rotacao_z_graus());
  gerador.spin_rotacao->setValue(gerador.dial_rotacao->value());
  lambda_connect(gerador.dial_rotacao, SIGNAL(valueChanged(int)), [gerador] {
    gerador.spin_rotacao->setValue(gerador.dial_rotacao->value());
  });
  lambda_connect(gerador.spin_rotacao, SIGNAL(valueChanged(int)), [gerador] {
    gerador.dial_rotacao->setValue(gerador.spin_rotacao->value());
  });

  // Translacao em Z.
  gerador.spin_translacao->setValue(entidade.pos().z());
  // Rotacao em Y.
  gerador.dial_rotacao_y->setSliderPosition(-entidade.rotacao_y_graus() - 180.0f);
  gerador.spin_rotacao_y->setValue(entidade.rotacao_y_graus());
  lambda_connect(gerador.dial_rotacao_y, SIGNAL(valueChanged(int)), [gerador] {
    gerador.spin_rotacao_y->setValue(180 - gerador.dial_rotacao_y->value());
  });
  lambda_connect(gerador.spin_rotacao_y, SIGNAL(valueChanged(int)), [gerador] {
    gerador.dial_rotacao_y->setValue(-gerador.spin_rotacao_y->value() - 180);
  });
  // Rotacao em X.
  gerador.dial_rotacao_x->setSliderPosition(-entidade.rotacao_x_graus() - 180.0f);
  gerador.spin_rotacao_x->setValue(entidade.rotacao_x_graus());
  lambda_connect(gerador.dial_rotacao_x, SIGNAL(valueChanged(int)), [gerador] {
    gerador.spin_rotacao_x->setValue(180 - gerador.dial_rotacao_x->value());
  });
  lambda_connect(gerador.spin_rotacao_x, SIGNAL(valueChanged(int)), [gerador] {
    gerador.dial_rotacao_x->setValue(-gerador.spin_rotacao_x->value() - 180);
  });

  // Escalas.
  gerador.spin_escala_x->setValue(entidade.escala().x());
  gerador.spin_escala_y->setValue(entidade.escala().y());
  gerador.spin_escala_z->setValue(entidade.escala().z());

  // Transicao de cenario.
  lambda_connect(gerador.checkbox_transicao_cenario, SIGNAL(stateChanged(int)), [gerador] {
    bool habilitado = gerador.checkbox_transicao_cenario->checkState() == Qt::Checked;
    gerador.linha_transicao_cenario->setEnabled(habilitado);
    gerador.checkbox_transicao_posicao->setEnabled(habilitado);
    gerador.spin_trans_x->setEnabled(habilitado);
    gerador.spin_trans_y->setEnabled(habilitado);
    gerador.spin_trans_z->setEnabled(habilitado);
  });
  if (!entidade.transicao_cenario().has_id_cenario()) {
    gerador.checkbox_transicao_cenario->setCheckState(Qt::Unchecked);
    gerador.linha_transicao_cenario->setEnabled(false);
    gerador.checkbox_transicao_posicao->setEnabled(false);
    gerador.spin_trans_x->setEnabled(false);
    gerador.spin_trans_y->setEnabled(false);
    gerador.spin_trans_z->setEnabled(false);
  } else {
    gerador.checkbox_transicao_cenario->setCheckState(Qt::Checked);
    gerador.linha_transicao_cenario->setText(QString::number(entidade.transicao_cenario().id_cenario()));
    if (entidade.transicao_cenario().has_x()) {
      gerador.checkbox_transicao_posicao->setCheckState(Qt::Checked);
      gerador.spin_trans_x->setValue(entidade.transicao_cenario().x());
      gerador.spin_trans_y->setValue(entidade.transicao_cenario().y());
      gerador.spin_trans_z->setValue(entidade.transicao_cenario().z());
    } else {
      gerador.checkbox_transicao_posicao->setCheckState(Qt::Unchecked);
    }
  }

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor ] () {
    if (gerador.spin_max_pontos_vida->value() > 0) {
      proto_retornado->set_max_pontos_vida(gerador.spin_max_pontos_vida->value());
      proto_retornado->set_pontos_vida(gerador.spin_pontos_vida->value());
    } else {
      proto_retornado->clear_max_pontos_vida();
      proto_retornado->clear_pontos_vida();
    }
    QStringList lista_rotulos = gerador.lista_rotulos->toPlainText().split("\n", QString::SkipEmptyParts);
    proto_retornado->clear_rotulo_especial();
    for (const auto& rotulo : lista_rotulos) {
      proto_retornado->add_rotulo_especial(rotulo.toStdString());
    }
    if (gerador.checkbox_luz->checkState() == Qt::Checked) {
      proto_retornado->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
    } else {
      proto_retornado->clear_luz();
    }
    proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
    proto_retornado->mutable_cor()->set_a(gerador.slider_alfa->value() / 100.0f);
    proto_retornado->set_visivel(gerador.checkbox_visibilidade->checkState() == Qt::Checked);
    proto_retornado->set_selecionavel_para_jogador(gerador.checkbox_selecionavel->checkState() == Qt::Checked);
    bool fixa = gerador.checkbox_fixa->checkState() == Qt::Checked;
    if (fixa) {
      // Override.
      proto_retornado->set_selecionavel_para_jogador(false);
    }
    proto_retornado->set_fixa(fixa);
    proto_retornado->set_rotacao_z_graus(gerador.dial_rotacao->sliderPosition());
    proto_retornado->set_rotacao_y_graus(-gerador.dial_rotacao_y->sliderPosition() + 180.0f);
    proto_retornado->set_rotacao_x_graus(-gerador.dial_rotacao_x->sliderPosition() + 180.0f);
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao->value());
    proto_retornado->clear_translacao_z_deprecated();
    proto_retornado->mutable_escala()->set_x(gerador.spin_escala_x->value());
    proto_retornado->mutable_escala()->set_y(gerador.spin_escala_y->value());
    proto_retornado->mutable_escala()->set_z(gerador.spin_escala_z->value());
    if (gerador.checkbox_transicao_cenario->checkState() == Qt::Checked) {
      bool ok = false;
      int val = gerador.linha_transicao_cenario->text().toInt(&ok);
      if (!ok || (val < CENARIO_PRINCIPAL)) {
        LOG(WARNING) << "Ignorando valor de transicao: " << gerador.linha_transicao_cenario->text().toStdString();
      }
      proto_retornado->mutable_transicao_cenario()->set_id_cenario(ok ? val : CENARIO_INVALIDO);
      if (ok) {
        if (gerador.checkbox_transicao_posicao->checkState() == Qt::Checked) {
          proto_retornado->mutable_transicao_cenario()->set_x(gerador.spin_trans_x->value());
          proto_retornado->mutable_transicao_cenario()->set_y(gerador.spin_trans_y->value());
          proto_retornado->mutable_transicao_cenario()->set_z(gerador.spin_trans_z->value());
        } else {
          proto_retornado->mutable_transicao_cenario()->clear_x();
          proto_retornado->mutable_transicao_cenario()->clear_y();
          proto_retornado->mutable_transicao_cenario()->clear_z();
        }
      }
    } else {
      // Valor especial para denotar ausencia.
      proto_retornado->mutable_transicao_cenario()->set_id_cenario(CENARIO_INVALIDO);
    }
    if (gerador.combo_textura->currentIndex() == 0) {
      proto_retornado->clear_info_textura();
    } else {
      PreencheTexturaProtoRetornado(entidade.info_textura(), gerador.combo_textura, proto_retornado->mutable_info_textura());
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

ent::EntidadeProto* Visualizador3d::AbreDialogoTipoEntidade(
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
  // Rotulo.
  QString rotulo_str;
  gerador.campo_rotulo->setText(entidade.rotulo().c_str());
  // Rotulos especiais.
  std::string rotulos_especiais;
  for (const std::string& rotulo_especial : entidade.rotulo_especial()) {
    rotulos_especiais += rotulo_especial + "\n";
  }
  gerador.lista_rotulos->appendPlainText(rotulos_especiais.c_str());
  // Eventos entidades.
  std::string eventos;
  for (const auto& evento : entidade.evento()) {
    eventos += evento.descricao();
    if (evento.has_complemento()) {
      eventos += " (";
      eventos += net::to_string(evento.complemento());
      eventos += ")";
    }
    eventos += ": " + net::to_string(evento.rodadas()) + "\n";
  }
  gerador.lista_eventos->appendPlainText(eventos.c_str());

  // Visibilidade.
  gerador.checkbox_visibilidade->setCheckState(entidade.visivel() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_visibilidade->setEnabled(false);
  }
  // Selecionavel para jogadores.
  gerador.checkbox_selecionavel->setCheckState(entidade.selecionavel_para_jogador() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_selecionavel->setEnabled(false);
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
    QColor cor = QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    ent_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
  });
  // Cor da luz.
  ent::EntidadeProto luz_cor;
  if (entidade.has_luz()) {
    luz_cor.mutable_cor()->CopyFrom(entidade.luz().cor());
    gerador.botao_luz->setStyleSheet(CorParaEstilo(entidade.luz().cor()));
    gerador.spin_raio->setValue(entidade.luz().has_raio() ? entidade.luz().raio() : 6.0f);
  } else {
    ent::Cor branco;
    branco.set_r(1.0f);
    branco.set_g(1.0f);
    branco.set_b(1.0f);
    luz_cor.mutable_cor()->CopyFrom(branco);
    gerador.botao_luz->setStyleSheet(CorParaEstilo(branco));
    gerador.spin_raio->setValue(0.0f);
  }
  lambda_connect(gerador.botao_luz, SIGNAL(clicked()), [this, dialogo, &gerador, &luz_cor] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(luz_cor.cor()), dialogo, QObject::tr("Cor da luz"));
    if (!cor.isValid()) {
      return;
    }
    luz_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
    gerador.botao_luz->setStyleSheet(CorParaEstilo(cor));
    if (gerador.spin_raio->value() == 0.0) {
      gerador.spin_raio->setValue(6.0f);
    }
  });
  // Textura do objeto.
  PreencheComboTextura(entidade.info_textura().id(), notificacao.tabuleiro().id_cliente(), gerador.combo_textura);
  // Pontos de vida.
  gerador.spin_pontos_vida->setValue(entidade.pontos_vida());
  gerador.spin_max_pontos_vida->setValue(entidade.max_pontos_vida());
  // Aura.
  gerador.spin_aura->setValue(entidade.aura_m());
  // Voo.
  gerador.checkbox_voadora->setCheckState(entidade.voadora() ? Qt::Checked : Qt::Unchecked);
  // Caida.
  gerador.checkbox_caida->setCheckState(entidade.caida() ? Qt::Checked : Qt::Unchecked);
  // Morta.
  gerador.checkbox_morta->setCheckState(entidade.morta() ? Qt::Checked : Qt::Unchecked);
  // Translacao em Z.
  gerador.spin_translacao->setValue(entidade.pos().z());

  // Proxima salvacao: para funcionar, o combo deve estar ordenado da mesma forma que a enum ResultadoSalvacao.
  gerador.combo_salvacao->setCurrentIndex((int)entidade.proxima_salvacao());

  // Tipo de visao.
  gerador.combo_visao->setCurrentIndex((int)entidade.tipo_visao());
  lambda_connect(gerador.combo_visao, SIGNAL(currentIndexChanged(int)), [this, &gerador] () {
    gerador.spin_raio_visao_escuro->setEnabled(gerador.combo_visao->currentIndex() == ent::VISAO_ESCURO);
  });
  gerador.spin_raio_visao_escuro->setValue(entidade.has_alcance_visao() ? entidade.alcance_visao() : 18);
  gerador.spin_raio_visao_escuro->setEnabled(entidade.tipo_visao() == ent::VISAO_ESCURO);

  // Coisas que nao estao na UI.
  if (entidade.has_direcao_queda()) {
    proto_retornado->mutable_direcao_queda()->CopyFrom(entidade.direcao_queda());
  }

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor] () {
    if (gerador.campo_rotulo->text().isEmpty()) {
      proto_retornado->clear_rotulo();
    } else {
      proto_retornado->set_rotulo(gerador.campo_rotulo->text().toStdString());
    }
    QStringList lista_rotulos = gerador.lista_rotulos->toPlainText().split("\n", QString::SkipEmptyParts);
    for (const auto& rotulo : lista_rotulos) {
      proto_retornado->add_rotulo_especial(rotulo.toStdString());
    }
    google::protobuf::RepeatedPtrField<ent::EntidadeProto::Evento> eventos = ent::LeEventos(gerador.lista_eventos->toPlainText().toStdString());
    proto_retornado->mutable_evento()->Swap(&eventos);

    proto_retornado->set_tamanho(static_cast<ent::TamanhoEntidade>(gerador.slider_tamanho->sliderPosition()));
    proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
    if (gerador.spin_raio->value() > 0.0f) {
      proto_retornado->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
      proto_retornado->mutable_luz()->set_raio(gerador.spin_raio->value());
    } else {
      proto_retornado->clear_luz();
    }
    if (gerador.combo_textura->currentIndex() == 0) {
      proto_retornado->clear_info_textura();
    } else {
      PreencheTexturaProtoRetornado(entidade.info_textura(), gerador.combo_textura, proto_retornado->mutable_info_textura());
    }
    proto_retornado->set_pontos_vida(gerador.spin_pontos_vida->value());
    proto_retornado->set_max_pontos_vida(gerador.spin_max_pontos_vida->value());
    int aura = gerador.spin_aura->value();
    if (aura > 0) {
      proto_retornado->set_aura_m(aura);
    } else {
      proto_retornado->clear_aura_m();
    }
    proto_retornado->set_voadora(gerador.checkbox_voadora->checkState() == Qt::Checked);
    proto_retornado->set_caida(gerador.checkbox_caida->checkState() == Qt::Checked);
    proto_retornado->set_morta(gerador.checkbox_morta->checkState() == Qt::Checked);
    proto_retornado->set_visivel(gerador.checkbox_visibilidade->checkState() == Qt::Checked);
    proto_retornado->set_selecionavel_para_jogador(gerador.checkbox_selecionavel->checkState() == Qt::Checked);
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao->value());
    proto_retornado->clear_translacao_z_deprecated();
    proto_retornado->set_proxima_salvacao((ent::ResultadoSalvacao)gerador.combo_salvacao->currentIndex());
    proto_retornado->set_tipo_visao((ent::TipoVisao)gerador.combo_visao->currentIndex());
    if (proto_retornado->tipo_visao() == ent::VISAO_ESCURO) {
      proto_retornado->set_alcance_visao(gerador.spin_raio_visao_escuro->value());
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

ent::EntidadeProto* Visualizador3d::AbreDialogoEntidade(
    const ntf::Notificacao& notificacao) {
  if (notificacao.entidade().tipo() == ent::TE_ENTIDADE) {
    return AbreDialogoTipoEntidade(notificacao);
  } else if (notificacao.entidade().tipo() == ent::TE_FORMA || notificacao.entidade().tipo() == ent::TE_COMPOSTA) {
    return AbreDialogoTipoForma(notificacao);
  }
  return nullptr;
}

ent::TabuleiroProto* Visualizador3d::AbreDialogoCenario(
    const ntf::Notificacao& notificacao) {
  auto* proto_retornado = new ent::TabuleiroProto;
  proto_retornado->set_id_cenario(notificacao.tabuleiro().id_cenario());
  ifg::qt::Ui::DialogoIluminacao gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  const auto& tab_proto = notificacao.tabuleiro();
  VLOG(1) << "Modificando tabuleiro: " << tab_proto.ShortDebugString();

  // Id.
  gerador.campo_id->setText(QString::number(notificacao.tabuleiro().id_cenario()));
  gerador.campo_descricao->setText(notificacao.tabuleiro().descricao_cenario().c_str());
  // Cor ambiente.
  ent::Cor cor_ambiente_proto(tab_proto.luz_ambiente());
  gerador.botao_cor_ambiente->setStyleSheet(CorParaEstilo(cor_ambiente_proto));
  lambda_connect(gerador.botao_cor_ambiente, SIGNAL(clicked()), [this, dialogo, &gerador, &cor_ambiente_proto] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(cor_ambiente_proto), dialogo, QObject::tr("Cor da luz ambiente"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor_ambiente->setStyleSheet(CorParaEstilo(cor));
    cor_ambiente_proto.CopyFrom(CorParaProto(cor));
  });

  // Cor direcional.
  ent::Cor cor_direcional_proto(tab_proto.luz_direcional().cor());
  gerador.botao_cor_direcional->setStyleSheet(CorParaEstilo(cor_direcional_proto));
  lambda_connect(gerador.botao_cor_direcional, SIGNAL(clicked()), [this, dialogo, &gerador, &cor_direcional_proto] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(cor_direcional_proto), dialogo, QObject::tr("Cor da luz ambiente"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor_direcional->setStyleSheet(CorParaEstilo(cor));
    cor_direcional_proto.CopyFrom(CorParaProto(cor));
  });

  // Posicao na rosa dos ventos. No slider, o zero fica pra baixo enquanto no proto ele fica para direita.
  gerador.dial_posicao->setSliderPosition(tab_proto.luz_direcional().posicao_graus() + 90.0f);
  // Inclinacao: o zero do slider fica para baixo enquanto no proto ele fica para direita.
  gerador.dial_inclinacao->setSliderPosition(tab_proto.luz_direcional().inclinacao_graus() + 90.0f);

  // Nevoa.
  if (tab_proto.has_nevoa()) {
    gerador.checkbox_nevoa->setCheckState(Qt::Checked);
    gerador.linha_nevoa_min->setEnabled(true);
    gerador.linha_nevoa_min->setText(QString().setNum(tab_proto.nevoa().minimo()));
    gerador.linha_nevoa_max->setEnabled(true);
    gerador.linha_nevoa_max->setText(QString().setNum(tab_proto.nevoa().maximo()));
  } else {
    gerador.checkbox_nevoa->setCheckState(Qt::Unchecked);
  }

  // Textura do tabuleiro.
  PreencheComboTextura(tab_proto.info_textura().id().c_str(), notificacao.tabuleiro().id_cliente(), gerador.combo_fundo);
  // Ceu do tabuleiro.
  PreencheComboTextura(tab_proto.info_textura_ceu().id().c_str(), notificacao.tabuleiro().id_cliente(), gerador.combo_ceu);
  gerador.checkbox_luz_ceu->setCheckState(tab_proto.aplicar_luz_ambiente_textura_ceu() ? Qt::Checked : Qt::Unchecked);

  // Ladrilho de textura.
  gerador.checkbox_ladrilho->setCheckState(tab_proto.ladrilho() ? Qt::Checked : Qt::Unchecked);
  // grade.
  gerador.checkbox_grade->setCheckState(tab_proto.desenha_grade() ? Qt::Checked : Qt::Unchecked);
  // Mestre apenas.
  gerador.checkbox_mestre_apenas->setCheckState(tab_proto.textura_mestre_apenas() ? Qt::Checked : Qt::Unchecked);

  // Tamanho.
  gerador.linha_largura->setText(QString::number(tab_proto.largura()));
  gerador.linha_altura->setText(QString::number(tab_proto.altura()));
  lambda_connect(gerador.checkbox_tamanho_automatico, SIGNAL(stateChanged(int)), [this, &gerador] () {
    int novo_estado = gerador.checkbox_tamanho_automatico->checkState();
    // Deve ter textura.
    if (novo_estado == Qt::Checked && gerador.combo_fundo->currentIndex() == 0) {
      gerador.checkbox_tamanho_automatico->setCheckState(Qt::Unchecked);
      return;
    }
    gerador.linha_largura->setEnabled(novo_estado != Qt::Checked);
    gerador.linha_altura->setEnabled(novo_estado != Qt::Checked);
  });

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(gerador.botoes, SIGNAL(accepted()),
                 [this, tab_proto, dialogo, &gerador, &cor_ambiente_proto, &cor_direcional_proto, proto_retornado] {
    proto_retornado->mutable_luz_direcional()->set_posicao_graus(gerador.dial_posicao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz_direcional()->set_inclinacao_graus(gerador.dial_inclinacao->sliderPosition() - 90.0f);
    proto_retornado->mutable_luz_direcional()->mutable_cor()->Swap(&cor_direcional_proto);
    proto_retornado->mutable_luz_ambiente()->Swap(&cor_ambiente_proto);
    // Nevoa.
    if (gerador.checkbox_nevoa->checkState() == Qt::Checked) {
      bool ok;
      int d_min = gerador.linha_nevoa_min->text().toInt(&ok);
      if (!ok) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, nevoa minima invalida: "
                     << gerador.linha_nevoa_min->text().toStdString();
        return;
      }
      int d_max = gerador.linha_nevoa_max->text().toInt(&ok);
      if (!ok || d_min >= d_max) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, nevoa maxima invalida: "
                     << gerador.linha_nevoa_max->text().toStdString();
        return;
      }
      proto_retornado->mutable_nevoa()->set_minimo(d_min);
      proto_retornado->mutable_nevoa()->set_maximo(d_max);
    } else {
      proto_retornado->clear_nevoa();
    }
    // Descricao.
    proto_retornado->set_descricao_cenario(gerador.campo_descricao->text().toStdString());
    // Textura.
    if (gerador.combo_fundo->currentIndex() == 0) {
      proto_retornado->clear_info_textura();
    } else {
      PreencheTexturaProtoRetornado(tab_proto.info_textura(), gerador.combo_fundo, proto_retornado->mutable_info_textura());
    }
    // Ladrilho.
    if (gerador.combo_fundo->currentIndex() != 0) {
      proto_retornado->set_ladrilho(gerador.checkbox_ladrilho->checkState() == Qt::Checked);
    } else {
      proto_retornado->clear_ladrilho();
    }
    // Textura ceu.
    if (gerador.combo_ceu->currentIndex() == 0) {
      proto_retornado->clear_info_textura_ceu();
    } else {
      PreencheTexturaProtoRetornado(tab_proto.info_textura_ceu(), gerador.combo_ceu, proto_retornado->mutable_info_textura_ceu());
    }
    proto_retornado->set_aplicar_luz_ambiente_textura_ceu(gerador.checkbox_luz_ceu->checkState() == Qt::Checked);
    // Tamanho do tabuleiro.
    if (gerador.checkbox_tamanho_automatico->checkState() == Qt::Checked) {
      int indice = gerador.combo_fundo->currentIndex();
      arq::tipo_e tipo = static_cast<arq::tipo_e>(gerador.combo_fundo->itemData(indice).toInt());
      // Busca tamanho da textura. Copia o objeto aqui porque a funcao PreencheInfoTextura o modifica.
      ent::InfoTextura textura = proto_retornado->info_textura();
      unsigned int largura = 0, altura = 0;
      std::string nome;
      if (tipo == arq::TIPO_TEXTURA_LOCAL) {
        nome = gerador.combo_fundo->itemText(indice).split(":")[1].toStdString();
      } else {
        nome = gerador.combo_fundo->itemText(indice).toStdString();
      }
      PreencheInfoTextura(nome, tipo, &textura, &largura, &altura);
      proto_retornado->set_largura(largura / 8);
      proto_retornado->set_altura(altura / 8);
    } else {
      // Converte da entrada.
      bool ok = true;
      int largura = gerador.linha_largura->text().toInt(&ok);
      if (!ok) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, largura invalido: "
                     << gerador.linha_altura->text().toStdString();
        return;
      }
      int altura = gerador.linha_altura->text().toInt(&ok);
      if (!ok) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, altura invalido: "
                     << gerador.linha_largura->text().toStdString();
        return;
      }
      proto_retornado->set_largura(largura);
      proto_retornado->set_altura(altura);
    }
    // Grade.
    proto_retornado->set_desenha_grade(
        gerador.checkbox_grade->checkState() == Qt::Checked ? true : false);
    // Mestre apenas.
    proto_retornado->set_textura_mestre_apenas(
        gerador.checkbox_mestre_apenas->checkState() == Qt::Checked ? true : false);

    VLOG(1) << "Retornando tabuleiro: " << proto_retornado->ShortDebugString();
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
  gerador.checkbox_mostrar_fps->setCheckState(opcoes_proto.mostra_fps() ? Qt::Checked : Qt::Unchecked);
  // Texturas de frente.
  gerador.checkbox_texturas_sempre_de_frente->setCheckState(
      opcoes_proto.texturas_sempre_de_frente() ? Qt::Checked : Qt::Unchecked);
  // Iluminacao mestre.
  gerador.checkbox_iluminacao_mestre->setCheckState(
      opcoes_proto.iluminacao_mestre_igual_jogadores() ? Qt::Checked : Qt::Unchecked);
  // Rosa dos ventos.
  gerador.checkbox_rosa_dos_ventos->setCheckState(
      opcoes_proto.desenha_rosa_dos_ventos() ? Qt::Checked : Qt::Unchecked);
  // Serrilhamento.
  gerador.checkbox_anti_aliasing->setCheckState(
      opcoes_proto.anti_aliasing() ? Qt::Checked : Qt::Unchecked);
  // grade.
  gerador.checkbox_grade->setCheckState(opcoes_proto.desenha_grade() ? Qt::Checked : Qt::Unchecked);
  // Controle virtual.
  gerador.checkbox_controle->setCheckState(opcoes_proto.desenha_controle_virtual() ? Qt::Checked : Qt::Unchecked);

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()), [this, dialogo, &gerador, proto_retornado] {
    proto_retornado->set_mostra_fps(gerador.checkbox_mostrar_fps->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_texturas_sempre_de_frente(
        gerador.checkbox_texturas_sempre_de_frente->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_iluminacao_mestre_igual_jogadores(
        gerador.checkbox_iluminacao_mestre->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_desenha_rosa_dos_ventos(
        gerador.checkbox_rosa_dos_ventos->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_anti_aliasing(gerador.checkbox_anti_aliasing->checkState() == Qt::Checked);
    proto_retornado->set_desenha_grade(
        gerador.checkbox_grade->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_desenha_controle_virtual(
        gerador.checkbox_controle->checkState() == Qt::Checked ? true : false);
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

}  // namespace qt
}  // namespace ifg
