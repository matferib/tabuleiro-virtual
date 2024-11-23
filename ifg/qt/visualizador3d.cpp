#include "ifg/qt/visualizador3d.h"

#include <QtCore/QAbstractNativeEventFilter>
#include <QtGui/QWindow>
#include <QtGui/QMouseEvent>
#include <QtGui/QOpenGLContext>
#include <QtGui/QScreen>
#include <algorithm>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_win32.h>

#include "arq/arquivo.h"
#include "ent/constantes.h"
#include "ent/recomputa.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
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

#if USAR_QT5
#define EVENT_X(event) event->x()
#define EVENT_Y(event) event->y()
#define EVENT_GLOBAL_X(event) event->globalX()
#define EVENT_GLOBAL_Y(event) event->globalY()
#else
#define EVENT_X(event) event->position().x()
#define EVENT_Y(event) event->position().y()
#define EVENT_GLOBAL_X(event) event->globalPosition().x()
#define EVENT_GLOBAL_Y(event) event->globalPosition().y()
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ifg {
namespace qt {
namespace {

using namespace std;
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

}  // namespace

Visualizador3d::Visualizador3d(
    const ent::Tabelas& tabelas,
    m3d::Modelos3d* m3d,
    tex::Texturas* texturas,
    TratadorTecladoMouse* teclado_mouse,
    ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro, QWidget* pai, bool imgui)
    :  QOpenGLWidget(pai),
       tabelas_(tabelas),
       m3d_(m3d),
       texturas_(texturas),
       teclado_mouse_(teclado_mouse),
       central_(central), tabuleiro_(tabuleiro),
       imgui_(imgui) {
  //const ent::OpcoesProto& opcoes = tabuleiro->Opcoes();
  tipo_iluminacao_ = ent::OpcoesProto::TI_ESPECULAR;
  central_->RegistraReceptor(this);
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  setUpdateBehavior(QOpenGLWidget::PartialUpdate);

  std::cerr << "phyx: " << QGuiApplication::primaryScreen()->physicalDotsPerInchX() << std::endl;
  std::cerr << "phyy: " << QGuiApplication::primaryScreen()->physicalDotsPerInchY() << std::endl;
  std::cerr << "logx: " << QGuiApplication::primaryScreen()->logicalDotsPerInchX() << std::endl;
  std::cerr << "logy: " << QGuiApplication::primaryScreen()->logicalDotsPerInchY() << std::endl;
  //std::cerr << "screen: " << *QGuiApplication::primaryScreen();
}

namespace {

void FinalizaImGui() {
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}

}  // namespace

Visualizador3d::~Visualizador3d() {
  FinalizaImGui();
  gl::FinalizaGl();
}

namespace {

class ImguiEventFilter : public QAbstractNativeEventFilter {
public:
  ImguiEventFilter(Visualizador3d& v3d, HWND hwnd) : hwnd_(hwnd), v3d_(v3d) {}
  ~ImguiEventFilter() override {}
  // Doc QT:
  // If you want to filter the message out, i.e. stop it being handled further, return true; otherwise return false.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override
#else
  bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override
#endif
  {
    MSG* msg = static_cast<MSG*>(message);
    LRESULT dummy;
    if ((dummy = ImGui_ImplWin32_WndProcHandler(hwnd_, msg->message, msg->wParam, msg->lParam)) != 0) {
      if (result != nullptr) {
        *result = dummy;
      }
      //v3d_.setAttribute(Qt::WA_TransparentForMouseEvents, true);
      return true;
    }
    //v3d_.setAttribute(Qt::WA_TransparentForMouseEvents, false);
    return false;
  }

private:
  HWND hwnd_ = nullptr;
  Visualizador3d& v3d_;
};

void IniciaImGui(Visualizador3d& v3d, HWND hwnd) {
    QApplication::instance()->installNativeEventFilter(new ImguiEventFilter(v3d, hwnd));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL2_Init();
}

}  // namespace

// reimplementacoes OpenGL.
void Visualizador3d::initializeGL() {   
  LOG(INFO) << "Inicializando GL.......................";
  try {
    const auto& opcoes = tabuleiro_->Opcoes();
    const float escala_fonte = opcoes.escala() > 0.0
          ? opcoes.escala()
          : QGuiApplication::primaryScreen()->devicePixelRatio();
    gl::IniciaGl(static_cast<gl::TipoLuz>(tipo_iluminacao_), escala_fonte);
    tabuleiro_->IniciaGL();
    if (imgui_) {
      IniciaImGui(*this, HWND(this->effectiveWinId()));
    }

    LOG(INFO) << "GL iniciado";
  } catch (const std::logic_error& erro) {
    // Este log de erro eh pro caso da aplicacao morrer e nao conseguir mostrar a mensagem.
    LOG(ERROR) << "Erro na inicializacao GL " << erro.what();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErro(erro.what()));
  }
}

void Visualizador3d::resizeGL(int w, int h) {
  const float dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
  w *= dpr;
  h *= dpr;
  //LOG(INFO) << "w x h: " << w << "x" << h;
  tabuleiro_->TrataRedimensionaJanela(w, h);
}

namespace {
unsigned long TempoMs(const boost::timer::cpu_timer& timer) {
  return timer.elapsed().wall / ent::DIV_NANO_PARA_MS;
}

}  // namespace


namespace {

void DesenhaImgui(void* hwnd) {
  ImGui_ImplOpenGL2_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  static bool show_demo_window = true;
  if (show_demo_window) {
    ImGui::ShowDemoWindow(&show_demo_window);
  }
  ImGui::End();
  gl::UsaPrograma(0);  // fala pro imgui usar o shader proprio.
  ImGui::Render();
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

}  // namespace

void Visualizador3d::paintGL() {
  if (tabuleiro_->Opcoes().pular_frames() && skip_-- > 0) {
    timer_.stop();
    LOG(INFO) << "skipping: one frame, still " << skip_ << " left";
    return;
  }
  glFinish();
  timer_.stop();
  unsigned long passou_ms = TempoMs(timer_);
  timer_.start();
  //LOG(INFO) << "passou " << passou_ms;
  skip_ = std::min(passou_ms * (static_cast<unsigned long>(tabuleiro_->Opcoes().fps())) / 1000ULL, 10ULL);
  tabuleiro_->Desenha();
  if (imgui_) {
    DesenhaImgui(HWND(this->effectiveWinId()));
  }

  glFlush();
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
  makeCurrent();
  ++contexto_cref_;
}

void Visualizador3d::LiberaContexto() {
  if (--contexto_cref_ == 0) {
    doneCurrent();
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
          : QGuiApplication::primaryScreen()->devicePixelRatio());
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
  const float dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
  return x * dpr;
}

int Visualizador3d::YPara3d(int y) const {
  const float dpr = QGuiApplication::primaryScreen()->devicePixelRatio();
  return y * dpr;
}

// mouse

void Visualizador3d::mousePressEvent(QMouseEvent* event) {
  PegaContexto();
  teclado_mouse_->TrataBotaoMousePressionado(
       BotaoMouseQtParaTratadorTecladoMouse(event->button()),
       ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
       XPara3d(EVENT_X(event)),
       YPara3d(height() - EVENT_Y(event)));
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
    auto* event2 =
#if USAR_QT5
      new QMouseEvent(*event)
#else
      event->clone()
#endif
    ;
    mousePressEvent(event);
    mousePressEvent(event2);
    delete event2;
    return;
  }
  PegaContexto();
  teclado_mouse_->TrataDuploCliqueMouse(
      BotaoMouseQtParaTratadorTecladoMouse(event->button()),
      ModificadoresQtParaTratadorTecladoMouse(event->modifiers()),
      XPara3d(EVENT_X(event)), YPara3d(height() - EVENT_Y(event)));
  event->accept();
  LiberaContexto();
}

void Visualizador3d::mouseMoveEvent(QMouseEvent* event) {
  PegaContexto();
  int x = EVENT_GLOBAL_X(event);
  int y = EVENT_GLOBAL_Y(event);
  if (teclado_mouse_->TrataMovimentoMouse(XPara3d(EVENT_X(event)), YPara3d(height() - EVENT_Y(event)))) {
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
  teclado_mouse_->TrataRodela(event->angleDelta().y());
  event->accept();
  LiberaContexto();
}

}  // namespace qt
}  // namespace ifg
