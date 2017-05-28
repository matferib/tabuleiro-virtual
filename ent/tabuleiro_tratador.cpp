// Este arquivo contem as rotinas responsaveis por tratar eventos de
// teclado e mouse do tabuleiro.

#include <algorithm>
#include <boost/filesystem.hpp>
#include <tuple>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

//#define VLOG_NIVEL 1
#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/controle_virtual.pb.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/tabuleiro_interface.h"
#include "ent/tabuleiro_terreno.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "matrix/vectors.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace ent {

namespace {

// Retorna 0 se nao andou quadrado, 1 se andou no eixo x, 2 se andou no eixo y, 3 se andou em ambos.
int AndouQuadrado(const Posicao& p1, const Posicao& p2) {
  float dx = fabs(p1.x() - p2.x());
  float dy = fabs(p1.y() - p2.y());
  if (dx >= TAMANHO_LADO_QUADRADO && dy >= TAMANHO_LADO_QUADRADO) return 3;
  if (dx >= TAMANHO_LADO_QUADRADO) return 1;
  if (dy >= TAMANHO_LADO_QUADRADO) return 2;
  return 0;
}

// Retorna o quadrado da distancia horizontal entre pos1 e pos2.
float DistanciaHorizontalQuadrado(const Posicao& pos1, const Posicao& pos2) {
  float distancia = powf(pos1.x() - pos2.x(), 2) + powf(pos1.y() - pos2.y(), 2);
  VLOG(4) << "Distancia: " << distancia;
  return distancia;
}

/** Retorna true se o ponto (x,y) estiver dentro do quadrado qx1, qy1, qx2, qy2. */
bool PontoDentroQuadrado(float x, float y, float qx1, float qy1, float qx2, float qy2) {
  float xesq = qx1;
  float xdir = qx2;
  if (xesq > xdir) {
    std::swap(xesq, xdir);
  }
  float ybaixo = qy1;
  float yalto = qy2;
  if (ybaixo > yalto) {
    std::swap(ybaixo, yalto);
  }
  if (x < xesq || x > xdir) {
    VLOG(2) << "xesq: " << xesq << ", x: " << x << ", xdir: " << xdir;
    return false;
  }
  if (y < ybaixo || y > yalto) {
    VLOG(2) << "ybaixo: " << ybaixo << ", y: " << y << ", yalto: " << yalto;
    return false;
  }
  return true;
}

void PreencheNotificacaoDerrubaOrigem(
    const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  entidade_depois->set_caida(true);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    *n_desfazer->mutable_entidade() = *entidade_depois;
    auto* e_antes = n_desfazer->mutable_entidade_antes();
    e_antes->set_id(entidade.Id());
    e_antes->set_caida(entidade.Proto().caida());
  }
}

}  // namespace

void Tabuleiro::TrataTeclaPressionada(int tecla) {
  return;
#if 0
  switch (tecla) {
    default:
      ;
  }
#endif
}

void Tabuleiro::TrataBotaoD20PressionadoPosPicking(float x3d, float y3d, float z3d) {
  char texto[31];
  snprintf(texto, 30, "d20 = %d", RolaDado(20));
  auto* n = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  auto* a = n->mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_local_apenas(false);
  auto* pd = a->mutable_pos_entidade();
  pd->set_x(x3d);
  pd->set_y(y3d);
  pd->set_z(z3d);
  pd->set_id_cenario(cenario_corrente_);
  a->set_texto(texto);
  central_->AdicionaNotificacao(n);
}

void Tabuleiro::TrataEscalaPorDelta(int delta) {
  bool atualizar_mapa_luzes = false;
  if (estado_ == ETAB_ENTS_PRESSIONADAS || estado_ == ETAB_ENTS_ESCALA) {
    if (estado_ == ETAB_ENTS_PRESSIONADAS) {
      FinalizaEstadoCorrente();
      estado_ = ETAB_ENTS_ESCALA;
    }
    ntf::Notificacao grupo_notificacoes;
    grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
      n->mutable_entidade_antes()->CopyFrom(entidade->Proto());
      auto* e = n->mutable_entidade();
      e->CopyFrom(entidade->Proto());
      float fator = 1.0f + delta * SENSIBILIDADE_RODA * 0.1f;
      e->mutable_escala()->set_x(e->escala().x() * fator);
      e->mutable_escala()->set_y(e->escala().y() * fator);
      e->mutable_escala()->set_z(e->escala().z() * fator);
      if (entidade->Tipo() != TE_ENTIDADE) {
        atualizar_mapa_luzes = true;
      }
    }
    TrataNotificacao(grupo_notificacoes);
    // Para desfazer.
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  } else if (estado_ == ETAB_QUAD_SELECIONADO && ModoClique() == MODO_TERRENO) {
    TrataDeltaTerreno(delta > 0 ? TAMANHO_LADO_QUADRADO : -TAMANHO_LADO_QUADRADO);
    atualizar_mapa_luzes = true;
  } else {
    if (camera_ == CAMERA_ISOMETRICA) {
      TrataInclinacaoPorDelta(-delta * SENSIBILIDADE_RODA);
    } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
      AlteraAnguloVisao(angulo_visao_vertical_graus_ - (delta * SENSIBILIDADE_RODA * 2.0f));
    } else {
      // move o olho no eixo Z de acordo com o eixo Y do movimento
      AtualizaRaioOlho(olho_.raio() - (delta * SENSIBILIDADE_RODA));
    }
  }
  if (atualizar_mapa_luzes) {
    luzes_pontuais_.clear();
  }
}

void Tabuleiro::TrataInicioPinca(int x1, int y1, int x2, int y2) {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return;
  }
  VLOG(1) << "Pinca iniciada";
  parametros_desenho_.set_desenha_terreno(false);
  parametros_desenho_.set_nao_desenha_entidades_fixas(true);
  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x1, y1, &id, &tipo_objeto, &profundidade);
  unsigned int id2, tipo_objeto2;
  BuscaHitMaisProximo(x2, y2, &id2, &tipo_objeto2, &profundidade);
  if (tipo_objeto == OBJ_ENTIDADE && tipo_objeto == tipo_objeto2 && id == id2) {
    const auto* e = BuscaEntidade(id);
    if (e != nullptr && (e->SelecionavelParaJogador() || EmModoMestreIncluindoSecundario())) {
      SelecionaEntidade(id);
      estado_anterior_ = estado_;
      estado_ = ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA;
      EntidadeProto proto;
      proto.set_id(id);
      proto.set_rotacao_z_graus(e->RotacaoZGraus());
      *proto.mutable_escala() = e->Proto().escala();
      *proto.mutable_pos() = e->Pos();
      translacoes_rotacoes_escalas_antes_.clear();
      translacoes_rotacoes_escalas_antes_[id].Swap(&proto);
      ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_MOVIMENTOS_PARCIAIS;
    }
  }
}

void Tabuleiro::TrataEscalaPorFator(float fator) {
  bool atualizar_mapa_luzes = false;
  if (estado_ == ETAB_QUAD_SELECIONADO && ModoClique() == MODO_TERRENO) {
    // Eh possivel chegar aqui?
    TrataDeltaTerreno(fator * TAMANHO_LADO_QUADRADO);
  } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    fator = 1.0f / fator;
    AlteraAnguloVisao(angulo_visao_vertical_graus_ * fator);
    return;
  } else if (estado_ == ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA) {
    VLOG(1) << "escalando entidade";
    if (translacoes_rotacoes_escalas_antes_.empty()) {
      LOG(ERROR) << "translacoes_rotacoes_escalas_antes_ vazio";
      return;
    }
    auto* entidade = BuscaEntidade(translacoes_rotacoes_escalas_antes_.begin()->first);
    if (entidade != nullptr) {
      EntidadeProto proto;
      proto.mutable_escala()->set_x(entidade->Proto().escala().x() * fator);
      proto.mutable_escala()->set_y(entidade->Proto().escala().y() * fator);
      proto.mutable_escala()->set_z(entidade->Proto().escala().z() * fator);
      entidade->AtualizaParcial(proto);
      if (entidade->Tipo() != TE_ENTIDADE) {
        atualizar_mapa_luzes = true;
      }
    } else {
      LOG(WARNING) << "Nao eh possivel escalar, entidade nullptr";
    }
  } else {
    AtualizaRaioOlho(olho_.raio() / fator);
  }
  if (atualizar_mapa_luzes) {
    luzes_pontuais_.clear();
  }
}

void Tabuleiro::TrataRotacaoPorDelta(float delta_rad) {
  bool atualizar_mapa_luzes = false;
  if (camera_ != CAMERA_PRIMEIRA_PESSOA && estado_ == ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA) {
    if (translacoes_rotacoes_escalas_antes_.empty()) {
      return;
    }
    auto* entidade = BuscaEntidade(translacoes_rotacoes_escalas_antes_.begin()->first);
    if (entidade != nullptr) {
      if (entidade->Tipo() != TE_ENTIDADE) {
        atualizar_mapa_luzes = true;
      }
      entidade->IncrementaRotacaoZGraus(-delta_rad * RAD_PARA_GRAUS);
      return;
    }
  }
  // Realiza a rotacao da tela.
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    delta_rad = -delta_rad;
  }
  float olho_rotacao = olho_.rotacao_rad() + delta_rad;
  if (olho_rotacao >= 2 * M_PI) {
    olho_rotacao -= 2 * M_PI;
  } else if (olho_rotacao <= - 2 * M_PI) {
    olho_rotacao += 2 * M_PI;
  }
  olho_.set_rotacao_rad(olho_rotacao);
  AtualizaOlho(0, true  /*forcar*/);
  if (atualizar_mapa_luzes) {
    luzes_pontuais_.clear();
  }
}

