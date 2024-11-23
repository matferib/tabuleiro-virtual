#include "ifg/imgui/interface_imgui.h"

#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <imgui_impl_win32.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <Windowsx.h>
#include <WinUser.h>
#include <GL/GL.h>
#include <tchar.h>

#include "ent/tabuleiro.h"
#include "ifg/tecladomouse.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ifg::imgui {

// Estado da interface. Colocando aqui pq é tudo singleton.
namespace {
HWND hwnd_ = nullptr;
WNDCLASSEXW wc_;
bool done_ = false;
bool show_demo_window_ = true;
unsigned long long last_update_ = 0;

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };
// Data
static HGLRC            hrc_;
static WGL_WindowData   main_window_;
static int              vp_width_;
static int              vp_height_;
ifg::TratadorTecladoMouse* g_teclado_mouse_ = nullptr;
ent::Tabuleiro* g_tabuleiro_ = nullptr;

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data) {
  HDC hDc = ::GetDC(hWnd);
  PIXELFORMATDESCRIPTOR pfd = { 0 };
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  const int pf = ::ChoosePixelFormat(hDc, &pfd);
  if (pf == 0)
    return false;
  if (::SetPixelFormat(hDc, pf, &pfd) == FALSE) {
    return false;
  }
  ::ReleaseDC(hWnd, hDc);

  data->hDC = ::GetDC(hWnd);
  if (!hrc_) {
    hrc_ = wglCreateContext(data->hDC);
  }
  return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data) {
  wglMakeCurrent(nullptr, nullptr);
  ::ReleaseDC(hWnd, data->hDC);
}

