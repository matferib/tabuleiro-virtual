// Este arquivo contem as rotinas responsaveis por tratar eventos de
// teclado e mouse do tabuleiro.

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
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
#include <google/protobuf/text_format.h>

//#define VLOG_NIVEL 1
#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/controle_virtual.pb.h"
#include "ent/entidade.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/tabuleiro_interface.h"
#include "ent/tabuleiro_terreno.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "matrix/vectors.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace ent {

namespace {

using google::protobuf::StringPrintf;

std::string StringTipoCarregamento(TipoCarregamento tc) {
  switch (tc) {
    case TC_RODADA_COMPLETA: return "rodada";
    case TC_PADRAO: return "padrao";
    case TC_MOVIMENTO: return "movimento";
    default: return "livre";
  }
}

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

// As funcoes Preenchem assumem ATUALIZAR_PARCIAL, permitindo multiplas chamadas sem comprometer as anteriores.
// n e n_desfazer podem ser iguais.
void PreencheNotificacaoDerrubar(
    const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  entidade_depois->set_caida(true);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    *n_desfazer->mutable_entidade() = *entidade_depois;
    auto* e_antes = n_desfazer->mutable_entidade_antes();
    e_antes->set_id(entidade.Id());
    e_antes->set_caida(entidade.Proto().caida());
  }
}

void PreencheNotificacaoRemoverUmReflexo(
    const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  const auto& proto = entidade.Proto();
  const std::vector<const EntidadeProto::Evento*>& eventos = EventosTipo(EFEITO_REFLEXOS, proto);
  if (eventos.empty()) return;
  const EntidadeProto::Evento* maior_evento = nullptr;
  for (const auto* evento : eventos) {
    if (evento->complementos().empty() || evento->complementos(0) <= 0) continue;
    if (maior_evento == nullptr || evento->complementos(0) > maior_evento->complementos(0)) {
      maior_evento = evento;
    }
  }
  if (maior_evento == nullptr) return;

  EntidadeProto *proto_antes, *proto_depois;
  std::tie(proto_antes, proto_depois) = PreencheNotificacaoEntidade(
      ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  *proto_antes->add_evento() = *maior_evento;
  auto* evento_depois = proto_depois->add_evento();
  *evento_depois = *maior_evento;
  evento_depois->mutable_complementos()->Set(0, maior_evento->complementos(0) - 1);

  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
  }
}

void PreencheNotificacaoAgarrar(
    unsigned int id, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  *entidade_depois->mutable_agarrado_a() = entidade.Proto().agarrado_a();
  entidade_depois->add_agarrado_a(id);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    *n_desfazer->mutable_entidade() = *entidade_depois;
    auto* e_antes = n_desfazer->mutable_entidade_antes();
    e_antes->set_id(entidade.Id());
    *e_antes->mutable_agarrado_a() = entidade.Proto().agarrado_a();
    if (e_antes->agarrado_a().empty()) {
      e_antes->add_agarrado_a(Entidade::IdInvalido);
    }
  }
}

void PreencheNotificacaoDesagarrar(
    unsigned int id, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  for (unsigned int aid : entidade.Proto().agarrado_a()) {
    if (id != aid) entidade_depois->add_agarrado_a(aid);
  }
  if (entidade_depois->agarrado_a().empty()) {
    entidade_depois->add_agarrado_a(Entidade::IdInvalido);
  }

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    *n_desfazer->mutable_entidade() = *entidade_depois;
    auto* e_antes = n_desfazer->mutable_entidade_antes();
    e_antes->set_id(entidade.Id());
    *e_antes->mutable_agarrado_a() = entidade.Proto().agarrado_a();
  }
}

