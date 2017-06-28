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
#include <QItemDelegate>
#include <QMessageBox>
#include <QMouseEvent>
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
#include "ifg/qt/constantes.h"
#include "ifg/qt/ui/bonus_individual.h"
#include "ifg/qt/ui/dialogo_bonus.h"
#include "ifg/qt/ui/entidade.h"
#include "ifg/qt/texturas.h"
#include "ifg/qt/util.h"
#include "ifg/qt/ui/dialogobonus.h"
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

constexpr int ULTIMO_TIPO_VALIDO = 12;
int TipoParaIndice(const std::string& tipo_str) {
  // Os tipos sao encontrados no arquivo dados/acoes.asciiproto.
  // Os indices sao na ordem definida pela UI.
  if (tipo_str == "Ácido") {
    return 0;
  } else if (tipo_str == "Ataque Corpo a Corpo") {
    return 1;
  } else if (tipo_str == "Ataque a Distância") {
    return 2;
  } else if (tipo_str == "Bola de Fogo") {
    return 3;
  } else if (tipo_str == "Coluna de Chamas") {
    return 4;
  } else if (tipo_str == "Cone de Gelo") {
    return 5;
  } else if (tipo_str == "Feitiço de Toque") {
    return 6;
  } else if (tipo_str == "Fogo Alquímico") {
    return 7;
  } else if (tipo_str == "Mãos Flamejantes") {
    return 8;
  } else if (tipo_str == "Míssil Mágico") {
    return 9;
  } else if (tipo_str == "Pedrada (gigante)") {
    return 10;
  } else if (tipo_str == "Raio") {
    return 11;
  } else if (tipo_str == "Relâmpago") {
    return 12;
  } else if (tipo_str == "Tempestade Glacial") {
    return 13;
  } else {
    return 14;
  }
}

std::string IndiceParaTipo(int indice) {
  // Os tipos sao encontrados no arquivo dados/acoes.asciiproto.
  // Os indices sao na ordem definida pela UI.
  switch (indice) {
    case 0: return "Ácido";
    case 1: return "Ataque Corpo a Corpo";
    case 2: return "Ataque a Distância";
    case 3: return "Bola de Fogo";
    case 4: return "Coluna de Chamas";
    case 5: return "Cone de Gelo";
    case 6: return "Feitiço de Toque";
    case 7: return "Fogo Alquímico";
    case 8: return "Mãos Flamejantes";
    case 9: return "Míssil Mágico";
    case 10: return "Pedrada (gigante)";
    case 11: return "Raio";
    case 12: return "Relâmpago";
    case 13: return "Tempestade Glacial";
    default: return "Ataque Corpo a Corpo";
  }
};

// Retorna uma string de estilo para background-color baseada na cor passada.
const QString CorParaEstilo(const QColor& cor) {
  QString estilo_fmt("background-color: rgb(%1, %2, %3);");
  QString estilo = estilo_fmt.arg(cor.red()).arg(cor.green()).arg(cor.blue());
  VLOG(3) << "Retornando estilo: " << estilo.toUtf8().constData();
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
}

