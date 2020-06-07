#include "ifg/qt/visualizador3d.h"

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
#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>

#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/recomputa.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "goog/stringprintf.h"
#include "ifg/qt/atualiza_ui.h"
#include "ifg/qt/bonus_util.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/evento_util.h"
#include "ifg/qt/inimigo_predileto_util.h"
#include "ifg/qt/itens_magicos_util.h"
#include "ifg/qt/pericias_util.h"
#include "ifg/qt/pocoes_util.h"
#include "ifg/qt/talentos_util.h"
#include "ifg/qt/ui/cenario.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/ui/forma.h"
#include "ifg/qt/ui/opcoes.h"
#include "ifg/qt/util.h"
#include "ifg/tecladomouse.h"
#include "log/log.h"
#include "m3d/m3d.h"
#include "net/util.h"
#include "ntf/notificacao.pb.h"

namespace ifg {
namespace qt {
namespace {

using namespace std;
using google::protobuf::StringPrintf;
using google::protobuf::RepeatedPtrField;

// Redimensiona o container.
template <class T>
void Redimensiona(int tam, RepeatedPtrField<T>* c) {
  if (tam == c->size()) return;
  if (tam < c->size()) {
    c->DeleteSubrange(tam, c->size() - tam);
    return;
  }
  while (c->size() < tam) c->Add();
}

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

void AdicionaSeparador(const QString& rotulo, QComboBox* combo_textura) {
  QStandardItem* item = new QStandardItem(rotulo);
  item->setFlags(item->flags() & ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
  QFont font = item->font();
  font.setBold(true);
  item->setFont(font);
  ((QStandardItemModel*)combo_textura->model())->appendRow(item);
}

// Usado para manter textura local nao presente no cliente.
const int MANTEM_TEXTURA = -2;

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
      // Neste caso, a textura não existe para o cliente. Criar a entrada agora.
      combo_textura->addItem(QString(combo_textura->tr("Manter Atual")), QVariant(MANTEM_TEXTURA));
      index = combo_textura->count() - 1;
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
    QVariant dados(combo_textura->currentData());
    QString nome(combo_textura->currentText());
    if (nome.toStdString() == info_antes.id() || dados.toInt() == MANTEM_TEXTURA) {
      // Textura igual a anterior.
      VLOG(2) << "Textura igual a anterior.";
      info_textura->set_id(info_antes.id());
    } else {
      VLOG(2) << "Textura diferente da anterior.";
      if (dados.toInt() == arq::TIPO_TEXTURA_LOCAL) {
        VLOG(2) << "Textura local, recarregando.";
        info_textura->set_id(nome.toStdString());
        ent::PreencheInfoTextura(nome.split(":")[1].toStdString(),
            arq::TIPO_TEXTURA_LOCAL, info_textura);
      } else {
        info_textura->Clear();
        info_textura->set_id(nome.toStdString());
      }
    }
    VLOG(2) << "Id textura: " << info_textura->id();
  } else {
    info_textura->Clear();
  }
}

void PreencheComboCenarioPai(const ent::TabuleiroProto& tabuleiro, QComboBox* combo) {
  combo->addItem("Sem herança", QVariant(CENARIO_INVALIDO));
  combo->addItem("Principal", QVariant(CENARIO_PRINCIPAL));
  for (const auto& sub_cenario : tabuleiro.sub_cenario()) {
    std::string descricao;
    if (sub_cenario.descricao_cenario().empty()) {
      descricao = StringPrintf("Sub Cenário: %d", sub_cenario.id_cenario());
    } else {
      descricao = StringPrintf("%s (%d)", sub_cenario.descricao_cenario().c_str(), sub_cenario.id_cenario());
    }
    combo->addItem(QString::fromUtf8(descricao.c_str()), QVariant(sub_cenario.id_cenario()));
  }
  ExpandeComboBox(combo);
}

void PreencheComboCenarios(const ent::TabuleiroProto& tabuleiro, QComboBox* combo) {
  combo->addItem("Novo", QVariant());
  combo->addItem("Principal", QVariant(CENARIO_PRINCIPAL));
  for (const auto& sub_cenario : tabuleiro.sub_cenario()) {
    std::string descricao;
    if (sub_cenario.descricao_cenario().empty()) {
      descricao = StringPrintf("Sub Cenário: %d", sub_cenario.id_cenario());
    } else {
      descricao = StringPrintf("%s (%d)", sub_cenario.descricao_cenario().c_str(), sub_cenario.id_cenario());
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

Visualizador3d::Visualizador3d(
    const ent::Tabelas& tabelas,
    m3d::Modelos3d* m3d,
    tex::Texturas* texturas,
    TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QOpenGLWidget(pai),
       tabelas_(tabelas),
       m3d_(m3d),
       texturas_(texturas),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro) {
  //const ent::OpcoesProto& opcoes = tabuleiro->Opcoes();
  tipo_iluminacao_ = ent::OpcoesProto::TI_ESPECULAR;
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
  LOG(INFO) << "Inicializando GL.......................";
  static bool once = false;
  try {
    if (!once) {
      once = true;
      gl::IniciaGl(static_cast<gl::TipoLuz>(tipo_iluminacao_), scale_);
    }
    tabuleiro_->IniciaGL();
    gl_iniciado_ = true;
    LOG(INFO) << "GL iniciado";
  } catch (const std::logic_error& erro) {
    // Este log de erro eh pro caso da aplicacao morrer e nao conseguir mostrar a mensagem.
    LOG(ERROR) << "Erro na inicializacao GL " << erro.what();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErro(erro.what()));
  }
}

void Visualizador3d::resizeGL(int width, int height) {
  LOG(INFO) << "w: " << width << ", h: " << height;
  width *= scale_;
  height *= scale_;
  tabuleiro_->TrataRedimensionaJanela(width, height);
  update();
}

void Visualizador3d::paintGL() {
  tabuleiro_->Desenha();
}

// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_MUDAR_CURSOR:
      switch (notificacao.id_generico()) {
        case ent::Tabuleiro::MODO_ROTACAO:
          setCursor(QCursor(Qt::SizeAllCursor));
          break;
        case ent::Tabuleiro::MODO_AGUARDANDO:
          setCursor(QCursor(Qt::WaitCursor));
          break;
        case ent::Tabuleiro::MODO_SINALIZACAO:
        case ent::Tabuleiro::MODO_ACAO:
          if (!notificacao.entidade().dados_ataque().empty() &&
              !notificacao.entidade().dados_ataque(0).acao().has_tipo()) {
            setCursor(QCursor(Qt::ForbiddenCursor));
          } else {
            setCursor(QCursor(Qt::CrossCursor));
          }
          break;
        case ent::Tabuleiro::MODO_AJUDA:
          setCursor(QCursor(Qt::WhatsThisCursor));
          break;
        case ent::Tabuleiro::MODO_REGUA:
          setCursor(QCursor(Qt::CrossCursor));
          break;
        case ent::Tabuleiro::MODO_PERICIA:
          setCursor(QCursor(Qt::PointingHandCursor));
          break;
        case ent::Tabuleiro::MODO_ROLA_DADO:
          setCursor(QCursor(Qt::OpenHandCursor));
          break;
        default:
          setCursor(QCursor(Qt::ArrowCursor));
      }
      return true;
    case ntf::TN_CARREGAR_TEXTURA: {
      makeCurrent();
      texturas_->CarregaTexturas(notificacao);
      doneCurrent();
      break;
    }
    case ntf::TN_DESCARREGAR_TEXTURA: {
      makeCurrent();
      texturas_->DescarregaTexturas(notificacao);
      doneCurrent();
      break;
    }
    case ntf::TN_CARREGAR_MODELO_3D: {
      makeCurrent();
      m3d_->CarregaModelo3d(notificacao.entidade().modelo_3d().id());
      doneCurrent();
      break;
    }
    case ntf::TN_DESCARREGAR_MODELO_3D: {
      makeCurrent();
      m3d_->DescarregaModelo3d(notificacao.entidade().modelo_3d().id());
      doneCurrent();
      break;
    }
    case ntf::TN_REINICIAR_GRAFICO: {
      makeCurrent();
      tabuleiro_->ResetGrafico();
      doneCurrent();
      break;
    }
    case ntf::TN_TEMPORIZADOR_MOUSE: {
      makeCurrent();
      tabuleiro_->TrataMouseParadoEm(notificacao.pos().x(), notificacao.pos().y());
      doneCurrent();
      break;
    }
    case ntf::TN_INICIADO: {
      // chama o resize pra iniciar a geometria e desenha a janela
      makeCurrent();
      resizeGL(width(), height());
      doneCurrent();
      break;
    }
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
      makeCurrent();
      tabuleiro_->TrataNotificacao(*n);
      doneCurrent();
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
      makeCurrent();
      tabuleiro_->TrataNotificacao(*n);
      doneCurrent();
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
      makeCurrent();
      tabuleiro_->TrataNotificacao(*n);
      doneCurrent();
      break;
    }
    case ntf::TN_TEMPORIZADOR:
      if (gl_iniciado_) {
        makeCurrent();
        tabuleiro_->AtualizaPorTemporizacao();
        doneCurrent();
        update();
      }
      //glDraw();
      break;
    default: ;
  }
  return true;
}

// teclado.
void Visualizador3d::keyPressEvent(QKeyEvent* event) {
  makeCurrent();
  teclado_mouse_->TrataTeclaPressionada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
  doneCurrent();
}

void Visualizador3d::keyReleaseEvent(QKeyEvent* event) {
  makeCurrent();
  teclado_mouse_->TrataTeclaLiberada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
  doneCurrent();
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  makeCurrent();
  teclado_mouse_->TrataBotaoMousePressionado(
       BotaoMouseQtParaTratadorTecladoMouse(event->button()),
       ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
       event->x() * scale_,
       (height() - event->y()) * scale_);
  event->accept();
  doneCurrent();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  makeCurrent();
  teclado_mouse_->TrataBotaoMouseLiberado();
  event->accept();
  doneCurrent();
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
  makeCurrent();
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      event->x() * scale_, (height() - event->y()) * scale_);
  event->accept();
  doneCurrent();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  makeCurrent();
  int x = event->globalX();
  int y = event->globalY();
  if (teclado_mouse_->TrataMovimentoMouse(event->x() * scale_, (height() - event->y()) * scale_)) {
    QCursor::setPos(x_antes_, y_antes_);
  } else {
    x_antes_ = x;
    y_antes_ = y;
  }
  doneCurrent();
  event->accept();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  makeCurrent();
  teclado_mouse_->TrataRodela(event->delta());
  event->accept();
  doneCurrent();
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
  ent::DadosAtaque da = indice_valido ? proto_retornado->dados_ataque(indice) : ent::DadosAtaque::default_instance();
  da.set_tipo_ataque(CurrentData(gerador.combo_tipo_ataque).toString().toStdString());