// Preenche uma notificacao de esquiva contra a entidade de id passado.
void PreencheNotificacaoEsquiva(
    unsigned int id_entidade_destino, const Entidade& entidade, ntf::Notificacao* n, ntf::Notificacao* n_desfazer) {
  EntidadeProto *proto_antes, *proto_depois;
  std::tie(proto_antes, proto_depois) = PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade, n);
  proto_antes->mutable_dados_defesa()->set_entidade_esquiva(
      entidade.Proto().dados_defesa().has_entidade_esquiva() ? entidade.Proto().dados_defesa().entidade_esquiva() : Entidade::IdInvalido);
  proto_depois->mutable_dados_defesa()->set_entidade_esquiva(id_entidade_destino);
  if (n_desfazer != nullptr) {
    *n_desfazer = *n;
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

void Tabuleiro::TrataBotaoRolaDadoPressionadoPosPicking(float x3d, float y3d, float z3d) {
  if (faces_dado_ <= 0) {
    LOG(ERROR) << "Erro rolando dado: faces <= 0, " << faces_dado_ ;
    return;
  }
  char texto[31];
  snprintf(texto, 30, "d%d = %d", faces_dado_, RolaDado(faces_dado_));
  auto n = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  auto* a = n->mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_local_apenas(false);
  auto* pd = a->mutable_pos_entidade();
  pd->set_x(x3d);
  pd->set_y(y3d);
  pd->set_z(z3d);
  pd->set_id_cenario(IdCenario());
  a->set_texto(texto);
  central_->AdicionaNotificacao(n.release());
  mostrar_dados_ = false;
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
      n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
      n->mutable_entidade_antes()->set_id(entidade->Id());
      n->mutable_entidade()->set_id(entidade->Id());
      float fator = 1.0f + delta * SENSIBILIDADE_RODA * 0.1f;
      if (entidade->Tipo() == TE_ENTIDADE) {
        n->mutable_entidade_antes()->mutable_escala()->set_x(fator > 1.0 ? 0.8f : 1.2f);
        n->mutable_entidade()->mutable_escala()->set_x(      fator > 1.0 ? 1.2f : 0.8f);
      } else {
        *n->mutable_entidade_antes()->mutable_escala() = entidade->Proto().escala();
        auto* e = n->mutable_entidade();
        *e->mutable_escala() = entidade->Proto().escala();
        e->mutable_escala()->set_x(e->escala().x() * fator);
        e->mutable_escala()->set_y(e->escala().y() * fator);
        e->mutable_escala()->set_z(e->escala().z() * fator);
      }
      atualizar_mapa_luzes = true;
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
    RequerAtualizacaoLuzesPontuais();
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
    RequerAtualizacaoLuzesPontuais();
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
    RequerAtualizacaoLuzesPontuais();
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
      if (translacao_rotacao_ == TR_ROTACAO) {
        for (unsigned int id : ids_entidades_selecionadas_) {
          auto* e = BuscaEntidade(id);
          if (e == nullptr) continue;
          e->IncrementaRotacaoZGraus(delta_x);
          if (e->Tipo() != TE_ENTIDADE) {
            atualizar_mapa_luzes = true;
          }
        }
      } else {
        for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
          auto* e = BuscaEntidade(id);
          if (e == nullptr) continue;
          e->IncrementaZ(delta_y * SENSIBILIDADE_ROTACAO_Y);
          if (e->Tipo() != TE_ENTIDADE) {
            atualizar_mapa_luzes = true;
          }
        }
      }
      if (atualizar_mapa_luzes) {
        RequerAtualizacaoLuzesPontuais();
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
#if __APPLE__ && !USAR_OPENGL_ES
      // Bizarramente, o macosx ta ignorando o setCursor.
      ultimo_x_ = x;
      ultimo_y_ = y;
      AtualizaOlho(0, true  /*forcar*/);
      return false;
#else
      // A rotacao nao altera o cursor, portanto nao deve atualizar o ultimo_xy.
      //ultimo_x_ = x;
      //ultimo_y_ = y;
      AtualizaOlho(0, true  /*forcar*/);
      return true;
#endif
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
      for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
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
        if (entidade_selecionada->RespeitaSolo()) {
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
        RequerAtualizacaoLuzesPontuais();
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
        parametros_desenho_.set_offset_terreno(primeiro_z_3d_);
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
        auto* alvo = olho_.mutable_alvo();
        float novo_x = alvo->x() - delta_x;
        float novo_y = alvo->y() - delta_y;
        const float tolerancia_quadrados = 10;
        const float maximo_x = TamanhoX() + TAMANHO_LADO_QUADRADO * tolerancia_quadrados;
        const float maximo_y = TamanhoY() + TAMANHO_LADO_QUADRADO * tolerancia_quadrados;
        if (novo_x < -maximo_x || novo_x > maximo_x || novo_y < -maximo_y || novo_y > maximo_y) {
          VLOG(1) << "Olho fora do tabuleiro";
          return false;
        }
        alvo->set_x(novo_x);
        alvo->set_y(novo_y);
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
          n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
          auto* e_antes = n->mutable_entidade_antes();
          e_antes->set_id(entidade->Id());
          e_antes->set_rotacao_z_graus(proto_antes.rotacao_z_graus());
          *e_antes->mutable_pos() = proto_antes.pos();
          *e_antes->mutable_escala() = proto_antes.escala();
          e_antes->set_apoiada(entidade->Apoiada());
          auto* e_depois = n->mutable_entidade();
          e_depois->set_id(id_proto.first);
          e_depois->set_rotacao_z_graus(entidade->RotacaoZGraus());
          const Posicao& pos = entidade->Pos();
          float altura_olho = entidade->AlturaOlho();
          bool apoiado = Apoiado(pos.x(), pos.y(), pos.z() + altura_olho, altura_olho);
          *e_depois->mutable_pos() = pos;
          *e_depois->mutable_escala() = entidade->Proto().escala();
          e_depois->set_apoiada(apoiado);
          // Como a notificacao so vai para os clientes, apoia manualmente aqui.
          entidade->Apoia(apoiado);
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
      parametros_desenho_.clear_offset_terreno();
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
      for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        {
          auto n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
          auto* e = n->mutable_entidade();
          e->set_id(id);
          auto* destino = e->mutable_destino();
          destino->set_x(entidade_selecionada->X());
          destino->set_y(entidade_selecionada->Y());
          destino->set_z(entidade_selecionada->Z());
          destino->set_id_cenario(entidade_selecionada->IdCenario());
          central_->AdicionaNotificacaoRemota(n.release());
        }
        // Para desfazer.
        auto* n_desfazer = g_desfazer.add_notificacao();
        n_desfazer->set_tipo(ntf::TN_MOVER_ENTIDADE);
        n_desfazer->mutable_entidade()->set_id(id);
        n_desfazer->mutable_entidade_antes()->set_id(id);
        auto* pos_final = n_desfazer->mutable_entidade()->mutable_destino();
        pos_final->set_x(entidade_selecionada->X());
        pos_final->set_y(entidade_selecionada->Y());
        pos_final->set_z(entidade_selecionada->Z());
        pos_final->set_id_cenario(entidade_selecionada->IdCenario());
        auto* pos_original = n_desfazer->mutable_entidade_antes()->mutable_pos();
        pos_original->set_x(entidade_selecionada->X() - vetor_delta.x());
        pos_original->set_y(entidade_selecionada->Y() - vetor_delta.y());
        pos_original->set_z(entidade_selecionada->Z() - vetor_delta.z());
        pos_original->set_id_cenario(entidade_selecionada->IdCenario());
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

namespace {

// De acordo com o modo de desenho, altera as configuracoes de pd.
// tipo_objeto: um dos OBJ_*.
void ConfiguraParametrosDesenho(const Entidade* entidade_origem, Tabuleiro::modo_clique_e modo_clique, ParametrosDesenho* pd) {
  pd->set_nao_desenha_entidades_fixas_translucidas(true);
  switch (modo_clique) {
    case Tabuleiro::MODO_NORMAL:
      break;
    case Tabuleiro::MODO_SELECAO_TRANSICAO:
      break;
    case Tabuleiro::MODO_DOACAO:
      break;
    case Tabuleiro::MODO_ACAO:
      if (entidade_origem != nullptr) {
        const auto& dado_corrente = entidade_origem->DadoCorrente();
        if (dado_corrente != nullptr && dado_corrente->ignora_origem())
        pd->add_nao_desenhar_entidades(entidade_origem->Id());
      }
      break;
    case Tabuleiro::MODO_TERRENO:
      return;
    case Tabuleiro::MODO_SINALIZACAO:
      break;
    case Tabuleiro::MODO_TRANSICAO:
      pd->set_usar_transparencias(false);
      break;
    case Tabuleiro::MODO_REGUA:
      break;
    case Tabuleiro::MODO_ROLA_DADO:
      break;
    case Tabuleiro::MODO_DESENHO:
      break;
    case Tabuleiro::MODO_ROTACAO:
      break;
    case Tabuleiro::MODO_AJUDA:
      break;
    case Tabuleiro::MODO_ESQUIVA:
      break;
    default:
      ;
  }
}

}  // namespace



void Tabuleiro::TrataBotaoAcaoPressionado(bool acao_padrao, int x, int y) {
  ConfiguraParametrosDesenho(EntidadeCameraPresaOuSelecionada(), MODO_ACAO, &parametros_desenho_);
  // Preenche os dados comuns.
  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  TrataBotaoAcaoPressionadoPosPicking(acao_padrao, x, y, id, tipo_objeto, profundidade);
}

namespace {

const DadosAtaque& DadoCorrenteOuPadrao(const Entidade& entidade) {
  auto* da = entidade.DadoCorrente();
  if (da == nullptr) return DadosAtaque::default_instance();
  return *da;
}
const DadosAtaque& DadoCorrenteOuPadrao(const Entidade* entidade) {
  if (entidade == nullptr) return DadosAtaque::default_instance();
  auto* da = entidade->DadoCorrente();
  if (da == nullptr) return DadosAtaque::default_instance();
  return *da;
}

}  // namespace


float Tabuleiro::TrataAcaoExpulsarFascinarMortosVivos(
    float atraso_s, const Entidade* entidade, AcaoProto* acao_proto,
    ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  const int nivel_expulsao = NivelExpulsao(tabelas_, entidade->Proto());
  // Confere se entidade pode expulsar mortos vivos.
  if (nivel_expulsao <= 0) {
    AdicionaAcaoTextoLogado(entidade->Id(), "Entidade não pode expulsar/fascinar mortos vivos.");
    return atraso_s;
  }
  // TODO numero de expulsoes.

  // Teste de expulsao: d20 + carisma.
  const int d20 = RolaDado(20);
  const int modificador_carisma = ModificadorAtributo(TA_CARISMA, entidade->Proto());
  // Ta no livro do jogador.
  // TODO: o bonus vai acabar variando com tipo de expulsao. Por exemplo, extra planares, plantas etc.
  const int bonus_graduacao = Pericia("conhecimento_religiao", entidade->Proto()).pontos() > 5 ? 2 : 0;
  // Tabela de expulsao:
  // 0 or lower Cleric’s level -4
  // 1—3 Cleric’s level -3
  // 4—6 Cleric’s level -2
  // 7—9 Cleric’s level -1
  // 10—12 Cleric’s level
  // 13—15 Cleric’s level +1
  // 16—18 Cleric’s level +2
  // 19—21 Cleric’s level +3
  // 22 or higher Cleric’s level +4
  const int total_teste = d20 + modificador_carisma + bonus_graduacao;
  const int modificador_nivel = -4 + (std::max(std::min(22, total_teste), 0) + 2) / 3;
  const int maior_nivel = nivel_expulsao + modificador_nivel;
  acao_proto->set_bem_sucedida(true);  // acao realizada.

  // Coleta todos os alvos que estao dentro do raio de alcance da entidade.
  *acao_proto->mutable_pos_tabuleiro() = entidade->PosicaoAcao();
  std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(*acao_proto);
  atraso_s += acao_proto->duracao_s();
  std::vector<const Entidade*> entidades_por_distancia;
  for (auto id : ids_afetados) {
    const Entidade* entidade_destino = BuscaEntidade(id);
    if (entidade_destino == nullptr) {
      // Nunca deveria acontecer pois a funcao EntidadesAfetadasPorAcao ja buscou a entidade.
      LOG(ERROR) << "Entidade nao encontrada, nunca deveria acontecer.";
      continue;
    }
    if (!AcaoAfetaAlvo(*acao_proto, *entidade_destino)) {
      VLOG(1) << "Alvo " << RotuloEntidade(entidade_destino) << " não é morto vivo";
      continue;
    }

    if (Nivel(entidade_destino->Proto()) > maior_nivel) {
      acao_proto->set_gera_outras_acoes(true);  // mesmo que nao de dano, tem os textos.
      auto* por_entidade = acao_proto->add_por_entidade();
      por_entidade->set_id(id);
      por_entidade->set_texto("imune por dados de vida");
      continue;
    }

    VLOG(1) << "Alvo " << RotuloEntidade(entidade_destino) << " possivelmente afetado";
    entidades_por_distancia.push_back(entidade_destino);
  }

  // Ordena por distancia.
  auto OrdenacaoPorDistancia = [entidade] (const Entidade* lhs, const Entidade* rhs) {
    const float dql = DistanciaEmMetrosAoQuadrado(entidade->Pos(), lhs->Pos());
    const float dqr = DistanciaEmMetrosAoQuadrado(entidade->Pos(), rhs->Pos());
    return dql < dqr;
  };
  std::sort(entidades_por_distancia.begin(), entidades_por_distancia.end(), OrdenacaoPorDistancia);

  // Ve os que podem ser afetados, mais proximos primeiro.
  const int d6x2 = RolaValor("2d6");
  int dados_vida_afetados = d6x2 + nivel_expulsao + modificador_carisma;
  for (const auto* entidade_destino : entidades_por_distancia) {
    std::vector<int> ids_unicos_entidade_destino(IdsUnicosEntidade(*entidade_destino));
    const int nivel_destino = Nivel(entidade_destino->Proto());
    auto* por_entidade = acao_proto->add_por_entidade();
    por_entidade->set_id(entidade_destino->Id());
    if (nivel_destino > dados_vida_afetados) {
      VLOG(1) << "Alvo " << RotuloEntidade(entidade_destino) << " escapou: dados de expulsao insuficientes";
      por_entidade->set_texto("dados de expulsao insuficientes");
      continue;
    }
    dados_vida_afetados -= nivel_destino;
    acao_proto->set_afeta_pontos_vida(true);

    // Afetado!
    if (nivel_expulsao >= 2 * nivel_destino) {
      int delta = -entidade_destino->MaximoPontosVida() * 2;
      por_entidade->set_delta(delta);
      VLOG(1) << "Alvo " << RotuloEntidade(entidade_destino) << " destruido.";
      // Apenas para desfazer, o dano sera dado pela acao.
      auto* nd = grupo_desfazer->add_notificacao();
      PreencheNotificacaoAtualizacaoPontosVida(*entidade_destino, delta, TD_LETAL, nd, nd);
    } else {
      por_entidade->set_texto("afetado por expulsão");
      std::unique_ptr<ntf::Notificacao> n_efeito(new ntf::Notificacao);
      AcaoProto::EfeitoAdicional efeito_adicional;
      // TODO fascinar.
      efeito_adicional.set_efeito(EFEITO_MORTO_VIVO_EXPULSO);
      efeito_adicional.set_rodadas(10);
      PreencheNotificacaoEventoEfeitoAdicional(
          entidade->Id(), nivel_expulsao, *entidade_destino, efeito_adicional,
          &ids_unicos_entidade_destino, n_efeito.get(), grupo_desfazer->add_notificacao());
      central_->AdicionaNotificacao(n_efeito.release());
      atraso_s += 0.5f;
      VLOG(1) << "Alvo " << RotuloEntidade(entidade_destino) << " afetado.";
    }
  }
  VLOG(2) << "Acao de expulsao: " << acao_proto->ShortDebugString();
  AdicionaLogEvento(entidade->Id(),
      StringPrintf("Teste de expulsao: d20 (%d) + mod carisma (%d) + bonus religiao (%d) = %d; "
                   "Maior DV: nivel expulsao (%d) + modificador expulsao (%d) = %d; "
                   "maximo DV afetados: %d; destroi DV <= %d",
                   d20, modificador_carisma, bonus_graduacao, total_teste,
                   nivel_expulsao, modificador_nivel, maior_nivel,
                   dados_vida_afetados, nivel_expulsao / 2));
  *n->mutable_acao() = *acao_proto;
  return atraso_s;
}

float Tabuleiro::TrataAcaoProjetilArea(
    unsigned int id_entidade_destino, float atraso_s, const Posicao& pos_entidade_destino,
    Entidade* entidade, AcaoProto* acao_proto,
    ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  // Verifica antes se ha valor, para nao causar o efeito de area se nao houver.
  const bool ha_valor = HaValorListaPontosVida();

  const auto& da = DadoCorrenteOuPadrao(entidade);
  const bool incrementa_ataque = da.incrementa_proximo_ataque();
  atraso_s += TrataAcaoIndividual(
      id_entidade_destino, atraso_s, pos_entidade_destino, entidade, acao_proto, n, grupo_desfazer);
  if (!n->has_acao()) {
    // Nao realizou a acao. Nem continuar.
    VLOG(2) << "Acao de projetil de area nao realizada";
    return atraso_s;
  }
  if (!ha_valor) return atraso_s;

  const Entidade* entidade_destino = BuscaEntidade(id_entidade_destino);
  const bool acertou_direto = acao_proto->bem_sucedida();

  // A acao individual incrementou o ataque.
  if (incrementa_ataque) entidade->AtaqueAnterior();

  if (!acertou_direto && entidade_destino != nullptr && entidade != nullptr) {
    // Escolhe direcao aleatoria e soma um quadrado por incremento.
    float alcance_m = entidade->AlcanceAtaqueMetros();
    const float distancia_m = DistanciaAcaoAoAlvoMetros(*entidade, *entidade_destino, pos_entidade_destino);
    const int total_incrementos = distancia_m / alcance_m;
    VLOG(1) << "nao acertou projetil de area direto, vendo posicao de impacto. total de incrementos: " << total_incrementos;
    if (total_incrementos > 0) {
      Matrix4 rm;
      rm.rotateZ(RolaDado(360.0f));
      Vector3 v_d_impacto(TAMANHO_LADO_QUADRADO * total_incrementos, 0.0f, 0.0f);
      v_d_impacto = rm * v_d_impacto;
      Vector3 pos_impacto = v_d_impacto + PosParaVector3(pos_entidade_destino);
      pos_impacto.z = entidade_destino->Z();
      Vector3 v_e = PosParaVector3(entidade->PosicaoAcao());
      Vector3 v_e_impacto = pos_impacto - v_e;
      auto res = DetectaColisao(*entidade, v_e_impacto);
      if (res.colisao) {
        v_e_impacto.normalize();
        v_e_impacto *= res.profundidade;
        pos_impacto = v_e + v_e_impacto;
      }
      acao_proto->clear_pos_entidade();
      *acao_proto->mutable_pos_tabuleiro() = Vector3ParaPosicao(pos_impacto);
    }
  }

  std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(*acao_proto);
  for (auto id : ids_afetados) {
    const Entidade* entidade_destino = BuscaEntidade(id);
    if (entidade_destino == nullptr) {
      // Nunca deveria acontecer pois a funcao EntidadesAfetadasPorAcao ja buscou a entidade.
      LOG(ERROR) << "Entidade nao encontrada, nunca deveria acontecer.";
      continue;
    }
    if (!entidade_destino->Proto().has_max_pontos_vida()) {
      VLOG(1) << "Ignorando entidade que nao pode ser afetada por acao de area em projetil de area";
      continue;
    }

    AcaoProto::PorEntidade* por_entidade = nullptr;
    if (id == id_entidade_destino) {
      if (acertou_direto) {
        continue;
      } else {
        auto find_result = std::find_if(acao_proto->mutable_por_entidade()->begin(), acao_proto->mutable_por_entidade()->end(),
            [id_entidade_destino](AcaoProto::PorEntidade& po) { return po.id() == id_entidade_destino; });
        if (find_result == acao_proto->mutable_por_entidade()->end()) continue;
        por_entidade = &(*find_result);
      }
    } else {
       por_entidade = acao_proto->add_por_entidade();
    }
    if (por_entidade == nullptr) continue;

    acao_proto->set_bem_sucedida(true);
    acao_proto->set_afeta_pontos_vida(true);
    por_entidade->set_id(id);

    int delta_pv = -1;

    std::string texto_afeta;
    if (!AcaoAfetaAlvo(*acao_proto, *entidade_destino, &texto_afeta)) {
      delta_pv = 0;
      if (!texto_afeta.empty()) {
        ConcatenaString(texto_afeta, por_entidade->mutable_texto());
        AdicionaLogEvento(id, texto_afeta);
      }
    } else {
      ResultadoImunidadeOuResistencia resultado_elemento =
          ImunidadeOuResistenciaParaElemento(delta_pv, da, entidade_destino->Proto(), acao_proto->elemento());
      if (resultado_elemento.causa != ALT_NENHUMA) {
        delta_pv += resultado_elemento.resistido;
        ConcatenaString(resultado_elemento.texto, por_entidade->mutable_texto());
        AdicionaLogEvento(id, resultado_elemento.texto);
      }
    }
    por_entidade->set_delta(delta_pv);

    // Para desfazer.
    // Notificacao de desfazer.
    auto* nd = grupo_desfazer->add_notificacao();
    PreencheNotificacaoAtualizacaoPontosVida(*entidade_destino, delta_pv, TD_LETAL, nd, nd);
  }
  VLOG(2) << "Acao de projetil de area: " << acao_proto->ShortDebugString();
  *n->mutable_acao() = *acao_proto;
  if (incrementa_ataque) {
    entidade->ProximoAtaque();
  }
  return atraso_s;
}

float Tabuleiro::TrataAcaoEfeitoArea(
    float atraso_s, const Posicao& pos_entidade_destino, Entidade* entidade_origem, AcaoProto* acao_proto,
    ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  const Posicao* pos_destino = &acao_proto->pos_tabuleiro();
  if (pos_entidade_destino.has_x()) {
    // Usa a posicao da acao na entidade ao inves do tabuleiro. Esse é o ponto onde o picking acertou a entidade
    // e nao a posicao da entidade.
    *acao_proto->mutable_pos_entidade() = pos_entidade_destino;
    pos_destino = &acao_proto->pos_entidade();
  }
  // Verifica alcance.
  {
    const float alcance_m = entidade_origem->AlcanceAtaqueMetros();
    const Posicao& pos_origem = entidade_origem->PosicaoAcao();
    Vector3 va(pos_origem.x(), pos_origem.y(), pos_origem.z());
    Vector3 vd(pos_destino->x(), pos_destino->y(), pos_destino->z());
    VLOG(2) << "va: " << va;
    VLOG(2) << "vd: " << vd;
    const float distancia_m = (va - vd).length();
    VLOG(1)
        << "distancia: " << distancia_m << ", em quadrados: " << (distancia_m * METROS_PARA_QUADRADOS)
        << ", alcance maximo_m: " << alcance_m << ", em quadrados: " << (alcance_m * METROS_PARA_QUADRADOS);
    if (distancia_m > alcance_m) {
      AdicionaAcaoTextoLogado(entidade_origem->Id(), StringPrintf("Fora de alcance: %0.1f m, maximo: %0.1f m", distancia_m, alcance_m));
      return atraso_s;
    }
  }

  const auto& da = DadoCorrenteOuPadrao(entidade_origem);
  if (da.has_limite_vezes() || !da.taxa_refrescamento().empty()) {
    std::unique_ptr<ntf::Notificacao> n_consumo(new ntf::Notificacao);
    PreencheNotificacaoConsumoAtaque(*entidade_origem, da, n_consumo.get(), grupo_desfazer->add_notificacao());
    central_->AdicionaNotificacao(n_consumo.release());
  }

  int delta_pontos_vida_inicial = 0;
  if (HaValorListaPontosVida()) {
    delta_pontos_vida_inicial =
        LeValorListaPontosVida(entidade_origem, EntidadeProto(), acao_proto->id());
    if (da.cura()) {
      delta_pontos_vida_inicial = -delta_pontos_vida_inicial;
    }
    if (da.incrementa_proximo_ataque()) {
      // TODO desfazer.
      entidade_origem->ProximoAtaque();
    }
    acao_proto->set_delta_pontos_vida(delta_pontos_vida_inicial);
    acao_proto->set_afeta_pontos_vida(true);
  }
  // Este valor deve ser inalterado.
  const int delta_pontos_vida = delta_pontos_vida_inicial;
  acao_proto->clear_por_entidade();
  std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(*acao_proto);
  atraso_s += acao_proto->duracao_s();

  for (auto id : ids_afetados) {
    const Entidade* entidade_destino = BuscaEntidade(id);
    if (entidade_destino == nullptr) {
      // Nunca deveria acontecer pois a funcao EntidadesAfetadasPorAcao ja buscou a entidade.
      LOG(ERROR) << "Entidade nao encontrada, nunca deveria acontecer.";
      continue;
    }
    if (!entidade_destino->Proto().has_max_pontos_vida()) {
      VLOG(1) << "Ignorando entidade que nao pode ser afetada por acao de area";
      continue;
    }
    std::vector<int> ids_unicos_entidade_origem = entidade_origem == nullptr ? std::vector<int>() : IdsUnicosEntidade(*entidade_origem);
    std::vector<int> ids_unicos_entidade_destino = entidade_destino == nullptr ? std::vector<int>() : IdsUnicosEntidade(*entidade_destino);
    acao_proto->set_gera_outras_acoes(true);  // mesmo que nao de dano, tem os textos.
    auto* por_entidade = acao_proto->add_por_entidade();
    por_entidade->set_id(id);
    por_entidade->set_delta(delta_pontos_vida);

    std::string texto_afeta;
    if (!AcaoAfetaAlvo(*acao_proto, *entidade_destino, &texto_afeta)) {
      VLOG(1) << "Ignorando entidade que nao pode ser afetada por este tipo de ataque.";
      por_entidade->set_delta(0);
      if (!texto_afeta.empty()) {
        por_entidade->set_texto(texto_afeta);
      }
      continue;
    }

    if (!acao_proto->ignora_resistencia_magia() && entidade_destino->Proto().dados_defesa().resistencia_magia() > 0) {
      std::string resultado_rm;
      bool ataque_passou_rm;
      std::tie(ataque_passou_rm, resultado_rm) = AtaqueVsResistenciaMagia(tabelas_, da, *entidade_origem, *entidade_destino);
      por_entidade->set_texto(resultado_rm);
      AdicionaLogEvento(entidade_origem->Id(), resultado_rm);
      if (!ataque_passou_rm) {
        por_entidade->set_delta(0);
        continue;
      }
    }
    int delta_pv_pos_salvacao = delta_pontos_vida;
    bool salvou = false;
    if (acao_proto->permite_salvacao()) {
      std::string texto_salvacao;
      // pega o dano da acao.
      std::tie(delta_pv_pos_salvacao, salvou, texto_salvacao) = AtaqueVsSalvacao(acao_proto->delta_pontos_vida(), &da, *entidade_origem, *entidade_destino);
      std::unique_ptr<ntf::Notificacao> n(new ntf::Notificacao(PreencheNotificacaoExpiracaoEventoPosSalvacao(*entidade_destino)));
      if (n->has_tipo()) {
        *grupo_desfazer->add_notificacao() = *n;
        central_->AdicionaNotificacao(n.release());
      }
      atraso_s += 1.5f;
      ConcatenaString(texto_salvacao, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_origem->Id(), texto_salvacao);
    }
    // Imunidade ao tipo de ataque.
    ResultadoImunidadeOuResistencia resultado_elemento =
        ImunidadeOuResistenciaParaElemento(delta_pv_pos_salvacao, da, entidade_destino->Proto(), acao_proto->elemento());
    if (resultado_elemento.causa != ALT_NENHUMA) {
      delta_pv_pos_salvacao += resultado_elemento.resistido;
      atraso_s += 1.5f;
      ConcatenaString(resultado_elemento.texto, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_origem->Id(), resultado_elemento.texto);
      if (resultado_elemento.causa == ALT_IMUNIDADE || (resultado_elemento.causa == ALT_RESISTENCIA && delta_pv_pos_salvacao == 0)) {
        por_entidade->set_delta(0);
        continue;
      }
    }

    // Efeitos adicionais.
    // TODO: ver questao da reducao de dano e rm.
    atraso_s = AplicaEfeitosAdicionais(
        atraso_s, salvou, *entidade_origem, *entidade_destino, da,
        por_entidade, acao_proto, &ids_unicos_entidade_origem, &ids_unicos_entidade_destino, grupo_desfazer, central_);

    if (da.derruba_sem_teste() && !salvou && !entidade_destino->Proto().caida()) {
      acao_proto->set_consequencia(TC_DERRUBA_ALVO);
      por_entidade->set_forca_consequencia(true);
      // Apenas para desfazer, pois a consequencia derrubara.
      auto* nd = grupo_desfazer->add_notificacao();
      std::unique_ptr<ntf::Notificacao> n_derrubar(new ntf::Notificacao);
      PreencheNotificacaoDerrubar(*entidade_destino, n_derrubar.get(), nd);
      central_->AdicionaNotificacao(n_derrubar.release());
      ConcatenaString("derruba sem teste", por_entidade->mutable_texto());
    }

    delta_pv_pos_salvacao = CompartilhaDanoSeAplicavel(
        delta_pv_pos_salvacao, entidade_destino->Proto(), *this, TD_LETAL, por_entidade, acao_proto, grupo_desfazer);

    acao_proto->set_bem_sucedida(true);
    por_entidade->set_id(id);
    por_entidade->set_delta(delta_pv_pos_salvacao);
    // Notificacao de desfazer.
    auto* nd = grupo_desfazer->add_notificacao();
    PreencheNotificacaoAtualizacaoPontosVida(*entidade_destino, delta_pv_pos_salvacao, TD_LETAL, nd, nd);
  }
  VLOG(2) << "Acao de area: " << acao_proto->ShortDebugString();
  *n->mutable_acao() = *acao_proto;
  return atraso_s;
}

float Tabuleiro::TrataAcaoCriacao(
    float atraso_s, const Posicao& pos_criacao, Entidade* entidade, AcaoProto* acao_proto,
    ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  VLOG(1) << "pos criacao: " << pos_criacao.DebugString();
  const auto& modelo_com_parametros = tabelas_.ModeloEntidade(acao_proto->id_modelo_entidade());
  if (modelo_com_parametros.id() != acao_proto->id_modelo_entidade()) {
    LOG(ERROR) << "Modelo de entidade invalido: " << acao_proto->id_modelo_entidade();
    return atraso_s;
  }
  // Verifica alcance.
  {
    const float alcance_m = entidade->AlcanceAtaqueMetros();
    const Posicao& pos_origem = entidade->PosicaoAcao();
    Vector3 va(pos_origem.x(), pos_origem.y(), pos_origem.z());
    Vector3 vd(pos_criacao.x(), pos_criacao.y(), pos_criacao.z());
    const float distancia_m = (va - vd).length();
    VLOG(1)
        << "distancia: " << distancia_m << ", em quadrados: " << (distancia_m * METROS_PARA_QUADRADOS)
        << ", alcance maximo_m: " << alcance_m << ", em quadrados: " << (alcance_m * METROS_PARA_QUADRADOS);
    if (distancia_m > alcance_m) {
      AdicionaAcaoTextoLogado(entidade->Id(), StringPrintf("Fora de alcance: %0.1f m, maximo: %0.1f m", distancia_m, alcance_m));
      return atraso_s;
    }
  }

  // Consome ataque.
  const auto* da = entidade->DadoCorrente();
  {
    if (da != nullptr && (da->has_limite_vezes() || !da->taxa_refrescamento().empty())) {
      std::unique_ptr<ntf::Notificacao> n_consumo(new ntf::Notificacao);
      PreencheNotificacaoConsumoAtaque(*entidade, *da, n_consumo.get(), grupo_desfazer->add_notificacao());
      central_->AdicionaNotificacao(n_consumo.release());
    }
  }

  const auto* referencia = entidade;
  int quantidade = 1;
  if (!acao_proto->quantidade_entidades().empty()) {
    try {
      quantidade = RolaValor(acao_proto->quantidade_entidades());
      switch (acao_proto->modificador_quantidade()) {
        case MQ_NENHUM:
        break;
        case MQ_POR_NIVEL:
          quantidade *= NivelConjuradorParaAcao(*acao_proto, *entidade);
        break;
      }
    } catch (...) {
      quantidade = -1;
    }
    // colocar uns limites.
    if (quantidade < 0 || quantidade > 100) {
      LOG(ERROR) << "Quantidade invalida: " << acao_proto->quantidade_entidades() << " gerou: " << quantidade;
      quantidade = 1;
    }
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (int i = 0; i < quantidade; ++i) {
    ntf::Notificacao* filha;
    ent::EntidadeProto* e_antes, *modelo_entidade;
    std::tie(filha, e_antes, modelo_entidade) = NovaNotificacaoFilha(ntf::TN_ADICIONAR_ENTIDADE, EntidadeProto::default_instance(), &grupo_notificacoes);
    // A entidade nao tem id ainda.
    e_antes->clear_id();
    modelo_entidade->clear_id();
    *modelo_entidade = modelo_com_parametros.entidade();
    if (modelo_com_parametros.has_parametros()) {
      PreencheModeloComParametros(da == nullptr ? ArmaProto::default_instance()
                                                : tabelas_.Feitico(da->id_arma()),
                                  modelo_com_parametros.parametros(), *referencia, modelo_entidade);
    }
    Vector2 offset;
    if (i > 0) {
      offset = Vector2(cosf((i-1) * (M_PI / 3.0f)), sinf((i-1) * (M_PI / 3.0f)));
      offset *= TAMANHO_LADO_QUADRADO * (((i / 6) + 1));
    }
    *modelo_entidade->mutable_pos() = pos_criacao;
    modelo_entidade->mutable_pos()->set_x(pos_criacao.x() + offset.x);
    modelo_entidade->mutable_pos()->set_y(pos_criacao.y() + offset.y);
  }
  TrataNotificacao(grupo_notificacoes);
  if (ids_adicionados_.size() == static_cast<unsigned int>(grupo_notificacoes.notificacao_size())) {
    for (int i = 0; i < grupo_notificacoes.notificacao_size(); ++i) {
      grupo_notificacoes.mutable_notificacao(i)->mutable_entidade()->set_id(ids_adicionados_[i]);
      grupo_desfazer->add_notificacao()->Swap(grupo_notificacoes.mutable_notificacao(i));
    }
  } else {
    LOG(ERROR) << "ids_adicionados diferente do numero de notificacoes: " << ids_adicionados_.size() << " x " << grupo_notificacoes.notificacao_size() << ", quantidade era: " << quantidade << ", n: " << grupo_notificacoes.DebugString();
  }
  return atraso_s;
}

namespace {
// Passa para proximo ataque, atualiza em corpo a corpo etc.
void AtualizaAtaquesAposAtaqueIndividual(
    const DadosAtaque& da, Entidade* entidade_origem, Entidade* entidade_destino, ntf::Notificacao* grupo_desfazer) {
  if (entidade_origem == nullptr) return;

  ntf::Notificacao* filha;
  EntidadeProto *e_antes, *e_depois;
  std::tie(filha, e_antes, e_depois) =
      NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade_origem->Proto(), grupo_desfazer);
  e_antes->set_em_corpo_a_corpo(entidade_origem->Proto().em_corpo_a_corpo());
  if (da.acao().tipo() == ACAO_CORPO_A_CORPO) {
    e_depois->set_em_corpo_a_corpo(true);
    // Alvo vai pro CAC tb.
    if (entidade_destino != nullptr) {
      ntf::Notificacao* filha;
      EntidadeProto *e_antes, *e_depois;
      std::tie(filha, e_antes, e_depois) =
          NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, entidade_destino->Proto(), grupo_desfazer);
      e_antes->set_em_corpo_a_corpo(entidade_destino->Proto().em_corpo_a_corpo());
      e_depois->set_em_corpo_a_corpo(true);
      entidade_destino->AtualizaParcial(*e_depois);
    }
  } else {
    e_depois->set_em_corpo_a_corpo(false);
  }
  entidade_origem->AtualizaParcial(*e_depois);

  if ((!da.has_limite_vezes() || da.limite_vezes() == 1) && da.incrementa_proximo_ataque()) {
    // O refazer vai falhar, mas fodas.
    e_antes->set_reiniciar_ataque(true);
    e_depois->set_reiniciar_ataque(false);
    entidade_origem->ProximoAtaque();
  }
}

}  // namespace

float Tabuleiro::TrataAcaoIndividual(
    unsigned int id_entidade_destino, float atraso_s, const Posicao& pos_entidade_destino,
    Entidade* entidade_origem, AcaoProto* acao_proto, ntf::Notificacao* n, ntf::Notificacao* grupo_desfazer) {
  // Efeito individual.
  Entidade* entidade_destino =
     id_entidade_destino != Entidade::IdInvalido ? BuscaEntidade(id_entidade_destino) : nullptr;
  // Indica que a acao devera ser adicionada a notificacao no final (e fara o efeito grafico).
  acao_proto->set_bem_sucedida(true);
  std::vector<int> ids_unicos_entidade_destino = entidade_destino == nullptr ? std::vector<int>() : IdsUnicosEntidade(*entidade_destino);
  std::vector<int> ids_unicos_entidade_origem = entidade_origem == nullptr ? std::vector<int>() : IdsUnicosEntidade(*entidade_origem);
  if (entidade_destino == nullptr) {
    // Pode ser um projetil de area. Então retorna a acao como veio.
    *n->mutable_acao() = *acao_proto;
    return atraso_s;
  }

  if (HaValorListaPontosVida()) {
    // O valor default de posicao nao tem coordenadas, portanto a funcao usara o valor da posicao da entidade.
    auto pos_alvo = entidade_destino->Tipo() != TE_ENTIDADE || opcoes_.ataque_vs_defesa_posicao_real() ? pos_entidade_destino : Posicao();
    float distancia_m = 0.0f;
    // Verifica alcance.
    {
      bool realiza_acao;
      std::string texto_falha_alcance;
      std::tie(texto_falha_alcance, realiza_acao, distancia_m) =
          VerificaAlcanceMunicao(*acao_proto, *entidade_origem, *entidade_destino, pos_alvo);
      if (!realiza_acao) {
        AdicionaAcaoTextoLogado(entidade_origem->Id(), texto_falha_alcance);
        return atraso_s;
      }
    }
    // Verifica carregamento.
    const auto& da = DadoCorrenteOuPadrao(entidade_origem);
    if (!da.grupo().empty() && entidade_origem != nullptr) {
      acao_proto->set_grupo_ataque(da.grupo());
      acao_proto->set_indice_ataque(entidade_origem->IndiceAtaque());
    }
    if (da.requer_carregamento() && da.descarregada()) {
      const auto& arma = tabelas_.Arma(da.id_arma());
      std::unique_ptr<ntf::Notificacao> n_carregamento(new ntf::Notificacao);
      PreencheNotificacaoRecarregamento(*entidade_origem, da, n_carregamento.get(), grupo_desfazer->add_notificacao());
      AdicionaAcaoTextoLogado(
          entidade_origem->Id(),
          StringPrintf("recarregando (%s)", StringTipoCarregamento(arma.carregamento().tipo_carregamento()).c_str()));
      central_->AdicionaNotificacao(n_carregamento.release());
      return atraso_s;
    }

    // Para textos mesmo que de 0 pontos de vida.
    acao_proto->set_gera_outras_acoes(true);
    auto* por_entidade = acao_proto->add_por_entidade();
    por_entidade->set_id(entidade_destino->Id());
    por_entidade->set_delta(0);

    // Quantas vezes o ataque acertou. Por exemplo: 2 para dano duplo.
    // -1 indica falha critica.
    ResultadoAtaqueVsDefesa resultado;
    if (modo_dano_automatico_) {
      VLOG(1) << "--------------------------";
      VLOG(1) << "iniciando ataque vs defesa";
      resultado =
          AtaqueVsDefesa(distancia_m, *acao_proto, *entidade_origem, *entidade_destino, pos_alvo);
      VLOG(1) << "--------------------------";
      AdicionaLogEvento(entidade_origem->Id(), resultado.texto);
      por_entidade->set_texto(resultado.texto);
      acao_proto->set_bem_sucedida(resultado.Sucesso());
    } else {
      resultado.resultado = RA_SUCESSO;
      resultado.vezes = 1;
      acao_proto->set_bem_sucedida(true);
    }

    if (resultado.resultado == RA_FALHA_CRITICA || resultado.resultado == RA_SEM_ACAO) {
      if (resultado.resultado == RA_FALHA_CRITICA) {
        std::unique_ptr<ntf::Notificacao> n_derrubar(new ntf::Notificacao);
        PreencheNotificacaoDerrubar(*entidade_origem, n_derrubar.get(), grupo_desfazer->add_notificacao());
        central_->AdicionaNotificacao(n_derrubar.release());
      }
      AdicionaAcaoTexto(entidade_origem->Id(), resultado.texto, atraso_s);
      return atraso_s;
    }

    // Acao realizada, ao terminar funcao, roda isso.
    RodaNoRetorno roda_no_retorno([&da, entidade_origem, entidade_destino, grupo_desfazer]() {
      AtualizaAtaquesAposAtaqueIndividual(da, entidade_origem, entidade_destino, grupo_desfazer);
    });

    if (da.has_municao() || da.has_limite_vezes() || da.requer_carregamento() || !da.taxa_refrescamento().empty()) {
      // Consome vezes e/ou municao e carregamento.
      if (da.requer_carregamento()) {
        atraso_s += 0.5f;
        AdicionaAcaoTextoLogado(entidade_origem->Id(), "descarregada", atraso_s);
      }
      std::unique_ptr<ntf::Notificacao> n_consumo(new ntf::Notificacao);
      PreencheNotificacaoConsumoAtaque(*entidade_origem, da, n_consumo.get(), grupo_desfazer->add_notificacao());
      central_->AdicionaNotificacao(n_consumo.release());
    }

    if (resultado.resultado == RA_FALHA_REFLEXO) {
      // acao atingiu reflexo.
      std::unique_ptr<ntf::Notificacao> n_ref(new ntf::Notificacao);
      PreencheNotificacaoRemoverUmReflexo(*entidade_destino, n_ref.get(), grupo_desfazer->add_notificacao());
      central_->AdicionaNotificacao(n_ref.release());
      por_entidade->set_texto(resultado.texto);
      *n->mutable_acao() = *acao_proto;
      return atraso_s;
    }

    std::string texto_afeta;
    if (resultado.Sucesso() && !AcaoAfetaAlvo(*acao_proto, *entidade_destino, &texto_afeta)) {
      // Seta afeta pontos de vida para indicar que houve acerto, apesar da imunidade.
      por_entidade->set_texto(texto_afeta.empty() ? "ataque não afeta" : texto_afeta);
      *n->mutable_acao() = *acao_proto;
      return atraso_s;
    }

    // Aplica dano e critico, furtivo.
    int delta_pontos_vida = 0;
    const bool acao_cura = da.cura();
    if (resultado.Sucesso()) {
      int max_predileto = 0;
      for (const auto& ip : entidade_origem->Proto().dados_ataque_global().inimigos_prediletos()) {
        if (!acao_cura && entidade_destino->TemTipoDnD(ip.tipo()) && (!ip.has_sub_tipo() || entidade_destino->TemSubTipoDnD(ip.sub_tipo()))) {
          max_predileto = std::max(2 * ip.vezes(), max_predileto);
        }
      }
      if (max_predileto > 0) {
        max_predileto *= resultado.vezes;
        ConcatenaString(StringPrintf("inimigo predileto: %+d", max_predileto), por_entidade->mutable_texto());
        AdicionaLogEvento(entidade_origem->Id(), StringPrintf("acertou inimigo predileto: %+d de dano", max_predileto));
      }
      delta_pontos_vida -= max_predileto;
      for (int i = 0; i < resultado.vezes; ++i) {
        delta_pontos_vida += LeValorListaPontosVida(
            entidade_origem, entidade_destino->Proto(), acao_proto->id());
      }
      if (!entidade_destino->ImuneFurtivo() && !acao_cura) {
        if ((entidade_origem->Proto().dados_ataque_global().furtivo() || !DestrezaNaCA(entidade_destino->Proto()))
            && distancia_m <= (6 * QUADRADOS_PARA_METROS)) {
          int delta_furtivo = LeValorAtaqueFurtivo(entidade_origem);
          if (delta_furtivo < 0) {
            ConcatenaString(StringPrintf("furtivo: %+d", -delta_furtivo), por_entidade->mutable_texto());
            delta_pontos_vida += delta_furtivo;
          }
        }
      }
      if (acao_cura) {
        delta_pontos_vida = -delta_pontos_vida;
      }
    }

    // TODO: se o tipo de veneno for toque ou inalacao, deve ser aplicado.
    if (resultado.Sucesso() && da.has_veneno()) {
      std::string veneno_str;
      if (entidade_destino->ImuneVeneno()) {
        veneno_str = "Imune a veneno";
      } else {
        // A mesma notificacao pode gerar mais de um efeito, com ids unicos separados.
        std::unique_ptr<ntf::Notificacao> n_veneno(new ntf::Notificacao);
        const auto& veneno = da.veneno();
        // TODO permitir salvacao pre definida?
        int d20 = RolaDado(20);
        int bonus = entidade_destino->SalvacaoVeneno();
        int total = d20 + bonus;
        bool primario_aplicado = false;
        if (total < veneno.cd()) {
          // nao salvou: criar o efeito do dano.
          veneno_str = StringPrintf("não salvou veneno (%d + %d < %d)", d20, bonus, veneno.cd());
          if (!PossuiEvento(EFEITO_RETARDAR_ENVENENAMENTO, entidade_destino->Proto())) {
            primario_aplicado = true;
            PreencheNotificacaoEventoParaVenenoPrimario(
                entidade_destino->Id(), veneno, /*rodadas=*/DIA_EM_RODADAS, &ids_unicos_entidade_destino, n_veneno.get(), nullptr);
          }
        } else {
          veneno_str = StringPrintf("salvou veneno (%d + %d >= %d)", d20, bonus, veneno.cd());
          primario_aplicado = true;
        }
        // Aplica efeito de veneno: independente de salvacao. Apenas para marcar a entidade como envenenada.
        // O veneno vai serializado para quando acabar por passagem de rodadas, aplicar o secundario.
        {
          std::string veneno_proto_str;
          auto copia_veneno = veneno;
          copia_veneno.set_primario_aplicado(primario_aplicado);
          google::protobuf::TextFormat::PrintToString(copia_veneno, &veneno_proto_str);
          std::string origem = StringPrintf("%d", AchaIdUnicoEvento(ids_unicos_entidade_destino));
          PreencheNotificacaoEventoComComplementoStr(
              entidade_destino->Id(), origem, EFEITO_VENENO, veneno_proto_str, /*rodadas=*/primario_aplicado ? 10 : 1,
              &ids_unicos_entidade_destino, n_veneno.get(), grupo_desfazer->add_notificacao());
        }
        central_->AdicionaNotificacao(n_veneno.release());
      }
      atraso_s += 2.0f;
      ConcatenaString(veneno_str, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_destino->Id(), veneno_str);
    }
    // Doença.
    if (resultado.Sucesso() && da.has_doenca()) {
      std::string doenca_str;
      if (entidade_destino->ImuneDoenca()) {
        doenca_str = "Imune a doença";
      } else {
        std::unique_ptr<ntf::Notificacao> n_doenca(new ntf::Notificacao);
        // TODO permitir salvacao pre definida?
        const auto& doenca = da.doenca();
        int d20 = RolaDado(20);
        int bonus = entidade_destino->Salvacao(*entidade_origem, TS_FORTITUDE);
        int total = d20 + bonus;
        if (total < doenca.cd()) {
          doenca_str = StringPrintf("salvou doenca (%d + %d >= %d)", d20, bonus, doenca.cd());
        } else {
          doenca_str = StringPrintf("não salvou doenca (%d + %d < %d)", d20, bonus, doenca.cd());
          // Aplica efeito de doenca: independente de salvacao. Apenas para marcar a entidade como doente.
          // O doenca vai serializado para quando acabar periodo de incubacao, aplicar o efeito.
          auto copia_doenca = doenca;
          copia_doenca.set_passou_incubacao(false);
          std::string doenca_proto_str;
          google::protobuf::TextFormat::PrintToString(copia_doenca, &doenca_proto_str);
          std::string origem = StringPrintf("%d", AchaIdUnicoEvento(ids_unicos_entidade_destino));
          int rodadas = -1;
          try {
            rodadas = RolaValor(doenca.incubacao_dias()) * DIA_EM_RODADAS;
            PreencheNotificacaoEventoComComplementoStr(
                entidade_destino->Id(), origem, EFEITO_DOENCA, doenca_proto_str, rodadas,
                &ids_unicos_entidade_destino, n_doenca.get(), grupo_desfazer->add_notificacao());
            central_->AdicionaNotificacao(n_doenca.release());
          } catch (...) {
            LOG(ERROR) << "incubação mal formada: " << doenca.incubacao_dias();
          }
        }
      }
      atraso_s += 2.0f;
      ConcatenaString(doenca_str, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_destino->Id(), doenca_str);
    }

    if (resultado.Sucesso() && !acao_proto->ignora_resistencia_magia() &&
        entidade_destino->Proto().dados_defesa().resistencia_magia() > 0) {
      std::string resultado_rm;
      bool sucesso;
      std::tie(sucesso, resultado_rm) = AtaqueVsResistenciaMagia(tabelas_, da, *entidade_origem, *entidade_destino);
      if (!sucesso) {
        atraso_s += 0.5f;
        delta_pontos_vida = 0;
        por_entidade->set_delta(0);
        resultado.resultado = RA_FALHA_IMUNE;
      }
      ConcatenaString(resultado_rm, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_destino->Id(), resultado_rm);
    }

    std::string resultado_salvacao;
    bool salvou = false;
    if (resultado.Sucesso() && acao_proto->permite_salvacao() &&
        (delta_pontos_vida < 0 || !acao_proto->efeitos_adicionais().empty() ||
         ((da.derrubar_automatico() || da.derruba_sem_teste())))) {
      std::tie(delta_pontos_vida, salvou, resultado_salvacao) = AtaqueVsSalvacao(delta_pontos_vida, &da, *entidade_origem, *entidade_destino);
      std::unique_ptr<ntf::Notificacao> n(new ntf::Notificacao(PreencheNotificacaoExpiracaoEventoPosSalvacao(*entidade_destino)));
      if (n->has_tipo()) {
        *grupo_desfazer->add_notificacao() = *n;
        central_->AdicionaNotificacao(n.release());
      }
      // Corrige o valor.
      por_entidade->set_delta(delta_pontos_vida);
      ConcatenaString(resultado_salvacao, por_entidade->mutable_texto());
      AdicionaLogEvento(entidade_destino->Id(), resultado_salvacao);
    }

    // Reducao de dano.
    std::string texto_reducao;
    if (delta_pontos_vida < 0 && !IgnoraReducaoDano(&da, *acao_proto)) {
      google::protobuf::RepeatedField<int> descritores = da.descritores();
      ResultadoReducaoDano rrd = AlteraDeltaPontosVidaPorMelhorReducao(delta_pontos_vida, entidade_destino->Proto(), descritores);
      const int dano_absorvido = -(delta_pontos_vida - rrd.delta_pv);
      delta_pontos_vida = rrd.delta_pv;
      texto_reducao = rrd.texto;
      // Aqui poderia se verificar se a reducao de dano tem id_unico (vem de algum efeito) para poder diminui-la.
      // Exemplo: pele rochosa tem limite de reducao que consegue absorver.
      if (rrd.id_unico >= 0) {
        // Cria uma atualizacao do efeito.
        const auto* evento = AchaEvento(rrd.id_unico, entidade_destino->Proto());
        if (evento != nullptr && evento->id_efeito() == EFEITO_PELE_ROCHOSA && !evento->complementos().empty()) {
          LOG(INFO) << "dano absorvido" << dano_absorvido;
          std::unique_ptr<ntf::Notificacao> nefeito(new ntf::Notificacao);
          EntidadeProto *proto_antes, *proto_depois;
          std::tie(proto_antes, proto_depois) =
              ent::PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, *entidade_destino, nefeito.get());
          *proto_antes->add_evento() = *evento;
          auto* evento_depois = proto_depois->add_evento();
          *evento_depois = *evento;
          evento_depois->mutable_complementos()->Set(0, std::max(0, evento_depois->complementos(0) - dano_absorvido));
          *grupo_desfazer->add_notificacao() = *nefeito;
          central_->AdicionaNotificacao(nefeito.release());
        }
      }
      if (!texto_reducao.empty()) {
        atraso_s += 0.5f;
        ConcatenaString(texto_reducao, por_entidade->mutable_texto());
      }
      if (delta_pontos_vida == 0) {
        // Seta delta para indicar que houve acerto, apesar da imunidade/resistencia.
        por_entidade->set_delta(0);
        resultado.resultado = RA_FALHA_REDUCAO;  // dano reduzido para 0.
      }
    }

    // Efeitos adicionais.
    if (resultado.Sucesso()) {
      atraso_s = AplicaEfeitosAdicionais(
          atraso_s, salvou, *entidade_origem, *entidade_destino, da,
          por_entidade, acao_proto, &ids_unicos_entidade_origem, &ids_unicos_entidade_destino, grupo_desfazer, central_);
    }

    if (resultado.Sucesso() &&
        (da.derrubar_automatico() || da.derruba_sem_teste()) &&
        !entidade_destino->Proto().caida()) {
      ResultadoAtaqueVsDefesa resultado_derrubar;
      if (da.derruba_sem_teste()) {
        if (!salvou) {
          resultado_derrubar.resultado = RA_SUCESSO;
          resultado_derrubar.texto = "derruba sem teste";
        } else {
          resultado_derrubar.resultado = RA_FALHA_REFLEXO;
          resultado_derrubar.texto = "salvou";
        }
      } else {
        resultado_derrubar = AtaqueVsDefesaDerrubar(*entidade_origem, *entidade_destino);
      }
      if (resultado_derrubar.Sucesso()) {
        por_entidade->set_forca_consequencia(true);
        acao_proto->set_consequencia(TC_DERRUBA_ALVO);
        // Apenas para desfazer, pois a consequencia derrubara.
        auto* nd = grupo_desfazer->add_notificacao();
        PreencheNotificacaoDerrubar(*entidade_destino, nd, nd);
      }
      ConcatenaString(resultado_derrubar.texto, por_entidade->mutable_texto());
    }
    if (resultado.Sucesso() &&
        da.agarrar_aprimorado() &&
        entidade_origem != nullptr &&
        entidade_destino->Proto().tamanho() < entidade_origem->Proto().tamanho()) {
      // agarrar
      ResultadoAtaqueVsDefesa resultado_agarrar = AtaqueVsDefesaAgarrar(*entidade_origem, *entidade_destino);
      if (resultado_agarrar.Sucesso()) {
        por_entidade->set_forca_consequencia(true);
        acao_proto->set_consequencia(TC_AGARRA_ALVO);
        // Apenas para desfazer.
        auto* no = grupo_desfazer->add_notificacao();
        PreencheNotificacaoAgarrar(entidade_destino->Id(), *entidade_origem, no, no);
        auto* nd = grupo_desfazer->add_notificacao();
        PreencheNotificacaoAgarrar(entidade_origem->Id(), *entidade_destino, nd, nd);
      }
      ConcatenaString(resultado_agarrar.texto, por_entidade->mutable_texto());
    }

    // Resistencias e imunidades.
    ResultadoImunidadeOuResistencia resultado_elemento =
        ImunidadeOuResistenciaParaElemento(delta_pontos_vida, da, entidade_destino->Proto(), acao_proto->elemento());
    if (resultado_elemento.causa != ALT_NENHUMA) {
      delta_pontos_vida += resultado_elemento.resistido;
      ConcatenaString(resultado_elemento.texto, por_entidade->mutable_texto());
      if (delta_pontos_vida == 0) {
        // Seta delta para indicar que houve acerto, apesar da imunidade/resistencia.
        por_entidade->set_delta(0);
      }
    }

    bool nao_letal = da.nao_letal();

    delta_pontos_vida = DesviaObjetoSeAplicavel(
        tabelas_, delta_pontos_vida, *entidade_destino, da, this, por_entidade, grupo_desfazer);

    // Compartilhamento de dano.
    delta_pontos_vida = CompartilhaDanoSeAplicavel(
        delta_pontos_vida, entidade_destino->Proto(), *this, nao_letal ? TD_NAO_LETAL : TD_LETAL,
        por_entidade, acao_proto, grupo_desfazer);

    VLOG(1) << "delta pontos vida: " << delta_pontos_vida;
    acao_proto->set_nao_letal(nao_letal);

    AdicionaLogEvento(google::protobuf::StringPrintf(
          "entidade %s %s %d em entidade %s. Texto: '%s'",
          RotuloEntidade(entidade_origem).c_str(),
          delta_pontos_vida <= 0 ? "causou dano" : "curou",
          std::abs(delta_pontos_vida),
          RotuloEntidade(entidade_destino).c_str(),
          acao_proto->texto().c_str()));
    if (delta_pontos_vida != 0) {
      por_entidade->set_delta(delta_pontos_vida);
      acao_proto->set_afeta_pontos_vida(true);
      // Apenas para desfazer.
      auto* nd = grupo_desfazer->add_notificacao();
      PreencheNotificacaoAtualizacaoPontosVida(
          *entidade_destino, delta_pontos_vida, nao_letal ? TD_NAO_LETAL : TD_LETAL, nd, nd);
    }
    if (entidade_origem != nullptr &&
        delta_pontos_vida < 0 && std::abs(delta_pontos_vida) > entidade_destino->PontosVida() &&
        PossuiTalento("trespassar", entidade_origem->Proto())) {
      atraso_s += 1.0f;
      AdicionaAcaoTexto(entidade_origem->Id(), "trespassar", atraso_s);
    }
  }

  if (acao_proto->tipo() == ACAO_AGARRAR && acao_proto->bem_sucedida()) {
    // Se agarrou, desfaz aqui.
    auto* no = grupo_desfazer->add_notificacao();
    PreencheNotificacaoAgarrar(entidade_destino->Id(), *entidade_origem, no, no);
    auto* nd = grupo_desfazer->add_notificacao();
    PreencheNotificacaoAgarrar(entidade_origem->Id(), *entidade_destino, nd, nd);
  }

  VLOG(1) << "Acao individual: " << acao_proto->ShortDebugString();
  // Projetil de area usa isso para saber se a acao foi realizada ou nao. Caso mude, ver a funcao TrataAcaoProjetilArea.
  *n->mutable_acao() = *acao_proto;
  return atraso_s;
}

void Tabuleiro::AtualizaEsquivaAoAtacar(const Entidade& entidade_origem, unsigned int id_entidade_destino, ntf::Notificacao* grupo_desfazer) {
  if (!PossuiTalento("esquiva", entidade_origem.Proto())) return;
  const auto& dd = entidade_origem.Proto().dados_defesa();
  if (dd.has_entidade_esquiva()) {
    const auto* entidade_esquiva_corrente = BuscaEntidade(dd.entidade_esquiva());
    if (entidade_esquiva_corrente != nullptr && !entidade_esquiva_corrente->Morta() &&
      entidade_esquiva_corrente->IdCenario() == entidade_origem.IdCenario()) {
      // Entidade atual existe, esta viva e no mesmo cenario.
      return;
    }
  }

  const auto* entidade_destino = BuscaEntidade(id_entidade_destino);
  if (entidade_destino != nullptr && entidade_destino->Tipo() == TE_ENTIDADE) {
    ntf::Notificacao n;
    PreencheNotificacaoEsquiva(id_entidade_destino, entidade_origem, &n, grupo_desfazer == nullptr ? nullptr : grupo_desfazer->add_notificacao());
    AdicionaLogEvento(entidade_origem.Id(), StringPrintf("esquivando de %s", RotuloEntidade(entidade_destino->Proto()).c_str()));
    TrataNotificacao(n);
  }
}

float Tabuleiro::TrataPreAcaoComum(
    float atraso_s, const Posicao& pos_tabuleiro, const Entidade& entidade_origem, unsigned int id_entidade_destino, AcaoProto* acao_proto,
    ntf::Notificacao* grupo_desfazer) {
  if (acao_proto->has_dado_pv_mais_alto()) {
    acao_proto->set_pv_mais_alto(RolaValor(acao_proto->dado_pv_mais_alto()));
  }
  bool pode_agir;
  std::string razao;
  std::tie(pode_agir, razao) = PodeAgir(entidade_origem.Proto());
  if (!pode_agir) {
    AdicionaAcaoTextoLogado(entidade_origem.Id(), StringPrintf("Entidade nao pode agir: %s", razao.c_str()), atraso_s);
    acao_proto->set_bem_sucedida(false);
    return atraso_s;
  }
  if (!acao_proto->has_tipo()) {
    AdicionaAcaoTextoLogado(entidade_origem.Id(), "Ação inválida: sem tipo", atraso_s);
    LOG(WARNING) << "acao sem tipo: " << acao_proto->DebugString();
    acao_proto->set_bem_sucedida(false);
    return atraso_s;
  }
  const auto& da = DadoCorrenteOuPadrao(entidade_origem);
  // Acao com cool down.
  if (da.disponivel_em() > 0) {
      AdicionaAcaoTextoLogado(entidade_origem.Id(), StringPrintf("Ação disponível em %d rodadas", da.disponivel_em()), atraso_s);
      acao_proto->set_bem_sucedida(false);
      return atraso_s;
  }
  // Pergaminhos...
  if (da.has_nivel_conjurador_pergaminho()) {
    // Testa a classe.
    bool pode_lancar;
    std::string texto;
    std::tie(pode_lancar, texto) = PodeLancarPergaminho(tabelas_, entidade_origem.Proto(), da);
    if (!pode_lancar) {
      AdicionaAcaoTextoLogado(entidade_origem.Id(), texto, atraso_s);
      acao_proto->set_bem_sucedida(false);
      return atraso_s;
    }
  }
  if (da.has_limite_vezes() && da.limite_vezes() == 0) {
    AdicionaAcaoTextoLogado(entidade_origem.Id(), "Ataque esgotado", atraso_s);
    acao_proto->set_bem_sucedida(false);
    return atraso_s;
  }
  if (da.requer_modelo_ativo() && !PossuiModeloAtivo(da.requer_modelo_ativo(), entidade_origem.Proto())) {
    AdicionaAcaoTextoLogado(entidade_origem.Id(), StringPrintf("Ação requer %s", tabelas_.EfeitoModelo(da.requer_modelo_ativo()).nome().c_str()), atraso_s);
    acao_proto->set_bem_sucedida(false);
    return atraso_s;
  }

  if (id_entidade_destino != Entidade::IdInvalido) {
    AtualizaEsquivaAoAtacar(entidade_origem, id_entidade_destino, grupo_desfazer);
  }
  acao_proto->set_bem_sucedida(true);
  acao_proto->set_atraso_s(atraso_s);
  *acao_proto->mutable_pos_tabuleiro() = pos_tabuleiro;
  acao_proto->set_id_entidade_origem(entidade_origem.Id());
  VLOG(1) << "acao proto: " << acao_proto->DebugString();
  return atraso_s;
}

float Tabuleiro::TrataAcaoUmaEntidade(
    Entidade* entidade_origem, const Posicao& pos_entidade_destino, const Posicao& pos_tabuleiro,
    unsigned int id_entidade_destino, float atraso_s, const AcaoProto* acao_preenchida) {

  std::unique_ptr<Entidade> e_falsa(NovaEntidadeFalsa(tabelas_));
  Entidade* entidade_origem_nao_null = entidade_origem == nullptr ? e_falsa.get() : entidade_origem;

  AcaoProto acao_proto = acao_preenchida == nullptr ? entidade_origem_nao_null->Acao() : *acao_preenchida;
  if (acao_proto.parametros_lancamento().parametros().size() >= 1) {
    // Escolhe o tipo de acao antes de agir.
    std::unique_ptr<ntf::Notificacao> n(new ntf::Notificacao);
    n->set_tipo(ntf::TN_ABRIR_DIALOGO_ESCOLHER_DECISAO_LANCAMENTO);
    *n->mutable_acao() = acao_proto;
    *n->mutable_acao()->mutable_pos_tabuleiro() = pos_tabuleiro;
    *n->mutable_acao()->mutable_pos_entidade() = pos_entidade_destino;
    if (entidade_origem != nullptr) {
      n->mutable_acao()->set_id_entidade_origem(entidade_origem->Id());
    }
    if (id_entidade_destino != Entidade::IdInvalido) {
      n->mutable_acao()->set_id_entidade_destino(id_entidade_destino);
    }
    central_->AdicionaNotificacao(n.release());
    return atraso_s;
  }

  ntf::Notificacao grupo_desfazer;
  grupo_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  atraso_s = TrataPreAcaoComum(
      atraso_s, pos_tabuleiro, *entidade_origem_nao_null, id_entidade_destino, &acao_proto, &grupo_desfazer);

  if (acao_proto.bem_sucedida()) {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_ADICIONAR_ACAO);
    if (acao_proto.tipo() == ACAO_EXPULSAR_FASCINAR_MORTOS_VIVOS) {
      atraso_s = TrataAcaoExpulsarFascinarMortosVivos(atraso_s, entidade_origem, &acao_proto, &n, &grupo_desfazer);
    } else if (acao_proto.tipo() == ACAO_CRIACAO_ENTIDADE) {
      atraso_s = TrataAcaoCriacao(atraso_s, pos_tabuleiro, entidade_origem, &acao_proto, &n, &grupo_desfazer);
    } else if (acao_proto.efeito_projetil_area()) {
      atraso_s = TrataAcaoProjetilArea(id_entidade_destino, atraso_s, pos_entidade_destino, entidade_origem, &acao_proto, &n, &grupo_desfazer);
    } else if (EfeitoArea(acao_proto)) {
      atraso_s = TrataAcaoEfeitoArea(atraso_s, pos_entidade_destino, entidade_origem, &acao_proto, &n, &grupo_desfazer);
    } else {
      atraso_s = TrataAcaoIndividual(id_entidade_destino, atraso_s, pos_entidade_destino, entidade_origem, &acao_proto, &n, &grupo_desfazer);
    }
    if (n.has_acao()) {
      TrataNotificacao(n);
    }
  }
  // TODO fazer um TrataPosAcaoComum, para consumir municao, atualizar ataques etc.

  // Mesmo nao havendo acao, tem que adicionar a lista de desfazer porque ha efeitos que independem disso.
  // Exemplo: o ataque foi falha critica, gerando uma queda.
  if (!grupo_desfazer.notificacao().empty()) {
    AdicionaNotificacaoListaEventos(grupo_desfazer);
  }
  return atraso_s;
}

