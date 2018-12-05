#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

#include <QBoxLayout>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemDelegate>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScreen>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>

#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "goog/stringprintf.h"
#include "ifg/qt/atualiza_ui.h"
#include "ifg/qt/bonus_util.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/evento_util.h"
#include "ifg/qt/itens_magicos_util.h"
#include "ifg/qt/pericias_util.h"
#include "ifg/qt/pocoes_util.h"
#include "ifg/qt/talentos_util.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/util.h"
#include "ifg/qt/ui/forma.h"
#include "ifg/qt/ui/cenario.h"
#include "ifg/qt/ui/opcoes.h"
#include "ifg/qt/visualizador3d.h"
#include "ifg/tecladomouse.h"
#include "log/log.h"
#include "m3d/m3d.h"
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
    tabuleiro_->DesativaWatchdogSeMestre();
  }
  ~DesativadorWatchdogEscopo() {
    tabuleiro_->ReativaWatchdogSeMestre();
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
  auto format = QGLFormat(
      QGL::DepthBuffer | QGL::Rgba | QGL::DoubleBuffer | QGL::AlphaChannel | QGL::SampleBuffers | QGL::StencilBuffer);
  format.setVersion(2, 1);
  return format;
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
void PreencheComboTextura(const std::string& id_corrente, int id_cliente, std::function<bool(const std::string&)> filtro, QComboBox* combo_textura) {
  combo_textura->addItem(combo_textura->tr("Nenhuma"), QVariant(-1));
  auto Ordena = [filtro] (std::vector<std::string> texturas) -> std::vector<std::string> {
    std::sort(texturas.begin(), texturas.end());
    return texturas;
  };
  std::vector<std::string> texturas = Ordena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA, filtro));
  std::vector<std::string> texturas_baixadas = Ordena((arq::ConteudoDiretorio(arq::TIPO_TEXTURA_BAIXADA, filtro)));
  std::vector<std::string> texturas_locais = Ordena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA_LOCAL, filtro));

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
  ExpandeComboBox(combo_textura);
}

void PreencheComboTexturaCeu(const std::string& id_corrente, int id_cliente, QComboBox* combo_textura) {
  combo_textura->addItem(combo_textura->tr("Nenhuma"), QVariant(-1));
  auto NaoEhSkybox = [] (const std::string& nome_arquivo) {
    return nome_arquivo.find("skybox") != 0;
  };
  auto EhCuboSecundario = [] (const std::string& nome_arquivo) {
    return nome_arquivo.find("esquerda.png") != std::string::npos ||
           nome_arquivo.find("frente.png") != std::string::npos ||
           nome_arquivo.find("atras.png") != std::string::npos ||
           nome_arquivo.find("cima.png") != std::string::npos ||
           nome_arquivo.find("baixo.png") != std::string::npos;
  };
  auto FiltraOrdena = [NaoEhSkybox, EhCuboSecundario] (std::vector<std::string> texturas) -> std::vector<std::string> {
    texturas.erase(std::remove_if(texturas.begin(), texturas.end(), NaoEhSkybox), texturas.end());
    texturas.erase(std::remove_if(texturas.begin(), texturas.end(), EhCuboSecundario), texturas.end());
    for (std::string& textura : texturas) {
      std::size_t pos = textura.find("direita.png");
      if (pos != std::string::npos) {
        textura.replace(pos, 11, ".cube");
      }
    }
    std::sort(texturas.begin(), texturas.end());
    return texturas;
  };
  std::vector<std::string> texturas = FiltraOrdena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA));
  std::vector<std::string> texturas_baixadas = FiltraOrdena((arq::ConteudoDiretorio(arq::TIPO_TEXTURA_BAIXADA)));
  std::vector<std::string> texturas_locais = FiltraOrdena(arq::ConteudoDiretorio(arq::TIPO_TEXTURA_LOCAL));

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

void PreencheComboModelo3d(const std::string& id_corrente, QComboBox* combo_modelos_3d) {
  combo_modelos_3d->addItem(combo_modelos_3d->tr("Nenhum"), QVariant(-1));
  auto Ordena = [] (std::vector<std::string> modelos) -> std::vector<std::string> {
    std::sort(modelos.begin(), modelos.end());
    return modelos;
  };
  std::vector<std::string> modelos_3d = Ordena(m3d::Modelos3d::ModelosDisponiveis(true  /*global*/));
  std::vector<std::string> modelos_3d_baixados = Ordena(m3d::Modelos3d::ModelosDisponiveis(false  /*global*/));

  AdicionaSeparador(combo_modelos_3d->tr("Globais"), combo_modelos_3d);
  for (const std::string& modelo_3d : modelos_3d) {
    combo_modelos_3d->addItem(QString(modelo_3d.substr(0, modelo_3d.find(".binproto")).c_str()), QVariant(arq::TIPO_MODELOS_3D));
  }
  AdicionaSeparador(combo_modelos_3d->tr("Baixados"), combo_modelos_3d);
  for (const std::string& modelo_3d : modelos_3d_baixados) {
    combo_modelos_3d->addItem(modelo_3d.substr(0, modelo_3d.find(".binproto")).c_str(), QVariant(arq::TIPO_MODELOS_3D_BAIXADOS));
  }
  if (id_corrente.empty()) {
    combo_modelos_3d->setCurrentIndex(0);
  } else {
    int index = combo_modelos_3d->findText(QString(id_corrente.c_str()));
    if (index == -1) {
      index = 0;
    }
    combo_modelos_3d->setCurrentIndex(index);
  }
  ExpandeComboBox(combo_modelos_3d);
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
    const ent::Tabelas& tabelas,
    TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QGLWidget(Formato(), pai),
       tabelas_(tabelas),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro) {
  const ent::OpcoesProto& opcoes = tabuleiro->Opcoes();
  luz_por_pixel_ = opcoes.iluminacao_por_pixel();
  central_->RegistraReceptor(this);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  //std::cerr << "phyx: " << QGuiApplication::primaryScreen()->physicalDotsPerInchX();
  //std::cerr << "phyy: " << QGuiApplication::primaryScreen()->physicalDotsPerInchY();
  //std::cerr << "logx: " << QGuiApplication::primaryScreen()->logicalDotsPerInchX();
  //std::cerr << "logy: " << QGuiApplication::primaryScreen()->logicalDotsPerInchY();
  //std::cerr << "screen: " << *QGuiApplication::primaryScreen();

  scale_ = QApplication::desktop()->devicePixelRatio();
  std::cerr << "scale: " << scale_ << std::endl;
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
      gl::IniciaGl(luz_por_pixel_, scale_);
    }
    tabuleiro_->IniciaGL();
  } catch (const std::logic_error& erro) {
    // Este log de erro eh pro caso da aplicacao morrer e nao conseguir mostrar a mensagem.
    LOG(ERROR) << "Erro na inicializacao GL " << erro.what();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErro(erro.what()));
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
      std::unique_ptr<ent::EntidadeProto> entidade_proto(AbreDialogoEntidade(notificacao));
      if (entidade_proto == nullptr) {
        VLOG(1) << "Alterações descartadas";
        break;
      }
      auto n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_ENTIDADE);
      n->mutable_entidade()->Swap(entidade_proto.get());
      central_->AdicionaNotificacao(n.release());
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
      auto n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_TABULEIRO);
      n->mutable_tabuleiro()->Swap(tabuleiro);
      central_->AdicionaNotificacao(n.release());
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_OPCOES: {
      if (!notificacao.has_opcoes()) {
        // O tabuleiro criara a mensagem completa.
        return false;
      }
      DesativadorWatchdogEscopo dw(tabuleiro_);
      std::unique_ptr<ent::OpcoesProto> opcoes(AbreDialogoOpcoes(notificacao));
      if (opcoes.get() == nullptr) {
        VLOG(1) << "Alterações de opcoes descartadas";
        break;
      }
      auto n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_OPCOES);
      n->mutable_opcoes()->Swap(opcoes.get());
      central_->AdicionaNotificacao(n.release());
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
       event->x() * scale_,
       (height() - event->y()) * scale_);
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
    delete event2;
    return;
  }
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      event->x() * scale_, (height() - event->y()) * scale_);
  event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  int x = event->globalX();
  int y = event->globalY();
  if (teclado_mouse_->TrataMovimentoMouse(event->x() * scale_, (height() - event->y()) * scale_)) {
    QCursor::setPos(x_antes_, y_antes_);
  } else {
    x_antes_ = x;
    y_antes_ = y;
  }
  event->accept();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  teclado_mouse_->TrataRodela(event->delta());
  event->accept();
}

