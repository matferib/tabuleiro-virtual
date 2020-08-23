#include "ifg/qt/visualizador3d.h"

#include <QDesktopWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QScreen>
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

QSurfaceFormat Formato() {
  QSurfaceFormat formato;
  formato.setVersion(2, 1);
  formato.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
  //formato.setSwapBehavior(QSurfaceFormat::SingleBuffer);
  formato.setSwapInterval(0);  // vsync off
  formato.setRedBufferSize(8);
  formato.setGreenBufferSize(8);
  formato.setBlueBufferSize(8);
  // Nao faca isso! Isso aqui deixara a janela transparente, quebrando a transparencia.
  //formato.setAlphaBufferSize(8);
  formato.setDepthBufferSize(24);
  formato.setStencilBufferSize(1);
  formato.setRenderableType(QSurfaceFormat::OpenGL);
  //formato.setSamples(2);
  return formato;
}

}  // namespace

Visualizador3d::Visualizador3d(
    const ent::Tabelas& tabelas,
    m3d::Modelos3d* m3d,
    tex::Texturas* texturas,
    TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai)
    :  QWidget(pai),
       tabelas_(tabelas),
       m3d_(m3d),
       texturas_(texturas),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro),
       contexto_(pai) {
  //const ent::OpcoesProto& opcoes = tabuleiro->Opcoes();
  tipo_iluminacao_ = ent::OpcoesProto::TI_ESPECULAR;
  central_->RegistraReceptor(this);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  std::cerr << "phyx: " << QGuiApplication::primaryScreen()->physicalDotsPerInchX() << std::endl;
  std::cerr << "phyy: " << QGuiApplication::primaryScreen()->physicalDotsPerInchY() << std::endl;
  std::cerr << "logx: " << QGuiApplication::primaryScreen()->logicalDotsPerInchX() << std::endl;
  std::cerr << "logy: " << QGuiApplication::primaryScreen()->logicalDotsPerInchY() << std::endl;
  //std::cerr << "screen: " << *QGuiApplication::primaryScreen();

  IniciaGL();
}

Visualizador3d::~Visualizador3d() {
  gl::FinalizaGl();
}

QOpenGLFramebufferObjectFormat FormatoFramebuffer(bool anti_aliasing) {
  QOpenGLFramebufferObjectFormat formato;
  formato.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  formato.setInternalTextureFormat(GL_RGB);
  formato.setMipmap(false);
  formato.setTextureTarget(GL_TEXTURE_2D);
  formato.setSamples(anti_aliasing ? 2 : 0);
  return formato;
}

// reimplementacoes
void Visualizador3d::IniciaGL() {
  LOG(INFO) << "Inicializando GL.......................";
  try {
    //contexto_.setFormat(Formato());
    contexto_.create();
    //surface_.setFormat(Formato());
    surface_.create();
    PegaContexto();
    const auto& opcoes = tabuleiro_->Opcoes();
    const float escala_fonte = opcoes.escala() > 0.0
          ? opcoes.escala()
          : opcoes.renderizacao_em_framebuffer_fixo() ? 1.0 : QApplication::desktop()->devicePixelRatio();
    gl::IniciaGl(static_cast<gl::TipoLuz>(tipo_iluminacao_), escala_fonte);
    tabuleiro_->IniciaGL();
    LOG(INFO) << "GL iniciado";
    LiberaContexto();
  } catch (const std::logic_error& erro) {
    // Este log de erro eh pro caso da aplicacao morrer e nao conseguir mostrar a mensagem.
    LOG(ERROR) << "Erro na inicializacao GL " << erro.what();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErro(erro.what()));
  }
}

void Visualizador3d::resizeEvent(QResizeEvent *event) {
  int height = event->size().height();
  int width = event->size().width();
  QWidget::resizeEvent(event);
  RecriaFramebuffer(width, height, tabuleiro_->Opcoes());
}