void Tabuleiro::TrataBotaoEsquivaPressionadoPosPicking(unsigned int id, unsigned int tipo_objeto) {
  modo_clique_ = MODO_NORMAL;
  if (tipo_objeto != OBJ_ENTIDADE) {
    LOG(INFO) << "Tipo invalido para esquiva";
    return;
  }
  const auto* entidade_atacante = BuscaEntidade(id);
  if (entidade_atacante == nullptr || entidade_atacante->Tipo() != TE_ENTIDADE) {
    LOG(INFO) << "Atacante nullptr ou tipo invalido";
    return;
  }
  const auto* entidade_defensora = EntidadePrimeiraPessoaOuSelecionada();
  if (entidade_defensora == nullptr) {
    LOG(INFO) << "Defesor nullptr";
    return;
  }
  auto n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
  n->mutable_entidade_antes()->set_id(entidade_defensora->Id());
  n->mutable_entidade_antes()->mutable_dados_defesa()->set_entidade_esquiva(
      entidade_defensora->Proto().dados_defesa().has_entidade_esquiva()
      ? entidade_defensora->Proto().dados_defesa().entidade_esquiva()
      : Entidade::IdInvalido);
  n->mutable_entidade()->set_id(entidade_defensora->Id());
  n->mutable_entidade()->mutable_dados_defesa()->set_entidade_esquiva(id);
  AdicionaNotificacaoListaEventos(*n);
  central_->AdicionaNotificacao(n.release());
  AdicionaAcaoTexto(
      entidade_defensora->Id(),
      google::protobuf::StringPrintf("esquivando de %s", entidade_atacante->Proto().rotulo().empty()
                                                         ? net::to_string(id).c_str()
                                                         : entidade_atacante->Proto().rotulo().c_str()));
}