  da.set_bonus_magico(gerador.spin_bonus_magico->value());
  da.set_municao(gerador.spin_municao->value());
  if (gerador.spin_limite_vezes->value() > 0) {
    da.set_limite_vezes(gerador.spin_limite_vezes->value());
  } else {
    da.clear_limite_vezes();
  }
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
  if (gerador.spin_nivel_conjurador_pergaminho->isEnabled()) {
    da.set_nivel_conjurador_pergaminho(gerador.spin_nivel_conjurador_pergaminho->value());
    da.set_modificador_atributo_pergaminho(gerador.spin_modificador_atributo_pergaminho->value());
  } else {
    da.clear_nivel_conjurador_pergaminho();
    da.clear_modificador_atributo_pergaminho();
  }
  if (gerador.checkbox_ignora_rm->checkState() == Qt::Checked) {
    da.mutable_acao_fixa()->set_ignora_resistencia_magia(true);
  } else {
    da.mutable_acao_fixa()->clear_ignora_resistencia_magia();
  }
  if (gerador.checkbox_permite_salvacao->checkState() == Qt::Checked) {
    da.mutable_acao_fixa()->set_permite_salvacao(true);
  } else {
    da.mutable_acao_fixa()->clear_permite_salvacao();
  }
  if (gerador.checkbox_ataque_agarrar->checkState() == Qt::Checked) {
    da.mutable_acao_fixa()->set_ataque_agarrar(true);
  } else {
    da.mutable_acao_fixa()->clear_ataque_agarrar();
  }
  if (gerador.checkbox_ataque_toque->checkState() == Qt::Checked) {
    da.mutable_acao_fixa()->set_ataque_toque(true);
  } else {
    da.mutable_acao_fixa()->clear_ataque_toque();
  }

