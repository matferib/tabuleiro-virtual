#if USAR_QT
#include <QApplication>
#include <QClipboard>
#include <google/protobuf/text_format.h>
#endif
#include <algorithm>
#include <boost/filesystem.hpp>
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

#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.pb.h"


namespace ent {

namespace {

/** campo de visao vertical em graus. */
const double CAMPO_VERTICAL_GRAUS = 60.0;

/** altura inicial do olho. */
const double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
const double OLHO_ALTURA_MAXIMA = 45.0;
/** altura minima do olho. */
const double OLHO_ALTURA_MINIMA = 1.5;

/** raio (distancia) inicial do olho. */
const double OLHO_RAIO_INICIAL = 20.0;
/** raio maximo do olho. */
const double OLHO_RAIO_MAXIMO = 40.0;
/** raio minimo do olho. */
const double OLHO_RAIO_MINIMO = 1.5;

/** sensibilidade da rodela do mouse. */
const double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
const double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** expessura da linha do tabuleiro. */
const float EXPESSURA_LINHA = 0.2f;
const float EXPESSURA_LINHA_2 = EXPESSURA_LINHA / 2.0f;
/** velocidade do olho. */
const float VELOCIDADE_POR_EIXO = 0.1f;  // deslocamento em cada eixo (x, y, z) por chamada de atualizacao.

/** tamanho maximo da lista de eventos para desfazer. */
const unsigned int TAMANHO_MAXIMO_LISTA = 10;

/** Distancia minima entre pontos no desenho livre. */
const float DELTA_MINIMO_DESENHO_LIVRE = 0.2;

/** A Translacao e a rotacao de objetos so ocorre depois que houver essa distancia de pixels percorrida pelo mouse. */
const int DELTA_MINIMO_TRANSLACAO_ROTACAO = 5;

/** Os clipping planes. Isso afeta diretamente a precisao do Z buffer. */
#if ZBUFFER_16_BITS
const double DISTANCIA_PLANO_CORTE_PROXIMO = 2.0f;
const double DISTANCIA_PLANO_CORTE_DISTANTE = 80.0f;
#else
const double DISTANCIA_PLANO_CORTE_PROXIMO = 1.0f;
const double DISTANCIA_PLANO_CORTE_DISTANTE = 160.0f;
#endif

const char* ID_ACAO_ATAQUE_CORPO_A_CORPO = "Ataque Corpo a Corpo";

// Retorna 0 se nao andou quadrado, 1 se andou no eixo x, 2 se andou no eixo y, 3 se andou em ambos.
int AndouQuadrado(const Posicao& p1, const Posicao& p2) {
  float dx = fabs(p1.x() - p2.x());
  float dy = fabs(p1.y() - p2.y());
  if (dx >= TAMANHO_LADO_QUADRADO && dy >= TAMANHO_LADO_QUADRADO) return 3;
  if (dx >= TAMANHO_LADO_QUADRADO) return 1;
  if (dy >= TAMANHO_LADO_QUADRADO) return 2;
  return 0;
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

// Retorna o quadrado da distancia horizontal entre pos1 e pos2.
float DistanciaHorizontalQuadrado(const Posicao& pos1, const Posicao& pos2) {
  float distancia = powf(pos1.x() - pos2.x(), 2) + powf(pos1.y() - pos2.y(), 2);
  VLOG(4) << "Distancia: " << distancia;
  return distancia;
}

float DistanciaQuadrado(const Posicao& pos1, const Posicao& pos2) {
  float distancia = powf(pos1.x() - pos2.x(), 2) + powf(pos1.y() - pos2.y(), 2) + powf(pos1.z() - pos2.z(), 2);
  VLOG(4) << "Distancia: " << distancia;
  return distancia;
}

const std::string StringEstado(ent::etab_t estado) {
  switch (estado) {
    case ent::ETAB_OCIOSO:
      return "ETAB_OCIOSO";
    case ent::ETAB_ROTACAO:
      return "ETAB_ROTACAO";
    case ent::ETAB_DESLIZANDO:
      return "ETAB_DESLIZANDO";
    case ent::ETAB_ENTS_PRESSIONADAS:
      return "ETAB_ENTS_PRESSIONADAS";
    case ent::ETAB_ENTS_TRANSLACAO_ROTACAO:
      return "ETAB_ENTS_TRANSLACAO_ROTACAO";
    case ent::ETAB_ENTS_ESCALA:
      return "ETAB_ENTS_ESCALA";
    case ent::ETAB_ENTS_SELECIONADAS:
      return "ETAB_ENTS_SELECIONADAS";
    case ent::ETAB_QUAD_PRESSIONADO:
      return "ETAB_QUAD_PRESSIONADO";
    case ent::ETAB_QUAD_SELECIONADO:
      return "ETAB_QUAD_SELECIONADO";
    case ent::ETAB_SELECIONANDO_ENTIDADES:
      return "ETAB_SELECIONANDO_ENTIDADES";
    case ent::ETAB_DESENHANDO:
      return "ETAB_DESENHANDO";
    default:
      return "DESCONHECIDO";
  }
}

// Retorna a string sem os caracteres UTF-8 para desenho OpenGL.
const std::string StringSemUtf8(const std::string& id_acao) {
  std::string ret(id_acao);
#if USAR_OPENGL_ES
  std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
#endif
  const static std::map<std::string, std::string> mapa = {
    { "á", "a" },
    { "ã", "a" },
    { "â", "a" },
    { "é", "e" },
    { "ê", "e" },
    { "í", "i" },
    { "ç", "c" },
    { "ô", "o" },
    { "ó", "o" },
    { "õ", "o" },
    { "Á", "A" },
    { "Â", "A" },
    { "É", "E" },
    { "Ê", "E" },
    { "Í", "I" },
    { "Ç", "C" },
    { "Ô", "O" },
    { "Ó", "O" },
  };
  for (const auto& it_mapa : mapa) {
    auto it = ret.find(it_mapa.first);
    while (it != std::string::npos) {
      ret.replace(it, it_mapa.first.size(), it_mapa.second);
      it = ret.find(it_mapa.first);
    }
  }
  return ret;
}

// O delta de pontos de vida afeta outros bits tambem.
void PreencheNotificacaoDeltaPontosVida(
    const Entidade& entidade, int delta_pontos_vida, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  entidade_depois->set_pontos_vida(entidade.PontosVida() + delta_pontos_vida);

  if (n_desfazer != nullptr) {
    n_desfazer->mutable_entidade()->CopyFrom(*entidade_depois);
    auto* entidade_antes = n_desfazer->mutable_entidade_antes();
    entidade_antes->set_id(entidade.Id());
    entidade_antes->set_pontos_vida(entidade.PontosVida());
    entidade_antes->set_morta(entidade.Proto().morta());
    entidade_antes->set_caida(entidade.Proto().caida());
    entidade_antes->set_voadora(entidade.Proto().voadora());
    entidade_antes->set_aura(entidade.Proto().aura());
    entidade_antes->mutable_pos()->CopyFrom(entidade.Pos());
    entidade_antes->mutable_direcao_queda()->CopyFrom(entidade.Proto().direcao_queda());
  }
}

}  // namespace.

Tabuleiro::Tabuleiro(const Texturas* texturas, ntf::CentralNotificacoes* central) :
    id_cliente_(0),
    proximo_id_cliente_(1),
    texturas_(texturas),
    central_(central),
    modo_mestre_(true) {
  central_->RegistraReceptor(this);

  // Modelos.
  auto* modelo_padrao = new EntidadeProto;  // padrao eh cone verde.
  modelo_padrao->mutable_cor()->set_g(1.0f);
  mapa_modelos_.insert(std::make_pair("Padrão", std::unique_ptr<EntidadeProto>(modelo_padrao)));
  modelo_selecionado_ = modelo_padrao;
  Modelos modelos;
  const std::string arquivos_modelos[] = { ARQUIVO_MODELOS, ARQUIVO_MODELOS_NAO_SRD };
  for (const std::string& nome_arquivo_modelo : arquivos_modelos) {
    try {
      arq::LeArquivoAsciiProto(arq::TIPO_DADOS, nome_arquivo_modelo, &modelos);
    } catch (const std::logic_error& erro) {
      LOG(ERROR) << erro.what();
    }
    for (const auto& m : modelos.modelo()) {
      mapa_modelos_.insert(std::make_pair(
            m.id(), std::unique_ptr<EntidadeProto>(new EntidadeProto(m.entidade()))));
    }
  }
  // Acoes.
  Acoes acoes;
  try {
    LeArquivoAsciiProto(arq::TIPO_DADOS, ARQUIVO_ACOES, &acoes);
  } catch (const std::logic_error& erro) {
    LOG(ERROR) << erro.what();
  }
  for (const auto& a : acoes.acao()) {
    auto* nova_acao = new AcaoProto(a);
    mapa_acoes_.insert(std::make_pair(a.id(), std::unique_ptr<AcaoProto>(nova_acao)));
    id_acoes_.push_back(a.id());
  }
  // Controle virtual.
  CarregaTexturasControleVirtual();

  opcoes_.set_desenha_controle_virtual(true);

  EstadoInicial();
#if USAR_WATCHDOG
  watchdog_.Inicia([this] () {
    LOG(ERROR) << "Estado do tabuleiro: " << StringEstado(estado_)
               << ", anterior_rotacao: " << StringEstado(estado_anterior_)
               << ", acoes.size() == " << acoes_.size()
               << ", ids_entidades_selecionadas_.size() == " << ids_entidades_selecionadas_.size()
               << ", entidades_.size() == " << entidades_.size()
               << ", rastros_movimento_.size() == " << rastros_movimento_.size()
               << ", translacoes_rotacoes_antes_.size() == " << translacoes_rotacoes_antes_.size()
               << ", lista_eventos_.size() == " << lista_eventos_.size()
               << ", processando_grupo_: " << processando_grupo_;

    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_SERIALIZAR_TABULEIRO);
    notificacao.set_endereco("tabuleiros_salvos/tabuleiro_watchdog.binproto");
    this->TrataNotificacao(notificacao);
  });
#endif
}

Tabuleiro::~Tabuleiro() {
  LiberaTextura();
  LiberaTexturasControleVirtual();
  if (nome_buffer_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_);
  }
  if (nome_buffer_indice_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_indice_);
  }
  if (nome_buffer_grade_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_grade_);
  }
  if (nome_buffer_indice_grade_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_indice_grade_);
  }
}

void Tabuleiro::LiberaTextura() {
  if (proto_.has_info_textura()) {
    VLOG(2) << "Liberando textura: " << proto_.info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->set_id(proto_.info_textura().id());
    central_->AdicionaNotificacao(nl);
  }
}

void Tabuleiro::EstadoInicial() {
  // Proto do tabuleiro.
  LiberaTextura();
  proto_.Clear();
  // Iluminacao ambiente inicial.
  proto_.mutable_luz_ambiente()->set_r(0.5f);
  proto_.mutable_luz_ambiente()->set_g(0.5f);
  proto_.mutable_luz_ambiente()->set_b(0.5f);
  // Iluminacao direcional inicial.
  proto_.mutable_luz_direcional()->mutable_cor()->set_r(0.5f);
  proto_.mutable_luz_direcional()->mutable_cor()->set_g(0.5f);
  proto_.mutable_luz_direcional()->mutable_cor()->set_b(0.5f);
  // Vinda de 45 graus leste.
  proto_.mutable_luz_direcional()->set_posicao_graus(0.0f);
  proto_.mutable_luz_direcional()->set_inclinacao_graus(45.0f);
  // Olho.
  ReiniciaCamera();

  // Valores iniciais.
  ultimo_x_ = ultimo_y_ = 0;
  ultimo_x_3d_ = ultimo_y_3d_ = ultimo_z_3d_ = 0;
  primeiro_x_3d_ = primeiro_y_3d_ = primeiro_z_3d_ = 0;
  // Mapa de entidades e acoes vazios.
  entidades_.clear();
  acoes_.clear();
  // Entidades selecionadas.
  ids_entidades_selecionadas_.clear();
  // Outras variaveis.
  id_entidade_detalhada_ = Entidade::IdInvalido;
  quadrado_selecionado_ = -1;
  estado_ = ETAB_OCIOSO;
  proximo_id_entidade_ = 0;
  processando_grupo_ = false;
  // Lista de eventos.
  lista_eventos_.clear();
  evento_corrente_ = lista_eventos_.end();
  ignorar_lista_eventos_ = false;
  // Desenho.
  forma_proto_.Clear();
  forma_selecionada_ = TF_CUBO;
  CorParaProto(COR_BRANCA, &forma_cor_);
  // Tempo renderizacao.
  tempos_renderizacao_.clear();
  // Modo de acao.
  modo_acao_ = false;
  if (gl_iniciado_) {
    RegeraVbo();
  }
}

void Tabuleiro::ConfiguraProjecao() {
  if (camera_isometrica_) {
    const Posicao& alvo = olho_.alvo();
    // o tamanho do vetor
    float dif_x = alvo.x() - olho_.pos().x();
    float dif_y = alvo.y() - olho_.pos().y();
    float fator_zoom = sqrt(dif_x * dif_x + dif_y * dif_y);
    const float tam_base = 3.0f;
    float largura = tam_base * Aspecto() * fator_zoom;
    float altura = tam_base * fator_zoom;
    gl::Ortogonal(-largura, largura, -altura, altura,
                  DISTANCIA_PLANO_CORTE_PROXIMO, DISTANCIA_PLANO_CORTE_DISTANTE);
  } else {
    gl::Perspectiva(CAMPO_VERTICAL_GRAUS, Aspecto(), DISTANCIA_PLANO_CORTE_PROXIMO, DISTANCIA_PLANO_CORTE_DISTANTE);
  }
}

void Tabuleiro::ConfiguraOlhar() {
  if (camera_presa_) {
    const auto* e = BuscaEntidade(id_camera_presa_);
    if (e == nullptr) {
      AlternaCameraPresa();
    } else {
      olho_.mutable_destino()->CopyFrom(e->Pos());
      AtualizaOlho(true  /*forcar*/);
    }
  }
  const Posicao& alvo = olho_.alvo();
  if (camera_isometrica_) {
    gl::OlharPara(
        // from.
        alvo.x(), alvo.y(), olho_.pos().z(),
        // to.
        alvo.x(), alvo.y(), alvo.z(),
        // up
        alvo.x() - olho_.pos().x(), alvo.y() - olho_.pos().y(), 0.0);
  } else {
    gl::OlharPara(
        // from.
        olho_.pos().x(), olho_.pos().y(), olho_.pos().z(),
        // to.
        alvo.x(), alvo.y(), alvo.z(),
        // up
        0, 0, 1.0);
  }
}

void Tabuleiro::Desenha() {
  // Varios lugares chamam desenha cena com parametros especifico. Essa funcao
  // desenha a cena padrao, entao ela restaura os parametros para seus valores
  // default. Alem disso a matriz de projecao eh diferente para picking.
  parametros_desenho_.Clear();
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  gl::ModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  ConfiguraProjecao();
  // Aplica opcoes do jogador.
  parametros_desenho_.set_desenha_fps(opcoes_.mostra_fps());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
  if (modo_debug_) {
    parametros_desenho_.set_iluminacao(false);
    parametros_desenho_.set_desenha_texturas(false);
    parametros_desenho_.set_desenha_grade(false);
    parametros_desenho_.set_desenha_fps(false);
    parametros_desenho_.set_desenha_aura(false);
    parametros_desenho_.set_desenha_sombras(false);
    parametros_desenho_.set_limpa_fundo(false);
    parametros_desenho_.set_transparencias(false);
    parametros_desenho_.set_desenha_acoes(false);
    parametros_desenho_.set_desenha_lista_pontos_vida(false);
    parametros_desenho_.set_desenha_quadrado_selecao(false);
    parametros_desenho_.set_desenha_rastro_movimento(false);
    parametros_desenho_.set_desenha_forma_selecionada(false);
    parametros_desenho_.set_desenha_rosa_dos_ventos(false);
    parametros_desenho_.set_desenha_nevoa(false);
  }
  DesenhaCena();
}

void Tabuleiro::AdicionaEntidadeNotificando(const ntf::Notificacao& notificacao) {
  try {
    if (notificacao.local()) {
      EntidadeProto modelo(notificacao.has_entidade() ? notificacao.entidade() : *modelo_selecionado_);
      if (modelo.tipo() == TE_FORMA && !modo_mestre_) {
        LOG(ERROR) << "Apenas o mestre pode adicionar formas.";
        return;
      }
      if (!notificacao.has_entidade()) {
        if (estado_ != ETAB_QUAD_SELECIONADO) {
          return;
        }
        // Notificacao sem entidade: posicao do quadrado selecionado.
        float x, y, z;
        CoordenadaQuadrado(quadrado_selecionado_, &x, &y, &z);
        modelo.mutable_pos()->set_x(x);
        modelo.mutable_pos()->set_y(y);
        modelo.mutable_pos()->set_z(z);
      }
      unsigned int id_entidade = GeraIdEntidade(id_cliente_);
      if (processando_grupo_) {
        ids_adicionados_.push_back(id_entidade);
      }
      // Visibilidade e selecionabilidade: se nao estiver desfazendo, usa o modo mestre para determinar
      // se a entidade eh visivel e selecionavel para os jogadores.
      if (!ignorar_lista_eventos_) {
        modelo.set_visivel(!modo_mestre_);
        modelo.set_selecionavel_para_jogador(!modo_mestre_);
        modelo.set_id(id_entidade);
      } else {
        if (BuscaEntidade(modelo.id()) != nullptr) {
          // Este caso eh raro, mas talvez possa acontecer quando estiver perto do limite de entidades.
          // Isso tem potencial de erro caso o mestre remova entidade de jogadores.
          throw std::logic_error("Id da entidade já está sendo usado.");
        }
      }
      auto* entidade = NovaEntidade(modelo, texturas_, central_);
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
      // Se a entidade selecionada for TE_ENTIDADE e a entidade adicionada for FORMA, deseleciona a entidade.
      for (const auto id : ids_entidades_selecionadas_) {
        auto* e_selecionada = BuscaEntidade(id);
        if (e_selecionada == nullptr) {
          continue;
        }
        if (e_selecionada->Tipo() == TE_ENTIDADE && entidade->Tipo() == TE_FORMA) {
          DeselecionaEntidades();
          break;
        }
      }
      AdicionaEntidadesSelecionadas({ entidade->Id() });
      {
        // Para desfazer.
        ntf::Notificacao n_desfazer(notificacao);
        n_desfazer.mutable_entidade()->CopyFrom(modelo);
        AdicionaNotificacaoListaEventos(n_desfazer);
      }
      // Envia a entidade para os outros.
      auto* n = ntf::NovaNotificacao(notificacao.tipo());
      n->mutable_entidade()->CopyFrom(entidade->Proto());
      central_->AdicionaNotificacaoRemota(n);
    } else {
      // Mensagem veio de fora.
      auto* entidade = NovaEntidade(notificacao.entidade(), texturas_, central_);
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
    }
  } catch (const std::logic_error& erro) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro(erro.what());
    central_->AdicionaNotificacao(n);
  }
}