namespace {
void PreencheComboCenarios(const ent::TabuleiroProto& tabuleiro, QComboBox* combo) {
  combo->addItem("Novo", QVariant());
  combo->addItem("Principal", QVariant(CENARIO_PRINCIPAL));
  for (const auto& sub_cenario : tabuleiro.sub_cenario()) {
    std::string descricao;
    if (sub_cenario.descricao_cenario().empty()) {
      descricao = google::protobuf::StringPrintf("Sub Cenário: %d", sub_cenario.id_cenario());
    } else {
      descricao = google::protobuf::StringPrintf("%s (%d)", sub_cenario.descricao_cenario().c_str(), sub_cenario.id_cenario());
    }
    combo->addItem(QString::fromUtf8(descricao.c_str()), QVariant(sub_cenario.id_cenario()));
  }
  ExpandeComboBox(combo);
}

// Retorna CENARIO_INVALIDO (-2) caso igual a novo.
int IdCenarioComboCenarios(const QComboBox* combo) {
  QVariant qval = combo->itemData(combo->currentIndex());
  if (!qval.isValid()) return CENARIO_INVALIDO;
  return qval.toInt();
}

void SelecionaCenarioComboCenarios(int id_cenario, const ent::TabuleiroProto& proto, QComboBox* combo) {
  if (id_cenario == CENARIO_PRINCIPAL) {
    // 0 eh novo, 1 eh o principal.
    combo->setCurrentIndex(1);
    return;
  }
  int i = 2;
  for (const auto& sub_cenario : proto.sub_cenario()) {
    if (sub_cenario.id_cenario() == id_cenario) {
      combo->setCurrentIndex(i);
      return;
    }
    ++i;
  }
  combo->setCurrentIndex(0);
}

}  // namespace

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
  gerador.lista_rotulos->appendPlainText((rotulos_especiais.c_str()));
  // Visibilidade.
  gerador.checkbox_visibilidade->setCheckState(entidade.visivel() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_faz_sombra->setCheckState(entidade.faz_sombra() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_dois_lados->setCheckState(entidade.dois_lados() ? Qt::Checked : Qt::Unchecked);

  // Fixa.
  gerador.checkbox_fixa->setCheckState(entidade.fixa() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre()) {
    gerador.checkbox_selecionavel->setEnabled(false);
  }
  lambda_connect(gerador.checkbox_fixa, SIGNAL(clicked()),
      [&gerador, &notificacao] () {
    gerador.checkbox_selecionavel->setEnabled(notificacao.modo_mestre() &&
                                              (gerador.checkbox_fixa->checkState() != Qt::Checked));
  });
  // Selecionavel para jogadores.
  gerador.checkbox_selecionavel->setCheckState(entidade.selecionavel_para_jogador() ? Qt::Checked : Qt::Unchecked);
  if (!notificacao.modo_mestre() || entidade.fixa()) {
    gerador.checkbox_selecionavel->setEnabled(false);
  }
  // Colisao.
  gerador.checkbox_colisao->setCheckState(entidade.causa_colisao() ? Qt::Checked : Qt::Unchecked);

  // Textura do objeto.
  PreencheComboTextura(entidade.info_textura().id(), notificacao.tabuleiro().id_cliente(), ent::FiltroTexturaEntidade, gerador.combo_textura);
  gerador.checkbox_ladrilho->setCheckState(
      entidade.info_textura().has_modo_textura()
      ? (entidade.info_textura().modo_textura() == GL_REPEAT ? Qt::Checked : Qt::Unchecked)
      : Qt::Unchecked);
  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(entidade.cor());
  gerador.checkbox_cor->setCheckState(entidade.has_cor() ? Qt::Checked : Qt::Unchecked);
  gerador.botao_cor->setStyleSheet(CorParaEstilo(entidade.cor()));
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [dialogo, &gerador, &ent_cor] {
    QColor cor = QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    gerador.checkbox_cor->setCheckState(Qt::Checked);
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

  // Translacao em Z.
  gerador.spin_translacao_quad->setValue(entidade.pos().z() * ent::METROS_PARA_QUADRADOS);

  // Rotacoes.
  auto AjustaSliderSpin = [] (float angulo, QDial* dial, QSpinBox* spin) {
    // Poe o angulo entre -180, 180
    angulo = fmodf(angulo, 360.0f);
    if (angulo < -180.0f) {
      angulo += 360.0f;
    } else if (angulo > 180.0f) {
      angulo -= 360.0f;
    }
    dial->setSliderPosition(-angulo - 180.0f);
    spin->setValue(angulo);
    lambda_connect(dial, SIGNAL(valueChanged(int)), [spin, dial] {
        spin->setValue(180 - dial->value());
    });
    lambda_connect(spin, SIGNAL(valueChanged(int)), [spin, dial] {
        dial->setValue(-spin->value() - 180);
    });
  };
  AjustaSliderSpin(entidade.rotacao_z_graus(), gerador.dial_rotacao, gerador.spin_rotacao);
  AjustaSliderSpin(entidade.rotacao_y_graus(), gerador.dial_rotacao_y, gerador.spin_rotacao_y);
  AjustaSliderSpin(entidade.rotacao_x_graus(), gerador.dial_rotacao_x, gerador.spin_rotacao_x);

  // Escalas.
  gerador.spin_escala_x_quad->setValue(ent::METROS_PARA_QUADRADOS * entidade.escala().x());
  gerador.spin_escala_y_quad->setValue(ent::METROS_PARA_QUADRADOS * entidade.escala().y());
  gerador.spin_escala_z_quad->setValue(ent::METROS_PARA_QUADRADOS * entidade.escala().z());

  gerador.lista_tesouro->appendPlainText((entidade.tesouro().tesouro().c_str()));

  PreencheComboCenarios(tabuleiro_->Proto(), gerador.combo_id_cenario);

  // Transicao de cenario.
  auto habilita_posicao = [gerador] {
    gerador.checkbox_transicao_posicao->setCheckState(Qt::Checked);
  };
  lambda_connect(gerador.spin_trans_x, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.spin_trans_y, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.spin_trans_z, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.combo_transicao, SIGNAL(currentIndexChanged(int)), [this, &gerador, proto_retornado] {
    bool trans_cenario = gerador.combo_transicao->currentIndex() == ent::EntidadeProto::TRANS_CENARIO;
    //gerador.linha_transicao_cenario->setEnabled(trans_cenario);
    SelecionaCenarioComboCenarios(
        proto_retornado->transicao_cenario().has_id_cenario() ? proto_retornado->transicao_cenario().id_cenario() : CENARIO_INVALIDO,
        tabuleiro_->Proto(), gerador.combo_id_cenario);
    gerador.combo_id_cenario->setEnabled(trans_cenario);
    gerador.checkbox_transicao_posicao->setEnabled(trans_cenario);
    gerador.spin_trans_x->setEnabled(trans_cenario);
    gerador.spin_trans_y->setEnabled(trans_cenario);
    gerador.spin_trans_z->setEnabled(trans_cenario);
  });
  if (!entidade.transicao_cenario().has_id_cenario()) {
    bool trans_tesouro = entidade.tipo_transicao() == ent::EntidadeProto::TRANS_TESOURO;
    gerador.combo_transicao->setCurrentIndex(trans_tesouro ? ent::EntidadeProto::TRANS_TESOURO : ent::EntidadeProto::TRANS_NENHUMA);
    //gerador.linha_transicao_cenario->setEnabled(false);
    gerador.combo_id_cenario->setEnabled(false);
    gerador.checkbox_transicao_posicao->setEnabled(false);
    gerador.spin_trans_x->setEnabled(false);
    gerador.spin_trans_y->setEnabled(false);
    gerador.spin_trans_z->setEnabled(false);
  } else {
    gerador.combo_transicao->setCurrentIndex(ent::EntidadeProto::TRANS_CENARIO);
    //gerador.linha_transicao_cenario->setText(QString::number(entidade.transicao_cenario().id_cenario()));
    SelecionaCenarioComboCenarios(entidade.transicao_cenario().id_cenario(), tabuleiro_->Proto(), gerador.combo_id_cenario);

    gerador.combo_id_cenario->setEnabled(true);
    if (entidade.transicao_cenario().has_x()) {
      gerador.checkbox_transicao_posicao->setCheckState(Qt::Checked);
      gerador.spin_trans_x->setValue(entidade.transicao_cenario().x());
      gerador.spin_trans_y->setValue(entidade.transicao_cenario().y());
      gerador.spin_trans_z->setValue(entidade.transicao_cenario().z());
    } else {
      gerador.checkbox_transicao_posicao->setCheckState(Qt::Unchecked);
    }
  }
  // Esconde dialogo e entra no modo de selecao.
  lambda_connect(gerador.botao_transicao_mapa, SIGNAL(clicked()), [this, dialogo, gerador, &entidade, &proto_retornado] {
    auto notificacao = ntf::NovaNotificacao(ntf::TN_ENTRAR_MODO_SELECAO_TRANSICAO);
    notificacao->mutable_entidade()->set_id(entidade.id());
    notificacao->mutable_entidade()->set_tipo_transicao(ent::EntidadeProto::TRANS_CENARIO);
    if (entidade.has_transicao_cenario()) {
      *notificacao->mutable_entidade()->mutable_transicao_cenario() = entidade.transicao_cenario();
    }
    central_->AdicionaNotificacao(notificacao.release());
    delete proto_retornado;
    proto_retornado = nullptr;
    dialogo->reject();
    LOG(INFO) << "rejeitando...";
  });

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
    if (gerador.checkbox_cor->checkState() == Qt::Checked) {
      proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
      proto_retornado->mutable_cor()->set_a(gerador.slider_alfa->value() / 100.0f);
    } else {
      proto_retornado->clear_cor();
    }
    proto_retornado->set_visivel(gerador.checkbox_visibilidade->checkState() == Qt::Checked);
    proto_retornado->set_faz_sombra(gerador.checkbox_faz_sombra->checkState() == Qt::Checked);
    proto_retornado->set_dois_lados(gerador.checkbox_dois_lados->checkState() == Qt::Checked);
    proto_retornado->set_selecionavel_para_jogador(gerador.checkbox_selecionavel->checkState() == Qt::Checked);
    proto_retornado->set_causa_colisao(gerador.checkbox_colisao->checkState() == Qt::Checked);
    bool fixa = gerador.checkbox_fixa->checkState() == Qt::Checked;
    if (fixa) {
      // Override.
      proto_retornado->set_selecionavel_para_jogador(false);
    }
    proto_retornado->set_fixa(fixa);
    proto_retornado->set_rotacao_z_graus(-gerador.dial_rotacao->sliderPosition() + 180.0f);
    proto_retornado->set_rotacao_y_graus(-gerador.dial_rotacao_y->sliderPosition() + 180.0f);
    proto_retornado->set_rotacao_x_graus(-gerador.dial_rotacao_x->sliderPosition() + 180.0f);
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao_quad->value() * ent::QUADRADOS_PARA_METROS);
    proto_retornado->clear_translacao_z_deprecated();
    proto_retornado->mutable_escala()->set_x(ent::QUADRADOS_PARA_METROS * gerador.spin_escala_x_quad->value());
    proto_retornado->mutable_escala()->set_y(ent::QUADRADOS_PARA_METROS * gerador.spin_escala_y_quad->value());
    proto_retornado->mutable_escala()->set_z(ent::QUADRADOS_PARA_METROS * gerador.spin_escala_z_quad->value());
    proto_retornado->mutable_tesouro()->set_tesouro(gerador.lista_tesouro->toPlainText().toStdString());
    if (gerador.combo_transicao->currentIndex() == ent::EntidadeProto::TRANS_CENARIO) {
      proto_retornado->set_tipo_transicao(ent::EntidadeProto::TRANS_CENARIO);
      //bool ok = false;
      //int val = gerador.linha_transicao_cenario->text().toInt(&ok);
      QVariant qval = gerador.combo_id_cenario->itemData(gerador.combo_id_cenario->currentIndex());
      int val = 0;
      if (!qval.isValid()) {
        // Busca um novo id.
        while (std::any_of(tabuleiro_->Proto().sub_cenario().begin(), tabuleiro_->Proto().sub_cenario().end(),
               [val] (const ent::TabuleiroProto& cenario) { return cenario.id_cenario() == val; })) {
          ++val;
        }
      } else {
        val = qval.toInt();
      }
      proto_retornado->mutable_transicao_cenario()->set_id_cenario(val);
      if (gerador.checkbox_transicao_posicao->checkState() == Qt::Checked) {
        proto_retornado->mutable_transicao_cenario()->set_x(gerador.spin_trans_x->value());
        proto_retornado->mutable_transicao_cenario()->set_y(gerador.spin_trans_y->value());
        proto_retornado->mutable_transicao_cenario()->set_z(gerador.spin_trans_z->value());
      } else {
        proto_retornado->mutable_transicao_cenario()->clear_x();
        proto_retornado->mutable_transicao_cenario()->clear_y();
        proto_retornado->mutable_transicao_cenario()->clear_z();
      }
    } else if (gerador.combo_transicao->currentIndex() == ent::EntidadeProto::TRANS_TESOURO) {
      proto_retornado->set_tipo_transicao(ent::EntidadeProto::TRANS_TESOURO);
    } else {
      // Valor especial para denotar ausencia.
      proto_retornado->set_tipo_transicao(ent::EntidadeProto::TRANS_NENHUMA);
      proto_retornado->mutable_transicao_cenario()->set_id_cenario(CENARIO_INVALIDO);
    }
    if (gerador.combo_textura->currentIndex() == 0) {
      proto_retornado->clear_info_textura();
    } else {
      PreencheTexturaProtoRetornado(entidade.info_textura(), gerador.combo_textura, proto_retornado->mutable_info_textura());
      if (gerador.checkbox_ladrilho->checkState() == Qt::Checked) {
        proto_retornado->mutable_info_textura()->set_modo_textura(GL_REPEAT);
      } else {
        proto_retornado->mutable_info_textura()->clear_modo_textura();
      }
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

namespace {

//-----------------------------------------------------
// Funcoes para Visualizador3d::AbreDialogoTipoEntidade
//-----------------------------------------------------

QString NumeroSinalizado(int valor) {
  QString ret = QString::number(valor);
  if (valor > 0) ret.prepend("+");
  return ret;
}

ent::EmpunhaduraArma ComboParaEmpunhadura(const QComboBox* combo) {
  if (!ent::EmpunhaduraArma_IsValid(combo->currentIndex())) {
    return ent::EA_ARMA_APENAS;
  }
  return static_cast<ent::EmpunhaduraArma>(combo->currentIndex());
}

// Se houver selecionada, ira atualizar o dado de ataque. Caso contrario, criara um novo dado de ataque com os dados
// lidos dos controles da UI. No final, recomputara dependencias e atualizara a UI.
void AdicionaOuAtualizaAtaqueEntidade(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  int indice = gerador.lista_ataques->currentRow();
  bool indice_valido = (indice >= 0 && indice < proto_retornado->dados_ataque().size());
  ent::EntidadeProto::DadosAtaque da = indice_valido ? proto_retornado->dados_ataque(indice) : ent::EntidadeProto::DadosAtaque::default_instance();
  da.set_tipo_ataque(CurrentData(gerador.combo_tipo_ataque).toString().toStdString());

  da.set_bonus_magico(gerador.spin_bonus_magico->value());
  da.set_municao(gerador.spin_municao->value());
  if (gerador.spin_ordem_ataque->value() > 0) {
    da.set_ordem_ataque(gerador.spin_ordem_ataque->value() - 1);
  } else {
    da.clear_ordem_ataque();
  }

  da.set_obra_prima(gerador.checkbox_op->checkState() == Qt::Checked);
  da.set_empunhadura(ComboParaEmpunhadura(gerador.combo_empunhadura));
  std::string id = gerador.combo_arma->itemData(gerador.combo_arma->currentIndex()).toString().toStdString();
  if (id == "nenhuma") {
    ent::DanoArma dano_arma = ent::LeDanoArma(gerador.linha_dano->text().toStdString());
    da.set_dano_basico(dano_arma.dano);
    da.set_multiplicador_critico(dano_arma.multiplicador);
    da.set_margem_critico(dano_arma.margem_critico);
  } else {
    da.set_id_arma(id);
    // Se houver dano, usa ele mesmo com id. Deixa RecomputaDependencias decidir.
    if (!gerador.linha_dano->text().isEmpty()) {
      ent::DanoArma dano_arma = ent::LeDanoArma(gerador.linha_dano->text().toUtf8().constData());
      da.set_dano_basico(dano_arma.dano);
      da.set_multiplicador_critico(dano_arma.multiplicador);
      da.set_margem_critico(dano_arma.margem_critico);
    }
  }
  da.set_grupo(gerador.linha_grupo_ataque->text().toStdString());
  da.set_rotulo(gerador.linha_rotulo_ataque->text().toStdString());
  da.set_incrementos(gerador.spin_incrementos->value());
  if (gerador.spin_alcance_quad->value() > 0) {
    da.set_alcance_m(gerador.spin_alcance_quad->value() * ent::QUADRADOS_PARA_METROS);
  } else {
    da.clear_alcance_m();
  }
  if (indice_valido) {
    proto_retornado->mutable_dados_ataque(indice)->Swap(&da);
  } else {
    proto_retornado->add_dados_ataque()->Swap(&da);
  }
  RecomputaDependencias(tabelas, proto_retornado);
  AtualizaUI(tabelas, gerador, *proto_retornado);
}

void PreencheConfiguraPontosVida(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  lambda_connect(gerador.botao_bonus_pv_temporario, SIGNAL(clicked()), [this_, &gerador, proto_retornado] () {
    AbreDialogoBonus(this_, proto_retornado->mutable_pontos_vida_temporarios_por_fonte());
    ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
    AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
  });
  AtualizaUIPontosVida(gerador, proto);
}

void PreencheConfiguraTendencia(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  lambda_connect(gerador.slider_bem_mal, SIGNAL(valueChanged(int)), [&gerador, proto_retornado] () {
    proto_retornado->mutable_tendencia()->set_eixo_bem_mal(gerador.slider_bem_mal->value() / 8.0f);
  });
  lambda_connect(gerador.slider_ordem_caos, SIGNAL(valueChanged(int)), [&gerador, proto_retornado] () {
    proto_retornado->mutable_tendencia()->set_eixo_ordem_caos(gerador.slider_ordem_caos->value() / 8.0f);
  });
  AtualizaUITendencia(this_->tabelas(), gerador, *proto_retornado);
}

void PreencheConfiguraComboArmaduraEscudo(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  QComboBox* combo_armadura = gerador.combo_armadura;
  QComboBox* combo_escudo = gerador.combo_escudo;
  const ent::Tabelas& tabelas = this_->tabelas();
  for (const auto& armadura : tabelas.todas().tabela_armaduras().armaduras()) {
    combo_armadura->addItem((armadura.nome().c_str()), QVariant(armadura.id().c_str()));
  }
  for (const auto& escudo : tabelas.todas().tabela_escudos().escudos()) {
    combo_escudo->addItem((escudo.nome().c_str()), QVariant(escudo.id().c_str()));
  }
  lambda_connect(combo_armadura, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_armadura] () {
    QVariant id = combo_armadura->itemData(combo_armadura->currentIndex());
    proto_retornado->mutable_dados_defesa()->set_id_armadura(id.toString().toStdString());
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
    AtualizaUIIniciativa(tabelas, gerador, *proto_retornado);
    AtualizaUIAtributos(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(combo_escudo, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_escudo] () {
    QVariant id = combo_escudo->itemData(combo_escudo->currentIndex());
    proto_retornado->mutable_dados_defesa()->set_id_escudo(id.toString().toStdString());
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
    AtualizaUIIniciativa(tabelas, gerador, *proto_retornado);
    AtualizaUIAtributos(tabelas, gerador, *proto_retornado);
  });
  ExpandeComboBox(combo_armadura);
  ExpandeComboBox(combo_escudo);
}

void PreencheConfiguraEventos(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  *proto_retornado->mutable_evento() = proto.evento();
  auto* modelo(new ModeloEvento(proto_retornado->mutable_evento(), gerador.tabela_lista_eventos));
  std::unique_ptr<QItemSelectionModel> delete_old(gerador.tabela_lista_eventos->selectionModel());
  gerador.tabela_lista_eventos->setModel(modelo);
  lambda_connect(gerador.botao_adicionar_evento, SIGNAL(clicked()), [&gerador, modelo] () {
    const int linha = modelo->rowCount();
    modelo->insertRows(linha, 1, modelo->index(linha, 0).parent());
    gerador.tabela_lista_eventos->selectRow(linha);
  });
  lambda_connect(gerador.botao_remover_evento, SIGNAL(clicked()), [&gerador, modelo] () {
    std::set<int, std::greater<int>> linhas;
    for (const QModelIndex& index : gerador.tabela_lista_eventos->selectionModel()->selectedIndexes()) {
      linhas.insert(index.row());
    }
    for (int linha : linhas) {
      modelo->removeRows(linha, 1, modelo->index(linha, 0).parent());
    }
  });
  auto* delegado = new TipoEfeitoDelegate(gerador.tabela_lista_eventos, modelo, gerador.tabela_lista_eventos);
  std::unique_ptr<QAbstractItemDelegate> delete_old_delegate(gerador.tabela_lista_eventos->itemDelegateForColumn(0));
  gerador.tabela_lista_eventos->setItemDelegateForColumn(0, delegado);
  delegado->deleteLater();
  gerador.tabela_lista_eventos->resizeColumnsToContents();
  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [this_, proto_retornado, &gerador] () {
    RecomputaDependencias(this_->tabelas(), proto_retornado);
    AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
  });
}

// Troca o delegate da tabela pelo passado. Detem a posse do objeto.
void TrocaDelegateColuna(unsigned int coluna, QAbstractItemDelegate* delegado, QTableView* tabela) {
  std::unique_ptr<QAbstractItemDelegate> delete_old_delegate(tabela->itemDelegateForColumn(coluna));
  tabela->setItemDelegateForColumn(coluna, delegado);
  delegado->deleteLater();
}

void PreencheConfiguraPericias(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto,
    ent::EntidadeProto* proto_retornado) {
  const ent::Tabelas& tabelas = this_->tabelas();
  auto* modelo(new ModeloPericias(tabelas, *proto_retornado, gerador.tabela_pericias));
  std::unique_ptr<QItemSelectionModel> delete_old(gerador.tabela_pericias->selectionModel());
  gerador.tabela_pericias->setModel(modelo);
  gerador.tabela_pericias->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  //gerador.tabela_pericias->resizeColumnsToContents();
  //lambda_connect(gerador.tabela_pericias, SIGNAL(resizeEvent(QResizeEvent* event)), [&gerador]() {
  //  gerador.tabela_pericias->resizeColumnsToContents();
  //});
  // Todas as colunas de mesmo tamanho... foi o melhor que consegui.
  for (int c = 0; c < gerador.tabela_pericias->horizontalHeader()->count(); ++c) {
    gerador.tabela_pericias->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);
  }
  gerador.tabela_pericias->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
  gerador.tabela_pericias->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [&tabelas, &gerador, proto_retornado, modelo] () {
    *proto_retornado->mutable_info_pericias() = modelo->Converte();
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
}

void PreencheConfiguraTalentos(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto,
    ent::EntidadeProto* proto_retornado) {
  const ent::Tabelas& tabelas = this_->tabelas();
  *proto_retornado->mutable_info_talentos() = proto.info_talentos();
  auto* modelo(new ModeloTalentos(tabelas, proto_retornado->mutable_info_talentos(), gerador.tabela_talentos));
  std::unique_ptr<QItemSelectionModel> delete_old(gerador.tabela_talentos->selectionModel());
  std::map<std::string, std::string> mapa;
  for (const auto& t : tabelas.todas().tabela_talentos().talentos()) {
    if (!t.monstro()) mapa[t.nome()] = t.id();
  }
  TrocaDelegateColuna(0, new MapaDelegate(mapa, modelo, gerador.tabela_talentos), gerador.tabela_talentos);
  TrocaDelegateColuna(1, new ComplementoTalentoDelegate(tabelas, modelo, gerador.tabela_talentos), gerador.tabela_talentos);

  gerador.tabela_talentos->setModel(modelo);
  lambda_connect(gerador.botao_adicionar_talento, SIGNAL(clicked()), [&gerador, modelo] () {
    const int linha = modelo->rowCount();
    modelo->insertRows(linha, 1, QModelIndex());
    gerador.tabela_talentos->selectRow(linha);
  });
  lambda_connect(gerador.botao_remover_talento, SIGNAL(clicked()), [&gerador, modelo] () {
    std::set<int, std::greater<int>> linhas;
    for (const QModelIndex& index : gerador.tabela_talentos->selectionModel()->selectedIndexes()) {
      linhas.insert(index.row());
    }
    for (int linha : linhas) {
      modelo->removeRows(linha, 1, QModelIndex());
    }
  });

  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [&tabelas, &gerador, proto_retornado, modelo] () {
    // TODO alterar o delegate do complemento.
    *proto_retornado->mutable_info_talentos() = modelo->Converte();
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
}

void PreencheConfiguraFeiticos(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto,
    ent::EntidadeProto* proto_retornado) {
  // Menu de contexto da arvore.
  gerador.arvore_feiticos->setContextMenuPolicy(Qt::CustomContextMenu);
  lambda_connect(gerador.botao_renovar_feiticos, SIGNAL(clicked()), [this_, &gerador, proto_retornado] () {
    ent::RenovaFeiticos(proto_retornado);
    gerador.arvore_feiticos->blockSignals(true);
    AtualizaUIFeiticos(this_->tabelas(), gerador, *proto_retornado);
    gerador.arvore_feiticos->blockSignals(true);
  });
  lambda_connect(
      gerador.arvore_feiticos, SIGNAL(customContextMenuRequested(const QPoint &)),
      [this_, &gerador, proto_retornado] (const QPoint& pos) {
    QTreeWidgetItem* item = gerador.arvore_feiticos->itemAt(pos);
    // Não há nada na árvore. Mostrar um menu de contexto adicionando itens?
    if (item == nullptr) return;
    QVariant data = item->data(0, Qt::UserRole);
    if (!data.isValid()) return;
    switch (data.toInt()) {
      case RAIZ_CONHECIDO: {
        QMenu menu("Menu Nivel", gerador.arvore_feiticos);
        QAction acao("Adicionar", &menu);
        lambda_connect(&acao, SIGNAL(triggered()), [this_, &gerador, proto_retornado, item] () {
          gerador.arvore_feiticos->blockSignals(true);
          std::string id_classe = item->data(TCOL_ID_CLASSE, Qt::UserRole).toString().toStdString();
          if (ClasseDeveConhecerFeitico(this_->tabelas(), id_classe)) return;
          int nivel = item->data(TCOL_NIVEL, Qt::UserRole).toInt();
          auto* fn = FeiticosNivel(id_classe, nivel, proto_retornado);
          fn->add_conhecidos()->set_nome("");
          AdicionaItemFeiticoConhecido(this_->tabelas(), gerador, "", "", id_classe, nivel, fn->conhecidos_size() - 1, item);
          item->setExpanded(true);
          if (ent::ClassePrecisaMemorizar(this_->tabelas(), id_classe)) {
            AtualizaCombosParaLancar(this_->tabelas(), gerador, id_classe, *proto_retornado);
          }
          gerador.arvore_feiticos->blockSignals(false);
        });
        menu.addAction(&acao);
        menu.exec(gerador.arvore_feiticos->mapToGlobal(pos));
      }
      break;
      case CONHECIDO: {
        QMenu menu("Menu Feitico", gerador.arvore_feiticos);
        QAction acao("Remover", &menu);
        lambda_connect(&acao, SIGNAL(triggered()), [this_, &gerador, proto_retornado, item] () {
          gerador.arvore_feiticos->blockSignals(true);
          std::string id_classe = item->data(TCOL_ID_CLASSE, Qt::UserRole).toString().toStdString();
          int nivel = item->data(TCOL_NIVEL, Qt::UserRole).toInt();
          int indice = item->data(TCOL_INDICE, Qt::UserRole).toInt();
          const auto& fc = ent::FeiticosClasse(id_classe, *proto_retornado);
          auto* fn = FeiticosNivel(id_classe, nivel, proto_retornado);
          if (indice < 0 || indice >= fn->conhecidos_size()) {
            gerador.arvore_feiticos->blockSignals(false);
            return;
          }
          fn->mutable_conhecidos()->DeleteSubrange(indice, 1);
          AtualizaFeiticosConhecidosNivel(this_->tabelas(), gerador, id_classe, nivel, *proto_retornado, item->parent());
          // aqui tem que corrigir todos para lancar que apontavam para feiticos do mesmo nivel:
          // 1- indice do removido: resetar para primeiro indice.
          // 2- indice > removido: diminuir o indice em 1.
          if (ent::ClassePrecisaMemorizar(this_->tabelas(), id_classe)) {
            for (int i = 0; i < fc.feiticos_por_nivel().size(); ++i) {
              auto* fn_correcao = FeiticosNivel(id_classe, i, proto_retornado);
              for (auto& pl : *fn_correcao->mutable_para_lancar()) {
                if (pl.nivel_conhecido() != nivel ||
                    !pl.has_indice_conhecido() || pl.indice_conhecido() < indice) continue;
                if (pl.indice_conhecido() == indice) {
                  pl.clear_indice_conhecido();
                } else {
                  pl.set_indice_conhecido(pl.indice_conhecido() - 1);
                }
              }
            }
            AtualizaCombosParaLancar(this_->tabelas(), gerador, id_classe, *proto_retornado);
          }
          gerador.arvore_feiticos->blockSignals(false);
        });
        menu.addAction(&acao);
        menu.exec(gerador.arvore_feiticos->mapToGlobal(pos));
      }
      break;
      default: ;
    }
  });
  lambda_connect(gerador.arvore_feiticos, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
      [this_, &gerador, proto_retornado] (QTreeWidgetItem* item, int column) {
    std::string id_classe = item->data(TCOL_ID_CLASSE, Qt::UserRole).toString().toStdString();
    int nivel = item->data(TCOL_NIVEL, Qt::UserRole).toInt();
    int indice = item->data(TCOL_INDICE, Qt::UserRole).toInt();
    auto* f = FeiticosNivel(id_classe, nivel, proto_retornado);
    if (item->data(0, Qt::UserRole).toInt() == CONHECIDO) {
      if (indice < 0 || indice >= f->conhecidos_size()) return;
      f->mutable_conhecidos(indice)->set_nome(item->data(TCOL_NOME_FEITICO, Qt::UserRole).toString().toStdString());
      if (!item->data(TCOL_ID_FEITICO, Qt::UserRole).toString().isEmpty()) {
        f->mutable_conhecidos(indice)->set_id(item->data(TCOL_ID_FEITICO, Qt::UserRole).toString().toStdString());
      } else {
        f->mutable_conhecidos(indice)->clear_id();
      }
      if (ent::ClassePrecisaMemorizar(this_->tabelas(), id_classe)) {
        AtualizaCombosParaLancar(this_->tabelas(), gerador, id_classe, *proto_retornado);
      }
    }
    if (item->data(0, Qt::UserRole).toInt() == PARA_LANCAR) {
      if (indice < 0 || indice >= f->para_lancar_size()) return;
      f->mutable_para_lancar(indice)->set_nivel_conhecido(
          item->data(TCOL_NIVEL_CONHECIDO, Qt::UserRole).toInt());
      f->mutable_para_lancar(indice)->set_indice_conhecido(
          item->data(TCOL_INDICE_CONHECIDO, Qt::UserRole).toInt());
      f->mutable_para_lancar(indice)->set_usado(
          item->data(TCOL_USADO, Qt::UserRole).toBool());
      VLOG(1) << "atualizando feitico para lancar nivel " << nivel << ", indice: " << indice
              << ": " << f->para_lancar(indice).ShortDebugString();
    }
  });
  AtualizaUIFeiticos(this_->tabelas(), gerador, proto);
}

// Formas alternativas.
void PreencheConfiguraFormasAlternativas(
    Visualizador3d* this_, QDialog* dialogo,
    ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado,
    bool forma_primaria = true) {
  AtualizaUIFormasAlternativas(gerador, proto);
  lambda_connect(gerador.botao_adicionar_forma_alternativa, SIGNAL(clicked()),
      [this_, dialogo, &gerador, &proto, proto_retornado] () {
    ntf::Notificacao n;
    std::unique_ptr<ent::EntidadeProto> proto_forma = this_->AbreDialogoTipoEntidade(n, false, dialogo);
    if (proto_forma == nullptr) return;
    ent::AdicionaFormaAlternativa(*proto_forma, proto_retornado);
    AtualizaUIFormasAlternativas(gerador, *proto_retornado);
  });
  lambda_connect(gerador.botao_remover_forma_alternativa, SIGNAL(clicked()), [&gerador, proto_retornado] () {
    ent::RemoveFormaAlternativa(gerador.lista_formas_alternativas->currentRow(), proto_retornado);
    gerador.lista_formas_alternativas->setCurrentRow(-1);
    AtualizaUIFormasAlternativas(gerador, *proto_retornado);
  });
  // Clique duplo em item.
  lambda_connect(gerador.lista_formas_alternativas, SIGNAL(doubleClicked(const QModelIndex &)),
      [this_, &proto, dialogo, &gerador, proto_retornado] () {
    const int row = gerador.lista_formas_alternativas->currentRow();
    if (row < 0 || row >= proto_retornado->formas_alternativas_size() || row == proto.forma_alternativa_corrente()) return;
    ntf::Notificacao n;
    *n.mutable_entidade() = proto_retornado->formas_alternativas(row);
    std::unique_ptr<ent::EntidadeProto> proto_forma = this_->AbreDialogoTipoEntidade(n, false, dialogo);
    if (proto_forma == nullptr) return;
    *proto_retornado->mutable_formas_alternativas(row) = ProtoFormaAlternativa(*proto_forma);
    AtualizaUIFormasAlternativas(gerador, *proto_retornado);
  });
}

void ConfiguraListaItensMagicos(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, TipoItem tipo,
    QListWidget* lista, QPushButton* botao_usar, QPushButton* botao_adicionar, QPushButton* botao_remover,
    ent::EntidadeProto* proto_retornado) {
  // Delegado.
  std::unique_ptr<QAbstractItemDelegate> delete_old(lista->itemDelegate());
  auto* delegado = new ItemMagicoDelegate(tabelas, tipo, lista, proto_retornado);
  lista->setItemDelegate(delegado);
  delegado->deleteLater();
  // Sinal de alteracao.
  lambda_connect(lista, SIGNAL(currentRowChanged(int)), [tipo, lista, botao_usar, &tabelas, proto_retornado] () {
    int row = lista->currentRow();
    if (row < 0 || row >= lista->count() || row >= ItensPersonagem(tipo, *proto_retornado).size()) {
      botao_usar->setText("Vestir");
    } else {
      botao_usar->setText(ItensPersonagem(tipo, *proto_retornado).Get(row).em_uso() ? "Tirar" : "Vestir");
    }
  });
  // Botao de usar.
  lambda_connect(botao_usar, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado] () {
    const int indice = lista->currentRow();
    auto* itens_personagem = ItensPersonagemMutavel(tipo, proto_retornado);
    if (indice < 0 || indice >= itens_personagem->size()) {
      return;
    }
    auto* item = itens_personagem->Mutable(indice);
    bool em_uso_antes = item->em_uso();
    if (!em_uso_antes) {
      int num_em_uso = std::count_if(
        itens_personagem->begin(), itens_personagem->end(), [] (const ent::ItemMagicoProto& item) {
           return item.em_uso();
        });
      if (num_em_uso >= MaximoEmUso(tipo)) {
        QMessageBox::information(
            lista, QObject::tr("Informação"),
            QObject::tr(google::protobuf::StringPrintf("Apenas %d item(s) permitido(s).", MaximoEmUso(tipo)).c_str()));
        return;
      }
      const auto& item_tabela = ItemTabela(tabelas, tipo, item->id());
      for (int id_unico : AdicionaEventoItemMagicoContinuo(proto_retornado->evento(), item_tabela, proto_retornado)) {
        item->add_ids_efeitos(id_unico);
      }
      item->set_em_uso(true);
    } else {
      for (uint32_t id_unico : item->ids_efeitos()) {
        ent::ExpiraEventoItemMagico(id_unico, proto_retornado);
      }
      item->clear_ids_efeitos();
      item->set_em_uso(false);
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(botao_adicionar, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado] () {
    auto* itens = ItensPersonagemMutavel(tipo, proto_retornado);
    itens->Add();
    AtualizaUITesouro(tabelas, gerador, *proto_retornado);
    lista->setCurrentRow(itens->size() - 1);
  });
  lambda_connect(botao_remover, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado] () {
    const int indice = lista->currentRow();
    auto* itens = ItensPersonagemMutavel(tipo, proto_retornado);
    if (indice < 0 || indice >= itens->size()) {
      return;
    }
    auto* item = itens->Mutable(indice);
    for (uint32_t id_unico : item->ids_efeitos()) {
      ent::ExpiraEventoItemMagico(id_unico, proto_retornado);
    }
    if (indice >= 0 && indice < itens->size()) {
      itens->DeleteSubrange(indice, 1);
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
    lista->setCurrentRow(indice);
  });
}

void PreencheConfiguraTesouro(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  const auto& tabelas = this_->tabelas();

  // Pocoes.
  {
    std::unique_ptr<QAbstractItemDelegate> delete_old(gerador.lista_pocoes->itemDelegate());
    auto* delegado = new PocaoDelegate(tabelas, gerador.lista_pocoes, proto_retornado);
    gerador.lista_pocoes->setItemDelegate(delegado);
    delegado->deleteLater();

    lambda_connect(gerador.botao_adicionar_pocao, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
      /*auto* pocao = */proto_retornado->mutable_tesouro()->add_pocoes();
      // Para aparecer pocao vazia.
      //pocao->set_id("forca_touro");
      AtualizaUITesouro(tabelas, gerador, *proto_retornado);
      gerador.lista_pocoes->setCurrentRow(proto_retornado->tesouro().pocoes_size() - 1);
    });
    lambda_connect(gerador.botao_remover_pocao, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
      const int indice = gerador.lista_pocoes->currentRow();
      if (indice >= 0 && indice < proto_retornado->tesouro().pocoes_size()) {
        proto_retornado->mutable_tesouro()->mutable_pocoes()->DeleteSubrange(indice, 1);
      }
      AtualizaUITesouro(tabelas, gerador, *proto_retornado);
      gerador.lista_pocoes->setCurrentRow(indice);
    });
  }

  // Aneis.
  ConfiguraListaItensMagicos(
      tabelas, gerador, TipoItem::TIPO_ANEL,
      gerador.lista_aneis, gerador.botao_usar_anel, gerador.botao_adicionar_anel, gerador.botao_remover_anel,
      proto_retornado);
  // Luvas.
  ConfiguraListaItensMagicos(
      tabelas, gerador, TipoItem::TIPO_LUVAS,
      gerador.lista_luvas, gerador.botao_usar_luvas, gerador.botao_adicionar_luvas, gerador.botao_remover_luvas,
      proto_retornado);
  // Mantos.
  ConfiguraListaItensMagicos(
      tabelas, gerador, TipoItem::TIPO_MANTO,
      gerador.lista_mantos, gerador.botao_usar_manto, gerador.botao_adicionar_manto, gerador.botao_remover_manto,
      proto_retornado);
  ConfiguraListaItensMagicos(
      tabelas, gerador, TipoItem::TIPO_BRACADEIRAS,
      gerador.lista_bracadeiras, gerador.botao_usar_bracadeiras, gerador.botao_adicionar_bracadeiras, gerador.botao_remover_bracadeiras,
      proto_retornado);

  AtualizaUITesouro(tabelas, gerador, proto);
  gerador.lista_tesouro->setPlainText((proto.tesouro().tesouro().c_str()));
}

void PreencheConfiguraAtributos(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  // Atualiza os campos.
  auto* atrib = proto_retornado->mutable_atributos();
  std::vector<std::tuple<QPushButton*, QSpinBox*, ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.botao_bonus_forca,        gerador.spin_forca,        atrib->mutable_forca()),
    std::make_tuple(gerador.botao_bonus_destreza,     gerador.spin_destreza,     atrib->mutable_destreza()),
    std::make_tuple(gerador.botao_bonus_constituicao, gerador.spin_constituicao, atrib->mutable_constituicao()),
    std::make_tuple(gerador.botao_bonus_inteligencia, gerador.spin_inteligencia, atrib->mutable_inteligencia()),
    std::make_tuple(gerador.botao_bonus_sabedoria,    gerador.spin_sabedoria,    atrib->mutable_sabedoria()),
    std::make_tuple(gerador.botao_bonus_carisma,      gerador.spin_carisma,      atrib->mutable_carisma()),
  };
  for (auto& t : tuplas) {
    QPushButton* botao; QSpinBox* spin; ent::Bonus* bonus;
    std::tie(botao, spin, bonus) = t;
    // bb tem que ser capturado por valor, porque a variavel sai de escopo no loop.
    lambda_connect(botao, SIGNAL(clicked()), [this_, bonus, proto_retornado, &gerador] () {
      AbreDialogoBonus(this_, bonus);
      ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
      AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
    });
    lambda_connect(spin, SIGNAL(valueChanged(int)), [this_, &gerador, spin, bonus, proto_retornado] () {
      ent::AtribuiBonus(spin->value(), ent::TB_BASE, "base", bonus);
      ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
      AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
    });
  }
  AtualizaUIAtributos(this_->tabelas(), gerador, proto);
}

void PreencheConfiguraDadosDefesa(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  AtualizaUIAtaquesDefesa(this_->tabelas(), gerador, proto);
  // Imune critico.
  gerador.checkbox_imune_critico->setCheckState(proto.dados_defesa().imune_critico() ? Qt::Checked : Qt::Unchecked);
  gerador.spin_rm->setValue(proto.dados_defesa().resistencia_magia());
  lambda_connect(gerador.spin_rm, SIGNAL(valueChanged(int)), [&gerador, proto_retornado]() {
    if (gerador.spin_rm->value() > 0) {
      proto_retornado->mutable_dados_defesa()->set_resistencia_magia(gerador.spin_rm->value());
    } else {
      proto_retornado->mutable_dados_defesa()->clear_resistencia_magia();
    }
  });

  auto* mca = proto_retornado->mutable_dados_defesa()->mutable_ca();
  const ent::Tabelas& tabelas = this_->tabelas();
  lambda_connect(gerador.combo_armadura, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, mca] () {
    QComboBox* combo = gerador.combo_armadura;
    std::string id = combo->itemData(combo->currentIndex()).toString().toStdString();
    proto_retornado->mutable_dados_defesa()->set_id_armadura(id);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtributos(tabelas, gerador, *proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.combo_escudo, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, mca] () {
    QComboBox* combo = gerador.combo_escudo;
    std::string id = combo->itemData(combo->currentIndex()).toString().toStdString();
    proto_retornado->mutable_dados_defesa()->set_id_escudo(id);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtributos(tabelas, gerador, *proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.spin_ca_armadura_melhoria, SIGNAL(valueChanged(int)), [tabelas, &gerador, proto_retornado, mca] () {
    ent::AtribuiBonus(gerador.spin_ca_armadura_melhoria->value(), ent::TB_ARMADURA_MELHORIA, "armadura", mca);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.spin_ca_escudo_melhoria, SIGNAL(valueChanged(int)), [tabelas, &gerador, proto_retornado, mca] () {
    ent::AtribuiBonus(gerador.spin_ca_escudo_melhoria->value(), ent::TB_ESCUDO_MELHORIA, "escudo", mca);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.botao_bonus_ca, SIGNAL(clicked()), [tabelas, this_, &gerador, proto_retornado, mca] () {
    AbreDialogoBonus(this_, mca);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
}

// Preenche o combo de arma de acordo com o tipo de ataque selecionado.
void ConfiguraComboArma(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto* combo_arma = gerador.combo_arma;
  std::string maior_string;
  for (const auto& arma : tabelas.todas().tabela_armas().armas()) {
    if (arma.nome().length() > maior_string.length()) maior_string = arma.nome();
  }
  gerador.combo_arma->addItem(QString::fromUtf8(maior_string.c_str()), QVariant(""));
  ExpandeComboBox(gerador.combo_arma);
  gerador.combo_arma->clear();

  lambda_connect(combo_arma, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_arma] () {
    const int index_combo = gerador.combo_arma->currentIndex();
    std::string id_arma = index_combo < 0 ? "nenhuma" : combo_arma->itemData(index_combo).toString().toStdString();

    const int index_lista = gerador.lista_ataques->currentRow();
    if (index_lista < 0 || index_lista >= proto_retornado->dados_ataque_size()) return;
    auto* da = proto_retornado->mutable_dados_ataque(index_lista);
    if (id_arma == "nenhuma") {
      da->clear_id_arma();
    } else {
      da->set_id_arma(id_arma);
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    gerador.linha_dano->setEnabled(!tabelas.ArmaOuFeitico(id_arma).has_dano());
    gerador.combo_material_arma->setEnabled(id_arma != "nenhuma");
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
}

ent::DescritorAtaque IndiceParaMaterialArma(int indice) {
  switch (indice) {
    case 1: return ent::DESC_ADAMANTE;
    case 2: return ent::DESC_FERRO_FRIO;
    case 3: return ent::DESC_MADEIRA_NEGRA;
    case 4: return ent::DESC_MITRAL;
    case 5: return ent::DESC_PRATA_ALQUIMICA;
  }
  return ent::DESC_NENHUM;
}

// Preenche o combo de material da arma.
void ConfiguraComboMaterial(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto* combo_material = gerador.combo_material_arma;
  lambda_connect(combo_material, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_material] () {
    const int index_lista = gerador.lista_ataques->currentRow();
    if (index_lista < 0 || index_lista >= proto_retornado->dados_ataque_size()) return;
    auto* da = proto_retornado->mutable_dados_ataque(index_lista);
    ent::DescritorAtaque desc = IndiceParaMaterialArma(combo_material->currentIndex());
    if (desc == ent::DESC_NENHUM) {
      da->clear_material_arma();
    } else {
      da->set_material_arma(desc);
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUIAtaquesDefesa(tabelas, gerador, *proto_retornado);
  });
  ExpandeComboBox(combo_material);
}

void PreencheConfiguraComboTipoAtaque(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, std::function<void()> EditaAtualizaUIAtaque, ent::EntidadeProto* proto_retornado) {
  const ent::Acoes& acoes = tabelas.TodasAcoes();
  std::set<std::string> tipos_acoes;
  for (const auto& acao : acoes.acao()) {
    tipos_acoes.insert(acao.id());
  }
  for (const auto& id : tipos_acoes) {
    gerador.combo_tipo_ataque->addItem((id.c_str()), QVariant(id.c_str()));
  }
  ExpandeComboBox(gerador.combo_tipo_ataque);
  lambda_connect(gerador.combo_tipo_ataque, SIGNAL(currentIndexChanged(int)),
      [tabelas, &gerador, EditaAtualizaUIAtaque, proto_retornado]() {
    const auto& tipo_ataque = CurrentData(gerador.combo_tipo_ataque).toString().toStdString();
    gerador.combo_arma->setEnabled(
        tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância" || tipo_ataque == "Projétil de Área" ||
        tipo_ataque == "Feitiço de Mago" || tipo_ataque == "Feitiço de Clérigo" || tipo_ataque == "Feitiço de Druida");
    gerador.combo_material_arma->setEnabled(
        tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância");
    EditaAtualizaUIAtaque();
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size()) {
      // Se nao for edicao, tem que atualizar a UI por causa do combo de armas.
      AtualizaUIAtaque(tabelas, gerador, *proto_retornado);
    }
  });
}

void PreencheConfiguraDadosAtaque(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& ent, ent::EntidadeProto* proto_retornado) {
  const ent::Tabelas& tabelas = this_->tabelas();
  auto EditaAtualizaUIAtaque = [this_, &tabelas, &gerador, proto_retornado] () {
    int indice_antes = gerador.lista_ataques->currentRow();
    if (indice_antes < 0 || indice_antes >= proto_retornado->dados_ataque().size()) {
      // Vale apenas para edicao.
      return;
    }
    AdicionaOuAtualizaAtaqueEntidade(tabelas, gerador, proto_retornado);
    if (indice_antes < proto_retornado->dados_ataque().size()) {
      gerador.lista_ataques->setCurrentRow(indice_antes);
    } else {
      gerador.lista_ataques->setCurrentRow(-1);
    }
  };

  gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));
  PreencheConfiguraComboTipoAtaque(tabelas, gerador, EditaAtualizaUIAtaque, proto_retornado);
  ConfiguraComboArma(tabelas, gerador, proto_retornado);
  ConfiguraComboMaterial(tabelas, gerador, proto_retornado);

  AtualizaUIAtaque(tabelas, gerador, *proto_retornado);

  // Furtivo
  gerador.linha_furtivo->setText((ent.dados_ataque_global().dano_furtivo().c_str()));

  ExpandeComboBox(gerador.combo_empunhadura);

  lambda_connect(gerador.lista_ataques, SIGNAL(currentRowChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    AtualizaUIAtaque(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.botao_clonar_ataque, SIGNAL(clicked()), [this_, &tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size()) {
      AdicionaOuAtualizaAtaqueEntidade(tabelas, gerador, proto_retornado);
    } else {
      *proto_retornado->mutable_dados_ataque()->Add() = proto_retornado->dados_ataque(indice);
    }
    AtualizaUIAtaque(tabelas, gerador, *proto_retornado);
    gerador.lista_ataques->setCurrentRow(proto_retornado->dados_ataque().size() - 1);
  });

  lambda_connect(gerador.botao_ataque_cima, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice <= 0 || indice >= proto_retornado->dados_ataque().size() ||
        proto_retornado->dados_ataque().size() <= 1 || indice >= proto_retornado->dados_ataque().size()) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice - 1));
    gerador.lista_ataques->setCurrentRow(indice - 1);
  });
  lambda_connect(gerador.botao_ataque_baixo, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size() - 1 ||
        proto_retornado->dados_ataque().size() <= 1) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice + 1));
    gerador.lista_ataques->setCurrentRow(indice + 1);
  });

  lambda_connect(gerador.botao_remover_ataque, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    if (gerador.lista_ataques->currentRow() == -1 || gerador.lista_ataques->currentRow() >= proto_retornado->dados_ataque().size()) {
      return;
    }
    proto_retornado->mutable_dados_ataque()->DeleteSubrange(gerador.lista_ataques->currentRow(), 1);
    gerador.lista_ataques->setCurrentRow(-1);
  });
  // Ao adicionar aqui, adicione nos sinais bloqueados tb (blockSignals). Exceto para textEdited, que nao dispara sinal programaticamente.
  lambda_connect(gerador.linha_grupo_ataque, SIGNAL(textEdited(const QString&)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.linha_rotulo_ataque, SIGNAL(textEdited(const QString&)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.linha_dano, SIGNAL(editingFinished()), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );  // nao pode refrescar no meio pois tem processamento da string.
  lambda_connect(gerador.spin_bonus_magico, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_municao, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_ordem_ataque, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_op, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.combo_empunhadura, SIGNAL(currentIndexChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_incrementos, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_alcance_quad, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.botao_bonus_ataque, SIGNAL(clicked()), [this_, EditaAtualizaUIAtaque, &gerador, proto_retornado] {
    if (gerador.lista_ataques->currentRow() == -1 || gerador.lista_ataques->currentRow() >= proto_retornado->dados_ataque().size()) {
      return;
    }
    AbreDialogoBonus(this_, proto_retornado->mutable_dados_ataque(gerador.lista_ataques->currentRow())->mutable_bonus_ataque());
    EditaAtualizaUIAtaque();
  });
  lambda_connect(gerador.botao_bonus_dano, SIGNAL(clicked()), [this_, EditaAtualizaUIAtaque, &gerador, proto_retornado] {
    if (gerador.lista_ataques->currentRow() == -1 || gerador.lista_ataques->currentRow() >= proto_retornado->dados_ataque().size()) {
      return;
    }
    AbreDialogoBonus(this_, proto_retornado->mutable_dados_ataque(gerador.lista_ataques->currentRow())->mutable_bonus_dano());
    EditaAtualizaUIAtaque();
  });
}

void PreencheComboSalvacoesFortes(QComboBox* combo) {
  combo->addItem("Fortitude");
  combo->addItem("Reflexo");
  combo->addItem("Vontade");
  combo->addItem("Fortitude e Reflexo");
  combo->addItem("Fortitude e Vontade");
  combo->addItem("Reflexo e Vontade");
  combo->addItem("Todas");
  ExpandeComboBox(combo);
}

std::vector<ent::TipoSalvacao> ComboParaSalvacoesFortes(const QComboBox* combo) {
  switch (combo->currentIndex()) {
    case 0: return { ent::TS_FORTITUDE };
    case 1: return { ent::TS_REFLEXO };
    case 2: return { ent::TS_VONTADE };
    case 3: return { ent::TS_FORTITUDE, ent::TS_REFLEXO };
    case 4: return { ent::TS_FORTITUDE, ent::TS_VONTADE };
    case 5: return { ent::TS_REFLEXO, ent::TS_VONTADE };
    case 6: return { ent::TS_FORTITUDE, ent::TS_REFLEXO, ent::TS_VONTADE };
    default:
      LOG(INFO) << "valor de combo invalido: " << combo->currentIndex();
      return { ent::TS_FORTITUDE };
  }
}

// Chamado tb durante a finalizacao, por causa do problema de apertar enter e fechar a janela. Nao atualiza a UI.
void AdicionaOuEditaNivel(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  const int indice = gerador.lista_niveis->currentRow();
  if (gerador.linha_classe->text().isEmpty()) {
    return;
  }
  ent::InfoClasse* ic = (indice < 0 || indice >= proto_retornado->info_classes().size())
      ? proto_retornado->add_info_classes() : proto_retornado->mutable_info_classes(indice);
  ic->set_id(gerador.linha_classe->text().toStdString());
  ic->set_nivel(gerador.spin_nivel_classe->value());
  ic->set_nivel_conjurador(gerador.spin_nivel_conjurador->value());
  ic->set_bba(gerador.spin_bba->value());
  ic->set_atributo_conjuracao(static_cast<ent::TipoAtributo>(gerador.combo_mod_conjuracao->currentIndex()));
  ic->clear_salvacoes_fortes();
  for (auto ts : ComboParaSalvacoesFortes(gerador.combo_salvacoes_fortes)) {
    ic->add_salvacoes_fortes(ts);
  }
  ent::RecomputaDependencias(tabelas, proto_retornado);
  AtualizaUI(tabelas, gerador, *proto_retornado);
}

void LimpaCamposClasse(ifg::qt::Ui::DialogoEntidade& gerador) {
  gerador.combo_classe->setCurrentIndex(-1);
  gerador.linha_classe->clear();
  gerador.spin_nivel_classe->setValue(1);
  gerador.spin_nivel_conjurador->clear();
  gerador.spin_bba->clear();
  gerador.label_mod_conjuracao->setText("0");
  gerador.combo_mod_conjuracao->setCurrentIndex(0);
  gerador.botao_remover_nivel->setEnabled(false);
}

void PreencheConfiguraComboClasse(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto* combo = gerador.combo_classe;
  combo->addItem(combo->tr("Outro"), "outro");
  for (const auto& ic : tabelas.todas().tabela_classes().info_classes()) {
    combo->addItem((ic.nome().c_str()), ic.id().c_str());
  }
  ExpandeComboBox(combo);
  ExpandeComboBox(gerador.combo_mod_conjuracao);
  lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, &tabelas, &gerador, proto_retornado] () {
    std::vector<QObject*> objs = {
      gerador.spin_nivel_classe, gerador.spin_nivel_conjurador, gerador.linha_classe, gerador.spin_bba,
      gerador.combo_mod_conjuracao, gerador.lista_niveis, gerador.combo_salvacoes_fortes, gerador.combo_classe
    };
    const auto& classe_tabelada = tabelas.Classe(combo->itemData(combo->currentIndex()).toString().toStdString());
    const int indice = gerador.lista_niveis->currentRow();
    if (indice >= 0 && indice < proto_retornado->info_classes_size()) {
      if (classe_tabelada.has_nome()) {
        proto_retornado->mutable_info_classes(indice)->set_id(
          combo->itemData(combo->currentIndex()).toString().toStdString());
      } else {
        proto_retornado->mutable_info_classes(indice)->clear_id();
      }
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
}

void PreencheConfiguraClassesNiveis(Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  const auto& tabelas = this_->tabelas();
  // Ao mudar a selecao, atualiza os controles.
  lambda_connect(gerador.lista_niveis, SIGNAL(currentRowChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    AtualizaUIClassesNiveis(tabelas, gerador, *proto_retornado);
  });

  lambda_connect(gerador.spin_niveis_negativos, SIGNAL(valueChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    if (gerador.spin_niveis_negativos->value() > 0) {
      proto_retornado->set_niveis_negativos(gerador.spin_niveis_negativos->value());
    } else {
      proto_retornado->clear_niveis_negativos();
    }
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });

  // Adiciona um nivel ao personagem ao clicar no botao de adicionar.
  lambda_connect(gerador.botao_adicionar_nivel, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    gerador.lista_niveis->setCurrentRow(-1);
    AdicionaOuEditaNivel(tabelas, gerador, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
    // Deixa deselecionado e zera campos, mais intuitivo.
    LimpaCamposClasse(gerador);
  });

  PreencheConfiguraComboClasse(tabelas, gerador, proto_retornado);

  PreencheComboSalvacoesFortes(gerador.combo_salvacoes_fortes);
  lambda_connect(gerador.combo_salvacoes_fortes, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    if (gerador.lista_niveis->currentRow() == -1 ||
        gerador.lista_niveis->currentRow() >= proto_retornado->info_classes().size()) {
      return;
    }
    auto* ic = proto_retornado->mutable_info_classes(gerador.lista_niveis->currentRow());
    ic->clear_salvacoes_fortes();
    for (auto ts : ComboParaSalvacoesFortes(gerador.combo_salvacoes_fortes)) {
      ic->add_salvacoes_fortes(ts);
    }
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });

  // Remove o nivel selecionado.
  lambda_connect(gerador.botao_remover_nivel, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    if (gerador.lista_niveis->currentRow() == -1 ||
        gerador.lista_niveis->currentRow() >= proto_retornado->info_classes().size()) {
      return;
    }
    proto_retornado->mutable_info_classes()->DeleteSubrange(gerador.lista_niveis->currentRow(), 1);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
    LimpaCamposClasse(gerador);
  });

  // Responde uma edicao da UI se houver selecao. caso contrario nada sera feito.
  auto EditaAtualizaNiveis = [&tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_niveis->currentRow();
    if (indice < 0 || indice >= proto_retornado->info_classes().size()) {
      return;
    }
    AdicionaOuEditaNivel(tabelas, gerador, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  };
  // Ao adicionar aqui, adicione nos sinais bloqueados tb (blockSignals).
  lambda_connect(gerador.linha_classe, SIGNAL(textEdited(const QString&)), [EditaAtualizaNiveis]() { EditaAtualizaNiveis(); } );
  lambda_connect(gerador.spin_nivel_classe, SIGNAL(valueChanged(int)), [EditaAtualizaNiveis]() { EditaAtualizaNiveis(); } );
  lambda_connect(gerador.spin_nivel_conjurador, SIGNAL(valueChanged(int)), [EditaAtualizaNiveis]() { EditaAtualizaNiveis(); } );
  lambda_connect(gerador.spin_bba, SIGNAL(valueChanged(int)), [EditaAtualizaNiveis]() { EditaAtualizaNiveis(); } );
  lambda_connect(gerador.combo_mod_conjuracao, SIGNAL(currentIndexChanged(int)), [EditaAtualizaNiveis]() { EditaAtualizaNiveis(); } );

  AtualizaUIClassesNiveis(tabelas, gerador, *proto_retornado);
}

void PreencheConfiguraSalvacoes(ifg::qt::Visualizador3d* pai, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto) {
  auto* dd = proto->mutable_dados_defesa();
  AtualizaUISalvacoes(gerador, *proto);
  std::vector<std::tuple<QPushButton*, ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.botao_bonus_salvacao_fortitude, dd->mutable_salvacao_fortitude()),
    std::make_tuple(gerador.botao_bonus_salvacao_reflexo, dd->mutable_salvacao_reflexo()),
    std::make_tuple(gerador.botao_bonus_salvacao_vontade, dd->mutable_salvacao_vontade()),
  };
  for (const auto& t : tuplas) {
    QPushButton* botao; ent::Bonus* bonus;
    std::tie(botao, bonus) = t;
    lambda_connect(botao, SIGNAL(clicked()), [pai, bonus, &gerador, proto] () {
      AbreDialogoBonus(pai, bonus);
      AtualizaUISalvacoes(gerador, *proto);
    });
  }
}

void PreencheConfiguraDadosIniciativa(
    ifg::qt::Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  lambda_connect(gerador.checkbox_iniciativa, SIGNAL(stateChanged(int)), [&gerador] () {
    gerador.spin_iniciativa->setEnabled(gerador.checkbox_iniciativa->checkState() == Qt::Checked);
  });
  lambda_connect(gerador.spin_iniciativa, SIGNAL(valueChanged(int)), [&gerador] () {
    gerador.checkbox_iniciativa->setCheckState(Qt::Checked);
  });
  lambda_connect(gerador.botao_bonus_iniciativa, SIGNAL(clicked()), [this_, &gerador, proto_retornado] () {
    auto* bonus_iniciativa = proto_retornado->mutable_bonus_iniciativa();
    AbreDialogoBonus(this_, bonus_iniciativa);
    gerador.botao_bonus_iniciativa->setText(NumeroSinalizado(ent::BonusTotal(*bonus_iniciativa)));
  });
  AtualizaUIIniciativa(this_->tabelas(), gerador, proto);
}

}  // namespace