void Tabuleiro::TrataAcaoSinalizacao(unsigned int id_entidade_destino, const Posicao& pos_tabuleiro) {
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
    acao_proto.add_por_entidade()->set_id(id_entidade_destino);
  }
  acao_proto.mutable_pos_tabuleiro()->CopyFrom(pos_tabuleiro);
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ADICIONAR_ACAO);
  n.mutable_acao()->Swap(&acao_proto);
  TrataNotificacao(n);
}

void Tabuleiro::TrataBotaoAcaoPressionadoPosPicking(
    bool acao_padrao, int x, int y, unsigned int id, unsigned int tipo_objeto, float profundidade) {
  if ((tipo_objeto != OBJ_TABULEIRO) && (tipo_objeto != OBJ_ENTIDADE) && (tipo_objeto != OBJ_ENTIDADE_LISTA)) {
    // invalido.
    return;
  }
  // Primeiro, entidades.
  unsigned int id_entidade_destino = Entidade::IdInvalido;
  Posicao pos_entidade_destino;
  Posicao pos_tabuleiro;
  if (tipo_objeto == OBJ_ENTIDADE || tipo_objeto == OBJ_ENTIDADE_LISTA) {
    VLOG(1) << "Acao em entidade: " << id;
    // Entidade.
    id_entidade_destino = id;
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    pos_entidade_destino.set_x(x3d);
    pos_entidade_destino.set_y(y3d);
    pos_entidade_destino.set_z(z3d);
    pos_entidade_destino.set_id_cenario(IdCenario());
    // Depois tabuleiro.
    pos_tabuleiro.set_x(x3d);
    pos_tabuleiro.set_y(y3d);
    pos_tabuleiro.set_z(z3d);
    pos_tabuleiro.set_id_cenario(IdCenario());
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
    pos_tabuleiro.set_id_cenario(IdCenario());
  }

  // Executa a acao: se nao houver ninguem selecionado, faz sinalizacao. Se houver, ha dois modos de execucao:
  // - Efeito de area
  // - Efeito individual.
  std::vector<unsigned int> ids_origem;
  ids_origem = IdsPrimeiraPessoaOuEntidadesSelecionadas();
  if (acao_padrao || ids_origem.size() == 0) {
    TrataAcaoSinalizacao(id_entidade_destino, pos_tabuleiro);
  } else {
    // Realiza a acao de cada entidade contra o alvo ou local.
    // Usa modelo selecionado.
    VLOG(1) << "Acao de entidades.";
    float atraso_s = 0.0f;
    for (auto id_selecionado : ids_origem) {
      Entidade* entidade = BuscaEntidade(id_selecionado);
      if (entidade == nullptr || entidade->Tipo() != TE_ENTIDADE) {
        continue;
      }
      atraso_s = TrataAcaoUmaEntidade(entidade, pos_entidade_destino, pos_tabuleiro, id_entidade_destino, atraso_s);
    }
  }

  // Atualiza as acoes executadas da entidade se houver apenas uma. A sinalizacao nao eh adicionada a entidade porque ela possui forma propria.
  auto* e = EntidadeSelecionada();
  if (e == nullptr) {
    return;
  }
  AcaoProto acao_executada = e->Acao();
  if (!acao_executada.has_tipo() || acao_executada.tipo() == ACAO_SINALIZACAO || acao_executada.id().empty()) {
    return;
  }
  e->AdicionaAcaoExecutada(acao_executada.id());
  if (!EmModoMestre() && IdCameraPresa() == e->Id()) {
    // Envia para o mestre as lista de acoes executadas da entidade.
    auto n = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL);
    n->set_servidor_apenas(true);
    auto* entidade  = n->mutable_entidade();
    entidade->set_id(e->Id());
    entidade->mutable_lista_acoes()->CopyFrom(e->Proto().lista_acoes());
    central_->AdicionaNotificacaoRemota(n.release());
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

void Tabuleiro::TrataBotaoRemocaoGrupoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto) {
  modo_clique_ = MODO_NORMAL;
  if (tipo_objeto != OBJ_ENTIDADE) {
    LOG(INFO) << "Remocao de entidade valida apenas para objetos compostos.";
    return;
  }
  Entidade* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    LOG(ERROR) << "Entidade " << id << " nao encontrada";
    return;
  }
  if (entidade->Tipo() != TE_COMPOSTA) {
    LOG(INFO) << "Entidade " << id << " nao é composta";
    return;
  }
  parametros_desenho_.set_desenha_objeto_desmembrado(id);
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  if (tipo_objeto != OBJ_ENTIDADE) {
    LOG(ERROR) << "tipo invalido: " << tipo_objeto;
    return;
  }
  auto proto_antes = entidade->Proto();
  auto sub_forma = entidade->RemoveSubForma(id);

  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  {
    auto* notificacao = grupo_notificacoes.add_notificacao();
    notificacao->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
    *notificacao->mutable_entidade() = entidade->Proto();
    *notificacao->mutable_entidade_antes() = proto_antes;
  }
  {
    // Se mudar a ordem aqui, mudar embaixo depois do Trata tb.
    auto* notificacao = grupo_notificacoes.add_notificacao();
    notificacao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    notificacao->mutable_entidade()->Swap(&sub_forma);
  }
  TrataNotificacao(grupo_notificacoes);
  if (ids_adicionados_.size() == 1) {
    grupo_notificacoes.mutable_notificacao(1)->mutable_entidade()->set_id(ids_adicionados_[0]);
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}