void Tabuleiro::TrataInclinacaoPorDelta(float delta) {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    delta = -delta;
  }
  float olho_altura = olho_.altura() + delta;
  if (olho_altura > OLHO_ALTURA_MAXIMA) {
    olho_altura = OLHO_ALTURA_MAXIMA;
  } else if (olho_altura < OLHO_ALTURA_MINIMA) {
    olho_altura = OLHO_ALTURA_MINIMA;
  }
  olho_.set_altura(olho_altura);
  AtualizaOlho(0, true  /*forcar*/);
}

void Tabuleiro::TrataTranslacaoPorDelta(int x, int y, int nx, int ny) {
  // Faz picking do tabuleiro sem entidades.
  parametros_desenho_.set_desenha_entidades(false);
  float x0, y0, z0;
  if (!MousePara3dParaleloZero(x, y, &x0, &y0, &z0)) {
    return;
  }

  float x1, y1, z1;
  if (!MousePara3dParaleloZero(nx, ny, &x1, &y1, &z1)) {
    return;
  }

  float delta_x = x1 - x0;
  float delta_y = y1 - y0;
  auto* p = olho_.mutable_alvo();
  p->set_x(p->x() - delta_x);
  p->set_y(p->y() - delta_y);
  olho_.clear_destino();
  AtualizaOlho(0, true);
}

void Tabuleiro::TrataMovimentoMouse() {
  if (temporizador_detalhamento_ms_ <= 0) {
    id_entidade_detalhada_ = Entidade::IdInvalido;
    tipo_entidade_detalhada_ = OBJ_INVALIDO;
  }
}

bool Tabuleiro::TrataMovimentoMouse(int x, int y) {
  if (modo_clique_ == MODO_ROTACAO && estado_ != ETAB_ROTACAO) {
    TrataBotaoRotacaoPressionado(x, y);
    // Aqui ainda retorna false, para nao voltar o cursor para a posicao anterior. A partir daqui,
    // as rotacoes retornarao false.
    return false;
  }
  if (x == ultimo_x_ && y == ultimo_y_) {
    // No tablet pode acontecer de gerar estes eventos com mesma coordenadas.
    return false;
  }
  switch (estado_) {
    case ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA: {
      // So trata translacao aqui, rotacao e escala sao tratadas em outros lugares.
      if (translacoes_rotacoes_escalas_antes_.empty()) {
        break;
      }
      const auto& id_proto = *translacoes_rotacoes_escalas_antes_.begin();
      auto* entidade = BuscaEntidade(id_proto.first);
      if (entidade == nullptr) {
        break;
      }
      gl::MatrizEscopo salva_matriz(gl::MATRIZ_MODELAGEM_CAMERA);
      gl::CarregaIdentidade();
      ConfiguraOlhar();
      // Faz picking do tabuleiro sem entidades.
      float nx, ny, nz;
      if (!MousePara3dParaleloZero(x, y, &nx, &ny, &nz)) {
        return false;
      }
      float dx = nx - ultimo_x_3d_;
      float dy = ny - ultimo_y_3d_;
      entidade->MoveDelta(dx, dy, 0.0f);
      ultimo_x_ = x;
      ultimo_y_ = y;
      ultimo_x_3d_ = nx;
      ultimo_y_3d_ = ny;
    }
    break;
    case ETAB_ENTS_TRANSLACAO_ROTACAO: {
      bool atualizar_mapa_luzes = false;
      if (translacao_rotacao_ == TR_NENHUM) {
        int abs_delta_x = abs(primeiro_x_ - x);
        int abs_delta_y = abs(primeiro_y_ - y);
        // Ve se ja da pra decidir.
        if (abs_delta_x > DELTA_MINIMO_TRANSLACAO_ROTACAO && abs_delta_y > DELTA_MINIMO_TRANSLACAO_ROTACAO) {
          // Usa o maior delta.
          translacao_rotacao_ = (abs_delta_x > abs_delta_y) ? TR_ROTACAO : TR_TRANSLACAO;
          VLOG(1) << "Comecando (desempate) " << ((translacao_rotacao_ == TR_ROTACAO) ? "rotacao" : "translacao");
        } else if (abs_delta_x > DELTA_MINIMO_TRANSLACAO_ROTACAO) {
          translacao_rotacao_ = TR_ROTACAO;
          VLOG(1) << "Comecando rotacao";
        } else if (abs_delta_y > DELTA_MINIMO_TRANSLACAO_ROTACAO) {
          VLOG(1) << "Comecando translacao";
          translacao_rotacao_ = TR_TRANSLACAO;
        } else {
          VLOG(1) << "Ainda indeciso";
          ultimo_x_ = x;
          ultimo_y_ = y;
          return false;
        }
        // Se chegou aqui eh pq mudou de estado. Comeca a temporizar.
        ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_MOVIMENTOS_PARCIAIS;
      }
      // Deltas desde o ultimo movimento.
      float delta_x = (x - ultimo_x_);
      float delta_y = (y - ultimo_y_);
      // Realiza rotacao/translacao da entidade.
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* e = BuscaEntidade(id);
        if (e == nullptr) {
          continue;
        }
        if (translacao_rotacao_ == TR_ROTACAO) {
          e->IncrementaRotacaoZGraus(delta_x);
        } else if (translacao_rotacao_ == TR_TRANSLACAO) {
          e->IncrementaZ(delta_y * SENSIBILIDADE_ROTACAO_Y);
        }
        if (e->Tipo() != TE_ENTIDADE) {
          atualizar_mapa_luzes = true;
        }
      }
      if (atualizar_mapa_luzes) {
        luzes_pontuais_.clear();
      }
      ultimo_x_ = x;
      ultimo_y_ = y;
      return false;
    }
    case ETAB_ROTACAO: {
      // Realiza a rotacao da tela.
      float olho_rotacao = olho_.rotacao_rad();
      olho_rotacao -= (x - ultimo_x_) * SENSIBILIDADE_ROTACAO_X;
      VLOG(1) << "x: " << x << ", ultimo_x: " << ultimo_x_;
      VLOG(1) << "y: " << x << ", ultimo_y: " << ultimo_x_;
      if (olho_rotacao >= 2 * M_PI) {
        olho_rotacao -= 2 * M_PI;
      } else if (olho_rotacao <= - 2 * M_PI) {
        olho_rotacao += 2 * M_PI;
      }
      VLOG(1) << "olho rotacao: " << olho_rotacao;
      olho_.set_rotacao_rad(olho_rotacao);
      // move o olho no eixo Z de acordo com o eixo Y do movimento
      float olho_altura = olho_.altura();
      olho_altura -= (y - ultimo_y_) * SENSIBILIDADE_ROTACAO_Y;
      if (olho_altura < OLHO_ALTURA_MINIMA) {
        olho_altura = OLHO_ALTURA_MINIMA;
      }
      else if (olho_altura > OLHO_ALTURA_MAXIMA) {
        olho_altura = OLHO_ALTURA_MAXIMA;
      }
      VLOG(1) << "olho altura: " << olho_altura;
      olho_.set_altura(olho_altura);
      // A rotacao nao altera o cursor, portanto nao deve atualizar o ultimo_xy.
      //ultimo_x_ = x;
      //ultimo_y_ = y;
      AtualizaOlho(0, true  /*forcar*/);
      return true;
    }
    break;
    case ETAB_ENTS_PRESSIONADAS: {
      // Realiza o movimento da entidade paralelo ao XY na mesma altura do click original.
      parametros_desenho_.set_offset_terreno(ultimo_z_3d_);
      parametros_desenho_.set_desenha_entidades(false);
      float nx, ny, nz;
      if (!MousePara3dParaleloZero(x, y, &nx, &ny, &nz)) {
        return false;
      }

      float dx = nx - ultimo_x_3d_;
      float dy = ny - ultimo_y_3d_;
      int quantidade_movimento = 0;
      bool atualizar_mapa_luzes = false;
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        float ex0 = entidade_selecionada->X();
        float ey0 = entidade_selecionada->Y();
        float ex1 = ex0 + dx;
        float ey1 = ey0 + dy;
        float z_depois = entidade_selecionada->Z();
        // Apenas entidades respeitam o solo, formas podem entrar.
        if (entidade_selecionada->Tipo() == TE_ENTIDADE) {
          float z_olho = entidade_selecionada->ZOlho();
          float altura_olho = entidade_selecionada->AlturaOlho();
          bool manter_chao = entidade_selecionada->Apoiada(); //Apoiado(ex0, ey0, z_olho, altura_olho);
          if (manter_chao) {
            ResultadoZApoio res = ZApoio(ex1, ey1, z_olho, altura_olho);
            float z_chao_depois = ZChao(nx, ny);
            VLOG(2) << "zchao_depois: " << z_chao_depois << ", res.z_apoio: " << res.z_apoio;
            if (!res.apoiado) {
              VLOG(2) << "usando chao";
              z_depois = z_chao_depois;
            } else {
              VLOG(2) << "usando res apoio";
              z_depois = res.z_apoio;
            }
            VLOG(2) << "mantendo apoio: z_depois: " << z_depois;
          } else {
            z_depois = std::max(ZChao(nx, ny), entidade_selecionada->Z());
            VLOG(2) << "nao mantendo apoio, z_depois " << z_depois;
          }
        } else {
          atualizar_mapa_luzes = true;
        }
        entidade_selecionada->MovePara(ex1, ey1, z_depois);
        Posicao pos;
        pos.set_x(ex1);
        pos.set_y(ey1);
        pos.set_z(0.0f);
        auto& vp = rastros_movimento_[id];
        int andou = AndouQuadrado(pos, vp.back());
        if (andou != 0) {
          if (andou == 1) {
            // eixo X mantem, eixo y volta.
            pos.set_y(vp.back().y());
          } else if (andou == 2) {
            // eixo y
            pos.set_x(vp.back().x());
          }
          vp.push_back(pos);
          if (quantidade_movimento == 0) {
            quantidade_movimento += (andou == 3) ? 2 : 1;
          }
        }
      }
      if (atualizar_mapa_luzes) {
        luzes_pontuais_.clear();
      }
      quadrados_movimentados_ += quantidade_movimento;
      ultimo_x_ = x;
      ultimo_y_ = y;
      ultimo_x_3d_ = nx;
      ultimo_y_3d_ = ny;
    }
    break;
    case ETAB_DESLIZANDO: {
      if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
        // Primeira pessoa ajusta altura do olho.
        float delta_y = (y - ultimo_y_) * 0.01f;
        float olho_altura = olho_.altura() - delta_y;
        if (olho_altura > OLHO_ALTURA_MAXIMA) {
          olho_altura = OLHO_ALTURA_MAXIMA;
        } else if (olho_altura < OLHO_ALTURA_MINIMA) {
          olho_altura = OLHO_ALTURA_MINIMA;
        }
        olho_.set_altura(olho_altura);
        float delta_x =  (x - ultimo_x_) * 0.003f;
        TrataRotacaoPorDelta(delta_x);
        ultimo_x_ = x;
        ultimo_y_ = y;
      } else {
        // Como pode ser chamado entre atualizacoes, atualiza a MODELVIEW.
        //gl::ModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
        gl::MatrizEscopo salva_matriz(gl::MATRIZ_MODELAGEM_CAMERA);
        gl::CarregaIdentidade();
        ConfiguraOlhar();
        // Faz picking do tabuleiro sem entidades.
        float nx, ny, nz;
        if (!MousePara3dParaleloZero(x, y, &nx, &ny, &nz)) {
          return false;
        }

        float delta_x = nx - ultimo_x_3d_;
        float delta_y = ny - ultimo_y_3d_;
        // Dependendo da posicao da pinca, os deltas podem se tornar muito grandes. Tentar manter o olho no tabuleiro.
        auto* p = olho_.mutable_alvo();
        float novo_x = p->x() - delta_x;
        float novo_y = p->y() - delta_y;
        const float tolerancia_quadrados = 10;
        const float maximo_x = TamanhoX() + TAMANHO_LADO_QUADRADO * tolerancia_quadrados;
        const float maximo_y = TamanhoY() + TAMANHO_LADO_QUADRADO * tolerancia_quadrados;
        if (novo_x < -maximo_x || novo_x > maximo_x || novo_y < -maximo_y || novo_y > maximo_y) {
          VLOG(1) << "Olho fora do tabuleiro";
          return false;
        }
        p->set_x(novo_x);
        p->set_y(novo_y);
        olho_.clear_destino();
        AtualizaOlho(0  /*intervalo_ms*/, true  /*forca*/);
        ultimo_x_ = x;
        ultimo_y_ = y;
        // No caso de deslizamento, nao precisa atualizar as coordenadas do ultimo_*_3d porque por definicao
        // do movimento, ela fica fixa (o tabuleiro desliza acompanhando o dedo).
      }
    }
    break;
    case ETAB_RELEVO: {
      TrataNivelamentoTerreno(x, y);
      break;
    }
    case ETAB_QUAD_PRESSIONADO:
      if (modo_clique_ == MODO_TERRENO) {
        estado_ = ETAB_RELEVO;
        ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_TERRENO;
        notificacao_desfazer_.set_tipo(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
        auto* cenario_antes = notificacao_desfazer_.mutable_tabuleiro_antes();
        cenario_antes->set_id_cenario(proto_corrente_->id_cenario());
        *cenario_antes->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
        TrataNivelamentoTerreno(x, y);
        break;
      }
      // Passthough proposital.
    case ETAB_SELECIONANDO_ENTIDADES: {
      quadrado_selecionado_ = -1;
      float x3d, y3d, z3d;
      parametros_desenho_.set_desenha_entidades(false);
      if (!MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d)) {
        // Mouse fora do tabuleiro.
        return false;
      }
      ultimo_x_3d_ = x3d;
      ultimo_y_3d_ = y3d;
      ultimo_z_3d_ = z3d;
      // Encontra as entidades cujos centros estao dentro dos limites da selecao.
      std::vector<unsigned int> es;
      for (const auto& eit : entidades_) {
        ids_entidades_selecionadas_.clear();
        const Entidade& e = *eit.second;
        if (e.IdCenario() == proto_corrente_->id_cenario() &&
            PontoDentroQuadrado(e.X(), e.Y(), primeiro_x_3d_, primeiro_y_3d_, ultimo_x_3d_, ultimo_y_3d_)) {
          es.push_back(e.Id());
        }
      }
      SelecionaEntidades(es);
      estado_ = ETAB_SELECIONANDO_ENTIDADES;
    }
    break;
    case ETAB_DESENHANDO: {
      float x3d, y3d, z3d;
      parametros_desenho_.set_desenha_entidades(false);
      parametros_desenho_.set_offset_terreno(primeiro_z_3d_);
      if (!MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d)) {
        // Mouse fora do tabuleiro.
        return false;
      }
      ultimo_x_3d_ = x3d;
      ultimo_y_3d_ = y3d;
      if (forma_selecionada_ == TF_LIVRE) {
        Posicao pos;
        pos.set_x(ultimo_x_3d_ - primeiro_x_3d_);
        pos.set_y(ultimo_y_3d_ - primeiro_y_3d_);
        pos.set_z(ZChao(ultimo_x_3d_, ultimo_y_3d_));
        if (DistanciaHorizontalQuadrado(pos, forma_proto_.ponto(forma_proto_.ponto_size() - 1)) > DELTA_MINIMO_DESENHO_LIVRE) {
          forma_proto_.add_ponto()->Swap(&pos);
        }
      } else {
        auto* pos = forma_proto_.mutable_pos();
        pos->set_x((primeiro_x_3d_ + ultimo_x_3d_) / 2.0f);
        pos->set_y((primeiro_y_3d_ + ultimo_y_3d_) / 2.0f);
        auto* escala = forma_proto_.mutable_escala();
        escala->set_x(fabs(primeiro_x_3d_ - ultimo_x_3d_));
        escala->set_y(fabs(primeiro_y_3d_ - ultimo_y_3d_));
        escala->set_z(TAMANHO_LADO_QUADRADO);
      }
      VLOG(2) << "Prosseguindo: " << forma_proto_.ShortDebugString();
    }
    break;
    default: ;
  }
  return false;
}

