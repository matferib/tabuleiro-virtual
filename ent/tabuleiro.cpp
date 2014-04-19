#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
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

/** Os clipping planes. */
const double DISTANCIA_PLANO_CORTE_PROXIMO = 0.5;
const double DISTANCIA_PLANO_CORTE_DISTANTE = 500.0f;

#if USAR_OPENGL_ES
const unsigned int NUM_DIVISOES_DETALHAMENTO_PICKING = 10;
const float ALTURA_DETALHAMENTO_PICKING = 0.1f;
#endif

// Retorna 0 se nao andou quadrado, 1 se andou no eixo x, 2 se andou no eixo y, 3 se andou em ambos.
int AndouQuadrado(const Posicao& p1, const Posicao& p2) {
  float dx = fabs(p1.x() - p2.x());
  float dy = fabs(p1.y() - p2.y());
  if (dx >= TAMANHO_LADO_QUADRADO && dy >= TAMANHO_LADO_QUADRADO) return 3;
  if (dx >= TAMANHO_LADO_QUADRADO) return 1;
  if (dy >= TAMANHO_LADO_QUADRADO) return 2;
  return 0;
}

/** Desenha apenas a string. */
void DesenhaString(const std::string& s) {
  gl::PosicaoRaster(1, 1);
  for (const char c : s) {
    gl::DesenhaCaractere(c);
  }
}

/** Renderiza o tempo de desenho no canto superior esquerdo da tela. */
void DesenhaStringTempo(const std::string& tempo) {
  MudaCor(COR_PRETA);
  gl::Retangulo(0.0f, 0.0f, tempo.size() * 8.0f + 2.0f, 15.0f);
  MudaCor(COR_BRANCA);
  DesenhaString(tempo);
}

/** Le um arquivo proto serializado de forma binaria. */
bool LeArquivoProto(const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::ifstream arquivo(nome_arquivo,  std::ios_base::in | std::ios_base::binary);
  return mensagem->ParseFromIstream(&arquivo);
}

/** Le um arquivo proto serializado de forma texto (arquivos de configuracao). */
bool LeArquivoAsciiProto(const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::ifstream arquivo(nome_arquivo);
  google::protobuf::io::IstreamInputStream zis(&arquivo);
  return google::protobuf::TextFormat::Parse(&zis, mensagem);
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

// Para picking em OpenGL ES.
#if USAR_OPENGL_ES
void DesenhaQuadradoDetalhado(int linha, int coluna) {
  float lado_quadrado = TAMANHO_LADO_QUADRADO / NUM_DIVISOES_DETALHAMENTO_PICKING;
  for (unsigned int y = 0; y < NUM_DIVISOES_DETALHAMENTO_PICKING; ++y) {
    for (unsigned int x = 0; x < NUM_DIVISOES_DETALHAMENTO_PICKING; ++x) {
      unsigned int id = (y * NUM_DIVISOES_DETALHAMENTO_PICKING) + x;
      gl::CarregaNome(id);
      float inicio_x = x * lado_quadrado;
      float fim_x = inicio_x + lado_quadrado;
      float inicio_y = y * lado_quadrado;
      float fim_y = inicio_y + lado_quadrado;
      gl::Retangulo(inicio_x, inicio_y, fim_x, fim_y);
    }
  }
}

void DesenhaEntidadeDetalhada(const ParametrosDesenho& pd, Entidade* entidade) {
  // Desenha varios cilindros da altura da entidade.
  gl::MatrizEscopo salva_matriz;
  float multiplicador_tamanho = entidade->MultiplicadorTamanho();
  float lado = TAMANHO_LADO_QUADRADO * multiplicador_tamanho * 0.8f;
  float altura = multiplicador_tamanho * TAMANHO_LADO_QUADRADO * 2.0f;
  unsigned int id = 0;
  gl::Translada(entidade->X(), entidade->Y(), entidade->Z());
  // De frente para camera.
  double dx = entidade->X() - pd.pos_olho().x();
  double dy = entidade->Y() - pd.pos_olho().y();
  double r = sqrt(pow(dx, 2) + pow(dy, 2));
  float angulo = (acosf(dx / r) * RAD_PARA_GRAUS);
  if (dy < 0) {
    // A funcao asin tem dois resultados mas sempre retorna o positivo [0, PI].
    // Se o vetor estiver nos quadrantes de baixo, inverte o angulo.
    angulo = -angulo;
  }
  gl::Roda(angulo - 90.0f, 0, 0, 1.0);
  gl::Escala(1.0f, ALTURA_DETALHAMENTO_PICKING, 1.0f);

  for (float base = 0; base < altura; base += ALTURA_DETALHAMENTO_PICKING) {
    gl::CarregaNome(id++);
    gl::MatrizEscopo salva_matriz;
    gl::Translada(0.0f, 0.0f, base + (ALTURA_DETALHAMENTO_PICKING / 2.0f));
    gl::Escala(1.0f, 1.0f, ALTURA_DETALHAMENTO_PICKING);
    // Desenha retangulo
    gl::CuboSolido(lado);
  }
}
#endif  // USAR_OPENGL_ES

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
  std::string arq_modelos(std::string(DIR_DADOS) + "/" + ARQUIVO_MODELOS);
  if (!LeArquivoAsciiProto(arq_modelos, &modelos)) {
    LOG(ERROR) << "Falha ao importar modelos do arquivo '" << arq_modelos << "'";
  } else {
    for (const auto& m : modelos.modelo()) {
      mapa_modelos_.insert(std::make_pair(
            m.id(), std::unique_ptr<EntidadeProto>(new EntidadeProto(m.entidade()))));
    }
  }
  // Acoes.
  acao_selecionada_ = nullptr;
  Acoes acoes;
  std::string arq_acoes(std::string(DIR_DADOS) + "/" + ARQUIVO_ACOES);
  if (!LeArquivoAsciiProto(arq_acoes, &acoes)) {
    LOG(ERROR) << "Falha ao importar acoes do arquivo '" << arq_acoes << "'";
  } else {
    for (const auto& a : acoes.acao()) {
      auto* nova_acao = new AcaoProto(a);
      if (nova_acao->tipo() == ACAO_SINALIZACAO) {
        acao_selecionada_ = nova_acao;
      }
      mapa_acoes_.insert(std::make_pair(a.id(), std::unique_ptr<AcaoProto>(nova_acao)));
    }
  }
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
    notificacao.set_endereco("tabuleiro_watchdog.binproto");
    this->TrataNotificacao(notificacao);
  });
#endif
}