void Visualizador3d::RecriaFramebuffer(int width, int height, const ent::OpcoesProto& opcoes) {
  const bool fixo = opcoes.renderizacao_em_framebuffer_fixo();

  PegaContexto();
  if (framebuffer_ != nullptr) {
    framebuffer_->release();
    framebuffer_.reset();
  }
  if (fixo) {
    const int tam = opcoes.tamanho_framebuffer_fixo();
    if (width >= height) {
      height = tam / (static_cast<float>(width) / static_cast<float>(height));
      width = tam;
    } else {
      width = tam / (static_cast<float>(height) / static_cast<float>(width));
      height = tam;
    }
  } else {
    const float dpr = QApplication::desktop()->devicePixelRatio();
    width *= dpr;
    height *= dpr;
  }
  framebuffer_.reset(new QOpenGLFramebufferObject(width, height, FormatoFramebuffer(opcoes.anti_aliasing())));
  LOG(INFO) << "w: " << width << ", h: " << height;
  tabuleiro_->TrataRedimensionaJanela(width, height);
  update();
  LiberaContexto();
}

namespace {
unsigned long long TempoMs(const boost::timer::cpu_timer& timer) {
  constexpr unsigned long long DIV_NANO_PARA_MS = 1000000ULL;
  return timer.elapsed().wall / DIV_NANO_PARA_MS;
}

void ImprimeTempo(const std::string& label, unsigned long long &total, int &c) {
  if (c == 10) {
    LOG(INFO) << "media ultimos 10 frames " << label << ": " << (static_cast<float>(total) / c) << "ms";
    total = 0;
    c = 0;
  }
}
}  // namespace

void Visualizador3d::paintEvent(QPaintEvent* event) {
  PegaContexto();
  framebuffer_->bind();
  tabuleiro_->Desenha();
  framebuffer_->release();
  contexto_.swapBuffers(&surface_);


  if (0) {
    boost::timer::cpu_timer timer;
    timer.start();
    glFinish();
    timer.stop();
    static unsigned long long total = 0ULL;
    static int c = 0;
    total += TempoMs(timer);
    ImprimeTempo("finish", total, ++c);
  }

  boost::timer::cpu_timer timer_grab;
  timer_grab.start();
  QImage image = framebuffer_->toImage();
  timer_grab.stop();
  if (0) {
    const ent::OpcoesProto& opcoes = tabuleiro_->Opcoes();
    QRect source_rect(0, 0, opcoes.tamanho_framebuffer_fixo(), opcoes.tamanho_framebuffer_fixo());
    // O QT transforma o 0 em default framebuffer.
    QRect dest_rect(0, 0, width(), height()); // geometry();
    QOpenGLFramebufferObject::blitFramebuffer(nullptr, dest_rect, framebuffer_.get(), source_rect, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
  static unsigned long long total_grab = 0ULL;
  static int c_grab = 0;
  total_grab += TempoMs(timer_grab);
  ImprimeTempo("grab", total_grab, ++c_grab);

  if (1) {
    QPainter painter(this);
    QRect rect(0, 0, width(), height());
    boost::timer::cpu_timer timer;
    timer.start();
    painter.drawImage(rect, image);
    timer.stop();
    static unsigned long long total_draw = 0ULL;
    static int c_draw = 0;
    total_draw += TempoMs(timer);
    ImprimeTempo("drawImage", total_draw, ++c_draw);
  }
  LiberaContexto();
  event->accept();
}

void Visualizador3d::PegaContexto() {
  // Nota sobre assimetria de PegaContexto e LiberaContexto:
  // A funcao nao empilha nada, apenas seta qual o FB o OpenGL vai usar.
  // Portanto, chamar 10 vezes nao ocasiona problema.
  // Entao porque precisa do contador de referencia?
  // Porque no inicio do temporizador a gente pega o contexto grafico (Principal::Temporizador) e chama as notificacoes.
  // O mesmo ocorre nos tratamento de eventos neste arquivo.
  // Isso permite que todas as funcoes de temporizacao e eventos rodem com o contexto correto.
  // No entanto, ao abrir novas janelas dentro desta thread, elas roubam o contexto (versão, objeto, opcoes etc).
  // Apos o fechamento, precisamos do contexto de novo para coisas graficas, como atualizar luzes por exemplo.
  // Ai chamamos PegaContexto de novo dentro de uma chamada ja de PegaContexto.
  // Ocorre que, ao sair desse escopo, o LiberaContexto ira liberar o contexto se nao houver contador de referencia.
  // Então, a chamada pai ficara sem contexto para continuar.
  // O certo mesmo seria pegar todas as aberturas de janela, liberar o contexto e pegar de novo. Mas isso é fragil e dificil de manter.
  // Esta solução assimétrica resolve o problema. Vai haver mais de um makeCurrent por doneCurrent, mas isso nao vaza memoria.
  contexto_.makeCurrent(&surface_);
  ++contexto_cref_;
  //contexto_ = QOpenGLContext::currentContext();
}

void Visualizador3d::LiberaContexto() {
  if (--contexto_cref_ == 0) {
    //LOG(INFO) << "liberando contexto";
    contexto_.doneCurrent();
    //contexto_ = nullptr;
  }
  if (contexto_cref_ < 0) {
    LOG(ERROR) << "Contador de contexto negativo";
  }
}


// notificacao
bool Visualizador3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_ATUALIZAR_OPCOES: {
      LOG(INFO) << "alterando escala para: " << notificacao.opcoes().escala();
      gl::AlteraEscala(notificacao.opcoes().escala() > 0
          ? notificacao.opcoes().escala()
          : notificacao.opcoes().renderizacao_em_framebuffer_fixo() ? 1.0 : QApplication::desktop()->devicePixelRatio());
      RecriaFramebuffer(width(), height(), notificacao.opcoes());
      break;
    }
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
    case ntf::TN_TEMPORIZADOR_MOUSE: {
      tabuleiro_->TrataMouseParadoEm(notificacao.pos().x(), notificacao.pos().y());
      break;
    }
    case ntf::TN_INICIADO: {
      // chama o resize pra iniciar a geometria e desenha a janela
      //IniciaGL();
      //resizeGL(width(), height());
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
      // Como abriu outra janela, ela pode ter mexido no contexto.
      PegaContexto();
      tabuleiro_->TrataNotificacao(*n);
      LiberaContexto();
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
      PegaContexto();
      tabuleiro_->TrataNotificacao(*n);
      LiberaContexto();
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
      PegaContexto();
      TrataNotificacao(*n);
      tabuleiro_->TrataNotificacao(*n);
      LiberaContexto();
      break;
    }
    case ntf::TN_TEMPORIZADOR:
      update();
      break;
    default: ;
  }
  return true;
}