void Tabuleiro::FinalizaEstadoCorrente() {
  switch (estado_) {
    case ETAB_ENTS_TRANSLACAO_ROTACAO:
    case ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA: {
      ciclos_para_atualizar_ = -1;
      if (estado_ == ETAB_ENTS_TRANSLACAO_ROTACAO && translacao_rotacao_ == TR_NENHUM) {
        // Nada a fazer.
      } else {
        ntf::Notificacao grupo_notificacoes;
        grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
        for (const auto& id_proto : translacoes_rotacoes_escalas_antes_) {
          const auto& proto_antes = id_proto.second;
          auto* entidade = BuscaEntidade(id_proto.first);
          if (entidade == nullptr) {
            continue;
          }
          auto* n = grupo_notificacoes.add_notificacao();
          n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
          auto* e_antes = n->mutable_entidade_antes();
          e_antes->set_id(entidade->Id());
          e_antes->set_rotacao_z_graus(proto_antes.rotacao_z_graus());
          *e_antes->mutable_pos() = proto_antes.pos();
          *e_antes->mutable_escala() = proto_antes.escala();
          auto* e_depois = n->mutable_entidade();
          e_depois->set_id(id_proto.first);
          e_depois->set_rotacao_z_graus(entidade->RotacaoZGraus());
          *e_depois->mutable_pos() = entidade->Pos();
          *e_depois->mutable_escala() = entidade->Proto().escala();
          central_->AdicionaNotificacaoRemota(new ntf::Notificacao(*n));
        }
        // Para desfazer.
        AdicionaNotificacaoListaEventos(grupo_notificacoes);
      }
      estado_ = estado_anterior_;
      translacoes_rotacoes_escalas_antes_.clear();
      return;
    }
    case ETAB_ROTACAO:
      estado_ = estado_anterior_;
      return;
    case ETAB_DESLIZANDO:
      VLOG(1) << "primeiro_x_: " << primeiro_x_ << ", ultimo_x_: " << ultimo_x_
              << ", primeiro_y_: " << primeiro_y_ << ", ultimo_y_: " << ultimo_y_;
      if (camera_ == CAMERA_PRIMEIRA_PESSOA && primeiro_x_ == ultimo_x_ && primeiro_y_ == ultimo_y_) {
        // Isso provavelmente foi um clique convertido em deslize.
        DeselecionaEntidades();
      }
      estado_ = estado_anterior_;
      return;
    case ETAB_ENTS_PRESSIONADAS: {
      ciclos_para_atualizar_ = -1;
      if (primeiro_x_3d_ == ultimo_x_3d_ &&
          primeiro_y_3d_ == ultimo_y_3d_) {
        // Nao houve movimento.
        estado_ = ETAB_ENTS_SELECIONADAS;
        rastros_movimento_.clear();
        quadrados_movimentados_ = 0;
        return;
      }
      // Para desfazer.
      ntf::Notificacao g_desfazer;
      g_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
      Posicao vetor_delta;
      vetor_delta.set_x(ultimo_x_3d_ - primeiro_x_3d_);
      vetor_delta.set_y(ultimo_y_3d_ - primeiro_y_3d_);
      vetor_delta.set_z(ultimo_z_3d_ - primeiro_z_3d_);
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
        auto* e = n->mutable_entidade();
        e->set_id(id);
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        auto* destino = e->mutable_destino();
        destino->set_x(entidade_selecionada->X());
        destino->set_y(entidade_selecionada->Y());
        destino->set_z(entidade_selecionada->Z());
        central_->AdicionaNotificacaoRemota(n);
        // Para desfazer.
        auto* n_desfazer = g_desfazer.add_notificacao();
        n_desfazer->set_tipo(ntf::TN_MOVER_ENTIDADE);
        n_desfazer->mutable_entidade()->set_id(id);
        n_desfazer->mutable_entidade_antes()->set_id(id);
        auto* pos_final = n_desfazer->mutable_entidade()->mutable_destino();
        pos_final->set_x(entidade_selecionada->X());
        pos_final->set_y(entidade_selecionada->Y());
        pos_final->set_z(entidade_selecionada->Z());
        auto* pos_original = n_desfazer->mutable_entidade_antes()->mutable_pos();
        pos_original->set_x(entidade_selecionada->X() - vetor_delta.x());
        pos_original->set_y(entidade_selecionada->Y() - vetor_delta.y());
        pos_original->set_z(entidade_selecionada->Z() - vetor_delta.z());
      }
      AdicionaNotificacaoListaEventos(g_desfazer);
      estado_ = ETAB_ENTS_SELECIONADAS;
      rastros_movimento_.clear();
      quadrados_movimentados_ = 0;
      //parametros_desenho_.Clear();
      parametros_desenho_.clear_offset_terreno();
      parametros_desenho_.clear_desenha_entidades();
      return;
    }
    case ETAB_SELECIONANDO_ENTIDADES: {
      if (ids_entidades_selecionadas_.empty()) {
        DeselecionaEntidades();
      } else {
        estado_ = ETAB_ENTS_SELECIONADAS;
      }
      return;
    }
    case ETAB_QUAD_PRESSIONADO:
      estado_ = ETAB_QUAD_SELECIONADO;
      return;
    case ETAB_DESENHANDO: {
      estado_ = estado_anterior_;
      modo_clique_ = MODO_NORMAL;  // garante que o modo clique sai depois de usar o ctrl+botao direito.
      VLOG(1) << "Finalizando: " << forma_proto_.ShortDebugString();
      forma_proto_.mutable_cor()->CopyFrom(forma_cor_);
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      n.mutable_entidade()->Swap(&forma_proto_);
      TrataNotificacao(n);
      return;
    }
    case ETAB_RELEVO: {
      estado_ = estado_anterior_;
      ciclos_para_atualizar_ = -1;
      RefrescaTerrenoParaClientes();
      auto* cenario_depois = notificacao_desfazer_.mutable_tabuleiro();
      cenario_depois->set_id_cenario(proto_corrente_->id_cenario());
      *cenario_depois->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
      AdicionaNotificacaoListaEventos(notificacao_desfazer_);
      notificacao_desfazer_.Clear();
      return;
    }
    default:
      //estado_ = ETAB_OCIOSO;
      ;
  }
}


