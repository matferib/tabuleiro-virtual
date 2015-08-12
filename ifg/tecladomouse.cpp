#include "ifg/tecladomouse.h"
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/tabuleiro.h"
#include "log/log.h"

#define LOGT(X) VLOG(1)

namespace ifg {

namespace {

// Temporizadores * 10ms.
const float MAX_TEMPORIZADOR_TECLADO_S = 3;
const float  MAX_TEMPORIZADOR_MOUSE_S = 1;

const float MAX_TEMPORIZADOR_TECLADO = MAX_TEMPORIZADOR_TECLADO_S * ATUALIZACOES_POR_SEGUNDO;
const float MAX_TEMPORIZADOR_MOUSE = MAX_TEMPORIZADOR_MOUSE_S * ATUALIZACOES_POR_SEGUNDO;

int CalculaDanoSimples(const std::vector<teclas_e>::const_iterator& inicio_teclas_normal,
                       const std::vector<teclas_e>::const_iterator& fim_teclas_normal) {
  std::vector<teclas_e>::const_reverse_iterator inicio_teclas(fim_teclas_normal);
  std::vector<teclas_e>::const_reverse_iterator fim_teclas(inicio_teclas_normal);

  int delta = 0;
  int multiplicador = 1;
  for (auto it = inicio_teclas; it < fim_teclas; ++it) {
    if (*it < Tecla_0 || *it > Tecla_9) {
      LOG(WARNING) << "Tecla inválida para delta pontos de vida";
      continue;
    }
    delta += (*it - Tecla_0) * multiplicador;
    multiplicador *= 10;
  }
  VLOG(1) << "Tratando acao de delta pontos de vida, total: " << delta;
  return delta;
}

// Calcula o dano acumulado no vetor de teclas.
const std::vector<int> CalculaDano(const std::vector<teclas_e>::const_iterator& inicio_teclas,
                                   const std::vector<teclas_e>::const_iterator& fim_teclas) {
  std::vector<int> result;
  auto it_inicio = inicio_teclas;
  for (auto it = inicio_teclas; it < fim_teclas; ++it) {
    if (*it == Tecla_Espaco) {
      result.push_back(CalculaDanoSimples(it_inicio, it));
      it_inicio = it + 1;  // pula o espaco.
    }
  }
  result.push_back(CalculaDanoSimples(it_inicio, fim_teclas));
  return result;
}

}  // namespace

TratadorTecladoMouse::TratadorTecladoMouse(ntf::CentralNotificacoes* central, ent::Tabuleiro* tabuleiro)
    : central_(central), tabuleiro_(tabuleiro) {
  central_->RegistraReceptor(this);
  temporizador_mouse_ = 0;
  temporizador_teclado_ = 0;
  MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
}


TratadorTecladoMouse::~TratadorTecladoMouse() {
  central_->DesregistraReceptor(this);
}

void TratadorTecladoMouse::TrataAcaoTemporizadaTeclado() {
  // Busca primeira tecla.
  if (teclas_.empty()) {
    LOG(ERROR) << "Temporizador sem teclas";
    return;
  }
  int primeira_tecla = *teclas_.begin();
  switch (primeira_tecla) {
    case Tecla_A: {
      if (teclas_.size() < 2) {
        return;
      }
      if (teclas_[1] == Tecla_Delete) {
        tabuleiro_->LimpaListaPontosVida();
      } else if (teclas_[1] == Tecla_Backspace) {
        tabuleiro_->LimpaUltimoListaPontosVida();
      } else if (teclas_[1] == Tecla_D || teclas_[1] == Tecla_C) {
        if (teclas_.size() < 2) {
          break;
        }
        auto lista_dano = CalculaDano(teclas_.begin() + 2, teclas_.end());
        if (teclas_[1] == Tecla_D) {
          // Inverte o dano.
          for (int& pv : lista_dano) {
            pv = -pv;
          }
        }
        tabuleiro_->AcumulaPontosVida(lista_dano);
      } else if (teclas_[1] == Tecla_E) {
        if (teclas_.size() < 2) {
          break;
        }
        auto rodadas = CalculaDano(teclas_.begin() + 2, teclas_.end());
        for (const auto& r : rodadas) {
          tabuleiro_->AdicionaEventoEntidadesSelecionadasNotificando(r);
        }
      }
    }
    break;
    case Tecla_C:
    case Tecla_D: {
      auto lista_pv = CalculaDano(teclas_.begin(), teclas_.end());
      if (lista_pv.size() != 1) {
        break;
      }
      if (primeira_tecla == Tecla_D) {
        lista_pv[0] = -lista_pv[0];
      }
      tabuleiro_->TrataAcaoAtualizarPontosVidaEntidades(lista_pv[0]);
    }
    break;
    case Tecla_R: {
      if (teclas_.size() != 2) {
        LOG(ERROR) << "Resistencia requer uma tecla, recebi: " << (teclas_.size() - 1);
      }
      ent::ResultadoSalvacao rs = ent::RS_FALHOU;
      if (teclas_[1] == Tecla_2) {
        rs = ent::RS_MEIO;
      } else if (teclas_[1] == Tecla_2) {
        rs = ent::RS_MEIO;
      } else if (teclas_[1] == Tecla_4) {
        rs = ent::RS_QUARTO;
      } else if (teclas_[1] == Tecla_0) {
        rs = ent::RS_ANULOU;
      }
      tabuleiro_->AtualizaSalvacaoEntidadesSelecionadas(rs);
    }
    break;
    default:
      VLOG(1) << "Tecla de temporizador nao reconhecida: " << primeira_tecla;
  }
}

void TratadorTecladoMouse::TrataTeclaPressionada(teclas_e tecla, modificadores_e modificadores) {
  VLOG(1) << "Tecla: " << (void*)tecla << ", mod: " << (void*)modificadores;
  if (estado_ == ESTADO_TEMPORIZANDO_TECLADO) {
    switch (tecla) {
      case Tecla_Esc:
        break;
      case Tecla_EnterKeypad:
      case Tecla_EnterNormal:
        // Finaliza temporizacao.
        TrataAcaoTemporizadaTeclado();
        break;
      case Tecla_Backspace:
      case Tecla_Delete:
        // Finaliza temporizacao.
        teclas_.push_back(tecla);
        TrataAcaoTemporizadaTeclado();
        break;
      default:
        // Nao muda estado mas reinicia o timer.
        teclas_.push_back(tecla);
        temporizador_teclado_ = MAX_TEMPORIZADOR_TECLADO;
        return;
    }
    // Ao terminar, volta pro mouse.
    MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
    return;
  }
  switch (tecla) {
    case Tecla_Backspace:
    case Tecla_Delete:
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE));
      return;
    case Tecla_AltEsquerdo:
      tabuleiro_->DetalharTodasEntidades(true);
      return;
    case Tecla_Cima: {
      // Nao pode usar == pq a seta tambem aplica modificador de keypad.
      float incremento = ((modificadores & Modificador_Shift) != 0) ? 0.1f : 1.0f;
      if ((modificadores & Modificador_Ctrl) != 0) {
        tabuleiro_->TrataTranslacaoZEntidadesSelecionadas(incremento);
      } else {
        tabuleiro_->TrataMovimentoEntidadesSelecionadas(true, incremento);
      }
      return;
    }
    case Tecla_Baixo: {
      float incremento = ((modificadores & Modificador_Shift) != 0) ? -0.1f : -1.0f;
      if ((modificadores & Modificador_Ctrl) != 0) {
        tabuleiro_->TrataTranslacaoZEntidadesSelecionadas(incremento);
      } else {
        tabuleiro_->TrataMovimentoEntidadesSelecionadas(true, incremento);
      }
      return;
    }
    case Tecla_Esquerda: {
      float incremento = ((modificadores & Modificador_Shift) != 0) ? -0.1f : -1.0f;
      tabuleiro_->TrataMovimentoEntidadesSelecionadas(false, incremento);
      return;
    }
    case Tecla_Direita: {
      float incremento = ((modificadores & Modificador_Shift) != 0) ? 0.1f : 1.0f;
      tabuleiro_->TrataMovimentoEntidadesSelecionadas(false, incremento);
      return;
    }
    case Tecla_F:
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_FIXA);
      return;
    case Tecla_F1:
      tabuleiro_->AlternaCameraPresa();
      return;
    case Tecla_F2:
      tabuleiro_->AlternaCameraIsometrica();
      return;
    case Tecla_F4:
      tabuleiro_->AlternaVisaoJogador();
      return;
    case Tecla_G:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->AgrupaEntidadesSelecionadas();
      } else if (modificadores == (Modificador_Ctrl | Modificador_Shift)) {
        tabuleiro_->DesagrupaEntidadesSelecionadas();
      }
      return;
    case Tecla_V:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->ColaEntidadesSelecionadas();
      } else if (modificadores == Modificador_Alt) {
        tabuleiro_->AtualizaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VISIBILIDADE, true);
      } else if (modificadores == Modificador_Shift) {
        tabuleiro_->AtualizaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VISIBILIDADE, false);
      } else {
        tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VISIBILIDADE);
      }
      return;
    case Tecla_I:
      tabuleiro_->TrataBotaoAlternarIluminacaoMestre();
      return;
    case Tecla_L:
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ILUMINACAO);
      return;
    case Tecla_Y:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->TrataComandoRefazer();
        return;
      }
      return;
    case Tecla_Z:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->TrataComandoDesfazer();
        return;
      }
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VOO);
      return;
    case Tecla_Q:
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_CAIDA);
      return;
    case Tecla_A:
      if (modificadores == (Modificador_Ctrl | Modificador_Shift)) {
        tabuleiro_->SelecionaTudo(true  /*fixas*/);
        return;
      }
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->SelecionaTudo(false  /*fixas*/);
        return;
      }
      MudaEstado(ESTADO_TEMPORIZANDO_TECLADO);
      teclas_.push_back(tecla);
      return;
    case Tecla_C:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->CopiaEntidadesSelecionadas();
      } else {
        MudaEstado(ESTADO_TEMPORIZANDO_TECLADO);
        teclas_.push_back(tecla);
        return;
      }
      break;
    case Tecla_D:
      if (modificadores == (Modificador_Ctrl | Modificador_Alt)) {
        tabuleiro_->AlternaModoDebug();
        return;
      }
      // Entra em modo de temporizacao.
      MudaEstado(ESTADO_TEMPORIZANDO_TECLADO);
      teclas_.push_back(tecla);
      return;
    case Tecla_M: {
      //tabuleiro_->AlternaModoMestre();
      //central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_REMOVER_CENARIO));
      return;
    }
    case Tecla_P: {
      auto* n = ntf::NovaNotificacao(ntf::TN_PASSAR_UMA_RODADA);
      central_->AdicionaNotificacao(n);
      return;
    }
    case Tecla_S:
      if ((modificadores & Modificador_Ctrl) != 0) {
        central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO));
        return;
      }
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_SELECIONAVEL);
      return;
    case Tecla_R:
      MudaEstado(ESTADO_TEMPORIZANDO_TECLADO);
      teclas_.push_back(tecla);
      return;
    case Tecla_Tab:
      if (modificadores == Modificador_Shift) {
        // No android, shift tab eh tab.
        tabuleiro_->AcaoAnterior();
      } else {
        tabuleiro_->ProximaAcao();
      }
      return;
    case Tecla_TabInvertido:
      // No qt, shift tab vira uma tecla.
      tabuleiro_->AcaoAnterior();
      return;
    default:
      LOG(INFO) << "Tecla nao reconhecida: " << (void*)tecla;
  }
}