void Tabuleiro::TrataBotaoMontariaPressionadoPosPicking(unsigned int id, unsigned int tipo_objeto) {
  modo_clique_ = MODO_NORMAL;
  const auto ids = IdsEntidadesSelecionadasOuPrimeiraPessoa();
  if (ids.empty()) {
    LOG(ERROR) << "Nao ha entidades selecionadas.";
    return;
  }
  std::unordered_set<unsigned int> ids_montarias;
  // Montadores.
  std::vector<const Entidade*> montadores;
  for (auto id : ids) {
    const auto* montador = BuscaEntidade(id);
    if (montador == nullptr) continue;
    montadores.push_back(montador);
  }

  const Entidade* montaria = tipo_objeto == OBJ_ENTIDADE ? BuscaEntidade(id) : nullptr;
  ntf::Notificacao grupo;
  grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  if (montaria == nullptr) {
    PreencheNotificacoesDesmontar(montadores, &grupo);
  } else {
    PreencheNotificacoesMontarEm(montadores, montaria, &grupo);
  }
  TrataNotificacao(grupo);
  AdicionaNotificacaoListaEventos(grupo);
}

void Tabuleiro::TrataBotaoTransicaoPressionadoPosPicking(int x, int y, unsigned int id, unsigned int tipo_objeto) {
  if (tipo_objeto != OBJ_ENTIDADE && tipo_objeto != OBJ_ENTIDADE_LISTA) {
    LOG(ERROR) << "Apenas entidades podem servir de transicao, tipo: '" << tipo_objeto << "'";
    return;
  }
  Entidade* doador = BuscaEntidade(id);
  if (doador == nullptr) {
    LOG(ERROR) << "Entidade " << id << " nao encontrada";
    return;
  }
  if (doador->Tipo() == TE_ENTIDADE || doador->TipoTransicao() == EntidadeProto::TRANS_TESOURO) {
    LOG(INFO) << "Transicao de tesouro";
    if (doador->Tipo() == TE_ENTIDADE && !doador->Morta()) {
      // invalido.
      LOG(INFO) << "Transicao de tesouro so funciona em entidades mortas";
      return;
    }
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
    if (receptor->Id() == doador->Id()) {
      LOG(INFO) << "Receptor tem que ser diferente";
      return;
    }
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    PreencheNotificacoesTransicaoTesouro(tabelas_, *doador, *receptor, &n, /*n_desfazer=*/nullptr);
    TrataNotificacao(n);
    AdicionaNotificacaoListaEventos(n);
    return;
  }

  if (tipo_objeto != OBJ_ENTIDADE) {
    // invalido.
    LOG(INFO) << "Transicao de cenario so funciona em entidades";
    return;
  }
  // Neste contexto, o nome faz mais sentido.
  auto* entidade_transicao = doador;
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
  auto n = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  auto* a = n->mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_local_apenas(true);
  auto* pd = a->mutable_pos_entidade();
  pd->set_x(x3d);
  pd->set_y(y3d);
  pd->set_z(z3d);
  pd->set_id_cenario(entidade->IdCenario());
  a->set_texto(texto);
  central_->AdicionaNotificacao(n.release());
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
      auto n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      e->set_id(id_ent.first);
      auto* destino = e->mutable_destino();
      destino->set_x(entidade->X() + delta_x);
      destino->set_y(entidade->Y() + delta_y);
      destino->set_z(entidade->Z());
      // Posicao final para desfazer.
      n_desfazer->mutable_entidade()->mutable_destino()->CopyFrom(*destino);
      central_->AdicionaNotificacaoRemota(n.release());
    }
  }
  AdicionaNotificacaoListaEventos(g_desfazer);
}