void PreencheComboTexturaCeu(const std::string& id_corrente, int id_cliente, QComboBox* combo_textura) {
  combo_textura->addItem(combo_textura->tr("Nenhuma"), QVariant(-1));
  auto NaoEhSkybox = [] (const std::string& nome_arquivo) {
    return nome_arquivo.find("skybox") != 0;
  };
  auto EhCuboSecundario = [] (const std::string& nome_arquivo) {
    return nome_arquivo.find("negx.png") != std::string::npos ||
           nome_arquivo.find("posy.png") != std::string::npos ||
           nome_arquivo.find("negy.png") != std::string::npos ||
           nome_arquivo.find("posz.png") != std::string::npos ||
           nome_arquivo.find("negz.png") != std::string::npos;
  };
  auto FiltraOrdena = [NaoEhSkybox, EhCuboSecundario] (std::vector<std::string> texturas) -> std::vector<std::string> {
    texturas.erase(std::remove_if(texturas.begin(), texturas.end(), NaoEhSkybox), texturas.end());
    texturas.erase(std::remove_if(texturas.begin(), texturas.end(), EhCuboSecundario), texturas.end());
    for (std::string& textura : texturas) {
      std::size_t pos = textura.find("posx.png");
      if (pos != std::string::npos) {
        textura.replace(pos, 8, ".cube");
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
}

// Preenche proto_retornado usando entidade e o combo como base.
void PreencheTexturaProtoRetornado(const ent::InfoTextura& info_antes, const QComboBox* combo_textura,
                                   ent::InfoTextura* info_textura) {
  if (combo_textura->currentIndex() != 0) {
    if (combo_textura->currentText().toUtf8().constData() == info_antes.id()) {
      // Textura igual a anterior.
      VLOG(2) << "Textura igual a anterior.";
      info_textura->set_id(info_antes.id());
    } else {
      VLOG(2) << "Textura diferente da anterior.";
      QString nome(combo_textura->currentText());
      QVariant dados(combo_textura->itemData(combo_textura->currentIndex()));
      if (dados.toInt() == arq::TIPO_TEXTURA_LOCAL) {
        VLOG(2) << "Textura local, recarregando.";
        info_textura->set_id(nome.toUtf8().constData());
        PreencheInfoTextura(nome.split(":")[1].toUtf8().constData(),
            arq::TIPO_TEXTURA_LOCAL, info_textura);
      } else {
        info_textura->set_id(nome.toUtf8().constData());
      }
    }
    VLOG(2) << "Id textura: " << info_textura->id();
  } else {
    info_textura->Clear();
  }
}

}  // namespace

Visualizador3d::Visualizador3d(
    TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QGLWidget(Formato(), pai),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro) {
  const ent::OpcoesProto& opcoes = tabuleiro->Opcoes();
  luz_por_pixel_ = opcoes.iluminacao_por_pixel();
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
      gl::IniciaGl(luz_por_pixel_);
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
      n->set_endereco(file_str.toUtf8().constData());
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
      n->set_endereco(file_str.toUtf8().constData());
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
      std::unique_ptr<ent::OpcoesProto> opcoes(AbreDialogoOpcoes(notificacao));
      if (opcoes.get() == nullptr) {
        VLOG(1) << "Alterações de opcoes descartadas";
        break;
      }
      auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_OPCOES);
      n->mutable_opcoes()->Swap(opcoes.get());
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
    delete event2;
    return;
  }
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      event->x(), height() - event->y());
  event->accept();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  int x = event->globalX();
  int y = event->globalY();
  if (teclado_mouse_->TrataMovimentoMouse(event->x(), height() - event->y())) {
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

int TipoParaIndice(ent::TipoBonus tipo) {
  return tipo;
}

int NumeroLinhas(const ent::Bonus& bonus) {
  int total = 0;
  for (const auto& bi : bonus.bonus_individual()) {
    total += bi.por_origem_size();
  }
  return total;
}

void PreencheComboBonus(ent::TipoBonus tipo, QComboBox* combo) {
  for (int tipo = ent::TipoBonus_MIN; tipo <= ent::TipoBonus_MAX; tipo++) {
    if (!ent::TipoBonus_IsValid(tipo)) continue;
    combo->addItem(ent::TipoBonus_Name(ent::TipoBonus(tipo)).c_str(), QVariant(tipo));
  }
  combo->setCurrentIndex(tipo);
}

// Modelo de bonus para ser usado pelos views de tabela.
class ModeloBonus : public QAbstractTableModel {
 public:
  ModeloBonus(const ent::Bonus& bonus, QTableView* tabela)
      : QAbstractTableModel(tabela), tabela_(tabela), bonus_(bonus) {}

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return NumeroLinhas(bonus_);
  }

  // 0: tipo. 1: origem. 2: valor.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 3;
  }

  bool insertRows(int row, int count, const QModelIndex& parent) override {
    if (count != 1) return false;
    beginInsertRows(parent, 0, 0);
    ent::AtribuiBonus(0, ent::TB_BASE, "origem", &bonus_);
    endInsertRows();
    return true;
  }

  bool removeRows(int row, int count, const QModelIndex& parent) override {
    beginRemoveRows(parent, row, row + count - 1);
    std::vector<std::pair<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*>> bis_pos(count);
    while (count--) {
      auto& bi_po = bis_pos[count];
      std::tie(bi_po.first, bi_po.second) = DadosEm(row);
    }
    for (const auto& bi_po : bis_pos) {
      RemoveBonus(bi_po.first->tipo(), bi_po.second->origem(), &bonus_);
    }
    endRemoveRows();
    return true;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return QVariant();
    }
    switch (section) {
      case 0: return QVariant("Tipo");
      case 1: return QVariant("Origem");
      case 2: return QVariant("Valor");
    }
    return QVariant("Desconhecido");
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
      return QVariant();
    }

    const ent::BonusIndividual* bi;
    const ent::BonusIndividual_PorOrigem* po;
    std::tie(bi, po) = DadosEm(index);
    if (bi == nullptr || po == nullptr) {
      LOG(INFO) << "bi == nullptr?  " << (bi == nullptr ? "YES" : "NO")
                << ", po == nullptr? " << (po == nullptr ? "YES" : "NO");
      return QVariant();
    }
    const int column = index.column();
    switch (column) {
      case 0: return role == Qt::EditRole ? QVariant() : QVariant(ent::TipoBonus_Name(bi->tipo()).c_str());
      case 1: return QVariant(po->origem().c_str());
      case 2: return QVariant(po->valor());
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    if (role != Qt::EditRole) {
      return false;
    }

    ent::BonusIndividual* bi = nullptr;
    ent::BonusIndividual_PorOrigem* po = nullptr;
    std::tie(bi, po) = DadosEm(index);
    if (bi == nullptr || po == nullptr) {
      LOG(INFO) << "bi == nullptr?  " << (bi == nullptr ? "YES" : "NO")
                << ", po == nullptr? " << (po == nullptr ? "YES" : "NO");
      return false;
    }
    const int column = index.column();
    switch (column) {
      case 0: {
        int tipo = value.toInt();
        if (!ent::TipoBonus_IsValid(tipo)) {
          LOG(INFO) << "Tipo de bonus invalido: " << tipo;
          return false;
        }
        if (tipo == bi->tipo()) {
          LOG(INFO) << "Sem mudanca de tipo: " << value.toString().toUtf8().constData();
          return false;
        }
        // Adiciona nova origem.
        ent::AtribuiBonus(po->valor(), ent::TipoBonus(tipo), po->origem(), &bonus_);
        // Remove a origem do tipo corrente.
        RemoveBonus(bi->tipo(), po->origem(), &bonus_);
        LOG(INFO) << "novo proto: " << bonus_.DebugString();
        tabela_->setIndexWidget(index, nullptr);
        return true;
      }
      case 1: {
        po->set_origem(value.toString().toUtf8().constData());
        return true;
      }
      case 2: {
        bool ok = false;
        int valor = value.toInt(&ok);
        if (!ok) return false;
        po->set_valor(valor);
        return true;
      }
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
  }

  const ent::Bonus Bonus() const { return bonus_; }

 private:
  std::tuple<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*> DadosEm(int row) {
    while (row >= 0) {
      for (auto& bi : *bonus_.mutable_bonus_individual()) {
        if (row < bi.por_origem_size()) {
          // achou o bi.
          return std::make_tuple(&bi, bi.mutable_por_origem(row));
        } else {
          row -= bi.por_origem_size();
        }
      }
    }
    return std::make_tuple(nullptr, nullptr);
  }

  std::tuple<ent::BonusIndividual*, ent::BonusIndividual_PorOrigem*> DadosEm(const QModelIndex& index) {
    return DadosEm(index.row());
  }

  std::tuple<const ent::BonusIndividual*, const ent::BonusIndividual_PorOrigem*> DadosEm(
      const QModelIndex& index) const {
    return DadosEm(index.row());
  }

  std::tuple<const ent::BonusIndividual*, const ent::BonusIndividual_PorOrigem*> DadosEm(int row) const {
    while (row >= 0) {
      for (auto& bi : bonus_.bonus_individual()) {
        if (row < bi.por_origem_size()) {
          // achou o bi.
          return std::make_tuple(&bi, &bi.por_origem(row));
        } else {
          row -= bi.por_origem_size();
        }
      }
    }
    return std::make_tuple(nullptr, nullptr);
  }

 private:
  QTableView* tabela_;
  ent::Bonus bonus_;
};

// Responsavel por tratar a edicao do tipo de bonus.
class TipoBonusDelegate : public QItemDelegate {
 public:
  TipoBonusDelegate(QTableView* tabela, ModeloBonus* modelo, QObject* parent)
      : QItemDelegate(), tabela_(tabela), modelo_(modelo) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    QComboBox* combo = new QComboBox(parent);
    PreencheComboBonus(ent::TB_BASE, combo);
    LOG(INFO) << "Criando combo";
    return combo;
  }

  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo, index] () {
      setModelData(combo, modelo_, index);
      emit closeEditor(combo);
    });
    QVariant data = modelo_->data(index);
    ent::TipoBonus tipo;
    if (!ent::TipoBonus_Parse(data.toString().toUtf8().constData(), &tipo)) {
      return;
    }
    combo->setCurrentIndex(tipo);
  }

  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(INFO) << "combo == nullptr em setEditorData";
      return;
    }
    modelo_->setData(index, combo->currentIndex(), Qt::EditRole);
    tabela_->reset();
  }

 private:
  QTableView* tabela_;
  ModeloBonus* modelo_;
  ent::TipoBonus tipo_;
};

