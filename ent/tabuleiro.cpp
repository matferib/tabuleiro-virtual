#include <algorithm>
#include <boost/timer/timer.hpp>
#include <cassert>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
#include <vector>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "log/log.h"
#include "ntf/notificacao.pb.h"


namespace ent {

namespace {
/** campo de visao vertical em graus. */
const double CAMPO_VERTICAL = 60.0;

/** altura inicial do olho. */
const double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
const double OLHO_ALTURA_MAXIMA = 15;
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

/** Retorna o quadrado da distancia de um ponto a outro. */
double DistanciaQuadrado(const Posicao& p1, const Posicao& p2) {
  return pow(p1.x() - p2.x(), 2) + pow(p1.y() - p2.y(), 2) + pow(p1.z() - p2.z(), 2);
}


/** Desenha apenas a string. */
void DesenhaString(const std::string& s) {
  glRasterPos2i(1, 1);
  for (const char c : s) {
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
  }
}

/** Renderiza o tempo de desenho no canto superior esquerdo da tela. */
void DesenhaStringTempo(const std::string& tempo) {
  MudaCor(COR_PRETA);
  glRectf(0.0f, 0.0f, tempo.size() * 8.0f + 2.0f, 15.0f);

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

}  // namespace.

Tabuleiro::Tabuleiro(Texturas* texturas, ntf::CentralNotificacoes* central) :
    id_cliente_(0),
    id_entidade_detalhada_(0xFFFFFFFF),
    quadrado_selecionado_(-1),
    estado_(ETAB_OCIOSO), proximo_id_entidade_(0), proximo_id_cliente_(1),
    texturas_(texturas),
    central_(central),
    modo_mestre_(true) {
  central_->RegistraReceptor(this);
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
  // Valores iniciais.
  largura_ = altura_ = 0;
  ultimo_x_3d_ = ultimo_y_3d_ = ultimo_z_3d_ = 0;
  // Modelos.
  auto* modelo_padrao = new EntidadeProto;  // padrao eh cone verde.
  modelo_padrao->mutable_cor()->set_g(1.0f);
  mapa_modelos_.insert(std::make_pair("Padrão", std::unique_ptr<ent::EntidadeProto>(modelo_padrao)));
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
      if (acao_selecionada_ == nullptr) {
        acao_selecionada_ = nova_acao;
      }
      mapa_acoes_.insert(std::make_pair(a.id(), std::unique_ptr<AcaoProto>(nova_acao)));
    }
  }
}

Tabuleiro::~Tabuleiro() {
  if (proto_.has_info_textura()) {
    VLOG(2) << "Liberando textura: " << proto_.info_textura().id();
    auto* nl = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    nl->mutable_info_textura()->set_id(proto_.info_textura().id());
    central_->AdicionaNotificacao(nl);
  }
}

int Tabuleiro::TamanhoX() const {
  return proto_.largura();
}

int Tabuleiro::TamanhoY() const {
  return proto_.altura();
}

void Tabuleiro::Desenha() {
  // Varios lugares chamam desenha cena com parametros especifico. Essa funcao
  // desenha a cena padrao, entao ela restaura os parametros para seus valores
  // default. Alem disso a matriz de projecao eh diferente para picking.
  parametros_desenho_.Clear();
  parametros_desenho_.set_modo_mestre(modo_mestre_);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(CAMPO_VERTICAL, Aspecto(), 0.5, 500.0);
  // Aplica opcoes do jogador.
  parametros_desenho_.set_desenha_fps(opcoes_.mostrar_fps());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
  DesenhaCena();
}

void Tabuleiro::AdicionaEntidade(const ntf::Notificacao& notificacao) {
  try {
    if (notificacao.local()) {
      if (estado_ != ETAB_QUAD_SELECIONADO) {
        return;
      }
      int id_entidade = GeraIdEntidade(id_cliente_);
      double x, y, z;
      CoordenadaQuadrado(quadrado_selecionado_, &x, &y, &z);
      auto* entidade = NovaEntidade(TE_ENTIDADE, texturas_, central_);
      EntidadeProto modelo(*modelo_selecionado_);
      PreencheEntidadeProto(id_cliente_, id_entidade, !modo_mestre_, x, y, z, &modelo);
      entidade->Inicializa(modelo);
      entidades_.insert(std::make_pair(entidade->Id(), entidade));
      SelecionaEntidade(entidade->Id());
      // Envia a entidade para os outros.
      auto* n = ntf::NovaNotificacao(notificacao.tipo());
      n->mutable_entidade()->CopyFrom(entidade->Proto());
      central_->AdicionaNotificacaoRemota(n);
    } else {
      // Mensagem veio de fora.
      auto* entidade = NovaEntidade(notificacao.entidade().tipo(), texturas_, central_);
      entidade->Inicializa(notificacao.entidade());
      entidades_.insert(std::make_pair(entidade->Id(), entidade));
    }
  } catch (const std::logic_error& erro) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro(erro.what());
    central_->AdicionaNotificacao(n);
  }
}

void Tabuleiro::RemoveEntidade(const ntf::Notificacao& notificacao) {
  unsigned int id_remocao = 0;
  if (!notificacao.local()) {
    // Comando vindo de fora.
    id_remocao = notificacao.entidade().id();
  } else if ((estado_ == ETAB_ENT_SELECIONADA) || estado_ == ETAB_ENTS_SELECIONADAS) {
    for (unsigned int id_remocao : ids_entidades_selecionadas_) {
      // Envia para os clientes.
      auto* n = ntf::NovaNotificacao(ntf::TN_REMOVER_ENTIDADE);
      n->mutable_entidade()->set_id(id_remocao);
      central_->AdicionaNotificacaoRemota(n);
    }
  } else {
    VLOG(1) << "Remocao de entidade sem seleção";
    return;
  }
  if (RemoveEntidade(id_remocao)) {
    DeselecionaEntidade();
  }
}