Tabuleiro::~Tabuleiro() {
  LiberaTextura();
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
  proto_.mutable_luz_ambiente()->set_r(0.2f);
  proto_.mutable_luz_ambiente()->set_g(0.2f);
  proto_.mutable_luz_ambiente()->set_b(0.2f);
  // Iluminacao direcional inicial.
  proto_.mutable_luz_direcional()->mutable_cor()->set_r(0.2f);
  proto_.mutable_luz_direcional()->mutable_cor()->set_g(0.2f);
  proto_.mutable_luz_direcional()->mutable_cor()->set_b(0.2f);
  // Vinda de 45 graus leste.
  proto_.mutable_luz_direcional()->set_posicao_graus(0.0f);
  proto_.mutable_luz_direcional()->set_inclinacao_graus(45.0f);
  // Olho.
  auto* pos = olho_.mutable_alvo();
  pos->set_x(0.0f);
  pos->set_y(0.0f);
  pos->set_z(0.0f);
  // Olho sempre comeca olhando do sul (-pi/2).
  olho_.set_rotacao_rad(-M_PI / 2.0f);
  olho_.set_altura(OLHO_ALTURA_INICIAL);
  olho_.set_raio(OLHO_RAIO_INICIAL);
  olho_.clear_destino();
  AtualizaOlho(true  /*forcar*/);

  // Valores iniciais.
  ultimo_x_ = ultimo_y_ = 0;
  ultimo_x_3d_ = ultimo_y_3d_ = ultimo_z_3d_ = 0;
  primeiro_x_3d_ = primeiro_y_3d_ = primeiro_z_3d_ = 0;
  // Mapa de entidades e acoes vazios.
  entidades_.clear();
  acoes_.clear();
  // Outras variaveis.
  id_entidade_detalhada_ = 0xFFFFFFFF;
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
}

void Tabuleiro::Desenha() {
  // Varios lugares chamam desenha cena com parametros especifico. Essa funcao
  // desenha a cena padrao, entao ela restaura os parametros para seus valores
  // default. Alem disso a matriz de projecao eh diferente para picking.
  parametros_desenho_.Clear();
  parametros_desenho_.set_modo_mestre(modo_mestre_);
  gl::ModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Perspectiva(CAMPO_VERTICAL_GRAUS, Aspecto(), DISTANCIA_PLANO_CORTE_PROXIMO, DISTANCIA_PLANO_CORTE_DISTANTE);
  // Aplica opcoes do jogador.
  parametros_desenho_.set_desenha_fps(opcoes_.mostra_fps());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
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
      int id_entidade = GeraIdEntidade(id_cliente_);
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

void Tabuleiro::AtualizaBitsEntidadeNotificando(int bits) {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* n = grupo_notificacoes.add_notificacao();
    auto* entidade_selecionada = BuscaEntidade(id);
    EntidadeProto proto = entidade_selecionada->Proto();
    // Para desfazer.
    n->mutable_entidade_antes()->CopyFrom(proto);
    if ((bits & BIT_VISIBILIDADE) > 0 && modo_mestre_) {
      // Apenas modo mestre.
      proto.set_visivel(!proto.visivel());
    }
    if ((bits & BIT_ILUMINACAO) > 0) {
      if (proto.has_luz()) {
        proto.clear_luz();
      } else {
        auto* luz = proto.mutable_luz()->mutable_cor();
        luz->set_r(1.0f);
        luz->set_g(1.0f);
        luz->set_b(1.0f);
      }
    }
    if ((bits & BIT_VOO) > 0) {
      proto.set_voadora(!proto.voadora());
    }
    if (bits & BIT_CAIDA) {
      proto.set_caida(!proto.caida());
    }
    if (bits & BIT_MORTA) {
      proto.set_morta(!proto.morta());
    }
    if (bits & BIT_SELECIONAVEL) {
      proto.set_selecionavel_para_jogador(!proto.selecionavel_para_jogador());
    }
    proto.set_id(id);
    n->set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
    n->mutable_entidade()->Swap(&proto);
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
  ntf::Notificacao g_desfazer;
  g_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    // Atualizacao.
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE);
    n->mutable_entidade()->set_id(entidade_selecionada->Id());
    n->mutable_entidade()->set_pontos_vida(entidade_selecionada->PontosVida() + delta_pontos_vida);
    // Acao.
    auto* na = grupo_notificacoes.add_notificacao();
    na->set_tipo(ntf::TN_ADICIONAR_ACAO);
    auto* a = na->mutable_acao();
    a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    a->set_id_entidade_destino(entidade_selecionada->Id());
    a->set_delta_pontos_vida(delta_pontos_vida);
    // Para desfazer.
    {
      auto* n_desfazer = g_desfazer.add_notificacao();
      n_desfazer->CopyFrom(*n);
      n_desfazer->mutable_entidade_antes()->CopyFrom(entidade_selecionada->Proto());
    }
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(g_desfazer);
}

void Tabuleiro::AtualizaPontosVidaEntidadeNotificando(const ntf::Notificacao& notificacao) {
  if (!notificacao.entidade().has_pontos_vida() || !notificacao.entidade().has_id()) {
    LOG(ERROR) << "Notificacao de atualizacao de pontos de vida sem pontos de vida ou id: " << notificacao.ShortDebugString();
    return;
  }
  auto* entidade = BuscaEntidade(notificacao.entidade().id());
  if (entidade == nullptr) {
    VLOG(1) << "Entidade invalida para notificacao de atualizacao de pontos de vida";
    return;
  }
  entidade->AtualizaPontosVida(notificacao.entidade().pontos_vida());
  if (notificacao.local()) {
    auto* n_remota = new ntf::Notificacao(notificacao);
    central_->AdicionaNotificacaoRemota(n_remota);
  }
}

void Tabuleiro::AtualizaPontosVidaEntidadePorAcao(unsigned int id_entidade, int delta_pontos_vida) {
  auto* entidade = BuscaEntidade(id_entidade);
  if (entidade == nullptr) {
    LOG(WARNING) << "Entidade nao encontrada: " << id_entidade;
    return;
  }
  // Atualizacao de pontos de vida.
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE);
  n.mutable_entidade()->set_id(id_entidade);
  n.mutable_entidade()->set_pontos_vida(entidade->PontosVida() + delta_pontos_vida);
  TrataNotificacao(n);

  // Acao de pontos de vida sem efeito.
  ntf::Notificacao na;
  na.set_tipo(ntf::TN_ADICIONAR_ACAO);
  auto* a = na.mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_id_entidade_destino(entidade->Id());
  a->set_delta_pontos_vida(delta_pontos_vida);
  a->set_afeta_pontos_vida(false);
  TrataNotificacao(na);
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

void Tabuleiro::LimpaUltimoListaPontosVida() {
  lista_pontos_vida_.pop_back();
}