void Tabuleiro::AtualizaBitsEntidadeNotificando(int bits, bool valor) {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* n = grupo_notificacoes.add_notificacao();
    auto* entidade_selecionada = BuscaEntidade(id);
    const auto& proto_original = entidade_selecionada->Proto();
    // Para desfazer.
    auto* proto_antes = n->mutable_entidade_antes();
    auto* proto_depois = n->mutable_entidade();
    if ((bits & BIT_VISIBILIDADE) > 0 && (modo_mestre_ || proto_original.selecionavel_para_jogador())) {
      // Apenas modo mestre ou para selecionaveis.
      proto_antes->set_visivel(proto_original.visivel());
      proto_depois->set_visivel(valor);
    }
    if ((bits & BIT_ILUMINACAO) > 0) {
      // Luz eh tricky pq nao eh um bit. Mas tem que setar pra um valor para mostrar que o ha atualizacao no campo.
      if (proto_original.has_luz()) {
        proto_antes->mutable_luz()->CopyFrom(proto_original.luz());
        if (valor) {
          proto_depois->mutable_luz()->CopyFrom(proto_original.luz());
        } else {
          auto* luz_depois = proto_depois->mutable_luz()->mutable_cor();
          luz_depois->set_r(0);
          luz_depois->set_g(0);
          luz_depois->set_b(0);
        }
      } else {
        auto* luz_antes = proto_antes->mutable_luz()->mutable_cor();
        luz_antes->set_r(0);
        luz_antes->set_g(0);
        luz_antes->set_b(0);
        if (valor) {
          auto* luz = proto_depois->mutable_luz()->mutable_cor();
          luz->set_r(1.0f);
          luz->set_g(1.0f);
          luz->set_b(1.0f);
        } else {
          proto_depois->mutable_luz()->CopyFrom(proto_antes->luz());
        }

      }
    }
    if ((bits & BIT_VOO) > 0) {
      proto_antes->set_voadora(proto_original.voadora());
      proto_depois->set_voadora(valor);
    }
    if (bits & BIT_CAIDA) {
      proto_antes->set_caida(proto_original.caida());
      proto_depois->set_caida(valor);
    }
    if (bits & BIT_MORTA) {
      proto_antes->set_morta(proto_original.morta());
      proto_depois->set_morta(valor);
    }
    if (bits & BIT_SELECIONAVEL) {
      proto_antes->set_selecionavel_para_jogador(proto_original.selecionavel_para_jogador());
      proto_depois->set_selecionavel_para_jogador(valor);
    }
    if (bits & BIT_FIXA) {
      proto_antes->set_fixa(proto_original.fixa());
      proto_depois->set_fixa(valor);
    }
    proto_antes->set_id(id);
    proto_depois->set_id(id);
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::AlternaBitsEntidadeNotificando(int bits) {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* n = grupo_notificacoes.add_notificacao();
    auto* entidade_selecionada = BuscaEntidade(id);
    const auto& proto_original = entidade_selecionada->Proto();
    // Para desfazer.
    auto* proto_antes = n->mutable_entidade_antes();
    auto* proto_depois = n->mutable_entidade();
    if ((bits & BIT_VISIBILIDADE) > 0 && (modo_mestre_ || proto_original.selecionavel_para_jogador())) {
      // Apenas modo mestre ou para selecionaveis.
      proto_antes->set_visivel(proto_original.visivel());
      proto_depois->set_visivel(!proto_original.visivel());
    }
    if ((bits & BIT_ILUMINACAO) > 0) {
      // Luz eh tricky pq nao eh um bit.
      if (proto_original.has_luz()) {
        proto_antes->mutable_luz()->CopyFrom(proto_original.luz());
        auto* luz_depois = proto_depois->mutable_luz()->mutable_cor();
        luz_depois->set_r(0);
        luz_depois->set_g(0);
        luz_depois->set_b(0);
      } else {
        auto* luz = proto_depois->mutable_luz()->mutable_cor();
        luz->set_r(1.0f);
        luz->set_g(1.0f);
        luz->set_b(1.0f);
        auto* luz_antes = proto_antes->mutable_luz()->mutable_cor();
        luz_antes->set_r(0);
        luz_antes->set_g(0);
        luz_antes->set_b(0);
      }
    }
    if ((bits & BIT_VOO) > 0) {
      proto_antes->set_voadora(proto_original.voadora());
      proto_depois->set_voadora(!proto_original.voadora());
    }
    if (bits & BIT_CAIDA) {
      proto_antes->set_caida(proto_original.caida());
      proto_depois->set_caida(!proto_original.caida());
    }
    if (bits & BIT_MORTA) {
      proto_antes->set_morta(proto_original.morta());
      proto_depois->set_morta(!proto_original.morta());
    }
    if (bits & BIT_SELECIONAVEL) {
      proto_antes->set_selecionavel_para_jogador(proto_original.selecionavel_para_jogador());
      proto_depois->set_selecionavel_para_jogador(!proto_original.selecionavel_para_jogador());
    }
    if (bits & BIT_FIXA) {
      proto_antes->set_fixa(proto_original.fixa());
      proto_depois->set_fixa(!proto_original.fixa());
    }
    proto_antes->set_id(id);
    proto_depois->set_id(id);
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::TrataAcaoAtualizarPontosVidaEntidades(int delta_pontos_vida) {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  // Para desfazer.
  ntf::Notificacao grupo_desfazer;
  grupo_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    // Atualizacao.
    PreencheNotificacaoDeltaPontosVida(*entidade_selecionada,
                                       delta_pontos_vida,
                                       grupo_notificacoes.add_notificacao(),
                                       grupo_desfazer.add_notificacao());
    // Acao.
    auto* na = grupo_notificacoes.add_notificacao();
    na->set_tipo(ntf::TN_ADICIONAR_ACAO);
    auto* a = na->mutable_acao();
    a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    a->add_id_entidade_destino(entidade_selecionada->Id());
    a->set_delta_pontos_vida(delta_pontos_vida);
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_desfazer);
}

void Tabuleiro::AtualizaParcialEntidadeNotificando(const ntf::Notificacao& notificacao) {
  if (!notificacao.entidade().has_id()) {
    LOG(ERROR) << "Notificacao de atualizacao parcial sem id: " << notificacao.ShortDebugString();
    return;
  }
  auto* entidade = BuscaEntidade(notificacao.entidade().id());
  if (entidade == nullptr) {
    VLOG(1) << "Entidade '" << notificacao.entidade().id() << "' invalida para notificacao de atualizacao parcial";
    return;
  }
  entidade->AtualizaParcial(notificacao.entidade());
  if (notificacao.local()) {
    auto* n_remota = new ntf::Notificacao(notificacao);
    central_->AdicionaNotificacaoRemota(n_remota);
  }
}

void Tabuleiro::AtualizaPontosVidaEntidadePorAcao(const Acao& acao, unsigned int id_entidade, int delta_pontos_vida) {
  auto* entidade = BuscaEntidade(id_entidade);
  if (entidade == nullptr) {
    LOG(WARNING) << "Entidade nao encontrada: " << id_entidade;
    return;
  }

  const auto& ap = acao.Proto();
  if (ap.permite_salvacao()) {
    if (entidade->ProximaSalvacao() == RS_MEIO) {
      delta_pontos_vida /= 2;
    } else if (entidade->ProximaSalvacao() == RS_QUARTO) {
      delta_pontos_vida /= 4;
    } else if (entidade->ProximaSalvacao() == RS_ANULOU) {
      delta_pontos_vida = 0;
    }
  }

  // Atualizacao de pontos de vida. Nao preocupa com desfazer porque isso foi feito no inicio da acao.
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  PreencheNotificacaoDeltaPontosVida(*entidade, delta_pontos_vida, &n);
  TrataNotificacao(n);

  // Acao de pontos de vida sem efeito.
  ntf::Notificacao na;
  na.set_tipo(ntf::TN_ADICIONAR_ACAO);
  auto* a = na.mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->add_id_entidade_destino(entidade->Id());
  a->set_delta_pontos_vida(delta_pontos_vida);
  a->set_afeta_pontos_vida(false);
  TrataNotificacao(na);
}

void Tabuleiro::AtualizaSalvacaoEntidadesSelecionadas(ResultadoSalvacao rs) {
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade = BuscaEntidade(id);
    if (entidade == nullptr) {
      continue;
    }
    auto* ntf = grupo_notificacoes.add_notificacao();
    ntf->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    auto* entidade_antes = ntf->mutable_entidade_antes();
    entidade_antes->set_id(entidade->Id());
    entidade_antes->set_proxima_salvacao(entidade->Proto().proxima_salvacao());
    auto* entidade_depois = ntf->mutable_entidade();
    entidade_depois->set_id(entidade->Id());
    entidade_depois->set_proxima_salvacao(rs);
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::AcumulaPontosVida(const std::vector<int>& lista_pv) {
  for (int pv : lista_pv) {
    if (pv >= 1000 || pv <= -1000) {
      LOG(ERROR) << "Ignorando pv: " << pv;
      continue;
    }
    lista_pontos_vida_.emplace_back(pv);
  }
}

void Tabuleiro::LimpaListaPontosVida() {
  lista_pontos_vida_.clear();
}

void Tabuleiro::AlteraUltimoPontoVidaListaPontosVida(int delta) {
  int valor = 0;
  if (!lista_pontos_vida_.empty()) {
    valor = lista_pontos_vida_.back();
    lista_pontos_vida_.pop_back();
  }
  lista_pontos_vida_.push_back(valor + delta);
}

void Tabuleiro::AlternaUltimoPontoVidaListaPontosVida() {
  if (!lista_pontos_vida_.empty()) {
    int valor = -lista_pontos_vida_.back();
    lista_pontos_vida_.pop_back();
    lista_pontos_vida_.push_back(valor);
    modo_acao_cura_ = valor >= 0;
  } else {
    modo_acao_cura_ = !modo_acao_cura_;
  }
}

void Tabuleiro::LimpaUltimoListaPontosVida() {
  if (!lista_pontos_vida_.empty()) {
    lista_pontos_vida_.pop_back();
  }
}

bool Tabuleiro::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_CONECTAR: {
      AlterarModoMestre(false);
      return true;
    }
    case ntf::TN_PASSAR_UMA_RODADA: {
      PassaUmaRodadaNotificando();
      break;
    }
    case ntf::TN_ATUALIZAR_RODADAS: {
      proto_.set_contador_rodadas(notificacao.tabuleiro().contador_rodadas());
      if (notificacao.local()) {
        auto* nr = ntf::NovaNotificacao(notificacao.tipo());
        nr->mutable_tabuleiro()->set_contador_rodadas(notificacao.tabuleiro().contador_rodadas());
        central_->AdicionaNotificacaoRemota(nr);
      }

      break;
    }
    case ntf::TN_DESCONECTADO: {
      if (ModoMestre()) {
        // cliente desconectado.
        for (auto it : clientes_) {
          if (it.second == notificacao.id()) {
            clientes_.erase(it.first);
            return true;
          }
        }
        LOG(ERROR) << "Nao encontrei cliente desconectado: '" << notificacao.id() << "'";
        return true;
      } else {
        if (notificacao.has_erro()) {
          auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
          n->set_erro(notificacao.erro());
          central_->AdicionaNotificacao(n);
        }
        return true;
      }
    }
    case ntf::TN_GRUPO_NOTIFICACOES:
      // Nunca deve vir da central.
      processando_grupo_ = true;
      ids_adicionados_.clear();
      for (const auto& n : notificacao.notificacao()) {
        TrataNotificacao(n);
      }
      processando_grupo_ = false;
      return true;
    case ntf::TN_REINICIAR_CAMERA:
      ReiniciaCamera();
      return true;
    case ntf::TN_SALVAR_CAMERA:
      SalvaCameraInicial();
      return true;
    case ntf::TN_RESPOSTA_CONEXAO:
      if (notificacao.local()) {
        if (!notificacao.has_erro()) {
          auto* ni = ntf::NovaNotificacao(ntf::TN_INFO);
          ni->set_erro(std::string("Conectado ao servidor"));
          central_->AdicionaNotificacao(ni);
        } else {
          AlterarModoMestre(true);  // volta modo mestre.
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro conectando ao servidor: ") + notificacao.erro());
          central_->AdicionaNotificacao(ne);
        }
      }
      return true;
    case ntf::TN_ADICIONAR_ENTIDADE:
      AdicionaEntidadeNotificando(notificacao);
      return true;
    case ntf::TN_ADICIONAR_ACAO: {
      auto* acao = NovaAcao(notificacao.acao(), this);
      if (acao == nullptr) {
        return true;
      }
      acoes_.push_back(std::unique_ptr<Acao>(acao));
      if (notificacao.local()) {
        auto* n_remota = new ntf::Notificacao(notificacao);
        n_remota->mutable_acao()->clear_afeta_pontos_vida();
        central_->AdicionaNotificacaoRemota(n_remota);
      }
      return true;
    }
    case ntf::TN_REMOVER_ENTIDADE: {
      RemoveEntidadeNotificando(notificacao);
      return true;
    }
    case ntf::TN_TEMPORIZADOR: {
      AtualizaOlho();
      AtualizaEntidades();
      AtualizaAcoes();
#if USAR_WATCHDOG
      watchdog_.Refresca();
#endif
      return true;
    }
    case ntf::TN_REINICIAR_TABULEIRO: {
      EstadoInicial();
      // Repassa aos clientes.
      if (notificacao.local()) {
        central_->AdicionaNotificacaoRemota(ntf::NovaNotificacao(ntf::TN_REINICIAR_TABULEIRO));
      }
      return true;
    }
    case ntf::TN_SERIALIZAR_TABULEIRO: {
      std::unique_ptr<ntf::Notificacao> nt_tabuleiro(SerializaTabuleiro());
      if (notificacao.has_endereco()) {
        // Salvar com nome corrente se endereco for vazio, caso contrario usar o nome da notificacao.
        std::string caminho_str;
        if (notificacao.endereco().empty()) {
          // Busca o nome atual do tabuleiro.
          if (proto_.nome().empty()) {
            auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
            ne->set_erro("Tabuleiro ainda não salvo.");
            central_->AdicionaNotificacao(ne);
            return true;
          }
          caminho_str = proto_.nome();
        } else {
          caminho_str = notificacao.endereco();
        }
        try {
          boost::filesystem::path caminho(caminho_str);
          proto_.set_nome(caminho.filename().string());
          arq::EscreveArquivoBinProto(arq::TIPO_TABULEIRO, caminho.filename().string(), *nt_tabuleiro);
        } catch (const std::logic_error& erro) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(erro.what());
          central_->AdicionaNotificacao(ne);
          return true;
        }
        auto* notificacao = ntf::NovaNotificacao(ntf::TN_INFO);
        notificacao->set_erro(std::string("Tabuleiro '") + caminho_str + "' salvo.");
        central_->AdicionaNotificacao(notificacao);
      } else {
        // Enviar remotamente.
        if (notificacao.clientes_pendentes()) {
          try {
            // Estamos enviando para um novo cliente.
            nt_tabuleiro->set_id(notificacao.id());
            int id_tab = GeraIdTabuleiro();
            clientes_.insert(std::make_pair(id_tab, notificacao.id()));
            nt_tabuleiro->mutable_tabuleiro()->set_id_cliente(id_tab);
          } catch (const std::logic_error& e) {
            auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
            ne->set_erro(e.what());
            // Envia para os clientes pendentes tb.
            auto* copia_ne = new ntf::Notificacao(*ne);
            copia_ne->set_clientes_pendentes(true);
            copia_ne->set_id(notificacao.id());
            central_->AdicionaNotificacao(ne);
            central_->AdicionaNotificacaoRemota(copia_ne);
            return true;
          }
        }
        nt_tabuleiro->set_clientes_pendentes(notificacao.clientes_pendentes());
        central_->AdicionaNotificacaoRemota(nt_tabuleiro.release());
      }
      return true;
    }
    case ntf::TN_DESERIALIZAR_TABULEIRO: {
      if (notificacao.has_endereco()) {
        // Deserializar de arquivo.
        ntf::Notificacao nt_tabuleiro;
        try {
          boost::filesystem::path caminho(notificacao.endereco());
          arq::LeArquivoBinProto(arq::TIPO_TABULEIRO, caminho.filename().string(), &nt_tabuleiro);
          nt_tabuleiro.mutable_tabuleiro()->set_nome(caminho.filename().string());
        } catch (std::logic_error& erro) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro lendo arquivo: ") + notificacao.endereco());
          central_->AdicionaNotificacao(ne);
          return true;
        }
        nt_tabuleiro.set_endereco(notificacao.endereco());
        nt_tabuleiro.mutable_tabuleiro()->set_manter_entidades(notificacao.tabuleiro().manter_entidades());
        DeserializaTabuleiro(nt_tabuleiro);
        // Envia para os clientes.
        central_->AdicionaNotificacaoRemota(SerializaTabuleiro());
      } else {
        // Deserializar da rede.
        DeserializaTabuleiro(notificacao);
      }
      return true;
    }
    case ntf::TN_SERIALIZAR_ENTIDADES_SELECIONAVEIS: {
      std::unique_ptr<ntf::Notificacao> n(SerializaEntidadesSelecionaveis());
      try {
        boost::filesystem::path caminho(notificacao.endereco());
        arq::EscreveArquivoBinProto(arq::TIPO_ENTIDADES, caminho.filename().string(), *n);
        auto* ninfo = ntf::NovaNotificacao(ntf::TN_INFO);
        ninfo->set_erro("Entidades selecionáveis salvas");
        central_->AdicionaNotificacao(ninfo);
      } catch (const std::logic_error& e) {
        auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
        n->set_erro(e.what());
        central_->AdicionaNotificacao(n);
      }
      return true;
    }
    case ntf::TN_DESERIALIZAR_ENTIDADES_SELECIONAVEIS: {
      try {
        boost::filesystem::path caminho(notificacao.endereco());
        ntf::Notificacao n;
        arq::LeArquivoBinProto(arq::TIPO_ENTIDADES, caminho.filename().string(), &n);
        DeserializaEntidadesSelecionaveis(n);
        auto* ninfo = ntf::NovaNotificacao(ntf::TN_INFO);
        ninfo->set_erro("Entidades selecionáveis restauradas");
        central_->AdicionaNotificacao(ninfo);
      } catch (const std::logic_error& e) {
        auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
        n->set_erro(e.what());
        central_->AdicionaNotificacao(n);
      }
      return true;
    }
    case ntf::TN_ATUALIZAR_OPCOES: {
      DeserializaOpcoes(notificacao.opcoes());
      return true;
    }
    case ntf::TN_MOVER_ENTIDADE: {
      MoveEntidadeNotificando(notificacao);
      return true;
    }
    case ntf::TN_ATUALIZAR_ENTIDADE: {
      AtualizaEntidadeNotificando(notificacao);
      return true;
    }
    case ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE: {
      AtualizaParcialEntidadeNotificando(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO: {
      if (notificacao.has_tabuleiro()) {
        // Notificacao ja foi criada, deixa pra ifg fazer o resto.
        return false;
      }
      central_->AdicionaNotificacao(SerializaPropriedades());
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_OPCOES: {
      if (notificacao.has_opcoes()) {
        // Notificacao ja foi criada, deixa pra ifg fazer o resto.
        return false;
      }
      central_->AdicionaNotificacao(SerializaOpcoes());
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ENTIDADE: {
      if (notificacao.has_entidade()) {
        return false;
      }
      if (estado_ != ETAB_ENTS_SELECIONADAS || ids_entidades_selecionadas_.size() > 1) {
        auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
        ne->set_erro("Deve haver uma entidade (e apenas uma) selecionada.");
        central_->AdicionaNotificacao(ne);
        return true;
      }
      auto* n = new ntf::Notificacao(notificacao);
      central_->AdicionaNotificacao(n);
      n->set_modo_mestre(modo_mestre_);
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      return true;
    }
    case ntf::TN_ATUALIZAR_TABULEIRO: {
      DeserializaPropriedades(notificacao.tabuleiro());
      if (notificacao.local()) {
        // So repassa a notificacao pros clientes se a origem dela for local, para evitar ficar enviando
        // infinitamente.
        auto* n_remota = new ntf::Notificacao(notificacao);
        central_->AdicionaNotificacaoRemota(n_remota);
      }
      return true;
    }
    case ntf::TN_LIMPAR_SALVACOES: {
      for (auto& id_entidade : entidades_) {
        id_entidade.second->AtualizaProximaSalvacao(RS_FALHOU);
      }
      if (notificacao.local()) {
        // So repassa a notificacao pros clientes se a origem dela for local, para evitar ficar enviando
        // infinitamente.
        central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
      }
      return true;
    }
    default: ;
  }
  return false;
}

void Tabuleiro::TrataTeclaPressionada(int tecla) {
  switch (tecla) {
    default:
      ;
  }
}

void Tabuleiro::TrataEscalaPorDelta(int delta) {
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
    }
    TrataNotificacao(grupo_notificacoes);
    // Para desfazer.
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  } else {
    // move o olho no eixo Z de acordo com o eixo Y do movimento
    AtualizaRaioOlho(olho_.raio() - (delta * SENSIBILIDADE_RODA));
  }
}

void Tabuleiro::TrataEscalaPorFator(float fator) {
  AtualizaRaioOlho(olho_.raio() / fator);
}

void Tabuleiro::TrataRotacaoPorDelta(float delta_rad) {
  // Realiza a rotacao da tela.
  float olho_rotacao = olho_.rotacao_rad() + delta_rad;
  if (olho_rotacao >= 2 * M_PI) {
    olho_rotacao -= 2 * M_PI;
  } else if (olho_rotacao <= - 2 * M_PI) {
    olho_rotacao += 2 * M_PI;
  }
  olho_.set_rotacao_rad(olho_rotacao);
  AtualizaOlho(true  /*forcar*/);
}

void Tabuleiro::TrataInclinacaoPorDelta(float delta) {
  float olho_altura = olho_.altura() + delta;
  if (olho_altura > OLHO_ALTURA_MAXIMA) {
    olho_altura = OLHO_ALTURA_MAXIMA;
  } else if (olho_altura < OLHO_ALTURA_MINIMA) {
    olho_altura = OLHO_ALTURA_MINIMA;
  }
  olho_.set_altura(olho_altura);
  AtualizaOlho(true  /*forcar*/);
}