// teclado.
void Visualizador3d::keyPressEvent(QKeyEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataTeclaPressionada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
  LiberaContexto();
}

void Visualizador3d::keyReleaseEvent(QKeyEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataTeclaLiberada(
      TeclaQtParaTratadorTecladoMouse(event->key()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()));
  event->accept();
  LiberaContexto();
}

int Visualizador3d::XPara3d(int x) const {
  float fracao = static_cast<float>(x) / width();
  return fracao * framebuffer_->size().width();
}

int Visualizador3d::YPara3d(int y) const {
  float fracao = static_cast<float>(y) / height();
  return fracao * framebuffer_->size().height();
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataBotaoMousePressionado(
       BotaoMouseQtParaTratadorTecladoMouse(event->button()),
       ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
       XPara3d(event->x()),
       YPara3d(height() - event->y()));
  event->accept();
  LiberaContexto();
}

void Visualizador3d::mouseReleaseEvent(QMouseEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataBotaoMouseLiberado();
  event->accept();
  LiberaContexto();
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
  PegaContexto();
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      XPara3d(event->x()), YPara3d(height() - event->y()));
  event->accept();
  LiberaContexto();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  PegaContexto();
  int x = event->globalX();
  int y = event->globalY();
  if (teclado_mouse_->TrataMovimentoMouse(XPara3d(event->x()), YPara3d(height() - event->y()))) {
    QCursor::setPos(x_antes_, y_antes_);
  } else {
    x_antes_ = x;
    y_antes_ = y;
  }
  LiberaContexto();
  event->accept();
}

void Visualizador3d::wheelEvent(QWheelEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataRodela(event->delta());
  event->accept();
  LiberaContexto();
}

}  // namespace qt
}  // namespace ifg