bool Tabuleiro::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_GRUPO_NOTIFICACOES:
      // Nunca deve vir da central.
      processando_grupo_ = true;
      ids_adicionados_.clear();
      for (const auto& n : notificacao.notificacao()) {
        TrataNotificacao(n);
      }
      processando_grupo_ = false;
      return true;
    case ntf::TN_RESPOSTA_CONEXAO:
      if (!notificacao.has_erro()) {
        ModoJogador();
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
        std::string nt_tabuleiro_str = nt_tabuleiro->SerializeAsString();
        // Salvar no endereco.
        std::ofstream arquivo(notificacao.endereco());
        arquivo.write(nt_tabuleiro_str.c_str(), nt_tabuleiro_str.size());
        if (!arquivo) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro lendo arquivo: ") + notificacao.endereco());
          central_->AdicionaNotificacao(ne);
        }
        arquivo.close();
      } else {
        // Enviar remotamente.
        nt_tabuleiro->set_clientes_pendentes(notificacao.clientes_pendentes());
        central_->AdicionaNotificacaoRemota(nt_tabuleiro.release());
      }
      return true;
    }
    case ntf::TN_DESERIALIZAR_TABULEIRO: {
      if (notificacao.has_endereco()) {
        // Deserializar de arquivo.
        std::ifstream arquivo(notificacao.endereco());
        if (!arquivo) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro lendo arquivo: ") + notificacao.endereco());
          central_->AdicionaNotificacao(ne);
          return true;
        }
        ntf::Notificacao nt_tabuleiro;
        if (!LeArquivoProto(notificacao.endereco(), &nt_tabuleiro)) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro lendo arquivo: ") + notificacao.endereco());
          central_->AdicionaNotificacao(ne);
          return true;
        }
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
    case ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE:
      AtualizaPontosVidaEntidadeNotificando(notificacao);
      return true;
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
    default:
      return false;
  }
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

void Tabuleiro::TrataMovimentoMouse() {
  id_entidade_detalhada_ = 0xFFFFFFFF;
}

void Tabuleiro::TrataMovimentoMouse(int x, int y) {
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
      parametros_desenho_.set_offset_terreno(ultimo_z_3d_);
      parametros_desenho_.set_desenha_entidades(false);
      float nx, ny, nz;
      if (!MousePara3d(x, y, &nx, &ny, &nz)) {
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
      // Faz picking do tabuleiro sem entidades.
      parametros_desenho_.set_desenha_entidades(false);
      float nx, ny, nz;
      if (!MousePara3d(x, y, &nx, &ny, &nz)) {
        return;
      }

      float delta_x = nx - ultimo_x_3d_;
      float delta_y = ny - ultimo_y_3d_;
      auto* p = olho_.mutable_alvo();
      p->set_x(p->x() - delta_x);
      p->set_y(p->y() - delta_y);
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
      if (!MousePara3d(x, y, &x3d, &y3d, &z3d)) {
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
      if (!MousePara3d(x, y, &x3d, &y3d, &z3d)) {
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
  AcaoProto acao_proto;
  if (acao_padrao) {
    // usa acao de sinalizacao.
    VLOG(1) << "Botao acao sinalizacao";
    acao_proto.set_tipo(ACAO_SINALIZACAO);
  } else {
    // Usa modelo selecionado.
    VLOG(1) << "Botao acao de modelo selecionado";
    acao_proto.CopyFrom(*acao_selecionada_);
  }
  unsigned int id, pos_pilha;
  float profundidade;
  // Primeiro, entidades.
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha > 1) {
    VLOG(1) << "Acao em entidade: " << id;
    // Entidade.
    acao_proto.set_id_entidade_destino(id);
  }
  // Depois tabuleiro.
  parametros_desenho_.set_desenha_entidades(false);
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha == 1) {
    VLOG(1) << "Acao no tabuleiro: " << id;
    // Tabuleiro, posicao do quadrado clicado.
    float x, y, z;
    CoordenadaQuadrado(id, &x, &y, &z);
    auto* pos_quadrado = acao_proto.mutable_pos_quadrado();
    pos_quadrado->set_x(x);
    pos_quadrado->set_y(y);
    pos_quadrado->set_z(z);
    // Posicao exata do clique.
    float x3d, y3d, z3d;
    if (MousePara3d(x, y, profundidade, &x3d, &y3d, &z3d)) {
      auto* pos_tabuleiro = acao_proto.mutable_pos_tabuleiro();
      pos_tabuleiro->set_x(x3d);
      pos_tabuleiro->set_y(y3d);
      pos_tabuleiro->set_z(z3d);
    }
  }

  if (estado_ == ETAB_OCIOSO || estado_ == ETAB_QUAD_SELECIONADO) {
    // Acoes sem origem.
    if (!lista_pontos_vida_.empty() && acao_proto.has_id_entidade_destino()) {
      int delta_pontos_vida = lista_pontos_vida_.front();
      lista_pontos_vida_.pop_front();
      acao_proto.set_delta_pontos_vida(delta_pontos_vida);
      acao_proto.set_afeta_pontos_vida(true);
    }
    VLOG(2) << "Acao: " << acao_proto.ShortDebugString();
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_ADICIONAR_ACAO);
    n.mutable_acao()->Swap(&acao_proto);
    TrataNotificacao(n);
  } else if (estado_ == ETAB_ENTS_SELECIONADAS) {
    if (acao_proto.efeito_area()) {
      if (ids_entidades_selecionadas_.size() == 1) {
        // So poe origem se houver uma entidade selecionada.
        acao_proto.set_id_entidade_origem(*ids_entidades_selecionadas_.begin());
      } else {
        VLOG(1) << "Nao colocando origem em acao de area pois ha mais de uma entidade selecionada.";
      }
      // Para acoes de area, faz apenas uma acao.
      VLOG(2) << "Acao: " << acao_proto.ShortDebugString();
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_ADICIONAR_ACAO);
      n.mutable_acao()->CopyFrom(acao_proto);
      TrataNotificacao(n);
    } else {
      // Uma acao feita por cada entidade selecionada.
      float atraso_segundos = 0;
      ntf::Notificacao grupo_notificacoes;
      grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
      // Para desfazer.
      ntf::Notificacao grupo_desfazer;
      grupo_desfazer.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
      Entidade* entidade_destino = acao_proto.has_id_entidade_destino() ? BuscaEntidade(acao_proto.id_entidade_destino()) : nullptr;
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        acao_proto.set_id_entidade_origem(entidade_selecionada->Id());
        acao_proto.set_atraso_s(atraso_segundos);
        if (!lista_pontos_vida_.empty() && entidade_destino != nullptr) {
          int delta_pontos_vida = lista_pontos_vida_.front();
          lista_pontos_vida_.pop_front();
          acao_proto.set_delta_pontos_vida(delta_pontos_vida);
          acao_proto.set_afeta_pontos_vida(true);
          // Para desfazer
          {
            auto* nd = grupo_desfazer.add_notificacao();
            // Efeito antes acao.
            nd->mutable_entidade_antes()->CopyFrom(entidade_destino->Proto());
            // Hit points depois.
            auto* e_depois = nd->mutable_entidade();
            e_depois->set_id(entidade_destino->Id());
            e_depois->set_pontos_vida(entidade_destino->PontosVida() + delta_pontos_vida);
            nd->set_tipo(ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE);
          }
        }
        VLOG(2) << "Acao: " << acao_proto.ShortDebugString();
        auto* n = grupo_notificacoes.add_notificacao();
        n->set_tipo(ntf::TN_ADICIONAR_ACAO);
        n->mutable_acao()->CopyFrom(acao_proto);
        atraso_segundos += 0.5;
      }
      TrataNotificacao(grupo_notificacoes);
      if (entidade_destino != nullptr) {
        AdicionaNotificacaoListaEventos(grupo_desfazer);
      }
    }
  }
}