void Tabuleiro::TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao) {
  ultimo_x_ = x;
  ultimo_y_ = y;

  ConfiguraParametrosDesenho(EntidadeCameraPresaOuSelecionada(), modo_clique_, &parametros_desenho_);
  unsigned int id, tipo_objeto;
  float profundidade;
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
        auto n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
        *n->mutable_entidade() = entidade->Proto();
        n->mutable_entidade()->set_tipo_transicao(EntidadeProto::TRANS_CENARIO);
        auto* trans = n->mutable_entidade()->mutable_transicao_cenario();
        trans->set_x(x3d);
        trans->set_y(y3d);
        trans->set_z(z3d);
        trans->set_id_cenario(IdCenario());
        central_->AdicionaNotificacao(n.release());
        break;
      }
      case MODO_DOACAO: {
        if (tipo_objeto != OBJ_ENTIDADE && tipo_objeto != OBJ_ENTIDADE_LISTA) {
          break;
        }
        const auto* receptor = BuscaEntidade(id);
        if (receptor == nullptr) {
          LOG(INFO) << "Entidade receptora nao existe mais.";
          break;
        }
        if (receptor->Id() == notificacao_doacao_.entidade().id()) {
          LOG(INFO) << "Nao se pode doar para si mesmo.";
          break;
        }
        const auto* doador = BuscaEntidade(notificacao_doacao_.entidade().id());
        if (doador == nullptr) {
          LOG(INFO) << "Doador nao existe mais.";
          break;
        }
        ntf::Notificacao grupo_notificacoes;
        grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
        PreencheNotificacoesDoacaoParcialTesouro(
            tabelas_, notificacao_doacao_, doador->Proto(), receptor->Proto(), &grupo_notificacoes, &grupo_notificacoes);
        TrataNotificacao(grupo_notificacoes);
        AdicionaNotificacaoListaEventos(grupo_notificacoes);
        break;
      }
      case MODO_ACAO:
        TrataBotaoAcaoPressionadoPosPicking(false, x, y, id, tipo_objeto, profundidade);
        if (!lista_pontos_vida_.empty() && !modo_dano_automatico_) {
          return;  // Mantem o MODO_ACAO.
        }
        break;
      case MODO_ESQUIVA:
        TrataBotaoEsquivaPressionadoPosPicking(id, tipo_objeto);
        return;
      case MODO_TERRENO:
        TrataBotaoTerrenoPressionadoPosPicking(x3d, y3d, z3d);
        // Nao quero voltar para o modo normal.
        return;
      case MODO_REMOCAO_DE_GRUPO:
        TrataBotaoRemocaoGrupoPressionadoPosPicking(x, y, id, tipo_objeto);
        return;
      case MODO_MONTAR:
        TrataBotaoMontariaPressionadoPosPicking(id, tipo_objeto);
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
      case MODO_ROLA_DADO:
        TrataBotaoRolaDadoPressionadoPosPicking(x3d, y3d, z3d);
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
      for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
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
  float x3d, y3d, z3d;
  if (camera_ != CAMERA_PRIMEIRA_PESSOA) {
    parametros_desenho_.set_offset_terreno(olho_.alvo().z());
  }
  MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d);
  //MousePara3d(x, y, &x3d, &y3d, &z3d);
  VLOG(1) << "Botao direito pressionado: x3d: " << x3d << ", y3d: " << y3d << ", z3d: " << z3d;
  primeiro_x_ = ultimo_x_ = x;
  primeiro_y_ = ultimo_y_ = y;
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  if (estado_ != ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA) {
    estado_anterior_ = estado_;
    estado_ = ETAB_DESLIZANDO;
    primeiro_z_3d_ = z3d;
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
    for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
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

void Tabuleiro::TrataDuploCliqueEsquerdo(int x, int y, bool forcar) {
  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha == OBJ_TABULEIRO) {
    id = ultima_entidade_selecionada_;
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
    notificacao.set_forcado(forcar);
    TrataNotificacao(notificacao);
  } else if (pos_pilha == OBJ_ENTIDADE || pos_pilha == OBJ_ENTIDADE_LISTA) {
    // Entidade.
    if (SelecionaEntidade(id, true  /*forcar_fixa*/)) {
      auto n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
      n->set_modo_mestre(EmModoMestreIncluindoSecundario());
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      central_->AdicionaNotificacao(n.release());
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
      pos_depois->set_id_cenario(entidade->Pos().id_cenario());
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
  const std::vector<unsigned int> ids_colisao = IdsPrimeiraPessoaOuEntidadesSelecionadasMontadas();
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
  for (unsigned int id : ids_colisao) {
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
    for (unsigned int id : IdsEntidadesSelecionadasEMontadasOuPrimeiraPessoa()) {
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
      e_antes->set_apoiada(entidade_selecionada->Apoiada());
      // Altera a translacao em Z.
      //entidade_selecionada->IncrementaZ(delta * TAMANHO_LADO_QUADRADO);
      e->mutable_destino()->CopyFrom(entidade_selecionada->Pos());
      e->mutable_destino()->set_z(e->destino().z() + delta * TAMANHO_LADO_QUADRADO);
      const Posicao& pos = e->destino();
      float altura_olho = entidade_selecionada->AlturaOlho();
      e->set_apoiada(Apoiado(pos.x(), pos.y(), pos.z() + altura_olho, altura_olho));
    }
    // Nop mas envia para os clientes.
    TrataNotificacao(grupo_notificacoes);
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}

void Tabuleiro::DesagarraEntidadesSelecionadasNotificando() {
  VLOG(1) << "desgarrando";
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* e = BuscaEntidade(id);
    if (e == nullptr) continue;
    std::unique_ptr<ntf::Notificacao> grupo(ntf::NovaNotificacao(ntf::TN_GRUPO_NOTIFICACOES));
    ntf::Notificacao grupo_desfazer;
    grupo_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    for (const auto& id_alvo : e->Proto().agarrado_a()) {
      VLOG(1) << "desgarrando " << e->Id() << " de " << id_alvo;
      auto* ealvo = BuscaEntidade(id_alvo);
      if (ealvo == nullptr) continue;
      if (modo_dano_automatico_) {
        ResultadoAtaqueVsDefesa resultado;
        AcaoProto acao_proto;
        acao_proto.set_id("Agarrar");
        acao_proto.set_tipo(ACAO_AGARRAR);
        resultado = AtaqueVsDefesa(0.1/*distancia*/, acao_proto, *e, e->DadoAgarrar(), *ealvo, ealvo->Pos());
        resultado.texto = "desagarrar: " + resultado.texto;
        AdicionaAcaoTextoLogado(e->Id(), resultado.texto, 0.0f  /*atraso*/);
        if (!resultado.Sucesso()) continue;
      }
      PreencheNotificacaoDesagarrar(id_alvo, *e, grupo->add_notificacao(), grupo_desfazer.add_notificacao());
      PreencheNotificacaoDesagarrar(e->Id(), *ealvo, grupo->add_notificacao(), grupo_desfazer.add_notificacao());
      VLOG(1) << "desgarrando " << ealvo->Id() << " de " << e->Id();
    }

    if (!grupo->notificacao().empty()) {
      TrataNotificacao(*grupo);
      central_->AdicionaNotificacaoRemota(grupo.release());
      AdicionaNotificacaoListaEventos(grupo_desfazer);
    }
  }
}

void Tabuleiro::DesligaEsquivaNotificando() {
  auto* e = EntidadePrimeiraPessoaOuSelecionada();
  if (e == nullptr) {
    LOG(INFO) << "Nao ha entidade para usar feitico";
    return;
  }
  if (!e->Proto().dados_defesa().has_entidade_esquiva()) {
    LOG(INFO) << "Entidade nao esta esquivando";
    return;
  }
  ntf::Notificacao n;
  PreencheNotificacaoEsquiva(Entidade::IdInvalido, *e, &n, nullptr);
  AdicionaLogEvento(e->Id(), StringPrintf("desligando esquiva"));
  TrataNotificacao(n);
  AdicionaNotificacaoListaEventos(n);
}

void Tabuleiro::DescansaPersonagemNotificando() {
  auto* e = EntidadePrimeiraPessoaOuSelecionada();
  if (e == nullptr) {
    LOG(INFO) << "Nao ha entidade para usar feitico";
    return;
  }
  const auto& proto = e->Proto();
  int nivel = Nivel(proto);
  auto n_grupo = ntf::NovaNotificacao(ntf::TN_GRUPO_NOTIFICACOES);
  const int kNumRodadas = 8 * 60 * 10;

  // Cura 1 PV por nivel.
  {
    ntf::Notificacao* n_cura;
    EntidadeProto* e_antes, *e_depois;
    std::tie(n_cura, e_antes, e_depois) =
        NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n_grupo.get());
    e_antes->set_pontos_vida(e->PontosVida());
    e_depois->set_pontos_vida(std::min(e->MaximoPontosVida(), e->PontosVida() + nivel));
    AdicionaAcaoDeltaPontosVidaSemAfetar(e->Id(), nivel, 0);
  }
  // Renova feiticos.
  {
    ntf::Notificacao* n_feitico;
    EntidadeProto *e_antes, *e_depois;
    std::tie(n_feitico, e_antes, e_depois) =
        NovaNotificacaoFilha(ntf::TN_ALTERAR_TODOS_FEITICOS_NOTIFICANDO, proto, n_grupo.get());
    *e_antes->mutable_feiticos_classes() = proto.feiticos_classes();
    *e_depois->mutable_feiticos_classes() = proto.feiticos_classes();
    RenovaFeiticos(e_depois);
  }
  // Atualiza taxas de refrescamento de ataques.
  {
    ntf::Notificacao* n_feitico;
    EntidadeProto *e_antes, *e_depois;
    std::tie(n_feitico, e_antes, e_depois) =
        NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n_grupo.get());
    *e_antes->mutable_dados_ataque() = proto.dados_ataque();
    *e_depois->mutable_dados_ataque() = proto.dados_ataque();
    for (auto& da : *e_depois->mutable_dados_ataque()) {
      if (da.requer_carregamento()) {
        da.set_descarregada(false);
      }
      if (da.has_disponivel_em() && da.disponivel_em() > 0) {
        // passaram-se 8 horas.
        int novo_de = std::max(da.disponivel_em() - kNumRodadas, 0);
        da.set_disponivel_em(novo_de);
        if (novo_de == 0 && da.has_limite_vezes_original()) {
          // Zerou o contador, volta pro limite original.
          da.set_limite_vezes(da.limite_vezes_original());
        }
      }
    }
  }
  // Efeitos.
  {
    ntf::Notificacao* n_feitico;
    EntidadeProto *e_antes, *e_depois;
    std::tie(n_feitico, e_antes, e_depois) =
        NovaNotificacaoFilha(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, proto, n_grupo.get());
    *e_antes->mutable_evento() = proto.evento();
    *e_depois->mutable_evento() = proto.evento();
    for (auto& evento : *e_depois->mutable_evento()) {
      if (!evento.continuo()) {
        evento.set_rodadas(std::max(-1, evento.rodadas() - kNumRodadas));
      }
    }
  }

  AdicionaNotificacaoListaEventos(*n_grupo);
  TrataNotificacao(*n_grupo);
}