  if (indice_valido) {
    proto_retornado->mutable_dados_ataque(indice)->Swap(&da);
  } else {
    proto_retornado->add_dados_ataque()->Swap(&da);
  }
  ent::RecomputaDependencias(tabelas, proto_retornado);
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

ent::DescritorAtaque IndiceParaMaterialArmadura(int indice) {
  switch (indice) {
    case 1: return ent::DESC_ADAMANTE;
    case 2: return ent::DESC_COURO_DRAGAO;
    case 3: return ent::DESC_MITRAL;
  }
  return ent::DESC_NENHUM;
}

ent::DescritorAtaque IndiceParaMaterialEscudo(int indice) {
  switch (indice) {
    case 1: return ent::DESC_ADAMANTE;
    case 2: return ent::DESC_COURO_DRAGAO;
    case 3: return ent::DESC_MADEIRA_NEGRA;
    case 4: return ent::DESC_MITRAL;
  }
  return ent::DESC_NENHUM;
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
  auto* combo_material_armadura = gerador.combo_material_armadura;
  lambda_connect(combo_material_armadura, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_material_armadura] () {
    proto_retornado->mutable_dados_defesa()->set_material_armadura(IndiceParaMaterialArmadura(combo_material_armadura->currentIndex()));
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
  auto* combo_material_escudo = gerador.combo_material_escudo;
  lambda_connect(combo_material_escudo, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, combo_material_escudo] () {
    proto_retornado->mutable_dados_defesa()->set_material_escudo(IndiceParaMaterialEscudo(combo_material_escudo->currentIndex()));
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
  auto* modelo(new ModeloEvento(proto_retornado->evento(), gerador.tabela_lista_eventos));
  std::unique_ptr<QItemSelectionModel> delete_old(gerador.tabela_lista_eventos->selectionModel());
  gerador.tabela_lista_eventos->setModel(modelo);
  gerador.tabela_lista_eventos->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  gerador.tabela_lista_eventos->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  gerador.tabela_lista_eventos->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  gerador.tabela_lista_eventos->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  lambda_connect(gerador.botao_adicionar_evento, SIGNAL(clicked()), [&gerador, modelo] () {
    const int linha = modelo->rowCount();
    modelo->insertRows(linha, 1, modelo->index(linha, 0).parent());
    gerador.tabela_lista_eventos->selectRow(linha);
  });
  lambda_connect(gerador.botao_remover_evento, SIGNAL(clicked()), [&gerador, modelo] () {
    std::set<int, std::greater<int>> linhas_a_remover;
    for (const QModelIndex& index : gerador.tabela_lista_eventos->selectionModel()->selectedIndexes()) {
      linhas_a_remover.insert(index.row());
    }
    for (int linha : linhas_a_remover) {
      modelo->removeRows(linha, 1, modelo->index(linha, 0).parent());
    }
  });
  auto* delegado = new TipoEfeitoDelegate(gerador.tabela_lista_eventos, modelo, gerador.tabela_lista_eventos);
  std::unique_ptr<QAbstractItemDelegate> delete_old_delegate(gerador.tabela_lista_eventos->itemDelegateForColumn(0));
  gerador.tabela_lista_eventos->setItemDelegateForColumn(0, delegado);
  delegado->deleteLater();
  gerador.tabela_lista_eventos->resizeColumnsToContents();
  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [this_, proto_retornado, &gerador, modelo] () {
    *proto_retornado->mutable_evento() = modelo->LeEventos();
    ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
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
  TrocaDelegateColuna(3, new RichTextDelegate(gerador.tabela_talentos), gerador.tabela_talentos);

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

  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  gerador.tabela_talentos->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [&tabelas, &gerador, proto_retornado, modelo] () {
    // TODO alterar o delegate do complemento.
    *proto_retornado->mutable_info_talentos() = modelo->Converte();
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  AtualizaUIPericias(tabelas, gerador, proto);
}

ent::TipoEvasao IndiceComboParaTipoEvasao(int indice) {
  if (indice < 0 || indice > ent::TE_EVASAO_APRIMORADA) return ent::TE_NENHUM;
  // TODO tratar com switch?
  return (ent::TipoEvasao)indice;
}

void PreencheConfiguraEvasao(Visualizador3d* this_,
                             ifg::qt::Ui::DialogoEntidade& gerador,
                             const ent::EntidadeProto& proto,
                             ent::EntidadeProto* proto_retornado) {
  AtualizaUIEvasao(this_->tabelas(), gerador, proto);
  const ent::Tabelas& tabelas = this_->tabelas();
  lambda_connect(gerador.combo_evasao_estatica, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    proto_retornado->mutable_dados_defesa()->set_evasao_estatica(IndiceComboParaTipoEvasao(gerador.combo_evasao_estatica->currentIndex()));
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.combo_evasao_dinamica, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado] () {
    proto_retornado->mutable_dados_defesa()->set_evasao(IndiceComboParaTipoEvasao(gerador.combo_evasao_dinamica->currentIndex()));
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
}

void PreencheConfiguraEsquivaSobrenatural(Visualizador3d* this_,
                                          ifg::qt::Ui::DialogoEntidade& gerador,
                                          const ent::EntidadeProto& proto,
                                          ent::EntidadeProto* proto_retornado) {
  AtualizaUIEsquivaSobrenatural(this_->tabelas(), gerador, proto);
}

void PreencheConfiguraInimigosPrediletos(Visualizador3d* this_,
                                         ifg::qt::Ui::DialogoEntidade& gerador,
                                         const ent::EntidadeProto& proto,
                                         ent::EntidadeProto* proto_retornado) {
  const ent::Tabelas& tabelas = this_->tabelas();

  auto* modelo(new ModeloInimigoPredileto(tabelas, *proto_retornado, gerador.tabela_inimigos_prediletos));
  std::unique_ptr<QItemSelectionModel> delete_old(gerador.tabela_inimigos_prediletos->selectionModel());

  TrocaDelegateColuna(2, new TipoDnDDelegate(tabelas, modelo, gerador.tabela_inimigos_prediletos), gerador.tabela_inimigos_prediletos);
  TrocaDelegateColuna(3, new SubTipoDnDDelegate(tabelas, modelo, gerador.tabela_inimigos_prediletos), gerador.tabela_inimigos_prediletos);

  gerador.tabela_inimigos_prediletos->setModel(modelo);
  lambda_connect(gerador.botao_adicionar_inimigo_predileto, SIGNAL(clicked()), [&gerador, modelo]() {
    const int linha = modelo->rowCount();
    modelo->insertRows(linha, 1, QModelIndex());
    gerador.tabela_inimigos_prediletos->selectRow(linha);
  });
  lambda_connect(gerador.botao_remover_inimigo_predileto, SIGNAL(clicked()), [&gerador, modelo]() {
    std::set<int, std::greater<int>> linhas;
    for (const QModelIndex& index : gerador.tabela_inimigos_prediletos->selectionModel()->selectedIndexes()) {
      linhas.insert(index.row());
    }
    for (int linha : linhas) {
      modelo->removeRows(linha, 1, QModelIndex());
    }
  });

  gerador.tabela_inimigos_prediletos->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
  gerador.tabela_inimigos_prediletos->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  gerador.tabela_inimigos_prediletos->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  gerador.tabela_inimigos_prediletos->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

  lambda_connect(modelo, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                 [&tabelas, &gerador, proto_retornado, modelo]() {
    *proto_retornado->mutable_dados_ataque_global()->mutable_inimigos_prediletos() = modelo->Converte();
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
          auto* fn = ent::FeiticosNivel(id_classe, nivel, proto_retornado);
          fn->add_conhecidos()->set_nome("");
          AdicionaItemFeiticoConhecido(this_->tabelas(), gerador, "", "", id_classe, nivel, fn->conhecidos_size() - 1, *proto_retornado, item);
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
          auto* fn = ent::FeiticosNivel(id_classe, nivel, proto_retornado);
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
              auto* fn_correcao = ent::FeiticosNivel(id_classe, i, proto_retornado);
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
    auto* f = ent::FeiticosNivel(id_classe, nivel, proto_retornado);
    if (item->data(0, Qt::UserRole).toInt() == RAIZ_CONHECIDO) {
      gerador.arvore_feiticos->blockSignals(true);
      std::string id_classe = item->data(TCOL_ID_CLASSE, Qt::UserRole).toString().toStdString();
      if (ClasseDeveConhecerFeitico(this_->tabelas(), id_classe)) return;
      int nivel = item->data(TCOL_NIVEL, Qt::UserRole).toInt();
      auto* fn = ent::FeiticosNivel(id_classe, nivel, proto_retornado);
      fn->add_conhecidos()->set_nome("");
      AdicionaItemFeiticoConhecido(this_->tabelas(), gerador, "", "", id_classe, nivel, fn->conhecidos_size() - 1, *proto_retornado, item);
      item->setExpanded(true);
      if (ent::ClassePrecisaMemorizar(this_->tabelas(), id_classe)) {
        AtualizaCombosParaLancar(this_->tabelas(), gerador, id_classe, *proto_retornado);
      }
      gerador.arvore_feiticos->blockSignals(false);
    }
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
    std::unique_ptr<ent::EntidadeProto> proto_forma = this_->AbreDialogoTipoEntidade(n, /*forma_primaria=*/false, dialogo);
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
    std::unique_ptr<ent::EntidadeProto> proto_forma = this_->AbreDialogoTipoEntidade(n, /*forma_primaria=*/false, dialogo);
    if (proto_forma == nullptr) return;
    *proto_retornado->mutable_formas_alternativas(row) = ProtoFormaAlternativa(*proto_forma);
    AtualizaUIFormasAlternativas(gerador, *proto_retornado);
  });
}

template <class Dialogo, class Gerador>
void ConfiguraListaItensMagicos(
    Dialogo* dialogo, const ent::Tabelas& tabelas, Gerador& gerador, std::function<void(const ent::Tabelas&, Gerador&, const ent::EntidadeProto& proto)> f_atualiza_ui,
    ent::TipoItem tipo,
    QListWidget* lista, QPushButton* botao_usar, QPushButton* botao_adicionar, QPushButton* botao_remover, QPushButton* botao_doar,
    const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado, ntf::CentralNotificacoes* central) {
  // Delegado.
  std::unique_ptr<QAbstractItemDelegate> delete_old(lista->itemDelegate());
  auto* delegado = new ItemMagicoDelegate(tabelas, tipo, lista, proto_retornado);
  lista->setItemDelegate(delegado);
  delegado->deleteLater();

  if (botao_usar != nullptr) {
    // Muda botao de usar.
    lambda_connect(lista, SIGNAL(currentRowChanged(int)), [tipo, lista, botao_usar, &tabelas, proto_retornado] () {
      int row = lista->currentRow();
      if (row < 0 || row >= lista->count() || row >= ItensProto(tipo, *proto_retornado).size()) {
        botao_usar->setText("Vestir");
      } else {
        botao_usar->setText(ItensProto(tipo, *proto_retornado).Get(row).em_uso() ? "Tirar" : "Vestir");
      }
    });
    // Botao de usar.
    lambda_connect(botao_usar, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado, f_atualiza_ui] () {
      const int indice = lista->currentRow();
      auto* itens_personagem = ent::ItensProtoMutavel(tipo, proto_retornado);
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
        item->set_em_uso(true);
      } else {
        item->set_em_uso(false);
      }
      ent::RecomputaDependencias(tabelas, proto_retornado);
      // AtualizaUI
      f_atualiza_ui(tabelas, gerador, *proto_retornado);
    });
  }
  lambda_connect(botao_adicionar, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado] () {
    auto* itens = ent::ItensProtoMutavel(tipo, proto_retornado);
    itens->Add();
    AtualizaUITesouro(tabelas, gerador, *proto_retornado);
    lista->setCurrentRow(itens->size() - 1);
  });
  lambda_connect(botao_remover, SIGNAL(clicked()), [tipo, &tabelas, &gerador, lista, proto_retornado, f_atualiza_ui] () {
    const int indice = lista->currentRow();
    auto* itens = ent::ItensProtoMutavel(tipo, proto_retornado);
    if (indice < 0 || indice >= itens->size()) {
      return;
    }
    auto* item = itens->Mutable(indice);
    item->set_em_uso(false);
    // Tira o efeito do personagem.
    ent::RecomputaDependencias(tabelas, proto_retornado);
    // Tira o item do personagem.
    if (indice >= 0 && indice < itens->size()) {
      itens->DeleteSubrange(indice, 1);
    }
    ent::RecomputaDependencias(tabelas, proto_retornado);
    // AtualizaUI.
    f_atualiza_ui(tabelas, gerador, *proto_retornado);
    lista->setCurrentRow(indice >= itens->size() ? - 1 : indice);
  });
  lambda_connect(botao_doar, SIGNAL(clicked()), [tipo, dialogo, &tabelas, lista, &gerador, &proto, proto_retornado, central] {
    const int indice = lista->currentRow();
    const auto& itens = ent::ItensProto(tipo, *proto_retornado);
    if (indice < 0 || indice >= itens.size()) {
      return;
    }
    const auto& item = itens.Get(indice);
    auto notificacao = ntf::NovaNotificacao(ntf::TN_ENTRAR_MODO_DOACAO);
    notificacao->mutable_entidade()->set_id(proto_retornado->id());
    auto* itens_notificacao = ent::ItensProtoMutavel(tipo, notificacao->mutable_entidade());
    if (itens_notificacao == nullptr) {
      LOG(WARNING) << "tipo de item invalido: " << tipo;
      return;
    }
    auto* item_notificacao = itens_notificacao->Add();
    *item_notificacao = item;
    central->AdicionaNotificacao(notificacao.release());
    LOG(INFO) << "fechando dialogo para doacao de: " << item.DebugString();
    dialogo->reject();
  });
}

template <class Gerador>
void DuplicaItem(const ent::Tabelas& tabelas, Gerador& gerador, ent::TipoItem tipo, QListWidget* lista, ent::EntidadeProto* proto_retornado) {
  auto* itens = ent::ItensProtoMutavel(tipo, proto_retornado);
  int indice_antes = lista->currentRow();
  std::string id_selecionado;
  if (indice_antes < 0 || indice_antes >= itens->size()) {
    return;
  }
  id_selecionado = itens->Get(indice_antes).id();
  auto* item = itens->Add();
  item->set_id(id_selecionado);
  AtualizaUITesouro(tabelas, gerador, *proto_retornado);
}

template <class Gerador>
void OrdenaItens(const ent::Tabelas& tabelas, Gerador& gerador, ent::TipoItem tipo, QListWidget* lista, ent::EntidadeProto* proto_retornado) {
  auto* itens = ent::ItensProtoMutavel(tipo, proto_retornado);
  std::string id_selecionado;
  int indice_antes = lista->currentRow();
  if (indice_antes >= 0 && indice_antes < itens->size()) {
    id_selecionado = itens->Get(indice_antes).id();
  }
  std::sort(itens->begin(), itens->end(), [&tabelas, tipo](const ent::ItemMagicoProto& lhs, const ent::ItemMagicoProto& rhs) {
      return ent::ItemTabela(tabelas, tipo, lhs.id()).nome() < ent::ItemTabela(tabelas, tipo, rhs.id()).nome();
  } );
  AtualizaUITesouro(tabelas, gerador, *proto_retornado);
  for (int i = 0; indice_antes != -1 && i < itens->size(); ++i) {
    if (itens->Get(i).id() == id_selecionado) {
      lista->setCurrentRow(i);
      break;
    }
  }
}

template <class Dialogo, class Gerador>
void ConfiguraListaPergaminhosMundanosOuPocoes(
    Dialogo* dialogo, const ent::Tabelas& tabelas, Gerador& gerador, std::function<void(const ent::Tabelas&, Gerador&, const ent::EntidadeProto& proto)> f_atualiza_ui,
    ent::TipoItem tipo, QListWidget* lista,
    QPushButton* botao_usar, QPushButton* botao_adicionar, QPushButton* botao_duplicar, QPushButton* botao_remover, QPushButton* botao_ordenar, QPushButton* botao_doar,
    const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado, ntf::CentralNotificacoes* central) {
  lambda_connect(botao_duplicar, SIGNAL(clicked()), [&tabelas, &gerador, tipo, lista, proto_retornado] () {
    DuplicaItem(tabelas, gerador, tipo, lista, proto_retornado);
  });
  lambda_connect(botao_ordenar, SIGNAL(clicked()), [&tabelas, &gerador, tipo, lista, proto_retornado] () {
    OrdenaItens(tabelas, gerador, tipo, lista, proto_retornado);
  });

  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, tipo, lista,
      botao_usar, botao_adicionar, botao_remover, botao_doar,
      proto, proto_retornado, central);
}

template <class Dialogo, class Gerador>
void PreencheConfiguraTesouro(
    Visualizador3d* this_, Dialogo* dialogo, Gerador& gerador, std::function<void(const ent::Tabelas&, Gerador&, const ent::EntidadeProto& proto)> f_atualiza_ui,
    const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado, ntf::CentralNotificacoes* central) {
  const auto& tabelas = this_->tabelas();

  auto* tesouro_retornado = proto_retornado->mutable_tesouro()->mutable_moedas();
  gerador.spin_po->setValue(proto.tesouro().moedas().po());
  lambda_connect(gerador.spin_po, SIGNAL(valueChanged(int)), [tesouro_retornado, &gerador]() { tesouro_retornado->set_po(gerador.spin_po->value()); });
  gerador.spin_pp->setValue(proto.tesouro().moedas().pp());
  lambda_connect(gerador.spin_pp, SIGNAL(valueChanged(int)), [tesouro_retornado, &gerador]() { tesouro_retornado->set_pp(gerador.spin_pp->value()); });
  gerador.spin_pc->setValue(proto.tesouro().moedas().pc());
  lambda_connect(gerador.spin_pc, SIGNAL(valueChanged(int)), [tesouro_retornado, &gerador]() { tesouro_retornado->set_pc(gerador.spin_pc->value()); });
  gerador.spin_pl->setValue(proto.tesouro().moedas().pl());
  lambda_connect(gerador.spin_pl, SIGNAL(valueChanged(int)), [tesouro_retornado, &gerador]() { tesouro_retornado->set_pl(gerador.spin_pl->value()); });
  gerador.spin_pe->setValue(proto.tesouro().moedas().pe());
  lambda_connect(gerador.spin_pe, SIGNAL(valueChanged(int)), [tesouro_retornado, &gerador]() { tesouro_retornado->set_pe(gerador.spin_pe->value()); });

  // Pocoes: por algum motivo isso tava separado do resto. Comentei pra ver o que da.
#if 0
  if (0) {
    std::unique_ptr<QAbstractItemDelegate> delete_old(gerador.lista_pocoes->itemDelegate());
    auto* delegado = new PocaoDelegate(tabelas, gerador.lista_pocoes, proto_retornado);
    gerador.lista_pocoes->setItemDelegate(delegado);
    delegado->deleteLater();

    lambda_connect(gerador.botao_ordenar_pocoes, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
      OrdenaItens(tabelas, gerador, ent::TipoItem::TIPO_POCAO, gerador.lista_pocoes, proto_retornado);
    });
    lambda_connect(gerador.botao_duplicar_pocao, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
      DuplicaItem(tabelas, gerador, ent::TipoItem::TIPO_POCAO, gerador.lista_pocoes, proto_retornado);
    });

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
#endif

  // Pocoes.
  ConfiguraListaPergaminhosMundanosOuPocoes(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_POCAO,
      gerador.lista_pocoes, /*usar=*/nullptr,
      gerador.botao_adicionar_pocao, gerador.botao_duplicar_pocao,
      gerador.botao_remover_pocao, gerador.botao_ordenar_pocoes,
      gerador.botao_doar_pocao,
      proto, proto_retornado, central);
  // Pergaminhos.
  ConfiguraListaPergaminhosMundanosOuPocoes(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_PERGAMINHO_ARCANO,
      gerador.lista_pergaminhos_arcanos, /*usar=*/nullptr,
      gerador.botao_adicionar_pergaminho_arcano, gerador.botao_duplicar_pergaminho_arcano,
      gerador.botao_remover_pergaminho_arcano, gerador.botao_ordenar_pergaminhos_arcanos,
      gerador.botao_doar_pergaminho_arcano,
      proto, proto_retornado, central);
  ConfiguraListaPergaminhosMundanosOuPocoes(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_PERGAMINHO_DIVINO,
      gerador.lista_pergaminhos_divinos, /*usar=*/nullptr,
      gerador.botao_adicionar_pergaminho_divino, gerador.botao_duplicar_pergaminho_divino,
      gerador.botao_remover_pergaminho_divino, gerador.botao_ordenar_pergaminhos_divinos,
      gerador.botao_doar_pergaminho_divino,
      proto, proto_retornado, central);
  // Itens mundanos.
  ConfiguraListaPergaminhosMundanosOuPocoes(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_ITEM_MUNDANO,
      gerador.lista_itens_mundanos, /*usar=*/nullptr,
      gerador.botao_adicionar_item_mundano, gerador.botao_duplicar_item_mundano,
      gerador.botao_remover_item_mundano, gerador.botao_ordenar_item_mundano,
      gerador.botao_doar_item_mundano,
      proto, proto_retornado, central);
  // Aneis.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_ANEL,
      gerador.lista_aneis, gerador.botao_usar_anel, gerador.botao_adicionar_anel, gerador.botao_remover_anel,
      gerador.botao_doar_anel,
      proto, proto_retornado, central);
  // Luvas.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_LUVAS,
      gerador.lista_luvas, gerador.botao_usar_luvas, gerador.botao_adicionar_luvas, gerador.botao_remover_luvas,
      gerador.botao_doar_luvas,
      proto, proto_retornado, central);
  // Botas.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_BOTAS,
      gerador.lista_botas, gerador.botao_usar_botas, gerador.botao_adicionar_botas, gerador.botao_remover_botas,
      gerador.botao_doar_botas,
      proto, proto_retornado, central);
  // Amuletos.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_AMULETO,
      gerador.lista_amuletos, gerador.botao_usar_amuleto, gerador.botao_adicionar_amuleto, gerador.botao_remover_amuleto,
      gerador.botao_doar_amuleto,
      proto, proto_retornado, central);
  // Mantos.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_MANTO,
      gerador.lista_mantos, gerador.botao_usar_manto, gerador.botao_adicionar_manto, gerador.botao_remover_manto,
      gerador.botao_doar_manto,
      proto, proto_retornado, central);
  // Bracadeiras.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_BRACADEIRAS,
      gerador.lista_bracadeiras, gerador.botao_usar_bracadeiras, gerador.botao_adicionar_bracadeiras, gerador.botao_remover_bracadeiras,
      gerador.botao_doar_bracadeiras,
      proto, proto_retornado, central);
  // Chapeu.
  ConfiguraListaItensMagicos(
      dialogo, tabelas, gerador, f_atualiza_ui, ent::TipoItem::TIPO_CHAPEU,
      gerador.lista_chapeus, gerador.botao_vestir_chapeu, gerador.botao_adicionar_chapeu, gerador.botao_remover_chapeu,
      gerador.botao_doar_chapeu,
      proto, proto_retornado, central);

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