void Tabuleiro::TrataBotaoLiberado() {
  FinalizaEstadoCorrente();
}

void Tabuleiro::TrataMouseParadoEm(int x, int y) {
  unsigned int id;
  unsigned int pos_pilha;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha);
  if (pos_pilha <= 1) {
    // Mouse no tabuleiro.
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

void Tabuleiro::InicializaGL() {
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::Desabilita(GL_BLEND);

  // Nao desenha as costas dos poligonos.
  gl::Habilita(GL_CULL_FACE);
  gl::FaceNula(GL_BACK);
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
  acao_selecionada_ = it->second.get();
}

// privadas
void Tabuleiro::DesenhaCena() {
  // Caso o parametros_desenho_.desenha_fps() seja false, ele computara mas nao desenhara o objeto.
  // Isso eh importante para computacao de frames lentos, mesmo que nao seja mostrado naquele quadro.
  TimerEscopo timer_escopo(this, opcoes_.mostra_fps());
  gl::Habilita(GL_DEPTH_TEST);
  gl::CorLimpeza(proto_.luz_ambiente().r(),
                 proto_.luz_ambiente().g(),
                 proto_.luz_ambiente().b(),
                 proto_.luz_ambiente().a());
  if (parametros_desenho_.limpa_fundo()) {
    gl::Limpa(GL_COLOR_BUFFER_BIT);
  }
  gl::Limpa(GL_DEPTH_BUFFER_BIT);
  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  gl::ModoMatriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  const Posicao& alvo = olho_.alvo();
  gl::OlharPara(
    // from.
    olho_.pos().x(), olho_.pos().y(), olho_.pos().z(),
    // to.
    alvo.x(), alvo.y(), alvo.z(),
    // up
    0, 0, 1.0);
  parametros_desenho_.mutable_pos_olho()->CopyFrom(olho_.pos());
  // Verifica o angulo em relacao ao tabuleiro para decidir se as texturas ficarao viradas para cima.
  if (olho_.altura() > (2 * olho_.raio())) {
    parametros_desenho_.set_desenha_texturas_para_cima(true);
  } else {
    parametros_desenho_.set_desenha_texturas_para_cima(false);
  }

  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
    GLfloat cor_luz_ambiente[] = { proto_.luz_ambiente().r(),
                                   proto_.luz_ambiente().g(),
                                   proto_.luz_ambiente().b(),
                                   proto_.luz_ambiente().a()};
    if (modo_mestre_ && !opcoes_.iluminacao_mestre_igual_jogadores()) {
      // Adiciona luz pro mestre ver melhor.
      cor_luz_ambiente[0] = std::max(0.4f, cor_luz_ambiente[0]);
      cor_luz_ambiente[1] = std::max(0.4f, cor_luz_ambiente[1]);
      cor_luz_ambiente[2] = std::max(0.4f, cor_luz_ambiente[2]);
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

    // Posiciona as luzes dinâmicas.
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      auto* e = it->second.get();
      e->DesenhaLuz(&parametros_desenho_);
    }
  } else {
    gl::Desabilita(GL_LIGHTING);
  }

  //ceu_.desenha(parametros_desenho_);

  // desenha tabuleiro do sul para o norte.
  DesenhaTabuleiro();

  if (parametros_desenho_.desenha_grade()) {
    gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
    DesenhaGrade();
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

  if (modo_mestre_ && parametros_desenho_.desenha_pontos_rolagem()) {
    // Pontos de rolagem na terceira posicao da pilha.
    gl::NomesEscopo nomes_ent(0);
    gl::NomesEscopo nomes_pontos(0);
    DesenhaPontosRolagem();
  }

  if (!parametros_desenho_.desenha_entidades()) {
    return;
  }

  // Desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
  // na hora do picking.
  {
    gl::NomesEscopo nomes(0);
#if USAR_OPENGL_ES
    if (parametros_desenho_.params_opengles().has_id()) {
      Entidade* entidade = BuscaEntidade(parametros_desenho_.params_opengles().id());
      if (entidade == nullptr) {
        LOG(ERROR) << "Entidade " << parametros_desenho_.params_opengles().id() << " nao encontrada.";
        return;
      }
      DesenhaEntidadeDetalhada(parametros_desenho_, entidade);
      return;
    }
#endif
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

  // Transparencias devem vir por ultimo porque dependem do que esta atras. As transparencias nao atualizam
  // o buffer de profundidade, ja que se dois objetos transparentes forem desenhados um atras do outro,
  // a ordem nao importa. Ainda assim, o z buffer eh necessario para comparar o objeto transparentes
  // a outros nao transparentes.
  if (parametros_desenho_.transparencias()) {
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
    gl::DesligaTesteProfundidadeEscopo desliga_teste_profundidade_escopo;
    parametros_desenho_.set_alfa_translucidos(0.5);
    DesenhaEntidadesTranslucidas();
    parametros_desenho_.clear_alfa_translucidos();
    DesenhaAuras();
  } else {
    // Desenha os translucidos de forma solida para picking.
    gl::NomesEscopo nomes(0);
    DesenhaEntidadesTranslucidas();
  }

  if (estado_ == ETAB_ENTS_PRESSIONADAS && parametros_desenho_.desenha_rastro_movimento() && !rastros_movimento_.empty()) {
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
    LigaStencil();
    DesenhaRastros();
    DesenhaStencil(COR_AZUL_ALFA);
  }

  if (estado_ == ETAB_DESENHANDO && parametros_desenho_.desenha_forma_selecionada()) {
    DesenhaFormaSelecionada();
  }

  if (parametros_desenho_.desenha_rosa_dos_ventos() && opcoes_.desenha_rosa_dos_ventos()) {
    DesenhaRosaDosVentos();
  }

  if (parametros_desenho_.desenha_quadrado_selecao() && estado_ == ETAB_SELECIONANDO_ENTIDADES) {
    gl::DesligaTesteProfundidadeEscopo desliga_teste_escopo;
    gl::DesabilitaEscopo cull_escopo(GL_CULL_FACE);
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
    gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
    gl::DesvioProfundidade(-3.0f, -30.0f);
    MudaCorAlfa(COR_AZUL_ALFA);
    gl::Retangulo(primeiro_x_3d_, primeiro_y_3d_, ultimo_x_3d_, ultimo_y_3d_);
  }

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }
}

void Tabuleiro::DesenhaTabuleiro() {
  GLuint id_textura = parametros_desenho_.desenha_texturas() && proto_.has_info_textura() ?
      texturas_->Textura(proto_.info_textura().id()) : GL_INVALID_VALUE;
  std::unique_ptr<gl::HabilitaEscopo> habilita_textura;
  if (id_textura != GL_INVALID_VALUE) {
    habilita_textura.reset(new gl::HabilitaEscopo(GL_TEXTURE_2D));
    glBindTexture(GL_TEXTURE_2D, id_textura);
  }

  gl::MatrizEscopo salva_matriz;
  double deltaX = -TamanhoX() * TAMANHO_LADO_QUADRADO;
  double deltaY = -TamanhoY() * TAMANHO_LADO_QUADRADO;
  gl::Normal(0, 0, 1.0f);
  gl::Translada(deltaX / 2.0f,
                deltaY / 2.0f,
                parametros_desenho_.has_offset_terreno() ? parametros_desenho_.offset_terreno() : 0.0f);
  if (parametros_desenho_.has_offset_terreno()) {
    // Para mover entidades acima do plano do olho.
    gl::Desabilita(GL_CULL_FACE);
  } else {
    gl::Habilita(GL_CULL_FACE);
  }
  int id = 0;
  // Desenha o chao mais pro fundo.
  // TODO transformar offsets em constantes.
  gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(2.0f, 20.0f);
  bool usar_textura = id_textura != GL_INVALID_VALUE;
  GLfloat cinza[] = { 0.5f, 0.5f, 0.5f, 1.0f };
  GLfloat cinza_claro[] = { 0.8f, 0.8f, 0.8f, 1.0f };
  if (!usar_textura) {
    MudaCor(cinza_claro);
  } else {
    gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    MudaCor(COR_BRANCA);
  }

  gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  float tamanho_texel_h = 1.0f / TamanhoX();
  float tamanho_texel_v = 1.0f / TamanhoY();
  static const float vertices[] = {
    0.0f, 0.0f,
    TAMANHO_LADO_QUADRADO, 0.0f,
    TAMANHO_LADO_QUADRADO, TAMANHO_LADO_QUADRADO,
    0.0f, TAMANHO_LADO_QUADRADO,
  };
  static const float vertices_texel_ladrilho[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
  };
  static const unsigned short indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, };
  for (int y = 0; y < TamanhoY(); ++y) {
    float inicio_texel_v = (TamanhoY() - y) * tamanho_texel_v;
    float inicio_texel_h = 0.0f;
    for (int x = 0; x < TamanhoX(); ++x) {
      const float vertices_texel_nao_ladrilho[] = {
        inicio_texel_h,                   inicio_texel_v,
        inicio_texel_h + tamanho_texel_h, inicio_texel_v,
        inicio_texel_h + tamanho_texel_h, inicio_texel_v - tamanho_texel_v,
        inicio_texel_h,                   inicio_texel_v - tamanho_texel_v,
      };
      const float* vertices_texels = proto_.ladrilho() ? vertices_texel_ladrilho : vertices_texel_nao_ladrilho;

      // desenha quadrado
      if (id == quadrado_selecionado_ && !usar_textura) {
        MudaCor(cinza);
      }
      DesenhaQuadrado(id, y, x,
                      vertices, vertices_texels, indices);
      if (id == quadrado_selecionado_ && !usar_textura)  {
        MudaCor(cinza_claro);
      }
      // anda 1 quadrado direita
      gl::Translada(TAMANHO_LADO_QUADRADO, 0, 0);
      ++id;
      inicio_texel_h += tamanho_texel_h;
    }
    // volta tudo esquerda e sobe 1 quadrado
    gl::Translada(deltaX, TAMANHO_LADO_QUADRADO, 0);
  }
  gl::DesabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
  gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
}