void Tabuleiro::TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y) {
  ultimo_x_ = x;
  ultimo_y_ = y;

  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  float x3d, y3d, z3d;
  MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  ultimo_z_3d_ = z3d;
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;

  if (tipo_objeto == OBJ_ENTIDADE || tipo_objeto == OBJ_ENTIDADE_LISTA) {
    // Entidade.
    VLOG(1) << "Picking alternar selecao entidade id " << id;
    AlternaSelecaoEntidade(id);
  } else if (tipo_objeto == OBJ_CONTROLE_VIRTUAL) {
    VLOG(1) << "Picking alternar selecao no controle virtual " << id;
    PickingControleVirtual(x, y, true  /*alt*/, false  /*duplo*/, id);
  } else {
    VLOG(1) << "Picking alternar selecao ignorado.";
  }
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataBotaoAcaoPressionado(bool acao_padrao, int x, int y) {
  // Preenche os dados comuns.
  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  TrataBotaoAcaoPressionadoPosPicking(acao_padrao, x, y, id, tipo_objeto, profundidade);
}

void Tabuleiro::TrataBotaoAcaoPressionadoPosPicking(
    bool acao_padrao, int x, int y, unsigned int id, unsigned int tipo_objeto, float profundidade) {
  if ((tipo_objeto != OBJ_TABULEIRO) && (tipo_objeto != OBJ_ENTIDADE) && (tipo_objeto != OBJ_ENTIDADE_LISTA)) {
    // invalido.
    return;
  }
  // Primeiro, entidades.
  unsigned int id_entidade_destino = Entidade::IdInvalido;
  Posicao pos_entidade;
  Posicao pos_tabuleiro;
  if (tipo_objeto == OBJ_ENTIDADE || tipo_objeto == OBJ_ENTIDADE_LISTA) {
    VLOG(1) << "Acao em entidade: " << id;
    // Entidade.
    id_entidade_destino = id;
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    pos_entidade.set_x(x3d);
    pos_entidade.set_y(y3d);
    pos_entidade.set_z(z3d);
    pos_entidade.set_id_cenario(cenario_corrente_);
    // Depois tabuleiro.
    pos_tabuleiro.set_x(x3d);
    pos_tabuleiro.set_y(y3d);
    pos_tabuleiro.set_z(z3d);
    pos_tabuleiro.set_id_cenario(cenario_corrente_);
  } else if (tipo_objeto == OBJ_TABULEIRO) {
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    unsigned int id_quadrado = IdQuadrado(x3d, y3d);
    VLOG(1) << "Acao no tabuleiro: " << id_quadrado;
    // Tabuleiro, posicao do quadrado clicado.
    // Posicao exata do clique.
    pos_tabuleiro.set_x(x3d);
    pos_tabuleiro.set_y(y3d);
    pos_tabuleiro.set_z(z3d);
    pos_tabuleiro.set_id_cenario(cenario_corrente_);
  }

  // Executa a acao: se nao houver ninguem selecionado, faz sinalizacao. Se houver, ha dois modos de execucao:
  // - Efeito de area
  // - Efeito individual.
  std::vector<unsigned int> ids_origem;
  ids_origem = IdsPrimeiraPessoaOuEntidadesSelecionadas();
  if (acao_padrao || ids_origem.size() == 0) {
    auto it = mapa_acoes_.find("Sinalização");
    AcaoProto acao_proto;
    if (it == mapa_acoes_.end()) {
      acao_proto.set_tipo(ACAO_SINALIZACAO);
    } else {
      acao_proto = *it->second;
    }
    // Sem entidade selecionada, realiza sinalizacao.
    VLOG(1) << "Acao de sinalizacao: " << acao_proto.ShortDebugString();
    if (id_entidade_destino != Entidade::IdInvalido) {
      acao_proto.add_id_entidade_destino(id_entidade_destino);
    }
    acao_proto.mutable_pos_tabuleiro()->CopyFrom(pos_tabuleiro);
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_ADICIONAR_ACAO);
    n.mutable_acao()->Swap(&acao_proto);
    TrataNotificacao(n);
  } else {
    // Realiza a acao de cada entidade contra o alvo ou local.
    // Usa modelo selecionado.
    VLOG(1) << "Acao de entidades.";
    // Para desfazer.
    ntf::Notificacao grupo_desfazer;
    grupo_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    float atraso_segundos = 0;
    for (auto id_selecionado : ids_origem) {
      Entidade* entidade = BuscaEntidade(id_selecionado);
      if (entidade == nullptr || entidade->Tipo() != TE_ENTIDADE) {
        continue;
      }
      AcaoProto acao_proto = entidade->Acao(mapa_acoes_);
      if (!acao_proto.has_tipo()) {
        LOG(ERROR) << "Acao invalida da entidade";
        continue;
      }
      if (id_entidade_destino != Entidade::IdInvalido) {
        acao_proto.add_id_entidade_destino(id_entidade_destino);
      }
      acao_proto.set_atraso_s(atraso_segundos);
      acao_proto.mutable_pos_tabuleiro()->CopyFrom(pos_tabuleiro);
      acao_proto.set_id_entidade_origem(id_selecionado);

      ntf::Notificacao n;
      n.set_tipo(ntf::TN_ADICIONAR_ACAO);
      if (acao_proto.efeito_area()) {
        if (pos_entidade.has_x()) {
          acao_proto.mutable_pos_entidade()->CopyFrom(pos_entidade);
        }
        int delta_pontos_vida = 0;
        if (HaValorListaPontosVida()) {
          delta_pontos_vida = LeValorListaPontosVida(entidade, acao_proto.id());
          entidade->ProximoAtaque();
          acao_proto.set_delta_pontos_vida(delta_pontos_vida);
          acao_proto.set_afeta_pontos_vida(true);
        }
        acao_proto.clear_id_entidade_destino();
        std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(acao_proto);
        for (auto id : ids_afetados) {
          const Entidade* entidade_destino = BuscaEntidade(id);
          if (entidade_destino == nullptr) {
            // Nunca deveria acontecer pois a funcao EntidadesAfetadasPorAcao ja buscou a entidade.
            LOG(ERROR) << "Entidade nao encontrada, nunca deveria acontecer.";
            continue;
          }
          if (entidade_destino->MaximoPontosVida() <= 0) {
            VLOG(1) << "Ignorando entidade que nao pode ser afetada por acao de area";
            continue;
          }
          acao_proto.add_id_entidade_destino(id);
          // Para desfazer.
          if (delta_pontos_vida == 0) {
            continue;
          }
          int delta_pv_pos_salvacao = delta_pontos_vida;
          if (acao_proto.permite_salvacao()) {
            if (entidade_destino->ProximaSalvacao() == RS_MEIO) {
              delta_pv_pos_salvacao /= 2;
            } else if (entidade_destino->ProximaSalvacao() == RS_QUARTO) {
              delta_pv_pos_salvacao /= 4;
            } else if (entidade_destino->ProximaSalvacao() == RS_ANULOU) {
              delta_pv_pos_salvacao = 0;
            }
          }
          auto* nd = grupo_desfazer.add_notificacao();
          PreencheNotificacaoAtualizaoPontosVida(*entidade_destino, delta_pv_pos_salvacao, nd, nd);
        }
        VLOG(2) << "Acao de area: " << acao_proto.ShortDebugString();
        n.mutable_acao()->CopyFrom(acao_proto);
      } else {
        Entidade* entidade_destino =
           id_entidade_destino != Entidade::IdInvalido ? BuscaEntidade(id_entidade_destino) : nullptr;
        bool realiza_acao = true;
        if (HaValorListaPontosVida() && entidade_destino != nullptr) {
          int vezes = 1;
          std::string texto;
          if (modo_dano_automatico_ && acao_proto.permite_defesa_armadura()) {
            VLOG(1) << "--------------------------";
            VLOG(1) << "iniciando ataque vs defesa";
            std::tie(vezes, texto, realiza_acao) = AtaqueVsDefesa(*entidade, *entidade_destino, opcoes_.ataque_vs_defesa_posicao_real() ? pos_entidade : Posicao());
            VLOG(1) << "--------------------------";
            AdicionaLogEvento(std::string("entidade ") +
                (entidade->Proto().rotulo().empty() ? net::to_string(entidade->Id()) : entidade->Proto().rotulo()) + " " +
                texto);
            acao_proto.set_texto(texto);
          }
          int delta_pontos_vida = 0;
          auto* nd = grupo_desfazer.add_notificacao();
          if (vezes < 0 || !realiza_acao) {
            if (vezes < 0) {
              PreencheNotificacaoDerrubaOrigem(*entidade, &n, nd);
            }
            ntf::Notificacao n_texto;
            n_texto.set_tipo(ntf::TN_ADICIONAR_ACAO);
            auto* acao_texto = n_texto.mutable_acao();
            acao_texto->set_tipo(ACAO_DELTA_PONTOS_VIDA);
            acao_texto->set_texto(acao_proto.texto());
            acao_texto->add_id_entidade_destino(entidade->Id());  // o destino eh a origem.
            TrataNotificacao(n_texto);
          } else {
            // Ja faz a notificacao de desfazer aqui.
            for (int i = 0; i < vezes; ++i) {
              delta_pontos_vida += LeValorListaPontosVida(entidade, acao_proto.id());
            }
            if (vezes > 0) {
              delta_pontos_vida += LeValorAtaqueFurtivo(entidade);
            }
            entidade->ProximoAtaque();
            if (acao_proto.permite_salvacao()) {
              if (entidade_destino->ProximaSalvacao() == RS_MEIO) {
                delta_pontos_vida /= 2;
              } else if (entidade_destino->ProximaSalvacao() == RS_QUARTO) {
                delta_pontos_vida /= 4;
              } else if (entidade_destino->ProximaSalvacao() == RS_ANULOU) {
                delta_pontos_vida = 0;
              }
            }
            acao_proto.set_delta_pontos_vida(delta_pontos_vida);
            acao_proto.set_afeta_pontos_vida(true);
            PreencheNotificacaoAtualizaoPontosVida(*entidade_destino, delta_pontos_vida, nd, nd);
          }
        }
        if (realiza_acao) {
          VLOG(1) << "Acao individual: " << acao_proto.ShortDebugString();
          n.mutable_acao()->CopyFrom(acao_proto);
        }
      }
      TrataNotificacao(n);
      atraso_segundos += 0.5f;
    }
    AdicionaNotificacaoListaEventos(grupo_desfazer);
  }
  // Atualiza as acoes executadas da entidade se houver apenas uma. A sinalizacao nao eh adicionada a entidade porque ela possui forma propria.
  auto* e = EntidadeSelecionada();
  if (e == nullptr) {
    return;
  }
  AcaoProto acao_executada = e->Acao(mapa_acoes_);
  if (!acao_executada.has_tipo() || acao_executada.tipo() == ACAO_SINALIZACAO || acao_executada.id().empty()) {
    return;
  }
  e->AdicionaAcaoExecutada(acao_executada.id());
  if (!EmModoMestre() && IdCameraPresa() == e->Id()) {
    // Envia para o mestre as lista de acoes executadas da entidade.
    auto* n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    n->set_servidor_apenas(true);
    auto* entidade  = n->mutable_entidade();
    entidade->set_id(e->Id());
    entidade->mutable_lista_acoes()->CopyFrom(e->Proto().lista_acoes());
    central_->AdicionaNotificacaoRemota(n);
  }
}