void Tabuleiro::AtualizaBitsEntidade(int bits) {
  if (estado_ != ETAB_ENT_SELECIONADA && estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    EntidadeProto proto = entidade_selecionada->Proto();
    if ((bits & BIT_VISIBILIDADE) > 0) {
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
    if ((bits & BIT_CAIDA)) {
      proto.set_caida(!proto.caida());
    }
    if ((bits & BIT_MORTA)) {
      proto.set_morta(!proto.morta());
    }
    proto.set_id(id);
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
    n.mutable_entidade()->Swap(&proto);
    TrataNotificacao(n);
  }
}

void Tabuleiro::AtualizaPontosVidaEntidade(int delta_pontos_vida) {
  if (estado_ != ETAB_ENT_SELECIONADA && estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    auto proto = entidade_selecionada->Proto();
    int pontos_vida = proto.pontos_vida();
    if (pontos_vida >= 0 && pontos_vida + delta_pontos_vida < 0) {
      entidade_selecionada->MataEntidade();
      proto = entidade_selecionada->Proto();
    }
    proto.set_pontos_vida(pontos_vida + delta_pontos_vida);
    proto.set_id(entidade_selecionada->Id());
    // Atualizacao.
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
    n.mutable_entidade()->Swap(&proto);
    TrataNotificacao(n);
    // Acao.
    ntf::Notificacao na;
    na.set_tipo(ntf::TN_ADICIONAR_ACAO);
    auto* a = na.mutable_acao();
    a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    a->set_id_entidade_destino(entidade_selecionada->Id());
    a->set_delta_pontos_vida(delta_pontos_vida);
    TrataNotificacao(na);
  }
}

void Tabuleiro::AtualizaPontosVidaEntidade(unsigned int id_entidade, int delta_pontos_vida) {
  auto* entidade = BuscaEntidade(id_entidade);
  if (entidade == nullptr) {
    LOG(WARNING) << "Entidade nao encontrada: " << id_entidade;
    return;
  }
  auto proto = entidade->Proto();
  int pontos_vida = proto.pontos_vida();
  if (pontos_vida >= 0 && pontos_vida + delta_pontos_vida < 0) {
    entidade->MataEntidade();
    proto = entidade->Proto();
  }
  proto.set_pontos_vida(pontos_vida + delta_pontos_vida);
  proto.set_id(entidade->Id());
  // Atualizacao.
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_ENTIDADE);
  n.mutable_entidade()->Swap(&proto);
  TrataNotificacao(n);
  // Acao.
  ntf::Notificacao na;
  na.set_tipo(ntf::TN_ADICIONAR_ACAO);
  auto* a = na.mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->set_id_entidade_destino(entidade->Id());
  a->set_delta_pontos_vida(delta_pontos_vida);
  TrataNotificacao(na);
}

void Tabuleiro::AcumulaPontosVida(int pv) {
  if (pv >= 1000 || pv <= -1000 || pv == 0) {
    LOG(ERROR) << "Ignorando pv: " << pv;
    return;
  }
  lista_pontos_vida_.emplace_back(pv);
}

void Tabuleiro::LimpaListaPontosVida() {
  lista_pontos_vida_.clear();
}