void Tabuleiro::TrataTranslacaoPorDelta(int x, int y, int nx, int ny) {
  // Faz picking do tabuleiro sem entidades.
  parametros_desenho_.set_desenha_entidades(false);
  float x0, y0, z0;
  if (!MousePara3dTabuleiro(x, y, &x0, &y0, &z0)) {
    return;
  }

  float x1, y1, z1;
  if (!MousePara3dTabuleiro(nx, ny, &x1, &y1, &z1)) {
    return;
  }

  float delta_x = x1 - x0;
  float delta_y = y1 - y0;
  auto* p = olho_.mutable_alvo();
  p->set_x(p->x() - delta_x);
  p->set_y(p->y() - delta_y);
  olho_.clear_destino();
  AtualizaOlho(true);
}

void Tabuleiro::TrataMovimentoMouse() {
  id_entidade_detalhada_ = Entidade::IdInvalido;
}

void Tabuleiro::TrataMovimentoMouse(int x, int y) {
  if (x == ultimo_x_ && y == ultimo_y_) {
    // No tablet pode acontecer de gerar estes eventos com mesma coordenadas.
    return;
  }
  switch (estado_) {
    case ETAB_ENTS_TRANSLACAO_ROTACAO: {
      if (translacao_rotacao_ == TR_NENHUM) {
        int abs_delta_x = fabs(primeiro_x_ - x);
        int abs_delta_y = fabs(primeiro_y_ - y);
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
          ultimo_x_ = x;
          ultimo_y_ = y;
          return;
        }
      }
      // Deltas desde o ultimo movimento.
      float delta_x = (x - ultimo_x_);
      float delta_y = (y - ultimo_y_);
      // Realiza rotacao da entidade.
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* e = BuscaEntidade(id);
        if (e == nullptr) {
          continue;
        }
        if (translacao_rotacao_ == TR_ROTACAO && e->Tipo() != TE_ENTIDADE) {
          // Rotacao nao se aplica a entidades.
          e->AlteraRotacaoZ(delta_x);
        } else if (translacao_rotacao_ == TR_TRANSLACAO) {
          e->AlteraTranslacaoZ(delta_y * SENSIBILIDADE_ROTACAO_Y);
        }
      }
      ultimo_x_ = x;
      ultimo_y_ = y;
      return;
    }
    case ETAB_ROTACAO: {
      // Realiza a rotacao da tela.
      float olho_rotacao = olho_.rotacao_rad();
      olho_rotacao -= (x - ultimo_x_) * SENSIBILIDADE_ROTACAO_X;
      if (olho_rotacao >= 2 * M_PI) {
        olho_rotacao -= 2 * M_PI;
      } else if (olho_rotacao <= - 2 * M_PI) {
        olho_rotacao += 2 * M_PI;
      }
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
      olho_.set_altura(olho_altura);
      ultimo_x_ = x;
      ultimo_y_ = y;
      AtualizaOlho(true  /*forcar*/);
    }
    break;
    case ETAB_ENTS_PRESSIONADAS: {
      // Realiza o movimento da entidade paralelo ao XY na mesma altura do click original.
      camera_presa_ = false;  // temporariamente.
      parametros_desenho_.set_offset_terreno(ultimo_z_3d_);
      parametros_desenho_.set_desenha_entidades(false);
      float nx, ny, nz;
      if (!MousePara3dTabuleiro(x, y, &nx, &ny, &nz)) {
        return;
      }
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        entidade_selecionada->MoveDelta(nx - ultimo_x_3d_, ny - ultimo_y_3d_, 0.0f);
        Posicao pos;
        pos.set_x(entidade_selecionada->X());
        pos.set_y(entidade_selecionada->Y());
        pos.set_z(ZChao(pos.x(), pos.y()));
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
          auto* n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
          auto* e = n->mutable_entidade();
          e->set_id(id);
          e->mutable_destino()->CopyFrom(pos);
          central_->AdicionaNotificacaoRemota(n);
        }
      }
      ultimo_x_ = x;
      ultimo_y_ = y;
      ultimo_x_3d_ = nx;
      ultimo_y_3d_ = ny;
    }
    break;
    case ETAB_DESLIZANDO: {
      if (camera_presa_) {
        return;
      }
      // Como pode ser chamado entre atualizacoes, atualiza a MODELVIEW.
      //gl::ModoMatriz(GL_MODELVIEW);
      gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
      gl::CarregaIdentidade();
      ConfiguraOlhar();
      // Faz picking do tabuleiro sem entidades.
      float nx, ny, nz;
      if (!MousePara3dTabuleiro(x, y, &nx, &ny, &nz)) {
        return;
      }

      float delta_x = nx - ultimo_x_3d_;
      float delta_y = ny - ultimo_y_3d_;
      // Dependendo da posicao da pinca, os deltas podem se tornar muito grandes. Tentar manter o olho no tabuleiro.
      auto* p = olho_.mutable_alvo();
      float novo_x = p->x() - delta_x;
      float novo_y = p->y() - delta_y;
      if (novo_x < -TamanhoX() || novo_x > TamanhoX() || novo_y < -TamanhoY() || novo_y > TamanhoY()) {
        return;
      }
      p->set_x(novo_x);
      p->set_y(novo_y);
      olho_.clear_destino();
      AtualizaOlho(true);
      ultimo_x_ = x;
      ultimo_y_ = y;
      // No caso de deslizamento, nao precisa atualizar as coordenadas do ultimo_*_3d porque por definicao
      // do movimento, ela fica fixa (o tabuleiro desliza acompanhando o dedo).
    }
    break;
    case ETAB_QUAD_PRESSIONADO:
    case ETAB_SELECIONANDO_ENTIDADES: {
      quadrado_selecionado_ = -1;
      float x3d, y3d, z3d;
      parametros_desenho_.set_desenha_entidades(false);
      if (!MousePara3dTabuleiro(x, y, &x3d, &y3d, &z3d)) {
        // Mouse fora do tabuleiro.
        return;
      }
      ultimo_x_3d_ = x3d;
      ultimo_y_3d_ = y3d;
      ultimo_z_3d_ = z3d;
      // Encontra as entidades cujos centros estao dentro dos limites da selecao.
      std::vector<unsigned int> es;
      for (const auto& eit : entidades_) {
        ids_entidades_selecionadas_.clear();
        const Entidade& e = *eit.second;
        if (PontoDentroQuadrado(e.X(), e.Y(), primeiro_x_3d_, primeiro_y_3d_, ultimo_x_3d_, ultimo_y_3d_)) {
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
      if (!MousePara3dTabuleiro(x, y, &x3d, &y3d, &z3d)) {
        // Mouse fora do tabuleiro.
        return;
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

}

void Tabuleiro::TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y) {
  TrataBotaoEsquerdoPressionado(x, y, true  /*alternar selecao*/);
}

void Tabuleiro::TrataBotaoAlternarIluminacaoMestre() {
  opcoes_.set_iluminacao_mestre_igual_jogadores(!opcoes_.iluminacao_mestre_igual_jogadores());
}

void Tabuleiro::TrataBotaoAcaoPressionado(bool acao_padrao, int x, int y) {
  // Preenche os dados comuns.
  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha > 2) {
    // invalido.
    return;
  }
  // Primeiro, entidades.
  unsigned int id_entidade_destino = Entidade::IdInvalido;
  Posicao pos_entidade;
  if (pos_pilha == OBJ_ENTIDADE) {
    VLOG(1) << "Acao em entidade: " << id;
    // Entidade.
    id_entidade_destino = id;
    float x3d, y3d, z3d;
#if !USAR_OPENGL_ES
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
#else
    MousePara3dComId(x, y, id, OBJ_ENTIDADE, &x3d, &y3d, &z3d);
#endif
    pos_entidade.set_x(x3d);
    pos_entidade.set_y(y3d);
    pos_entidade.set_z(z3d);
    // Depois tabuleiro.
    parametros_desenho_.set_desenha_entidades(false);
    BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  }
  Posicao pos_tabuleiro;
  if (pos_pilha == OBJ_TABULEIRO) {
    float x3d, y3d, z3d;
    MousePara3dTabuleiro(x, y, &x3d, &y3d, &z3d);
    unsigned int id_quadrado = IdQuadrado(x3d, y3d);
    VLOG(1) << "Acao no tabuleiro: " << id_quadrado;
    // Tabuleiro, posicao do quadrado clicado.
    // Posicao exata do clique.
    pos_tabuleiro.set_x(x3d);
    pos_tabuleiro.set_y(y3d);
    pos_tabuleiro.set_z(z3d);
  }

  // Executa a acao: se nao houver ninguem selecionado, faz sinalizacao. Se houver, ha dois modos de execucao:
  // - Efeito de area
  // - Efeito individual.
  if (acao_padrao || ids_entidades_selecionadas_.size() == 0) {
    AcaoProto acao_proto;
    // Sem entidade selecionada, realiza sinalizacao.
    VLOG(1) << "Acao de sinalizacao: " << acao_proto.ShortDebugString();
    acao_proto.set_tipo(ACAO_SINALIZACAO);
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
    for (auto id_selecionado : ids_entidades_selecionadas_) {
      Entidade* entidade = BuscaEntidade(id_selecionado);
      if (entidade == nullptr || entidade->Tipo() != TE_ENTIDADE) {
        continue;
      }
      std::string ultima_acao = entidade->Proto().ultima_acao().empty() ?
         ID_ACAO_ATAQUE_CORPO_A_CORPO : entidade->Proto().ultima_acao();
      auto acao_it = mapa_acoes_.find(ultima_acao);
      if (acao_it == mapa_acoes_.end()) {
        LOG(ERROR) << "Acao invalida da entidade: '" << ultima_acao << "'";
        continue;
      }
      AcaoProto acao_proto(*acao_it->second);
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
        if (!lista_pontos_vida_.empty()) {
          delta_pontos_vida = lista_pontos_vida_.front();
          lista_pontos_vida_.pop_front();
          acao_proto.set_delta_pontos_vida(delta_pontos_vida);
          acao_proto.set_afeta_pontos_vida(true);
        }
        std::vector<unsigned int> ids_afetados = EntidadesAfetadasPorAcao(acao_proto);
        for (auto id : ids_afetados) {
          acao_proto.add_id_entidade_destino(id);
          // Para desfazer.
          if (delta_pontos_vida == 0) {
            continue;
          }
          const Entidade* entidade_destino = BuscaEntidade(id);
          if (entidade_destino == nullptr) {
            // Nunca deveria acontecer pois a funcao EntidadesAfetadasPorAcao ja buscou a entidade.
            LOG(ERROR) << "Entidade nao encontrada, nunca deveria acontecer.";
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
          PreencheNotificacaoDeltaPontosVida(*entidade_destino, delta_pv_pos_salvacao, nd, nd);
        }
        VLOG(2) << "Acao de area: " << acao_proto.ShortDebugString();
        n.mutable_acao()->CopyFrom(acao_proto);
      } else {
        Entidade* entidade_destino =
           id_entidade_destino != Entidade::IdInvalido ? BuscaEntidade(id_entidade_destino) : nullptr;
        if (!lista_pontos_vida_.empty() && entidade_destino != nullptr) {
          int delta_pontos_vida = lista_pontos_vida_.front();
          lista_pontos_vida_.pop_front();
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
          acao_proto.set_delta_pontos_vida(delta_pv_pos_salvacao);
          acao_proto.set_afeta_pontos_vida(true);
          auto* nd = grupo_desfazer.add_notificacao();
          PreencheNotificacaoDeltaPontosVida(*entidade_destino, delta_pv_pos_salvacao, nd, nd);
        }
        VLOG(2) << "Acao individual: " << acao_proto.ShortDebugString();
        n.mutable_acao()->CopyFrom(acao_proto);
      }
      TrataNotificacao(n);
      atraso_segundos += 0.5f;
    }
    AdicionaNotificacaoListaEventos(grupo_desfazer);
  }
}

void Tabuleiro::TrataBotaoLiberado() {
  FinalizaEstadoCorrente();
}

void Tabuleiro::TrataMouseParadoEm(int x, int y) {
  unsigned int id;
  unsigned int pos_pilha;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha);
  if (pos_pilha != OBJ_ENTIDADE) {
    // Mouse no tabuleiro.
    id_entidade_detalhada_ = Entidade::IdInvalido;
    return;
  }
  id_entidade_detalhada_ = id;
}

void Tabuleiro::TrataRedimensionaJanela(int largura, int altura) {
  gl::Viewport(0, 0, (GLint)largura, (GLint)altura);
  largura_ = largura;
  altura_ = altura;
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
      auto* pos_original = n_desfazer->mutable_entidade()->mutable_pos();
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

void Tabuleiro::IniciaGL() {
  gl::Desabilita(GL_DITHER);
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::Habilita(GL_BLEND);

  // Nao desenha as costas dos poligonos.
  gl::Habilita(GL_CULL_FACE);
  gl::FaceNula(GL_BACK);

  // Isso aqui nao funciona, mas vai que funciona em algum dispositivo...
  gl::Habilita(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  if (glGetError() != GL_NO_ERROR) {
    LOG(WARNING) << "Erro no GL_LINE_SMOOTH_HINT";
  }
  glHint(GL_FOG_HINT, GL_NICEST);
  if (glGetError() != GL_NO_ERROR) {
    LOG(WARNING) << "Erro no GL_FOG_HINT";
  }
  RegeraVbo();
  GeraVboCaixaCeu();
  gl_iniciado_ = true;

  Entidade::IniciaGl();
}

void Tabuleiro::SelecionaModeloEntidade(const std::string& id_modelo) {
  auto it = mapa_modelos_.find(id_modelo);
  if (it == mapa_modelos_.end()) {
    LOG(ERROR) << "Id de modelo inválido: " << id_modelo;
    return;
  }
  modelo_selecionado_ = it->second.get();
}

const EntidadeProto* Tabuleiro::BuscaModelo(const std::string& id_modelo) const {
  auto it = mapa_modelos_.find(id_modelo);
  if (it == mapa_modelos_.end()) {
    LOG(ERROR) << "Id de modelo inválido: " << id_modelo;
    return nullptr;
  }
  return it->second.get();
}

void Tabuleiro::SelecionaAcao(const std::string& id_acao) {
  auto it = mapa_acoes_.find(id_acao);
  if (it == mapa_acoes_.end()) {
    LOG(ERROR) << "Id de acao inválido: " << id_acao;
    return;
  }
  for (auto id_selecionado : ids_entidades_selecionadas_) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    entidade->AtualizaAcao(it->first);
  }
}

void Tabuleiro::ProximaAcao() {
  if (id_acoes_.size() == 0) {
    return;
  }
  for (auto id_selecionado : ids_entidades_selecionadas_) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    std::string acao(entidade->Acao());
    if (acao.empty()) {
      acao = ID_ACAO_ATAQUE_CORPO_A_CORPO;
    }
    auto it = std::find(id_acoes_.begin(), id_acoes_.end(), acao);
    if (it == id_acoes_.end()) {
      LOG(ERROR) << "Id de acao inválido: " << entidade->Acao();
      continue;
    }
    ++it;
    if (it == id_acoes_.end()) {
      it = id_acoes_.begin();
    }
    entidade->AtualizaAcao(*it);
  }
}

void Tabuleiro::AcaoAnterior() {
  if (id_acoes_.size() == 0) {
    return;
  }
  for (auto id_selecionado : ids_entidades_selecionadas_) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    std::string acao(entidade->Acao());
    if (acao.empty()) {
      acao = ID_ACAO_ATAQUE_CORPO_A_CORPO;
    }
    auto it = std::find(id_acoes_.rbegin(), id_acoes_.rend(), acao);
    if (it == id_acoes_.rend()) {
      LOG(ERROR) << "Id de acao inválido: " << entidade->Acao();
      continue;
    }
    ++it;
    if (it == id_acoes_.rend()) {
      it = id_acoes_.rbegin();
    }
    entidade->AtualizaAcao(*it);
  }
}

// privadas
void Tabuleiro::DesenhaCena() {
  // Caso o parametros_desenho_.desenha_fps() seja false, ele computara mas nao desenhara o objeto.
  // Isso eh importante para computacao de frames lentos, mesmo que nao seja mostrado naquele quadro.
  TimerEscopo timer_escopo(this, opcoes_.mostra_fps());

  gl::InicioCena();
  gl::IniciaNomes();

  gl::Habilita(GL_DEPTH_TEST);
  gl::CorLimpeza(proto_.luz_ambiente().r(),
                 proto_.luz_ambiente().g(),
                 proto_.luz_ambiente().b(),
                 proto_.luz_ambiente().a());
  gl::Limpa(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  gl::ModoMatriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  ConfiguraOlhar();
  parametros_desenho_.mutable_pos_olho()->CopyFrom(olho_.pos());
  // Verifica o angulo em relacao ao tabuleiro para decidir se as texturas ficarao viradas para cima.
  if (camera_isometrica_ || (olho_.altura() > (2 * olho_.raio()))) {
    parametros_desenho_.set_desenha_texturas_para_cima(true);
  } else {
    parametros_desenho_.set_desenha_texturas_para_cima(false);
  }

  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
    DesenhaLuzes();
  } else {
    gl::Desabilita(GL_LIGHTING);
    gl::Desabilita(GL_FOG);
  }

  //ceu_.desenha(parametros_desenho_);
  if (!parametros_desenho_.has_picking_x() && parametros_desenho_.desenha_texturas() && proto_.has_info_textura_ceu()) {
    DesenhaCaixaCeu();
  }

  // desenha tabuleiro do sul para o norte.
  {
    gl::TipoEscopo nomes_tabuleiro(OBJ_TABULEIRO);
    DesenhaTabuleiro();
    if (parametros_desenho_.desenha_grade() &&
        opcoes_.desenha_grade() &&
        (proto_.desenha_grade() || (!VisaoMestre() && proto_.textura_mestre_apenas()))) {
      // Pra evitar z fight, desliga a profundidade,
      gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
      DesenhaGrade();
    }
  }

  // Algumas verificacoes.
  GLint depth = 0;
  gl::Le(GL_MODELVIEW_STACK_DEPTH, &depth);
  if (depth > 2) {
    LOG(ERROR) << "Pilha MODELVIEW com vazamento: " << depth;
  }
  gl::Le(GL_PROJECTION_STACK_DEPTH, &depth);
  if (depth > 2) {
    LOG(ERROR) << "Pilha PROJECTION com vazamento: " << depth;
  }
#if !USAR_OPENGL_ES
  gl::Le(GL_ATTRIB_STACK_DEPTH, &depth);
  if (depth > 2) {
    LOG(ERROR) << "Pilha de ATRIBUTOS com vazamento: " << depth;
  }
#endif

  if (VisaoMestre() && parametros_desenho_.desenha_pontos_rolagem()) {
    // Pontos de rolagem na terceira posicao da pilha.
    gl::TipoEscopo pontos(OBJ_ROLAGEM);
    DesenhaPontosRolagem();
  }

  if (parametros_desenho_.desenha_entidades()) {
    gl::TipoEscopo nomes(OBJ_ENTIDADE);
    // Desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
    // na hora do picking.
    DesenhaEntidades();
  }

  if (parametros_desenho_.desenha_acoes()) {
    DesenhaAcoes();
  }

  // Sombras.
  if (parametros_desenho_.desenha_sombras() &&
      proto_.luz_direcional().inclinacao_graus() > 5.0 &&
      proto_.luz_direcional().inclinacao_graus() < 180.0f) {
    bool desenha_texturas = parametros_desenho_.desenha_texturas();
    parametros_desenho_.set_desenha_texturas(false);
    DesenhaSombras();
    parametros_desenho_.set_desenha_texturas(desenha_texturas);
  }

  if (estado_ == ETAB_ENTS_PRESSIONADAS && parametros_desenho_.desenha_rastro_movimento() && !rastros_movimento_.empty()) {
    //gl::HabilitaEscopo blend_escopo(GL_BLEND);
    LigaStencil();
    DesenhaRastros();
    DesenhaStencil(COR_AZUL_ALFA);
  }

  if (estado_ == ETAB_DESENHANDO && parametros_desenho_.desenha_forma_selecionada()) {
    DesenhaFormaSelecionada();
  }

  if (parametros_desenho_.desenha_quadrado_selecao() && estado_ == ETAB_SELECIONANDO_ENTIDADES) {
    gl::DesligaEscritaProfundidadeEscopo desliga_teste_escopo;
    gl::DesabilitaEscopo cull_escopo(GL_CULL_FACE);
    //gl::HabilitaEscopo blend_escopo(GL_BLEND);
    gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
    gl::DesvioProfundidade(-3.0f, -30.0f);
    MudaCorAlfa(COR_AZUL_ALFA);
    gl::Retangulo(primeiro_x_3d_, primeiro_y_3d_, ultimo_x_3d_, ultimo_y_3d_);
  }

  // Transparencias devem vir por ultimo porque dependem do que esta atras. As transparencias nao atualizam
  // o buffer de profundidade, ja que se dois objetos transparentes forem desenhados um atras do outro,
  // a ordem nao importa. Ainda assim, o z buffer eh necessario para comparar o objeto transparente
  // a outros nao transparentes durante o picking.
  if (parametros_desenho_.desenha_entidades()) {
    if (parametros_desenho_.transparencias()) {
      gl::HabilitaEscopo teste_profundidade(GL_DEPTH_TEST);
      gl::DesligaEscritaProfundidadeEscopo desliga_escrita_profundidade_escopo;
      parametros_desenho_.set_alfa_translucidos(0.5);
      DesenhaEntidadesTranslucidas();
      parametros_desenho_.clear_alfa_translucidos();
      DesenhaAuras();
    } else {
      gl::TipoEscopo nomes(OBJ_ENTIDADE);
      // Desenha os translucidos de forma solida para picking.
      DesenhaEntidadesTranslucidas();
    }
  }


  if (parametros_desenho_.desenha_rosa_dos_ventos() && opcoes_.desenha_rosa_dos_ventos()) {
    DesenhaRosaDosVentos();
  }

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }

  if (parametros_desenho_.desenha_id_acao()) {
    DesenhaIdAcaoEntidade();
  }

  if (parametros_desenho_.desenha_controle_virtual() && opcoes_.desenha_controle_virtual()) {
    // Controle na quarta posicao da pilha.
    gl::TipoEscopo controle(OBJ_CONTROLE_VIRTUAL);
    DesenhaControleVirtual();
  }
}