std::unique_ptr<ent::EntidadeProto> Visualizador3d::AbreDialogoTipoEntidade(
    const ntf::Notificacao& notificacao, bool forma_primaria, QWidget* pai) {
  const auto& entidade = notificacao.entidade();
  std::unique_ptr<ent::EntidadeProto> delete_proto_retornado(new ent::EntidadeProto(entidade));
  auto* proto_retornado = delete_proto_retornado.get();
  proto_retornado->set_id(entidade.id());
  ifg::qt::Ui::DialogoEntidade gerador;
  auto* dialogo = new QDialog(pai == nullptr ? this : pai);
  gerador.setupUi(dialogo);
  // ID.
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(entidade.id()));
  // Rotulo.
  QString rotulo_str;
  gerador.campo_rotulo->setText((entidade.rotulo().c_str()));
  // Rotulos especiais.
  std::string rotulos_especiais;
  for (const std::string& rotulo_especial : entidade.rotulo_especial()) {
    rotulos_especiais += rotulo_especial + "\n";
  }
  gerador.lista_rotulos->appendPlainText((rotulos_especiais.c_str()));

  // Eventos.
  PreencheConfiguraEventos(this, gerador, entidade, proto_retornado);

  // Formas alternativas.
  PreencheConfiguraFormasAlternativas(this, dialogo, gerador, entidade, proto_retornado);

  // Pericias e Talentos.
  PreencheConfiguraPericias(this, gerador, entidade, proto_retornado);
  PreencheConfiguraTalentos(this, gerador, entidade, proto_retornado);

  // Feiticos.
  PreencheConfiguraFeiticos(this, gerador, entidade, proto_retornado);

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
  gerador.slider_tamanho->setSliderPosition(ent::BonusIndividualTotal(ent::TB_BASE, entidade.bonus_tamanho()));
  gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
  lambda_connect(gerador.slider_tamanho, SIGNAL(valueChanged(int)), [this, &gerador, proto_retornado] () {
    ent::AtribuiBonus(gerador.slider_tamanho->sliderPosition(), ent::TB_BASE, "base", proto_retornado->mutable_bonus_tamanho());
    gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
    ent::RecomputaDependencias(tabelas(), proto_retornado);
    AtualizaUI(tabelas(), gerador, *proto_retornado);
  });
  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(entidade.cor());
  gerador.botao_cor->setStyleSheet(CorParaEstilo(entidade.cor()));
  gerador.checkbox_cor->setCheckState(entidade.has_cor() ? Qt::Checked : Qt::Unchecked);
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [this, dialogo, &gerador, &ent_cor] {
    QColor cor = QColorDialog::getColor(ProtoParaCor(ent_cor.cor()), dialogo, QObject::tr("Cor do objeto"));
    if (!cor.isValid()) {
      return;
    }
    gerador.checkbox_cor->setCheckState(Qt::Checked);
    gerador.botao_cor->setStyleSheet(CorParaEstilo(cor));
    ent_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
  });
  gerador.slider_alfa->setValue(static_cast<int>(ent_cor.cor().a() * 100.0f));

  // Cor da luz.
  ent::EntidadeProto luz_cor;
  if (entidade.has_luz()) {
    luz_cor.mutable_cor()->CopyFrom(entidade.luz().cor());
    gerador.botao_luz->setStyleSheet(CorParaEstilo(entidade.luz().cor()));
    gerador.spin_raio_quad->setValue(ent::METROS_PARA_QUADRADOS * (entidade.luz().has_raio_m() ? entidade.luz().raio_m() : 6.0f));
  } else {
    ent::Cor branco;
    branco.set_r(1.0f);
    branco.set_g(1.0f);
    branco.set_b(1.0f);
    luz_cor.mutable_cor()->CopyFrom(branco);
    gerador.botao_luz->setStyleSheet(CorParaEstilo(branco));
    gerador.spin_raio_quad->setValue(0.0f);
  }
  lambda_connect(gerador.botao_luz, SIGNAL(clicked()), [this, dialogo, &gerador, &luz_cor] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(luz_cor.cor()), dialogo, QObject::tr("Cor da luz"));
    if (!cor.isValid()) {
      return;
    }
    luz_cor.mutable_cor()->CopyFrom(CorParaProto(cor));
    gerador.botao_luz->setStyleSheet(CorParaEstilo(cor));
    if (gerador.spin_raio_quad->value() == 0.0) {
      gerador.spin_raio_quad->setValue(ent::METROS_PARA_QUADRADOS * 6.0f);
    }
  });
  // Textura do objeto.
  PreencheComboTextura(entidade.info_textura().id(), notificacao.tabuleiro().id_cliente(), ent::FiltroTexturaEntidade, gerador.combo_textura);
  gerador.spin_tex_largura->setValue(entidade.info_textura().largura());
  gerador.spin_tex_altura->setValue(entidade.info_textura().altura());
  gerador.spin_tex_trans_x->setValue(entidade.info_textura().translacao_x());
  gerador.spin_tex_trans_y->setValue(entidade.info_textura().translacao_y());

  // Modelo 3d.
  PreencheComboModelo3d(entidade.modelo_3d().id(), gerador.combo_modelos_3d);
  // Pontos de vida.
  PreencheConfiguraPontosVida(this, gerador, entidade, proto_retornado);
  // Aura.
  gerador.spin_aura_quad->setValue(entidade.aura_m() * ent::METROS_PARA_QUADRADOS);
  // Voo.
  gerador.checkbox_voadora->setCheckState(entidade.voadora() ? Qt::Checked : Qt::Unchecked);
  // Caida.
  gerador.checkbox_caida->setCheckState(entidade.caida() ? Qt::Checked : Qt::Unchecked);
  // Morta.
  gerador.checkbox_morta->setCheckState(entidade.morta() ? Qt::Checked : Qt::Unchecked);
  // Translacao em Z.
  gerador.spin_translacao_quad->setValue(entidade.pos().z() * ent::METROS_PARA_QUADRADOS);

  PreencheConfiguraTesouro(this, gerador, entidade, proto_retornado);

  gerador.texto_notas->appendPlainText((entidade.notas().c_str()));

  // Proxima salvacao: para funcionar, o combo deve estar ordenado da mesma forma que a enum ResultadoSalvacao.
  gerador.checkbox_salvacao->setCheckState(entidade.has_proxima_salvacao() ? Qt::Checked : Qt::Unchecked);
  gerador.combo_salvacao->setCurrentIndex((int)entidade.proxima_salvacao());

  // Tipo de visao.
  gerador.combo_visao->setCurrentIndex((int)entidade.tipo_visao());
  lambda_connect(gerador.combo_visao, SIGNAL(currentIndexChanged(int)), [this, &gerador] () {
    gerador.spin_raio_visao_escuro_quad->setEnabled(gerador.combo_visao->currentIndex() == ent::VISAO_ESCURO);
  });
  gerador.spin_raio_visao_escuro_quad->setValue(ent::METROS_PARA_QUADRADOS * (entidade.has_alcance_visao_m() ? entidade.alcance_visao_m() : 18));
  gerador.spin_raio_visao_escuro_quad->setEnabled(entidade.tipo_visao() == ent::VISAO_ESCURO);

  // Preenche os atributos.
  PreencheConfiguraAtributos(this, gerador, entidade, proto_retornado);

  // Iniciativa.
  PreencheConfiguraDadosIniciativa(this, gerador, entidade, proto_retornado);

  // Tendencia.
  PreencheConfiguraTendencia(this, gerador, proto_retornado);

  // Combos dinamicos.
  PreencheConfiguraComboArmaduraEscudo(this, gerador, proto_retornado);

  // Dados de defesa.
  PreencheConfiguraDadosDefesa(this, gerador, entidade, proto_retornado);

  // Dados de ataque.
  PreencheConfiguraDadosAtaque(this, gerador, entidade, proto_retornado);

  // Preenche configura classes e niveis.
  PreencheConfiguraClassesNiveis(this, gerador, proto_retornado);

  // Preenche a parte de resistencias.
  PreencheConfiguraSalvacoes(this, gerador, proto_retornado);

  // Coisas que nao estao na UI.
  if (entidade.has_direcao_queda()) {
    proto_retornado->mutable_direcao_queda()->CopyFrom(entidade.direcao_queda());
  }
  if (entidade.has_desenha_base()) {
    proto_retornado->set_desenha_base(entidade.desenha_base());
  }

  gerador.spin_xp->setValue(entidade.experiencia());

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, &entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor, forma_primaria] () {
    ent::RecomputaDependencias(tabelas(), proto_retornado);
    if (gerador.campo_rotulo->text().isEmpty()) {
      proto_retornado->clear_rotulo();
    } else {
      proto_retornado->set_rotulo(gerador.campo_rotulo->text().toStdString());
    }
    QStringList lista_rotulos = gerador.lista_rotulos->toPlainText().split("\n", QString::SkipEmptyParts);
    proto_retornado->clear_rotulo_especial();
    for (const auto& rotulo : lista_rotulos) {
      proto_retornado->add_rotulo_especial(rotulo.toStdString());
    }

    proto_retornado->set_tamanho(static_cast<ent::TamanhoEntidade>(gerador.slider_tamanho->sliderPosition()));
    if (gerador.checkbox_cor->checkState() == Qt::Checked) {
      proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
      proto_retornado->mutable_cor()->set_a(gerador.slider_alfa->value() / 100.0f);
    } else {
      proto_retornado->clear_cor();
    }
    if (gerador.spin_raio_quad->value() > 0.0f) {
      proto_retornado->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
      proto_retornado->mutable_luz()->set_raio_m(gerador.spin_raio_quad->value() * ent::QUADRADOS_PARA_METROS);
    } else {
      proto_retornado->clear_luz();
    }
    if (gerador.combo_textura->currentIndex() == 0) {
      proto_retornado->clear_info_textura();
    } else {
      PreencheTexturaProtoRetornado(entidade.info_textura(), gerador.combo_textura, proto_retornado->mutable_info_textura());
      proto_retornado->mutable_info_textura()->set_largura(gerador.spin_tex_largura->value());
      proto_retornado->mutable_info_textura()->set_altura(gerador.spin_tex_altura->value());
      proto_retornado->mutable_info_textura()->set_translacao_x(gerador.spin_tex_trans_x->value());
      proto_retornado->mutable_info_textura()->set_translacao_y(gerador.spin_tex_trans_y->value());
    }
    if (gerador.combo_modelos_3d->currentIndex() == 0) {
      proto_retornado->clear_modelo_3d();
    } else {
      proto_retornado->mutable_modelo_3d()->set_id(gerador.combo_modelos_3d->currentText().toStdString());
    }
    proto_retornado->set_pontos_vida(gerador.spin_pontos_vida->value());
    proto_retornado->set_dano_nao_letal(gerador.spin_dano_nao_letal->value());
    proto_retornado->set_max_pontos_vida(gerador.spin_max_pontos_vida->value());
    float aura_m = gerador.spin_aura_quad->value() * ent::QUADRADOS_PARA_METROS;
    if (aura_m > 0) {
      proto_retornado->set_aura_m(aura_m);
    } else {
      proto_retornado->clear_aura_m();
    }
    proto_retornado->set_voadora(gerador.checkbox_voadora->checkState() == Qt::Checked);
    proto_retornado->set_caida(gerador.checkbox_caida->checkState() == Qt::Checked);
    proto_retornado->set_morta(gerador.checkbox_morta->checkState() == Qt::Checked);
    proto_retornado->set_visivel(gerador.checkbox_visibilidade->checkState() == Qt::Checked);
    proto_retornado->set_selecionavel_para_jogador(gerador.checkbox_selecionavel->checkState() == Qt::Checked);
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao_quad->value() * ent::QUADRADOS_PARA_METROS);
    proto_retornado->clear_translacao_z_deprecated();
    if (gerador.checkbox_salvacao->isChecked()) {
      proto_retornado->set_proxima_salvacao((ent::ResultadoSalvacao)gerador.combo_salvacao->currentIndex());
    } else {
      proto_retornado->clear_proxima_salvacao();
    }
    proto_retornado->set_tipo_visao((ent::TipoVisao)gerador.combo_visao->currentIndex());
    proto_retornado->mutable_dados_defesa()->set_imune_critico(gerador.checkbox_imune_critico->checkState() == Qt::Checked);
    proto_retornado->mutable_dados_ataque_global()->set_dano_furtivo(gerador.linha_furtivo->text().toStdString());
    if (proto_retornado->tipo_visao() == ent::VISAO_ESCURO) {
      proto_retornado->set_alcance_visao_m(gerador.spin_raio_visao_escuro_quad->value() * ent::QUADRADOS_PARA_METROS);
    }
    if (gerador.checkbox_iniciativa->checkState() == Qt::Checked) {
      proto_retornado->set_iniciativa(gerador.spin_iniciativa->value());
    } else {
      proto_retornado->clear_iniciativa();
    }

    if ((gerador.lista_ataques->currentRow() >= 0 && gerador.lista_ataques->currentRow() < proto_retornado->dados_ataque().size()) ||
        gerador.linha_dano->text().size() > 0) {
      AdicionaOuAtualizaAtaqueEntidade(this->tabelas(), gerador, proto_retornado);
    }
    if (gerador.spin_nivel_classe->value() > 0) {
      AdicionaOuEditaNivel(this->tabelas(), gerador, proto_retornado);
    }
    if (forma_primaria &&
        entidade.forma_alternativa_corrente() >= 0 &&
        entidade.forma_alternativa_corrente() < proto_retornado->formas_alternativas_size()) {
      // Atualiza a forma alternativa correspondente.
      *proto_retornado->mutable_formas_alternativas(entidade.forma_alternativa_corrente()) =
          ProtoFormaAlternativa(*proto_retornado);
    }
    // Campos de texto eh melhor atualizar so no final, porque os eventos sao complicados de lidar.
    proto_retornado->set_notas(gerador.texto_notas->toPlainText().toStdString());
    proto_retornado->mutable_tesouro()->set_tesouro(gerador.lista_tesouro->toPlainText().toStdString());
    proto_retornado->set_experiencia(gerador.spin_xp->value());
  });
  // TODO: Ao aplicar as mudanças refresca e nao fecha.

  // Cancelar.
  lambda_connect(dialogo, SIGNAL(rejected()), [&notificacao, &delete_proto_retornado] {
      delete_proto_retornado.reset();
  });
  dialogo->exec();
  delete dialogo;
  return delete_proto_retornado;
}

