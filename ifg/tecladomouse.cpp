#include <cstdlib>
#include "ifg/tecladomouse.h"
#include "ent/acoes.h"
#include "ent/constantes.h"
#include "ent/tabuleiro.h"
#include "ent/util.h"
#include "log/log.h"

namespace ifg {

namespace {

// Temporizadores * 10ms.
const float MAX_TEMPORIZADOR_TECLADO_S = 3;
const float MAX_TEMPORIZADOR_MOUSE_S = 1;

const float MAX_TEMPORIZADOR_TECLADO = MAX_TEMPORIZADOR_TECLADO_S * ATUALIZACOES_POR_SEGUNDO;
const float MAX_TEMPORIZADOR_MOUSE = MAX_TEMPORIZADOR_MOUSE_S * ATUALIZACOES_POR_SEGUNDO;

std::string CalculaDanoSimples(const std::vector<teclas_e>::const_iterator& inicio_teclas_normal,
                               const std::vector<teclas_e>::const_iterator& fim_teclas_normal) {
  std::string s;
  for (auto it = inicio_teclas_normal; it < fim_teclas_normal; ++it) {
    if (*it >= Tecla_0 && *it <= Tecla_9) {
      s.push_back('0' + *it - Tecla_0);
      continue;
    }
    if (*it == Tecla_D) {
      s.push_back('d');
      continue;
    }
    // Igual com shift eh mais.
    if (*it == Tecla_Mais || *it == Tecla_Igual) {
      s.push_back('+');
      continue;
    }
    if (*it == Tecla_Menos) {
      s.push_back('-');
      continue;
    }
  }
  return s;
}

// Calcula o dano acumulado no vetor de teclas. Sempre retorna o valor como positivo.
const std::vector<std::pair<int, std::string>> CalculaDano(const std::vector<teclas_e>::const_iterator& inicio_teclas,
                                                           const std::vector<teclas_e>::const_iterator& fim_teclas) {
  std::vector<std::pair<int, std::string>> result;
  auto it_inicio = inicio_teclas;
  for (auto it = inicio_teclas; it < fim_teclas; ++it) {
    if (*it == Tecla_Espaco) {
      result.push_back(std::make_pair(1, CalculaDanoSimples(it_inicio, it)));
      it_inicio = it + 1;  // pula o espaco.
    }
  }
  result.push_back(std::make_pair(1, CalculaDanoSimples(it_inicio, fim_teclas)));
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
        std::vector<std::pair<int, std::string>> lista_dano = CalculaDano(teclas_.begin() + 2, teclas_.end());
        if (teclas_[1] == Tecla_D) {
          // Inverte o dano.
          for (auto& dano : lista_dano) {
            dano.first *= -1;
          }
        }
        tabuleiro_->AcumulaPontosVida(lista_dano);
      } else if (teclas_[1] == Tecla_R) {
        for (unsigned int i = 2; i < teclas_.size(); ++i) {
          if (teclas_[i] == Tecla_Delete || teclas_[i] == Tecla_Backspace) {
            ent::LimpaDadosAcumulados();
            return;
          }
        }
        std::vector<std::pair<int, std::string>> lista_dano = CalculaDano(teclas_.begin() + 2, teclas_.end());
        if (lista_dano.empty()) {
          LOG(INFO) << "d20 forcado sem lista de dano: ";
          break;
        }
        for (auto [mais_menos, texto] : lista_dano) {
          int valor = atoi(texto.c_str());
          if (valor >= 1 && valor <= 100) {
            VLOG(1) << "Lido: " << texto << ", mais_menos: " << mais_menos;
            ent::AcumulaDado(valor);
          } else {
            LOG(INFO) << "d20 forcado invalido: " << texto << ", mais_menos: " << mais_menos;
          }
        }
      }
    }
    break;
    case Tecla_C:
    case Tecla_D: {
      std::vector<std::pair<int, std::string>> lista_pv = CalculaDano(teclas_.begin() + 1, teclas_.end());
      if (lista_pv.size() != 1) {
        break;
      }
      int res;
      std::vector<std::pair<int, int>> dados;
      std::tie(res, dados) = ent::GeraPontosVida(lista_pv[0].second);
      if (primeira_tecla == Tecla_D) {
        res *= -1;
      }
      tabuleiro_->TrataAcaoAtualizarPontosVidaEntidades(res);
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
  //if (estado_ == ESTADO_MOSTRANDO_DIALOGO) {
  //  return;
  //}
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
        temporizador_teclado_ = static_cast<int>(MAX_TEMPORIZADOR_TECLADO);
        return;
    }
    // Ao terminar, volta pro mouse.
    MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
    return;
  }
  switch (tecla) {
    case Tecla_Insert:
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_ADICAO_ENTIDADE);
    break;
    case Tecla_Backspace:
    case Tecla_Delete: {
      auto n = ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE);
      tabuleiro_->TrataNotificacao(*n);
      return;
    }
    case Tecla_AltEsquerdo:
      tabuleiro_->DetalharTodasEntidades(true);
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_ACAO);
      return;
    case Tecla_Ctrl:
      //tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_DESENHO);
      return;
    case Tecla_Shift:
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_ROTACAO);
      estado_ = ESTADO_OUTRO;
      return;
    case Tecla_Cima: {
      // Nao pode usar == pq a seta tambem aplica modificador de keypad.
      float incremento = ((modificadores & Modificador_Shift) != 0) ? 0.1f : 1.0f;
      if ((modificadores & Modificador_Ctrl) != 0) {
        tabuleiro_->TrataTranslacaoZ(incremento);
      } else {
        tabuleiro_->TrataMovimentoEntidadesSelecionadasOuCamera(true, incremento);
      }
      return;
    }
    case Tecla_Baixo: {
      float incremento = ((modificadores & Modificador_Shift) != 0) ? -0.1f : -1.0f;
      if ((modificadores & Modificador_Ctrl) != 0) {
        tabuleiro_->TrataTranslacaoZ(incremento);
      } else {
        tabuleiro_->TrataMovimentoEntidadesSelecionadasOuCamera(true, incremento);
      }
      return;
    }
    case Tecla_Esquerda: {
      if (((modificadores & Modificador_Ctrl) != 0)) {
        tabuleiro_->TrataEspiada(-1);
        return;
      }
      float incremento = ((modificadores & Modificador_Shift) != 0) ? -0.1f : -1.0f;
      tabuleiro_->TrataMovimentoEntidadesSelecionadasOuCamera(false, incremento);
      return;
    }
    case Tecla_Direita: {
      if (((modificadores & Modificador_Ctrl) != 0)) {
        tabuleiro_->TrataEspiada(1);
        return;
      }
      float incremento = ((modificadores & Modificador_Shift) != 0) ? 0.1f : 1.0f;
      tabuleiro_->TrataMovimentoEntidadesSelecionadasOuCamera(false, incremento);
      return;
    }
    case Tecla_F:
      tabuleiro_->AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_FIXA);
      return;
    case Tecla_F1:
      tabuleiro_->AlternaCameraPrimeiraPessoa();
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
    case Tecla_H:
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->Hack();
      }
      return;
    case Tecla_V:
      if ((modificadores & Modificador_Ctrl) != 0) {
        tabuleiro_->ColaEntidadesSelecionadas((modificadores & Modificador_Alt) != 0);
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
      if ((modificadores & Modificador_Ctrl) != 0 && (modificadores & Modificador_Shift) != 0) {
        tabuleiro_->AlternaMostraLogEventos();
        return;
      }
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
      if (modificadores == Modificador_Ctrl) {
        tabuleiro_->AlternaModoDebug();
        return;
      }
      if (modificadores == Modificador_Alt) {
        central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_HACK_ANDROID));
        return;
      }
      // Entra em modo de temporizacao.
      MudaEstado(ESTADO_TEMPORIZANDO_TECLADO);
      teclas_.push_back(tecla);
      return;
    case Tecla_J: {
      tabuleiro_->AlternaListaJogadores();
      return;
    }
    case Tecla_M: {
      //tabuleiro_->AlternaModoMestreSecundario();
      //tabuleiro_->AlternaModoMestre();
      //central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_REMOVER_CENARIO));
      return;
    }
    case Tecla_N: {
      tabuleiro_->LimpaIniciativasNotificando();
      return;
    }
    case Tecla_O: {
      tabuleiro_->AlternaListaObjetos();
      return;
    }
    case Tecla_P: {
      tabuleiro_->AlternaCameraPrimeiraPessoa();
      //auto* n = ntf::NovaNotificacao(ntf::TN_PASSAR_UMA_RODADA);
      //central_->AdicionaNotificacao(n);
      return;
    }
    case Tecla_S:
      if ((modificadores & Modificador_Ctrl) != 0) {
        bool versionar = (modificadores & Modificador_Shift) != 0;
        auto n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO_SE_NECESSARIO_OU_SALVAR_DIRETO);
        if (versionar) {
          n->mutable_tabuleiro()->add_versoes();
        }
        central_->AdicionaNotificacao(std::move(n));
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
  switch (tecla) {
    case Tecla_AltEsquerdo:
      tabuleiro_->DetalharTodasEntidades(false);
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_NORMAL);
      return;
    //case Tecla_Ctrl:
    case Tecla_Shift:
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_NORMAL);
      estado_ = ESTADO_TEMPORIZANDO_MOUSE;
      return;
    default:
      return;
  }
}