void Tabuleiro::TrataBotaoTerrenoPressionadoPosPicking(float x3d, float y3d, float z3d) {
  // converte x3d e y3d em um quadrado.
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;
  unsigned int id_quadrado = IdQuadrado(x3d, y3d);
  if (id_quadrado == static_cast<unsigned int>(-1)) {
    return;
  }
  SelecionaQuadrado(id_quadrado);
}

void Tabuleiro::TrataBotaoTransicaoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto) {
  if (tipo_objeto != OBJ_ENTIDADE) {
    // invalido.
    LOG(INFO) << "Transicao so funciona em entidades";
    return;
  }
  Entidade* entidade_transicao = BuscaEntidade(id);
  if (entidade_transicao == nullptr) {
    LOG(ERROR) << "Entidade " << id << " nao encontrada";
    return;
  }
  if (entidade_transicao->TipoTransicao() == EntidadeProto::TRANS_TESOURO) {
    LOG(INFO) << "Transicao de tesouro";
    auto ids_receber = IdsPrimeiraPessoaIncluindoEntidadesSelecionadas();
    if (ids_receber.size() != 1) {
      LOG(INFO) << "So pode transitar tesouro para uma entidade";
      return;
    }
    auto* receptor = BuscaEntidade(ids_receber[0]);
    if (receptor == nullptr) {
      LOG(INFO) << "Receptor eh nullptr";
      return;
    }
    if (receptor->Id() == entidade_transicao->Id()) {
      LOG(INFO) << "Receptor tem que ser diferente";
      return;
    }
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    {
      auto* n_perdeu = n.add_notificacao();
      n_perdeu->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
      n_perdeu->mutable_entidade()->set_id(id);
      n_perdeu->mutable_entidade()->mutable_tesouro()->set_tesouro("");
      n_perdeu->mutable_entidade_antes()->set_id(id);
      n_perdeu->mutable_entidade_antes()->mutable_tesouro()->set_tesouro(entidade_transicao->Proto().tesouro().tesouro());

      auto* n_ganhou = n.add_notificacao();
      const std::string& tesouro_corrente = receptor->Proto().tesouro().tesouro();
      n_ganhou->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
      n_ganhou->mutable_entidade()->set_id(ids_receber[0]);
      n_ganhou->mutable_entidade()->mutable_tesouro()->set_tesouro(
          tesouro_corrente + (tesouro_corrente.empty() ? "" : "\n") + entidade_transicao->Proto().tesouro().tesouro());
      n_ganhou->mutable_entidade_antes()->set_id(ids_receber[0]);
      n_ganhou->mutable_entidade_antes()->mutable_tesouro()->set_tesouro(tesouro_corrente);
    }
    {
      // Texto de transicao.
      auto* n_texto = n.add_notificacao();
      n_texto->set_tipo(ntf::TN_ADICIONAR_ACAO);
      auto* acao = n_texto->mutable_acao();
      acao->set_tipo(ACAO_DELTA_PONTOS_VIDA);
      acao->set_texto(entidade_transicao->Proto().tesouro().tesouro());
      acao->add_id_entidade_destino(receptor->Id());
      //*acao->mutable_pos_entidade() = receptor->Pos();
    }

    TrataNotificacao(n);
    AdicionaNotificacaoListaEventos(n);
    return;
  }
  if (!entidade_transicao->Proto().transicao_cenario().has_id_cenario()) {
    LOG(INFO) << "Entidade " << id << " nao possui transicao de cenario";
    return;
  }
  int id_cenario = entidade_transicao->Proto().transicao_cenario().id_cenario();
  if (id_cenario < CENARIO_PRINCIPAL) {
    LOG(ERROR) << "Id de cenario deve ser >= CENARIO_PRINCIPAL, id: " << id_cenario;
    return;
  }
  if (BuscaSubCenario(id_cenario) == nullptr && !EmModoMestreIncluindoSecundario()) {
    LOG(WARNING) << "Apenas o mestre pode criar cenarios";
    return;
  }
  if (EntidadeEstaSelecionada(entidade_transicao->Id())) {
    // Este caso eh muito irritante e acontece com frequencia. A entidade de transicao esta selecionada e vai para o novo cenario.
    DeselecionaEntidade(entidade_transicao->Id());
  }

  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);

  // Posicao destino especificada, caso contrario usa a posicao do objeto de transicao.
  Posicao pos_destino(entidade_transicao->Proto().transicao_cenario().has_x() ? entidade_transicao->Proto().transicao_cenario() : entidade_transicao->Pos());
  auto ids_a_transitar = IdsPrimeiraPessoaIncluindoEntidadesSelecionadas();
  if (!ids_a_transitar.empty()) {
    // Computa a posicao centro das entidades.
    Posicao centro;
    float x_centro = 0, y_centro = 0;
    int n_entidades = 0;
    for (unsigned int id : ids_a_transitar) {
      auto* entidade_movendo = BuscaEntidade(id);
      if (entidade_movendo == nullptr) {
        continue;
      }
      x_centro += entidade_movendo->X();
      y_centro += entidade_movendo->Y();
      ++n_entidades;
    }
    if (n_entidades > 0) {
      x_centro /= n_entidades;
      y_centro /= n_entidades;
    }

    for (unsigned int id : ids_a_transitar) {
      auto* entidade_movendo = BuscaEntidade(id);
      if (entidade_movendo == nullptr) {
        continue;
      }
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      n->mutable_entidade()->set_id(id);
      n->mutable_entidade_antes()->set_id(id);
      n->mutable_entidade_antes()->mutable_pos()->CopyFrom(entidade_movendo->Pos());  // Para desfazer.
      float dx = entidade_movendo->X() - x_centro;
      float dy = entidade_movendo->Y() - y_centro;
      n->mutable_entidade()->mutable_pos()->set_x(pos_destino.x() + dx);
      n->mutable_entidade()->mutable_pos()->set_y(pos_destino.y() + dy);
      n->mutable_entidade()->mutable_pos()->set_z(entidade_transicao->Proto().transicao_cenario().z());
      n->mutable_entidade()->mutable_pos()->set_id_cenario(id_cenario);
    }
  }
  // Criacao vem por ultimo para a inversao do desfazer funcionar, pois se a remocao for feita antes de mover as entidades de volta,
  // ao mover as entidades vao ter sido removidas.
  if (BuscaSubCenario(id_cenario) == nullptr) {
    {
      // Cria uma entidade igual a de transicao no cenario destino.
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      n->mutable_entidade()->CopyFrom(entidade_transicao->Proto());
      n->mutable_entidade()->clear_id();
      n->mutable_entidade()->mutable_destino()->CopyFrom(pos_destino);
      n->mutable_entidade()->mutable_destino()->set_id_cenario(id_cenario);
      // A volta, posicao do objeto de origem.
      n->mutable_entidade()->mutable_transicao_cenario()->CopyFrom(entidade_transicao->Pos());
      n->mutable_entidade()->mutable_transicao_cenario()->set_id_cenario(proto_corrente_->id_cenario());
    }
    {
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_CRIAR_CENARIO);
      n->mutable_tabuleiro()->set_id_cenario(id_cenario);
    }
  }

  {
    // A camera vai para a posicao de transicao ou para a posicao do objeto no outro cenario.
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_REINICIAR_CAMERA);
    Posicao pos_olho;
    if (entidade_transicao->Proto().transicao_cenario().has_x()) {
      pos_olho = entidade_transicao->Proto().transicao_cenario();
    } else {
      pos_olho = entidade_transicao->Pos();
    }
    pos_olho.set_id_cenario(id_cenario);
    *n->mutable_tabuleiro_antes()->mutable_camera_inicial() = olho_;
    *n->mutable_tabuleiro()->mutable_camera_inicial() = olho_;
    *n->mutable_tabuleiro()->mutable_camera_inicial()->mutable_alvo() = pos_olho;
  }

  if (grupo_notificacoes.notificacao_size() > 0) {
    TrataNotificacao(grupo_notificacoes);
    if (ids_adicionados_.size() == 1) {
      for (auto& n : *grupo_notificacoes.mutable_notificacao()) {
        if (n.tipo() == ntf::TN_ADICIONAR_ENTIDADE) {
          n.mutable_entidade()->set_id(ids_adicionados_[0]);
        }
      }
    }
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}