void PreencheConfiguraMovimento(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  // Atualiza os campos.
  auto* mov = proto_retornado->mutable_movimento();
  std::vector<std::tuple<QSpinBox*, std::function<void(int)>, QPushButton*, ent::Bonus*>> tuplas = {
    std::make_tuple(
        gerador.spin_mov_terrestre, [mov](int v) { mov->set_terrestre_basico_q(v); },
        gerador.botao_mov_terrestre, proto_retornado->mutable_movimento()->mutable_terrestre_q()),
    std::make_tuple(
        gerador.spin_mov_aereo, [mov](int v) { mov->set_aereo_basico_q(v); },
        gerador.botao_mov_aereo, proto_retornado->mutable_movimento()->mutable_aereo_q()),
    std::make_tuple(
        gerador.spin_mov_nadando, [mov](int v) { mov->set_aquatico_basico_q(v); },
        gerador.botao_mov_nadando, proto_retornado->mutable_movimento()->mutable_aquatico_q()),
    std::make_tuple(
        gerador.spin_mov_escavando, [mov](int v) { mov->set_escavando_basico_q(v); },
        gerador.botao_mov_escavando, proto_retornado->mutable_movimento()->mutable_escavando_q()),
    std::make_tuple(
        gerador.spin_mov_escalando, [mov](int v) { mov->set_escalando_basico_q(v); },
        gerador.botao_mov_escalando, proto_retornado->mutable_movimento()->mutable_escalando_q()),
  };
  for (auto& t : tuplas) {
    QSpinBox* spin;
    std::function<void(int)> setter;
    QPushButton* botao;
    ent::Bonus* bonus;
    std::tie(spin, setter, botao, bonus) = t;
    lambda_connect(spin, SIGNAL(valueChanged(int)), [this_, &gerador, spin, setter, proto_retornado] () {
      setter(spin->value());
      ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
      AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
    });
    lambda_connect(botao, SIGNAL(clicked()), [this_, &gerador, botao, bonus, proto_retornado]() {
      AbreDialogoBonus(this_, bonus);
      ent::RecomputaDependencias(this_->tabelas(), proto_retornado);
      AtualizaUI(this_->tabelas(), gerador, *proto_retornado);
    });
  }
  AtualizaUIMovimento(this_->tabelas(), gerador, proto);
}