void Tabuleiro::DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f) {
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second.get();
    if (entidade == nullptr) {
      LOG(ERROR) << "Entidade nao existe.";
      continue;
    }
    // Nao roda disco se estiver arrastando.
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS &&
                                                 EntidadeEstaSelecionada(entidade->Id()));
    parametros_desenho_.set_desenha_barra_vida(entidade->Id() == id_entidade_detalhada_);
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
  // TODO passar a parte nao desenho para a atualizacao.
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
  gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
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
  DesenhaEntidadesBase(std::bind(&Entidade::DesenhaSombra, std::placeholders::_1, std::placeholders::_2, matriz_shear));
  // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
  GLfloat cor_sombra[] = { 0.0f, 0.0f, 0.0f, std::min(0.5f, sinf(kAnguloInclinacao)) };
  gl::HabilitaEscopo habilita_blend(GL_BLEND);
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
      if (fabs(origem[i] - destino[i]) > VELOCIDADE_POR_EIXO) {
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
  for (auto& a : copia_acoes) {
    a->Atualiza();
    if (a->Finalizada()) {
      const auto& ap = a->Proto();
      if (ap.has_id_entidade_destino() &&
          ap.afeta_pontos_vida()) {
        AtualizaPontosVidaEntidadePorAcao(ap.id_entidade_destino(), ap.delta_pontos_vida());
      }
    } else {
      acoes_.push_back(std::unique_ptr<Acao>(a.release()));
    }
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
  gl::IniciaNomes();
  gl::NomesEscopo nomes(0);

  gl::ModoMatriz(GL_PROJECTION);
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::CarregaIdentidade();
  gl::MatrizPicking(x, y, 1.0, 1.0, viewport);
  gl::Perspectiva(CAMPO_VERTICAL_GRAUS, Aspecto(), DISTANCIA_PLANO_CORTE_PROXIMO, DISTANCIA_PLANO_CORTE_DISTANTE);

  // desenha a cena sem firulas.
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
  DesenhaCena();

  // Volta pro modo de desenho, retornando quanto pegou no SELECT.
  *numero_hits = gl::ModoRenderizacao(gl::MR_RENDER);
}

void Tabuleiro::BuscaHitMaisProximo(
    int x, int y, unsigned int* id, unsigned int* pos_pilha, float* profundidade) {
  GLuint buffer_hits[100] = {0};
  GLuint numero_hits = 0;
  EncontraHits(x, y, &numero_hits, buffer_hits);

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
  GLuint pos_pilha_menor = 0;
  GLuint id_menor = 0;
  // Busca o hit mais proximo.
  for (GLuint i = 0; i < numero_hits; ++i) {
    GLuint pos_pilha_corrente = *ptr_hits;
    GLuint z_corrente = *(ptr_hits + 1);
    // A posicao da pilha minimo eh 1.
    GLuint id_corrente = *(ptr_hits + 3 + (pos_pilha_corrente - 1));
    ptr_hits += (3 + (pos_pilha_corrente));
    if (z_corrente <= menor_z) {
      VLOG(3) << "pos_pilha_corrente: " << pos_pilha_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
      menor_z = z_corrente;
      pos_pilha_menor = pos_pilha_corrente;
      id_menor = id_corrente;
    } else {
      VLOG(3) << "Pulando objeto, pos_pilha_corrente: " << pos_pilha_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
    }
  }
  *pos_pilha = pos_pilha_menor;
  *id = id_menor;
  float menor_profundidade = 0.0f;
#if !USAR_OPENGL_ES
  // Profundidade de inteiro para float.
  menor_profundidade = static_cast<float>(menor_z) / static_cast<float>(0xFFFFFFFF);
  if (profundidade != nullptr) {
    *profundidade = menor_profundidade;
  }
#elif 0
  // OBS: tudo isso assume alvo no chao e solo plano.
  // Computa a profundidade na mao para tabuleiro.
  float meio_fov_vertical_rad = (CAMPO_VERTICAL_GRAUS / 2.0f) * GRAUS_PARA_RAD;
  float meia_altura_viewport = altura_ / 2.0f;
  float distancia_olho_near_clip_pixels = meia_altura_viewport * tanf(meio_fov_vertical_rad);
  float distancia_pixel_meio_viewport_pixels = y - meia_altura_viewport;
  float angulo_alfa_rad = atanf(distancia_pixel_meio_viewport_pixels / distancia_olho_near_clip_pixels);
  float angulo_beta_rad = atanf(olho_.altura() / olho_.raio());
  float angulo_teta_rad = (M_PI / 2.0f) - angulo_alfa_rad - angulo_beta_rad;
  float distancia_olho = tanf(angulo_teta_rad) * olho_.altura();
  float distancia_projecao = olho_.raio() - distancia_olho;
  LOG(INFO) << "Y: " << y;
  LOG(INFO) << "Distancia pixel ao centro viewport: " << distancia_pixel_meio_viewport_pixels;
  LOG(INFO) << "Distancia olho ao near clip em pixels: " << distancia_olho_near_clip_pixels;
  LOG(INFO) << "Angulo alfa: centro viewport ao pixel Y: " << (angulo_alfa_rad * RAD_PARA_GRAUS);
  LOG(INFO) << "Angulo beta: entre olho e solo: " << (angulo_beta_rad * RAD_PARA_GRAUS);
  LOG(INFO) << "Angulo teta: entre plano do pixel e eixo Z: " << (angulo_teta_rad * RAD_PARA_GRAUS);
  LOG(INFO) << "Distancia horizontal da projecao ao olho: " << distancia_projecao;
  if (profundidade != nullptr) {
    *profundidade = sqrtf(distancia_olho * distancia_olho + olho_.altura() * olho_.altura()) /
                    (DISTANCIA_PLANO_CORTE_DISTANTE - DISTANCIA_PLANO_CORTE_PROXIMO);
  }
#endif
  VLOG(1) << "Retornando menor profundidade: " << menor_profundidade
          << ", pos_pilha: " << pos_pilha_menor
          << ", id: " << id_menor;
}

bool Tabuleiro::MousePara3d(int x, int y, float* x3d, float* y3d, float* z3d) {
#if !USAR_OPENGL_ES
  GLuint not_used;
  float profundidade;
  BuscaHitMaisProximo(x, y, &not_used, &not_used, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
  return MousePara3d(x, y, profundidade, x3d, y3d, z3d);
#else
  GLuint id;
  GLuint pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
  parametros_desenho_.mutable_params_opengles()->set_id(id);
  // Busca mais detalhado.
  if (pos_pilha == 1) {
    // Para tabuleiro, aumenta resolucao em XY.
    unsigned int id_detalhado;
    parametros_desenho_.mutable_params_opengles()->set_tabuleiro(true);
    parametros_desenho_.set_desenha_entidades(false);
    BuscaHitMaisProximo(x, y, &id_detalhado, &pos_pilha, &profundidade);
    if (profundidade == 1.0f) {
      LOG(ERROR) << "Segunda chamada de BuscaHitMaisProximo nao deu hit em objeto.";
      return false;
    }
    CoordenadaQuadradoDetalhado(id, id_detalhado, x3d, y3d, z3d);
  } else {
    unsigned int id_detalhado;
    parametros_desenho_.mutable_params_opengles()->set_tabuleiro(false);
    parametros_desenho_.set_desenha_entidades(true);
    BuscaHitMaisProximo(x, y, &id_detalhado, &pos_pilha, &profundidade);
    if (profundidade == 1.0f) {
      // Como o desenho da entidade eh aproximado, pode nao ter dado hit. Neste caso, usa a posicao da entidade e boa.
      auto* entidade = BuscaEntidade(id);
      if (entidade == nullptr) {
        LOG(ERROR) << "Deveria encontrar a entidade.";
        return false;
      }
      *x3d = entidade->X();
      *y3d = entidade->Y();
      *z3d = entidade->Z();
    } else {
      // Entidade.
      CoordenadaEntidadeDetalhada(id, id_detalhado, x3d, y3d, z3d);
    }
  }
  // Importante para operacoes no mesmo frame nao se confundirem.
  parametros_desenho_.clear_params_opengles();
  return true;
#endif
}


bool Tabuleiro::MousePara3d(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d) {
#if !USAR_OPENGL_ES
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
  VLOG(3) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
#else
  return MousePara3d(x, y, x3d, y3d, z3d);
#endif
}

void Tabuleiro::TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao) {
  ultimo_x_ = x;
  ultimo_y_ = y;

  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  float x3d, y3d, z3d;
  MousePara3d(x, y, profundidade, &x3d, &y3d, &z3d);
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  ultimo_z_3d_ = z3d;
  primeiro_x_3d_ = x3d;
  primeiro_y_3d_ = y3d;
  primeiro_z_3d_ = z3d;
  if (pos_pilha == 1) {
    // Tabuleiro.
    VLOG(1) << "Picking no tabuleiro id quadrado: " << id;
    SelecionaQuadrado(id);
  } else if (pos_pilha == 2) {
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
  } else if (pos_pilha == 3) {
    VLOG(1) << "Picking em ponto de rolagem id " << id;
    TrataRolagem(static_cast<dir_rolagem_e>(id));
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
  MousePara3d(x, y, &x3d, &y3d, &z3d);
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
      translacoes_rotacoes_antes_.insert(std::make_pair(entidade->Id(), std::make_pair(entidade->TranslacaoZ(), entidade->RotacaoZGraus())));
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
  BuscaHitMaisProximo(x, y, &id, &pos_pilha);
  if (pos_pilha == 1) {
    // Tabuleiro: cria uma entidade nova.
    SelecionaQuadrado(id);
    estado_ = ETAB_QUAD_SELECIONADO;
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    TrataNotificacao(notificacao);
  } else if (pos_pilha == 2) {
    // Entidade.
    if (SelecionaEntidade(id)) {
      auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
      n->set_modo_mestre(modo_mestre_);
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      central_->AdicionaNotificacao(n);
    }
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

bool Tabuleiro::SelecionaEntidade(unsigned int id) {
  VLOG(2) << "Selecionando entidade: " << id;
  ids_entidades_selecionadas_.clear();
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }
  if (!modo_mestre_ && !entidade->SelecionavelParaJogador()) {
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
    if (entidade == nullptr || (!modo_mestre_ && !entidade->SelecionavelParaJogador())) {
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
          // Isso aqui ta meio tosco por causa das imprecisoes do float que no final pode gerar um delta total
          // diferente.
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

#if USAR_OPENGL_ES
void Tabuleiro::CoordenadaQuadradoDetalhado(unsigned int id_quadrado, unsigned int id_detalhado, float* x, float* y, float* z) {
  const float TAMANHO_LADO_QUADRADO_DETALHADO = TAMANHO_LADO_QUADRADO / NUM_DIVISOES_DETALHAMENTO_PICKING;
  const float TAMANHO_LADO_QUADRADO_DETALHADO_2 = TAMANHO_LADO_QUADRADO_DETALHADO / 2.0f;
  float quad_x;
  float quad_y;
  CoordenadaQuadrado(id_quadrado, &quad_x, &quad_y, z);
  quad_x -= TAMANHO_LADO_QUADRADO_2;
  quad_y -= TAMANHO_LADO_QUADRADO_2;
  *x = quad_x;
  *y = quad_y;
  VLOG(3) << "Id quadrado detalhado: " << id_detalhado;
  float dx = static_cast<float>(id_detalhado % NUM_DIVISOES_DETALHAMENTO_PICKING) * TAMANHO_LADO_QUADRADO_DETALHADO +
             TAMANHO_LADO_QUADRADO_DETALHADO_2;
  float dy = static_cast<float>(id_detalhado / NUM_DIVISOES_DETALHAMENTO_PICKING) * TAMANHO_LADO_QUADRADO_DETALHADO +
             TAMANHO_LADO_QUADRADO_DETALHADO_2;
  *x = quad_x + dx;
  *y = quad_y + dy;
  //LOG(INFO) << "IdQuadrado: " << id_quadrado << ", CoordenadaQuadradoDetalhado: " << *x << ", " << *y << ", id_detalhado: " << id_detalhado;
}

void Tabuleiro::CoordenadaEntidadeDetalhada(unsigned int id, unsigned int id_detalhado, float* x, float* y, float* z) {
  // Entidade: aumenta resolucao em Z.
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    LOG(ERROR) << "Nao encontrei entidade";
    return;
  }
  *x = entidade->X();
  *y = entidade->Y();
  *z = entidade->Z() + (id_detalhado * 0.1f);
  VLOG(3) << "Id entidade detalhado " << id_detalhado << ", pos: " << *x << " " << *y << " " << *z;
}
#endif

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
  }
  tabuleiro->set_largura(proto_.largura());
  tabuleiro->set_altura(proto_.altura());
  return notificacao;
}

ntf::Notificacao* Tabuleiro::SerializaOpcoes() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_OPCOES);
  notificacao->mutable_opcoes()->CopyFrom(opcoes_);
  return notificacao;
}

void Tabuleiro::DeserializaPropriedades(const ent::TabuleiroProto& novo_proto) {
  proto_.mutable_luz_ambiente()->CopyFrom(novo_proto.luz_ambiente());
  proto_.mutable_luz_direcional()->CopyFrom(novo_proto.luz_direcional());
  proto_.set_largura(novo_proto.largura());
  proto_.set_altura(novo_proto.altura());
  AtualizaTexturas(novo_proto);
}

ntf::Notificacao* Tabuleiro::SerializaTabuleiro() {
  auto* notificacao = new ntf::Notificacao;
  try {
    notificacao->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    auto* t = notificacao->mutable_tabuleiro();
    t->set_id_cliente(GeraIdCliente());
    t->CopyFrom(proto_);
    t->clear_entidade();  // As entidades vem do mapa de entidades.
    for (const auto& id_ent : entidades_) {
      t->add_entidade()->CopyFrom(id_ent.second->Proto());
    }
    return notificacao;
  } catch (const std::logic_error& error) {
    notificacao->set_tipo(ntf::TN_ERRO);
    notificacao->set_erro(error.what());
    return notificacao;
  }
}

void Tabuleiro::DeserializaTabuleiro(const ntf::Notificacao& notificacao) {
  const auto& tabuleiro = notificacao.tabuleiro();
  bool manter_entidades = tabuleiro.manter_entidades();
  if (manter_entidades) {
    VLOG(1) << "Deserializando tabuleiro mantendo entidades.";
  } else {
    VLOG(1) << "Deserializando tabuleiro todo.";
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
  proto_.clear_entidade();  // As entidades serao armazenadas abaixo.
  proto_.clear_id_cliente();
  bool usar_id = !notificacao.has_endereco();  // Se nao tem endereco, veio da rede.
  if (usar_id && id_cliente_ == 0) {
    // So usa o id novo se nao tiver.
    VLOG(1) << "Alterando id de cliente para " << id_cliente_;
    id_cliente_ = tabuleiro.id_cliente();
  }
  // So recebe as entidades se nao for para manter.
  // O campo entidade eh usado apenas como um marcador
  if (manter_entidades) {
    return;
  }
  for (const auto& ep : tabuleiro.entidade()) {
    auto* e = NovaEntidade(ep, texturas_, central_);
    if (!entidades_.insert(std::make_pair(e->Id(), std::unique_ptr<Entidade>(e))).second) {
      LOG(ERROR) << "Erro adicionando entidade: " << ep.ShortDebugString();
    }
  }
  VLOG(1) << "Foram adicionadas " << tabuleiro.entidade_size() << " entidades";
}

void Tabuleiro::DeserializaOpcoes(const ent::OpcoesProto& novo_proto) {
  opcoes_.CopyFrom(novo_proto);
}

Entidade* Tabuleiro::BuscaEntidade(unsigned int id) {
  auto it = entidades_.find(id);
  return (it != entidades_.end()) ? it->second.get() : nullptr;
}

void Tabuleiro::CopiaEntidadesSelecionadas() {
  entidades_copiadas_.clear();
  for (const unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade = BuscaEntidade(id);
    if (entidade != nullptr) {
      entidades_copiadas_.push_back(entidade->Proto());
      VLOG(1) << "Copiando: " << entidade->Proto().ShortDebugString();
    }
  }
}

void Tabuleiro::ColaEntidadesSelecionadas() {
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
    VLOG(1) << "Estado invalido." << estado_;
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
  // TODO desfazer.
  auto* notificacao = grupo_notificacoes.add_notificacao();
  notificacao->set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
  notificacao->mutable_entidade()->Swap(&nova_entidade);
  TrataNotificacao(grupo_notificacoes);
  {
    // para desfazer
    if (ids_adicionados_.size() == 1) {
      // So tem como desfazer se conseguiu adicionar a entidade.
      notificacao->mutable_entidade()->set_id(ids_adicionados_[0]);
    } else {
      LOG(WARNING) << "Impossivel desfazer a entidade adicionada porque ela no foi criada.";
    }
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}

void Tabuleiro::TrataMovimentoEntidadesSelecionadas(bool vertical, int valor) {
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
    case ntf::TN_ATUALIZAR_ENTIDADE:
      if (!n_original.has_entidade_antes()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_ATUALIZAR_ENTIDADE sem o proto novo e o proto anterior: "
                   << n_original.ShortDebugString();
        break;
      }
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
      n_inversa.mutable_entidade()->CopyFrom(n_original.entidade_antes());
      break;
    case ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE:
      if (!n_original.entidade_antes().has_id() || !n_original.entidade_antes().has_pontos_vida()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_ATUALIZAR_PONTOS_VIDA_ENTIDADE sem o proto novo e o proto anterior: "
                   << n_original.ShortDebugString();
        break;
      }
      // Volta entidade pro estado anterior a atualizacao.
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

int Tabuleiro::GeraIdEntidade(int id_cliente) {
  const int max_id_entidade = (1 << 28);
  int count = max_id_entidade;
  while (count-- > 0) {
    int id = (id_cliente << 28) | proximo_id_entidade_;
    proximo_id_entidade_ = ((proximo_id_entidade_ + 1) % max_id_entidade);
    auto it = entidades_.find(id);
    if (it == entidades_.end()) {
      return id;
    }
  }
  throw std::logic_error("Limite de entidades alcancado para cliente.");
}

int Tabuleiro::GeraIdCliente() {
  const int max_id_cliente = 15;
  int count = max_id_cliente;
  while (count-- > 0) {
    int id_cliente = proximo_id_cliente_;
    auto it = clientes_.find(id_cliente);
    // O id zero esta sempre reservado para o mestre.
    proximo_id_cliente_ = ((proximo_id_cliente_) % max_id_cliente) + 1;
    if (it == clientes_.end()) {
      return id_cliente;
    }
  }
  throw std::logic_error("Limite de clientes alcancado.");
}

void Tabuleiro::AtualizaTexturas(const ent::TabuleiroProto& novo_proto) {
  VLOG(2) << "Novo proto: " << novo_proto.ShortDebugString() << ", velho: " << proto_.ShortDebugString();
  // Libera textura anterior se houver e for diferente da corrente.
  if (proto_.has_info_textura() && proto_.info_textura().id() != novo_proto.info_textura().id()) {
    VLOG(2) << "Liberando textura: " << proto_.info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->CopyFrom(proto_.info_textura());
    central_->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_proto.has_info_textura() && novo_proto.info_textura().id() != proto_.info_textura().id()) {
    VLOG(2) << "Carregando textura: " << novo_proto.info_textura().id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->mutable_info_textura()->CopyFrom(novo_proto.info_textura());
    central_->AdicionaNotificacao(nc);
  }

  if (novo_proto.has_info_textura()) {
    proto_.mutable_info_textura()->CopyFrom(novo_proto.info_textura());
    proto_.set_ladrilho(novo_proto.ladrilho());
  } else {
    proto_.clear_info_textura();
    proto_.clear_ladrilho();
  }
}

void Tabuleiro::DesenhaQuadrado(unsigned int id,
                                int linha, int coluna,
                                const float* vertices, const float* vertices_texels, const unsigned short* indices) {
#if USAR_OPENGL_ES
  if (parametros_desenho_.has_params_opengles()) {
    if (parametros_desenho_.params_opengles().tabuleiro() && parametros_desenho_.params_opengles().id() == id) {
      DesenhaQuadradoDetalhado(linha, coluna);
    }
    return;
  }
#endif
  gl::CarregaNome(id);
  gl::PonteiroVertices(2, GL_FLOAT, vertices);
  gl::PonteiroVerticesTexturas(2, GL_FLOAT, vertices_texels);
  gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
}

void Tabuleiro::DesenhaGrade() {
  MudaCor(COR_PRETA);
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
  for (int i = limite_inferior; i <= x_2; ++i) {
    float x = i * TAMANHO_LADO_QUADRADO + incremento;
    gl::Retangulo(x - EXPESSURA_LINHA_2, -tamanho_y_2, x + EXPESSURA_LINHA_2, tamanho_y_2);
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
    gl::Retangulo(-tamanho_x_2, y - EXPESSURA_LINHA_2, tamanho_x_2, y + EXPESSURA_LINHA_2);
  }
}

void Tabuleiro::DesenhaListaPontosVida() {
  if (lista_pontos_vida_.empty()) {
    return;
  }
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::MatrizEscopo salva_matriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0, largura_, 0, altura_, 0, 1);

  {
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
    gl::CarregaIdentidade();
    gl::Desabilita(GL_DEPTH_TEST);
    gl::Desabilita(GL_LIGHTING);
    std::string titulo("Lista PV");
    gl::Translada(largura_ - 2 - 8 * titulo.size(), altura_ - 15.0f, 0.0f);
    MudaCor(COR_BRANCA);
    DesenhaString(titulo);
    gl::Translada((titulo.size() - 3) * 8, 0.0f, 0.0f);
    for (int pv : lista_pontos_vida_) {
      MudaCor(pv >= 0 ? COR_VERDE : COR_VERMELHA);
      char str[4];
      snprintf(str, 4, "%d", abs(pv));
      gl::Translada(0.0f, -15.0f, 0.0f);
      DesenhaString(str);
    }
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
#if ANDROID
  std::string tempo_str;
#else
  std::string tempo_str = std::to_string(maior_tempo_ms);
#endif
  while (tempo_str.size() < 4) {
    tempo_str.insert(0, "0");
  }
#if USAR_OPENGL_ES
  // OpenGL es ta sem caracteres.
  std::cout << "TR: " << tempo_str << std::endl;
#else
  // Modo 2d.
  gl::ModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  // Eixo com origem embaixo esquerda.
  gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
  gl::ModoMatriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  gl::Desabilita(GL_DEPTH_TEST);
  gl::Desabilita(GL_LIGHTING);
  gl::Translada(0.0, altura_ - 15.0f, 0.0f);
  DesenhaStringTempo(tempo_str);
#endif
}

double Tabuleiro::Aspecto() const {
  return static_cast<double>(largura_) / static_cast<double>(altura_);
}

void Tabuleiro::ModoJogador() {
#if USAR_WATCHDOG
  watchdog_.Para();
#endif
  modo_mestre_ = false;
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

}  // namespace ent