void Tabuleiro::TrataBotaoReguaPressionadoPosPicking(float x3d, float y3d, float z3d) {
  auto* entidade = EntidadeSelecionadaOuPrimeiraPessoa();
  if (entidade == nullptr) {
    VLOG(1) << "Ignorando clique de regua, ou nao ha entidade ou ha mais de uma selecionada.";
    return;
  }
  Vector3 ve(entidade->X(), entidade->Y(), entidade->Z());
  Vector3 vd(x3d, y3d, z3d);
  float distancia = (ve - vd).length();
  char texto[31];
  snprintf(texto, 30, "%.1f m, %d quadrados", distancia, static_cast<int>(distancia / TAMANHO_LADO_QUADRADO));
  auto* n = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  auto* a = n->mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_local_apenas(true);
  auto* pd = a->mutable_pos_entidade();
  pd->set_x(x3d);
  pd->set_y(y3d);
  pd->set_z(z3d);
  pd->set_id_cenario(entidade->IdCenario());
  a->set_texto(texto);
  central_->AdicionaNotificacao(n);
}

void Tabuleiro::TrataBotaoLiberado() {
  FinalizaEstadoCorrente();
}

void Tabuleiro::TrataMouseParadoEm(int x, int y) {
  unsigned int id;
  unsigned int pos_pilha;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha);
  if (pos_pilha != OBJ_ENTIDADE && pos_pilha != OBJ_CONTROLE_VIRTUAL) {
    // Mouse no tabuleiro.
    id_entidade_detalhada_ = Entidade::IdInvalido;
    tipo_entidade_detalhada_ = OBJ_INVALIDO;
    return;
  }
  id_entidade_detalhada_ = id;
  tipo_entidade_detalhada_ = pos_pilha;
  //temporizador_detalhamento_ms_ = TEMPO_DETALHAMENTO_MS;
}

void Tabuleiro::TrataRolagem(dir_rolagem_e direcao) {
  ntf::Notificacao g_desfazer;
  g_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (auto& id_ent : entidades_) {
    auto* entidade = id_ent.second.get();
    auto* n_desfazer = g_desfazer.add_notificacao();
    {
      // Posicao inicial para desfazer.
      n_desfazer->set_tipo(ntf::TN_MOVER_ENTIDADE);
      n_desfazer->mutable_entidade()->set_id(id_ent.first);
      auto* pos_original = n_desfazer->mutable_entidade_antes()->mutable_pos();
      pos_original->set_x(entidade->X());
      pos_original->set_y(entidade->Y());
      pos_original->set_z(entidade->Z());
    }

    float delta_x = 0.0f;
    float delta_y = 0.0f;
    if (direcao == DIR_LESTE) {
      delta_x = -(TamanhoX() - 3) * TAMANHO_LADO_QUADRADO;
    } else if (direcao == DIR_OESTE) {
      delta_x = (TamanhoX() - 3) * TAMANHO_LADO_QUADRADO;
    } else if (direcao == DIR_NORTE) {
      delta_y = -(TamanhoY() - 3) * TAMANHO_LADO_QUADRADO;
    } else if (direcao == DIR_SUL) {
      delta_y = (TamanhoY() - 3) * TAMANHO_LADO_QUADRADO;
    } else {
      LOG(ERROR) << "Direcao invalida";
      return;
    }

    {
      Posicao destino;
      destino.set_x(entidade->X() + delta_x);
      destino.set_y(entidade->Y() + delta_y);
      destino.set_z(entidade->Z());
      entidade->Destino(destino);
    }
    {
      // Notificacao para clientes remotos.
      auto* n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      e->set_id(id_ent.first);
      auto* destino = e->mutable_destino();
      destino->set_x(entidade->X() + delta_x);
      destino->set_y(entidade->Y() + delta_y);
      destino->set_z(entidade->Z());
      // Posicao final para desfazer.
      n_desfazer->mutable_entidade()->mutable_destino()->CopyFrom(*destino);
      central_->AdicionaNotificacaoRemota(n);
    }
  }
  AdicionaNotificacaoListaEventos(g_desfazer);
}

namespace {

// De acordo com o modo de desenho, altera as configuracoes de pd.
// tipo_objeto: um dos OBJ_*.
void ConfiguraParametrosDesenho(Tabuleiro::modo_clique_e modo_clique, ParametrosDesenho* pd) {
  pd->set_nao_desenha_entidades_fixas_translucidas(true);
  switch (modo_clique) {
    case Tabuleiro::MODO_NORMAL:
      break;
    case Tabuleiro::MODO_SELECAO_TRANSICAO:
      break;
    case Tabuleiro::MODO_ACAO:
      break;
    case Tabuleiro::MODO_TERRENO:
      return;
    case Tabuleiro::MODO_SINALIZACAO:
      break;
    case Tabuleiro::MODO_TRANSICAO:
      break;
    case Tabuleiro::MODO_REGUA:
      break;
    case Tabuleiro::MODO_D20:
      break;
    case Tabuleiro::MODO_DESENHO:
      break;
    case Tabuleiro::MODO_ROTACAO:
      break;
    case Tabuleiro::MODO_AJUDA:
      break;
    default:
      ;
  }
}

}  // namespace