void Tabuleiro::GeraVboCaixaCeu() {
  vbo_caixa_ceu_ = gl::VboCuboSolido(10.0f);
  // Valores de referencia:
  // imagem 4x3.
  // x = 0.0f, 0.25f, 0.50f, 0.75f, 1.0f
  // y = 1.0f, 0.66f, 0.33f, 0f
  const float texturas[6 * 4 * 2] = {
    // sul: 0-3 (linha meio, coluna 0),--,+-,++,-+
    0.25f, 0.66f,
    0.0f, 0.66f,
    0.0f, 0.33f,
    0.25f, 0.33f,
    // norte: 4-7 (linha meio, coluna 2),--,-+,++,+-
    0.50f, 0.66f,
    0.50f, 0.33f,
    0.75f, 0.33f,
    0.75f, 0.66f,
    // oeste: 8-11 (linha meio, coluna 1),--,-+,++,+-
    0.25f, 0.66f,
    0.25f, 0.33f,
    0.50f, 0.33f,
    0.50f, 0.66f,
    // leste: 12-15 (linha meio, coluna 3):--,+-,++,-+
    1.0f, 0.66f,
    0.75f, 0.66f,
    0.75f, 0.33f,
    1.0f, 0.33f,
    // cima: 16-19 (linha de cima, coluna 1):--,+-,++,-+
    0.25f, 0.33f,
    0.50f, 0.33f,
    0.50f, 0.0f,
    0.25f, 0.0f,
    // baixo: 20-23 (linha de baixo, coluna 1):--,-+,++,+-
    0.25f, 1.0f,
    0.25f, 0.66f,
    0.50f, 0.66f,
    0.50f, 1.0f,
  };
  vbo_caixa_ceu_.AtribuiTexturas(texturas);
  gl::GravaVbo(&vbo_caixa_ceu_);
}

void Tabuleiro::RegeraVbo() {
  // TODO quando limpar essa flag.
  // TODO limite de tamanho de tabuleiro.
  indices_tabuleiro_.clear();
  vertices_tabuleiro_.resize(TamanhoY() * TamanhoX() * 4 * 2);  // 4 vertices por quadrado, cada um dois pontos.
  unsigned short indice = 0;
  float x = 0, y = 0;
  float tamanho_texel_h;
  float tamanho_texel_v;
  if (proto_.ladrilho()) {
    tamanho_texel_h = 1.0f;
    tamanho_texel_v = 1.0f;
  } else {
    tamanho_texel_h = 1.0f / TamanhoX();
    tamanho_texel_v = 1.0f / TamanhoY();
  }
  for (int y_tab = 0; y_tab < TamanhoY(); ++y_tab) {
    if (indice + 4 > USHRT_MAX) {
      LOG(ERROR) << "Tabuleiro muito grande: " << TamanhoX() << "x" << TamanhoY();
      break;
    }
    float inicio_texel_v = proto_.ladrilho() ? 0.0f : (TamanhoY() - y_tab) * tamanho_texel_v;
    float inicio_texel_h = 0.0f;
    for (int x_tab = 0; x_tab < TamanhoX(); ++x_tab) {
      if (indice + 4 > USHRT_MAX) {
        break;
      }
      // desenha quadrado
      vertices_tabuleiro_[indice + 0].x = x;
      vertices_tabuleiro_[indice + 0].y = y;
      vertices_tabuleiro_[indice + 0].s0 = inicio_texel_h;
      vertices_tabuleiro_[indice + 0].t0 = inicio_texel_v;
      vertices_tabuleiro_[indice + 1].x = x + TAMANHO_LADO_QUADRADO;
      vertices_tabuleiro_[indice + 1].y = y;
      vertices_tabuleiro_[indice + 1].s0 = inicio_texel_h + tamanho_texel_h;
      vertices_tabuleiro_[indice + 1].t0 = inicio_texel_v;
      vertices_tabuleiro_[indice + 2].x = x + TAMANHO_LADO_QUADRADO;
      vertices_tabuleiro_[indice + 2].y = y + TAMANHO_LADO_QUADRADO;
      vertices_tabuleiro_[indice + 2].s0 = inicio_texel_h + tamanho_texel_h;
      vertices_tabuleiro_[indice + 2].t0 = inicio_texel_v - tamanho_texel_v;
      vertices_tabuleiro_[indice + 3].x = x;
      vertices_tabuleiro_[indice + 3].y = y + TAMANHO_LADO_QUADRADO;
      vertices_tabuleiro_[indice + 3].s0 = inicio_texel_h;
      vertices_tabuleiro_[indice + 3].t0 = inicio_texel_v - tamanho_texel_v;

      indices_tabuleiro_.push_back(indice);
      indices_tabuleiro_.push_back(indice + 1);
      indices_tabuleiro_.push_back(indice + 2);
      indices_tabuleiro_.push_back(indice);
      indices_tabuleiro_.push_back(indice + 2);
      indices_tabuleiro_.push_back(indice + 3);
      indice += 4;
      x += TAMANHO_LADO_QUADRADO;
      if (!proto_.ladrilho()) {
        inicio_texel_h += tamanho_texel_h;
      }
    }
    // volta tudo esquerda e sobe 1 quadrado
    x = 0;
    y += TAMANHO_LADO_QUADRADO;
  }
  // Cria o ID do VBO.
  if (nome_buffer_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_);
  }
  gl::GeraBuffers(1, &nome_buffer_);
  // Associa vertices com ARRAY_BUFFER.
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_buffer_);
  gl::BufferizaDados(GL_ARRAY_BUFFER, sizeof(InfoVerticeTabuleiro) * vertices_tabuleiro_.size(), vertices_tabuleiro_.data(), GL_STATIC_DRAW);
  // Cria buffer de indices.
  if (nome_buffer_indice_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_indice_);
  }
  gl::GeraBuffers(1, &nome_buffer_indice_);
  // Associa indices com GL_ELEMENT_ARRAY_BUFFER.
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, nome_buffer_indice_);
  gl::BufferizaDados(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices_tabuleiro_.size(), indices_tabuleiro_.data(), GL_STATIC_DRAW);

  // Regera a grade.
  if (nome_buffer_grade_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_grade_);
  }
  if (nome_buffer_indice_grade_ != 0) {
    gl::ApagaBuffers(1, &nome_buffer_indice_grade_);
  }
  vertices_grade_.clear();
  indices_grade_.clear();
  // A grade sera regerada independente dos valores do proto, ja que o controle se ela devera ser desenha ou nao
  // e feito durante o desenho da cena.
  const int x_2 = TamanhoX() / 2;
  const int y_2 = TamanhoY() / 2;
  const float tamanho_y_2 = (TamanhoY() / 2.0f) * TAMANHO_LADO_QUADRADO;
  const float tamanho_x_2 = (TamanhoX() / 2.0f) * TAMANHO_LADO_QUADRADO;

  // O tabuleiro tem caracteristicas diferentes se o numero de quadrados for par ou impar. Se for
  // impar, a grade passa pelo centro do tabuleiro. Caso contrario ela ladeia o centro. Por isso
  // ha o tratamento com base no tamanho, abaixo. O incremento eh o desvio de meio quadrado e o limite
  // inferior eh onde comeca a desenhar a linha.

  // Linhas verticais (S-N).
  int limite_inferior = -x_2;
  float incremento = 0.0f;
  if (TamanhoX() % 2 != 0) {
    --limite_inferior;
    incremento = TAMANHO_LADO_QUADRADO_2;
  }
  indice = 0;
  for (int i = limite_inferior; i <= x_2; ++i) {
    float x = i * TAMANHO_LADO_QUADRADO + incremento;
    vertices_grade_.push_back(x - EXPESSURA_LINHA_2);
    vertices_grade_.push_back(-tamanho_y_2);
    vertices_grade_.push_back(x + EXPESSURA_LINHA_2);
    vertices_grade_.push_back(-tamanho_y_2);
    vertices_grade_.push_back(x + EXPESSURA_LINHA_2);
    vertices_grade_.push_back(tamanho_y_2);
    vertices_grade_.push_back(x - EXPESSURA_LINHA_2);
    vertices_grade_.push_back(tamanho_y_2);
    indices_grade_.push_back(indice);
    indices_grade_.push_back(indice + 1);
    indices_grade_.push_back(indice + 2);
    indices_grade_.push_back(indice);
    indices_grade_.push_back(indice + 2);
    indices_grade_.push_back(indice + 3);
    indice += 4;
  }
  // Linhas horizontais (W-E).
  limite_inferior = -y_2;
  incremento = 0.0f;
  if (TamanhoY() % 2 != 0) {
    --limite_inferior;
    incremento = TAMANHO_LADO_QUADRADO_2;
  }
  for (int i = limite_inferior; i <= y_2; ++i) {
    float y = i * TAMANHO_LADO_QUADRADO + incremento;
    vertices_grade_.push_back(-tamanho_x_2);
    vertices_grade_.push_back(y - EXPESSURA_LINHA_2);
    vertices_grade_.push_back(tamanho_x_2);
    vertices_grade_.push_back(y - EXPESSURA_LINHA_2);
    vertices_grade_.push_back(tamanho_x_2);
    vertices_grade_.push_back(y + EXPESSURA_LINHA_2);
    vertices_grade_.push_back(-tamanho_x_2);
    vertices_grade_.push_back(y + EXPESSURA_LINHA_2);
    indices_grade_.push_back(indice);
    indices_grade_.push_back(indice + 1);
    indices_grade_.push_back(indice + 2);
    indices_grade_.push_back(indice);
    indices_grade_.push_back(indice + 2);
    indices_grade_.push_back(indice + 3);
    indice += 4;
  }
  gl::GeraBuffers(1, &nome_buffer_grade_);
  gl::GeraBuffers(1, &nome_buffer_indice_grade_);
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_buffer_grade_);
  gl::BufferizaDados(GL_ARRAY_BUFFER, sizeof(float) * vertices_grade_.size(), vertices_grade_.data(), GL_STATIC_DRAW);
  // Associa indices com GL_ELEMENT_ARRAY_BUFFER.
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, nome_buffer_indice_grade_);
  gl::BufferizaDados(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices_grade_.size(), indices_grade_.data(), GL_STATIC_DRAW);
}

void Tabuleiro::DesenhaTabuleiro() {
  gl::CarregaNome(0);
  gl::MatrizEscopo salva_matriz;
  float deltaX = -TamanhoX() * TAMANHO_LADO_QUADRADO;
  float deltaY = -TamanhoY() * TAMANHO_LADO_QUADRADO;
  gl::Normal(0, 0, 1.0f);
  if (parametros_desenho_.has_offset_terreno()) {
    // Para mover entidades acima do plano do olho.
    gl::Desabilita(GL_CULL_FACE);
  } else {
    gl::Habilita(GL_CULL_FACE);
  }
  // Desenha o chao mais pro fundo.
  // TODO transformar offsets em constantes.
  gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(2.0f, 20.0f);
  MudaCor(proto_.has_info_textura() ? COR_BRANCA : COR_CINZA_CLARO);
  gl::Translada(deltaX / 2.0f,
                deltaY / 2.0f,
                parametros_desenho_.has_offset_terreno() ? parametros_desenho_.offset_terreno() : 0.0f);
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  // Usa os vertices de VBO.
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_buffer_);
  gl::PonteiroVertices(2, GL_FLOAT, sizeof(InfoVerticeTabuleiro), (void*)0);
  GLuint id_textura = parametros_desenho_.desenha_texturas() &&
                      proto_.has_info_textura() &&
                      (!proto_.textura_mestre_apenas() || VisaoMestre()) ?
      texturas_->Textura(proto_.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, id_textura);
    gl::PonteiroVerticesTexturas(2, GL_FLOAT, sizeof(InfoVerticeTabuleiro), (void*)8);
  }
  // Usa os indices de VBO.
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, nome_buffer_indice_);
  gl::DesenhaElementos(GL_TRIANGLES, indices_tabuleiro_.size(), GL_UNSIGNED_SHORT, (void*)0);

  // Se a face nula foi desativada, reativa.
  gl::Habilita(GL_CULL_FACE);

  // Volta ao normal.
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  gl::Desabilita(GL_TEXTURE_2D);
  gl::DesabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);

  // Desenha quadrado selecionado.
  if (quadrado_selecionado_ != -1 && proto_.desenha_grade()) {
    //gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
    // Por algum motivo desligar o DEPTH aqui da biziu total no motoX.
    const float cor[4] = { 0.0f, 0.0f, 0.0f, 0.3f };
    MudaCorAlfa(cor);
    int linha = quadrado_selecionado_ / TamanhoX();
    int coluna = quadrado_selecionado_ % TamanhoX();
    float x3d = coluna * TAMANHO_LADO_QUADRADO, y3d = linha * TAMANHO_LADO_QUADRADO;
    const float vertices_s[] = {
      x3d, y3d, 0.05f,
      x3d + TAMANHO_LADO_QUADRADO, y3d, 0.05f,
      x3d + TAMANHO_LADO_QUADRADO, y3d + TAMANHO_LADO_QUADRADO, 0.05f,
      x3d, y3d + TAMANHO_LADO_QUADRADO, 0.05f,
    };
    unsigned short indices_s[] = { 0, 1, 2, 3 };
    gl::PonteiroVertices(3, GL_FLOAT, vertices_s);
    gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices_s);
  }

  // Desliga vertex array.
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Tabuleiro::DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f, bool sombra) {
  float limite_quad = 0.0f;
  if (sombra && proto_.has_nevoa()) {
    float limite = proto_.nevoa().distancia_minima() +
                   (proto_.nevoa().distancia_maxima() - proto_.nevoa().distancia_minima()) * 0.7f;
    limite_quad = pow(limite, 2);
  }
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second.get();
    if (entidade == nullptr) {
      LOG(ERROR) << "Entidade nao existe.";
      continue;
    }
    if (sombra && proto_.has_nevoa()) {
      // Distancia para camera. So desenha se estiver fora da nevoa.
      float distancia_quad = pow(entidade->X() - olho_.pos().x(), 2) +
                             pow(entidade->Y() - olho_.pos().y(), 2) +
                             pow(entidade->Z() - olho_.pos().z(), 2);
      if (distancia_quad >= limite_quad) {
        continue;
      }
    }
    // Nao roda disco se estiver arrastando.
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS &&
                                                 EntidadeEstaSelecionada(entidade->Id()));
    bool detalhar_tudo = detalhar_todas_entidades_ || modo_acao_;
    bool entidade_detalhada = parametros_desenho_.desenha_detalhes() &&
                              (entidade->Id() == id_entidade_detalhada_);
    parametros_desenho_.set_desenha_barra_vida(entidade_detalhada || detalhar_tudo);
    // Rotulos apenas individualmente.
    parametros_desenho_.set_desenha_rotulo(entidade_detalhada);
    parametros_desenho_.set_desenha_rotulo_especial(
        entidade_detalhada && (VisaoMestre() || entidade->SelecionavelParaJogador()));
    parametros_desenho_.set_desenha_eventos_entidades(VisaoMestre() || entidade->SelecionavelParaJogador());
    f(entidade, &parametros_desenho_);
  }
  parametros_desenho_.set_entidade_selecionada(false);
  parametros_desenho_.set_desenha_barra_vida(false);
}

void Tabuleiro::DesenhaRastros() {
  gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(-2.0f, -20.0f);
  for (const auto& it : rastros_movimento_) {
    auto* e = BuscaEntidade(it.first);
    if (e == nullptr || e->Tipo() != TE_ENTIDADE) {
      continue;
    }
    // Copia vetor de pontos e adiciona posicao corrente da entidade.
    auto pontos = it.second;
    Posicao pos;
    pos.set_x(e->X());
    pos.set_y(e->Y());
    pos.set_z(ZChao(pos.x(), pos.y()));
    pontos.push_back(pos);
    // Rastro pouco menor que quadrado.
    DesenhaLinha3d(pontos, e->MultiplicadorTamanho() * TAMANHO_LADO_QUADRADO * 0.8);
  }
}

void Tabuleiro::DesenhaAuras() {
  if (parametros_desenho_.desenha_aura()) {
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      Entidade* entidade = it->second.get();
      entidade->DesenhaAura(&parametros_desenho_);
    }
  }
}

void Tabuleiro::DesenhaAcoes() {
  for (auto& a : acoes_) {
    VLOG(4) << "Desenhando acao:" << a->Proto().ShortDebugString();
    a->Desenha(&parametros_desenho_);
  }
}

void Tabuleiro::DesenhaFormaSelecionada() {
  parametros_desenho_.set_alfa_translucidos(0.5);
  Entidade::DesenhaObjetoProto(forma_proto_, &parametros_desenho_, nullptr);
  parametros_desenho_.clear_alfa_translucidos();
}

void Tabuleiro::DesenhaRosaDosVentos() {
  // Modo 2d.
  gl::MatrizEscopo salva_matriz_proj(GL_PROJECTION);
  gl::CarregaIdentidade();
  // Eixo com origem embaixo esquerda.
  gl::Ortogonal(0, largura_, 0, altura_, -1.0f, 1.0f);
  gl::MatrizEscopo salva_matriz_mv(GL_MODELVIEW);
  gl::CarregaIdentidade();
  gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
  gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
  const static float kRaioRosa = 20.0f;
  // Deixa espaco para o N.
  gl::Translada(largura_ - kRaioRosa - 15.0f, kRaioRosa + 15.0f, 0.0f);
  MudaCor(COR_BRANCA);
  // Desenha fundo da rosa.
  DesenhaDisco(kRaioRosa, 8  /*faces*/);
  // Desenha seta.
  const static float kLarguraSeta = 5.0f;
  const static float kTamanhoSeta = kRaioRosa * 0.8f;
  MudaCor(COR_VERMELHA);
  unsigned short indices[] = { 0, 1, 2 };
  float vertices[] = {
    -kLarguraSeta, 0.0f,
    kLarguraSeta, 0.0f,
    0.0f, kTamanhoSeta,
  };
  unsigned short indices_n[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  float vertices_n[] = {
    // Primeira perna N.
    -4.0f, 0.0f,
    -1.0f, 0.0f,
    -4.0f, 13.0f,
    // Segunda.
    -4.0f, 13.0f,
    -4.0f, 8.0f,
    4.0f, 0.0f,
    // Terceira.
    4.0f, 0.0f,
    4.0f, 13.0f,
    1.0f, 13.0f,
  };

  // Roda pra posicao correta.
  Posicao direcao;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &direcao);
  // A diferenca eh em relacao ao leste e o norte esta a 90 graus. Quanto maior a diferenca, mais proximo do norte (ate 90.0f).
  float diferenca_graus = 90.0f - VetorParaRotacaoGraus(direcao.x(), direcao.y());

  // Seta.
  gl::Roda(diferenca_graus, 0.0f, 0.0f, 1.0f);
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 3, GL_UNSIGNED_SHORT, indices);
  // N.
  gl::Translada(0.0f, kRaioRosa + 2.0f, 0.0f);
  gl::PonteiroVertices(2, GL_FLOAT, vertices_n);
  gl::DesenhaElementos(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, indices_n);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Tabuleiro::DesenhaPontosRolagem() {
  // 4 pontos.
  MudaCor(COR_PRETA);
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  float translacao_x = ((TamanhoX() / 2) + 1) * TAMANHO_LADO_QUADRADO +
                       ((TamanhoX() % 2 != 0) ? TAMANHO_LADO_QUADRADO_2 : 0);
  float translacao_y = ((TamanhoY() / 2) + 1) * TAMANHO_LADO_QUADRADO +
                       ((TamanhoY() % 2 != 0) ? TAMANHO_LADO_QUADRADO_2 : 0);
  int id = 0;
  for (const std::pair<float, float>& delta : { std::pair<float, float>{ translacao_x, 0.0f },
                                                std::pair<float, float>{ -translacao_x, 0.0f },
                                                std::pair<float, float>{ 0.0f, translacao_y },
                                                std::pair<float, float>{ 0.0f, -translacao_y } }) {
    gl::CarregaNome(id++);
    gl::Retangulo(-TAMANHO_LADO_QUADRADO_2 + delta.first, -TAMANHO_LADO_QUADRADO_2 + delta.second,
                  TAMANHO_LADO_QUADRADO_2 + delta.first, TAMANHO_LADO_QUADRADO_2 + delta.second);
  }
}