bool Tabuleiro::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_RESPOSTA_CONEXAO:
      if (!notificacao.has_erro()) {
        ModoJogador();
      }
      return true;
    case ntf::TN_ADICIONAR_ENTIDADE:
      try {
        AdicionaEntidade(notificacao);
      } catch (const std::logic_error& e) {
        LOG(ERROR) << "Limite de entidades alcançado: " << e.what();
      }
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
      RemoveEntidade(notificacao);
      return true;
    }
    case ntf::TN_TEMPORIZADOR: {
      AtualizaOlho();
      for (auto& id_ent : entidades_) {
        id_ent.second->Atualiza();
      }
      for (auto& a : acoes_) {
        a->Atualiza();
      }
      return true;
    }
    case ntf::TN_SERIALIZAR_TABULEIRO: {
      auto* nt_tabuleiro = SerializaTabuleiro();
      if (notificacao.has_endereco()) {
        std::string nt_tabuleiro_str = nt_tabuleiro->SerializeAsString();
        // Salvar no endereco.
        std::ofstream arquivo(notificacao.endereco());
        arquivo.write(nt_tabuleiro_str.c_str(), nt_tabuleiro_str.size());
        if (!arquivo) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro escrevendo arquivo";
        }
        arquivo.close();
        delete nt_tabuleiro;
      } else {
        // Enviar remotamente.
        central_->AdicionaNotificacaoRemota(nt_tabuleiro);
      }
      return true;
    }
    case ntf::TN_DESERIALIZAR_TABULEIRO: {
      if (notificacao.has_endereco()) {
        // Deserializar de arquivo.
        std::ifstream arquivo(notificacao.endereco());
        if (!arquivo) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro lendo arquivo";
          return true;
        }
        ntf::Notificacao nt_tabuleiro;
        if (!LeArquivoProto(notificacao.endereco(), &nt_tabuleiro)) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro restaurando notificacao do arquivo";
          return true;
        }
        DeserializaTabuleiro(nt_tabuleiro);
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
      const auto& proto = notificacao.entidade();
      auto* entidade = BuscaEntidade(proto.id());
      if (entidade == nullptr) {
        LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
        return true;
      }
      entidade->Destino(proto);
      return true;
    }
    case ntf::TN_ATUALIZAR_ENTIDADE: {
      const auto& proto = notificacao.entidade();
      auto* entidade = BuscaEntidade(proto.id());
      if (entidade == nullptr) {
        LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
        return true;
      }
      entidade->AtualizaProto(proto);
      if (notificacao.local()) {
        // So repassa a notificacao pros clientes se a origem dela for local, para evitar ficar enviando infinitamente.
        auto* n_remota = new ntf::Notificacao(notificacao);
        central_->AdicionaNotificacaoRemota(n_remota);
      }
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
    case ntf::TN_ATUALIZAR_TABULEIRO: {
      DeserializaPropriedades(notificacao.tabuleiro());
      if (notificacao.local()) {
        // So repassa a notificacao pros clientes se a origem dela for local, para evitar ficar enviando infinitamente.
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

void Tabuleiro::TrataRodela(int delta) {
  // move o olho no eixo Z de acordo com o eixo Y do movimento
  float olho_raio = olho_.raio();
  olho_raio -= (delta * SENSIBILIDADE_RODA);
  if (olho_raio < OLHO_RAIO_MINIMO) {
    olho_raio = OLHO_RAIO_MINIMO;
  }
  else if (olho_raio > OLHO_RAIO_MAXIMO) {
    olho_raio = OLHO_RAIO_MAXIMO;
  }
  olho_.set_raio(olho_raio);
}

void Tabuleiro::TrataMovimento() {
  id_entidade_detalhada_ = 0xFFFFFFFF;
}

void Tabuleiro::TrataMovimento(botao_e botao, int x, int y) {
  if (estado_ == ETAB_ROTACAO) {
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
  } else if (estado_ == ETAB_ENT_PRESSIONADA) {
    // Realiza o movimento da entidade paralelo ao XY na mesma altura do click original.
    parametros_desenho_.set_offset_terreno(ultimo_z_3d_);
    parametros_desenho_.set_desenha_entidades(false);
    GLdouble nx, ny, nz;
    if (!MousePara3d(x, y, &nx, &ny, &nz)) {
      return;
    }
    EntidadeSelecionada()->MoveDelta(nx - ultimo_x_3d_, ny - ultimo_y_3d_, 0.0f);
    ultimo_x_ = x;
    ultimo_y_ = y;
    ultimo_x_3d_ = nx;
    ultimo_y_3d_ = ny;
  } else if (estado_ == ETAB_DESLIZANDO) {
    // Faz picking do tabuleiro sem entidades.
    parametros_desenho_.set_desenha_entidades(false);
    GLdouble nx, ny, nz;
    if (!MousePara3d(x, y, &nx, &ny, &nz)) {
      return;
    }

    float delta_x = nx - ultimo_x_3d_;
    float delta_y = ny - ultimo_y_3d_;
    auto* p = olho_.mutable_alvo();
    p->set_x(p->x() - delta_x);
    p->set_y(p->y() - delta_y);
    olho_.clear_destino();

    ultimo_x_ = x;
    ultimo_y_ = y;
    // No caso de deslizamento, nao precisa atualizar as coordenadas do ultimo_*_3d porque por definicao
    // do movimento, ela fica fixa (o tabuleiro desliza acompanhando o dedo).
  }
}

void Tabuleiro::TrataBotaoPressionado(botao_e botao, int x, int y) {
  ultimo_x_ = x;
  ultimo_y_ = y;
  if (botao == BOTAO_MEIO) {
    estado_anterior_rotacao_ = estado_;
    estado_ = ETAB_ROTACAO;
  } else if (botao == BOTAO_DIREITO) {
    TrataCliqueDireito(x, y);
  } else if (botao == BOTAO_ESQUERDO) {
    TrataCliqueEsquerdo(x, y);
  }
}

void Tabuleiro::TrataBotaoAlternarSelecaoEntidadePressionado(int x, int y) {
  ultimo_x_ = x;
  ultimo_y_ = y;
  TrataCliqueEsquerdo(x, y, true  /*alternar selecao*/);
}

void Tabuleiro::TrataBotaoAcaoPressionado(botao_e botao, int x, int y) {
  AcaoProto acao_proto;
  if (botao == BOTAO_ESQUERDO) {
    // usa acao padrao.
    VLOG(2) << "Acao padrao";
    acao_proto.CopyFrom(*acao_selecionada_);
  } else {
    // Usa acoes.
    VLOG(2) << "Acao sinalizacao";
    acao_proto.set_tipo(ACAO_SINALIZACAO); 
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
    double x, y, z;
    CoordenadaQuadrado(id, &x, &y, &z);
    auto* pos_quadrado = acao_proto.mutable_pos_quadrado();
    pos_quadrado->set_x(x);
    pos_quadrado->set_y(y);
    pos_quadrado->set_z(z);
    // Posicao exata do clique.
    double x3d, y3d, z3d;
    if (MousePara3d(x, y, profundidade, &x3d, &y3d, &z3d)) {
      auto* pos_tabuleiro = acao_proto.mutable_pos_tabuleiro();
      pos_tabuleiro->set_x(x3d);
      pos_tabuleiro->set_y(y3d);
      pos_tabuleiro->set_z(z3d);
    }
  }

  auto* entidade_selecionada = EntidadeSelecionada();
  if (entidade_selecionada != nullptr) {
    acao_proto.set_id_entidade_origem(entidade_selecionada->Id());
  }
  if (!lista_pontos_vida_.empty() && acao_proto.has_id_entidade_destino()) {
    int delta_pontos_vida = lista_pontos_vida_.front();
    lista_pontos_vida_.pop_front();
    acao_proto.set_delta_pontos_vida(delta_pontos_vida);
    acao_proto.set_afeta_pontos_vida(true);
  }

  VLOG(2) << "Acao: " << acao_proto.ShortDebugString();
  auto* n = ntf::NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
  n->mutable_acao()->Swap(&acao_proto);
  central_->AdicionaNotificacao(n);
}

void Tabuleiro::TrataDuploClique(botao_e botao, int x, int y) {
  if (botao == BOTAO_ESQUERDO) {
    TrataDuploCliqueEsquerdo(x, y);
  } else if (botao == BOTAO_DIREITO) {
    TrataDuploCliqueDireito(x, y);
  }
}

void Tabuleiro::TrataBotaoLiberado(botao_e botao) {
  switch (estado_) {
    case ETAB_ROTACAO:
    case ETAB_DESLIZANDO:
      estado_ = estado_anterior_rotacao_;
      return;
    case ETAB_ENT_PRESSIONADA: {
      if (botao != BOTAO_ESQUERDO) {
        return;
      }
      auto* n = new ntf::Notificacao;
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      auto* entidade_selecionada = EntidadeSelecionada();
      e->set_id(entidade_selecionada->Id());
      auto* p = e->mutable_destino();
      p->set_x(entidade_selecionada->X());
      p->set_y(entidade_selecionada->Y());
      p->set_z(entidade_selecionada->Z());
      central_->AdicionaNotificacaoRemota(n);
      estado_ = ETAB_ENT_SELECIONADA;
      return;
    }
    case ETAB_QUAD_PRESSIONADO:
      estado_ = ETAB_QUAD_SELECIONADO;
      return;
    default:
      ;
  }
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
  glViewport(0, 0, (GLint)largura, (GLint)altura);
  largura_ = largura;
  altura_ = altura;
}

void Tabuleiro::InicializaGL() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

  // Nao desenha as costas dos poligonos.
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
}

void Tabuleiro::SelecionaModeloEntidade(const std::string& id_modelo) {
  auto it = mapa_modelos_.find(id_modelo);
  if (it == mapa_modelos_.end()) {
    LOG(ERROR) << "Id de modelo inválido: " << id_modelo;
    return;
  }
  modelo_selecionado_ = it->second.get();
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
  boost::timer::cpu_timer timer;
  if (parametros_desenho_.desenha_fps()) {
    timer.start();
  }

  glEnable(GL_DEPTH_TEST);
  glClearColor(proto_.luz_ambiente().r(), 
               proto_.luz_ambiente().g(),
               proto_.luz_ambiente().b(),
               proto_.luz_ambiente().a());
  if (parametros_desenho_.limpa_fundo()) {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  glClear(GL_DEPTH_BUFFER_BIT);
  for (int i = 1; i < 8; ++i) {
    glDisable(GL_LIGHT0 + i);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  const Posicao& alvo = olho_.alvo();
  gluLookAt(
    // from.
    alvo.x() + cos(olho_.rotacao_rad()) * olho_.raio(),
    alvo.y() + sin(olho_.rotacao_rad()) * olho_.raio(),
    alvo.z() + olho_.altura(),
    // to.
    alvo.x(), alvo.y(), alvo.z(),
    // up
    0, 0, 1.0);
  Posicao* pos_olho = parametros_desenho_.mutable_pos_olho();;
  pos_olho->set_x(olho_.alvo().x() + cosf(olho_.rotacao_rad()) * olho_.raio());
  pos_olho->set_y(olho_.alvo().y() + sinf(olho_.rotacao_rad()) * olho_.raio());
  pos_olho->set_z(olho_.altura());

  if (parametros_desenho_.iluminacao()) {
    glEnable(GL_LIGHTING);
    GLfloat cor_luz_ambiente[] = { proto_.luz_ambiente().r(),
                                   proto_.luz_ambiente().g(),
                                   proto_.luz_ambiente().b(),
                                   proto_.luz_ambiente().a()};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, cor_luz_ambiente);

    // Iluminação distante direcional.
    glPushMatrix();
    // O vetor inicial esta no leste (origem da luz). O quarte elemento indica uma luz no infinito.
    GLfloat pos_luz[] = { 1.0, 0.0f, 0.0f, 0.0f };
    // Roda no eixo Z (X->Y) em direcao a posicao entao inclina a luz no eixo -Y (de X->Z).
    glRotatef(proto_.luz_direcional().posicao_graus(), 0.0f, 0.0f, 1.0f);
    glRotatef(proto_.luz_direcional().inclinacao_graus(), 0.0f, -1.0f, 0.0f);
    glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);
    glPopMatrix();
    // A cor da luz direcional.
    GLfloat cor_luz[] = { proto_.luz_direcional().cor().r(),
                          proto_.luz_direcional().cor().g(),
                          proto_.luz_direcional().cor().b(),
                          proto_.luz_direcional().cor().a() };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, cor_luz);
    glEnable(GL_LIGHT0);

    // Posiciona as luzes dinâmicas.
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      auto* e = it->second;
      e->DesenhaLuz(&parametros_desenho_);
    }
  } else {
    glDisable(GL_LIGHTING);
  }

  //ceu_.desenha(parametros_desenho_);

  // desenha tabuleiro do sul para o norte.
  GLuint id_textura = parametros_desenho_.desenha_texturas() && proto_.has_info_textura() ?
      texturas_->Textura(proto_.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id_textura);
  }

  glPushMatrix();
  double deltaX = -TamanhoX() * TAMANHO_LADO_QUADRADO;
  double deltaY = -TamanhoY() * TAMANHO_LADO_QUADRADO;
  glNormal3f(0, 0, 1.0f);
  glTranslated(deltaX / 2.0,
               deltaY / 2.0,
               parametros_desenho_.has_offset_terreno() ? parametros_desenho_.offset_terreno() : 0.0f);
  int id = 0;
  glEnable(GL_POLYGON_OFFSET_FILL);
  // Desenha o chao mais pro fundo.
  // TODO transformar offsets em constantes.
  glPolygonOffset(0.04f, 0.04f);
  for (int y = 0; y < TamanhoY(); ++y) {
    for (int x = 0; x < TamanhoX(); ++x) {
      // desenha quadrado
      DesenhaQuadrado(id, y, x, id == quadrado_selecionado_, id_textura != GL_INVALID_VALUE);
      // anda 1 quadrado direita
      glTranslated(TAMANHO_LADO_QUADRADO, 0, 0);
      ++id;
    }
    // volta tudo esquerda e sobe 1 quadrado
    glTranslated(deltaX, TAMANHO_LADO_QUADRADO, 0);
  }
  glDisable(GL_POLYGON_OFFSET_FILL);
  glPopMatrix();
  glDisable(GL_TEXTURE_2D);

  if (parametros_desenho_.desenha_grade()) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    // TODO transformar offsets em constantes.
    glPolygonOffset(-0.04f, -0.04f);
    DesenhaGrade();
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
  if (!parametros_desenho_.desenha_entidades()) {
    return;
  }

  // desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
  // na hora do picking.
  glPushName(0);
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second;
    parametros_desenho_.set_entidade_selecionada(EntidadeEstaSelecionada(entidade->Id()));
    parametros_desenho_.set_desenha_barra_vida(entidade->Id() == id_entidade_detalhada_);
    entidade->Desenha(&parametros_desenho_);
  }
  parametros_desenho_.set_entidade_selecionada(false);
  parametros_desenho_.set_desenha_barra_vida(false);
  glPopName();

  if (parametros_desenho_.desenha_acoes()) {
    std::vector<std::unique_ptr<Acao>> copia_acoes;
    copia_acoes.swap(acoes_);
    for (auto& a : copia_acoes) {
      VLOG(4) << "Desenhando acao";
      a->Desenha(&parametros_desenho_);
      if (a->Finalizada()) {
        const auto& ap = a->Proto();
        if (ap.has_delta_pontos_vida() &&
            ap.tipo() != ACAO_DELTA_PONTOS_VIDA &&
            ap.has_id_entidade_destino() &&
            ap.afeta_pontos_vida()) {
          AtualizaPontosVidaEntidade(ap.id_entidade_destino(), ap.delta_pontos_vida());
        }
      } else {
        acoes_.push_back(std::unique_ptr<Acao>(a.release()));
      }
    }
    VLOG(3) << "Numero de acoes ativas: " << acoes_.size();
  }

  glEnable(GL_BLEND);
  // Sombras.
  if (parametros_desenho_.desenha_sombras() &&
      proto_.luz_direcional().inclinacao_graus() > 5.0 &&
      proto_.luz_direcional().inclinacao_graus() < 180.0f) {
    const float kAnguloInclinacao = proto_.luz_direcional().inclinacao_graus() * GRAUS_PARA_RAD;
    const float kAnguloPosicao = proto_.luz_direcional().posicao_graus() * GRAUS_PARA_RAD;
    float fator_shear = proto_.luz_direcional().inclinacao_graus() == 90.0f ? 0.0f : 1.0f / tanf(kAnguloInclinacao);
    // Matriz eh column major, ou seja, esta invertida.
    // A ideia eh adicionar ao x a altura * fator de shear.
    GLfloat matriz_shear[] = {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      fator_shear * -cosf(kAnguloPosicao), fator_shear * -sinf(kAnguloPosicao), 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Habilita o stencil para desenhar apenas uma vez as sombras.
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);  // stencil zerado.
    glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);  // Funcao de teste sempre retornara true e referencia 0xFF & 0xFF = 0xFF.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // Quando passar (sempre) no stencil e no depth, escrevera referencia (0xFF).
    bool desenha_texturas = parametros_desenho_.desenha_texturas();
    parametros_desenho_.set_desenha_texturas(false);
    glColorMask(0, 0, 0, 0);  // Para nao desenhar nada de verdade, apenas no stencil.
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      Entidade* entidade = it->second;
      parametros_desenho_.set_entidade_selecionada(EntidadeEstaSelecionada(entidade->Id()));
      entidade->DesenhaSombra(&parametros_desenho_, matriz_shear);
    }
    parametros_desenho_.set_entidade_selecionada(false);

    // Neste ponto, os pixels desenhados tem 0xFF no stencil. Reabilita o desenho.
    glColorMask(true, true, true, true);
    glStencilFunc(GL_EQUAL, 0xFF, 0xFF);  // So passara no teste quem tiver 0xFF.
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);  // Mantem os valores do stencil.
    // Desenha uma chapa preta na tela toda, preenchera so os buracos do stencil.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // Eixo com origem embaixo esquerda.
    glOrtho(0, largura_, 0, altura_, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    GLfloat cor_sombra[] = { 0.0f, 0.0f, 0.0f, sinf(kAnguloInclinacao) };
    MudaCor(cor_sombra);
    glDisable(GL_DEPTH_TEST);
    glRectf(0.0f, 0.0f, largura_, altura_);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    parametros_desenho_.set_desenha_texturas(desenha_texturas);
    glDisable(GL_STENCIL_TEST);
  }

  // Transparencias devem vir por ultimo porque dependem do que esta atras. As transparencias nao atualizam o buffer
  // de profundidade, ja que se dois objetos transparentes forem desenhados um atras do outro, a ordem nao importa.
  // Ainda assim, o z buffer eh necessario para comparar o objeto transparentes com nao transparentes.
  if (parametros_desenho_.transparencias()) {
    glDepthMask(false);
    parametros_desenho_.set_alpha_translucidos(0.5);
    glPushName(0);
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      Entidade* entidade = it->second;
      parametros_desenho_.set_entidade_selecionada(EntidadeEstaSelecionada(entidade->Id()));
      parametros_desenho_.set_desenha_barra_vida(entidade->Id() == id_entidade_detalhada_);
      entidade->DesenhaTranslucido(&parametros_desenho_);
    }
    parametros_desenho_.set_entidade_selecionada(false);
    parametros_desenho_.set_desenha_barra_vida(false);

    glPopName();
    parametros_desenho_.clear_alpha_translucidos();
    if (parametros_desenho_.desenha_aura()) {
      for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
        Entidade* entidade = it->second;
        entidade->DesenhaAura(&parametros_desenho_);
      }
    }
    glDepthMask(true);
    glDisable(GL_BLEND);
  } else {
    // Desenha os translucidos de forma solida para picking.
    glPushName(0);
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      Entidade* entidade = it->second;
      parametros_desenho_.set_entidade_selecionada(EntidadeEstaSelecionada(entidade->Id()));
      entidade->DesenhaTranslucido(&parametros_desenho_);
    }
    glPopName();
    parametros_desenho_.set_entidade_selecionada(false);
    parametros_desenho_.set_desenha_barra_vida(false);
  }

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }

  if (parametros_desenho_.desenha_fps()) {
    glFlush();
    timer.stop();
    std::string tempo_str = timer.format(boost::timer::default_places, "%w");
    // Modo 2d.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Eixo com origem embaixo esquerda.
    glOrtho(0, largura_, 0, altura_, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glTranslatef(0.0, altura_ - 15.0f, 0.0f);
    DesenhaStringTempo(tempo_str);
  }
}