void TratadorTecladoMouse::TrataTeclaLiberada(teclas_e tecla, modificadores_e modificadores) {
  if (tecla == Tecla_AltEsquerdo) {
    tabuleiro_->DetalharTodasEntidades(false);
  }
}

void TratadorTecladoMouse::TrataBotaoMousePressionado(
    botoesmouse_e botao, unsigned int modificadores, int x, int y) {
  LOGT(1) << "Pressionado: " << x << ", " << y;
  MudaEstado(ESTADO_OUTRO);
  if (modificadores == Modificador_Alt) {
    // Acao padrao eh usada quando o botao eh o direito.
    tabuleiro_->TrataBotaoAcaoPressionado(botao == Botao_Direito, x, y);
  } else if (modificadores == Modificador_Ctrl) {
    if (botao == Botao_Esquerdo) {
      tabuleiro_->TrataBotaoAlternarSelecaoEntidadePressionado(x, y);
    } else if (botao == Botao_Direito) {
      tabuleiro_->TrataBotaoDesenhoPressionado(x, y);
    }
  } else {
    switch (botao) {
      case Botao_Esquerdo:
        if (modificadores == Modificador_Shift) {
          // Mac nao tem botao do meio, entao usa o shift para simular.
          tabuleiro_->TrataBotaoRotacaoPressionado(x, y);
        } else {
          tabuleiro_->TrataBotaoEsquerdoPressionado(x, y);
        }
        break;
      case Botao_Direito:
        tabuleiro_->TrataBotaoDireitoPressionado(x, y);
        break;
      case Botao_Meio:
        tabuleiro_->TrataBotaoRotacaoPressionado(x, y);
        break;
      default:
        ;
    }
  }
}