void TratadorTecladoMouse::TrataBotaoMousePressionado(botoesmouse_e botao, unsigned int modificadores, int x, int y) {
  MudaEstado(ESTADO_OUTRO);
  if (modificadores == Modificador_Alt) {
    VLOG(1) << "Pressionado e: " << (botao==Botao_Esquerdo) << " com alt, pos " << x << ", " << y;
    // Acao padrao eh usada quando o botao eh o direito.
    tabuleiro_->TrataBotaoAcaoPressionado(botao == Botao_Direito, x, y);
  } else if (modificadores == Modificador_Ctrl || modificadores == (Modificador_Ctrl | Modificador_AltGr)) {
    VLOG(1) << "Pressionado e: " << (botao==Botao_Esquerdo) << " com ctrl, pos " << x << ", " << y << ", altgr: " << ((modificadores & Modificador_AltGr) != 0);
    if (botao == Botao_Esquerdo) {
      tabuleiro_->TrataBotaoAlternarSelecaoEntidadePressionado(x, y, /*forca_selecao=*/(modificadores & Modificador_AltGr) != 0);
    } else if (botao == Botao_Direito) {
      tabuleiro_->TrataBotaoDesenhoPressionado(x, y);
    }
  } else {
    VLOG(1) << "Pressionado e: " << (botao==Botao_Esquerdo) << ", pos " << x << ", " << y;
    switch (botao) {
      case Botao_Esquerdo:
        if (modificadores == Modificador_Shift) {
          tabuleiro_->TrataBotaoRotacaoPressionado(x, y);
        } else {
          tabuleiro_->TrataBotaoEsquerdoPressionado(x, y, /*alterna_selecao=*/false, /*forca_selecao=*/(modificadores & Modificador_AltGr) != 0);
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

bool TratadorTecladoMouse::TrataMovimentoMouse(int x, int y) {
  ultimo_x_ = x;
  ultimo_y_ = y;
  VLOG(2) << "Movimento: " << x << ", " << y << ", ultimo_x " << ultimo_x_ << ", ultimo_y: " << ultimo_y_;
  if (estado_ == ESTADO_TEMPORIZANDO_MOUSE) {
    temporizador_mouse_ = MAX_TEMPORIZADOR_MOUSE;
    tabuleiro_->TrataMovimentoMouse();
    return false;
  }
  temporizador_mouse_ = MAX_TEMPORIZADOR_MOUSE;
  return tabuleiro_->TrataMovimentoMouse(x, y);
}

void TratadorTecladoMouse::TrataRodela(int delta) {
  VLOG(1) << "Rodela: " << delta;
  tabuleiro_->TrataEscalaPorDelta(delta);
}

void TratadorTecladoMouse::TrataPincaEscala(float fator) {
  VLOG(1) << "Pinca: " << fator;
  tabuleiro_->TrataEscalaPorFator(fator);
}

void TratadorTecladoMouse::TrataInicioPinca(int x1, int y1, int x2, int y2) {
  VLOG(1) << "Inicio Pinca: " << x1 << " " << y1 << "; " << x2 << " " << y2;
  tabuleiro_->TrataInicioPinca(x1, y1, x2, y2);
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
#if USAR_QT
  // No QT, o picking tem que ser feito dentro de contexto.
  auto n = ntf::NovaNotificacao(ntf::TN_TEMPORIZADOR_MOUSE);
  n->mutable_pos()->set_x(ultimo_x_);
  n->mutable_pos()->set_y(ultimo_y_);
  central_->AdicionaNotificacao(n.release());
#else
  tabuleiro_->TrataMouseParadoEm(ultimo_x_, ultimo_y_);
#endif
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
  VLOG(1) << "Duplo clique: " << x << ", " << y;
  if (botao == Botao_Esquerdo) {
    tabuleiro_->TrataDuploCliqueEsquerdo(x, y);
  } else if (botao == Botao_Direito) {
    tabuleiro_->TrataDuploCliqueDireito(x, y);
  }
}

void TratadorTecladoMouse::TrataBotaoMouseLiberado() {
  VLOG(1) << "Liberado";
  MudaEstado(ESTADO_TEMPORIZANDO_MOUSE);
  tabuleiro_->TrataBotaoLiberado();
}

}  // namespace ifg