void Tabuleiro::AtualizaOlho() {
  if (!olho_.has_destino()) {
    return;
  }
  auto* po = olho_.mutable_alvo();
  double origem[] = { po->x(), po->y(), po->z() };
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
  po->set_x(origem[0]);
  po->set_y(origem[1]);
  po->set_z(origem[2]);
  if (chegou) {
    olho_.clear_destino();
  }
}

// Esta operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador e a
// profundidade de quem o acertou.
void Tabuleiro::EncontraHits(int x, int y, unsigned int* numero_hits, unsigned int* buffer_hits) {
  // inicia o buffer de picking (selecao)
  glSelectBuffer(100, buffer_hits);
  // entra no modo de selecao e limpa a pilha de nomes e inicia com 0
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0); // inicia a pilha de nomes com 0 para sempre haver um nome.

  glMatrixMode(GL_PROJECTION);
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glLoadIdentity();
  gluPickMatrix(x, y, 1.0, 1.0, viewport);
  gluPerspective(CAMPO_VERTICAL, Aspecto(), 0.5, 500.0);

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
  DesenhaCena();

  // Volta pro modo de desenho, retornando quanto pegou no SELECT.
  *numero_hits = glRenderMode(GL_RENDER);
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
  VLOG(4) << "numero de hits no buffer de picking: " << numero_hits;
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
    if (z_corrente < menor_z) {
      VLOG(4) << "pos_pilha_corrente: " << pos_pilha_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
      menor_z = z_corrente;
      pos_pilha_menor = pos_pilha_corrente;
      id_menor = id_corrente;
    } else {
      VLOG(4) << "pulando objeto mais longe...";
    }
  }
  *pos_pilha = pos_pilha_menor;
  *id = id_menor;
  // Normaliza profundidade.
  float menor_profundidade = static_cast<float>(menor_z) / static_cast<float>(0xFFFFFFFF);
  if (profundidade != nullptr) {
    *profundidade = menor_profundidade;
  }
  VLOG(3) << "Retornando menor profundidade: " << menor_profundidade
          << ", pos_pilha: " << pos_pilha_menor
          << ", id: " << id_menor;
}