void AbreDialogoBonus(QWidget* pai, ent::Bonus* bonus) {
  ifg::qt::Ui::DialogoBonus gerador;
  std::unique_ptr<QDialog> dialogo(new QDialog(pai));
  gerador.setupUi(dialogo.get());
  std::unique_ptr<QItemSelectionModel> delete_model(gerador.tabela_bonus->selectionModel());
  std::unique_ptr<ModeloBonus> modelo(new ModeloBonus(*bonus, gerador.tabela_bonus));
  gerador.tabela_bonus->setModel(modelo.get());
  lambda_connect(gerador.botao_adicionar_bonus, SIGNAL(clicked()), [&gerador, &modelo] () {
    const int linha = modelo->rowCount();
    modelo->insertRows(linha, 1, QModelIndex());
    gerador.tabela_bonus->selectRow(linha);
  });
  lambda_connect(gerador.botao_remover_bonus, SIGNAL(clicked()), [&gerador, &modelo] () {
    std::set<int, std::greater<int>> linhas;
    for (const QModelIndex& index : gerador.tabela_bonus->selectionModel()->selectedIndexes()) {
      linhas.insert(index.row());
    }
    for (int linha : linhas) {
      modelo->removeRows(linha, 1, QModelIndex());
    }
  });
  std::unique_ptr<QAbstractItemDelegate> delegado(
      new TipoBonusDelegate(gerador.tabela_bonus, modelo.get(), gerador.tabela_bonus));
  std::unique_ptr<QAbstractItemDelegate> delete_previous(gerador.tabela_bonus->itemDelegateForColumn(0));
  gerador.tabela_bonus->setItemDelegateForColumn(0, delegado.get());

  auto res = dialogo->exec();
  if (res == QDialog::Rejected) {
    return;
  }
  *bonus = modelo->ModeloParaBonus();
}