void TratadorTecladoMouse::TrataMovimentoMouse(int x, int y) {
  LOGT(1) << "Movimento: " << x << ", " << y;
  ultimo_x_ = x;
  ultimo_y_ = y;
  if (estado_ == ESTADO_TEMPORIZANDO_MOUSE) {
    temporizador_mouse_ = MAX_TEMPORIZADOR_MOUSE;
    tabuleiro_->TrataMovimentoMouse();
    return;
  }
  temporizador_mouse_ = MAX_TEMPORIZADOR_MOUSE;
  tabuleiro_->TrataMovimentoMouse(x, y);
}

void TratadorTecladoMouse::TrataRodela(int delta) {
  LOGT(1) << "Rodela: " << delta;
  tabuleiro_->TrataEscalaPorDelta(delta);
}

void TratadorTecladoMouse::TrataPincaEscala(float fator) {
  LOGT(1) << "Pinca: " << fator;
  tabuleiro_->TrataEscalaPorFator(fator);
}

void TratadorTecladoMouse::MudaEstado(estado_e novo_estado) {
  if (novo_estado == ESTADO_TEMPORIZANDO_MOUSE) {
    temporizador_mouse_ = MAX_TEMPORIZADOR_MOUSE;
  } else if (novo_estado == ESTADO_TEMPORIZANDO_TECLADO) {
    teclas_.clear();
    temporizador_teclado_ = MAX_TEMPORIZADOR_TECLADO;
  }
  VLOG(2) << "Mudando para estado: " << novo_estado;
  estado_ = novo_estado;
}