bool Tabuleiro::MousePara3d(int x, int y, double* x3d, double* y3d, double* z3d) {
  GLuint not_used;
  float profundidade;
  BuscaHitMaisProximo(x, y, &not_used, &not_used, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
  return MousePara3d(x, y, profundidade, x3d, y3d, z3d);
}

bool Tabuleiro::MousePara3d(int x, int y, float profundidade, double* x3d, double* y3d, double* z3d) {
  GLdouble modelview[16], projection[16];
  GLint viewport[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);
  if (!gluUnProject(x, y, profundidade,
                    modelview, projection, viewport,
                    x3d, y3d, z3d)) {
    LOG(ERROR) << "Falha ao projetar x y no mundo 3d.";
    return false;
  }
  return true;
}

void Tabuleiro::TrataCliqueEsquerdo(int x, int y, bool alterna_selecao) {
  unsigned int id, pos_pilha;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha, &profundidade);
  if (pos_pilha == 1) {
    // Tabuleiro.
    VLOG(1) << "Picking no tabuleiro.";
    SelecionaQuadrado(id);
  } else if (pos_pilha > 1) {
    // Entidade.
    double x3d, y3d, z3d;
    MousePara3d(x, y, profundidade, &x3d, &y3d, &z3d);
    ultimo_x_3d_ = x3d;
    ultimo_y_3d_ = y3d;
    ultimo_z_3d_ = z3d;
    VLOG(1) << "Picking entidade.";
    if (alterna_selecao) {
      AlternaSelecaoEntidade(id);
    } else {
      SelecionaEntidade(id);
      estado_ = ETAB_ENT_PRESSIONADA;
    }
  } else {
    VLOG(1) << "Picking lugar nenhum.";
    DeselecionaEntidade();
  }
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataCliqueDireito(int x, int y) {
  estado_anterior_rotacao_ = estado_;
  double x3d, y3d, z3d;
  parametros_desenho_.set_desenha_entidades(false);
  MousePara3d(x, y, &x3d, &y3d, &z3d);
  ultimo_x_3d_ = x3d;
  ultimo_y_3d_ = y3d;
  estado_ = ETAB_DESLIZANDO;
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataDuploCliqueEsquerdo(int x, int y) {
  unsigned int id, pos_pilha;
  BuscaHitMaisProximo(x, y, &id, &pos_pilha);
  if (pos_pilha == 1) {
    // Tabuleiro: cria uma entidade nova.
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    AdicionaEntidade(notificacao);
  } else if (pos_pilha > 1) {
    // Entidade.
    SelecionaEntidade(id);
    auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
    n->set_modo_mestre(modo_mestre_);
    n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
    n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
    central_->AdicionaNotificacao(n);
  } else {
    ;
  }
}

void Tabuleiro::TrataDuploCliqueDireito(int x, int y) {
  parametros_desenho_.set_desenha_entidades(false);
  GLdouble x3d, y3d, z3d;
  if (!MousePara3d(x, y, &x3d, &y3d, &z3d)) {
    return;
  }
  auto* p = olho_.mutable_destino();
  p->set_x(x3d);
  p->set_y(y3d);
  p->set_z(z3d);
}

void Tabuleiro::SelecionaEntidade(unsigned int id) {
  VLOG(1) << "Selecionando entidade: " << id;
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }
  ids_entidades_selecionadas_.clear();
  ids_entidades_selecionadas_.insert(entidade->Id());
  quadrado_selecionado_ = -1;
  estado_ = ETAB_ENT_SELECIONADA;
}

void Tabuleiro::AlternaSelecaoEntidade(unsigned int id) {
  VLOG(1) << "Selecionando entidade: " << id;
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }

  if (EntidadeEstaSelecionada(id)) {
    ids_entidades_selecionadas_.erase(id);
  } else {
    ids_entidades_selecionadas_.insert(id);
  }
  // Alterna o estado.
  if (ids_entidades_selecionadas_.empty()) {
    estado_ = ETAB_OCIOSO;
  } else if (ids_entidades_selecionadas_.size() == 1) {
    estado_ = ETAB_ENT_SELECIONADA;
  } else {
    estado_ = ETAB_ENTS_SELECIONADAS;
  }
  quadrado_selecionado_ = -1;
}