void Tabuleiro::SelecionaFormaDesenho(TipoForma fd) {
  forma_selecionada_ = fd;
  switch (fd) {
    case TF_CILINDRO:
    case TF_CIRCULO:
    case TF_CONE:
    case TF_CUBO:
    case TF_ESFERA:
    case TF_LIVRE:
    case TF_PIRAMIDE:
    case TF_RETANGULO:
      break;
    default:
      LOG(ERROR) << "Forma de desenho invalida: " << fd;
  }
}

void Tabuleiro::DesenhaSombras() {
  const float kAnguloInclinacao = proto_.luz_direcional().inclinacao_graus() * GRAUS_PARA_RAD;
  const float kAnguloPosicao = proto_.luz_direcional().posicao_graus() * GRAUS_PARA_RAD;
  float fator_shear = proto_.luz_direcional().inclinacao_graus() == 90.0f ?
      0.0f : 1.0f / tanf(kAnguloInclinacao);
  // A sombra nao pode ser totalmente solida..
  float alfa_sombra = std::min(0.5f, sinf(kAnguloInclinacao));
  // Matriz eh column major, ou seja, esta invertida.
  // A ideia eh adicionar ao x a altura * fator de shear.
  GLfloat matriz_shear[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    fator_shear * -cosf(kAnguloPosicao), fator_shear * -sinf(kAnguloPosicao), 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
  };
  // Habilita o stencil para desenhar apenas uma vez as sombras.
  gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
  LigaStencil();
  DesenhaEntidadesBase(
      std::bind(&Entidade::DesenhaSombra, std::placeholders::_1, std::placeholders::_2, matriz_shear),
      true);
  // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
  GLfloat cor_sombra[] = { 0.0f, 0.0f, 0.0f, alfa_sombra };
  //gl::HabilitaEscopo habilita_blend(GL_BLEND);
  DesenhaStencil(cor_sombra);
}

void Tabuleiro::AtualizaOlho(bool forcar) {
  if (!forcar && !olho_.has_destino()) {
    return;
  }
  auto* pos_alvo = olho_.mutable_alvo();
  double origem[] = { pos_alvo->x(), pos_alvo->y(), pos_alvo->z() };
  if (olho_.has_destino()) {
    const auto& pd = olho_.destino();
    double destino[] = { pd.x(), pd.y(), pd.z() };
    bool chegou = true;
    for (int i = 0; i < 3; ++i) {
      double delta = (origem[i] > destino[i]) ? -VELOCIDADE_POR_EIXO : VELOCIDADE_POR_EIXO;
      if (fabs(origem[i] - destino[i]) > VELOCIDADE_POR_EIXO * 3) {
        origem[i] += delta * 3;
        chegou = false;
      } else if (fabs(origem[i] - destino[i]) > VELOCIDADE_POR_EIXO) {
        origem[i] += delta;
        chegou = false;
      } else {
        origem[i] = destino[i];
      }
    }
    if (chegou) {
      olho_.clear_destino();
    }
  }
  pos_alvo->set_x(origem[0]);
  pos_alvo->set_y(origem[1]);
  pos_alvo->set_z(origem[2]);

  Posicao* pos_olho = olho_.mutable_pos();;
  pos_olho->set_x(pos_alvo->x() + cosf(olho_.rotacao_rad()) * olho_.raio());
  pos_olho->set_y(pos_alvo->y() + sinf(olho_.rotacao_rad()) * olho_.raio());
  pos_olho->set_z(olho_.altura());
}

void Tabuleiro::AtualizaRaioOlho(float raio) {
  if (raio < OLHO_RAIO_MINIMO) {
    raio = OLHO_RAIO_MINIMO;
  }
  else if (raio > OLHO_RAIO_MAXIMO) {
    raio = OLHO_RAIO_MAXIMO;
  }
  olho_.set_raio(raio);
  AtualizaOlho(true  /*forcar*/);
}

void Tabuleiro::AtualizaEntidades() {
  for (auto& id_ent : entidades_) {
    id_ent.second->Atualiza();
  }
}

void Tabuleiro::AtualizaAcoes() {
  // Qualquer acao adicionada aqui ja foi colocada na lista de desfazer durante a criacao.
  ignorar_lista_eventos_ = true;
  std::vector<std::unique_ptr<Acao>> copia_acoes;
  copia_acoes.swap(acoes_);
  bool limpar_salvacoes = false;
  for (auto& acao : copia_acoes) {
    acao->Atualiza();
    if (acao->AtingiuAlvo()) {
      acao->AlvoProcessado();
      const auto& ap = acao->Proto();
      if (ap.id_entidade_destino_size() > 0 &&
          ap.afeta_pontos_vida()) {
        if (ap.permite_salvacao()) {
          limpar_salvacoes = true;
        }
        for (auto id_entidade_destino : ap.id_entidade_destino()) {
          AtualizaPontosVidaEntidadePorAcao(*acao, id_entidade_destino, ap.delta_pontos_vida());
        }
      }
    }
    // Apenas acoes nao finalizadas sao mantidas para a proxima iteracao.
    if (!acao->Finalizada()) {
      acoes_.push_back(std::unique_ptr<Acao>(acao.release()));
    }
  }
  if (limpar_salvacoes) {
    ntf::Notificacao ntf;
    ntf.set_tipo(ntf::TN_LIMPAR_SALVACOES);
    TrataNotificacao(ntf);
  }
  ignorar_lista_eventos_ = false;
  VLOG(3) << "Numero de acoes ativas: " << acoes_.size();
}

// Esta operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador e a
// profundidade de quem o acertou.
void Tabuleiro::EncontraHits(int x, int y, unsigned int* numero_hits, unsigned int* buffer_hits) {

  // inicia o buffer de picking (selecao)
  gl::BufferSelecao(100, buffer_hits);
  // entra no modo de selecao e limpa a pilha de nomes e inicia com 0
  gl::ModoRenderizacao(gl::MR_SELECT);

  gl::ModoMatriz(GL_PROJECTION);
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::CarregaIdentidade();
  gl::MatrizPicking(x, y, 1.0, 1.0, viewport);
  ConfiguraProjecao();

  // desenha a cena sem firulas.
  parametros_desenho_.set_picking_x(x);
  parametros_desenho_.set_picking_y(y);
  parametros_desenho_.set_iluminacao(false);
  parametros_desenho_.set_desenha_texturas(false);
  parametros_desenho_.set_desenha_grade(false);
  parametros_desenho_.set_desenha_fps(false);
  parametros_desenho_.set_desenha_aura(false);
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_transparencias(false);
  parametros_desenho_.set_desenha_acoes(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_quadrado_selecao(false);
  parametros_desenho_.set_desenha_rastro_movimento(false);
  parametros_desenho_.set_desenha_forma_selecionada(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_nevoa(false);
  parametros_desenho_.set_desenha_id_acao(false);
  parametros_desenho_.set_desenha_detalhes(false);
  parametros_desenho_.set_desenha_eventos_entidades(true);
  DesenhaCena();

  // Volta pro modo de desenho, retornando quanto pegou no SELECT.
  *numero_hits = gl::ModoRenderizacao(gl::MR_RENDER);

  // Restaura projecao manualmente por causa da pilha pequena.
  gl::ModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  ConfiguraProjecao();
}

void Tabuleiro::BuscaHitMaisProximo(
    int x, int y, unsigned int* id, unsigned int* tipo_objeto, float* profundidade) {
  GLuint buffer_hits[100] = {0};
  GLuint numero_hits = 0;
  EncontraHits(x, y, &numero_hits, buffer_hits);
  // Cada hit ocupa pelo menos 4 inteiros do buffer. Na pratica, por causa da pilha vao ocupar ate mais.
  if (numero_hits > 25) {
    LOG(WARNING) << "Muitos hits para a posicao, tamanho de buffer de selecao invalido.";
    *tipo_objeto = 0;
    *id = 0;
    return;
  }

  // Busca o hit mais próximo em buffer_hits. Cada posicao do buffer (hit record):
  // - 0: pos_pilha de nomes (numero de nomes empilhados);
  // - 1: profundidade minima.
  // - 2: profundidade maxima.
  // - 3: nomes empilhados (1 para cada pos pilha).
  // Dado o hit mais proximo, retorna o identificador, a posicao da pilha e a
  // profundidade do objeto (normalizado 0..1.0).
  VLOG(2) << "numero de hits no buffer de picking: " << numero_hits;
  GLuint* ptr_hits = buffer_hits;
  // valores do hit mais proximo.
  GLuint menor_z = 0xFFFFFFFF;
  GLuint tipo_objeto_menor = 0;
  GLuint id_menor = 0;

  // Busca o hit mais proximo.
  for (GLuint i = 0; i < numero_hits; ++i) {
    GLuint pos_pilha_corrente = *ptr_hits;
    ++ptr_hits;
    if (pos_pilha_corrente != 2) {
      LOG(ERROR) << "Tamanho da pilha diferente de 2: " << pos_pilha_corrente;
      *tipo_objeto = 0;
      *id = 0;
      return;
    }
    GLuint z_corrente = *ptr_hits;
    ptr_hits += 2;  // pula maximo.
    // Tipo do objeto do hit.
    GLuint tipo_corrente = *ptr_hits;
    ++ptr_hits;
    // Id do objeto.
    GLuint id_corrente = *ptr_hits;
    ++ptr_hits;

    if (z_corrente <= menor_z) {
      VLOG(3) << "tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
      menor_z = z_corrente;
      tipo_objeto_menor = tipo_corrente;
      id_menor = id_corrente;
    } else {
      VLOG(3) << "Pulando objeto, tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
    }
  }
  *tipo_objeto = tipo_objeto_menor;
  *id = id_menor;
  float menor_profundidade = 0.0f;
  // Converte profundidade de inteiro para float.
  // No OpenGL ES a profundidade retornada vai ser sempre zero. Se nao houver hit, menor_z vai ser 0xFFFFFFFF
  // e a profundidade maxima sera retornada.
  menor_profundidade = static_cast<float>(menor_z) / static_cast<float>(0xFFFFFFFF);
  if (profundidade != nullptr) {
    *profundidade = menor_profundidade;
  }
  VLOG(1) << "Retornando menor profundidade: " << menor_profundidade
          << ", tipo_objeto: " << tipo_objeto_menor 
          << ", id: " << id_menor;
}

bool Tabuleiro::MousePara3d(int x, int y, float* x3d, float* y3d, float* z3d) {
  GLuint id;
  GLuint pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
#if !USAR_OPENGL_ES
  return MousePara3dComProfundidade(x, y, profundidade, x3d, y3d, z3d);
#else
  return MousePara3dComId(x, y, id, pos_pilha, x3d, y3d, z3d);
#endif
}

bool Tabuleiro::MousePara3dTabuleiro(int x, int y, float* x3d, float* y3d, float* z3d) {
  // Intersecao de reta com plano z=0.
#if !USAR_OPENGL_ES
  GLdouble modelview[16], projection[16];
#else
  GLfloat modelview[16], projection[16];
#endif
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  float p1x, p1y, p1z;
  gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
  float p2x, p2y, p2z;
  gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
  if (p2z - p1z == 0) {
    LOG(ERROR) << "Retornando lixo";
    return false;
  }
  float mult = (parametros_desenho_.offset_terreno() - p1z) / (p2z - p1z);
  *x3d = p1x + (p2x - p1x) * mult;
  *y3d = p1y + (p2y - p1y) * mult;
  *z3d = parametros_desenho_.offset_terreno();
  VLOG(2) << "Retornando tabuleiro: " << *x3d << ", " << *y3d << ", " << *z3d;
  return true;
}

#if !USAR_OPENGL_ES
bool Tabuleiro::MousePara3dComProfundidade(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d) {
  GLdouble modelview[16], projection[16];
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  if (!gl::Desprojeta(x, y, profundidade,
                      modelview, projection, viewport,
                      x3d, y3d, z3d)) {
    LOG(ERROR) << "Falha ao projetar x y no mundo 3d.";
    return false;
  }
  VLOG(2) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}
#else
bool Tabuleiro::MousePara3dComId(int x, int y, unsigned int id, unsigned int pos_pilha, float* x3d, float* y3d, float* z3d) {
  // Busca mais detalhado.
  if (pos_pilha == 1) {
    MousePara3dTabuleiro(x, y, x3d, y3d, z3d);
  } else {
#if !USAR_OPENGL_ES
    GLdouble modelview[16], projection[16];
#else
    GLfloat modelview[16], projection[16];
#endif
    GLint viewport[4];
    gl::Le(GL_MODELVIEW_MATRIX, modelview);
    gl::Le(GL_PROJECTION_MATRIX, projection);
    gl::Le(GL_VIEWPORT, viewport);
    // Raio que sai do pixel.
    float p1x, p1y, p1z;
    gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
    if (camera_isometrica_) {
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }

    float p2x, p2y, p2z;
    gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
    if (p2z - p1z == 0) {
      LOG(ERROR) << "Retornando lixo";
      return false;
    }
    // Equacao parametrica do raio. Substituindo os p1* por x0, y0 e z0 e os p2* por x, y e z, temos:
    // x = x0 + at
    // y = y0 + bt
    // z = z0 + ct
    float a_raio = p2x - p1x;
    float b_raio = p2y - p1y;
    float c_raio = p2z - p1z;

    auto* e = BuscaEntidade(id);
    if (e == nullptr) {
      LOG(ERROR) << "Retornando lixo porque nao achei a entidade";
      return false;
    }
    // Cria um plano perpendicular a linha de visao para o objeto e com o plano XY.
    // Equacao do olho para o objeto. a_olho_obj * x + b_olho_obj = y.
    // Equacao da perdicular: a_perpendicular * x + b_perpendicular = y.
    //                         onde a_perpendicular = -1 / a_olho_obj.
    float a_perpendicular = (fabs(olho_.pos().x() -  e->X()) < 0.0001f) ?
        0.0f : (-1.0f / (olho_.pos().y() - e->Y()) / (olho_.pos().x() - e->X()));
    float b_perpendicular = e->Y() - e->X() * a_perpendicular;

    // Valor do t da intersecao.: onde a equacao perpendicular encontra com o plano.
    // (para simplicar nomenclatura, p = a_perpendicular, q = b_perpendicular, a = a_raio, b = b_raio).
    // (x0 = p1x, y0 = p1y, z0 = p1z).
    // y = y0 + bt = px + q;
    // t = (px + q - y0) / b. (1)
    // Como: x = x0 + at,
    // entao t = (x - x0) / a. (2)
    // Igualando (1) e (2):
    // (px + q - y0) / b = (x - x0) / a;
    // apx + aq - ay0 = bx - bx0;
    // apx - bx = ay0 - aq - bx0;
    // x (ap - b) = ay0 - aq - bx0;
    // Portanto, o x da intercessao eh (tanto faz, eh so multiplicar acima por -1 dos dois lados):
    // x = (ay0 - aq - bx0) / (ap - b) ou
    // x = (aq - ay0 + bx0) / (b - ap)
    if (fabs(b_raio - a_raio * a_perpendicular) < 0.0001f) {
      LOG(WARNING) << "Projecao praticamente perpendicular, retornando valores simples";
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }
    float x_inter = (a_raio * b_perpendicular - a_raio * p1y + b_raio * p1x) / (b_raio - a_raio * a_perpendicular);
    // Valor do t para interceptar o plano perpendicular
    float t_inter = (x_inter - p1x) / a_raio;
    // Outros valores da intersecao.
    float y_inter = p1y + b_raio * t_inter;
    float z_inter = p1z + c_raio * t_inter;

    *x3d = x_inter;
    *y3d = y_inter;
    *z3d = z_inter;
  }
  // Importante para operacoes no mesmo frame nao se confundirem.
  VLOG(2) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}
#endif

void Tabuleiro::TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao) {
  if (modo_acao_) {
    TrataBotaoAcaoPressionado(false, x, y);
    modo_acao_ = false;
    return;
  }
  ultimo_x_ = x;
  ultimo_y_ = y;

  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  float x3d, y3d, z3d;
#if !USAR_OPENGL_ES
  MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
#else
  MousePara3dComId(x, y, id, pos_pilha, &x3d, &y3d, &z3d);
#endif
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  ultimo_z_3d_ = z3d;
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;
  if (pos_pilha == OBJ_TABULEIRO) {
    // Tabuleiro.
    // Converte x3d y3d para id quadrado.
    SelecionaQuadrado(IdQuadrado(x3d, y3d));
  } else if (pos_pilha == OBJ_ENTIDADE) {
    // Entidade.
    VLOG(1) << "Picking entidade id " << id;
    if (alterna_selecao) {
      AlternaSelecaoEntidade(id);
    } else {
      if (!EntidadeEstaSelecionada(id)) {
        // Se nao estava selecionada, so ela.
        SelecionaEntidade(id);
      }
      // Se nao, pode mover mais.
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          // Forma nao deixa rastro.
          continue;
        }
        Posicao pos;
        pos.set_x(entidade_selecionada->X());
        pos.set_y(entidade_selecionada->Y());
        pos.set_z(ZChao(pos.x(), pos.y()));
        rastros_movimento_[id].push_back(pos);
      }
      estado_ = ETAB_ENTS_PRESSIONADAS;
    }
  } else if (pos_pilha == OBJ_ROLAGEM) {
    VLOG(1) << "Picking em ponto de rolagem id " << id;
    TrataRolagem(static_cast<dir_rolagem_e>(id));
  } else if (pos_pilha == OBJ_CONTROLE_VIRTUAL) {
    VLOG(1) << "Picking no controle virtual " << id;
    PickingControleVirtual(alterna_selecao, id);
  } else if (pos_pilha == OBJ_EVENTO_ENTIDADE) {
    VLOG(1) << "Picking em evento da entidade " << id;
    ApagaEventosZeradosDeEntidadeNotificando(id);
  } else {
    VLOG(1) << "Picking lugar nenhum.";
    DeselecionaEntidades();
  }
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataBotaoDireitoPressionado(int x, int y) {
  VLOG(1) << "Botao direito pressionado";
  ultimo_x_ = x;
  ultimo_y_ = y;

  estado_anterior_ = estado_;
  float x3d, y3d, z3d;
  parametros_desenho_.set_desenha_entidades(false);
  MousePara3dTabuleiro(x, y, &x3d, &y3d, &z3d);
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  estado_ = ETAB_DESLIZANDO;
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataBotaoRotacaoPressionado(int x, int y) {
  VLOG(1) << "Botao rotacao pressionado";
  primeiro_x_ = x;
  primeiro_y_ = y;
  ultimo_x_ = x;
  ultimo_y_ = y;
  if (estado_ == ETAB_ENTS_PRESSIONADAS) {
    FinalizaEstadoCorrente();
    estado_ = ETAB_ENTS_TRANSLACAO_ROTACAO;
    estado_anterior_ = ETAB_ENTS_SELECIONADAS;
    translacao_rotacao_ = TR_NENHUM;
    translacoes_rotacoes_antes_.clear();
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        continue;
      }
      translacoes_rotacoes_antes_.insert(
          std::make_pair(entidade->Id(),
                         std::make_pair(entidade->TranslacaoZ(), entidade->RotacaoZGraus())));
    }
  } else {
    estado_anterior_ = estado_;
    estado_ = ETAB_ROTACAO;
  }
}