void TratadorTecladoMouse::TrataAcaoTemporizadaMouse() {
  VLOG(1) << "Tratando acao temporizada de mouse em: " << ultimo_x_ << ", " << ultimo_y_;
  tabuleiro_->TrataMouseParadoEm(ultimo_x_, ultimo_y_);
}

bool TratadorTecladoMouse::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_TEMPORIZADOR:
      if (estado_ == ESTADO_TEMPORIZANDO_MOUSE) {
        if (--temporizador_mouse_ == 0) {
          TrataAcaoTemporizadaMouse();
        }
        break;
      } else if (estado_ == ESTADO_TEMPORIZANDO_TECLADO) {
        if (--temporizador_teclado_ == 0) {
          TrataAcaoTemporizadaTeclado();
          MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
        }
        break;
      }
      break;
    default:
      return false;
  }
  return true;
}

void TratadorTecladoMouse::TrataDuploCliqueMouse(botoesmouse_e botao, unsigned int modificadores, int x, int y) {
  LOGT(1) << "Duplo clique: " << x << ", " << y;
  if (botao == Botao_Esquerdo) {
    tabuleiro_->TrataDuploCliqueEsquerdo(x, y);
  } else if (botao == Botao_Direito) {
    tabuleiro_->TrataDuploCliqueDireito(x, y);
  }
}

void TratadorTecladoMouse::TrataBotaoMouseLiberado() {
  LOGT(1) << "Liberado";
  MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
  tabuleiro_->TrataBotaoLiberado();
}

}  // namespace ifg