std::unique_ptr<ent::EntidadeProto> Visualizador3d::AbreDialogoEntidade(
    const ntf::Notificacao& notificacao) {
  if (notificacao.entidade().tipo() == ent::TE_ENTIDADE) {
    return AbreDialogoTipoEntidade(notificacao);
  } else if (notificacao.entidade().tipo() == ent::TE_FORMA || notificacao.entidade().tipo() == ent::TE_COMPOSTA) {
    return std::unique_ptr<ent::EntidadeProto>(AbreDialogoTipoForma(notificacao));
  }
  return std::unique_ptr<ent::EntidadeProto>();
}

// ATENCAO: se mexer aqui, mexa tb em Tabuleiro::DeserializaPropriedades pois os campos sao copiados campo a campo
// para nao se perder outras coisas importantes do cenario.
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
  PreencheComboTextura(tab_proto.info_textura().id().c_str(), notificacao.tabuleiro().id_cliente(), ent::FiltroTexturaTabuleiro, gerador.combo_fundo);
  // Ceu do tabuleiro.
  PreencheComboTexturaCeu(tab_proto.info_textura_ceu().id().c_str(), notificacao.tabuleiro().id_cliente(), gerador.combo_ceu);
  gerador.checkbox_luz_ceu->setCheckState(tab_proto.aplicar_luz_ambiente_textura_ceu() ? Qt::Checked : Qt::Unchecked);

  // Ladrilho de textura.
  gerador.checkbox_ladrilho->setCheckState(tab_proto.ladrilho() ? Qt::Checked : Qt::Unchecked);
  // grade.
  gerador.checkbox_grade->setCheckState(tab_proto.desenha_grade() ? Qt::Checked : Qt::Unchecked);
  // Mestre apenas.
  gerador.checkbox_mestre_apenas->setCheckState(tab_proto.textura_mestre_apenas() ? Qt::Checked : Qt::Unchecked);
  // Cor de piso.
  gerador.checkbox_cor_piso->setCheckState(tab_proto.has_cor_piso() ? Qt::Checked : Qt::Unchecked);
  ent::Cor cor_piso_proto(tab_proto.cor_piso());
  gerador.botao_cor_piso->setStyleSheet(CorParaEstilo(cor_piso_proto));
  lambda_connect(gerador.botao_cor_piso, SIGNAL(clicked()), [this, dialogo, &gerador, &cor_piso_proto] {
    QColor cor =
        QColorDialog::getColor(ProtoParaCor(cor_piso_proto), dialogo, QObject::tr("Cor do piso"));
    if (!cor.isValid()) {
      return;
    }
    gerador.checkbox_cor_piso->setCheckState(Qt::Checked);
    gerador.botao_cor_piso->setStyleSheet(CorParaEstilo(cor));
    cor_piso_proto = CorParaProto(cor);
  });

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

  // Clonar cenario.
  PreencheComboCenarios(tabuleiro_->Proto(), gerador.combo_id_cenario);
  lambda_connect(gerador.botao_clonar, SIGNAL(clicked()), [this, &gerador, proto_retornado, dialogo] () {
    const int id_combo = IdCenarioComboCenarios(gerador.combo_id_cenario);
    const int id_corrente = proto_retornado->id_cenario();
    if (id_combo == CENARIO_INVALIDO || id_combo == id_corrente) return;
    const auto* cenario = ((const ent::Tabuleiro*)tabuleiro_)->BuscaSubCenario(id_combo);
    if (cenario != nullptr) {
      *proto_retornado = *cenario;
      proto_retornado->set_id_cenario(id_corrente);
      dialogo->accept();
      return;
    }
  });

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(gerador.botoes, SIGNAL(accepted()),
                 [this, tab_proto, dialogo, &gerador, &cor_ambiente_proto, &cor_direcional_proto, &cor_piso_proto, proto_retornado] {
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
    // Cor piso.
    if (gerador.checkbox_cor_piso->checkState() == Qt::Checked) {
      proto_retornado->mutable_cor_piso()->Swap(&cor_piso_proto);
    } else {
      proto_retornado->clear_cor_piso();
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
  auto* proto_retornado = new ent::OpcoesProto(notificacao.opcoes());
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
  // Mapeamento de sombras.
  gerador.checkbox_mapeamento_de_sombras->setCheckState(opcoes_proto.mapeamento_sombras() ? Qt::Checked : Qt::Unchecked);
  // Iluminacao por pixel.
  gerador.checkbox_iluminacao_por_pixel->setCheckState(opcoes_proto.iluminacao_por_pixel() ? Qt::Checked : Qt::Unchecked);
  // Oclusao.
  gerador.checkbox_mapeamento_oclusao->setCheckState(opcoes_proto.mapeamento_oclusao() ? Qt::Checked : Qt::Unchecked);
  // Ataque vs defesa posicao real.
  gerador.checkbox_ataque_vs_defesa_posicao_real->setCheckState(opcoes_proto.ataque_vs_defesa_posicao_real() ? Qt::Checked : Qt::Unchecked);

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
   proto_retornado->set_mapeamento_sombras(
        gerador.checkbox_mapeamento_de_sombras->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_iluminacao_por_pixel(
        gerador.checkbox_iluminacao_por_pixel->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_mapeamento_oclusao(
        gerador.checkbox_mapeamento_oclusao->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_ataque_vs_defesa_posicao_real(
        gerador.checkbox_ataque_vs_defesa_posicao_real->checkState() == Qt::Checked ? true : false);
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