bool Tabuleiro::EntidadeEstaSelecionada(unsigned int id) {
  return ids_entidades_selecionadas_.find(id) != ids_entidades_selecionadas_.end();
}

void Tabuleiro::DeselecionaEntidade() {
  ids_entidades_selecionadas_.clear();
  quadrado_selecionado_ = -1;
  estado_ = ETAB_OCIOSO;
}

void Tabuleiro::SelecionaQuadrado(int id_quadrado) {
  quadrado_selecionado_ = id_quadrado;
  ids_entidades_selecionadas_.clear();
  estado_ = ETAB_QUAD_PRESSIONADO;
}

void Tabuleiro::CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z) {
  int quad_x = id_quadrado % TamanhoX();
  int quad_y = id_quadrado / TamanhoY();

  // centro do quadrado
  *x = ((quad_x * TAMANHO_LADO_QUADRADO) + TAMANHO_LADO_QUADRADO_2) -
       (TamanhoX() * TAMANHO_LADO_QUADRADO_2);
  *y = ((quad_y * TAMANHO_LADO_QUADRADO) + TAMANHO_LADO_QUADRADO_2) -
       (TamanhoY() * TAMANHO_LADO_QUADRADO_2);
  *z = 0;
}

ntf::Notificacao* Tabuleiro::SerializaPropriedades() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO);
  auto* tabuleiro = notificacao->mutable_tabuleiro();
  tabuleiro->set_id_cliente(id_cliente_);
  tabuleiro->mutable_luz_ambiente()->CopyFrom(proto_.luz_ambiente());
  tabuleiro->mutable_luz_direcional()->CopyFrom(proto_.luz_direcional());
  if (proto_.has_info_textura()) {
    tabuleiro->mutable_info_textura()->CopyFrom(proto_.info_textura());
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
  if (!entidades_.empty()) {
    LOG(ERROR) << "Tabuleiro não está vazio!";
    return;
  }
  if (notificacao.has_erro()) {
    LOG(ERROR) << "Erro ao deserializar tabuleiro: " << notificacao.erro();
    auto* n = new ntf::Notificacao;
    n->set_tipo(ntf::TN_DESCONECTAR);
    central_->AdicionaNotificacao(n);
    return;
  }
  const auto& tabuleiro = notificacao.tabuleiro();
  AtualizaTexturas(tabuleiro);
  proto_.CopyFrom(tabuleiro);
  id_cliente_ = tabuleiro.id_cliente();
  for (const auto& ep : tabuleiro.entidade()) {
    auto* e = NovaEntidade(ep.tipo(), texturas_, central_);
    e->Inicializa(ep);
    if (!entidades_.insert({ e->Id(), e }).second) {
      LOG(ERROR) << "Erro adicionando entidade: " << ep.ShortDebugString();
    }
  }
  proto_.clear_entidade();
}

