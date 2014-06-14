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
  Tecla_AltEsquerdo = 57,
  Tecla_AltDireito = 58,
  Tecla_A = 0x41,
  Tecla_C = 0x43,
  Tecla_D = 0x44,
  Tecla_G = 0x47,
  Tecla_I = 0x49,
  Tecla_L = 0x4c,
  Tecla_Q = 0x51,
  Tecla_R = 0x52,
  Tecla_S = 0x53,
  Tecla_V = 0x56,
  Tecla_Y = 0x59,
  Tecla_Z = 0x5a,
};

enum modificadores_e {
  Modificador_Shift = 0x02000000,
  Modificador_Ctrl = 0x04000000,
  Modificador_Alt = 0x08000000,
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

  void TrataBotaoMousePressionado(botoesmouse_e botao, unsigned int modificadores, int x, int y);
  void TrataMovimentoMouse(int x, int y);
  void TrataDuploCliqueMouse(botoesmouse_e botao, modificadores_e modificadores, int x, int y);
  void TrataBotaoMouseLiberado();
  void TrataRodela(int delta);

  // Interface receptor.
  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 private:
  // Assumindo uma maquina de estados bem simples, que vai do ESTADO_OCIOSO pros outros e volta.
  enum estado_e {
    ESTADO_TEMPORIZANDO_TECLADO,
    ESTADO_TEMPORIZANDO_MOUSE,
    ESTADO_OUTRO,
  };

  void TrataAcaoTemporizadaTeclado();
  void TrataAcaoTemporizadaMouse();
  // Retorna o estado para ESTADO_OCIOSO.
  void MudaEstado(estado_e estado);

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