unsigned int Modificadores() {
  unsigned int modificadores = 0;
  if (GetKeyState(VK_MENU) != 0) {
    modificadores |= ifg::Modificador_Alt;
  }
  if (GetKeyState(VK_SHIFT) != 0) {
    modificadores |= ifg::Modificador_Shift;
  }
  if (GetKeyState(VK_CONTROL) != 0) {
    modificadores |= ifg::Modificador_Ctrl;
  }
  return modificadores;
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (LRESULT res = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam); res != 0) return res;

  switch (msg) {
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED) {
      vp_width_ = LOWORD(lParam);
      vp_height_ = HIWORD(lParam);
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  //---------------
  // Mouse commands
  //---------------
  case WM_MOUSEWHEEL: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataRodela(GET_WHEEL_DELTA_WPARAM(wParam));
    return 0;
  }
  case WM_LBUTTONDOWN: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataBotaoMousePressionado(ifg::Botao_Esquerdo, Modificadores(), GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_LBUTTONDBLCLK: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataDuploCliqueMouse(ifg::Botao_Esquerdo, Modificadores(), GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_RBUTTONDBLCLK: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataDuploCliqueMouse(ifg::Botao_Direito, Modificadores(), GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_MOUSEMOVE: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataMovimentoMouse(GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_MBUTTONDOWN: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataBotaoMousePressionado(ifg::Botao_Meio, Modificadores(), GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_RBUTTONDOWN: {
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataBotaoMousePressionado(ifg::Botao_Direito, Modificadores(), GET_X_LPARAM(lParam), vp_height_ - GET_Y_LPARAM(lParam));
    return 0;
  }
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    if (ImGui::GetIO().WantCaptureMouse) return 0;
    g_teclado_mouse_->TrataBotaoMouseLiberado();
    return 0;

  //---------
  // Keyboard
  //---------
  case WM_KEYDOWN: {
    if (ImGui::GetIO().WantCaptureKeyboard) return 0;
    g_teclado_mouse_->TrataTeclaPressionada(static_cast<ifg::teclas_e>(wParam), static_cast<modificadores_e>(Modificadores()));
    return 0;
  }
  }

  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

}  // namespace

InterfaceImgui::InterfaceImgui(const ent::Tabelas& tabelas, TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
    : ifg::InterfaceGrafica(tabelas, teclado_mouse, tabuleiro, central) {

  // Create application window
  //ImGui_ImplWin32_EnableDpiAwareness();
  wc_ = { sizeof(wc_), CS_OWNDC | CS_DBLCLKS, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Tabileiro Virtual", nullptr };
  ::RegisterClassExW(&wc_);
  hwnd_ = ::CreateWindowW(wc_.lpszClassName, L"Tabuleiro Virtual", WS_OVERLAPPEDWINDOW, 100, 100, kWindowWidth, kWindowHeight, nullptr, nullptr, wc_.hInstance, nullptr);

  // Initialize OpenGL
  if (!CreateDeviceWGL(hwnd_, &main_window_)) {
    CleanupDeviceWGL(hwnd_, &main_window_);
    ::DestroyWindow(hwnd_);
    ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
    throw std::logic_error("failed to initialize OpenGL");
  }
  wglMakeCurrent(main_window_.hDC, hrc_);

  // Show the window
  ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd_);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_InitForOpenGL(hwnd_);
  ImGui_ImplOpenGL2_Init();

  // TODO como reconfigurar? Usar KillTimer ao receber notificacao de opcoes proto.
  g_tabuleiro_ = tabuleiro;
  g_teclado_mouse_ = teclado_mouse;
  const auto& opcoes = tabuleiro_->Opcoes();
  float intervalo_notificacao_ms = 1000.0f / opcoes.fps();
  SetTimer(hwnd_,
           0,  // timer identifier (wparam)
           static_cast<unsigned int>(intervalo_notificacao_ms),
           (TIMERPROC)NULL);     // no timer callback 

  const float escala_fonte = opcoes.escala() > 0.0
    ? opcoes.escala()
    : 1.0f;
  gl::IniciaGl(static_cast<gl::TipoLuz>(gl::TL_POR_PIXEL_ESPECULAR), escala_fonte);
  tabuleiro_->IniciaGL();
}

InterfaceImgui::~InterfaceImgui() {
  KillTimer(hwnd_, 0);
  gl::FinalizaGl();
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceWGL(hwnd_, &main_window_);
  wglDeleteContext(hrc_);
  ::DestroyWindow(hwnd_);
  ::UnregisterClassW(wc_.lpszClassName, wc_.hInstance);
}

// Para testes...
void DesenhaImgui() {
  // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
  if (show_demo_window_) {
    ImGui::ShowDemoWindow(&show_demo_window_);
  }
}

void InterfaceImgui::Executa() {
  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //io.Fonts->AddFontDefault();
  //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  //IM_ASSERT(font != nullptr);

  // Main loop
  while (!done_) {
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32 backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      switch (msg.message) {
      case WM_QUIT: 
        done_ = true;
        break;
      case WM_TIMER:
        // Realiza a notificação de todos.
        auto* notificacao = new ntf::Notificacao;
        notificacao->set_tipo(ntf::TN_TEMPORIZADOR);
        central_->AdicionaNotificacao(notificacao);
        central_->Notifica();
        break;
      }
    }
    if (done_) break;
    if (::IsIconic(hwnd_)) {
      ::Sleep(10);
      continue;
    }

    tabuleiro_->TrataRedimensionaJanela(vp_width_, vp_height_);
    tabuleiro_->Desenha();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DesenhaImgui();

    // Rendering
    gl::UsaPrograma(0);  // Fala pro imgui usar o shader 0, senão ele tenta os do tabuleiro e da errado.
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Present
    ::SwapBuffers(main_window_.hDC);
  }
}


bool InterfaceImgui::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
  case ntf::TN_SAIR:
      done_ = true;
      return true;
  case ntf::TN_INICIAR: {
      auto* notificacao_iniciado = new ntf::Notificacao;
      notificacao_iniciado->set_tipo(ntf::TN_INICIADO);
      central_->AdicionaNotificacao(notificacao_iniciado);
      return true;
  }
  case ntf::TN_TEMPORIZADOR:
    //tabuleiro_->Desenha();
    return true;
  default:
    return InterfaceGrafica::TrataNotificacao(notificacao);
  }
}


}  // namespace ifg::qt