void Tabuleiro::DeserializaOpcoes(const ent::OpcoesProto& novo_proto) {
  opcoes_.CopyFrom(novo_proto);
}

Entidade* Tabuleiro::BuscaEntidade(unsigned int id) {
  auto it = entidades_.find(id);
  return (it != entidades_.end()) ? it->second : nullptr;
}

bool Tabuleiro::RemoveEntidade(unsigned int id) {
  MapaEntidades::iterator res_find = entidades_.find(id);
  if (res_find == entidades_.end()) {
    return false;
  }
  Entidade* entidade = res_find->second;
  entidades_.erase(res_find);
  delete entidade;
  // Retorna apenas para verificacao.
  return EntidadeEstaSelecionada(id);
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
  } else {
    proto_.clear_info_textura();
  }
}

void Tabuleiro::DesenhaQuadrado(
    unsigned int id, int linha, int coluna, bool selecionado, bool usar_textura) {
  glLoadName(id);
  if (!usar_textura) {
    // desenha o quadrado negro embaixo.
    if (selecionado) {
      GLfloat cinza[] = { 0.5f, 0.5f, 0.5f, 1.0f };
      MudaCor(cinza);
    } else {
      GLfloat cinza_claro[] = { 0.8f, 0.8f, 0.8f, 1.0f };
      MudaCor(cinza_claro);
    }
    glRectf(0.0f, 0.0f, TAMANHO_LADO_QUADRADO, TAMANHO_LADO_QUADRADO);
  } else {
    float tamanho_texel_h = 1.0f / TamanhoX();
    float tamanho_texel_v = 1.0f / TamanhoY();
    // desenha o quadrado branco.
    MudaCor(COR_BRANCA);
    glPushMatrix();
    glBegin(GL_QUADS);
    // O quadrado eh desenhado EB, DB, DC, EC. A textura tem o eixo Y invertido.
    float tamanho_y_linha = TamanhoY() - linha;
    glTexCoord2f(coluna * tamanho_texel_h, tamanho_y_linha * tamanho_texel_v);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f((coluna + 1) * tamanho_texel_h, tamanho_y_linha * tamanho_texel_v);
    glVertex3f(TAMANHO_LADO_QUADRADO, 0.0f, 0.0f);
    glTexCoord2f((coluna + 1) * tamanho_texel_h, (tamanho_y_linha - 1) * tamanho_texel_v);
    glVertex3f(TAMANHO_LADO_QUADRADO, TAMANHO_LADO_QUADRADO, 0.0f);
    glTexCoord2f(coluna * tamanho_texel_h, (tamanho_y_linha - 1) * tamanho_texel_v);
    glVertex3f(0.0f, TAMANHO_LADO_QUADRADO, 0.0f);
    glEnd();
    glPopMatrix();
  }
}