void PreencheConfiguraDadosDefesa(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  AtualizaUIAtaquesDefesa(this_->tabelas(), gerador, proto);
  // Imune critico.
  gerador.checkbox_imune_critico->setCheckState(proto.dados_defesa().imune_critico() ? Qt::Checked : Qt::Unchecked);

  lambda_connect(gerador.botao_resistencia_magia, SIGNAL(clicked()), [this_, &gerador, proto_retornado] () {
    AbreDialogoBonus(this_, proto_retornado->mutable_dados_defesa()->mutable_resistencia_magia_variavel());
    AtualizaUIAtaquesDefesa(this_->tabelas(), gerador, *proto_retornado);
  });

  auto* mdd = proto_retornado->mutable_dados_defesa();
  auto* mca = mdd->mutable_ca();
  const ent::Tabelas& tabelas = this_->tabelas();
  lambda_connect(gerador.combo_armadura, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, mca] () {
    QComboBox* combo = gerador.combo_armadura;
    std::string id = combo->itemData(combo->currentIndex()).toString().toStdString();
    proto_retornado->mutable_dados_defesa()->set_id_armadura(id);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.combo_escudo, SIGNAL(currentIndexChanged(int)), [&tabelas, &gerador, proto_retornado, mca] () {
    QComboBox* combo = gerador.combo_escudo;
    std::string id = combo->itemData(combo->currentIndex()).toString().toStdString();
    proto_retornado->mutable_dados_defesa()->set_id_escudo(id);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.spin_ca_armadura_melhoria, SIGNAL(valueChanged(int)), [tabelas, &gerador, proto_retornado, mdd] () {
    mdd->set_bonus_magico_armadura(gerador.spin_ca_armadura_melhoria->value());
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.checkbox_armadura_obra_prima, SIGNAL(stateChanged(int)), [tabelas, &gerador, proto_retornado, mdd]() {
    mdd->set_armadura_obra_prima(gerador.checkbox_armadura_obra_prima->checkState() == Qt::Checked);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.spin_ca_escudo_melhoria, SIGNAL(valueChanged(int)), [tabelas, &gerador, proto_retornado, mdd] () {
    mdd->set_bonus_magico_escudo(gerador.spin_ca_escudo_melhoria->value());
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.checkbox_escudo_obra_prima, SIGNAL(stateChanged(int)), [tabelas, &gerador, proto_retornado, mdd]() {
    mdd->set_escudo_obra_prima(gerador.checkbox_escudo_obra_prima->checkState() == Qt::Checked);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
  lambda_connect(gerador.botao_bonus_ca, SIGNAL(clicked()), [tabelas, this_, &gerador, proto_retornado, mca] () {
    AbreDialogoBonus(this_, mca);
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
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
    const bool pergaminho = tipo_ataque.find("Pergaminho") == 0;
    gerador.combo_arma->setEnabled(
        tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância" || tipo_ataque == "Projétil de Área" ||
        tipo_ataque.find("Feitiço de ") == 0 || pergaminho);
    gerador.combo_material_arma->setEnabled(
        tipo_ataque == "Ataque Corpo a Corpo" || tipo_ataque == "Ataque a Distância");
    EditaAtualizaUIAtaque();
    gerador.spin_nivel_conjurador_pergaminho->setEnabled(pergaminho);
    gerador.spin_modificador_atributo_pergaminho->setEnabled(pergaminho);
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
      gerador.lista_ataques->setCurrentRow(-1, QItemSelectionModel::Clear);
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
    gerador.lista_ataques->setCurrentRow(proto_retornado->dados_ataque().size() - 1, QItemSelectionModel::Clear);
  });

  lambda_connect(gerador.botao_ataque_cima, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice <= 0 || indice >= proto_retornado->dados_ataque().size() ||
        proto_retornado->dados_ataque().size() <= 1 || indice >= proto_retornado->dados_ataque().size()) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice - 1));
    gerador.lista_ataques->setCurrentRow(indice - 1, QItemSelectionModel::Clear);
  });
  lambda_connect(gerador.botao_ataque_baixo, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size() - 1 ||
        proto_retornado->dados_ataque().size() <= 1) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice + 1));
    gerador.lista_ataques->setCurrentRow(indice + 1, QItemSelectionModel::Clear);
  });

  lambda_connect(gerador.botao_remover_ataque, SIGNAL(clicked()), [&tabelas, &gerador, proto_retornado] () {
    auto lista_itens = gerador.lista_ataques->selectedItems();
    std::set<int, std::greater<int>> a_remover;
    for (auto* item : lista_itens) {
      const int indice = gerador.lista_ataques->row(item);
      if (indice == -1 || indice >= proto_retornado->dados_ataque().size()) continue;
      a_remover.insert(indice);
    }
    for (int indice : a_remover) {
      proto_retornado->mutable_dados_ataque()->DeleteSubrange(indice, 1);
    }
    gerador.lista_ataques->setCurrentRow(-1, QItemSelectionModel::Clear);
  });
  // Ao adicionar aqui, adicione nos sinais bloqueados tb (blockSignals). Exceto para textEdited, que nao dispara sinal programaticamente.
  lambda_connect(gerador.linha_grupo_ataque, SIGNAL(editingFinished()), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.linha_rotulo_ataque, SIGNAL(editingFinished()), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.linha_dano, SIGNAL(editingFinished()), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );  // nao pode refrescar no meio pois tem processamento da string.
  lambda_connect(gerador.spin_bonus_magico, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_municao, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_limite_vezes, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_ordem_ataque, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_op, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_ignora_rm, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_permite_salvacao, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_ataque_agarrar, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.checkbox_ataque_toque, SIGNAL(stateChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.combo_empunhadura, SIGNAL(currentIndexChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_incrementos, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_alcance_quad, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_nivel_conjurador_pergaminho, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
  lambda_connect(gerador.spin_modificador_atributo_pergaminho, SIGNAL(valueChanged(int)), [EditaAtualizaUIAtaque]() { EditaAtualizaUIAtaque(); } );
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

void LeDominioOuEscolaDoCombo(const QComboBox* combo, string* dominio) {
  const std::string& id = combo->itemData(combo->currentIndex()).toString().toStdString();
  if (id == "nenhum") {
    dominio->clear();
  } else {
    *dominio = id;
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
  const auto& classe_tabelada = tabelas.Classe(gerador.linha_classe->text().toStdString());
  ic->set_id(gerador.linha_classe->text().toStdString());
  ic->set_nivel(gerador.spin_nivel_classe->value());
  ic->set_nivel_conjurador(gerador.spin_nivel_conjurador->value());
  ic->set_bba(gerador.spin_bba->value());
  ic->set_atributo_conjuracao(static_cast<ent::TipoAtributo>(gerador.combo_mod_conjuracao->currentIndex()));
  ic->clear_salvacoes_fortes();
  for (auto ts : ComboParaSalvacoesFortes(gerador.combo_salvacoes_fortes)) {
    ic->add_salvacoes_fortes(ts);
  }
  auto* fc = ent::FeiticosClasse(classe_tabelada.id(), proto_retornado);
  if (classe_tabelada.possui_dominio()) {
    Redimensiona(2, fc->mutable_dominios());
    LeDominioDoCombo(gerador.combo_dominio_1, fc->mutable_dominios(0));
    LeDominioDoCombo(gerador.combo_dominio_2, fc->mutable_dominios(1));
  } else {
    fc->clear_dominios();
  }
  if (gerador.combo_especializacao_escola->) {
    Redimensiona(2, fc->escolas_proibidas());
    LeDominioDoCombo(gerador.combo_escolas_proibidas_1, fc->mutable_escolas_proibidas(0));
    LeDominioDoCombo(gerador.combo_escolas_proibidas_2, fc->mutable_escolas_proibidas(1));
  } else {
    fc->clear_escolas_proibidas();
    fc->clear_especializacao();
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

void PreencheConfiguraCombosDominio(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  std::vector<QComboBox*> combos = {gerador.combo_dominio_1, gerador.combo_dominio_2};
  for (auto* combo : combos) {
    combo->addItem(combo->tr("Nenhum"), "nenhum");
    combo->insertSeparator(combo->count());

    std::map<QString, std::string> dominios_ordenados;
    for (const auto& dominio : tabelas.todas().tabela_dominios().dominios()) {
      dominios_ordenados[combo->tr(dominio.nome().c_str())] = dominio.id();
    }
    for (const auto it : dominios_ordenados) {
      combo->addItem(it.first, QVariant(QString::fromStdString(it.second)));
    }
    ExpandeComboBox(combo);
    const int indice_dominio = combo == gerador.combo_dominio_1 ? 0 : 1;
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, indice_dominio, &tabelas, &gerador, proto_retornado] () {
      const int indice = gerador.lista_niveis->currentRow();
      if (gerador.linha_classe->text().isEmpty()) {
        return;
      }
      ent::InfoClasse* ic = (indice < 0 || indice >= proto_retornado->info_classes().size())
          ? nullptr : proto_retornado->mutable_info_classes(indice);
      if (ic == nullptr) {
        // Pode acontecer quando a classe ainda nao foi adicionada.
        combo->setToolTip("");
        return;
      }
      combo->blockSignals(true);
      auto* fc = ent::FeiticosClasse(ic->id(), proto_retornado);
      if (fc->dominios().size() != 2) {
        Redimensiona(2, fc->mutable_dominios());
      }
      QVariant data = combo->itemData(combo->currentIndex());
      if (data == "nenhum") {
        fc->mutable_dominios(indice_dominio)->clear();
        combo->setToolTip("");
      } else {
        string id = data.toString().toStdString();
        *fc->mutable_dominios(indice_dominio) = id;
        combo->setToolTip(combo->tr(tabelas.Dominio(id).descricao().c_str()));
      }
      ent::RecomputaDependencias(tabelas, proto_retornado);
      AtualizaUI(tabelas, gerador, *proto_retornado);
      combo->blockSignals(false);
    });
    combo->setEnabled(false);
  }
}

void PreencheConfiguraCombosEspecializacaoEscola(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  std::vector<QComboBox*> combos = {gerador.combo_especializacao_escola, gerador.combo_escola_proibida_1, gerador.escola_proibida_2};
  for (auto* combo : combos) {
    combo->addItem(combo->tr("Nenhuma"), "nenhuma");
    combo->insertSeparator(combo->count());

    std::map<QString, std::string> escolas_ordenadas;
    for (const auto& [nome, id]
            : {{"abjuração", "abjuracao"},
               {"adivinhação", "adivinhacao"},
               {"conjuração", "conjuracao"},
               {"encantamento", "encantamento"},
               {"evocação", "evocacao"},
               {"ilusão", "ilusao"},
               {"necromancia", "necromancia"},
               {"transmutação", "transmutacao"}}) {
      escolas_ordenadas[combo->tr(nome.c_str())] = id;
    }
    combo->clear();
    for (const auto& [nome, id] : escolas_ordenadas) {
      combo->addItem(nome, QVariant(QString::fromStdString(id)));
    }
    ExpandeComboBox(combo);
    if (combo == gerador.combo_especializacao_escola) {
      lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, &tabelas, &gerador, proto_retornado] () {
        const int indice = gerador.lista_niveis->currentRow();
        if (gerador.linha_classe->text().isEmpty()) {
          return;
        }
        ent::InfoClasse* ic = (indice < 0 || indice >= proto_retornado->info_classes().size())
            ? nullptr : proto_retornado->mutable_info_classes(indice);
        if (ic == nullptr) {
          // Pode acontecer quando a classe ainda nao foi adicionada.
          combo->setToolTip("");
          return;
        }
        combo->blockSignals(true);
        auto* fc = ent::FeiticosClasse(ic->id(), proto_retornado);
        QVariant data = combo->itemData(combo->currentIndex());
        if (data == "nenhuma") {
          fc->clear_especializacao_escola();
        } else {
          string id = data.toString().toStdString();
          fc->set_especializacao_escola(id);
        }
        ent::RecomputaDependencias(tabelas, proto_retornado);
        AtualizaUI(tabelas, gerador, *proto_retornado);
        combo->blockSignals(false);
      });
    } else {
      const int indice_escola_proibida = combo == gerador.combo_escola_proibida_1 ? 0 : 1;
      lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, indice_escola_proibida, &tabelas, &gerador, proto_retornado] () {
        const int indice = gerador.lista_niveis->currentRow();
        if (gerador.linha_classe->text().isEmpty()) {
          return;
        }
        ent::InfoClasse* ic = (indice < 0 || indice >= proto_retornado->info_classes().size())
            ? nullptr : proto_retornado->mutable_info_classes(indice);
        if (ic == nullptr) {
          // Pode acontecer quando a classe ainda nao foi adicionada.
          combo->setToolTip("");
          return;
        }
        combo->blockSignals(true);
        auto* fc = ent::FeiticosClasse(ic->id(), proto_retornado);
        if (fc->escolas_proibidas().size() != 2) {
          Redimensiona(2, fc->escolas_proibidas());
        }
        QVariant data = combo->itemData(combo->currentIndex());
        if (data == "nenhuma") {
          fc->mutable_escolas_proibidas(indice_escola_proibida)->clear();
        } else {
          string id = data.toString().toStdString();
          *fc->mutable_escolas_proibidas(indice_escola_proibida) = id;
        }
        ent::RecomputaDependencias(tabelas, proto_retornado);
        AtualizaUI(tabelas, gerador, *proto_retornado);
        combo->blockSignals(false);
      });
    }
    combo->setEnabled(false);
  }
}