void Tabuleiro::TrataBotaoDesenhoPressionado(int x, int y) {
  if (!modo_mestre_) {
    VLOG(1) << "Apenas mestre pode desenhar.";
    // Apenas mestre pode desenhar.
    return;
  }
  VLOG(1) << "Botao desenho pressionado";
  ultimo_x_ = x;
  ultimo_y_ = y;
  float x3d, y3d, z3d;
  MousePara3d(x, y, &x3d, &y3d, &z3d);
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  forma_proto_.Clear();
  forma_proto_.set_id(GeraIdEntidade(id_cliente_));
  forma_proto_.set_tipo(TE_FORMA);
  forma_proto_.set_sub_tipo(forma_selecionada_);
  auto* pos = forma_proto_.mutable_pos();
  pos->set_x(primeiro_x_3d_);
  pos->set_y(primeiro_y_3d_);
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
    float x3d, y3d, z3d;
    MousePara3dTabuleiro(x, y, &x3d, &y3d, &z3d);
    // Tabuleiro: cria uma entidade nova.
    SelecionaQuadrado(IdQuadrado(x3d, y3d));
    estado_ = ETAB_QUAD_SELECIONADO;
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    TrataNotificacao(notificacao);
  } else if (pos_pilha == OBJ_ENTIDADE) {
    // Entidade.
    if (SelecionaEntidade(id, true  /*forcar_fixa*/)) {
      auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
      n->set_modo_mestre(modo_mestre_);
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      central_->AdicionaNotificacao(n);
    }
  } if (pos_pilha == OBJ_CONTROLE_VIRTUAL) {
    PickingControleVirtual(false  /*alterna selecao*/, id);
  } else {
    ;
  }
}

void Tabuleiro::TrataDuploCliqueDireito(int x, int y) {
  parametros_desenho_.set_desenha_entidades(false);
  float x3d, y3d, z3d;
  if (!MousePara3d(x, y, &x3d, &y3d, &z3d)) {
    return;
  }
  auto* p = olho_.mutable_destino();
  p->set_x(x3d);
  p->set_y(y3d);
  p->set_z(z3d);
}

bool Tabuleiro::SelecionaEntidade(unsigned int id, bool forcar_fixa) {
  VLOG(2) << "Selecionando entidade: " << id;
  ids_entidades_selecionadas_.clear();
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }
  if ((!forcar_fixa && entidade->Fixa()) || (!modo_mestre_ && !entidade->SelecionavelParaJogador())) {
    DeselecionaEntidades();
    return false;
  }
  ids_entidades_selecionadas_.insert(entidade->Id());
  quadrado_selecionado_ = -1;
  estado_ = ETAB_ENTS_SELECIONADAS;
  return true;
}

void Tabuleiro::SelecionaEntidades(const std::vector<unsigned int>& ids) {
  if (ids.empty()) {
    DeselecionaEntidades();
    return;
  }
  if (ids.size() == 1) {
    SelecionaEntidade(ids[0]);
    return;
  }
  VLOG(2) << "Selecionando entidades";
  ids_entidades_selecionadas_.clear();
  AdicionaEntidadesSelecionadas(ids);
}

void Tabuleiro::AdicionaEntidadesSelecionadas(const std::vector<unsigned int>& ids) {
  for (unsigned int id : ids) {
    auto* entidade = BuscaEntidade(id);
    if (entidade == nullptr || entidade->Fixa() || (!modo_mestre_ && !entidade->SelecionavelParaJogador())) {
      continue;
    }
    ids_entidades_selecionadas_.insert(id);
  }
  MudaEstadoAposSelecao();
}

void Tabuleiro::AtualizaSelecaoEntidade(unsigned int id) {
  if (!EntidadeEstaSelecionada(id)) {
    return;
  }
  auto* e = BuscaEntidade(id);
  if (e == nullptr || (!modo_mestre_ && !e->SelecionavelParaJogador())) {
    ids_entidades_selecionadas_.erase(id);
  }
  MudaEstadoAposSelecao();
}

void Tabuleiro::AlternaSelecaoEntidade(unsigned int id) {
  VLOG(1) << "Selecionando entidade: " << id;
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }

  if (EntidadeEstaSelecionada(id)) {
    ids_entidades_selecionadas_.erase(id);
    MudaEstadoAposSelecao();
  } else {
    AdicionaEntidadesSelecionadas({id});
  }
}

void Tabuleiro::MudaEstadoAposSelecao() {
  // Alterna o estado. Note que eh possivel que essa chamada ocorra durante uma rotacao com botao do meio (ETAB_ROTACAO).
  VLOG(2) << "Estado antes mudanca: " << StringEstado(estado_);
  if (ids_entidades_selecionadas_.empty()) {
    if (estado_ == ETAB_ENTS_SELECIONADAS) {
      estado_ = ETAB_OCIOSO;
    }
  } else {
    if (estado_ == ETAB_OCIOSO || estado_ == ETAB_QUAD_PRESSIONADO || estado_ == ETAB_QUAD_SELECIONADO) {
      estado_ = ETAB_ENTS_SELECIONADAS;
    }
  }
  VLOG(2) << "Estado apos mudanca: " << StringEstado(estado_);
  quadrado_selecionado_ = -1;
}

void Tabuleiro::FinalizaEstadoCorrente() {
  switch (estado_) {
    case ETAB_ENTS_TRANSLACAO_ROTACAO: {
      if (translacao_rotacao_ == TR_NENHUM) {
        // Nada a fazer.
      } else {
        ntf::Notificacao grupo_notificacoes;
        grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
        for (unsigned int id : ids_entidades_selecionadas_) {
          auto* entidade = BuscaEntidade(id);
          if (entidade == nullptr || entidade->Tipo() == TE_ENTIDADE) {
            continue;
          }
          auto* n = grupo_notificacoes.add_notificacao();
          n->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
          auto* e_antes = n->mutable_entidade_antes();
          e_antes->CopyFrom(entidade->Proto());
          e_antes->set_translacao_z(translacoes_rotacoes_antes_[entidade->Id()].first);
          e_antes->set_rotacao_z_graus(translacoes_rotacoes_antes_[entidade->Id()].second);
          // A entidade ja foi alterada durante a rotacao.
          n->mutable_entidade()->CopyFrom(entidade->Proto());
        }
        // Vai ser um nop, mas envia as notificacoes para os clientes.
        TrataNotificacao(grupo_notificacoes);
        // Para desfazer.
        AdicionaNotificacaoListaEventos(grupo_notificacoes);
      }
      estado_ = estado_anterior_;
      return;
    }
    case ETAB_ROTACAO:
      estado_ = estado_anterior_;
      return;
    case ETAB_DESLIZANDO:
      estado_ = estado_anterior_;
      return;
    case ETAB_ENTS_PRESSIONADAS: {
      if (!camera_presa_ && id_camera_presa_ != Entidade::IdInvalido) {
        // Restaura camera presa antes do movimento.
        camera_presa_ = true;
      }
      if (primeiro_x_3d_ == ultimo_x_3d_ &&
          primeiro_y_3d_ == ultimo_y_3d_) {
        // Nao houve movimento.
        estado_ = ETAB_ENTS_SELECIONADAS;
        rastros_movimento_.clear();
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
        auto* pos_final = n_desfazer->mutable_entidade()->mutable_destino();
        pos_final->set_x(entidade_selecionada->X());
        pos_final->set_y(entidade_selecionada->Y());
        pos_final->set_z(entidade_selecionada->Z());
        auto* pos_original = n_desfazer->mutable_entidade()->mutable_pos();
        pos_original->set_x(entidade_selecionada->X() - vetor_delta.x());
        pos_original->set_y(entidade_selecionada->Y() - vetor_delta.y());
        pos_original->set_z(entidade_selecionada->Z() - vetor_delta.z());
      }
      AdicionaNotificacaoListaEventos(g_desfazer);
      estado_ = ETAB_ENTS_SELECIONADAS;
      rastros_movimento_.clear();
      parametros_desenho_.Clear();
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
      VLOG(1) << "Finalizando: " << forma_proto_.ShortDebugString();
      forma_proto_.mutable_cor()->CopyFrom(forma_cor_);
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      n.mutable_entidade()->Swap(&forma_proto_);
      TrataNotificacao(n);
      return;
    }
    default:
      //estado_ = ETAB_OCIOSO;
      ;
  }
}

bool Tabuleiro::EntidadeEstaSelecionada(unsigned int id) {
  return ids_entidades_selecionadas_.find(id) != ids_entidades_selecionadas_.end();
}

void Tabuleiro::DeselecionaEntidades() {
  ids_entidades_selecionadas_.clear();
  quadrado_selecionado_ = -1;
  estado_ = ETAB_OCIOSO;
}

void Tabuleiro::DeselecionaEntidade(unsigned int id) {
  ids_entidades_selecionadas_.erase(id);
  if (!ids_entidades_selecionadas_.empty()) {
    estado_ = ETAB_OCIOSO;
  }
}

void Tabuleiro::SelecionaQuadrado(int id_quadrado) {
  quadrado_selecionado_ = id_quadrado;
  ids_entidades_selecionadas_.clear();
  estado_ = ETAB_QUAD_PRESSIONADO;
}

unsigned int Tabuleiro::IdQuadrado(float x, float y) {
  float inicio_x = -(TamanhoX() * TAMANHO_LADO_QUADRADO) / 2.0f;
  float delta_x_float = x - inicio_x;
  int delta_x = delta_x_float / TAMANHO_LADO_QUADRADO;
  if (delta_x < 0 || delta_x >= TamanhoX()) {
    LOG(ERROR) << "Posicao invalida para tabuleiro, x: " << x;
    return -1;
  }
  float inicio_y = -(TamanhoY() * TAMANHO_LADO_QUADRADO) / 2.0f;
  float delta_y_float = y - inicio_y;
  int delta_y = delta_y_float / TAMANHO_LADO_QUADRADO;
  if (delta_y >= TamanhoY()) {
    LOG(ERROR) << "Posicao invalida para tabuleiro, y: " << y;
    return -1;
  }
  return delta_y * TamanhoX() + delta_x;
}

void Tabuleiro::CoordenadaQuadrado(unsigned int id_quadrado, float* x, float* y, float* z) {
  int quad_x = id_quadrado % TamanhoX();
  int quad_y = id_quadrado / TamanhoX();
  VLOG(2) << "id_quadrado: " << id_quadrado << ", quad_x: " << quad_x << ", quad_y: " << quad_y;

  // centro do quadrado
  *x = ((quad_x * TAMANHO_LADO_QUADRADO) + TAMANHO_LADO_QUADRADO_2) -
       (TamanhoX() * TAMANHO_LADO_QUADRADO_2);
  *y = ((quad_y * TAMANHO_LADO_QUADRADO) + TAMANHO_LADO_QUADRADO_2) -
       (TamanhoY() * TAMANHO_LADO_QUADRADO_2);
  *z = parametros_desenho_.offset_terreno();
}

ntf::Notificacao* Tabuleiro::SerializaPropriedades() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO);
  auto* tabuleiro = notificacao->mutable_tabuleiro();
  tabuleiro->set_id_cliente(id_cliente_);
  tabuleiro->mutable_luz_ambiente()->CopyFrom(proto_.luz_ambiente());
  tabuleiro->mutable_luz_direcional()->CopyFrom(proto_.luz_direcional());
  if (proto_.has_info_textura()) {
    tabuleiro->mutable_info_textura()->CopyFrom(proto_.info_textura());
    tabuleiro->set_ladrilho(proto_.ladrilho());
    tabuleiro->set_textura_mestre_apenas(proto_.textura_mestre_apenas());
  }
  if (proto_.has_info_textura_ceu()) {
    tabuleiro->mutable_info_textura_ceu()->CopyFrom(proto_.info_textura_ceu());
  }
  if (proto_.has_nevoa()) {
    tabuleiro->mutable_nevoa()->CopyFrom(proto_.nevoa());
  }
  tabuleiro->set_largura(proto_.largura());
  tabuleiro->set_altura(proto_.altura());
  tabuleiro->set_desenha_grade(proto_.desenha_grade());
  return notificacao;
}

ntf::Notificacao* Tabuleiro::SerializaOpcoes() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_OPCOES);
  notificacao->mutable_opcoes()->CopyFrom(opcoes_);
  return notificacao;
}

void Tabuleiro::DeserializaPropriedades(const ent::TabuleiroProto& novo_proto) {
  VLOG(1) << "Atualizando propriedades: " << novo_proto.ShortDebugString();
  proto_.mutable_luz_ambiente()->CopyFrom(novo_proto.luz_ambiente());
  proto_.mutable_luz_direcional()->CopyFrom(novo_proto.luz_direcional());
  proto_.set_largura(novo_proto.largura());
  proto_.set_altura(novo_proto.altura());
  proto_.set_desenha_grade(novo_proto.desenha_grade());
  if (novo_proto.has_nevoa()) {
    proto_.mutable_nevoa()->CopyFrom(novo_proto.nevoa());
  } else {
    proto_.clear_nevoa();
  }
  AtualizaTexturas(novo_proto);
  RegeraVbo();
}

ntf::Notificacao* Tabuleiro::SerializaTabuleiro(const std::string& nome) {
  auto* notificacao = new ntf::Notificacao;
  try {
    notificacao->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    auto* t = notificacao->mutable_tabuleiro();
    t->CopyFrom(proto_);
    if (t->info_textura().has_bits_crus()) {
      // Serializa apenas os bits crus.
      t->mutable_info_textura()->clear_bits();
    }
    t->clear_entidade();  // As entidades vem do mapa de entidades.
    for (const auto& id_ent : entidades_) {
      t->add_entidade()->CopyFrom(id_ent.second->Proto());
    }
    if (!nome.empty()) {
      t->set_nome(nome);
    }
    VLOG(1) << "Serializando tabuleiro " << t->ShortDebugString();
    return notificacao;
  } catch (const std::logic_error& error) {
    notificacao->set_tipo(ntf::TN_ERRO);
    notificacao->set_erro(error.what());
    return notificacao;
  }
}

// Aqui ocorre a deserializacao do tabuleiro todo. As propriedades como iluminacao sao atualizadas
// na funcao Tabuleiro::DeserializaPropriedades.
void Tabuleiro::DeserializaTabuleiro(const ntf::Notificacao& notificacao) {
  const auto& tabuleiro = notificacao.tabuleiro();
  bool manter_entidades = tabuleiro.manter_entidades();
  if (manter_entidades) {
    VLOG(1) << "Deserializando tabuleiro mantendo entidades: " << tabuleiro.ShortDebugString();
  } else {
    VLOG(1) << "Deserializando tabuleiro todo: " << tabuleiro.ShortDebugString();
    EstadoInicial();
  }
  if (notificacao.has_erro()) {
    auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
    ne->set_erro(std::string("Erro ao deserializar tabuleiro: ") + notificacao.erro());
    central_->AdicionaNotificacao(ne);
    auto* n = ntf::NovaNotificacao(ntf::TN_DESCONECTAR);
    central_->AdicionaNotificacao(n);
    return;
  }
  AtualizaTexturas(tabuleiro);
  proto_.CopyFrom(tabuleiro);
  if (proto_.has_camera_inicial()) {
    ReiniciaCamera();
  }
  proto_.clear_manter_entidades();  // Os clientes nao devem receber isso.
  proto_.clear_entidade();  // As entidades serao armazenadas abaixo.
  proto_.clear_id_cliente();
  RegeraVbo();
  bool usar_id = !notificacao.has_endereco();  // Se nao tem endereco, veio da rede.
  if (usar_id && id_cliente_ == 0) {
    // So usa o id novo se nao tiver.
    VLOG(1) << "Alterando id de cliente para " << tabuleiro.id_cliente();
    id_cliente_ = tabuleiro.id_cliente();
  }

  // Remove as entidades do tabuleiro corrente.
  std::vector<unsigned int> entidades_a_remover;
  for (const auto& id_entidade : entidades_) {
    if (manter_entidades && id_entidade.second->SelecionavelParaJogador()) {
      continue;
    }
    entidades_a_remover.push_back(id_entidade.first);
  }
  for (unsigned int id : entidades_a_remover) {
    RemoveEntidade(id);
  }

  // Recebe as entidades.
  for (EntidadeProto ep : tabuleiro.entidade()) {
    if (manter_entidades) {
      if (ep.selecionavel_para_jogador()) {
        continue;
      }
      // Para manter as entidades, os ids tem que ser regerados para as entidades do tabuleiro,
      // senao pode dar conflito.
      ep.set_id(GeraIdEntidade(id_cliente_));
    }
    auto* e = NovaEntidade(ep, texturas_, central_);
    if (!entidades_.insert(std::make_pair(e->Id(), std::unique_ptr<Entidade>(e))).second) {
      LOG(ERROR) << "Erro adicionando entidade: " << ep.ShortDebugString();
    }
  }
  VLOG(1) << "Foram adicionadas " << tabuleiro.entidade_size() << " entidades";
}

ntf::Notificacao* Tabuleiro::SerializaEntidadesSelecionaveis() const {
  std::unique_ptr<ntf::Notificacao> n(ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_ENTIDADES_SELECIONAVEIS));
  for (const auto& id_e : entidades_) {
    if (id_e.second->SelecionavelParaJogador()) {
      n->mutable_tabuleiro()->add_entidade()->CopyFrom(id_e.second->Proto());
    }
  }
  return n.release();
}

void Tabuleiro::DeserializaEntidadesSelecionaveis(const ntf::Notificacao& n) {
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  float media_x = 0.0f;
  float media_y = 0.0f;
  int num = 0;
  for (const auto& e : n.tabuleiro().entidade()) {
    if (e.selecionavel_para_jogador()) {
      ++num;
      ntf::Notificacao* n_adicao = grupo_notificacoes.add_notificacao();
      n_adicao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      n_adicao->mutable_entidade()->CopyFrom(e);
      media_x += e.pos().x();
      media_y += e.pos().y();
    }
  }
  if (num == 0) {
    return;
  }
  // Poe entidades onde a camera olha.
  media_x /= num;
  media_y /= num;
  for (auto& n : *grupo_notificacoes.mutable_notificacao()) {
    float x_original = n.entidade().pos().x();
    float y_original = n.entidade().pos().y();
    n.mutable_entidade()->mutable_pos()->set_x(x_original - media_x + olho_.alvo().x());
    n.mutable_entidade()->mutable_pos()->set_y(y_original - media_y + olho_.alvo().y());
  }

  // Hack para entidades aparecerem visiveis e selecionaveis.
  bool modo_mestre_anterior = modo_mestre_;
  modo_mestre_ = false;
  TrataNotificacao(grupo_notificacoes);
  modo_mestre_ = modo_mestre_anterior;
  // Para desfazer
  {
    if (ids_adicionados_.size() == static_cast<unsigned int>(grupo_notificacoes.notificacao_size())) {
      for (int i = 0; i < grupo_notificacoes.notificacao_size(); ++i) {
        grupo_notificacoes.mutable_notificacao(i)->mutable_entidade()->set_id(ids_adicionados_[i]);
      }
      AdicionaNotificacaoListaEventos(grupo_notificacoes);
    } else {
      LOG(ERROR) << "Impossivel adicionar notificacao para desfazer porque o numero de entidades adicionadas difere do que foi tentado.";
    }
  }
}

void Tabuleiro::DeserializaOpcoes(const ent::OpcoesProto& novo_proto) {
  opcoes_.CopyFrom(novo_proto);
}

Entidade* Tabuleiro::BuscaEntidade(unsigned int id) {
  auto it = entidades_.find(id);
  return (it != entidades_.end()) ? it->second.get() : nullptr;
}

void Tabuleiro::CopiaEntidadesSelecionadas() {
#if USAR_QT
  ent::EntidadesCopiadas entidades_copiadas;
  for (const unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade = BuscaEntidade(id);
    if (entidade != nullptr) {
      entidades_copiadas.add_entidade()->CopyFrom(entidade->Proto());
      VLOG(1) << "Copiando: " << entidade->Proto().ShortDebugString();
    }
  }
  std::string str_entidades;
  google::protobuf::TextFormat::PrintToString(entidades_copiadas, &str_entidades);
  QApplication::clipboard()->setText(QString::fromStdString(str_entidades));
#else
  entidades_copiadas_.clear();
  for (const unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade = BuscaEntidade(id);
    if (entidade != nullptr) {
      entidades_copiadas_.push_back(entidade->Proto());
      VLOG(1) << "Copiando: " << entidade->Proto().ShortDebugString();
    }
  }
#endif
}