void Tabuleiro::DesenhaGrade() {
  MudaCor(COR_PRETA);
  // Linhas verticais (S-N).
  const float tamanho_y_2 = (TamanhoY() / 2.0f) * TAMANHO_LADO_QUADRADO;
  const float tamanho_x_2 = (TamanhoX() / 2.0f) * TAMANHO_LADO_QUADRADO;
  const int x_2 = TamanhoX()  / 2;
  const int y_2 = TamanhoY() / 2;
  for (int i = -x_2; i <= x_2; ++i) {
    float x = i * TAMANHO_LADO_QUADRADO;
    glRectf(x - EXPESSURA_LINHA_2, -tamanho_y_2, x + EXPESSURA_LINHA_2, tamanho_y_2);
  }
  // Linhas horizontais (W-E).
  for (int i = -y_2; i <= y_2; ++i) {
    float y = i * TAMANHO_LADO_QUADRADO;
    glRectf(-tamanho_x_2, y - EXPESSURA_LINHA_2, tamanho_x_2, y + EXPESSURA_LINHA_2);
  }
}

void Tabuleiro::DesenhaListaPontosVida() {
  if (lista_pontos_vida_.empty()) {
    return;
  }
  // Modo 2d: eixo com origem embaixo esquerda.
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, largura_, 0, altura_, 0, 1);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  std::string titulo("Lista PV");
  glTranslatef(largura_ - 2 - 8 * titulo.size(), altura_ - 15.0f, 0.0f);
  MudaCor(COR_BRANCA);
  DesenhaString(titulo);
  glTranslatef((titulo.size() - 3) * 8, 0.0f, 0.0f);
  for (int pv : lista_pontos_vida_) {
    MudaCor(pv >= 0 ? COR_VERDE : COR_VERMELHA);
    char str[4];
    snprintf(str, 4, "%d", abs(pv));
    glTranslatef(0.0f, -15.0f, 0.0f);
    DesenhaString(str);
  }
  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

}

double Tabuleiro::Aspecto() const {
  return static_cast<double>(largura_) / static_cast<double>(altura_);
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
  return it->second;
}

}  // namespace ent