void PreencheConfiguraComboClasse(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto* combo = gerador.combo_classe;
  combo->addItem(combo->tr("Outro"), "outro");
  combo->insertSeparator(combo->count());

  std::unordered_map<int, std::vector<const ent::InfoClasse*>> classes_por_tipo;
  for (const auto& ic : tabelas.todas().tabela_classes().info_classes()) {
    classes_por_tipo[ic.tipo_classe()].push_back(&ic);
  }
  for (auto& tipo_classes : classes_por_tipo) {
    auto& classes = tipo_classes.second;
    std::sort(classes.begin(), classes.end(), [combo, &tabelas](const ent::InfoClasse* lhs, const ent::InfoClasse* rhs) {
      return combo->tr(lhs->nome().c_str()) < combo->tr(rhs->nome().c_str());
    });
  }

  for (const auto* ic : classes_por_tipo[ent::TC_BASICA]) {
    combo->addItem(combo->tr(ic->nome().c_str()), ic->id().c_str());
  }
  combo->insertSeparator(combo->count());
  for (const auto* ic : classes_por_tipo[ent::TC_PRESTIGIO]) {
    combo->addItem(combo->tr(ic->nome().c_str()), ic->id().c_str());
  }
  combo->insertSeparator(combo->count());
  for (const auto* ic : classes_por_tipo[ent::TC_PDM]) {
    combo->addItem(combo->tr(ic->nome().c_str()), ic->id().c_str());
  }
  combo->insertSeparator(combo->count());
  for (const auto* ic : classes_por_tipo[ent::TC_MONSTRO]) {
    combo->addItem(combo->tr(ic->nome().c_str()), ic->id().c_str());
  }

  ExpandeComboBox(combo);
  ExpandeComboBox(gerador.combo_mod_conjuracao);
  lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, &tabelas, &gerador, proto_retornado] () {
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
    gerador.combo_dominio_1->setEnabled(classe_tabelada.possui_dominio());
    gerador.combo_dominio_2->setEnabled(classe_tabelada.possui_dominio());
    ent::RecomputaDependencias(tabelas, proto_retornado);
    AtualizaUI(tabelas, gerador, *proto_retornado);
  });
}

