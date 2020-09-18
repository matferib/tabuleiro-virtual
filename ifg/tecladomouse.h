#ifndef IFG_TECLADO_H
#define IFG_TECLADO_H

#include <vector>
#include "ntf/notificacao.h"

namespace ent {
class Tabuleiro;
}  // namespace ent
namespace ntf {
class CentralNotificacoes;
}  // namespace ntf

namespace ifg {

// Codigos QT: http://qt-project.org/doc/qt-4.8/qt.html#Key-enum.
enum teclas_e {
  Tecla_0 = 0x30,
  Tecla_1 = 0x31,
  Tecla_2 = 0x32,
  Tecla_3 = 0x33,
  Tecla_4 = 0x34,
  Tecla_5 = 0x35,
  Tecla_6 = 0x36,
  Tecla_7 = 0x37,
  Tecla_8 = 0x38,
  Tecla_9 = 0x39,
  Tecla_F1 = 0x01000030,
  Tecla_F2 = 0x01000031,
  Tecla_F3 = 0x01000032,
  Tecla_F4 = 0x01000033,
  Tecla_F5 = 0x01000034,
  Tecla_F6 = 0x01000035,
  Tecla_Espaco = 0x20,
  Tecla_Esc = 0x01000000,
  Tecla_EnterKeypad = 0x01000005,
  Tecla_EnterNormal = 0x01000004,
  Tecla_Backspace = 0x01000003,
  Tecla_Delete = 0x01000007,
  Tecla_Cima = 0x01000013,
  Tecla_Baixo = 0x01000015,
  Tecla_Esquerda = 0x01000012,
  Tecla_Direita = 0x01000014,
  // Inventados pois nao tem no QT.
  Tecla_Alt = 0x01100007,
  Tecla_AltEsquerdo = 0x01000023,
  Tecla_AltDireito = 0x01001103,
  // Fim inventados
  Tecla_A = 0x41,
  Tecla_C = 0x43,
  Tecla_D = 0x44,
  Tecla_E = 0x45,
  Tecla_F = 0x46,
  Tecla_G = 0x47,
  Tecla_H = 0x48,
  Tecla_I = 0x49,
  Tecla_J = 0x4a,
  Tecla_K = 0x4b,
  Tecla_L = 0x4c,
  Tecla_M = 0x4d,
  Tecla_N = 0x4e,
  Tecla_O = 0x4f,
  Tecla_P = 0x50,
  Tecla_Q = 0x51,
  Tecla_R = 0x52,
  Tecla_S = 0x53,
  Tecla_T = 0x54,
  Tecla_V = 0x56,
  Tecla_Y = 0x59,
  Tecla_Z = 0x5a,
  Tecla_Igual = 0x3d, // usada como mais tb durante parsing.
  Tecla_Mais = 0x2b,  // numpad.
  Tecla_Menos = 0x2d, // menos de qq um.
  Tecla_Shift = 0x01000020,
  Tecla_Ctrl = 0x01000021,
  Tecla_Tab = 0x01000001,
  Tecla_TabInvertido = 0x01000002,
  Tecla_Insert = 0x01000006,
};

enum modificadores_e {
  Modificador_Shift = 0x02000000,
  Modificador_Ctrl =  0x04000000,
  Modificador_Alt =   0x08000000,
  Modificador_Meta =  0x10000000,
  Modificador_AltGr = 0x40000000,
};

enum botoesmouse_e {
  Botao_Esquerdo = 0x1,
  Botao_Direito  = 0x2,
  Botao_Meio     = 0x4,
};

// Classe para lidar com tratamentos de teclas independente de plataforma. Cada plataforma
// devera implementar a parte de io.
class TratadorTecladoMouse : public ntf::Receptor {
 public:
  TratadorTecladoMouse(ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro);
  ~TratadorTecladoMouse();

  void TrataTeclaPressionada(teclas_e tecla, modificadores_e modificadores);
  void TrataTeclaLiberada(teclas_e tecla, modificadores_e modificadores);

  void TrataBotaoMousePressionado(botoesmouse_e botao, unsigned int modificadores, int x, int y);
  /** @return true se o mouse deve ser restaurado. */
  bool TrataMovimentoMouse(int x, int y);
  void TrataDuploCliqueMouse(botoesmouse_e botao, unsigned int modificadores, int x, int y);
  void TrataBotaoMouseLiberado();
  void TrataRodela(int delta);
  void TrataPincaEscala(float fator);
  // Inicio de pinca.
  void TrataInicioPinca(int x1, int y1, int x2, int y2);

  // Interface receptor.
  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

  // Retorna o estado.
  // Assumindo uma maquina de estados bem simples, que vai do ESTADO_TEMPORIZANDO_MOUSE pros outros e volta.
  enum estado_e {
    ESTADO_TEMPORIZANDO_TECLADO,
    ESTADO_TEMPORIZANDO_MOUSE,
    //ESTADO_MOSTRANDO_DIALOGO,
    ESTADO_OUTRO,
  };
  void MudaEstado(estado_e estado);

 private:
  void TrataAcaoTemporizadaTeclado();
  void TrataAcaoTemporizadaMouse();

  estado_e estado_;
  // Ultimas coordenadas do mouse (em OpenGL).
  int ultimo_x_;
  int ultimo_y_;
  // Temporizador para teclas em sequencia.
  int temporizador_teclado_;
  int temporizador_mouse_;
  // As teclas pressionadas ate o temporizador estourar ou enter ser pressionado.
  std::vector<teclas_e> teclas_;
  ntf::CentralNotificacoes* central_;
  ent::Tabuleiro* tabuleiro_;
};

}  // namespace ifg

#endif  // IFG_TECLADO_H