void Tabuleiro::TrataBotaoUsarFeitico(bool conversao_espontanea, int nivel) {
  auto* e = EntidadePrimeiraPessoaOuSelecionada();
  if (e == nullptr) {
    LOG(INFO) << "Nao ha entidade para usar feitico";
    return;
  }
  // Encontra a classe para lancar magia.
  const auto& id_classe = ClasseFeiticoAtiva(e->Proto());
  central_->AdicionaNotificacao(NotificacaoEscolherFeitico(conversao_espontanea, id_classe, nivel, e->Proto()).release());
}

void Tabuleiro::TrataMudarClasseFeiticoAtiva() {
  auto* e = EntidadePrimeiraPessoaOuSelecionada();
  if (e == nullptr) {
    LOG(INFO) << "Nao ha entidade para usar feitico";
    return;
  }
  ntf::Notificacao n;
  EntidadeProto* e_antes, *e_depois;
  std::tie(e_antes, e_depois) =
      PreencheNotificacaoEntidade(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE_NOTIFICANDO_SE_LOCAL, *e, &n);
  e_antes->set_classe_feitico_ativa(e->Proto().classe_feitico_ativa());
  e_depois->set_classe_feitico_ativa(ProximaClasseFeiticoAtiva(e->Proto()));
  TrataNotificacao(n);
}

void Tabuleiro::TrataRolarPericiaNotificando(const std::string& id_pericia, const EntidadeProto& proto) {
  const auto& pericia = Pericia(id_pericia, proto);
  if (!pericia.has_id()) {
    LOG(ERROR) << "Personagem " << RotuloEntidade(proto) << " nao tem pericia " << id_pericia;
    return;
  }
  const auto& pericia_tabelada = tabelas_.Pericia(pericia.id());
  const bool treinado = pericia.pontos() > 0;
  std::string texto;
  if (treinado || pericia_tabelada.sem_treinamento()) {
    const int bonus = ent::BonusTotal(pericia.bonus());
    const int dado = ent::RolaDado(20);
    texto = google::protobuf::StringPrintf("%s: %d + %d = %d", pericia_tabelada.nome().c_str(), dado, bonus, dado + bonus);
  } else {
    texto = google::protobuf::StringPrintf("Pericia %s requer treinamento", pericia_tabelada.nome().c_str());
  }
  AdicionaAcaoTextoLogado(proto.id(), texto);
}

}  // namespace ent