void PreencheConfiguraComboRaca(
    const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto* combo = gerador.combo_raca;
  for (const auto& ir : tabelas.todas().tabela_racas().racas()) {
    combo->addItem(QString::fromUtf8(ir.nome().c_str()), ir.id().c_str());
  }
  ExpandeComboBox(combo);
  lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [combo, &tabelas, &gerador, proto_retornado] () {
    const auto& raca_tabelada = tabelas.Raca(combo->itemData(combo->currentIndex()).toString().toStdString());
    proto_retornado->set_raca(raca_tabelada.id());
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
    ent::AtribuiOuRemoveBonus(
        std::max(0, gerador.spin_niveis_negativos->value()), ent::TB_BASE, "base",
        proto_retornado->mutable_niveis_negativos_dinamicos());
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
  PreencheConfiguraComboRaca(tabelas, gerador, proto_retornado);
  PreencheConfiguraCombosDominio(tabelas, gerador, proto_retornado);
  PreencheConfiguraCombosEspecializacaoEscola(tabelas, gerador, proto_retornado);

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
    const ntf::Notificacao& notificacao, bool forma_corrente, QWidget* pai) {
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

  // Inimigos Prediletos.
  PreencheConfiguraInimigosPrediletos(this, gerador, entidade, proto_retornado);

  // Evasao estatica e dimamica.
  PreencheConfiguraEvasao(this, gerador, entidade, proto_retornado);
  // Esquiva sobrenatural.
  PreencheConfiguraEsquivaSobrenatural(this, gerador, entidade, proto_retornado);

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

  std::function<void(const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto)> f_atualiza_ui =
      [](const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
    AtualizaUI(tabelas, gerador, proto);
  };
  PreencheConfiguraTesouro(this, dialogo, gerador, f_atualiza_ui, entidade, proto_retornado, central_);

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
  PreencheConfiguraMovimento(this, gerador, entidade, proto_retornado);

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
                 [this, notificacao, &entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor, forma_corrente] () {
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
    if (forma_corrente &&
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
    proto_retornado->mutable_tesouro()->mutable_moedas()->set_po(gerador.spin_po->value());
    proto_retornado->mutable_tesouro()->mutable_moedas()->set_pp(gerador.spin_pp->value());
    proto_retornado->mutable_tesouro()->mutable_moedas()->set_pc(gerador.spin_pc->value());
    proto_retornado->mutable_tesouro()->mutable_moedas()->set_pl(gerador.spin_pl->value());
    proto_retornado->mutable_tesouro()->mutable_moedas()->set_pe(gerador.spin_pe->value());
  });
  // TODO: Ao aplicar as mudanças refresca e nao fecha.

  // Cancelar.
  lambda_connect(dialogo, SIGNAL(rejected()), [&notificacao, &delete_proto_retornado] {
      delete_proto_retornado.reset();
  });
  QSize tam = qobject_cast<QApplication*>(QApplication::instance())->primaryScreen()->size();
  if (tam.width() < dialogo->size().width() || tam.height() < dialogo->size().height()) {
    dialogo->showMaximized();
    LOG(INFO) << "desktop: " << tam.width() << "x" << tam.height() << ", dialogo: " << dialogo->size().width() << "x" << dialogo->size().height();
  }
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
  gerador.tabela_bonus->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  gerador.tabela_bonus->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  gerador.tabela_bonus->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

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
  PreencheComboTextura(tab_proto.info_textura_piso().id().c_str(), notificacao.tabuleiro().id_cliente(), ent::FiltroTexturaTabuleiro, gerador.combo_fundo);
  // Ceu do tabuleiro.
  PreencheComboTexturaCeu(tab_proto.info_textura_ceu().id().c_str(), notificacao.tabuleiro().id_cliente(), gerador.combo_ceu);
  gerador.checkbox_luz_ceu->setCheckState(tab_proto.aplicar_luz_ambiente_textura_ceu() ? Qt::Checked : Qt::Unchecked);


  // Combos de heranca.
  PreencheComboCenarioPai(tabuleiro_->Proto(), gerador.combo_herdar_piso);
  if (tab_proto.has_herdar_piso_de()) {
    gerador.combo_herdar_piso->setCurrentIndex(gerador.combo_herdar_piso->findData(QVariant(tab_proto.herdar_piso_de())));
  }
  PreencheComboCenarioPai(tabuleiro_->Proto(), gerador.combo_herdar_ceu);
  if (tab_proto.has_herdar_ceu_de()) {
    gerador.combo_herdar_ceu->setCurrentIndex(gerador.combo_herdar_ceu->findData(QVariant(tab_proto.herdar_ceu_de())));
  }
  PreencheComboCenarioPai(tabuleiro_->Proto(), gerador.combo_herdar_iluminacao);
  if (tab_proto.has_herdar_iluminacao_de()) {
    gerador.combo_herdar_iluminacao->setCurrentIndex(gerador.combo_herdar_iluminacao->findData(QVariant(tab_proto.herdar_iluminacao_de())));
  }
  PreencheComboCenarioPai(tabuleiro_->Proto(), gerador.combo_herdar_nevoa);
  if (tab_proto.has_herdar_nevoa_de()) {
    gerador.combo_herdar_nevoa->setCurrentIndex(gerador.combo_herdar_nevoa->findData(QVariant(tab_proto.herdar_nevoa_de())));
  }

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
    // Descricao.
    proto_retornado->set_descricao_cenario(gerador.campo_descricao->text().toStdString());
    // Nevoa.
    if (gerador.combo_herdar_nevoa->currentData().toInt() != CENARIO_INVALIDO) {
      proto_retornado->set_herdar_nevoa_de(gerador.combo_herdar_nevoa->currentData().toInt());
      proto_retornado->clear_nevoa();
    } else {
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
    }
    // Textura de piso.
    if (gerador.combo_herdar_piso->currentData().toInt() != CENARIO_INVALIDO) {
      proto_retornado->set_herdar_piso_de(gerador.combo_herdar_piso->currentData().toInt());
      proto_retornado->clear_info_textura_piso();
      proto_retornado->clear_ladrilho();
      proto_retornado->clear_cor_piso();
    } else {
      proto_retornado->clear_herdar_piso_de();
      // Textura.
      if (gerador.combo_fundo->currentIndex() == 0) {
        proto_retornado->clear_info_textura_piso();
      } else {
        PreencheTexturaProtoRetornado(tab_proto.info_textura_piso(), gerador.combo_fundo, proto_retornado->mutable_info_textura_piso());
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
    }
    // Textura ceu.
    if (gerador.combo_herdar_ceu->currentData().toInt() != CENARIO_INVALIDO) {
      proto_retornado->set_herdar_ceu_de(gerador.combo_herdar_ceu->currentData().toInt());
    } else {
      proto_retornado->clear_herdar_ceu_de();
      if (gerador.combo_ceu->currentIndex() == 0) {
        proto_retornado->clear_info_textura_ceu();
      } else {
        PreencheTexturaProtoRetornado(tab_proto.info_textura_ceu(), gerador.combo_ceu, proto_retornado->mutable_info_textura_ceu());
      }
    }
    // Iluminacao.
    if (gerador.combo_herdar_iluminacao->currentData().toInt() != CENARIO_INVALIDO) {
      proto_retornado->set_herdar_iluminacao_de(gerador.combo_herdar_iluminacao->currentData().toInt());
      proto_retornado->clear_luz_ambiente();
      proto_retornado->clear_luz_direcional();
      proto_retornado->clear_aplicar_luz_ambiente_textura_ceu();
    } else {
      proto_retornado->mutable_luz_direcional()->set_posicao_graus(gerador.dial_posicao->sliderPosition() - 90.0f);
      proto_retornado->mutable_luz_direcional()->set_inclinacao_graus(gerador.dial_inclinacao->sliderPosition() - 90.0f);
      proto_retornado->mutable_luz_direcional()->mutable_cor()->Swap(&cor_direcional_proto);
      proto_retornado->mutable_luz_ambiente()->Swap(&cor_ambiente_proto);
      proto_retornado->set_aplicar_luz_ambiente_textura_ceu(gerador.checkbox_luz_ceu->checkState() == Qt::Checked);
    }
    // Tamanho do tabuleiro.
    if (gerador.checkbox_tamanho_automatico->checkState() == Qt::Checked) {
      int indice = gerador.combo_fundo->currentIndex();
      arq::tipo_e tipo = static_cast<arq::tipo_e>(gerador.combo_fundo->itemData(indice).toInt());
      // Busca tamanho da textura. Copia o objeto aqui porque a funcao PreencheInfoTextura o modifica.
      ent::InfoTextura textura = proto_retornado->info_textura_piso();
      unsigned int largura = 0, altura = 0;
      std::string nome;
      if (tipo == arq::TIPO_TEXTURA_LOCAL) {
        nome = gerador.combo_fundo->itemText(indice).split(":")[1].toStdString();
      } else {
        nome = gerador.combo_fundo->itemText(indice).toStdString();
      }
      ent::PreencheInfoTextura(nome, tipo, &textura, &largura, &altura);
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
  // Oclusao.
  gerador.checkbox_mapeamento_oclusao->setCheckState(opcoes_proto.mapeamento_oclusao() ? Qt::Checked : Qt::Unchecked);
  // Ataque vs defesa posicao real.
  gerador.checkbox_ataque_vs_defesa_posicao_real->setCheckState(opcoes_proto.ataque_vs_defesa_posicao_real() ? Qt::Checked : Qt::Unchecked);
  // Tab ativa acao de ataque.
  gerador.checkbox_tab_ativa_ataque->setCheckState(opcoes_proto.tab_ativa_ataque() ? Qt::Checked : Qt::Unchecked);

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
    proto_retornado->clear_iluminacao_por_pixel();
    proto_retornado->set_tipo_iluminacao(ent::OpcoesProto::TI_ESPECULAR);
    proto_retornado->set_mapeamento_oclusao(
        gerador.checkbox_mapeamento_oclusao->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_ataque_vs_defesa_posicao_real(
        gerador.checkbox_ataque_vs_defesa_posicao_real->checkState() == Qt::Checked ? true : false);
    proto_retornado->set_tab_ativa_ataque(
        gerador.checkbox_tab_ativa_ataque->checkState() == Qt::Checked ? true : false);
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

// Funcoes para Dialogo tipo forma.

namespace {
bool PermiteMudarForma(const ent::EntidadeProto& proto) {
  if (proto.tipo() != ent::TE_FORMA) return false;
  switch (proto.sub_tipo()) {
    case ent::TF_CILINDRO:
    case ent::TF_CONE:
    case ent::TF_CUBO:
    case ent::TF_ESFERA:
    case ent::TF_PIRAMIDE:
    case ent::TF_HEMISFERIO:
      return true;
    default:
      return false;
  }
}

int SubTipoParaIndice(ent::TipoForma sub_tipo) {
  switch (sub_tipo) {
    case ent::TF_CILINDRO: return 0;
    case ent::TF_CONE: return 1;
    case ent::TF_CUBO: return 2;
    case ent::TF_ESFERA: return 3;
    case ent::TF_PIRAMIDE: return 4;
    case ent::TF_HEMISFERIO: return 5;
    default:
      return -1;
  }
}

ent::TipoForma IndiceParaSubTipo(int indice) {
  switch (indice) {
    case 0: return ent::TF_CILINDRO;
    case 1: return ent::TF_CONE;
    case 2: return ent::TF_CUBO;
    case 3: return ent::TF_ESFERA;
    case 4: return ent::TF_PIRAMIDE;
    case 5: return ent::TF_HEMISFERIO;
    default:
      LOG(ERROR) << "indice invalido, retornando cilindro: " << indice;
      return ent::TF_CILINDRO;
  }
}

void AjustaSliderSpin(float angulo, QDial* dial, QSpinBox* spin) {
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
}


}  // namespace

ent::EntidadeProto* Visualizador3d::AbreDialogoTipoForma(const ntf::Notificacao& notificacao) {
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

  // Combo de forma.
  const bool permite_mudar_forma = PermiteMudarForma(entidade);
  if (permite_mudar_forma) {
    gerador.combo_tipo_forma->setCurrentIndex(SubTipoParaIndice(entidade.sub_tipo()));
    gerador.combo_tipo_forma->setEnabled(true);
  } else {
    gerador.combo_tipo_forma->setEnabled(false);
  }

  gerador.checkbox_afetado_por_efeitos->setCheckState(entidade.pode_ser_afetada_por_acao() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_respeita_solo->setCheckState(entidade.forcar_respeita_solo() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_ignora_luz->setCheckState(entidade.ignora_luz() ? Qt::Checked : Qt::Unchecked);

  // Visibilidade.
  gerador.checkbox_visibilidade->setCheckState(entidade.visivel() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_faz_sombra->setCheckState(entidade.faz_sombra() ? Qt::Checked : Qt::Unchecked);
  gerador.checkbox_dois_lados->setCheckState(entidade.dois_lados() ? Qt::Checked : Qt::Unchecked);

  // Especularidade.
  gerador.checkbox_especular->setCheckState(entidade.especular() ? Qt::Checked : Qt::Unchecked);

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

  // Fumegando.
  gerador.checkbox_fumegando->setCheckState(entidade.fumegando() ? Qt::Checked : Qt::Unchecked);

  // Textura do objeto.
  PreencheComboTextura(entidade.info_textura().id(),
                       notificacao.tabuleiro().id_cliente(),
                       ent::FiltroTexturaEntidade, gerador.combo_textura);
  gerador.checkbox_ladrilho->setCheckState(
      entidade.info_textura().has_modo_textura()
      ? (entidade.info_textura().modo_textura() == GL_REPEAT ? Qt::Checked : Qt::Unchecked)
      : Qt::Unchecked);
  gerador.checkbox_bump->setCheckState(
      entidade.info_textura().textura_bump() ? Qt::Checked : Qt::Unchecked);
  gerador.spin_tex_periodo->setValue(entidade.info_textura().periodo_s());
  gerador.spin_tex_escala_x->setValue(entidade.info_textura().escala_x());
  gerador.spin_tex_escala_y->setValue(entidade.info_textura().escala_y());
  AjustaSliderSpin(entidade.info_textura().direcao_graus(), gerador.dial_tex_direcao, gerador.spin_tex_direcao);

  // Cor da entidade.
  ent::EntidadeProto ent_cor;
  ent_cor.mutable_cor()->CopyFrom(entidade.cor());
  gerador.spin_raio_quad->setValue(ent::METROS_PARA_QUADRADOS * (entidade.luz().has_raio_m() ? entidade.luz().raio_m() : 6.0f));
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
    if (gerador.spin_raio_quad->value() == 0.0) {
      gerador.spin_raio_quad->setValue(ent::METROS_PARA_QUADRADOS * 6.0f);
    }
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

  std::function<void(const ent::Tabelas&, ifg::qt::Ui::DialogoForma&, const ent::EntidadeProto&)> f_atualiza_ui =
    [](const ent::Tabelas& tabelas, ifg::qt::Ui::DialogoForma& gerador, const ent::EntidadeProto& proto) {
      // Tem que fazer a funcao de atualiza funcionar com tesouros...
    AtualizaUITesouro(tabelas, gerador, proto);
  };
  PreencheConfiguraTesouro(this, dialogo, gerador, f_atualiza_ui, entidade, proto_retornado, central_);

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor, permite_mudar_forma ] () {
    if (permite_mudar_forma) {
      proto_retornado->set_sub_tipo(IndiceParaSubTipo(gerador.combo_tipo_forma->currentIndex()));
    }
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
      if (gerador.spin_raio_quad->value() > 0.0f) {
        proto_retornado->mutable_luz()->set_raio_m(gerador.spin_raio_quad->value() * ent::QUADRADOS_PARA_METROS);
      } else {
        proto_retornado->mutable_luz()->clear_raio_m();
      }
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
    if (gerador.checkbox_especular->checkState() == Qt::Checked) {
      proto_retornado->set_especular(true);
    } else {
      proto_retornado->clear_especular();
    }
    if (gerador.checkbox_respeita_solo->checkState() == Qt::Checked) {
      proto_retornado->set_forcar_respeita_solo(true);
    } else {
      proto_retornado->clear_forcar_respeita_solo();
    }
    if (gerador.checkbox_ignora_luz->checkState() == Qt::Checked) {
      proto_retornado->set_ignora_luz(true);
    } else {
      proto_retornado->clear_ignora_luz();
    }
    if (gerador.checkbox_fumegando->checkState() == Qt::Checked) {
      proto_retornado->set_fumegando(true);
    } else {
      proto_retornado->clear_fumegando();
    }

    proto_retornado->set_pode_ser_afetada_por_acao(gerador.checkbox_afetado_por_efeitos->checkState() == Qt::Checked);
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
      proto_retornado->mutable_info_textura()->set_textura_bump(gerador.checkbox_bump->checkState() == Qt::Checked);
      proto_retornado->mutable_info_textura()->set_periodo_s(gerador.spin_tex_periodo->value());
      proto_retornado->mutable_info_textura()->set_escala_x(gerador.spin_tex_escala_x->value());
      proto_retornado->mutable_info_textura()->set_escala_y(gerador.spin_tex_escala_y->value());
      proto_retornado->mutable_info_textura()->set_direcao_graus(-gerador.dial_tex_direcao->sliderPosition() + 180.0f);
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


}  // namespace qt
}  // namespace ifg