void Tabuleiro::ColaEntidadesSelecionadas() {
#if USAR_QT
  std::string str_entidades(QApplication::clipboard()->text().toStdString());
  if (str_entidades.empty()) {
    VLOG(1) << "Ignorando colar, não há entidades selecionadas";
    return;
  }
  ent::EntidadesCopiadas entidades_copiadas;
  if (!google::protobuf::TextFormat::ParseFromString(str_entidades, &entidades_copiadas)) {
    LOG(ERROR) << "Erro colando, nao consegui entender o buffer de clipboard: " << str_entidades;
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (const auto& ep : entidades_copiadas.entidade()) {
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    n->mutable_entidade()->CopyFrom(ep);
  }
#else
  if (entidades_copiadas_.empty()) {
    VLOG(1) << "Ignorando colar, não há entidades selecionadas";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (const auto& ep : entidades_copiadas_) {
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    n->mutable_entidade()->CopyFrom(ep);
  }
#endif
  TrataNotificacao(grupo_notificacoes);
  SelecionaEntidadesAdicionadas();
  // Para desfazer
  {
    if (ids_adicionados_.size() == static_cast<unsigned int>(grupo_notificacoes.notificacao_size())) {
      for (int i = 0; i < grupo_notificacoes.notificacao_size(); ++i) {
        grupo_notificacoes.mutable_notificacao(i)->mutable_entidade()->set_id(ids_adicionados_[i]);
      }
      AdicionaNotificacaoListaEventos(grupo_notificacoes);
    } else {
      LOG(ERROR) << "Impossivel adicionar notificacao para desfazer porque o numero de entidades adicionadas difere do que foi tentado.";
    }
  }

}

void Tabuleiro::AgrupaEntidadesSelecionadas() {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Estado invalido para agrupar: " << estado_;
    return;
  }
  VLOG(1) << "Agrupando entidades selecionadas.";
  EntidadeProto nova_entidade;
  nova_entidade.set_tipo(TE_COMPOSTA);
  float x_medio = 0;
  float y_medio = 0;
  int num_entidades = 0;
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* e = BuscaEntidade(id);
    if (e == nullptr) {
      continue;
    }
    auto* notificacao = grupo_notificacoes.add_notificacao();
    notificacao->set_tipo(ntf::TN_REMOVER_ENTIDADE);
    notificacao->mutable_entidade()->CopyFrom(e->Proto());
    x_medio += e->X();
    y_medio += e->Y();
    nova_entidade.add_sub_forma()->CopyFrom(e->Proto());
    ++num_entidades;
  }
  x_medio /= num_entidades;
  y_medio /= num_entidades;
  nova_entidade.mutable_pos()->set_x(x_medio);
  nova_entidade.mutable_pos()->set_y(y_medio);
  for (auto& sub_forma : *nova_entidade.mutable_sub_forma()) {
    sub_forma.mutable_pos()->set_x(sub_forma.pos().x() - x_medio);
    sub_forma.mutable_pos()->set_y(sub_forma.pos().y() - y_medio);
  }
  auto* notificacao = grupo_notificacoes.add_notificacao();
  notificacao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
  notificacao->mutable_entidade()->Swap(&nova_entidade);
  TrataNotificacao(grupo_notificacoes);
  {
    // para desfazer
    if (ids_adicionados_.size() == 1) {
      // So tem como desfazer se conseguiu adicionar a entidade.
      notificacao->mutable_entidade()->set_id(ids_adicionados_[0]);
      AdicionaNotificacaoListaEventos(grupo_notificacoes);
    } else {
      LOG(WARNING) << "Impossivel desfazer a entidade adicionada porque ela no foi criada.";
    }
  }
}

void Tabuleiro::DesagrupaEntidadesSelecionadas() {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Estado invalido para desagrupar: " << estado_;
    return;
  }
  VLOG(1) << "Desagrupando entidades selecionadas.";
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  unsigned int num_adicionados = 0;
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* e = BuscaEntidade(id);
    if (e == nullptr) {
      continue;
    }
    const auto& proto_composto = e->Proto();
    for (const auto& sub_entidade : proto_composto.sub_forma()) {
      auto* notificacao_adicao = grupo_notificacoes.add_notificacao();
      notificacao_adicao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      auto* nova_entidade = notificacao_adicao->mutable_entidade();
      nova_entidade->CopyFrom(sub_entidade);
      nova_entidade->clear_id();
      auto* pos = nova_entidade->mutable_pos();
      pos->set_x(pos->x() + proto_composto.pos().x());
      pos->set_y(pos->y() + proto_composto.pos().y());
      pos->set_z(pos->z() + proto_composto.pos().z());
      ++num_adicionados;
    }
    auto* notificacao_remocao = grupo_notificacoes.add_notificacao();
    notificacao_remocao->set_tipo(ntf::TN_REMOVER_ENTIDADE);
    notificacao_remocao->mutable_entidade()->CopyFrom(proto_composto);
  }
  TrataNotificacao(grupo_notificacoes);
  {
    // para desfazer
    if (ids_adicionados_.size() == num_adicionados) {
      int i = 0;
      for (auto& n : *grupo_notificacoes.mutable_notificacao()) {
        if (n.tipo() != ntf::TN_ADICIONAR_ENTIDADE) {
          continue;
        }
        n.mutable_entidade()->set_id(ids_adicionados_[i++]);
      }
      AdicionaNotificacaoListaEventos(grupo_notificacoes);
    } else {
      LOG(WARNING) << "Impossivel desfazer desagrupamento porque numero de adicionados difere.";
    }
  }
}

void Tabuleiro::TrataMovimentoEntidadesSelecionadas(bool vertical, float valor) {
  Posicao vetor_camera;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &vetor_camera);
  // angulo da camera em relacao ao eixo X.
  float rotacao_graus = VetorParaRotacaoGraus(vetor_camera.x(), vetor_camera.y());
  float dx = 0;
  float dy = 0;
  if (rotacao_graus > -45.0f && rotacao_graus <= 45.0f) {
    // Camera apontando para x positivo.
    if (vertical) {
      dx = TAMANHO_LADO_QUADRADO * valor;
    } else {
      dy = TAMANHO_LADO_QUADRADO * -valor;
    }
  } else if (rotacao_graus > 45.0f && rotacao_graus <= 135) {
    // Camera apontando para y positivo.
    if (vertical) {
      dy = TAMANHO_LADO_QUADRADO * valor;
    } else {
      dx = TAMANHO_LADO_QUADRADO * valor;
    }
  } else if (rotacao_graus > 135 || rotacao_graus < -135) {
    // Camera apontando para x negativo.
    if (vertical) {
      dx = TAMANHO_LADO_QUADRADO * -valor;
    } else {
      dy = TAMANHO_LADO_QUADRADO * valor;
    }
  } else {
    // Camera apontando para y negativo.
    if (vertical) {
      dy = TAMANHO_LADO_QUADRADO * -valor;
    } else {
      dx = TAMANHO_LADO_QUADRADO * -valor;
    }
  }
  // TODO direito com eventos.
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  VLOG(1) << "Movendo entidades selecionadas dx: " << dx << ", dy: " << dy;
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    VLOG(2) << "Movendo entidade " << id << ", dx: " << dx << ", dy: " << dy;
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_MOVER_ENTIDADE);
    auto* e = n->mutable_entidade();
    e->set_id(id);
    auto* p = e->mutable_destino();
    p->set_x(entidade_selecionada->X() + dx);
    p->set_y(entidade_selecionada->Y() + dy);
    p->set_z(entidade_selecionada->Z());
    // Para desfazer.
    p = e->mutable_pos();
    p->set_x(entidade_selecionada->X());
    p->set_y(entidade_selecionada->Y());
    p->set_z(entidade_selecionada->Z());
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::TrataTranslacaoZEntidadesSelecionadas(float delta) {
  // TODO UNDO, limites e enviar para clientes.
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    // Salva para desfazer.
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
    auto* e_antes = n->mutable_entidade_antes();
    e_antes->CopyFrom(entidade_selecionada->Proto());
    // Altera a translacao em Z.
    entidade_selecionada->AlteraTranslacaoZ(delta);
    n->mutable_entidade()->CopyFrom(entidade_selecionada->Proto());
  }
  // Nop mas envia para os clientes.
  TrataNotificacao(grupo_notificacoes);
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::AdicionaNotificacaoListaEventos(const ntf::Notificacao& notificacao) {
  if (processando_grupo_ || ignorar_lista_eventos_) {
    VLOG(2) << "Ignorando notificacao adicionada a lista de desfazer pois (processando_grupo_ || ignorar_lista_eventos_) == true";
    return;
  }
  if (evento_corrente_ != lista_eventos_.end()) {
    // Remove tudo do corrente para a frente.
    lista_eventos_.erase(evento_corrente_, lista_eventos_.end());
  }
  lista_eventos_.emplace_back(notificacao);
  evento_corrente_ = lista_eventos_.end();
  if (lista_eventos_.size() > TAMANHO_MAXIMO_LISTA) {
    VLOG(1) << "Limite de notificacoes da lista de desfazer atingigo, removendo cabeca";
    lista_eventos_.pop_front();
  }
  VLOG(1) << "Adicionando notificacao a lista de desfazer, tamanho: " << lista_eventos_.size()
          << ", notificacao: " << notificacao.ShortDebugString();
}

namespace {
// Controi a notificacao inversa para comando de desfazer.
const ntf::Notificacao InverteNotificacao(const ntf::Notificacao& n_original) {
  ntf::Notificacao n_inversa;
  n_inversa.set_tipo(ntf::TN_ERRO);
  switch (n_original.tipo()) {
    // Tipos de notificacao que podem ser desfeitas.
    case ntf::TN_GRUPO_NOTIFICACOES:
      n_inversa.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
      for (const auto& n : n_original.notificacao()) {
        n_inversa.add_notificacao()->CopyFrom(InverteNotificacao(n));
      }
      break;
    case ntf::TN_ATUALIZAR_RODADAS:
      VLOG(1) << "Invertendo TN_ATUALIZAR_RODADAS";
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_RODADAS);
      n_inversa.mutable_tabuleiro()->set_contador_rodadas(n_original.tabuleiro_antes().contador_rodadas());
      break;
    case ntf::TN_ADICIONAR_ENTIDADE:
      if (!n_original.entidade().has_id()) {
        LOG(ERROR) << "Impossivel inverter TN_ADICIONAR_ENTIDADE sem id da entidade.";
        break;
      }
      VLOG(1) << "Invertendo TN_ADICIONAR_ENTIDADE";
      n_inversa.set_tipo(ntf::TN_REMOVER_ENTIDADE);
      n_inversa.mutable_entidade()->set_id(n_original.entidade().id());
      break;
    case ntf::TN_REMOVER_ENTIDADE:
      if (!n_original.has_entidade()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_REMOVER_ENTIDADE sem proto da entidade";
        break;
      }
      n_inversa.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
      n_inversa.mutable_entidade()->CopyFrom(n_original.entidade());
      break;
    case ntf::TN_MOVER_ENTIDADE:
      if (!n_original.entidade().has_pos() || !n_original.entidade().has_id()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_MOVER_ENTIDADE sem a posicao original ou ID.";
        break;
      }
      n_inversa.set_tipo(ntf::TN_MOVER_ENTIDADE);
      // Usa o destino.
      n_inversa.mutable_entidade()->mutable_destino()->CopyFrom(n_original.entidade().pos());
      n_inversa.mutable_entidade()->set_id(n_original.entidade().id());
      break;
    case ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE:
      if (!n_original.has_entidade_antes()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE sem o proto novo e o proto anterior: "
                   << n_original.ShortDebugString();
        break;
      }
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
      n_inversa.mutable_entidade()->CopyFrom(n_original.entidade_antes());
      break;
    case ntf::TN_ATUALIZAR_ENTIDADE:
      if (!n_original.has_entidade_antes()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_ATUALIZAR_ENTIDADE sem o proto novo e o proto anterior: "
                   << n_original.ShortDebugString();
        break;
      }
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
      n_inversa.mutable_entidade()->CopyFrom(n_original.entidade_antes());
      break;
    default:
      break;
  }
  return n_inversa;
}
}

void Tabuleiro::TrataComandoDesfazer() {
  if (lista_eventos_.empty()) {
    VLOG(1) << "Lista de eventos vazia.";
    return;
  }
  if (evento_corrente_ == lista_eventos_.begin()) {
    VLOG(1) << "Lista de eventos vazia.";
    return;
  }
  --evento_corrente_;
  ignorar_lista_eventos_ = true;
  const ntf::Notificacao& n_original = *evento_corrente_;
  ntf::Notificacao n_inversa = InverteNotificacao(n_original);
  if (n_inversa.tipo() != ntf::TN_ERRO) {
    TrataNotificacao(n_inversa);
  } else {
    LOG(ERROR) << "Nao consegui desfazer notificacao: " << n_original.ShortDebugString();
  }
  ignorar_lista_eventos_ = false;
  VLOG(1) << "Notificacao desfeita: " << n_original.ShortDebugString() << ", tamanho lista: " << lista_eventos_.size();
}

void Tabuleiro::TrataComandoRefazer() {
  if (lista_eventos_.empty()) {
    VLOG(1) << "Lista de eventos vazia.";
    return;
  }
  if (evento_corrente_ == lista_eventos_.end()) {
    VLOG(1) << "Não há ações para refazer.";
    return;
  }
  ignorar_lista_eventos_ = true;
  const ntf::Notificacao& n_original = *evento_corrente_;
  TrataNotificacao(n_original);
  ignorar_lista_eventos_ = false;
  ++evento_corrente_;
}

void Tabuleiro::SelecionaTudo() {
  std::vector<unsigned int> ids;
  for (const auto& id_ent : entidades_) {
    ids.push_back(id_ent.first);
  }
  SelecionaEntidades(ids);
}

void Tabuleiro::MoveEntidadeNotificando(const ntf::Notificacao& notificacao) {
  const auto& proto = notificacao.entidade();
  auto* entidade = BuscaEntidade(proto.id());
  if (entidade == nullptr) {
    LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
    return;
  }
  entidade->Destino(proto.destino());
  if (notificacao.local()) {
    central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
    // Para desfazer: salva a posicao original e destino.
    ntf::Notificacao n_desfazer;
    n_desfazer.set_tipo(ntf::TN_MOVER_ENTIDADE);
    n_desfazer.mutable_entidade()->set_id(entidade->Id());
    n_desfazer.mutable_entidade()->mutable_pos()->CopyFrom(entidade->Proto().pos());
    n_desfazer.mutable_entidade()->mutable_destino()->CopyFrom(proto.pos());
    AdicionaNotificacaoListaEventos(n_desfazer);
  }
}

void Tabuleiro::RemoveEntidadeNotificando(unsigned int id_remocao) {
  auto* entidade = BuscaEntidade(id_remocao);
  if (entidade == nullptr) {
    return;
  }
  {
    // Para desfazer.
    ntf::Notificacao n_desfazer;
    n_desfazer.set_tipo(ntf::TN_REMOVER_ENTIDADE);
    n_desfazer.mutable_entidade()->CopyFrom(entidade->Proto());
    AdicionaNotificacaoListaEventos(n_desfazer);
  }
  RemoveEntidade(id_remocao);
  // Envia para os clientes.
  auto* n = ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE);
  n->mutable_entidade()->set_id(id_remocao);
  central_->AdicionaNotificacaoRemota(n);
  DeselecionaEntidade(id_remocao);
}

void Tabuleiro::RemoveEntidadeNotificando(const ntf::Notificacao& notificacao) {
  if (notificacao.local()) {
    if (notificacao.entidade().has_id()) {
      RemoveEntidadeNotificando(notificacao.entidade().id());
    } else {
      ntf::Notificacao grupo_notificacoes;
      grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
      for (unsigned int id_remocao : ids_entidades_selecionadas_) {
        auto* entidade = BuscaEntidade(id_remocao);
        if (entidade == nullptr) {
          continue;
        }
        auto* n = grupo_notificacoes.add_notificacao();
        n->set_tipo(ntf::TN_REMOVER_ENTIDADE);
        // Para desfazer.
        n->mutable_entidade()->CopyFrom(entidade->Proto());
      }
      TrataNotificacao(grupo_notificacoes);
      AdicionaNotificacaoListaEventos(grupo_notificacoes);
    }
  } else {
    // Comando vindo de fora.
    unsigned int id = notificacao.entidade().id();
    RemoveEntidade(id);
    DeselecionaEntidade(id);
  }
}

bool Tabuleiro::RemoveEntidade(unsigned int id) {
  MapaEntidades::iterator res_find = entidades_.find(id);
  if (res_find == entidades_.end()) {
    return false;
  }
  entidades_.erase(res_find);
  return true;
}

void Tabuleiro::AtualizaEntidadeNotificando(const ntf::Notificacao& notificacao) {
  const auto& proto = notificacao.entidade();
  auto* entidade = BuscaEntidade(proto.id());
  if (entidade == nullptr) {
    LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
    return;
  }
  // Para desfazer.
  auto proto_antes = entidade->Proto();
  entidade->AtualizaProto(proto);
  AtualizaSelecaoEntidade(entidade->Id());
  if (notificacao.local()) {
    // So repassa a notificacao pros clientes se a origem dela for local,
    // para evitar ficar enviando infinitamente.
    central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
    // Para desfazer.
    ntf::Notificacao n_desfazer(notificacao);
    n_desfazer.mutable_entidade_antes()->Swap(&proto_antes);
    AdicionaNotificacaoListaEventos(n_desfazer);
  }
}

unsigned int Tabuleiro::GeraIdEntidade(int id_cliente) {
  const unsigned int max_id_entidade = (1 << 28);
  unsigned int count = max_id_entidade;
  while (count-- > 0) {
    unsigned int id = (id_cliente << 28) | proximo_id_entidade_;
    proximo_id_entidade_ = ((proximo_id_entidade_ + 1) % max_id_entidade);
    auto it = entidades_.find(id);
    if (it == entidades_.end()) {
      return id;
    }
  }
  throw std::logic_error("Limite de entidades alcancado para cliente.");
}

int Tabuleiro::GeraIdTabuleiro() {
  const int max_id_cliente = 15;
  int count = max_id_cliente;
  while (count-- > 0) {
    int id_tab = proximo_id_cliente_;
    auto it = clientes_.find(id_tab);
    // O id zero esta sempre reservado para o mestre.
    proximo_id_cliente_ = ((proximo_id_cliente_) % max_id_cliente) + 1;
    if (it == clientes_.end()) {
      VLOG(1) << "GeraIdTabuleiro retornando id para cliente: " << id_tab;
      return id_tab;
    }
  }
  throw std::logic_error("Limite de clientes alcancado.");
}

namespace {
// Retorna true se ha uma nova textura.
bool AtualizaTexturas(bool novo_tem, const ent::InfoTextura& novo_proto,
                      bool velho_tem, ent::InfoTextura* velho_proto, ntf::CentralNotificacoes* central) {
  VLOG(2) << "Atualizando texturas, novo proto: " << novo_proto.ShortDebugString() << ", velho: " << velho_proto->ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (velho_tem && velho_proto->id() != novo_proto.id()) {
    VLOG(2) << "Liberando textura: " << velho_proto->id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->CopyFrom(*velho_proto);
    central->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_tem && novo_proto.id() != velho_proto->id()) {
    VLOG(2) << "Carregando textura: " << novo_proto.id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->mutable_info_textura()->CopyFrom(novo_proto);
    central->AdicionaNotificacao(nc);
  }

  if (novo_tem) {
    // Os bits crus so sao reenviados se houver mudanca. Nao eh bom perde-los por causa de novas serializacoes
    // como salvamentos e novos jogadores. Salva aqui pra restaurar ali embaixo.
    bool manter_bits_crus =
        novo_proto.id() == velho_proto->id() && velho_proto->has_bits_crus();
    std::string bits_crus = manter_bits_crus ? velho_proto->bits_crus() : std::string("");
    velho_proto->CopyFrom(novo_proto);
    if (manter_bits_crus) {
      velho_proto->set_bits_crus(bits_crus);
    }
    return true;
  }
  return false;
}
}  // namespace

void Tabuleiro::AtualizaTexturas(const ent::TabuleiroProto& novo_proto) {
  if (ent::AtualizaTexturas(novo_proto.has_info_textura(), novo_proto.info_textura(),
                            proto_.has_info_textura(), proto_.mutable_info_textura(),
                            central_)) {
    proto_.set_ladrilho(novo_proto.ladrilho());
    proto_.set_textura_mestre_apenas(novo_proto.textura_mestre_apenas());
  } else {
    proto_.clear_info_textura();
    proto_.clear_ladrilho();
    proto_.clear_textura_mestre_apenas();
  }
  if (!ent::AtualizaTexturas(novo_proto.has_info_textura_ceu(), novo_proto.info_textura_ceu(),
                             proto_.has_info_textura_ceu(), proto_.mutable_info_textura_ceu(),
                             central_)) {
    proto_.clear_info_textura_ceu();
  }
}