void Tabuleiro::TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao) {
  ultimo_x_ = x;
  ultimo_y_ = y;

  ConfiguraParametrosDesenho(modo_clique_, &parametros_desenho_);
  unsigned int id, tipo_objeto;
  float profundidade;
  // Tem que ter uma forma de nao desenhar entidades fixas transparentes. Para poder fazer picking dentro dagua.
  // O problema desse jeito eh pegar coisas do outro lado de paredes solidas.
  // parametros_desenho_.set_nao_desenha_entidades_fixas(true);
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  float x3d, y3d, z3d;
  MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
  // Trata modos diferentes de clique, exceto para controle virtual.
  // Se o clique foi no controle virtual, apenas o MODO_AJUDA trata aqui. Isso permite que o controle
  // funcione em outros modos, ao mesmo tempo que a ajuda possa ser mostrada para os botoes do controle.
  if (modo_clique_ == MODO_AJUDA || (modo_clique_ != MODO_NORMAL && tipo_objeto != OBJ_CONTROLE_VIRTUAL)) {
    switch (modo_clique_) {
      case MODO_SELECAO_TRANSICAO: {
        if (tipo_objeto != OBJ_TABULEIRO) {
          return;
        }
        auto* entidade = BuscaEntidade(notificacao_selecao_transicao_.entidade().id());
        if (entidade == nullptr) {
          LOG(WARNING) << "Entidade nao existe mais.";
          // para sair do modo.
          break;
        }
        auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
        *n->mutable_entidade() = entidade->Proto();
        n->mutable_entidade()->set_tipo_transicao(EntidadeProto::TRANS_CENARIO);
        auto* trans = n->mutable_entidade()->mutable_transicao_cenario();
        trans->set_x(x3d);
        trans->set_y(y3d);
        trans->set_z(z3d);
        trans->set_id_cenario(cenario_corrente_);
        central_->AdicionaNotificacao(n);
        break;
      }
      case MODO_ACAO:
        TrataBotaoAcaoPressionadoPosPicking(false, x, y, id, tipo_objeto, profundidade);
        if (!lista_pontos_vida_.empty() && !modo_dano_automatico_) {
          return;  // Mantem o MODO_ACAO.
        }
        break;
      case MODO_TERRENO:
        TrataBotaoTerrenoPressionadoPosPicking(x3d, y3d, z3d);
        // Nao quero voltar para o modo normal.
        return;
      case MODO_SINALIZACAO:
        TrataBotaoAcaoPressionadoPosPicking(true, x, y, id, tipo_objeto, profundidade);
        break;
      case MODO_TRANSICAO:
        TrataBotaoTransicaoPressionadoPosPicking(x, y, id, tipo_objeto);
        break;
      case MODO_REGUA:
        TrataBotaoReguaPressionadoPosPicking(x3d, y3d, z3d);
        break;
      case MODO_D20:
        TrataBotaoD20PressionadoPosPicking(x3d, y3d, z3d);
        break;
      case MODO_DESENHO:
        TrataBotaoDesenhoPressionado(x, y);
        break;
      case MODO_ROTACAO:
        TrataBotaoRotacaoPressionado(x, y);
        break;
      case MODO_AJUDA:
        TrataMouseParadoEm(x, y);
        temporizador_detalhamento_ms_ = TEMPO_DETALHAMENTO_MS;
        modo_clique_ = MODO_NORMAL;
        break;
      default:
        ;
    }
    modo_clique_ = MODO_NORMAL;
    return;
  }

  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  ultimo_z_3d_ = z3d;
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;

  if (camera_ == CAMERA_PRIMEIRA_PESSOA &&
      (tipo_objeto == OBJ_TABULEIRO || tipo_objeto == OBJ_ENTIDADE || tipo_objeto == OBJ_ENTIDADE_LISTA ||
       tipo_objeto == OBJ_EVENTO_ENTIDADE)) {
    TrataBotaoDireitoPressionado(x, y);
    return;
  }

  if (tipo_objeto == OBJ_TABULEIRO) {
    // Tabuleiro.
    // Converte x3d y3d para id quadrado.
    SelecionaQuadrado(IdQuadrado(x3d, y3d));
  } else if (tipo_objeto == OBJ_ENTIDADE || tipo_objeto == OBJ_ENTIDADE_LISTA) {
    // Entidade.
    VLOG(1) << "Picking entidade id " << id;
    if (alterna_selecao) {
      AlternaSelecaoEntidade(id);
    } else {
      if (!EntidadeEstaSelecionada(id)) {
        // Se nao estava selecionada, so ela.
        SelecionaEntidade(id, tipo_objeto == OBJ_ENTIDADE_LISTA);
      }
      bool ha_entidades_selecionadas = !ids_entidades_selecionadas_.empty();
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          // Forma nao deixa rastro.
          continue;
        }
        Posicao pos;
        pos.set_x(entidade_selecionada->X());
        pos.set_y(entidade_selecionada->Y());
        pos.set_z(0.0f);
        rastros_movimento_[id].push_back(pos);
      }
      if (ha_entidades_selecionadas) {
        ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_MOVIMENTOS_PARCIAIS;
        estado_ = ETAB_ENTS_PRESSIONADAS;
      } else {
        MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d);
        ultimo_x_3d_ = x3d;
        ultimo_y_3d_ = y3d;
        ultimo_z_3d_ = z3d;
        primeiro_x_3d_ = x3d;
        primeiro_y_3d_ = y3d;
        primeiro_z_3d_ = z3d;
        SelecionaQuadrado(IdQuadrado(x3d, y3d));
      }
    }
  } else if (tipo_objeto == OBJ_ROLAGEM) {
    VLOG(1) << "Picking em ponto de rolagem id " << id;
    TrataRolagem(static_cast<dir_rolagem_e>(id));
  } else if (tipo_objeto == OBJ_CONTROLE_VIRTUAL) {
    VLOG(1) << "Picking no controle virtual " << id;
    PickingControleVirtual(x, y, alterna_selecao, false  /*duplo*/, id);
  } else if (tipo_objeto == OBJ_EVENTO_ENTIDADE) {
    VLOG(1) << "Picking em evento da entidade " << id;
    ApagaEventosZeradosDeEntidadeNotificando(id);
  } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    TrataBotaoDireitoPressionado(x, y);
    return;
  } else {
    VLOG(1) << "Picking lugar nenhum.";
    DeselecionaEntidades();
  }
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataBotaoDireitoPressionado(int x, int y) {
  VLOG(1) << "Botao direito pressionado";
  float x3d, y3d, z3d;
  MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d);
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  primeiro_x_ = ultimo_x_ = x;
  primeiro_y_ = ultimo_y_ = y;
  if (estado_ != ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA) {
    estado_anterior_ = estado_;
    estado_ = ETAB_DESLIZANDO;
  }
}

void Tabuleiro::TrataBotaoRotacaoPressionado(int x, int y) {
  primeiro_x_ = x;
  primeiro_y_ = y;
  ultimo_x_ = x;
  ultimo_y_ = y;
  VLOG(1) << "Botao rotacao pressionado x: " << x << " y: " << y << ", ultimo_x: " << ultimo_x_ << ", ultimo_y: " << ultimo_y_;
  if (estado_ == ETAB_ENTS_PRESSIONADAS) {
    FinalizaEstadoCorrente();
    estado_ = ETAB_ENTS_TRANSLACAO_ROTACAO;
    estado_anterior_ = ETAB_ENTS_SELECIONADAS;
    translacao_rotacao_ = TR_NENHUM;
    translacoes_rotacoes_escalas_antes_.clear();
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      // Neste caso, usa o X para rotacao e o Z para translacao.
      EntidadeProto proto;
      proto.mutable_pos()->set_z(entidade->Z());
      proto.set_rotacao_z_graus(entidade->RotacaoZGraus());
      translacoes_rotacoes_escalas_antes_[entidade->Id()].Swap(&proto);
    }
  } else if (estado_ == ETAB_DESENHANDO) {
    FinalizaEstadoCorrente();
    estado_anterior_ = ETAB_OCIOSO;
    estado_ = ETAB_ROTACAO;
  } else {
    estado_anterior_ = estado_;
    estado_ = ETAB_ROTACAO;
  }
}

void Tabuleiro::TrataBotaoDesenhoPressionado(int x, int y) {
#if APENAS_MESTRE_CRIA_FORMAS
  if (!EmModoMestreIncluindoSecundario()) {
    VLOG(1) << "Apenas mestre pode desenhar.";
    // Apenas mestre pode desenhar.
    return;
  }
#endif
  VLOG(1) << "Botao desenho pressionado";
  ultimo_x_ = x;
  ultimo_y_ = y;
  float x3d, y3d, z3d;
  MousePara3d(x, y, &x3d, &y3d, &z3d);
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  ultimo_z_3d_ = z3d;
  forma_proto_.Clear();
  forma_proto_.set_id(GeraIdEntidade(id_cliente_));
  forma_proto_.set_tipo(TE_FORMA);
  forma_proto_.set_sub_tipo(forma_selecionada_);
  auto* pos = forma_proto_.mutable_pos();
  pos->set_x(primeiro_x_3d_);
  pos->set_y(primeiro_y_3d_);
  pos->set_z(z3d);
  auto* escala = forma_proto_.mutable_escala();
  if (forma_selecionada_ == TF_LIVRE) {
    auto* ponto = forma_proto_.add_ponto();
    ponto->set_x(0.0f);
    ponto->set_y(0.0f);
    ponto->set_z(0.0f);
    // Usa a escala em Z para a largura da linha.
    escala->set_z(1.0f);
  } else {
    escala->set_x(0);
    escala->set_y(0);
    escala->set_z(0);
  }
  forma_proto_.mutable_cor()->CopyFrom(forma_cor_);
  forma_proto_.mutable_cor()->set_a(0.5f);
  estado_anterior_ = estado_;
  estado_ = ETAB_DESENHANDO;
  VLOG(2) << "Iniciando: " << forma_proto_.ShortDebugString();
}

void Tabuleiro::TrataDuploCliqueEsquerdo(int x, int y) {
  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha == OBJ_TABULEIRO) {
    unsigned int id = ultima_entidade_selecionada_;
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    // Tabuleiro: cria uma entidade nova.
    SelecionaQuadrado(IdQuadrado(x3d, y3d));
    estado_ = ETAB_QUAD_SELECIONADO;
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    if (id != Entidade::IdInvalido) {
      notificacao.set_id_referencia(id);
    }
    TrataNotificacao(notificacao);
  } else if (pos_pilha == OBJ_ENTIDADE || pos_pilha == OBJ_ENTIDADE_LISTA) {
    // Entidade.
    if (SelecionaEntidade(id, true  /*forcar_fixa*/)) {
      auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
      n->set_modo_mestre(EmModoMestreIncluindoSecundario());
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      central_->AdicionaNotificacao(n);
    }
  } if (pos_pilha == OBJ_CONTROLE_VIRTUAL) {
    PickingControleVirtual(x, y, false  /*alterna selecao*/, true  /*duplo*/, id);
  } else {
    ;
  }
}