void AbreDialogoBonus(QWidget* pai, ent::Bonus* bonus) {
  ifg::qt::Ui::DialogoBonus gerador;
  std::unique_ptr<QDialog> dialogo(new QDialog(pai));
  gerador.setupUi(dialogo.get());
  std::unique_ptr<QItemSelectionModel> delete_model(gerador.tabela_bonus->selectionModel());
  std::unique_ptr<ModeloBonus> modelo(new ModeloBonus(*bonus, gerador.tabela_bonus));
  gerador.tabela_bonus->setModel(modelo.get());
  lambda_connect(gerador.botao_adicionar_bonus, SIGNAL(clicked()), [&modelo] () {
    modelo->insertRows(0, 1, QModelIndex());
  });
  lambda_connect(gerador.botao_remover_bonus, SIGNAL(clicked()), [&modelo, &gerador] () {
#if 0
    auto selecionados = gerador.tabela_bonus->selectedIndexes();
    std::set<int, std::greater> linhas;
    for (const QModelIndex index : selecionados) {
      linhas.insert(index.row());
    }
    for (int linha : linhas) {
      modelo->RemoveRows(index.row(), 1, QModelIndex());
    }
#endif
  });
  std::unique_ptr<QAbstractItemDelegate> delegado(
      new TipoBonusDelegate(gerador.tabela_bonus, modelo.get(), gerador.tabela_bonus));
  std::unique_ptr<QAbstractItemDelegate> delete_previous(gerador.tabela_bonus->itemDelegateForColumn(0));
  gerador.tabela_bonus->setItemDelegateForColumn(0, delegado.get());

  auto res = dialogo->exec();
  if (res == QDialog::Rejected) {
    return;
  }
  *bonus = modelo->Bonus();
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
  gerador.lista_rotulos->appendPlainText(QString::fromUtf8(rotulos_especiais.c_str()));
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
      [this, &gerador, &notificacao] () {
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
  lambda_connect(gerador.botao_cor, SIGNAL(clicked()), [this, dialogo, &gerador, &ent_cor] {
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

  // Translacao em Z.
  gerador.spin_translacao_quad->setValue(entidade.pos().z() * METROS_PARA_QUADRADOS);

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
  gerador.spin_escala_x_quad->setValue(METROS_PARA_QUADRADOS * entidade.escala().x());
  gerador.spin_escala_y_quad->setValue(METROS_PARA_QUADRADOS * entidade.escala().y());
  gerador.spin_escala_z_quad->setValue(METROS_PARA_QUADRADOS * entidade.escala().z());

  if (entidade.has_tesouro()) {
    gerador.lista_tesouro->appendPlainText(QString::fromUtf8(entidade.tesouro().tesouro().c_str()));
  }

  // Transicao de cenario.
  auto habilita_posicao = [gerador] {
    gerador.checkbox_transicao_posicao->setCheckState(Qt::Checked);
  };
  lambda_connect(gerador.spin_trans_x, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.spin_trans_y, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.spin_trans_z, SIGNAL(valueChanged(double)), habilita_posicao);
  lambda_connect(gerador.combo_transicao, SIGNAL(currentIndexChanged(int)), [gerador] {
    bool trans_cenario = gerador.combo_transicao->currentIndex() == ent::EntidadeProto::TRANS_CENARIO;
    gerador.linha_transicao_cenario->setEnabled(trans_cenario);
    gerador.checkbox_transicao_posicao->setEnabled(trans_cenario);
    gerador.spin_trans_x->setEnabled(trans_cenario);
    gerador.spin_trans_y->setEnabled(trans_cenario);
    gerador.spin_trans_z->setEnabled(trans_cenario);
  });
  if (!entidade.transicao_cenario().has_id_cenario()) {
    bool trans_tesouro = entidade.tipo_transicao() == ent::EntidadeProto::TRANS_TESOURO;
    gerador.combo_transicao->setCurrentIndex(trans_tesouro ? ent::EntidadeProto::TRANS_TESOURO : ent::EntidadeProto::TRANS_NENHUMA);
    gerador.linha_transicao_cenario->setEnabled(false);
    gerador.checkbox_transicao_posicao->setEnabled(false);
    gerador.spin_trans_x->setEnabled(false);
    gerador.spin_trans_y->setEnabled(false);
    gerador.spin_trans_z->setEnabled(false);
  } else {
    gerador.combo_transicao->setCurrentIndex(ent::EntidadeProto::TRANS_CENARIO);
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
  // Esconde dialogo e entra no modo de selecao.
  lambda_connect(gerador.botao_transicao_mapa, SIGNAL(clicked()), [this, dialogo, gerador, &entidade, &proto_retornado] {
    auto* notificacao = ntf::NovaNotificacao(ntf::TN_ENTRAR_MODO_SELECAO_TRANSICAO);
    notificacao->mutable_entidade()->set_id(entidade.id());
    notificacao->mutable_entidade()->set_tipo_transicao(ent::EntidadeProto::TRANS_CENARIO);
    if (entidade.has_transicao_cenario()) {
      *notificacao->mutable_entidade()->mutable_transicao_cenario() = entidade.transicao_cenario();
    }
    central_->AdicionaNotificacao(notificacao);
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
      proto_retornado->add_rotulo_especial(rotulo.toUtf8().constData());
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
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao_quad->value() * QUADRADOS_PARA_METROS);
    proto_retornado->clear_translacao_z_deprecated();
    proto_retornado->mutable_escala()->set_x(QUADRADOS_PARA_METROS * gerador.spin_escala_x_quad->value());
    proto_retornado->mutable_escala()->set_y(QUADRADOS_PARA_METROS * gerador.spin_escala_y_quad->value());
    proto_retornado->mutable_escala()->set_z(QUADRADOS_PARA_METROS * gerador.spin_escala_z_quad->value());
    proto_retornado->mutable_tesouro()->set_tesouro(gerador.lista_tesouro->toPlainText().toUtf8().constData());
    if (gerador.combo_transicao->currentIndex() == ent::EntidadeProto::TRANS_CENARIO) {
      bool ok = false;
      int val = gerador.linha_transicao_cenario->text().toInt(&ok);
      if (!ok || (val < CENARIO_PRINCIPAL)) {
        LOG(WARNING) << "Ignorando valor de transicao: " << gerador.linha_transicao_cenario->text().toUtf8().constData();
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

// Monta a string de dano de um ataque, como 1d6 (x3).
std::string StringDano(const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  std::string critico;
  if (da.multiplicador_critico() > 2 || da.margem_critico() < 20) {
    critico += "(";
    if (da.margem_critico() < 20) {
      critico += net::to_string(da.margem_critico()) + "-20";
      if (da.multiplicador_critico() > 2) {
        critico += "/";
      }
    }
    if (da.multiplicador_critico() > 2) {
      critico += "x" + net::to_string(da.multiplicador_critico());
    }
    critico += "), " + google::protobuf::StringPrintf("CA: %d/%d/%d", da.ca_normal(), da.ca_surpreso(), da.ca_toque());
  }
  return da.dano() + critico;
}

// Retorna o resumo da arma, algo como id: rotulo, alcance: 10 q, 5 incrementos, bonus +3, dano: 1d8(x3), CA: 15, toque: 12, surpresa: 13.
std::string ResumoArma(const ent::EntidadeProto::DadosAtaque& da) {
  // Monta a string.
  char string_rotulo[40] = { '\0' };
  if (!da.rotulo().empty()) {
    snprintf(string_rotulo, 39, "%s, ", da.rotulo().c_str());
  }
  char string_alcance[40] = { '\0' };
  if (da.has_alcance_m()) {
    char string_incrementos[40] = { '\0' };
    if (da.has_incrementos()) {
      snprintf(string_incrementos, 39, ", inc %d", da.incrementos());
    }
    snprintf(string_alcance, 39, "alcance: %0.0f q%s, ", da.alcance_m() * METROS_PARA_QUADRADOS, string_incrementos);
  }
  std::string string_escudo = da.permite_escudo() ? "(escudo)" : "";
  return google::protobuf::StringPrintf(
      "id: %s%s, %sbonus: %d, dano: %s, ca%s: %d toque: %d surpresa%s: %d",
      string_rotulo, da.tipo_ataque().c_str(), string_alcance, da.bonus_ataque(), StringDano(da).c_str(), string_escudo.c_str(), da.ca_normal(),
      da.ca_toque(), string_escudo.c_str(), da.ca_surpreso());
}

//--------------------------------------------------------------------------------------------------
// As funcoes AtualizaUI* atualizam uma parte especifica da UI. Elas nao chamam dependencias
// pois sabem apenas o que fazer em uma parte. Normalmente, apos se atualizar algum campo, chama-se
// ent::RecomputaDependencias e as AtualizaUI apropriadas.
//--------------------------------------------------------------------------------------------------
// Atualiza a UI de atributos.
void AtualizaUIAtributos(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& a = proto.atributos();
  std::vector<std::tuple<const ent::Bonus*, QSpinBox*, QPushButton*, QLabel*>> tuplas = {
    std::make_tuple(&a.forca(),        gerador.spin_forca,        gerador.botao_bonus_forca,        gerador.label_mod_forca),
    std::make_tuple(&a.destreza(),     gerador.spin_destreza,     gerador.botao_bonus_destreza,     gerador.label_mod_destreza),
    std::make_tuple(&a.constituicao(), gerador.spin_constituicao, gerador.botao_bonus_constituicao, gerador.label_mod_constituicao),
    std::make_tuple(&a.inteligencia(), gerador.spin_inteligencia, gerador.botao_bonus_inteligencia, gerador.label_mod_inteligencia),
    std::make_tuple(&a.sabedoria(),    gerador.spin_sabedoria,    gerador.botao_bonus_sabedoria,    gerador.label_mod_sabedoria),
    std::make_tuple(&a.carisma(),      gerador.spin_carisma,      gerador.botao_bonus_carisma,      gerador.label_mod_carisma),
  };
  for (const auto& t : tuplas) {
    const ent::Bonus* bonus;
    QSpinBox* spin;
    QPushButton* botao;
    QLabel* label;
    std::tie(bonus, spin, botao, label) = t;
    spin->blockSignals(true);
    spin->setValue(ent::BonusIndividualTotal(ent::TB_BASE, *bonus));
    spin->blockSignals(false);
    const int bonus_total = ent::BonusTotal(*bonus);
    botao->setText(QString::number(bonus_total));
    label->setText(NumeroSinalizado(ent::ModificadorAtributo(bonus_total)));
  }
}

// Refresca a lista de ataques toda.
void AtualizaUIAtaque(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto_retornado) {
  int linha = gerador.lista_ataques->currentRow();
  gerador.lista_ataques->clear();
  for (const auto& da : proto_retornado.dados_ataque()) {
    gerador.lista_ataques->addItem(QString::fromUtf8(ResumoArma(da).c_str()));
  }
  gerador.lista_ataques->setCurrentRow(linha);
}

// Atualiza a UI de ataque e defesa baseada no proto.
void AtualizaUIAtaquesDefesa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  AtualizaUIAtaque(gerador, proto);

  const int modificador_destreza = ent::ModificadorAtributo(ent::BonusTotal(proto.atributos().destreza()));
  const auto& ca = proto.dados_defesa().ca();
  gerador.botao_bonus_ca->setText(QString::number(BonusTotal(ca)));

  std::vector<QWidget*> objs = { gerador.spin_ca_armadura, gerador.spin_ca_escudo };
  for (auto* obj : objs) obj->blockSignals(true);
  gerador.spin_ca_armadura->setValue(ent::BonusIndividualTotal(ent::TB_ARMADURA, ca));
  gerador.spin_ca_escudo->setValue(ent::BonusIndividualTotal(ent::TB_ESCUDO, ca));
  for (auto* obj : objs) obj->blockSignals(false);
  const int bonus_ca_total = ent::BonusTotal(ca);
  gerador.botao_bonus_ca->setText(QString::number(bonus_ca_total));
  gerador.label_ca_toque->setText(QString::number(
      ent::BonusTotalExcluindo(
        ca,
        { ent::TB_ARMADURA, ent::TB_ESCUDO, ent::TB_ARMADURA_NATURAL, ent::TB_ARMADURA_MELHORIA, ent::TB_ESCUDO_MELHORIA })));
  gerador.label_ca_surpreso->setText(QString::number(bonus_ca_total - std::max(modificador_destreza, 0)));
}

// Atualiza UI de iniciativa.
void AtualizaUIIniciativa(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  gerador.checkbox_iniciativa->setCheckState(proto.has_iniciativa() ? Qt::Checked : Qt::Unchecked);
  gerador.spin_iniciativa->setValue(proto.iniciativa());
  gerador.botao_bonus_iniciativa->setText(NumeroSinalizado(ent::BonusTotal(proto.bonus_iniciativa())));
}

void AtualizaUISalvacoes(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto) {
  const auto& dd = proto.dados_defesa();
  std::vector<std::tuple<QSpinBox*, QPushButton*, const ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.spin_salvacao_fortitude, gerador.botao_bonus_salvacao_fortitude, &dd.salvacao_fortitude()),
    std::make_tuple(gerador.spin_salvacao_reflexo, gerador.botao_bonus_salvacao_reflexo, &dd.salvacao_reflexo()),
    std::make_tuple(gerador.spin_salvacao_vontade, gerador.botao_bonus_salvacao_vontade, &dd.salvacao_vontade()),
  };
  for (const auto& t : tuplas) {
    QSpinBox* spin; QPushButton* botao; const ent::Bonus* bonus;
    std::tie(spin, botao, bonus) = t;
    spin->setValue(ent::BonusIndividualTotal(ent::TB_BASE, *bonus));
    botao->setText(NumeroSinalizado(ent::BonusTotal(*bonus)));
  }
}
// Fim AtualizaUI*.
//---------------------------------------------------------------------

// Usada fora do PreencheConfiguraDadosAtaque.
void AdicionaOuAtualizaAtaqueEntidade(ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  ent::EntidadeProto::DadosAtaque da;
  int indice = gerador.lista_ataques->currentRow();
  bool indice_valido = (indice >= 0 && indice < proto_retornado->dados_ataque().size());
  if (gerador.combo_tipo_ataque->currentIndex() > ULTIMO_TIPO_VALIDO && indice_valido) {
    da.set_tipo_ataque(proto_retornado->dados_ataque(indice).tipo_ataque());
  } else {
    da.set_tipo_ataque(IndiceParaTipo(gerador.combo_tipo_ataque->currentIndex()));
  }
  da.set_bonus_ataque(gerador.spin_ataque->value());
  da.set_permite_escudo(gerador.checkbox_permite_escudo->checkState() == Qt::Checked);
  ent::DanoArma dano_arma = ent::LeDanoArma(gerador.linha_dano->text().toUtf8().constData());
  da.set_dano(dano_arma.dano);
  da.set_multiplicador_critico(dano_arma.multiplicador);
  da.set_margem_critico(dano_arma.margem_critico);
  da.set_rotulo(gerador.linha_rotulo_ataque->text().toUtf8().constData());
  da.set_incrementos(gerador.spin_incrementos->value());
  if (gerador.spin_alcance_quad->value() > 0) {
    da.set_alcance_m(gerador.spin_alcance_quad->value() * QUADRADOS_PARA_METROS);
  } else {
    da.clear_alcance_m();
  }
  if (indice_valido) {
    proto_retornado->mutable_dados_ataque(indice)->MergeFrom(da);
  } else {
    proto_retornado->add_dados_ataque()->Swap(&da);
  }
  AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
}

void PreencheConfiguraAtributos(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  AtualizaUIAtributos(gerador, proto);
  // Atualiza os campos.
  auto* atrib = proto_retornado->mutable_atributos();
  lambda_connect(gerador.spin_forca, SIGNAL(valueChanged(int)), [&gerador, atrib, proto_retornado] () {
    ent::AtribuiBonus(gerador.spin_forca->value(), ent::TB_BASE, "base", atrib->mutable_forca());
    AtualizaUIAtributos(gerador, *proto_retornado);
  });
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
      ent::RecomputaDependencias(proto_retornado);
      AtualizaUIAtributos(gerador, *proto_retornado);
      AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
      AtualizaUIIniciativa(gerador, *proto_retornado);
      AtualizaUISalvacoes(gerador, *proto_retornado);
    });
    lambda_connect(spin, SIGNAL(valueChanged(int)), [&gerador, spin, bonus, proto_retornado] () {
      ent::AtribuiBonus(spin->value(), ent::TB_BASE, "base", bonus);
      ent::RecomputaDependencias(proto_retornado);
      AtualizaUIAtributos(gerador, *proto_retornado);
      AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
      AtualizaUIIniciativa(gerador, *proto_retornado);
      AtualizaUISalvacoes(gerador, *proto_retornado);
    });
  }
}

void PreencheConfiguraDadosDefesa(
    Visualizador3d* this_, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  AtualizaUIAtaquesDefesa(gerador, proto);
  // Imune critico.
  gerador.checkbox_imune_critico->setCheckState(proto.dados_defesa().imune_critico() ? Qt::Checked : Qt::Unchecked);

  auto* mca = proto_retornado->mutable_dados_defesa()->mutable_ca();
  ent::AtribuiBonus(10, ent::TB_BASE, "base",  mca);
  lambda_connect(gerador.spin_ca_armadura, SIGNAL(valueChanged(int)), [&gerador, proto_retornado, mca] () {
    ent::AtribuiBonus(gerador.spin_ca_armadura->value(), ent::TB_ARMADURA, "armadura", mca);
    ent::RecomputaDependencias(proto_retornado);
    AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
  });
  lambda_connect(gerador.spin_ca_escudo, SIGNAL(valueChanged(int)), [&gerador, proto_retornado, mca] () {
    ent::AtribuiBonus(gerador.spin_ca_escudo->value(), ent::TB_ESCUDO, "escudo", mca);
    ent::RecomputaDependencias(proto_retornado);
    AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
  });
  lambda_connect(gerador.botao_bonus_ca, SIGNAL(clicked()), [this_, &gerador, proto_retornado, mca] () {
    AbreDialogoBonus(this_, mca);
    ent::RecomputaDependencias(proto_retornado);
    AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
  });
}

void PreencheConfiguraDadosAtaque(ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& ent, ent::EntidadeProto* proto_retornado) {
  auto AdicionaOuAtualizaAtaque = [&gerador, proto_retornado] () {
    AdicionaOuAtualizaAtaqueEntidade(gerador, proto_retornado);
  };

  gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));
  AtualizaUIAtaque(gerador, *proto_retornado);

  auto EditaRefrescaLista = [&gerador, proto_retornado, AdicionaOuAtualizaAtaque] () {
    int indice_antes = gerador.lista_ataques->currentRow();
    if (indice_antes < 0 || indice_antes >= proto_retornado->dados_ataque().size()) {
      // Vale apenas para edicao.
      return;
    }
    AdicionaOuAtualizaAtaque();
    AtualizaUIAtaque(gerador, *proto_retornado);
    if (indice_antes < proto_retornado->dados_ataque().size()) {
      gerador.lista_ataques->setCurrentRow(indice_antes);
    } else {
      gerador.lista_ataques->setCurrentRow(-1);
    }
  };

  lambda_connect(gerador.lista_ataques, SIGNAL(currentRowChanged(int)), [&gerador, proto_retornado] () {
    std::vector<QObject*> objs =
        {gerador.spin_ataque, gerador.spin_alcance_quad, gerador.spin_incrementos,
         gerador.combo_tipo_ataque, gerador.linha_dano };
    for (auto* obj : objs) obj->blockSignals(true);
    if (gerador.lista_ataques->currentRow() == -1 || gerador.lista_ataques->currentRow() >= proto_retornado->dados_ataque().size()) {
      gerador.botao_remover_ataque->setEnabled(false);
      gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));
      gerador.botao_ataque_cima->setEnabled(false);
      gerador.botao_ataque_baixo->setEnabled(false);
    } else {
      gerador.botao_remover_ataque->setEnabled(true);
      const auto& da = proto_retornado->dados_ataque(gerador.lista_ataques->currentRow());
      gerador.linha_rotulo_ataque->setText(QString::fromUtf8(da.rotulo().c_str()));
      gerador.combo_tipo_ataque->setCurrentIndex(TipoParaIndice(da.tipo_ataque()));
      gerador.spin_ataque->setValue(da.bonus_ataque());
      gerador.checkbox_permite_escudo->setCheckState(da.permite_escudo() ? Qt::Checked : Qt::Unchecked);
      gerador.linha_dano->setText(StringDano(da).c_str());
      gerador.spin_incrementos->setValue(da.incrementos());
      gerador.spin_alcance_quad->setValue(METROS_PARA_QUADRADOS * (da.has_alcance_m() ? da.alcance_m() : -1.5f));
      gerador.botao_clonar_ataque->setText(QObject::tr("Clonar"));
      if (proto_retornado->dados_ataque().size() > 1) {
        gerador.botao_ataque_cima->setEnabled(true);
        gerador.botao_ataque_baixo->setEnabled(true);
      }
      for (auto* obj : objs) obj->blockSignals(false);
    }
  });
  lambda_connect(gerador.botao_clonar_ataque, SIGNAL(clicked()), [AdicionaOuAtualizaAtaque, &gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size()) {
      AdicionaOuAtualizaAtaque();
    } else {
      *proto_retornado->mutable_dados_ataque()->Add() = proto_retornado->dados_ataque(indice);
    }
    AtualizaUIAtaque(gerador, *proto_retornado);
    gerador.lista_ataques->setCurrentRow(proto_retornado->dados_ataque().size() - 1);
  });

  lambda_connect(gerador.botao_ataque_cima, SIGNAL(clicked()), [&gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice <= 0 || indice >= proto_retornado->dados_ataque().size() ||
        proto_retornado->dados_ataque().size() <= 1 || indice >= proto_retornado->dados_ataque().size()) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice - 1));
    AtualizaUIAtaque(gerador, *proto_retornado);
    gerador.lista_ataques->setCurrentRow(indice - 1);
  });
  lambda_connect(gerador.botao_ataque_baixo, SIGNAL(clicked()), [&gerador, proto_retornado] () {
    int indice = gerador.lista_ataques->currentRow();
    if (indice < 0 || indice >= proto_retornado->dados_ataque().size() - 1 ||
        proto_retornado->dados_ataque().size() <= 1) {
      return;
    }
    proto_retornado->mutable_dados_ataque(indice)->Swap(proto_retornado->mutable_dados_ataque(indice + 1));
    AtualizaUIAtaque(gerador, *proto_retornado);
    gerador.lista_ataques->setCurrentRow(indice + 1);
  });

  lambda_connect(gerador.botao_remover_ataque, SIGNAL(clicked()), [&gerador, proto_retornado] () {
    if (gerador.lista_ataques->currentRow() == -1 || gerador.lista_ataques->currentRow() >= proto_retornado->dados_ataque().size()) {
      return;
    }
    proto_retornado->mutable_dados_ataque()->DeleteSubrange(gerador.lista_ataques->currentRow(), 1);
    gerador.spin_ataque->clear();
    gerador.checkbox_permite_escudo->setCheckState(Qt::Unchecked);
    gerador.spin_incrementos->clear();
    gerador.spin_alcance_quad->clear();
    gerador.linha_dano->clear();
    gerador.linha_rotulo_ataque->clear();
    gerador.botao_clonar_ataque->setText(QObject::tr("Adicionar"));
    gerador.botao_ataque_cima->setEnabled(false);
    gerador.botao_ataque_baixo->setEnabled(false);
    AtualizaUIAtaque(gerador, *proto_retornado);
  });
  // Ao adicionar aqui, adicione nos sinais bloqueados tb (blockSignals). Exceto para textEdited, que nao dispara sinal programaticamente.
  lambda_connect(gerador.linha_rotulo_ataque, SIGNAL(textEdited(const QString&)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  lambda_connect(gerador.linha_dano, SIGNAL(editingFinished()), [EditaRefrescaLista]() { EditaRefrescaLista(); } );  // nao pode refrescar no meio pois tem processamento da string.
  lambda_connect(gerador.combo_tipo_ataque, SIGNAL(currentIndexChanged(int)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  lambda_connect(gerador.spin_ataque, SIGNAL(valueChanged(int)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  lambda_connect(gerador.checkbox_permite_escudo, SIGNAL(stateChanged(int)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  lambda_connect(gerador.spin_incrementos, SIGNAL(valueChanged(int)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  lambda_connect(gerador.spin_alcance_quad, SIGNAL(valueChanged(int)), [EditaRefrescaLista]() { EditaRefrescaLista(); } );
  // Furtivo
  gerador.linha_furtivo->setText(QString::fromUtf8(ent.dados_ataque_globais().dano_furtivo().c_str()));
}

// Chamado tb durante a finalizacao, por causa do problema de apertar enter e fechar a janela.
void AdicionaOuEditaNivel(ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  int indice = gerador.lista_niveis->currentRow();
  ent::InfoClasse* info_classe = (indice < 0 || indice >= proto_retornado->info_classes().size())
      ? proto_retornado->add_info_classes() : proto_retornado->mutable_info_classes(indice);
  info_classe->set_id(gerador.linha_classe->text().toUtf8().constData());
  info_classe->set_nivel(gerador.spin_nivel_classe->value());
  info_classe->set_nivel_conjurador(gerador.spin_nivel_conjurador->value());
  info_classe->set_bba(gerador.spin_bba->value());
  info_classe->set_modificador_atributo_conjuracao(gerador.spin_mod_conjuracao->value());
}

void PreencheConfiguraNiveis(ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto_retornado) {
  auto AtualizaNivelTotal = [&gerador, proto_retornado] () {
    int total = 0;
    int total_bba = 0;
    for (const auto& info_classe : proto_retornado->info_classes()) {
      total += info_classe.nivel();
      total_bba += info_classe.bba();
    }
    gerador.linha_nivel->setText(QString::number(total));
    gerador.linha_bba->setText(QString::number(total_bba));
  };
  auto RefrescaNiveis = [&gerador, proto_retornado, AtualizaNivelTotal] () {
    gerador.lista_niveis->clear();
    for (const auto& info_classe : proto_retornado->info_classes()) {
      std::string string_nivel;
      google::protobuf::StringAppendF(&string_nivel, "classe: %s, nível: %d", info_classe.id().c_str(), info_classe.nivel());
      if (info_classe.nivel_conjurador() > 0) {
        google::protobuf::StringAppendF(
            &string_nivel, ", conjurador: %d, mod: %d",
            info_classe.nivel_conjurador(), info_classe.modificador_atributo_conjuracao());
      }
      google::protobuf::StringAppendF(&string_nivel, ", BBA: %d", info_classe.bba());
      gerador.lista_niveis->addItem(QString::fromUtf8(string_nivel.c_str()));
    }
    AtualizaNivelTotal();
  };
  RefrescaNiveis();

  auto EditaRefrescaNiveis = [&gerador, proto_retornado, RefrescaNiveis] () {
    int indice_antes = gerador.lista_niveis->currentRow();
    if (indice_antes < 0 || indice_antes >= proto_retornado->info_classes().size()) {
      return;
    }
    AdicionaOuEditaNivel(gerador, proto_retornado);
    RefrescaNiveis();
    if (indice_antes < proto_retornado->info_classes().size()) {
      gerador.lista_niveis->setCurrentRow(indice_antes);
    } else {
      gerador.lista_niveis->setCurrentRow(-1);
    }
  };

  auto LimpaCampos = [&gerador] () {
    gerador.linha_classe->clear();
    gerador.spin_nivel_classe->clear();
    gerador.spin_nivel_conjurador->clear();
    gerador.spin_bba->clear();
    gerador.spin_mod_conjuracao->clear();
  };

  lambda_connect(gerador.lista_niveis, SIGNAL(currentRowChanged(int)), [&gerador, proto_retornado] () {
    std::vector<QObject*> objs = {
        gerador.spin_nivel_classe, gerador.spin_nivel_conjurador, gerador.linha_classe, gerador.spin_bba,
        gerador.spin_mod_conjuracao
    };
    for (auto* obj : objs) obj->blockSignals(true);
    if (gerador.lista_niveis->currentRow() == -1 ||
        gerador.lista_niveis->currentRow() >= proto_retornado->info_classes().size()) {
      gerador.botao_remover_nivel->setEnabled(false);
    } else {
      gerador.botao_remover_nivel->setEnabled(true);
      const auto& info_classe = proto_retornado->info_classes(gerador.lista_niveis->currentRow());
      gerador.linha_classe->setText(QString::fromUtf8(info_classe.id().c_str()));
      gerador.spin_nivel_classe->setValue(info_classe.nivel());
      gerador.spin_nivel_conjurador->setValue(info_classe.nivel_conjurador());
      gerador.spin_bba->setValue(info_classe.bba());
      gerador.spin_mod_conjuracao->setValue(info_classe.modificador_atributo_conjuracao());
      for (auto* obj : objs) obj->blockSignals(false);
    }
  });
  lambda_connect(gerador.botao_adicionar_nivel, SIGNAL(clicked()), [LimpaCampos, RefrescaNiveis, &gerador, proto_retornado] () {
    auto* info_classe = proto_retornado->mutable_info_classes()->Add();
    if (gerador.lista_niveis->currentRow() < 0 ||
        gerador.lista_niveis->currentRow() >= proto_retornado->info_classes().size()) {
      // So usa os campos se for um novo nivel.
      info_classe->set_id(gerador.linha_classe->text().toUtf8().constData());
      info_classe->set_nivel(gerador.spin_nivel_classe->value());
      info_classe->set_nivel_conjurador(gerador.spin_nivel_conjurador->value());
      info_classe->set_bba(gerador.spin_bba->value());
      info_classe->set_modificador_atributo_conjuracao(gerador.spin_mod_conjuracao->value());
    }
    RefrescaNiveis();
    // Deixa deselecionado.
    //gerador.lista_niveis->setCurrentRow(proto_retornado->info_classes().size() - 1);
    gerador.lista_niveis->setCurrentRow(-1);
    LimpaCampos();
  });

  lambda_connect(gerador.botao_remover_nivel, SIGNAL(clicked()), [LimpaCampos, RefrescaNiveis, &gerador, proto_retornado] () {
    if (gerador.lista_niveis->currentRow() == -1 ||
        gerador.lista_niveis->currentRow() >= proto_retornado->info_classes().size()) {
      return;
    }
    proto_retornado->mutable_info_classes()->DeleteSubrange(gerador.lista_niveis->currentRow(), 1);
    LimpaCampos();
    RefrescaNiveis();
  });
  // Ao adicionar aqui, adicione nos sinais bloqueados tb (blockSignals).
  lambda_connect(gerador.linha_classe, SIGNAL(textEdited(const QString&)), [EditaRefrescaNiveis]() { EditaRefrescaNiveis(); } );
  lambda_connect(gerador.spin_nivel_classe, SIGNAL(valueChanged(int)), [EditaRefrescaNiveis]() { EditaRefrescaNiveis(); } );
  lambda_connect(gerador.spin_nivel_conjurador, SIGNAL(valueChanged(int)), [EditaRefrescaNiveis]() { EditaRefrescaNiveis(); } );
  lambda_connect(gerador.spin_bba, SIGNAL(valueChanged(int)), [EditaRefrescaNiveis]() { EditaRefrescaNiveis(); } );
  lambda_connect(gerador.spin_mod_conjuracao, SIGNAL(valueChanged(int)), [EditaRefrescaNiveis]() { EditaRefrescaNiveis(); } );
}

void PreencheConfiguraSalvacoes(ifg::qt::Visualizador3d* pai, ifg::qt::Ui::DialogoEntidade& gerador, ent::EntidadeProto* proto) {
  auto* dd = proto->mutable_dados_defesa();
  AtualizaUISalvacoes(gerador, *proto);
  std::vector<std::tuple<QSpinBox*, QPushButton*, ent::Bonus*>> tuplas = {
    std::make_tuple(gerador.spin_salvacao_fortitude, gerador.botao_bonus_salvacao_fortitude, dd->mutable_salvacao_fortitude()),
    std::make_tuple(gerador.spin_salvacao_reflexo, gerador.botao_bonus_salvacao_reflexo, dd->mutable_salvacao_reflexo()),
    std::make_tuple(gerador.spin_salvacao_vontade, gerador.botao_bonus_salvacao_vontade, dd->mutable_salvacao_vontade()),
  };
  for (const auto& t : tuplas) {
    QSpinBox* spin; QPushButton* botao; ent::Bonus* bonus;
    std::tie(spin, botao, bonus) = t;
    lambda_connect(spin, SIGNAL(valueChanged(int)), [spin, bonus, &gerador, proto] {
      ent::AtribuiBonus(spin->value(), ent::TB_BASE, "base", bonus);
      AtualizaUISalvacoes(gerador, *proto);
    });
    lambda_connect(botao, SIGNAL(clicked()), [pai, bonus, &gerador, proto] () {
      AbreDialogoBonus(pai, bonus);
      AtualizaUISalvacoes(gerador, *proto);
    });
  }
}

void PreencheConfiguraDadosIniciativa(
    ifg::qt::Visualizador3d* pai, ifg::qt::Ui::DialogoEntidade& gerador, const ent::EntidadeProto& proto, ent::EntidadeProto* proto_retornado) {
  AtualizaUIIniciativa(gerador, proto);
  lambda_connect(gerador.checkbox_iniciativa, SIGNAL(stateChanged(int)), [&gerador] () {
    gerador.spin_iniciativa->setEnabled(gerador.checkbox_iniciativa->checkState() == Qt::Checked);
  });
  lambda_connect(gerador.spin_iniciativa, SIGNAL(valueChanged(int)), [&gerador] () {
    gerador.checkbox_iniciativa->setCheckState(Qt::Checked);
  });
  lambda_connect(gerador.botao_bonus_iniciativa, SIGNAL(clicked()), [pai, &gerador, proto_retornado] () {
    auto* bonus_iniciativa = proto_retornado->mutable_bonus_iniciativa();
    AbreDialogoBonus(pai, bonus_iniciativa);
    gerador.botao_bonus_iniciativa->setText(NumeroSinalizado(ent::BonusTotal(*bonus_iniciativa)));
  });
}

}  // namespace

ent::EntidadeProto* Visualizador3d::AbreDialogoTipoEntidade(
    const ntf::Notificacao& notificacao) {
  const auto& entidade = notificacao.entidade();
  auto* proto_retornado = new ent::EntidadeProto(entidade);
  proto_retornado->set_id(entidade.id());
  ifg::qt::Ui::DialogoEntidade gerador;
  auto* dialogo = new QDialog(this);
  gerador.setupUi(dialogo);
  // ID.
  QString id_str;
  gerador.campo_id->setText(id_str.setNum(entidade.id()));
  // Rotulo.
  QString rotulo_str;
  gerador.campo_rotulo->setText(QString::fromUtf8(entidade.rotulo().c_str()));
  // Rotulos especiais.
  std::string rotulos_especiais;
  for (const std::string& rotulo_especial : entidade.rotulo_especial()) {
    rotulos_especiais += rotulo_especial + "\n";
  }
  gerador.lista_rotulos->appendPlainText(QString::fromUtf8(rotulos_especiais.c_str()));
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
  gerador.lista_eventos->appendPlainText(QString::fromUtf8(eventos.c_str()));

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
  lambda_connect(gerador.slider_tamanho, SIGNAL(valueChanged(int)), [&gerador, proto_retornado] () {
    proto_retornado->set_tamanho(ent::TamanhoEntidade(gerador.slider_tamanho->sliderPosition()));
    gerador.label_tamanho->setText(TamanhoParaTexto(gerador.slider_tamanho->sliderPosition()));
    ent::RecomputaDependencias(proto_retornado);
    AtualizaUIAtaquesDefesa(gerador, *proto_retornado);
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
    gerador.spin_raio_quad->setValue(METROS_PARA_QUADRADOS * (entidade.luz().has_raio_m() ? entidade.luz().raio_m() : 6.0f));
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
      gerador.spin_raio_quad->setValue(METROS_PARA_QUADRADOS * 6.0f);
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
  gerador.spin_pontos_vida->setValue(entidade.pontos_vida());
  gerador.spin_pontos_vida_temporarios->setValue(entidade.pontos_vida_temporarios());
  gerador.spin_max_pontos_vida->setValue(entidade.max_pontos_vida());
  // Aura.
  gerador.spin_aura_quad->setValue(entidade.aura_m() * METROS_PARA_QUADRADOS);
  // Voo.
  gerador.checkbox_voadora->setCheckState(entidade.voadora() ? Qt::Checked : Qt::Unchecked);
  // Caida.
  gerador.checkbox_caida->setCheckState(entidade.caida() ? Qt::Checked : Qt::Unchecked);
  // Morta.
  gerador.checkbox_morta->setCheckState(entidade.morta() ? Qt::Checked : Qt::Unchecked);
  // Translacao em Z.
  gerador.spin_translacao_quad->setValue(entidade.pos().z() * METROS_PARA_QUADRADOS);

  if (entidade.has_tesouro()) {
    gerador.lista_tesouro->appendPlainText(QString::fromUtf8(entidade.tesouro().tesouro().c_str()));
  }
  gerador.texto_notas->appendPlainText(QString::fromUtf8(entidade.notas().c_str()));

  // Proxima salvacao: para funcionar, o combo deve estar ordenado da mesma forma que a enum ResultadoSalvacao.
  gerador.combo_salvacao->setCurrentIndex((int)entidade.proxima_salvacao());

  // Tipo de visao.
  gerador.combo_visao->setCurrentIndex((int)entidade.tipo_visao());
  lambda_connect(gerador.combo_visao, SIGNAL(currentIndexChanged(int)), [this, &gerador] () {
    gerador.spin_raio_visao_escuro_quad->setEnabled(gerador.combo_visao->currentIndex() == ent::VISAO_ESCURO);
  });
  gerador.spin_raio_visao_escuro_quad->setValue(METROS_PARA_QUADRADOS * (entidade.has_alcance_visao() ? entidade.alcance_visao() : 18));
  gerador.spin_raio_visao_escuro_quad->setEnabled(entidade.tipo_visao() == ent::VISAO_ESCURO);

  // Preenche os atributos.
  PreencheConfiguraAtributos(this, gerador, entidade, proto_retornado);

  // Iniciativa.
  PreencheConfiguraDadosIniciativa(this, gerador, entidade, proto_retornado);

  // Dados de defesa.
  PreencheConfiguraDadosDefesa(this, gerador, entidade, proto_retornado);

  // Dados de ataque.
  PreencheConfiguraDadosAtaque(gerador, entidade, proto_retornado);

  // Preenche configura niveis.
  PreencheConfiguraNiveis(gerador, proto_retornado);

  // Preenche a parte de resistencias.
  PreencheConfiguraSalvacoes(this, gerador, proto_retornado);

  // Coisas que nao estao na UI.
  if (entidade.has_direcao_queda()) {
    proto_retornado->mutable_direcao_queda()->CopyFrom(entidade.direcao_queda());
  }
  if (entidade.has_desenha_base()) {
    proto_retornado->set_desenha_base(entidade.desenha_base());
  }

  // Ao aceitar o diálogo, aplica as mudancas.
  lambda_connect(dialogo, SIGNAL(accepted()),
                 [this, notificacao, entidade, dialogo, &gerador, &proto_retornado, &ent_cor, &luz_cor] () {
    ent::RecomputaDependencias(proto_retornado);
    if (gerador.campo_rotulo->text().isEmpty()) {
      proto_retornado->clear_rotulo();
    } else {
      proto_retornado->set_rotulo(gerador.campo_rotulo->text().toUtf8().constData());
    }
    QStringList lista_rotulos = gerador.lista_rotulos->toPlainText().split("\n", QString::SkipEmptyParts);
    proto_retornado->clear_rotulo_especial();
    for (const auto& rotulo : lista_rotulos) {
      proto_retornado->add_rotulo_especial(rotulo.toUtf8().constData());
    }
    google::protobuf::RepeatedPtrField<ent::EntidadeProto::Evento> eventos = ent::LeEventos(gerador.lista_eventos->toPlainText().toUtf8().constData());
    proto_retornado->mutable_evento()->Swap(&eventos);

    proto_retornado->set_tamanho(static_cast<ent::TamanhoEntidade>(gerador.slider_tamanho->sliderPosition()));
    if (gerador.checkbox_cor->checkState() == Qt::Checked) {
      proto_retornado->mutable_cor()->Swap(ent_cor.mutable_cor());
      proto_retornado->mutable_cor()->set_a(gerador.slider_alfa->value() / 100.0f);
    } else {
      proto_retornado->clear_cor();
    }
    if (gerador.spin_raio_quad->value() > 0.0f) {
      proto_retornado->mutable_luz()->mutable_cor()->Swap(luz_cor.mutable_cor());
      proto_retornado->mutable_luz()->set_raio_m(gerador.spin_raio_quad->value() * QUADRADOS_PARA_METROS);
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
      proto_retornado->mutable_modelo_3d()->set_id(gerador.combo_modelos_3d->currentText().toUtf8().constData());
    }
    proto_retornado->set_pontos_vida(gerador.spin_pontos_vida->value());
    proto_retornado->set_pontos_vida_temporarios(gerador.spin_pontos_vida_temporarios->value());
    proto_retornado->set_max_pontos_vida(gerador.spin_max_pontos_vida->value());
    float aura_m = gerador.spin_aura_quad->value() * QUADRADOS_PARA_METROS;
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
    proto_retornado->mutable_pos()->set_z(gerador.spin_translacao_quad->value() * QUADRADOS_PARA_METROS);
    proto_retornado->clear_translacao_z_deprecated();
    proto_retornado->set_proxima_salvacao((ent::ResultadoSalvacao)gerador.combo_salvacao->currentIndex());
    proto_retornado->set_tipo_visao((ent::TipoVisao)gerador.combo_visao->currentIndex());
    proto_retornado->mutable_dados_defesa()->set_imune_critico(gerador.checkbox_imune_critico->checkState() == Qt::Checked);
    proto_retornado->mutable_dados_ataque_globais()->set_dano_furtivo(gerador.linha_furtivo->text().toUtf8().constData());
    if (proto_retornado->tipo_visao() == ent::VISAO_ESCURO) {
      proto_retornado->set_alcance_visao(gerador.spin_raio_visao_escuro_quad->value() * QUADRADOS_PARA_METROS);
    }
    proto_retornado->mutable_tesouro()->set_tesouro(gerador.lista_tesouro->toPlainText().toUtf8().constData());
    proto_retornado->set_notas(gerador.texto_notas->toPlainText().toUtf8().constData());
    if (gerador.checkbox_iniciativa->checkState() == Qt::Checked) {
      proto_retornado->set_iniciativa(gerador.spin_iniciativa->value());
    } else {
      proto_retornado->clear_iniciativa();
    }

    if ((gerador.lista_ataques->currentRow() >= 0 && gerador.lista_ataques->currentRow() < proto_retornado->dados_ataque().size()) ||
        gerador.linha_dano->text().size() > 0) {
      AdicionaOuAtualizaAtaqueEntidade(gerador, proto_retornado);
    }
    if (gerador.spin_nivel_classe->value() > 0) {
      AdicionaOuEditaNivel(gerador, proto_retornado);
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
                     << gerador.linha_nevoa_min->text().toUtf8().constData();
        return;
      }
      int d_max = gerador.linha_nevoa_max->text().toInt(&ok);
      if (!ok || d_min >= d_max) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, nevoa maxima invalida: "
                     << gerador.linha_nevoa_max->text().toUtf8().constData();
        return;
      }
      proto_retornado->mutable_nevoa()->set_minimo(d_min);
      proto_retornado->mutable_nevoa()->set_maximo(d_max);
    } else {
      proto_retornado->clear_nevoa();
    }
    // Descricao.
    proto_retornado->set_descricao_cenario(gerador.campo_descricao->text().toUtf8().constData());
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
        nome = gerador.combo_fundo->itemText(indice).split(":")[1].toUtf8().constData();
      } else {
        nome = gerador.combo_fundo->itemText(indice).toUtf8().constData();
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
                     << gerador.linha_altura->text().toUtf8().constData();
        return;
      }
      int altura = gerador.linha_altura->text().toInt(&ok);
      if (!ok) {
        LOG(WARNING) << "Descartando alteracoes tabuleiro, altura invalido: "
                     << gerador.linha_largura->text().toUtf8().constData();
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