void Tabuleiro::DesenhaLuzes() {
  GLfloat cor_luz_ambiente[] = { proto_.luz_ambiente().r(),
                                 proto_.luz_ambiente().g(),
                                 proto_.luz_ambiente().b(),
                                 proto_.luz_ambiente().a()};
  if (VisaoMestre() && !opcoes_.iluminacao_mestre_igual_jogadores()) {
    // Adiciona luz pro mestre ver melhor.
    cor_luz_ambiente[0] = std::max(0.65f, cor_luz_ambiente[0]);
    cor_luz_ambiente[1] = std::max(0.65f, cor_luz_ambiente[1]);
    cor_luz_ambiente[2] = std::max(0.65f, cor_luz_ambiente[2]);
  }
  gl::ModeloLuz(GL_LIGHT_MODEL_AMBIENT, cor_luz_ambiente);

  // Iluminação distante direcional.
  {
    gl::MatrizEscopo salva_matriz;
    // O vetor inicial esta no leste (origem da luz). O quarte elemento indica uma luz no infinito.
    GLfloat pos_luz[] = { 1.0, 0.0f, 0.0f, 0.0f };
    // Roda no eixo Z (X->Y) em direcao a posicao entao inclina a luz no eixo -Y (de X->Z).
    gl::Roda(proto_.luz_direcional().posicao_graus(), 0.0f, 0.0f, 1.0f);
    gl::Roda(proto_.luz_direcional().inclinacao_graus(), 0.0f, -1.0f, 0.0f);
    gl::Luz(GL_LIGHT0, GL_POSITION, pos_luz);
  }
  // A cor da luz direcional.
  GLfloat cor_luz[] = { proto_.luz_direcional().cor().r(),
                        proto_.luz_direcional().cor().g(),
                        proto_.luz_direcional().cor().b(),
                        proto_.luz_direcional().cor().a() };
  gl::Luz(GL_LIGHT0, GL_DIFFUSE, cor_luz);
  gl::Habilita(GL_LIGHT0);

  if (parametros_desenho_.desenha_nevoa() && proto_.has_nevoa() &&
      (!VisaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    gl::Habilita(GL_FOG);
    gl::ModoNevoa(GL_LINEAR);
    gl::Nevoa(GL_FOG_START, proto_.nevoa().distancia_minima());
    gl::Nevoa(GL_FOG_END, proto_.nevoa().distancia_maxima());
    gl::Nevoa(GL_FOG_COLOR, cor_luz_ambiente);
  } else {
    gl::Desabilita(GL_FOG);
  }

  // Posiciona as luzes dinâmicas.
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    auto* e = it->second.get();
    e->DesenhaLuz(&parametros_desenho_);
  }
}

void Tabuleiro::DesenhaCaixaCeu() {
  // Desliga luzes pontuais.
  //gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  for (int i = 0; i < parametros_desenho_.luz_corrente(); ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  gl::MatrizEscopo salva_mv(GL_MODELVIEW);
  gl::Translada(olho_.pos().x(), olho_.pos().y(), olho_.pos().z());
  MudaCor(COR_BRANCA);
  gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  gl::FaceNula(GL_FRONT);
  GLuint id_textura = texturas_->Textura(proto_.info_textura_ceu().id());
  if (id_textura == GL_INVALID_VALUE) {
    return;
  }
  gl::Habilita(GL_TEXTURE_2D);
  gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
  glBindTexture(GL_TEXTURE_2D, id_textura);
  gl::DesenhaVbo(vbo_caixa_ceu_);
  gl::Desabilita(GL_TEXTURE_2D);
  // Religa luzes.
  gl::FaceNula(GL_BACK);
  for (int i = 0; i < parametros_desenho_.luz_corrente(); ++i) {
    gl::Habilita(GL_LIGHT0 + i);
  }
}

void Tabuleiro::DesenhaGrade() {
  MudaCor(COR_PRETA);
  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_buffer_grade_);
  gl::PonteiroVertices(2, GL_FLOAT, (void*)0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, nome_buffer_indice_grade_);
  gl::DesenhaElementos(GL_TRIANGLES, indices_grade_.size(), GL_UNSIGNED_SHORT, (void*)0);
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Tabuleiro::DesenhaListaPontosVida() {
  if (lista_pontos_vida_.empty()) {
    return;
  }
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  // Modo 2d: eixo com origem embaixo esquerda.
  int raster_x = 0, raster_y = 0;
  int largura_fonte, altura_fonte;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte);
  raster_y = altura_ - altura_fonte;
  raster_x = largura_ - 2;
  PosicionaRaster2d(raster_x, raster_y, largura_, altura_);

  MudaCor(COR_BRANCA);
  std::string titulo("Lista PV");
  gl::DesenhaStringAlinhadoDireita(titulo);
  raster_y -= (altura_fonte + 2);
  for (int pv : lista_pontos_vida_) {
    PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
    raster_y -= (altura_fonte + 2);
    MudaCor(pv >= 0 ? COR_VERDE : COR_VERMELHA);
    char str[4];
    snprintf(str, 4, "%d", abs(pv));
    gl::DesenhaStringAlinhadoDireita(str);
  }
}

void Tabuleiro::DesenhaIdAcaoEntidade() {
  std::string id_acao;
  bool achou = false;
  for (const auto& id_entidade : ids_entidades_selecionadas_) {
    const Entidade* entidade = BuscaEntidade(id_entidade);
    if (entidade == nullptr || entidade->Tipo() != TE_ENTIDADE) {
      continue;
    }
    if (!achou) {
      id_acao.assign(entidade->Acao());
      achou = true;
    } else if (id_acao != entidade->Acao()) {
      id_acao.assign("acoes diferem");
      break;
    }
  }
  if (!achou) {
    return;
  }
  if (id_acao.empty()) {
    id_acao.assign(ID_ACAO_ATAQUE_CORPO_A_CORPO);
  }
  id_acao = "Ação: " + id_acao;
  id_acao = StringSemUtf8(id_acao);

  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0, largura_, 0, altura_, 0, 1);

  {
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
    gl::CarregaIdentidade();
    int largura_fonte, altura_fonte;
    gl::TamanhoFonte(&largura_fonte, &altura_fonte);

    int raster_y = altura_ - altura_fonte;
    int raster_x = largura_ / 2;
    gl::PosicaoRaster(raster_x, raster_y);
    MudaCor(COR_BRANCA);
    gl::DesenhaString(id_acao);
  }
}

void Tabuleiro::DesenhaTempoRenderizacao() {
  glFlush();
  timer_.stop();
  auto cpu_times = timer_.elapsed();
  uint64_t tempo_ms = (cpu_times.user + cpu_times.system) / 1000000ULL;
  if (tempos_renderizacao_.size() == kMaximoTamTemposRenderizacao) {
    tempos_renderizacao_.pop_back();
  }
  tempos_renderizacao_.push_front(tempo_ms);
  if (!parametros_desenho_.desenha_fps()) {
    // Se nao eh pra desenhar, nao gasta tempo com o resto. A parte de cima eh importante
    // para contabilizar os casos de picking.
    return;
  }
  // Acha o maior.
  uint64_t maior_tempo_ms = 0;
  for (uint64_t tempo_ms : tempos_renderizacao_) {
    if (tempo_ms > maior_tempo_ms) {
      maior_tempo_ms = tempo_ms;
    }
  }
#if ANDROID || WIN32
  std::string tempo_str;
  while (maior_tempo_ms > 0) {
    char c = (maior_tempo_ms % 10) + '0';
    tempo_str.push_back(c);
    maior_tempo_ms /= 10;
  }
  std::reverse(tempo_str.begin(), tempo_str.end());
#else
  std::string tempo_str = net::to_string(maior_tempo_ms);
#endif
  while (tempo_str.size() < 4) {
    tempo_str.insert(0, "0");
  }
  // Modo 2d.
  gl::MatrizEscopo salva_matriz_proj(GL_PROJECTION);
  gl::CarregaIdentidade();
  // Eixo com origem embaixo esquerda.
  gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
  gl::MatrizEscopo salva_matriz_mv(GL_MODELVIEW);
  gl::CarregaIdentidade();
  int largura_fonte, altura_fonte;
  gl::TamanhoFonte(largura_, altura_, &largura_fonte, &altura_fonte);
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  gl::DesligaEscritaProfundidadeEscopo profundidade_escopo;
  gl::PosicaoRaster(2, altura_ - altura_fonte - 2);
  MudaCor(COR_PRETA);
  gl::Retangulo(0.0f, altura_ - 15.0f, tempo_str.size() * 8.0f + 2.0f, altura_);
  MudaCor(COR_BRANCA);
  gl::DesenhaStringAlinhadoEsquerda(tempo_str);
}

double Tabuleiro::Aspecto() const {
  return static_cast<double>(largura_) / static_cast<double>(altura_);
}

void Tabuleiro::AlterarModoMestre(bool modo) {
  LOG(INFO) << "Alternando para modo mestre: " << modo;
  modo_mestre_ = modo;
#if USAR_WATCHDOG
  if (modo) {
    DesativaWatchdog();
  } else {
    ReativaWatchdog();
  }
#endif
}

const std::vector<unsigned int> Tabuleiro::EntidadesAfetadasPorAcao(const AcaoProto& acao) {
  std::vector<unsigned int> ids_afetados;
  const Posicao& pos_tabuleiro = acao.pos_tabuleiro();
  const Posicao& pos_destino = acao.pos_entidade();
  const Entidade* entidade_origem = BuscaEntidade(acao.id_entidade_origem());
  if (acao.tipo() == ACAO_DISPERSAO) {
    switch (acao.geometria()) {
      case ACAO_GEO_ESFERA: {
        const Posicao pos_para_computar = pos_destino.has_x() ? pos_destino : pos_tabuleiro;
        for (const auto& id_entidade_destino : entidades_) {
          const Entidade* entidade_destino = id_entidade_destino.second.get();
          Posicao pos_entidade(entidade_destino->Pos());
          pos_entidade.set_z(pos_entidade.z() + entidade_destino->TranslacaoZ());
          float d2 = DistanciaQuadrado(pos_para_computar, pos_entidade);
          if (d2 <= powf(acao.raio_area() * TAMANHO_LADO_QUADRADO, 2)) {
            VLOG(1) << "Adicionando id: " << id_entidade_destino.first;
            ids_afetados.push_back(id_entidade_destino.first);
          }
        }
      }
      break;
      case ACAO_GEO_CONE: {
        if (entidade_origem == nullptr) {
          LOG(WARNING) << "Entidade de origem nao encontrada";
          return ids_afetados;
        }
        // Vetor de direcao.
        const Posicao& pos_o = entidade_origem->Pos();
        Posicao vetor_direcao;
        vetor_direcao.set_x(pos_tabuleiro.x() - pos_o.x());
        vetor_direcao.set_y(pos_tabuleiro.y() - pos_o.y());
        float rotacao = VetorParaRotacaoGraus(vetor_direcao);
        // Ja temos a direcao, agora eh so rodar o triangulo.
        float distancia = acao.distancia() * TAMANHO_LADO_QUADRADO;
        float distancia_2 = distancia / 2.0f;
        std::vector<Posicao> vertices(3);
        vertices[1].set_x(distancia);
        vertices[1].set_y(-distancia_2);
        vertices[2].set_x(distancia);
        vertices[2].set_y(distancia_2);
        for (unsigned int i = 0; i < vertices.size(); ++i) {
          RodaVetor2d(rotacao, &vertices[i]);
          vertices[i].set_x(vertices[i].x() + pos_o.x());
          vertices[i].set_y(vertices[i].y() + pos_o.y());
        }
        for (const auto& id_entidade_destino : entidades_) {
          const Entidade* entidade_destino = id_entidade_destino.second.get();
          if (entidade_destino == entidade_origem) {
            continue;
          }
          if (PontoDentroDePoligono(entidade_destino->Pos(), vertices)) {
            VLOG(1) << "Adicionando id: " << id_entidade_destino.first;
            ids_afetados.push_back(id_entidade_destino.first);
          }
        }
      }
      break;
      default:
        LOG(WARNING) << "Geometria da acao nao implementada: " << acao.tipo();
    }
  } else if (acao.tipo() == ACAO_RAIO) {
    if (entidade_origem == nullptr) {
      LOG(WARNING) << "Entidade de origem nao encontrada";
      return ids_afetados;
    }
    // Vetor de direcao.
    const Posicao& pos_o = entidade_origem->Pos();
    Posicao vetor_direcao;
    vetor_direcao.set_x(pos_tabuleiro.x() - pos_o.x());
    vetor_direcao.set_y(pos_tabuleiro.y() - pos_o.y());
    float rotacao = VetorParaRotacaoGraus(vetor_direcao);
    // Ja temos a direcao, agora eh so rodar o retangulo.
    std::vector<Posicao> vertices(4);
    vertices[0].set_y(-TAMANHO_LADO_QUADRADO_2);
    vertices[1].set_y(TAMANHO_LADO_QUADRADO_2);
    vertices[2].set_y(TAMANHO_LADO_QUADRADO_2);
    vertices[2].set_x(acao.distancia() * TAMANHO_LADO_QUADRADO);
    vertices[3].set_y(-TAMANHO_LADO_QUADRADO_2);
    vertices[3].set_x(acao.distancia() * TAMANHO_LADO_QUADRADO);
    for (unsigned int i = 0; i < vertices.size(); ++i) {
      RodaVetor2d(rotacao, &vertices[i]);
      vertices[i].set_x(vertices[i].x() + pos_o.x());
      vertices[i].set_y(vertices[i].y() + pos_o.y());
    }
    for (const auto& id_entidade_destino : entidades_) {
      const Entidade* entidade_destino = id_entidade_destino.second.get();
      if (entidade_destino == entidade_origem) {
        continue;
      }
      if (PontoDentroDePoligono(entidade_destino->Pos(), vertices)) {
        VLOG(1) << "Adicionando id: " << id_entidade_destino.first;
        ids_afetados.push_back(id_entidade_destino.first);
      }
    }
  } else {
    LOG(WARNING) << "Tipo de acao nao reconhecido: " << acao.tipo();
  }

  return ids_afetados;
}

Entidade* Tabuleiro::EntidadeSelecionada() {
  if (ids_entidades_selecionadas_.size() != 1) {
    return nullptr;
  }
  unsigned int id = *ids_entidades_selecionadas_.begin();
  auto it = entidades_.find(id);
  if (it == entidades_.end()) {
    return nullptr;
  }
  return it->second.get();
}

void Tabuleiro::AlternaModoDebug() {
  gl::AlternaModoDebug();
  modo_debug_ = !modo_debug_;
}

void Tabuleiro::AdicionaEventoEntidadesSelecionadasNotificando(int rodadas) {
  if (rodadas < 0) {
    LOG(ERROR) << "Adicionando rodadas < 0";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (auto& id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    // Para desfazer.
    EntidadeProto proto_antes;
    proto_antes.set_id(id);
    proto_antes.mutable_evento()->CopyFrom(entidade_selecionada->Proto().evento());
    // Proto depois.
    EntidadeProto proto_depois;
    proto_depois.set_id(id);
    proto_depois.mutable_evento()->CopyFrom(entidade_selecionada->Proto().evento());
    proto_depois.add_evento()->set_rodadas(rodadas);

    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    n->mutable_entidade_antes()->Swap(&proto_antes);
    n->mutable_entidade()->Swap(&proto_depois);
  }
  if (grupo_notificacoes.notificacao_size() == 0) {
    return;
  }
  TrataNotificacao(grupo_notificacoes);
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::PassaUmaRodadaNotificando() {
  if (!ModoMestre()) {
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (auto& id_entidade : entidades_) {
    EntidadeProto proto_antes;
    EntidadeProto proto_depois;
    bool havera_mudanca = false;
    const auto* entidade = id_entidade.second.get();
    for (const auto& e : entidade->Proto().evento()) {
      if (e.rodadas() > 0) {
        havera_mudanca = true;
      }
    }
    if (!havera_mudanca) {
      continue;
    }
    // Desfazer.
    proto_antes.set_id(id_entidade.first);
    proto_antes.mutable_evento()->CopyFrom(entidade->Proto().evento());
    // Novo proto.
    proto_depois.set_id(id_entidade.first);
    for (const auto& e : entidade->Proto().evento()) {
      int rodadas = e.rodadas();
      if (rodadas > 0) {
        --rodadas;
      }
      auto* evento_depois = proto_depois.add_evento();
      evento_depois->set_rodadas(rodadas);
      evento_depois->set_descricao(e.descricao());
    }
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    n->mutable_entidade_antes()->Swap(&proto_antes);;
    n->mutable_entidade()->Swap(&proto_depois);;
  }
  auto* nr = grupo_notificacoes.add_notificacao();
  nr->set_tipo(ntf::TN_ATUALIZAR_RODADAS);
  nr->mutable_tabuleiro_antes()->set_contador_rodadas(proto_.contador_rodadas());
  nr->mutable_tabuleiro()->set_contador_rodadas(proto_.contador_rodadas() + 1);

  TrataNotificacao(grupo_notificacoes);
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::ZeraRodadasNotificando() {
  if (!ModoMestre()) {
    return;
  }
  ntf::Notificacao nr;
  nr.set_tipo(ntf::TN_ATUALIZAR_RODADAS);
  nr.mutable_tabuleiro_antes()->set_contador_rodadas(proto_.contador_rodadas());
  nr.mutable_tabuleiro()->set_contador_rodadas(0);
  TrataNotificacao(nr);
  AdicionaNotificacaoListaEventos(nr);
}

void Tabuleiro::ApagaEventosZeradosDeEntidadeNotificando(unsigned int id) {
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    LOG(ERROR) << "Entidade invalida para picking: " << id;
    return;
  }
  EntidadeProto proto_antes;
  EntidadeProto proto_depois;
  // Desfazer.
  proto_antes.set_id(id);
  proto_antes.mutable_evento()->CopyFrom(entidade->Proto().evento());
  // Novo proto.
  proto_depois.set_id(id);
  for (const auto& evento : entidade->Proto().evento()) {
    int rodadas = evento.rodadas();
    if (rodadas > 0) {
      proto_depois.add_evento()->CopyFrom(evento);
    }
  }
  // Hack: se nao tiver nenhum evento mais, cria um dummy para a atualizacao parcial saber que deve mexer nos eventos.
  if (proto_depois.evento_size() == 0) {
    proto_depois.add_evento();
  }

  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  n.mutable_entidade_antes()->Swap(&proto_antes);;
  n.mutable_entidade()->Swap(&proto_depois);;
  TrataNotificacao(n);
  AdicionaNotificacaoListaEventos(n);
}

void Tabuleiro::AlternaModoAcao() {
  modo_acao_ = !modo_acao_;
}

void Tabuleiro::SalvaCameraInicial() {
  proto_.mutable_camera_inicial()->CopyFrom(olho_);
  // Destino é para movimento.
  proto_.mutable_camera_inicial()->clear_destino();
}

void Tabuleiro::ReiniciaCamera() {
  if (proto_.has_camera_inicial()) {
    olho_.CopyFrom(proto_.camera_inicial());
  } else {
    auto* pos = olho_.mutable_alvo();
    pos->set_x(0.0f);
    pos->set_y(0.0f);
    pos->set_z(0.0f);
    // Olho sempre comeca olhando do sul (-pi/2).
    olho_.set_rotacao_rad(-M_PI / 2.0f);
    olho_.set_altura(OLHO_ALTURA_INICIAL);
    olho_.set_raio(OLHO_RAIO_INICIAL);
    olho_.clear_destino();
  }
  AtualizaOlho(true  /*forcar*/);
}

void Tabuleiro::AlternaCameraIsometrica() {
  camera_isometrica_ = !camera_isometrica_;
}

void Tabuleiro::AlternaCameraPresa() {
  if (camera_presa_) {
    camera_presa_ = false;
    id_camera_presa_ = Entidade::IdInvalido;
    LOG(INFO) << "Camera solta.";
  } else if (ids_entidades_selecionadas_.size() == 1) {
    camera_presa_ = true;
    id_camera_presa_ = *ids_entidades_selecionadas_.begin();
    LOG(INFO) << "Camera presa.";
  } else {
    LOG(INFO) << "Sem entidade selecionada.";
  }
}

void Tabuleiro::DesativaWatchdog() {
#if USAR_WATCHDOG
  if (!modo_mestre_) {
    return;
  }
  watchdog_.Para();
#endif
}

void Tabuleiro::ReativaWatchdog() {
#if USAR_WATCHDOG
  if (!modo_mestre_) {
    return;
  }
  watchdog_.Reinicia();
#endif
}

}  // namespace ent