void Tabuleiro::TrataDuploCliqueDireito(int x, int y) {
  float x3d, y3d, z3d;
  if (!MousePara3d(x, y, &x3d, &y3d, &z3d)) {
    return;
  }
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    // Move entidade primeira pessoa.
    auto* entidade = BuscaEntidade(IdCameraPresa());
    if (entidade != nullptr) {
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_MOVER_ENTIDADE);
      *n.mutable_entidade_antes()->mutable_pos() = entidade->Pos();
      n.mutable_entidade_antes()->set_id(entidade->Id());
      n.mutable_entidade_antes()->set_apoiada(entidade->Apoiada());
      auto* e = n.mutable_entidade();
      e->set_id(entidade->Id());
      auto* pos_depois = e->mutable_destino();
      pos_depois->set_x(x3d);
      pos_depois->set_y(y3d);
      pos_depois->set_z(z3d);
      e->set_apoiada(entidade->Apoiada());
      AdicionaNotificacaoListaEventos(n);
      MoveEntidadeNotificando(n);
    }
  } else {
    auto* p = olho_.mutable_destino();
    p->set_x(x3d);
    p->set_y(y3d);
    p->set_z(z3d);
  }
}

void Tabuleiro::TrataMovimentoEntidadesSelecionadas(bool frente_atras, float valor) {
  Posicao vetor_visao;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &vetor_visao);
  // angulo da camera em relacao ao eixo X.
  Vector2 vetor_movimento;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    // Ao andar na primeira pessoa, cancela espiada para evitar atravessar objetos ja que a deteccao de colisao eh feita
    // so no inicio.
    Entidade* entidade = BuscaEntidade(IdCameraPresa());
    if (entidade->Proto().espiando() != 0) {
      EntidadeProto proto;
      proto.set_espiando(0);
      entidade->AtualizaParcial(proto);
    }
  }
  if (camera_ == CAMERA_PRIMEIRA_PESSOA || !proto_corrente_->desenha_grade()) {
    if (frente_atras) {
      vetor_movimento = Vector2(vetor_visao.x(), vetor_visao.y());
    } else {
      RodaVetor2d(-90.0f, &vetor_visao);
      vetor_movimento = Vector2(vetor_visao.x(), vetor_visao.y());
    }
    vetor_movimento = vetor_movimento.normalize() * TAMANHO_LADO_QUADRADO;
    if (valor < 0.0f) {
      vetor_movimento *= -1;
    }
  } else {
    float rotacao_graus = VetorParaRotacaoGraus(vetor_visao.x(), vetor_visao.y());
    if (rotacao_graus > -45.0f && rotacao_graus <= 45.0f) {
      // Camera apontando para x positivo.
      if (frente_atras) {
        vetor_movimento.x = TAMANHO_LADO_QUADRADO * valor;
      } else {
        vetor_movimento.y = TAMANHO_LADO_QUADRADO * -valor;
      }
    } else if (rotacao_graus > 45.0f && rotacao_graus <= 135) {
      // Camera apontando para y positivo.
      if (frente_atras) {
        vetor_movimento.y = TAMANHO_LADO_QUADRADO * valor;
      } else {
        vetor_movimento.x = TAMANHO_LADO_QUADRADO * valor;
      }
    } else if (rotacao_graus > 135 || rotacao_graus < -135) {
      // Camera apontando para x negativo.
      if (frente_atras) {
        vetor_movimento.x = TAMANHO_LADO_QUADRADO * -valor;
      } else {
        vetor_movimento.y = TAMANHO_LADO_QUADRADO * valor;
      }
    } else {
      // Camera apontando para y negativo.
      if (frente_atras) {
        vetor_movimento.y = TAMANHO_LADO_QUADRADO * -valor;
      } else {
        vetor_movimento.x = TAMANHO_LADO_QUADRADO * -valor;
      }
    }
  }
  // Colisao
  float dx = 0.0f, dy = 0.0f, dz = 0.0f;
  std::vector<unsigned int> ids_colisao = IdsPrimeiraPessoaOuEntidadesSelecionadas();
  Entidade* entidade_referencia = nullptr;
  if (ids_colisao.size() == 1) {
    entidade_referencia = BuscaEntidade(ids_colisao[0]);
  } else if (ids_colisao.size() > 1) {
    Vector2 media;
    // Media.
    for (unsigned int id : ids_colisao) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      media.x += entidade->X();
      media.y += entidade->Y();
    }
    media /= ids_colisao.size();
    // Maior distancia para normalizacao.
    float maior = 0.0f;
    for (unsigned int id : ids_colisao) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      Vector2 dmedia(entidade->X() - media.x, entidade->Y() - media.y);
      maior = std::max(dmedia.length(), maior);
    }
    // Normaliza vetor de movimento.
    Vector2 vetor_referencia(vetor_movimento);
    vetor_referencia.normalize();
    vetor_referencia *= maior;
    float menor_distancia = std::numeric_limits<float>::max();
    for (unsigned int id : ids_colisao) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      Vector2 ev(entidade->X(), entidade->Y());
      ev -= media;
      float d = (ev - vetor_referencia).length();
      if (d < menor_distancia) {
        menor_distancia = d;
        entidade_referencia = entidade;
      }
    }
  }

  if (entidade_referencia != nullptr) {
    auto res_colisao = DetectaColisao(*entidade_referencia, Vector3(vetor_movimento.x, vetor_movimento.y, 0.0f));
    vetor_movimento.normalize();
    vetor_movimento *= res_colisao.profundidade;
  }
  dx = vetor_movimento.x;
  dy = vetor_movimento.y;

  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  std::unordered_set<unsigned int> ids;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    ids.insert(IdCameraPresa());
  } else {
    ids = ids_entidades_selecionadas_;
  }
  for (unsigned int id : ids) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    VLOG(2) << "Movendo entidade " << id << ", dx: " << dx << ", dy: " << dy << ", dz: " << dz;
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_MOVER_ENTIDADE);
    auto* e = n->mutable_entidade();
    auto* e_antes = n->mutable_entidade_antes();
    e->set_id(id);
    e_antes->set_id(id);
    float nx = entidade_selecionada->X() + dx;
    float ny = entidade_selecionada->Y() + dy;
    float nz = entidade_selecionada->Z() + dz;
    auto* p = e->mutable_destino();
    p->set_x(nx);
    p->set_y(ny);
    p->set_z(nz);
    if (entidade_selecionada->Tipo() == TE_ENTIDADE) {
      float z_olho = entidade_selecionada->ZOlho();
      float altura_olho = entidade_selecionada->AlturaOlho();
      bool manter_chao = entidade_selecionada->Apoiada();
      float z_chao_depois = ZChao(nx, ny);
      ResultadoZApoio res = ZApoio(nx, ny, z_olho, altura_olho);
      if (manter_chao) {
        VLOG(1) << "mantendo apoio";
        if (!res.apoiado) {
          res.z_apoio = z_chao_depois;
        }
        p->set_z(res.z_apoio);
      } else {
        float z_apoio = std::max(res.z_apoio, z_chao_depois);
        if (z_apoio > entidade_selecionada->Z()) {
          VLOG(1) << "apoiando entidade nao apoiada";
          p->set_z(z_apoio);
          e->set_apoiada(true);
          n->mutable_entidade_antes()->set_apoiada(false);
        } else {
          VLOG(1) << "nao mantendo apoio";
          p->set_z(entidade_selecionada->Z());
        }
      }
    }
    // Para desfazer.
    p = e_antes->mutable_pos();
    p->set_x(entidade_selecionada->X());
    p->set_y(entidade_selecionada->Y());
    p->set_z(entidade_selecionada->Z());
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    AtualizaOlho(0, true  /*forcar*/);
  }
}

void Tabuleiro::TrataTranslacaoZ(float delta) {
  if (ModoClique() == MODO_TERRENO) {
    TrataDeltaTerreno(delta * TAMANHO_LADO_QUADRADO);
  } else {
    ntf::Notificacao grupo_notificacoes;
    grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    for (unsigned int id : IdsEntidadesSelecionadasOuPrimeiraPessoa()) {
      auto* entidade_selecionada = BuscaEntidade(id);
      if (entidade_selecionada == nullptr) {
        continue;
      }
      // Salva para desfazer.
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      auto* e_antes = n->mutable_entidade_antes();
      e->set_id(entidade_selecionada->Id());
      e_antes->set_id(entidade_selecionada->Id());
      e_antes->mutable_pos()->CopyFrom(entidade_selecionada->Pos());
      // Altera a translacao em Z.
      //entidade_selecionada->IncrementaZ(delta * TAMANHO_LADO_QUADRADO);
      e->mutable_destino()->CopyFrom(entidade_selecionada->Pos());
      e->mutable_destino()->set_z(e->destino().z() + delta * TAMANHO_LADO_QUADRADO);
      const Posicao& pos = e->destino();
      float altura_olho = entidade_selecionada->AlturaOlho();
      e->set_apoiada(Apoiado(pos.x(), pos.y(), pos.z() + altura_olho, altura_olho));
      n->mutable_entidade_antes()->set_apoiada(entidade_selecionada->Apoiada());
    }
    // Nop mas envia para os clientes.
    TrataNotificacao(grupo_notificacoes);
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}


}  // namespace ent