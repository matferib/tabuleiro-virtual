#if USAR_QT
#include <QApplication>
#include <QClipboard>
#include <google/protobuf/text_format.h>
#else
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

#if USAR_OPENGL_ES
#define USAR_MAPEAMENTO_SOMBRAS_OPENGLES 1
#else
#define USAR_MAPEAMENTO_SOMBRAS_OPENGLES 0
#endif

using google::protobuf::RepeatedField;

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

/** Distancia maxima do olho da entidade de referencia. */
const float OLHO_DISTANCIA_MAXIMA_CAMERA_PRESA = 5.0f * TAMANHO_LADO_QUADRADO;

/** sensibilidade da rodela do mouse. */
const double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
const double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** expessura da linha da grade do tabuleiro. */
const float EXPESSURA_LINHA = 0.1f;
const float EXPESSURA_LINHA_2 = EXPESSURA_LINHA / 2.0f;
/** velocidade do olho. */
const float VELOCIDADE_OLHO_M_S = TAMANHO_LADO_QUADRADO * 10.0f;

/** Tempo que o detalhamento mostra os detalhes no hover. */
const int TEMPO_DETALHAMENTO_MS = 500;

/** tamanho maximo da lista de eventos para desfazer. */
const unsigned int TAMANHO_MAXIMO_LISTA = 10;

// Os offsets servem para evitar zfight. Eles adicionam à profundidade um valor
// dz * escala + r * unidades, onde dz eh grande dependendo do angulo do poligono em relacao
// a camera e r eh o menor offset que gera diferenca no zbuffer.
// Valores positivos afastam, negativos aproximam.
const float OFFSET_TERRENO_ESCALA_DZ = 1.0f;
const float OFFSET_TERRENO_ESCALA_R  = 2.0f;
//const float OFFSET_GRADE_ESCALA_DZ   = 0.5f;
//const float OFFSET_GRADE_ESCALA_R    = 1.0f;
const float OFFSET_RASTRO_ESCALA_DZ  = -2.0f;
const float OFFSET_RASTRO_ESCALA_R  = -20.0f;

/** Distancia minima entre pontos no desenho livre. */
const float DELTA_MINIMO_DESENHO_LIVRE = TAMANHO_LADO_QUADRADO / 2.0f;

/** A Translacao e a rotacao de objetos so ocorre depois que houver essa distancia de pixels percorrida pelo mouse. */
const int DELTA_MINIMO_TRANSLACAO_ROTACAO = 5;

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
    case ent::ETAB_RELEVO:
      return "ETAB_RELEVO";
    default:
      return "DESCONHECIDO";
  }
}

// O delta de pontos de vida afeta outros bits tambem.
void PreencheNotificacaoDeltaPontosVida(
    const Entidade& entidade, int delta_pontos_vida, ntf::Notificacao* n, ntf::Notificacao* n_desfazer = nullptr) {
  n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  auto* entidade_depois = n->mutable_entidade();
  entidade_depois->set_id(entidade.Id());
  entidade_depois->set_pontos_vida(entidade.PontosVida() + delta_pontos_vida);

  if (n_desfazer != nullptr) {
    n_desfazer->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
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

void SalvaConfiguracoes(const OpcoesProto& proto) {
  try {
    arq::EscreveArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
    LOG(INFO) << "Salvando opcoes em arquivo.";
  } catch (const std::logic_error& e) {
    LOG(ERROR) << "Falha salvando opcoes: " << e.what();
  }
}

}  // namespace.

Tabuleiro::Tabuleiro(
    const OpcoesProto& opcoes, tex::Texturas* texturas, const m3d::Modelos3d* m3d,
    ntf::CentralNotificacoes* central)
    : id_cliente_(0),
      proximo_id_cliente_(1),
      texturas_(texturas),
      m3d_(m3d),
      central_(central),
      modo_mestre_(true) {
  central_->RegistraReceptor(this);

  // Modelos.
  auto* modelo_padrao = new EntidadeProto;  // padrao eh cone verde.
  modelo_padrao->mutable_cor()->set_g(1.0f);
  mapa_modelos_.insert(std::make_pair("Padrão", std::unique_ptr<EntidadeProto>(modelo_padrao)));
  modelo_selecionado_.first = "Padrão";
  modelo_selecionado_.second = modelo_padrao;
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
  CarregaControleVirtual();

  opcoes_ = opcoes;
#if DEBUG
  opcoes_.set_mostra_fps(true);
  //opcoes_.set_desenha_olho(true);
#endif

  EstadoInicial(false);
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
  LiberaFramebuffer();
  LiberaTextura();
  LiberaControleVirtual();
}

void Tabuleiro::LiberaTextura() {
  TabuleiroProto dummy;
  for (const auto& sub_cenario : proto_.sub_cenario()) {
    VLOG(2) << "Liberando textura: " << sub_cenario.info_textura().id();
    dummy.set_id_cenario(sub_cenario.id_cenario());
    AtualizaTexturas(dummy);
  }
  dummy.set_id_cenario(CENARIO_PRINCIPAL);
  VLOG(2) << "Liberando textura: " << proto_.info_textura().id();
  AtualizaTexturas(dummy);
}

void Tabuleiro::LiberaFramebuffer() {
  if (!MapeamentoSombras()) {
    return;
  }
  LOG(ERROR) << "Liberando framebuffer";
  gl::ApagaFramebuffers(1, &framebuffer_);
  gl::ApagaTexturas(1, &textura_framebuffer_);
  gl::ApagaRenderbuffers(1, &renderbuffer_framebuffer_);
}


void Tabuleiro::EstadoInicial(bool reiniciar_grafico) {
  // Proto do tabuleiro.
  proto_.Clear();
  cenario_corrente_ = CENARIO_PRINCIPAL;
  proto_corrente_ = &proto_;
  // Iluminacao.
  ReiniciaIluminacao(&proto_);
  // Olho.
  ReiniciaCamera();

  // Valores iniciais.
  ultimo_x_ = ultimo_y_ = 0;
  ultimo_x_3d_ = ultimo_y_3d_ = ultimo_z_3d_ = 0;
  primeiro_x_3d_ = primeiro_y_3d_ = primeiro_z_3d_ = 0;
  ciclos_para_atualizar_ = -1;
  // Mapa de entidades e acoes vazios.
  entidades_.clear();
  acoes_.clear();
  // Entidades selecionadas.
  ids_entidades_selecionadas_.clear();
  // Outras variaveis.
  id_entidade_detalhada_ = Entidade::IdInvalido;
  tipo_entidade_detalhada_ = OBJ_INVALIDO;
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
  modo_clique_ = MODO_NORMAL;
  // Lista objetos.
  pagina_lista_objetos_ = 0;
  if (reiniciar_grafico) {
    LiberaTextura();
    IniciaGL();
    // Atencao V_ERRO so pode ser usado com contexto grafico.
    V_ERRO("estado inicial pos grafico");
  }
}

void Tabuleiro::ConfiguraProjecao() {
  if (MapeamentoSombras() && parametros_desenho_.desenha_sombra_projetada()) {
    float val = std::max(TamanhoX(), TamanhoY()) * TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO;
    gl::Ortogonal(-val, val, -val, val,
                  0.0 /*DISTANCIA_PLANO_CORTE_PROXIMO*/, 200.0f);
    return;
  }
  if (camera_ == CAMERA_ISOMETRICA) {
    const Posicao& alvo = olho_.alvo();
    // o tamanho do vetor
    float dif_z = alvo.z() - olho_.pos().z();
    float distancia = TAMANHO_LADO_QUADRADO + fabs(dif_z);
    const float largura = distancia * Aspecto() / 2.0f;
    const float altura = distancia / 2.0f;
    //LOG_EVERY_N(INFO, 30) << "Distancia: " << distancia;
    gl::Ortogonal(-largura, largura, -altura, altura,
                  0.0f /*DISTANCIA_PLANO_CORTE_PROXIMO*/, distancia * 1.2f);
  } else {
    gl::Perspectiva(CAMPO_VERTICAL_GRAUS, Aspecto(), DISTANCIA_PLANO_CORTE_PROXIMO, DISTANCIA_PLANO_CORTE_DISTANTE);
  }
}

void Tabuleiro::ConfiguraOlhar() {
  if (MapeamentoSombras() && parametros_desenho_.desenha_sombra_projetada()) {
    Matrix4 mr;
    mr.rotateY(-proto_corrente_->luz_direcional().inclinacao_graus());
    mr.rotateZ(proto_corrente_->luz_direcional().posicao_graus());
    mr.scale(150.0f);  // TODO valor.
    Vector4 vl(1.0f, 0.0f, 0.0f, 1.0f);
    vl = mr * vl;
    //LOG(INFO) << vl;
    Vector4 up(0.0f, 0.0f, 1.0f, 1.0f);
    if (fabs(vl.x) < 0.001f && fabs(vl.y) < 0.001f) {
      up.x = 0.0f;
      up.y = 1.0f;
      up.z = 0.0f;
    }
    // Para usar a posicao alvo do olho como referencia.
    //const Posicao& alvo = olho_.alvo();
    gl::OlharPara(
        // from.
        //alvo.x() + vl.x, alvo.y() + vl.y, alvo.z() + vl.z,
        vl.x, vl.y, vl.z,
        // to.
        //alvo.x(), alvo.y(), alvo.z(),
        0, 0, 0,
        // up
        up.x, up.y, up.z);
    //Matrix4 mt = gl::LeMatriz(gl::MATRIZ_SOMBRA);
    //LOG(INFO) << "mt: " << mt;
    //Vector4 vt(10.0f, 0.0f, 0.0f, 1.0f);
    //vt = vt * mt;
    //LOG(INFO) << "vt: " << (vt * mt);
    return;
  }
  const Posicao& alvo = olho_.alvo();
  if (camera_ == CAMERA_ISOMETRICA) {
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

void Tabuleiro::DesenhaSombraProjetada() {
  if (!parametros_desenho_.desenha_sombras()) {
    return;
  }
  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  // Zera as coisas nao usadas na sombra.
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_transparencias(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_id_acao(false);
  parametros_desenho_.set_desenha_detalhes(false);
  parametros_desenho_.set_desenha_eventos_entidades(false);
  parametros_desenho_.set_desenha_efeitos_entidades(false);
  parametros_desenho_.set_desenha_lista_objetos(false);
  parametros_desenho_.set_desenha_lista_jogadores(false);
  parametros_desenho_.set_desenha_fps(false);
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
  parametros_desenho_.set_iluminacao(false);
  parametros_desenho_.set_desenha_texturas(false);
  parametros_desenho_.set_desenha_grade(false);
  parametros_desenho_.set_desenha_aura(false);
  parametros_desenho_.set_desenha_quadrado_selecao(false);
  parametros_desenho_.set_desenha_rastro_movimento(false);
  parametros_desenho_.set_desenha_forma_selecionada(false);
  parametros_desenho_.set_desenha_nevoa(false);
  parametros_desenho_.set_desenha_coordenadas(false);
  parametros_desenho_.set_desenha_sombra_projetada(true);
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  parametros_desenho_.set_desenha_controle_virtual(false);

  if (usar_sampler_sombras_) {
    gl::UsaShader(gl::TSH_SIMPLES);
  } else {
    gl::UsaShader(gl::TSH_PROFUNDIDADE);
  }
  gl::UnidadeTextura(GL_TEXTURE1);
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::UnidadeTextura(GL_TEXTURE0);
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::Viewport(0, 0, 1024, 1024);
  gl::MudarModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade(false);
  ConfiguraProjecao();
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  V_ERRO("LigacaoComFramebufferSombraProjetada");
#if !USAR_MAPEAMENTO_SOMBRAS_OPENGLES
  gl::BufferDesenho(GL_NONE);
#endif
  DesenhaCena();
}

int Tabuleiro::Desenha() {
  V_ERRO_RET("InicioDesenha");
  TimerEscopo timer_escopo(this, opcoes_.mostra_fps());

  auto passou_ms = timer_para_renderizacao_.elapsed().wall / 1000000ULL;
  timer_para_renderizacao_.start();

  // Varios lugares chamam desenha cena com parametros especifico. Essa funcao
  // desenha a cena padrao, entao ela restaura os parametros para seus valores
  // default. Alem disso a matriz de projecao eh diferente para picking.
  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  gl::TipoShader tipo_shader;
  auto* entidade_referencia = BuscaEntidade(id_camera_presa_);
  if (entidade_referencia != nullptr && entidade_referencia->Proto().tipo_visao() == VISAO_ESCURO && visao_escuro_ &&
      (!VisaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    parametros_desenho_.set_tipo_visao(entidade_referencia->Proto().tipo_visao());
    parametros_desenho_.set_desenha_sombras(false);
    parametros_desenho_.set_desenha_sombra_projetada(false);
    tipo_shader = gl::TSH_PRETO_BRANCO;
  } else if (entidade_referencia != nullptr && entidade_referencia->Proto().tipo_visao() == VISAO_BAIXA_LUMINOSIDADE) {
    parametros_desenho_.set_tipo_visao(entidade_referencia->Proto().tipo_visao());
    parametros_desenho_.set_multiplicador_visao_penumbra(1.4f);
    tipo_shader = gl::TSH_LUZ;
  } else {
    tipo_shader = gl::TSH_LUZ;
  }
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  // Aplica opcoes do jogador.
  parametros_desenho_.set_desenha_lista_objetos(opcoes_.mostra_lista_objetos());
  parametros_desenho_.set_desenha_lista_jogadores(opcoes_.mostra_lista_jogadores());
  parametros_desenho_.set_desenha_fps(opcoes_.mostra_fps());
  parametros_desenho_.set_desenha_grade(opcoes_.desenha_grade());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
  if (modo_debug_) {
    parametros_desenho_.set_iluminacao(false);
    parametros_desenho_.set_desenha_texturas(false);
    parametros_desenho_.set_desenha_grade(false);
    parametros_desenho_.set_desenha_fps(false);
    parametros_desenho_.set_desenha_aura(false);
    parametros_desenho_.set_desenha_sombras(false);
    parametros_desenho_.set_desenha_sombra_projetada(false);
    parametros_desenho_.set_limpa_fundo(false);
    parametros_desenho_.set_transparencias(false);
    parametros_desenho_.set_desenha_acoes(false);
    parametros_desenho_.set_desenha_lista_pontos_vida(false);
    parametros_desenho_.set_desenha_quadrado_selecao(false);
    parametros_desenho_.set_desenha_rastro_movimento(false);
    parametros_desenho_.set_desenha_forma_selecionada(false);
    parametros_desenho_.set_desenha_rosa_dos_ventos(false);
    parametros_desenho_.set_desenha_nevoa(false);
    parametros_desenho_.set_desenha_coordenadas(false);
  }
  V_ERRO_RET("Antes desenha sombras");

  if (MapeamentoSombras() && parametros_desenho_.desenha_sombras()) {
    GLint original;
    gl::Le(GL_FRAMEBUFFER_BINDING, &original);
    ParametrosDesenho salva_pd(parametros_desenho_);
    DesenhaSombraProjetada();
    V_ERRO_RET("Depois DesenhaSombraProjetada");
    // Restaura os valores e usa a textura como sombra.
    gl::UsaShader(tipo_shader);
    gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
    // Desloca os componentes xyz do espaco [-1,1] para [0,1] que eh o formato armazenado no mapa de sombras.
    gl::MudarModoMatriz(gl::MATRIZ_PROJECAO_SOMBRA);
    gl::CarregaIdentidade(false);
    Matrix4 bias(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0);
    gl::MultiplicaMatriz(bias.get(), false);
    ConfiguraProjecao();  // antes de parametros_desenho_.set_desenha_sombra_projetada para configurar para luz.
    gl::MudarModoMatriz(gl::MATRIZ_PROJECAO);
    gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
#if !USAR_MAPEAMENTO_SOMBRAS_OPENGLES
    gl::BufferDesenho(GL_BACK);
#endif
    gl::UnidadeTextura(GL_TEXTURE1);
    gl::LigacaoComTextura(GL_TEXTURE_2D, textura_framebuffer_);
    gl::UnidadeTextura(GL_TEXTURE0);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
    parametros_desenho_ = salva_pd;
  } else {
    gl::UsaShader(tipo_shader);
    gl::UnidadeTextura(GL_TEXTURE0);
  }
  V_ERRO_RET("MeioDesenha");
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::MudarModoMatriz(GL_PROJECTION);
  gl::CarregaIdentidade();
  ConfiguraProjecao();
  DesenhaCena();
  V_ERRO_RET("FimDesenha");
  return passou_ms;
}

void Tabuleiro::AdicionaEntidadeNotificando(const ntf::Notificacao& notificacao) {
  try {
    if (notificacao.local()) {
      EntidadeProto modelo(notificacao.has_entidade() ? notificacao.entidade() : *modelo_selecionado_.second);
      if (modelo.tipo() == TE_FORMA && !EmModoMestre(true)) {
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
        modelo.mutable_pos()->set_id_cenario(cenario_corrente_);
      } else if (!Desfazendo()) {
        // Se nao estiver desfazendo, poe a entidade no cenario corrente.
        modelo.mutable_pos()->set_id_cenario(cenario_corrente_);
      }
      unsigned int id_entidade = GeraIdEntidade(id_cliente_);
      if (processando_grupo_) {
        ids_adicionados_.push_back(id_entidade);
      }
      // Visibilidade e selecionabilidade: se nao estiver desfazendo, usa o modo mestre para determinar
      // se a entidade eh visivel e selecionavel para os jogadores.
      if (!Desfazendo()) {
        modelo.set_visivel(!EmModoMestre(true));
        modelo.set_selecionavel_para_jogador(!EmModoMestre(true));
        modelo.set_id(id_entidade);
      } else {
        if (BuscaEntidade(modelo.id()) != nullptr) {
          // Este caso eh raro, mas talvez possa acontecer quando estiver perto do limite de entidades.
          // Isso tem potencial de erro caso o mestre remova entidade de jogadores.
          throw std::logic_error("Id da entidade já está sendo usado.");
        }
      }
      auto* entidade = NovaEntidade(modelo, texturas_, m3d_, central_);
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
      // Selecao: queremos selecionar entidades criadas ou coladas, mas apenas quando nao estiver tratando comando de desfazer.
      if (!Desfazendo()) {
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
      }
      if (!Desfazendo()) {
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
      auto* entidade = NovaEntidade(notificacao.entidade(), texturas_, m3d_, central_);
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
    if ((bits & BIT_VISIBILIDADE) > 0 &&
        (EmModoMestre(true) || proto_original.selecionavel_para_jogador())) {
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
    if ((bits & BIT_VISIBILIDADE) > 0 &&
        (EmModoMestre(true) || proto_original.selecionavel_para_jogador())) {
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
  if (notificacao.local()) {
    auto* n_remota = new ntf::Notificacao(notificacao);
    central_->AdicionaNotificacaoRemota(n_remota);
    // Para desfazer. O if eh so uma otimizacao de performance, pois a funcao AdicionaNotificacaoListaEventos faz o mesmo.
    if (!processando_grupo_ && !ignorar_lista_eventos_) {
      ntf::Notificacao n_desfazer(notificacao);
      n_desfazer.mutable_entidade_antes()->CopyFrom(entidade->Proto());
      AdicionaNotificacaoListaEventos(n_desfazer);
    }
  }
  entidade->AtualizaParcial(notificacao.entidade());
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
  modo_clique_ = MODO_ACAO;
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
  modo_clique_ = MODO_ACAO;
}

int Tabuleiro::LeValorListaPontosVida(const Entidade* entidade, const std::string& id_acao) {
  if (modo_dano_automatico_) {
    if (entidade == nullptr) {
      return 0;
    }
    return -entidade->ValorParaAcao(id_acao);
  } else {
    int delta_pontos_vida = lista_pontos_vida_.front();
    lista_pontos_vida_.pop_front();
    return delta_pontos_vida;
  }
}

bool Tabuleiro::HaValorListaPontosVida() {
  return !lista_pontos_vida_.empty() || modo_dano_automatico_;
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
      if (EmModoMestre()) {
        // cliente desconectado.
        for (auto it : clientes_) {
          if (it.second == notificacao.id_rede()) {
            LOG(INFO) << "Removendo cliente: " << notificacao.id_rede();
            clientes_.erase(it.first);
            mestres_secundarios_.erase(it.first);
            return true;
          }
        }
        LOG(ERROR) << "Nao encontrei cliente desconectado: '" << notificacao.id_rede() << "'";
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
    case ntf::TN_RESPOSTA_CONEXAO: {
      if (notificacao.local()) {
        if (!notificacao.has_erro()) {
          auto* ni = ntf::NovaNotificacao(ntf::TN_INFO);
          ni->set_erro(std::string("Conectado ao servidor"));
          central_->AdicionaNotificacao(ni);
          // texturas cuidara disso.
          // Aqui comeca o fluxo de envio de texturas de servidor para cliente. Nessa primeira mensagem
          // o cliente envia seus ids para o servidor.
          auto* nit = ntf::NovaNotificacao(ntf::TN_ENVIAR_ID_TEXTURAS_E_MODELOS_3D);
          nit->set_id_rede(notificacao.id_rede());
          VLOG(1) << "Enviando TN_ENVIAR_ID_TEXTURAS_E_MODELOS_3D: " << nit->DebugString();
          central_->AdicionaNotificacao(nit);
        } else {
          AlterarModoMestre(true);  // volta modo mestre.
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro conectando ao servidor: ") + notificacao.erro());
          central_->AdicionaNotificacao(ne);
        }
      }
      return true;
    }
    case ntf::TN_ADICIONAR_ENTIDADE:
      AdicionaEntidadeNotificando(notificacao);
      return true;
    case ntf::TN_ADICIONAR_ACAO: {
      std::unique_ptr<Acao> acao(NovaAcao(notificacao.acao(), this));
      // A acao pode estar finalizada se o setup dela estiver incorreto. Eh possivel haver estes casos
      // porque durante a construcao nao ha verificacao. Por exemplo, uma acao de toque sem destino eh
      // contruida como Finalizada.
      if (acao == nullptr || acao->Finalizada()) {
        return true;
      }
      acoes_.push_back(std::move(acao));
      if (notificacao.local() && !notificacao.acao().local_apenas()) {
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
      auto passou_ms = timer_para_atualizacoes_.elapsed().wall / 1000000ULL;
      //auto passou_ms = INTERVALO_NOTIFICACAO_MS;
      timer_para_atualizacoes_.start();
      AtualizaEntidades(passou_ms);
      AtualizaOlho(passou_ms, false  /*forcar*/);
      AtualizaAcoes(passou_ms);
      if (ciclos_para_atualizar_ == 0) {
        if (ModoClique() == MODO_TERRENO) {
          RefrescaTerrenoParaClientes();
          ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_TERRENO;
        } else {
          RefrescaMovimentosParciais();
          ciclos_para_atualizar_ = CICLOS_PARA_ATUALIZAR_MOVIMENTOS_PARCIAIS;
        }
      } else if (ciclos_para_atualizar_ > 0) {
        --ciclos_para_atualizar_;
      }
      if (temporizador_detalhamento_ms_ > 0) {
        temporizador_detalhamento_ms_ -= passou_ms;
      }
#if USAR_WATCHDOG
      watchdog_.Refresca();
#endif
      return true;
    }
    case ntf::TN_REINICIAR_TABULEIRO: {
      EstadoInicial(true);
      // Repassa aos clientes.
      if (notificacao.local()) {
        central_->AdicionaNotificacaoRemota(ntf::NovaNotificacao(ntf::TN_REINICIAR_TABULEIRO));
      }
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO_SE_NECESSARIO_OU_SALVAR_DIRETO: {
      if (TemNome()) {
        auto* n = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
        n->set_endereco("");  // Endereco vazio sinaliza para reusar o nome.
        central_->AdicionaNotificacao(n);
        break;
      }
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO));
      break;
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
            nt_tabuleiro->set_id_rede(notificacao.id_rede());
            int id_tab = GeraIdTabuleiro();
            clientes_.insert(std::make_pair(id_tab, notificacao.id_rede()));
            nt_tabuleiro->mutable_tabuleiro()->set_id_cliente(id_tab);
          } catch (const std::logic_error& e) {
            auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
            ne->set_erro(e.what());
            // Envia para os clientes pendentes tb.
            auto* copia_ne = new ntf::Notificacao(*ne);
            copia_ne->set_clientes_pendentes(true);
            copia_ne->set_id_rede(notificacao.id_rede());
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
          // Endereco deve ter {estatico|dinamico}://nome_arquivo.
          auto pos_separador = notificacao.endereco().find("://");
          if (pos_separador == std::string::npos) {
            throw std::logic_error(
                std::string(":// não encontrado no nome do arquivo: ") + notificacao.endereco());
          }
          std::string nome_arquivo = notificacao.endereco().substr(pos_separador + 3);
          std::string tipo_arquivo = notificacao.endereco().substr(0, pos_separador);
          arq::LeArquivoBinProto(tipo_arquivo == "estatico" ?
              arq::TIPO_TABULEIRO_ESTATICO : arq::TIPO_TABULEIRO,
              nome_arquivo,
              &nt_tabuleiro);
          nt_tabuleiro.mutable_tabuleiro()->set_nome(nome_arquivo);
        } catch (std::logic_error&) {
          auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
          ne->set_erro(std::string("Erro lendo arquivo: ") + notificacao.endereco());
          central_->AdicionaNotificacao(ne);
          return true;
        }
        nt_tabuleiro.set_endereco(nt_tabuleiro.tabuleiro().nome());
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
    case ntf::TN_CRIAR_CENARIO: {
      CriaSubCenarioNotificando(notificacao);
      return true;
    }
    case ntf::TN_REMOVER_CENARIO: {
      RemoveSubCenarioNotificando(notificacao);
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
        n.mutable_entidade()->set_selecionavel_para_jogador(notificacao.entidade().selecionavel_para_jogador());
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
      // Preenche o tabuleiro e envia para ifg tratar.
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
      n->set_modo_mestre(EmModoMestre(true));
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
    case ntf::TN_ALTERAR_MODO_MESTRE_SECUNDARIO: {
      if (EmModoMestre()) {
        std::string id_cliente;
        const auto& it = clientes_.find(notificacao.entidade().id());
        if (it == clientes_.end()) {
          LOG(WARNING) << "Erro, cliente " << notificacao.entidade().id() << " nao encontrado";
          return true;
        }
        auto* n = NovaNotificacao(ntf::TN_ALTERAR_MODO_MESTRE_SECUNDARIO);
        n->set_id_rede(it->second);
        central_->AdicionaNotificacaoRemota(n);
        if (mestres_secundarios_.find(it->first) == mestres_secundarios_.end()) {
          mestres_secundarios_.insert(it->first);
        } else {
          mestres_secundarios_.erase(it->first);
        }
      } else {
        AlternaModoMestreSecundario();
      }
      return true;
    }
    case ntf::TN_GERAR_TERRENO_ALEATORIO: {
      GeraTerrenoAleatorioNotificando();
      return true;
    }
    case ntf::TN_ATUALIZAR_RELEVO_TABULEIRO: {
      DeserializaRelevoCenario(notificacao.tabuleiro());
      if (notificacao.local()) {
        RefrescaTerrenoParaClientes();
      }
      return true;
    }
    default: ;
  }
  return false;
}

void Tabuleiro::RefrescaMovimentosParciais() {
  if (estado_ == ETAB_ENTS_PRESSIONADAS) {
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* e = BuscaEntidade(id);
      if (e == nullptr) {
        continue;
      }
      Posicao pos;
      pos.set_x(e->X());
      pos.set_y(e->Y());
      pos.set_z(e->Z());
      auto* n = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
      n->mutable_entidade()->set_id(id);
      n->mutable_entidade()->mutable_destino()->CopyFrom(pos);
      central_->AdicionaNotificacaoRemota(n);
    }
  } else if (estado_ == ETAB_ENTS_TRANSLACAO_ROTACAO) {
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* e = BuscaEntidade(id);
      if (e == nullptr) {
        continue;
      }
      // Atualiza clientes quando delta passar de algum valor.
      auto* nr = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
      nr->mutable_entidade()->set_id(e->Id());
      nr->mutable_entidade()->set_rotacao_z_graus(e->RotacaoZGraus());
      central_->AdicionaNotificacaoRemota(nr);
      Posicao pos;
      pos.set_x(e->X());
      pos.set_y(e->Y());
      pos.set_z(e->Z());
      auto* nm = ntf::NovaNotificacao(ntf::TN_MOVER_ENTIDADE);
      nm->mutable_entidade()->set_id(e->Id());
      nm->mutable_entidade()->mutable_destino()->CopyFrom(pos);
      central_->AdicionaNotificacaoRemota(nm);
    }
  }
}

void Tabuleiro::RefrescaTerrenoParaClientes() {
  central_->AdicionaNotificacaoRemota(SerializaRelevoCenario());
}

void Tabuleiro::TrataTeclaPressionada(int tecla) {
  return;
#if 0
  switch (tecla) {
    default:
      ;
  }
#endif
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
  } else if (estado_ == ETAB_QUAD_SELECIONADO && ModoClique() == MODO_TERRENO) {
    TrataDeltaTerreno(delta > 0 ? TAMANHO_LADO_QUADRADO : -TAMANHO_LADO_QUADRADO);
  } else {
    if (camera_ == CAMERA_ISOMETRICA) {
      TrataInclinacaoPorDelta(-delta * SENSIBILIDADE_RODA);
    } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
      // TODO
    } else {
      // move o olho no eixo Z de acordo com o eixo Y do movimento
      AtualizaRaioOlho(olho_.raio() - (delta * SENSIBILIDADE_RODA));
    }
  }
}

void Tabuleiro::TrataEscalaPorFator(float fator) {
  if (estado_ == ETAB_QUAD_SELECIONADO && ModoClique() == MODO_TERRENO) {
    // Eh possivel chegar aqui?
    TrataDeltaTerreno(fator * TAMANHO_LADO_QUADRADO);
  } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return;
  } else {
    AtualizaRaioOlho(olho_.raio() / fator);
  }
}

void Tabuleiro::TrataRotacaoPorDelta(float delta_rad) {
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

void Tabuleiro::TrataMovimentoMouse(int x, int y) {
  if (modo_clique_ == MODO_ROTACAO && estado_ != ETAB_ROTACAO) {
    TrataBotaoRotacaoPressionado(x, y);
    return;
  }
  if (x == ultimo_x_ && y == ultimo_y_) {
    // No tablet pode acontecer de gerar estes eventos com mesma coordenadas.
    return;
  }
  switch (estado_) {
    case ETAB_ENTS_TRANSLACAO_ROTACAO: {
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
          return;
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
      AtualizaOlho(0, true  /*forcar*/);
    }
    break;
    case ETAB_ENTS_PRESSIONADAS: {
      // Realiza o movimento da entidade paralelo ao XY na mesma altura do click original.
      camera_presa_ = false;  // temporariamente.
      parametros_desenho_.set_offset_terreno(ultimo_z_3d_);
      parametros_desenho_.set_desenha_entidades(false);
      float nx, ny, nz;
      if (!MousePara3dParaleloZero(x, y, &nx, &ny, &nz)) {
        return;
      }
      float dx = nx - ultimo_x_3d_;
      float dy = ny - ultimo_y_3d_;
      for (unsigned int id : ids_entidades_selecionadas_) {
        auto* entidade_selecionada = BuscaEntidade(id);
        if (entidade_selecionada == nullptr) {
          continue;
        }
        float ex0 = entidade_selecionada->X();
        float ey0 = entidade_selecionada->Y();
        float ex1 = ex0 + dx;
        float ey1 = ey0 + dy;
        float z_antes = entidade_selecionada->Z();
        float z_chao_antes = ZChao(ex0, ey0);
        bool manter_chao = (z_antes - z_chao_antes) < 0.001f;
        float z_depois = 0.0f;
        if (manter_chao) {
          z_depois = ZChao(ex1, ey1);
        } else {
          z_depois = std::max(z_antes, ZChao(ex1, ey1));
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
        }
      }
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
        camera_presa_ = false;  // temporariamente.
        // Como pode ser chamado entre atualizacoes, atualiza a MODELVIEW.
        //gl::ModoMatriz(GL_MODELVIEW);
        gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
        gl::CarregaIdentidade();
        ConfiguraOlhar();
        // Faz picking do tabuleiro sem entidades.
        float nx, ny, nz;
        if (!MousePara3dParaleloZero(x, y, &nx, &ny, &nz)) {
          return;
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
          return;
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
      if (!MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d)) {
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
    PickingControleVirtual(x, y, true  /*alt*/, id);
  } else {
    VLOG(1) << "Picking alternar selecao ignorado.";
  }
  ultimo_x_ = x;
  ultimo_y_ = y;
}

void Tabuleiro::TrataBotaoAlternarIluminacaoMestre() {
  opcoes_.set_iluminacao_mestre_igual_jogadores(!opcoes_.iluminacao_mestre_igual_jogadores());
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
  if ((tipo_objeto != OBJ_TABULEIRO) && (tipo_objeto != OBJ_ENTIDADE)) {
    // invalido.
    return;
  }
  // Primeiro, entidades.
  unsigned int id_entidade_destino = Entidade::IdInvalido;
  Posicao pos_entidade;
  if (tipo_objeto == OBJ_ENTIDADE) {
    VLOG(1) << "Acao em entidade: " << id;
    // Entidade.
    id_entidade_destino = id;
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    pos_entidade.set_x(x3d);
    pos_entidade.set_y(y3d);
    pos_entidade.set_z(z3d);
    // Depois tabuleiro.
    parametros_desenho_.set_desenha_entidades(false);
    BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  }
  Posicao pos_tabuleiro;
  if (tipo_objeto == OBJ_TABULEIRO) {
    float x3d, y3d, z3d;
    MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d);
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
  std::unordered_set<unsigned int> ids_origem;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    ids_origem.insert(id_camera_presa_);
  } else {
    ids_origem = ids_entidades_selecionadas_;
  }
  if (acao_padrao || ids_origem.size() == 0) {
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
    for (auto id_selecionado : ids_origem) {
      Entidade* entidade = BuscaEntidade(id_selecionado);
      if (entidade == nullptr || entidade->Tipo() != TE_ENTIDADE) {
        continue;
      }
      std::string ultima_acao = entidade->Acao(AcoesPadroes()).empty() ?
         ID_ACAO_ATAQUE_CORPO_A_CORPO : entidade->Acao(AcoesPadroes());
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
        if (HaValorListaPontosVida()) {
          delta_pontos_vida = LeValorListaPontosVida(entidade, acao_proto.id());
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
        if (HaValorListaPontosVida() && entidade_destino != nullptr) {
          int delta_pontos_vida = LeValorListaPontosVida(entidade, acao_proto.id());
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
        VLOG(1) << "Acao individual: " << acao_proto.ShortDebugString();
        n.mutable_acao()->CopyFrom(acao_proto);
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
  std::string acao_executada = e->Acao(AcoesPadroes());
  if (acao_executada.empty() || acao_executada == "Sinalização") {
    return;
  }
  e->AdicionaAcaoExecutada(acao_executada);
  if (!EmModoMestre() && id_camera_presa_ == e->Id()) {
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
  if (!entidade_transicao->Proto().transicao_cenario().has_id_cenario()) {
    LOG(INFO) << "Entidade " << id << " nao possui transicao de cenario";
    return;
  }
  int id_cenario = entidade_transicao->Proto().transicao_cenario().id_cenario();
  if (id_cenario < CENARIO_PRINCIPAL) {
    LOG(ERROR) << "Id de cenario deve ser >= CENARIO_PRINCIPAL";
    return;
  }
  if (BuscaSubCenario(id_cenario) == nullptr && !EmModoMestre(true)) {
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
  if (!ids_entidades_selecionadas_.empty()) {
    // Computa a posicao centro das entidades.
    Posicao centro;
    float x_centro = 0, y_centro = 0;
    int n_entidades = 0;
    for (unsigned int id : ids_entidades_selecionadas_) {
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

    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* entidade_movendo = BuscaEntidade(id);
      if (entidade_movendo == nullptr) {
        continue;
      }
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      n->mutable_entidade()->set_id(id);
      n->mutable_entidade()->mutable_pos()->CopyFrom(entidade_movendo->Pos());  // Para desfazer.
      float dx = entidade_movendo->X() - x_centro;
      float dy = entidade_movendo->Y() - y_centro;
      n->mutable_entidade()->mutable_destino()->set_x(pos_destino.x() + dx);
      n->mutable_entidade()->mutable_destino()->set_y(pos_destino.y() + dy);
      n->mutable_entidade()->mutable_destino()->set_z(entidade_transicao->Proto().transicao_cenario().z() + entidade_movendo->Z());
      n->mutable_entidade()->mutable_destino()->set_id_cenario(id_cenario);
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
  // A camera vai para a posicao de transicao ou para a posicao do objeto no outro cenario.
  Posicao pos_olho;
  if (entidade_transicao->Proto().transicao_cenario().has_x()) {
    pos_olho.CopyFrom(entidade_transicao->Proto().transicao_cenario());
  } else {
    pos_olho.CopyFrom(entidade_transicao->Pos());
    pos_olho.set_id_cenario(id_cenario);
  }
  CarregaSubCenario(id_cenario, pos_olho);
}

void Tabuleiro::TrataBotaoReguaPressionadoPosPicking(float x3d, float y3d, float z3d) {
  auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
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
#if !__APPLE__ || !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    gl::Habilita(GL_MULTISAMPLE);
  } else {
    gl::Desabilita(GL_MULTISAMPLE);
  }
#endif
  gl::Desabilita(GL_DITHER);
  // Faz com que AMBIENTE e DIFFUSE sigam as cores.

  // Nao desenha as costas dos poligonos.
  gl::Habilita(GL_CULL_FACE);
  gl::FaceNula(GL_BACK);

  GeraVboCaixaCeu();
  GeraVboRosaDosVentos();

  RegeraVboTabuleiro();
  GeraFramebuffer();
  Entidade::IniciaGl();
  //const GLubyte* ext = glGetString(GL_EXTENSIONS);
  //LOG(INFO) << "Extensoes: " << ext;
  V_ERRO("erro inicializando GL");
}

void Tabuleiro::SelecionaModeloEntidade(const std::string& id_modelo) {
  auto it = mapa_modelos_.find(id_modelo);
  if (it == mapa_modelos_.end()) {
    LOG(ERROR) << "Id de modelo inválido: " << id_modelo;
    return;
  }
  modelo_selecionado_.first = id_modelo;
  modelo_selecionado_.second = it->second.get();
}

const EntidadeProto* Tabuleiro::BuscaModelo(const std::string& id_modelo) const {
  auto it = mapa_modelos_.find(id_modelo);
  if (it == mapa_modelos_.end()) {
    LOG(ERROR) << "Id de modelo inválido: " << id_modelo;
    return nullptr;
  }
  return it->second.get();
}

void Tabuleiro::SelecionaSinalizacao() {
  modo_clique_ = MODO_SINALIZACAO;
}

void Tabuleiro::SelecionaAcao(const std::string& id_acao) {
  auto it = mapa_acoes_.find(id_acao);
  if (it == mapa_acoes_.end()) {
    LOG(ERROR) << "Id de acao inválido: " << id_acao;
    return;
  }

  std::unordered_set<unsigned int> ids;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    ids.insert(id_camera_presa_);
  } else {
    ids = ids_entidades_selecionadas_;
  }
  for (auto id_selecionado : ids) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    entidade->AtualizaAcao(it->first);
  }
}

void Tabuleiro::AlternaDanoAutomatico() {
  modo_dano_automatico_ = !modo_dano_automatico_;
  if (modo_dano_automatico_) {
    modo_clique_ = MODO_ACAO;
  }
}

void Tabuleiro::SelecionaAcaoExecutada(int indice) {
  Entidade* e = EntidadePrimeiraPessoaOuSelecionada();
  if (e == nullptr) {
    LOG(INFO) << "Nao selecionando acao pois ha 0 ou mais de uma entidade selecionada.";
    return;
  }
  std::string id_acao = e->AcaoExecutada(indice, AcoesPadroes());
  if (id_acao.empty()) {
    VLOG(1) << "Selecionando acao padrao pois id eh invalido para a entidade.";
    id_acao = AcaoPadrao(indice).id();
  }
  SelecionaAcao(id_acao);
  modo_clique_ = MODO_ACAO;
}

const std::vector<std::string>& Tabuleiro::AcoesPadroes() const {
  static const std::vector<std::string> acoes_padroes = {
    "Ataque Corpo a Corpo",
    "Ataque a Distância",
    "Feitiço de Toque"
  };
  return acoes_padroes;
}

const AcaoProto& Tabuleiro::AcaoPadrao(int indice) const {
  if (indice < 0 || indice >= static_cast<int>(AcoesPadroes().size())) {
    indice = 0;
  }
  return AcaoDoMapa(AcoesPadroes()[indice]);
}

const AcaoProto& Tabuleiro::AcaoDoMapa(const std::string& id_acao) const {
  const auto it = mapa_acoes_.find(id_acao);
  if (it == mapa_acoes_.end()) {
    return AcaoProto::default_instance();
  }
  return *it->second;
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
    std::string acao(entidade->Acao(AcoesPadroes()));
    if (acao.empty()) {
      acao = ID_ACAO_ATAQUE_CORPO_A_CORPO;
    }
    auto it = std::find(id_acoes_.begin(), id_acoes_.end(), acao);
    if (it == id_acoes_.end()) {
      LOG(ERROR) << "Id de acao inválido: " << entidade->Acao(AcoesPadroes());
      continue;
    }
    ++it;
    if (it == id_acoes_.end()) {
      it = id_acoes_.begin();
    }
    entidade->AtualizaAcao(*it);
  }
  modo_clique_ = MODO_ACAO;
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
    std::string acao(entidade->Acao(AcoesPadroes()));
    if (acao.empty()) {
      acao = ID_ACAO_ATAQUE_CORPO_A_CORPO;
    }
    auto it = std::find(id_acoes_.rbegin(), id_acoes_.rend(), acao);
    if (it == id_acoes_.rend()) {
      LOG(ERROR) << "Id de acao inválido: " << entidade->Acao(AcoesPadroes());
      continue;
    }
    ++it;
    if (it == id_acoes_.rend()) {
      it = id_acoes_.rbegin();
    }
    entidade->AtualizaAcao(*it);
  }
  modo_clique_ = MODO_ACAO;
}

// privadas
void Tabuleiro::DesenhaCena() {
  //if (glGetError() == GL_NO_ERROR) LOG(ERROR) << "ok!";
  V_ERRO("ha algum erro no opengl, investigue");

  gl::InicioCena();
  gl::IniciaNomes();
  V_ERRO("Inicio cena");

  gl::Habilita(GL_DEPTH_TEST);
  V_ERRO("Teste profundidade");
  int bits_limpar = GL_DEPTH_BUFFER_BIT;
  bool desenhar_caixa_ceu = false;
  // A camera isometrica tem problemas com a caixa de ceu, porque ela teria que ser maior que as dimensoes
  // da janela para cobrir o fundo todo.
  if (!parametros_desenho_.desenha_sombra_projetada() && !parametros_desenho_.has_picking_x() &&
      (parametros_desenho_.tipo_visao() != VISAO_ESCURO) && camera_ != CAMERA_ISOMETRICA) {
    desenhar_caixa_ceu = true;
  } else {
    bits_limpar |= GL_COLOR_BUFFER_BIT;
    if (parametros_desenho_.tipo_visao() == VISAO_ESCURO) {
      gl::CorLimpeza(0.0f, 0.0f, 0.0f, 1.0f);
    } else {
      gl::CorLimpeza(proto_corrente_->luz_ambiente().r(),
                     proto_corrente_->luz_ambiente().g(),
                     proto_corrente_->luz_ambiente().b(),
                     proto_corrente_->luz_ambiente().a());
    }
  }
  gl::Limpa(bits_limpar);
  V_ERRO("Limpa");

  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  V_ERRO("desabilitando luzes");

  gl::MudarModoMatriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  ConfiguraOlhar();

  if (MapeamentoSombras() && parametros_desenho_.desenha_sombras() && !parametros_desenho_.desenha_sombra_projetada()) {
    parametros_desenho_.set_desenha_sombra_projetada(true);
    gl::MudarModoMatriz(gl::MATRIZ_SOMBRA);
    ConfiguraOlhar();
    parametros_desenho_.set_desenha_sombra_projetada(false);
    gl::MudarModoMatriz(GL_MODELVIEW);
  }

  parametros_desenho_.mutable_pos_olho()->CopyFrom(olho_.pos());
  // Verifica o angulo em relacao ao tabuleiro para decidir se as texturas ficarao viradas para cima.
  if (camera_ == CAMERA_ISOMETRICA ||
      (camera_ != CAMERA_PRIMEIRA_PESSOA && (olho_.altura() > (2 * olho_.raio())))) {
    parametros_desenho_.set_desenha_texturas_para_cima(true);
  } else {
    parametros_desenho_.set_desenha_texturas_para_cima(false);
  }
  V_ERRO("configurando olhar");

  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
    DesenhaLuzes();
  } else {
    gl::Desabilita(GL_LIGHTING);
    gl::Desabilita(GL_FOG);
  }
  V_ERRO("desenhando luzes");

  // Aqui podem ser desenhados objetos normalmente. Caso contrario, a caixa do ceu vai ferrar tudo.
  // desenha tabuleiro do sul para o norte.
  {
    gl::TipoEscopo nomes_tabuleiro(OBJ_TABULEIRO);
    DesenhaTabuleiro();
    if (parametros_desenho_.desenha_grade() &&
        (proto_corrente_->desenha_grade() || (!VisaoMestre() && proto_corrente_->textura_mestre_apenas()))) {
      DesenhaGrade();
    }
    DesenhaQuadradoSelecionado();
  }
  V_ERRO("desenhando tabuleiro");

  if (opcoes_.desenha_olho()) {
    DesenhaOlho();
  }
  V_ERRO("desenhando olho");

#if 0
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
  V_ERRO("desenhando pontos rolagem");

  if (parametros_desenho_.desenha_entidades()) {
    gl::TipoEscopo nomes(OBJ_ENTIDADE);
    // Desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
    // na hora do picking.
    DesenhaEntidades();
  }
  V_ERRO("desenhando entidades");

  if (desenhar_caixa_ceu) {
    DesenhaCaixaCeu();
  }
  V_ERRO("desenhando caixa do ceu");

  if (parametros_desenho_.desenha_acoes()) {
    DesenhaAcoes();
  }
  V_ERRO("desenhando acoes");

  // Sombras.
  if (!MapeamentoSombras() && parametros_desenho_.desenha_sombras() &&
      proto_corrente_->luz_direcional().inclinacao_graus() > 5.0 &&
      proto_corrente_->luz_direcional().inclinacao_graus() < 180.0f) {
    bool desenha_texturas = parametros_desenho_.desenha_texturas();
    parametros_desenho_.set_desenha_texturas(false);
    DesenhaSombras();
    parametros_desenho_.set_desenha_texturas(desenha_texturas);
  }
  V_ERRO("desenhando sombras");

  if (estado_ == ETAB_ENTS_PRESSIONADAS && parametros_desenho_.desenha_rastro_movimento() && !rastros_movimento_.empty()) {
    LigaStencil();
    DesenhaRastros();
    float tam_x = proto_.largura() * TAMANHO_LADO_QUADRADO;
    float tam_y = proto_.altura() * TAMANHO_LADO_QUADRADO;
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
    DesenhaStencil3d(tam_x, tam_y, COR_AZUL_ALFA);
  }
  V_ERRO("desenhando stencil sombras");

  if (estado_ == ETAB_DESENHANDO && parametros_desenho_.desenha_forma_selecionada()) {
    DesenhaFormaSelecionada();
  }

  if (parametros_desenho_.desenha_quadrado_selecao() && estado_ == ETAB_SELECIONANDO_ENTIDADES) {
    gl::DesligaEscritaProfundidadeEscopo desliga_teste_escopo;
    gl::DesabilitaEscopo cull_escopo(GL_CULL_FACE);
    gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
    gl::DesvioProfundidade(OFFSET_RASTRO_ESCALA_DZ, OFFSET_RASTRO_ESCALA_R);
    MudaCorAlfa(COR_AZUL_ALFA);
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
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
      gl::HabilitaEscopo blend_escopo(GL_BLEND);
      gl::CorMistura(1.0f, 1.0f, 1.0f, parametros_desenho_.alfa_translucidos());
      DesenhaEntidadesTranslucidas();
      parametros_desenho_.clear_alfa_translucidos();
      DesenhaAuras();
      gl::CorMistura(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
      gl::TipoEscopo nomes(OBJ_ENTIDADE);
      // Desenha os translucidos de forma solida para picking.
      DesenhaEntidadesTranslucidas();
    }
  }
  V_ERRO("desenhando entidades alfa");

  if (MapeamentoSombras() && parametros_desenho_.desenha_sombra_projetada()) {
    return;
  }

  //-------------
  // DESENHOS 2D.
  //-------------
#if 0 && DEBUG
  if (MapeamentoSombras() && !parametros_desenho_.has_picking_x()) {
    gl::MatrizEscopo salva_matriz_proj(GL_PROJECTION);
    gl::CarregaIdentidade();
    // Eixo com origem embaixo esquerda.
    gl::Ortogonal(0, largura_, 0, altura_, -1.0f, 1.0f);
    gl::MatrizEscopo salva_matriz_mv(GL_MODELVIEW);
    gl::CarregaIdentidade(false);
    gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);

    MudaCor(COR_BRANCA);
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, textura_framebuffer_);
    gl::Retangulo(10, altura_ - 150, 110, altura_ - 50);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  }
#endif

  gl::Desabilita(GL_FOG);
  gl::UsaShader(gl::TSH_SIMPLES);

  if (parametros_desenho_.desenha_rosa_dos_ventos() && opcoes_.desenha_rosa_dos_ventos()) {
    DesenhaRosaDosVentos();
  }
  V_ERRO("desenhando rosa dos ventos");

  if (parametros_desenho_.desenha_lista_jogadores() && EmModoMestre()) {
    DesenhaListaJogadores();
  }
  V_ERRO("desenhando lista de objetos");

  if (parametros_desenho_.desenha_lista_objetos() && EmModoMestre()) {
    DesenhaListaObjetos();
  }
  V_ERRO("desenhando lista de objetos");

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }
  V_ERRO("desenhando lista pontos de vida");

  if (parametros_desenho_.desenha_id_acao()) {
    DesenhaIdAcaoEntidade();
  }
  V_ERRO("desenhando id_acao");

  if (parametros_desenho_.desenha_coordenadas()) {
    DesenhaCoordenadas();
  }
  V_ERRO("desenhando coordenadas");

  if (parametros_desenho_.desenha_controle_virtual() && opcoes_.desenha_controle_virtual()) {
    // Controle na quarta posicao da pilha.
    gl::TipoEscopo controle(OBJ_CONTROLE_VIRTUAL);
    DesenhaControleVirtual();
  }
  V_ERRO("desenhando controle virtual");

  if (gui_ != nullptr) {
    gl::TipoEscopo controle(OBJ_CONTROLE_VIRTUAL);
    gui_->Desenha(&parametros_desenho_);
  }
  V_ERRO("desenhando interface grafica");

  glFlush();
}

void Tabuleiro::DesenhaOlho() {
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  MudaCor(COR_AMARELA);
  gl::MatrizEscopo salva_matriz;
  gl::Translada(olho_.alvo().x(), olho_.alvo().y(), olho_.alvo().z());
  gl::EsferaSolida(1.0f, 5, 5);
}

void Tabuleiro::GeraVboRosaDosVentos() {
  const static float kRaioRosa = 20.0f;
  const static float kLarguraSeta = 5.0f;
  const static float kTamanhoSeta = kRaioRosa * 0.8f;

  gl::VboNaoGravado vbo_disco = std::move(gl::VboDisco(kRaioRosa, 8  /*faces*/));
  vbo_disco.AtribuiCor(1.0f, 1.0f, 1.0f, 1.0f);
  gl::VboNaoGravado vbo_seta("seta");
  {
    // Deixa espaco para o N.
    // Desenha seta.
    unsigned short indices_seta[] = { 0, 1, 2 };
    vbo_seta.AtribuiIndices(indices_seta, 3);
    float coordenadas[] = {
      -kLarguraSeta, 0.0f, 0.1f,
      kLarguraSeta, 0.0f, 0.1f,
      0.0f, kTamanhoSeta, 0.1f,
    };
    vbo_seta.AtribuiCoordenadas(3, coordenadas, 9);
    vbo_seta.AtribuiCor(1.0f, 0.0f, 0.0f, 1.0f);
  }

  gl::VboNaoGravado vbo_n("n");
  unsigned short indices_n[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  float coordenadas_n[] = {
    // Primeira perna N.
    -4.0f, 0.0f + kRaioRosa, 0.1f,
    -1.0f, 0.0f + kRaioRosa, 0.1f,
    -4.0f, 13.0f + kRaioRosa , 0.1f,
    // Segunda.
    -4.0f, 13.0f + kRaioRosa , 0.1f,
    -4.0f, 8.0f + kRaioRosa , 0.1f,
    4.0f, 0.0f + kRaioRosa , 0.1f,
    // Terceira.
    4.0f, 0.0f + kRaioRosa , 0.1f,
    4.0f, 13.0f + kRaioRosa , 0.1f,
    1.0f, 13.0f + kRaioRosa , 0.1f,
  };
  vbo_n.AtribuiIndices(indices_n, 9);
  vbo_n.AtribuiCoordenadas(3, coordenadas_n, 27);
  vbo_n.AtribuiCor(1.0f, 0.0f, 0.0f, 1.0f);

  vbo_disco.Concatena(vbo_seta);
  vbo_disco.Concatena(vbo_n);

  vbo_rosa_.Grava(vbo_disco);
}

void Tabuleiro::GeraVboCaixaCeu() {
  // O cubo tem que ser maior que a distancia do plano de corte minimo.
  gl::VboNaoGravado vbo = std::move(gl::VboCuboSolido(DISTANCIA_PLANO_CORTE_PROXIMO * 4.0));
  // Valores de referencia:
  // imagem 4x3.
  // x = 0.0f, 0.25f, 0.50f, 0.75f, 1.0f
  // y = 1.0f, 0.66f, 0.33f, 0f
  const float texturas[6 * 4 * 2] = {
    // sul: 0-3 (linha meio, coluna 0),--,+-,++,-+
    0.25f, 0.6666f,
    0.0f,  0.6666f,
    0.0f,  0.3333f,
    0.25f, 0.3333f,
    // norte: 4-7 (linha meio, coluna 2),--,-+,++,+-
    0.50f, 0.6666f,
    0.50f, 0.3333f,
    0.75f, 0.3333f,
    0.75f, 0.6666f,
    // oeste: 8-11 (linha meio, coluna 1),--,-+,++,+-
    0.25f, 0.6666f,
    0.25f, 0.3333f,
    0.50f, 0.3333f,
    0.50f, 0.6666f,
    // leste: 12-15 (linha meio, coluna 3):--,+-,++,-+
    1.0f,  0.6666f,
    0.75f, 0.6666f,
    0.75f, 0.3333f,
    1.0f,  0.3333f,
    // cima: 16-19 (linha de cima, coluna 1):--,+-,++,-+
    0.25f, 0.3333f,
    0.25f, 0.0f,
    0.50f, 0.0,
    0.50f, 0.3333f,
    // baixo: 20-23 (linha de baixo, coluna 1):--,-+,++,+-
    0.25f, 0.6666f,
    0.50f, 0.6666f,
    0.50f, 1.0f,
    0.25f, 1.0f,
  };
  vbo.AtribuiTexturas(texturas);
  vbo_caixa_ceu_.Grava(vbo);
}

void Tabuleiro::RegeraVboTabuleiro() {
  V_ERRO("RegeraVboTabuleiro inicio");
  std::vector<float> coordenadas_tabuleiro;
  std::vector<float> coordenadas_textura;
  std::vector<float> coordenadas_normais;
  std::vector<unsigned short> indices_tabuleiro;
  if (!proto_corrente_->ponto_terreno().empty() &&
      proto_corrente_->ponto_terreno_size() != (TamanhoX() + 1) * (TamanhoY() + 1)) {
    LOG(ERROR) << "Tamanho de terreno invalido: corrente " << proto_corrente_->ponto_terreno_size()
               << ", rhs: " << ((TamanhoX() + 1) * (TamanhoY() + 1));
    return;
  }
  LOG(INFO) << "Regerando vbo tabuleiro, pontos: " << proto_corrente_->ponto_terreno_size();
  Terreno terreno(TamanhoX(), TamanhoY(), proto_corrente_->ladrilho(),
                  Wrapper<RepeatedField<double>>(proto_corrente_->ponto_terreno()));
  terreno.Preenche(&indices_tabuleiro,
                   &coordenadas_tabuleiro,
                   &coordenadas_normais,
                   &coordenadas_textura);
  gl::VboNaoGravado tabuleiro_nao_gravado("tabuleiro_nao_gravado");
  tabuleiro_nao_gravado.AtribuiIndices(indices_tabuleiro.data(), indices_tabuleiro.size());
  tabuleiro_nao_gravado.AtribuiCoordenadas(3, coordenadas_tabuleiro.data(), coordenadas_tabuleiro.size());
  tabuleiro_nao_gravado.AtribuiTexturas(coordenadas_textura.data());
  tabuleiro_nao_gravado.AtribuiNormais(coordenadas_normais.data());
  V_ERRO("RegeraVboTabuleiro antes gravar");
  vbo_tabuleiro_.Grava(tabuleiro_nao_gravado);
  //vbo_tabuleiro_normais_ = std::move(tabuleiro_nao_gravado.ExtraiVboNormais());
  V_ERRO("RegeraVboTabuleiro depois gravar");

  // Regera a grade.
  std::vector<float> coordenadas_grade;
  std::vector<unsigned short> indices_grade;
  int indice = 0;
  float deslocamento_x = -TamanhoX() * TAMANHO_LADO_QUADRADO_2;
  float deslocamento_y = -TamanhoY() * TAMANHO_LADO_QUADRADO_2;
  // Linhas verticais (S-N).
  {
    for (int xcorrente = 0; xcorrente <= TamanhoX(); ++xcorrente) {
      float x_inicial = (xcorrente * TAMANHO_LADO_QUADRADO) - EXPESSURA_LINHA_2 + deslocamento_x;
      float x_final = x_inicial + EXPESSURA_LINHA;
      for (int ycorrente = 0; ycorrente < TamanhoY(); ++ycorrente) {
        float y_inicial = ycorrente * TAMANHO_LADO_QUADRADO + deslocamento_y;
        float y_final = y_inicial + TAMANHO_LADO_QUADRADO;
        coordenadas_grade.push_back(x_inicial);
        coordenadas_grade.push_back(y_inicial);
        coordenadas_grade.push_back(AlturaPonto(xcorrente, ycorrente));
        coordenadas_grade.push_back(x_final);
        coordenadas_grade.push_back(y_inicial);
        coordenadas_grade.push_back(AlturaPonto(xcorrente, ycorrente));
        coordenadas_grade.push_back(x_final);
        coordenadas_grade.push_back(y_final);
        coordenadas_grade.push_back(AlturaPonto(xcorrente, ycorrente + 1));
        coordenadas_grade.push_back(x_inicial);
        coordenadas_grade.push_back(y_final);
        coordenadas_grade.push_back(AlturaPonto(xcorrente, ycorrente + 1));
        indices_grade.push_back(indice);
        indices_grade.push_back(indice + 1);
        indices_grade.push_back(indice + 2);
        indices_grade.push_back(indice);
        indices_grade.push_back(indice + 2);
        indices_grade.push_back(indice + 3);
        indice += 4;
      }
    }
  }
  {
    for (int ycorrente = 0; ycorrente <= TamanhoY(); ++ycorrente) {
      float y_inicial = (ycorrente * TAMANHO_LADO_QUADRADO) - EXPESSURA_LINHA_2 + deslocamento_y;
      float y_final = y_inicial + EXPESSURA_LINHA;
      for (int xcorrente = 0; xcorrente < TamanhoX(); ++xcorrente) {
        float x_inicial = xcorrente * TAMANHO_LADO_QUADRADO + deslocamento_x;
        float x_final = x_inicial + TAMANHO_LADO_QUADRADO;
        coordenadas_grade.push_back(x_inicial);
        coordenadas_grade.push_back(y_inicial);
        coordenadas_grade.push_back(AlturaPonto(xcorrente, ycorrente));
        coordenadas_grade.push_back(x_final);
        coordenadas_grade.push_back(y_inicial);
        coordenadas_grade.push_back(AlturaPonto(xcorrente + 1, ycorrente));
        coordenadas_grade.push_back(x_final);
        coordenadas_grade.push_back(y_final);
        coordenadas_grade.push_back(AlturaPonto(xcorrente + 1, ycorrente));
        coordenadas_grade.push_back(x_inicial);
        coordenadas_grade.push_back(y_final);
        coordenadas_grade.push_back(AlturaPonto(xcorrente , ycorrente));
        indices_grade.push_back(indice);
        indices_grade.push_back(indice + 1);
        indices_grade.push_back(indice + 2);
        indices_grade.push_back(indice);
        indices_grade.push_back(indice + 2);
        indices_grade.push_back(indice + 3);
        indice += 4;
      }
    }
  }
  gl::VboNaoGravado grade_nao_gravada("grade_nao_gravada");
  grade_nao_gravada.AtribuiIndices(indices_grade.data(), indices_grade.size());
  grade_nao_gravada.AtribuiCoordenadas(3, coordenadas_grade.data(), coordenadas_grade.size());
  vbo_grade_.Grava(grade_nao_gravada);
  V_ERRO("RegeraVboTabuleiro fim");
}

void Tabuleiro::GeraFramebuffer() {
  if (!MapeamentoSombras()) {
    return;
  }
  GLint original;
  gl::Le(GL_FRAMEBUFFER_BINDING, &original);
  LOG(INFO) << "gerando framebuffer";
  gl::GeraFramebuffers(1, &framebuffer_);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  V_ERRO("LigacaoComFramebuffer");
  gl::GeraTexturas(1, &textura_framebuffer_);
  V_ERRO("GeraTexturas");
  gl::LigacaoComTextura(GL_TEXTURE_2D, textura_framebuffer_);
  V_ERRO("LigacaoComTextura");
#if USAR_MAPEAMENTO_SOMBRAS_OPENGLES
  if (gl::TemExtensao("GL_OES_depth_texture")) {
    usar_sampler_sombras_ = true;
    gl::ImagemTextura2d(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
  } else {
    usar_sampler_sombras_ = false;
    gl::ImagemTextura2d(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }
#else
  gl::ImagemTextura2d(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  // Indica que vamos comparar o valor de referencia passado contra o valor armazenado no mapa de textura.
  // Nas versoes mais nova, usa-se ref.
  //gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
#endif

  V_ERRO("ImagemTextura2d");
  //gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  V_ERRO("ParametroTextura");
#if USAR_MAPEAMENTO_SOMBRAS_OPENGLES
  if (usar_sampler_sombras_) {
    gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textura_framebuffer_, 0);
  } else {
    gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textura_framebuffer_, 0);
    gl::GeraRenderbuffers(1, &renderbuffer_framebuffer_);
    gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, renderbuffer_framebuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 1024, 1024);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer_framebuffer_);
  }
#else
  gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textura_framebuffer_, 0);
#endif
  V_ERRO("TexturaFramebuffer");

  // No OSX o framebuffer fica incompleto se nao desabilitar o buffer de desenho e leitura para esse framebuffer.
#if __APPLE__ && !USAR_OPENGL_ES
  gl::BufferDesenho(GL_NONE);
  gl::BufferLeitura(GL_NONE);
#endif
  auto ret = gl::VerificaFramebuffer(GL_FRAMEBUFFER);
  if (ret != GL_FRAMEBUFFER_COMPLETE) {
    LOG(ERROR) << "Framebuffer incompleto: " << ret;
  } else {
    LOG(INFO) << "Framebuffer completo";
  }

  // Volta estado normal.
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, 0);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
  V_ERRO("Fim da Geracao de framebuffer");
  LOG(INFO) << "framebuffer gerado";
}

void Tabuleiro::GeraTerrenoAleatorioNotificando() {
  if (!EmModoMestre(true  /*secundario*/)) {
    LOG(INFO) << "Apenas mestre pode gerar terreno.";
    return;
  }
  std::vector<double> pontos;
  try {
    pontos = Terreno::CriaPontosAleatorios(TamanhoX(), TamanhoY());
    if (pontos.size() != size_t((TamanhoX() + 1) * (TamanhoY() + 1))) {
      throw std::logic_error("Geracao de terreno invalida");
    }
  } catch (const std::logic_error& e) {
    LOG(ERROR) << "Geracao de terreno invalida: " << e.what();
    return;
  }
  proto_corrente_->mutable_ponto_terreno()->Resize((TamanhoX() + 1) * (TamanhoY() + 1), 0);
  std::copy(pontos.begin(), pontos.end(), proto_corrente_->mutable_ponto_terreno()->begin());
  RegeraVboTabuleiro();
  RefrescaTerrenoParaClientes();
}

namespace {

void AtualizaAlturaQuadrado(std::function<float(const RepeatedField<double>&, int)> funcao,
                            int quad_x, int quad_y, int num_quad_x, RepeatedField<double>* pontos) {
  for (int x = quad_x; x <= quad_x + 1; ++x) {
    for (int y = quad_y; y <= quad_y + 1; ++y) {
      int indice = Terreno::IndicePontoTabuleiro(x, y, num_quad_x);
      if (indice < 0 || indice >= pontos->size()) {
        LOG(ERROR) << "indice invalido: " << indice;
      }
      pontos->Set(indice, funcao(*pontos, indice));
    }
  }
}

}  // namespace

void Tabuleiro::TrataDeltaTerreno(float delta) {
  if (estado_ != ETAB_QUAD_SELECIONADO) {
    return;
  }
  ntf::Notificacao n_desfazer;
  n_desfazer.set_tipo(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
  {
    auto* cenario_antes = n_desfazer.mutable_tabuleiro_antes();
    cenario_antes->set_id_cenario(proto_corrente_->id_cenario());
    *cenario_antes->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  }

  if (proto_corrente_->ponto_terreno_size() != ((TamanhoX() + 1) * (TamanhoY() + 1))) {
    proto_corrente_->mutable_ponto_terreno()->Resize((TamanhoX() + 1) * (TamanhoY() + 1), 0.0f);
  }
  int quad_x = quadrado_selecionado_ % TamanhoX();
  int quad_y = quadrado_selecionado_ / TamanhoX();
  AtualizaAlturaQuadrado(
      [delta] (const RepeatedField<double>& pontos, int indice) { return pontos.Get(indice) + delta; },
      quad_x, quad_y, TamanhoX(), proto_corrente_->mutable_ponto_terreno());
  RegeraVboTabuleiro();
  RefrescaTerrenoParaClientes();
  {
    auto* cenario_depois = n_desfazer.mutable_tabuleiro();
    cenario_depois->set_id_cenario(proto_corrente_->id_cenario());
    *cenario_depois->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  }
  AdicionaNotificacaoListaEventos(n_desfazer);
}

void Tabuleiro::TrataNivelamentoTerreno(int x, int y) {
  parametros_desenho_.set_desenha_entidades(false);
  parametros_desenho_.set_offset_terreno(primeiro_z_3d_);
  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  if (tipo_objeto != OBJ_TABULEIRO) {
    return;
  }
  float x3d, y3d, z3d;
  if (!MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d)) {
    LOG(INFO) << "TrataNivelamentoTerreno: MousePara3dParaleloZero retornou false";
    return;
  }
  id = IdQuadrado(x3d, y3d);
  if (id == static_cast<unsigned int>(-1)) {
    LOG(INFO) << "TrataNivelamentoTerreno: id quadrado invalido";
    return;
  }
  if (proto_corrente_->ponto_terreno_size() != ((TamanhoX() + 1) * (TamanhoY() + 1))) {
    proto_corrente_->mutable_ponto_terreno()->Resize((TamanhoX() + 1) * (TamanhoY() + 1), 0.0f);
  }
  int quad_x = id % TamanhoX();
  int quad_y = id / TamanhoX();
  float pz3d = primeiro_z_3d_;
  AtualizaAlturaQuadrado(
      [pz3d] (const RepeatedField<double>&, int) { return pz3d; },
      quad_x, quad_y, TamanhoX(), proto_corrente_->mutable_ponto_terreno());
  RegeraVboTabuleiro();
}

void Tabuleiro::DesenhaTabuleiro() {
  V_ERRO("desenhando tabuleiro inicio");
  gl::CarregaNome(0);
  V_ERRO("desenhando tabuleiro nome");
  gl::MatrizEscopo salva_matriz;
  float deltaX = -TamanhoX() * TAMANHO_LADO_QUADRADO;
  float deltaY = -TamanhoY() * TAMANHO_LADO_QUADRADO;
  //gl::Normal(0, 0, 1.0f);
  V_ERRO("desenhando tabuleiro normal");
  if (parametros_desenho_.has_offset_terreno()) {
    // Para mover entidades acima do plano do olho.
    gl::Desabilita(GL_CULL_FACE);
  } else {
    gl::Habilita(GL_CULL_FACE);
  }
  V_ERRO("desenhando tabuleiro dentro");

  // Desenha o chao mais pro fundo.
  gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(OFFSET_TERRENO_ESCALA_DZ, OFFSET_TERRENO_ESCALA_R);
  V_ERRO("GL_POLYGON_OFFSET_FILL e desvio");
  MudaCor(proto_corrente_->has_info_textura() ? COR_BRANCA : COR_CINZA_CLARO);
  gl::Translada(deltaX / 2.0f,
                deltaY / 2.0f,
                parametros_desenho_.offset_terreno(),
                false);
  GLuint id_textura = parametros_desenho_.desenha_texturas() &&
                      proto_corrente_->has_info_textura() &&
                      (!proto_corrente_->textura_mestre_apenas() || VisaoMestre()) ?
      texturas_->Textura(proto_corrente_->info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
  } else if (parametros_desenho_.desenha_texturas() && proto_corrente_->has_info_textura()) {
    // Eh possivel que a textura esteja carregando ainda.
    LOG_EVERY_N(WARNING, 100) << "TEXTURA INVALIDA: " << proto_corrente_->info_textura().id();
  } else {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  }
  V_ERRO("textura");

  gl::DesenhaVbo(vbo_tabuleiro_, GL_TRIANGLES);
  //gl::DesenhaVbo(vbo_tabuleiro_normais_, GL_LINES);
  V_ERRO("vbo_tabuleiro_");
  // Se a face nula foi desativada, reativa.
  gl::Habilita(GL_CULL_FACE);
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::Desabilita(GL_TEXTURE_2D);
  V_ERRO("depois vbo_tabuleiro_");
}

void Tabuleiro::DesenhaQuadradoSelecionado() {
  // Desenha quadrado selecionado.
  gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
  if (quadrado_selecionado_ != -1 && proto_corrente_->desenha_grade() && parametros_desenho_.desenha_grade()) {
    gl::DesvioProfundidade(OFFSET_RASTRO_ESCALA_DZ, OFFSET_RASTRO_ESCALA_R);
    gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
    gl::HabilitaEscopo blend_escopo(GL_BLEND);
    const float cor[4] = { 0.0f, 0.0f, 0.0f, 0.3f };
    MudaCorAlfa(cor);
    int x_quad = quadrado_selecionado_ % TamanhoX();
    int y_quad = quadrado_selecionado_ / TamanhoX();
    struct Ponto {
      float x, y, z;
    };
    Ponto pontos[4];
    CoordenadaSwQuadrado(x_quad,     y_quad, &pontos[0].x, &pontos[0].y, &pontos[0].z);
    CoordenadaSwQuadrado(x_quad + 1, y_quad, &pontos[1].x, &pontos[1].y, &pontos[1].z);
    CoordenadaSwQuadrado(x_quad + 1, y_quad + 1, &pontos[2].x, &pontos[2].y, &pontos[2].z);
    CoordenadaSwQuadrado(x_quad,     y_quad + 1, &pontos[3].x, &pontos[3].y, &pontos[3].z);
    GLfloat coordenadas[12];
    for (int i = 0; i < 4; ++i) {
      coordenadas[i * 3]     = pontos[i].x;
      coordenadas[i * 3 + 1] = pontos[i].y;
      coordenadas[i * 3 + 2] = pontos[i].z;
    }
    unsigned short indices[6];
    if (Terreno::Inverte(x_quad, y_quad)) {
      indices[0] = 3;
      indices[1] = 1;
      indices[2] = 2;
      indices[3] = 3;
      indices[4] = 0;
      indices[5] = 1;
    } else {
      indices[0] = 0;
      indices[1] = 1;
      indices[2] = 2;
      indices[3] = 0;
      indices[4] = 2;
      indices[5] = 3;
    }
    gl::VboNaoGravado vbo;
    vbo.AtribuiCoordenadas(3, coordenadas, 12);
    vbo.AtribuiIndices(indices, 6);
    vbo.Nomeia("triangulo_quadrado_selecionado");
    gl::DesenhaVbo(vbo);
  }
}

void Tabuleiro::DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f, bool sombra) {
  //LOG(INFO) << "LOOP";
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second.get();
    if (entidade == nullptr) {
      LOG(ERROR) << "Entidade nao existe.";
      continue;
    }
    if (entidade->Pos().id_cenario() != cenario_corrente_) {
      continue;
    }
    if (!entidade->Proto().faz_sombra() && (parametros_desenho_.desenha_sombra_projetada() || sombra)) {
      continue;
    }
    // Nao desenha a propria entidade na primeira pessoa, apenas sua sombra.
    if (camera_ == CAMERA_PRIMEIRA_PESSOA &&
        !(parametros_desenho_.desenha_sombra_projetada() || sombra) &&
        entidade->Id() == id_camera_presa_) {
      continue;
    }
    // Nao roda disco se estiver arrastando.
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS &&
                                                 EntidadeEstaSelecionada(entidade->Id()));
    bool detalhar_tudo = detalhar_todas_entidades_ || modo_clique_ == MODO_ACAO;
    bool entidade_detalhada = parametros_desenho_.desenha_detalhes() &&
                              tipo_entidade_detalhada_ == OBJ_ENTIDADE &&
                              entidade->Id() == id_entidade_detalhada_;
    parametros_desenho_.set_desenha_barra_vida(entidade_detalhada || detalhar_tudo);
    // Rotulos apenas individualmente.
    parametros_desenho_.set_desenha_rotulo(entidade_detalhada);
    parametros_desenho_.set_desenha_rotulo_especial(
        entidade_detalhada && (VisaoMestre() || entidade->SelecionavelParaJogador()));
    parametros_desenho_.set_desenha_eventos_entidades(VisaoMestre() || entidade->SelecionavelParaJogador());
    //LOG(INFO) << "Desenhando: " << entidade->Id();
    f(entidade, &parametros_desenho_);
  }
  parametros_desenho_.set_entidade_selecionada(false);
  parametros_desenho_.set_desenha_barra_vida(false);
}

void Tabuleiro::DesenhaRastros() {
  gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(OFFSET_RASTRO_ESCALA_DZ, OFFSET_RASTRO_ESCALA_R);
  for (const auto& it : rastros_movimento_) {
    auto* e = BuscaEntidade(it.first);
    if (e == nullptr || e->Tipo() != TE_ENTIDADE) {
      continue;
    }
    // Copia vetor de pontos e adiciona posicao corrente da entidade.
    std::vector<Posicao> pontos(it.second);
    Posicao pos;
    pos.set_x(e->X());
    pos.set_y(e->Y());
    pos.set_z(0.0f);
    pontos.push_back(pos);
    // Rastro pouco menor que quadrado.
    DesenhaLinha3d(pontos, e->MultiplicadorTamanho() * TAMANHO_LADO_QUADRADO * 0.8);
  }
}

void Tabuleiro::DesenhaAuras() {
  if (parametros_desenho_.desenha_aura()) {
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      Entidade* entidade = it->second.get();
      if (entidade->IdCenario() != proto_corrente_->id_cenario()) {
        continue;
      }
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
  gl::CarregaIdentidade(false);
  gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
  gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
  const float kRaioRosa = 20.0f;
  // Deixa espaco para o N.
  gl::Translada(largura_ - kRaioRosa - 15.0f, kRaioRosa + 15.0f, 0.0f, false);
  // Roda pra posicao correta.
  Posicao direcao;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &direcao);
  // A diferenca eh em relacao ao leste e o norte esta a 90 graus. Quanto maior a diferenca, mais proximo do norte (ate 90.0f).
  float diferenca_graus = 90.0f - VetorParaRotacaoGraus(direcao.x(), direcao.y());
  gl::Roda(diferenca_graus, 0.0f, 0.0f, 1.0f, false);
  gl::DesenhaVbo(vbo_rosa_, GL_TRIANGLES);
}

void Tabuleiro::DesenhaPontosRolagem() {
  // 4 pontos.
  MudaCor(COR_PRETA);
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);
  float translacao_x = ((TamanhoX() / 2) + 1) * TAMANHO_LADO_QUADRADO +
                       ((TamanhoX() % 2 != 0) ? TAMANHO_LADO_QUADRADO_2 : 0);
  float translacao_y = ((TamanhoY() / 2) + 1) * TAMANHO_LADO_QUADRADO +
                       ((TamanhoY() % 2 != 0) ? TAMANHO_LADO_QUADRADO_2 : 0);
  int id = 0;
  V_ERRO("AQUI");
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
  switch (fd) {
    case TF_CILINDRO:
    case TF_CIRCULO:
    case TF_CONE:
    case TF_CUBO:
    case TF_ESFERA:
    case TF_LIVRE:
    case TF_PIRAMIDE:
    case TF_RETANGULO:
    case TF_TRIANGULO:
      break;
    default:
      LOG(ERROR) << "Forma de desenho invalida: " << fd;
  }
  forma_selecionada_ = fd;
}

void Tabuleiro::AlteraCorEntidadesSelecionadasNotificando(const Cor& cor) {
  ntf::Notificacao grupo_notificacao;
  grupo_notificacao.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (int id : ids_entidades_selecionadas_) {
    Entidade* e = BuscaEntidade(id);
    if (e == nullptr) {
      continue;
    }
    auto* ne = grupo_notificacao.add_notificacao();
    ne->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    auto* entidade_antes = ne->mutable_entidade_antes();
    entidade_antes->set_id(e->Id());
    entidade_antes->mutable_cor()->CopyFrom(e->CorDesenho());
    auto* entidade_depois = ne->mutable_entidade();
    entidade_depois->set_id(e->Id());
    entidade_depois->mutable_cor()->CopyFrom(cor);
  }
  TrataNotificacao(grupo_notificacao);
  if (grupo_notificacao.notificacao_size() > 0) {
    // Para desfazer;
    AdicionaNotificacaoListaEventos(grupo_notificacao);
  }
}

void Tabuleiro::AlteraTexturaEntidadesSelecionadasNotificando(const std::string& id_textura) {
  ntf::Notificacao grupo_notificacao;
  grupo_notificacao.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (int id : ids_entidades_selecionadas_) {
    Entidade* e = BuscaEntidade(id);
    if (e == nullptr) {
      continue;
    }
    auto* ne = grupo_notificacao.add_notificacao();
    ne->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    auto* entidade_antes = ne->mutable_entidade_antes();
    entidade_antes->set_id(e->Id());
    entidade_antes->mutable_info_textura()->CopyFrom(e->Proto().info_textura());
    auto* entidade_depois = ne->mutable_entidade();
    entidade_depois->set_id(e->Id());
    entidade_depois->mutable_info_textura()->set_id(id_textura);
  }
  TrataNotificacao(grupo_notificacao);
  if (grupo_notificacao.notificacao_size() > 0) {
    // Para desfazer;
    AdicionaNotificacaoListaEventos(grupo_notificacao);
  }
}


void Tabuleiro::DesenhaSombras() {
  const float kAnguloInclinacao = proto_corrente_->luz_direcional().inclinacao_graus() * GRAUS_PARA_RAD;
  const float kAnguloPosicao = proto_corrente_->luz_direcional().posicao_graus() * GRAUS_PARA_RAD;
  float fator_shear = proto_corrente_->luz_direcional().inclinacao_graus() == 90.0f ?
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
  float tam_x = proto_.largura() * TAMANHO_LADO_QUADRADO;
  float tam_y = proto_.altura() * TAMANHO_LADO_QUADRADO;
  gl::HabilitaEscopo blend_escopo(GL_BLEND);
  DesenhaStencil3d(tam_x, tam_y, cor_sombra);
}

void Tabuleiro::AtualizaOlho(int intervalo_ms, bool forcar) {
  const auto* entidade_referencia = BuscaEntidade(id_camera_presa_);
  if (camera_presa_) {
    if (entidade_referencia == nullptr) {
      AlternaCameraPresa();
    } else {
      bool cenario_diferente = entidade_referencia->Pos().id_cenario() != proto_corrente_->id_cenario();
      if (cenario_diferente) {
        // Pode acontecer da entidade estar se movendo para o cenario novo.
        if (entidade_referencia->Destino().has_id_cenario() &&
            (entidade_referencia->Destino().id_cenario() == proto_corrente_->id_cenario())) {
          cenario_diferente = false;
        }
      }
      if (cenario_diferente) {
        AlternaCameraPresa();
      } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
        olho_.clear_destino();
        olho_.mutable_pos()->set_x(entidade_referencia->X());
        olho_.mutable_pos()->set_y(entidade_referencia->Y());
        olho_.mutable_pos()->set_z(entidade_referencia->Z(true  /*delta_voo*/) + TAMANHO_LADO_QUADRADO * entidade_referencia->MultiplicadorTamanho());
        // Aqui fazemos o inverso da visao normal. Colocamos o alvo no angulo oposto da rotacao para olhar na mesma direcao
        // que a camera de perspectiva.
        olho_.mutable_alvo()->set_x(olho_.pos().x() + cosf(olho_.rotacao_rad() + M_PI));
        olho_.mutable_alvo()->set_y(olho_.pos().y() + sinf(olho_.rotacao_rad() + M_PI));
        olho_.mutable_alvo()->set_z(olho_.pos().z() - (olho_.altura() - OLHO_ALTURA_INICIAL) / 4.0f);
        return;
      } else {
        // Atualiza destino do olho caso a entidade tenha se afastado muito.
        Vector2 vdm(olho_.alvo().x() - entidade_referencia->X(), olho_.alvo().y() - entidade_referencia->Y());
        if (vdm.length() > OLHO_DISTANCIA_MAXIMA_CAMERA_PRESA) {
          vdm.normalize() *= OLHO_DISTANCIA_MAXIMA_CAMERA_PRESA;
          olho_.mutable_destino()->set_x(entidade_referencia->X() + vdm.x);
          olho_.mutable_destino()->set_y(entidade_referencia->Y() + vdm.y);
          olho_.mutable_destino()->set_z(entidade_referencia->Z());
        }
      }
    }
  }

  if (!forcar && !olho_.has_destino()) {
    return;
  }
  auto* pos_alvo = olho_.mutable_alvo();
  float origem[] = { pos_alvo->x(), pos_alvo->y(), pos_alvo->z() };
  if (olho_.has_destino() && intervalo_ms > 0) {
    const auto& pd = olho_.destino();
    float destino[] = { pd.x(), pd.y(), pd.z() };
    Vector3 v(Vector3(destino) - Vector3(origem));
    v.normalize() *= intervalo_ms * (VELOCIDADE_OLHO_M_S / 1000.0f);
    bool chegou = true;
    for (int i = 0; i < 3; ++i) {
      if (fabs(destino[i] - origem[i]) > fabs(v[i])) {
        origem[i] += v[i];
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
  pos_olho->set_z(pos_alvo->z() + olho_.altura());
}

void Tabuleiro::AtualizaRaioOlho(float raio) {
  if (raio < OLHO_RAIO_MINIMO) {
    raio = OLHO_RAIO_MINIMO;
  }
  else if (raio > OLHO_RAIO_MAXIMO) {
    raio = OLHO_RAIO_MAXIMO;
  }
  olho_.set_raio(raio);
  AtualizaOlho(0, true  /*forcar*/);
}

void Tabuleiro::AtualizaEntidades(int intervalo_ms) {
  for (auto& id_ent : entidades_) {
    id_ent.second->Atualiza(intervalo_ms);
  }
}

void Tabuleiro::AtualizaAcoes(int intervalo_ms) {
  // Qualquer acao adicionada aqui ja foi colocada na lista de desfazer durante a criacao.
  ignorar_lista_eventos_ = true;
  std::vector<std::unique_ptr<Acao>> copia_acoes;
  copia_acoes.swap(acoes_);
  bool limpar_salvacoes = false;
  for (auto& acao : copia_acoes) {
    acao->Atualiza(intervalo_ms);
    if (acao->EstadoAlvo() == Acao::ALVO_A_SER_ATINGIDO) {
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

void Tabuleiro::TrataBotaoEsquerdoPressionado(int x, int y, bool alterna_selecao) {
  ultimo_x_ = x;
  ultimo_y_ = y;

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
      case MODO_ACAO:
        TrataBotaoAcaoPressionadoPosPicking(false, x, y, id, tipo_objeto, profundidade);
        if (!lista_pontos_vida_.empty()) {
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
    PickingControleVirtual(x, y, alterna_selecao, id);
  } else if (tipo_objeto == OBJ_EVENTO_ENTIDADE) {
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
  MousePara3dParaleloZero(x, y, &x3d, &y3d, &z3d);
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
      // Neste caso, usa o X para rotacao e o Z para translacao.
      translacoes_rotacoes_antes_.insert(
          std::make_pair(entidade->Id(),
                         std::make_pair(entidade->Z(), entidade->RotacaoZGraus())));
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
  if (!EmModoMestre(true)) {
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
    MousePara3dComProfundidade(x, y, profundidade, &x3d, &y3d, &z3d);
    // Tabuleiro: cria uma entidade nova.
    SelecionaQuadrado(IdQuadrado(x3d, y3d));
    estado_ = ETAB_QUAD_SELECIONADO;
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    TrataNotificacao(notificacao);
  } else if (pos_pilha == OBJ_ENTIDADE || pos_pilha == OBJ_ENTIDADE_LISTA) {
    // Entidade.
    if (SelecionaEntidade(id, true  /*forcar_fixa*/)) {
      auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
      n->set_modo_mestre(EmModoMestre(true));
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(EntidadeSelecionada()->Proto());
      central_->AdicionaNotificacao(n);
    }
  } if (pos_pilha == OBJ_CONTROLE_VIRTUAL) {
    PickingControleVirtual(x, y, false  /*alterna selecao*/, id);
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
  if ((!forcar_fixa && entidade->Fixa()) || (!EmModoMestre(true) && !entidade->SelecionavelParaJogador())) {
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
    if (entidade == nullptr || entidade->Fixa() ||
        (!EmModoMestre(true) && !entidade->SelecionavelParaJogador())) {
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
  if (e == nullptr || (!EmModoMestre(true) && !e->SelecionavelParaJogador())) {
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
      ciclos_para_atualizar_ = -1;
      if (translacao_rotacao_ == TR_NENHUM) {
        // Nada a fazer.
      } else {
        ntf::Notificacao grupo_notificacoes;
        grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
        for (unsigned int id : ids_entidades_selecionadas_) {
          auto* entidade = BuscaEntidade(id);
          if (entidade == nullptr) {
            continue;
          }
          auto* n = grupo_notificacoes.add_notificacao();
          n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
          auto* e_antes = n->mutable_entidade_antes();
          e_antes->set_id(entidade->Id());
          e_antes->mutable_pos()->set_z(translacoes_rotacoes_antes_[entidade->Id()].first);
          e_antes->set_rotacao_z_graus(translacoes_rotacoes_antes_[entidade->Id()].second);
          // A entidade ja foi alterada durante a rotacao.
          n->mutable_entidade()->set_id(entidade->Id());
          n->mutable_entidade()->mutable_pos()->set_z(entidade->Z());
          n->mutable_entidade()->set_rotacao_z_graus(entidade->RotacaoZGraus());
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
      if (!camera_presa_ && id_camera_presa_ != Entidade::IdInvalido) {
        // Restaura camera presa antes do movimento.
        camera_presa_ = true;
      }
      return;
    case ETAB_ENTS_PRESSIONADAS: {
      ciclos_para_atualizar_ = -1;
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
  if (id_quadrado < 0) {
    LOG(WARNING) << "Id do quadrado invalido: " << id_quadrado;
    //return;
  }
  quadrado_selecionado_ = id_quadrado;
  ids_entidades_selecionadas_.clear();
  estado_ = ETAB_QUAD_PRESSIONADO;
}

unsigned int Tabuleiro::IdQuadrado(float x, float y) {
  float inicio_x = -(TamanhoX() * TAMANHO_LADO_QUADRADO) / 2.0f;
  float delta_x_float = x - inicio_x;
  int delta_x = delta_x_float / TAMANHO_LADO_QUADRADO;
  if (delta_x < 0 || delta_x >= TamanhoX()) {
    VLOG(1) << "Posicao invalida para tabuleiro, x: " << x;
    return -1;
  }
  float inicio_y = -(TamanhoY() * TAMANHO_LADO_QUADRADO) / 2.0f;
  float delta_y_float = y - inicio_y;
  int delta_y = delta_y_float / TAMANHO_LADO_QUADRADO;
  if (delta_y >= TamanhoY()) {
    VLOG(1) << "Posicao invalida para tabuleiro, y: " << y;
    return -1;
  }
  return delta_y * TamanhoX() + delta_x;
}

void Tabuleiro::CoordenadaQuadrado(unsigned int id_quadrado, float* x, float* y, float* z) {
  CoordenadaSwQuadrado(id_quadrado, x, y);
  // centro do quadrado
  *x += TAMANHO_LADO_QUADRADO_2;
  *y += TAMANHO_LADO_QUADRADO_2;
  //*z = parametros_desenho_.offset_terreno();
  *z = ZChao(*x, *y);
}

void Tabuleiro::CoordenadaSwQuadrado(unsigned int id_quadrado, float* x, float* y, float* z) {
  int x_quad = id_quadrado % TamanhoX();
  int y_quad = id_quadrado / TamanhoX();
  VLOG(2) << "id_quadrado: " << id_quadrado << ", x_quad: " << x_quad << ", y_quad: " << y_quad;
  CoordenadaSwQuadrado(x_quad, y_quad, x, y, z);
}

void Tabuleiro::CoordenadaSwQuadrado(int x_quad, int y_quad, float* x, float* y, float* z) {
  VLOG(2) << "x_quad: " << x_quad << ", y_quad: " << y_quad;
  *x = (x_quad * TAMANHO_LADO_QUADRADO) - (TamanhoX() * TAMANHO_LADO_QUADRADO_2);
  *y = (y_quad * TAMANHO_LADO_QUADRADO) - (TamanhoY() * TAMANHO_LADO_QUADRADO_2);
  if (z != nullptr) {
    // Mais barato que chamar ZChao.
    *z = AlturaPonto(x_quad, y_quad);
  }
}

ntf::Notificacao* Tabuleiro::SerializaPropriedades() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_PROPRIEDADES_TABULEIRO);
  auto* tabuleiro = notificacao->mutable_tabuleiro();
  tabuleiro->set_id_cenario(proto_corrente_->id_cenario());
  tabuleiro->set_id_cliente(id_cliente_);
  tabuleiro->mutable_luz_ambiente()->CopyFrom(proto_corrente_->luz_ambiente());
  tabuleiro->mutable_luz_direcional()->CopyFrom(proto_corrente_->luz_direcional());
  if (proto_corrente_->has_info_textura()) {
    tabuleiro->mutable_info_textura()->CopyFrom(proto_corrente_->info_textura());
    tabuleiro->set_ladrilho(proto_corrente_->ladrilho());
    tabuleiro->set_textura_mestre_apenas(proto_corrente_->textura_mestre_apenas());
  }
  if (proto_corrente_->has_info_textura_ceu()) {
    tabuleiro->mutable_info_textura_ceu()->CopyFrom(proto_corrente_->info_textura_ceu());
  }
  if (proto_corrente_->has_nevoa()) {
    tabuleiro->mutable_nevoa()->CopyFrom(proto_corrente_->nevoa());
  }
  tabuleiro->set_largura(proto_corrente_->largura());
  tabuleiro->set_altura(proto_corrente_->altura());
  tabuleiro->set_desenha_grade(proto_corrente_->desenha_grade());
  tabuleiro->set_aplicar_luz_ambiente_textura_ceu(proto_corrente_->aplicar_luz_ambiente_textura_ceu());
  return notificacao;
}

ntf::Notificacao* Tabuleiro::SerializaRelevoCenario() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
  auto* tabuleiro = notificacao->mutable_tabuleiro();
  tabuleiro->set_id_cenario(proto_corrente_->id_cenario());
  *tabuleiro->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  return notificacao;
}

ntf::Notificacao* Tabuleiro::SerializaOpcoes() const {
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_OPCOES);
  notificacao->mutable_opcoes()->CopyFrom(opcoes_);
  return notificacao;
}

namespace {
// Mantem a topologia apos uma mudanca de tamanho. Os valores de tam_* sao em quadrados, portante deve-se
// adicionar 1 para obter o numero de pontos.
void CorrigeTopologiaAposMudancaTamanho(
    int tam_x_velho, int tam_y_velho, int tam_x_novo, int tam_y_novo,
    RepeatedField<double>* pontos) {
  if ((tam_x_velho == tam_x_novo && tam_y_velho == tam_y_novo) ||
      pontos->empty()) {
    return;
  }
  LOG(INFO) << "tamxvelho: " << tam_x_velho << ", tamyvelho:" << tam_y_velho
            << ", tamxnovo: " << tam_x_novo << ", tamynovo: " << tam_y_novo
            << ", pontos_size: " << pontos->size();
  RepeatedField<double> novos_pontos;
  novos_pontos.Resize((tam_x_novo + 1) * (tam_y_novo + 1), 0.0f);
  for (int i = 0; i < pontos->size(); ++i) {
    float z = pontos->Get(i);
    if (z > 0) {
      LOG(INFO) << "ponto (" << i << ") > 0: " << z;
      break;
    }
  }

  for (int y = 0; y <= tam_y_velho; ++y) {
    for (int x = 0; x <= tam_x_velho; ++x) {
      int indice = y * (tam_x_velho + 1) + x;
      LOG(INFO) << "indice: " << indice;
      float z = pontos->Get(indice);
      if (z > 0) {
        LOG(INFO) << "ponto (" << x << ", " << y << ") > 0: " << z;
        break;
      }
    }
  }

  for (int y = 0; y <= tam_y_velho; ++y) {
    int novo_y = (y - tam_y_velho / 2) + (tam_y_novo / 2);
    if (novo_y < 0 || novo_y > tam_y_novo) {
      // ponto fora, descarta.
      continue;
    }
    for (int x = 0; x <= tam_x_velho; ++x) {
      int novo_x = (x - tam_x_velho / 2) + (tam_x_novo / 2);
      if (novo_x < 0 || novo_x > tam_x_novo) {
        continue;
      }
      float z = pontos->Get(y * (tam_x_velho + 1) + x);
      //LOG(INFO) << "copiando: " << x << ", " << y << ": '" << z << "' para " << novo_x << ", " << novo_y;
      novos_pontos.Set(novo_y * (tam_x_novo + 1) + novo_x, z);
    }
  }
  novos_pontos.Swap(pontos);
  VLOG(1) << "Swapei";
}
}  // namespace

void Tabuleiro::DeserializaPropriedades(const ent::TabuleiroProto& novo_proto_const) {
  // Copia para poder remover uns lixos.
  ent::TabuleiroProto novo_proto(novo_proto_const);
  if (novo_proto.info_textura_ceu().id().empty()) {
    novo_proto.clear_info_textura_ceu();
  }
  if (novo_proto.info_textura().id().empty()) {
    novo_proto.clear_info_textura();
  }
  VLOG(1) << "Atualizando propriedades: " << novo_proto.ShortDebugString();
  TabuleiroProto* proto_a_atualizar = BuscaSubCenario(novo_proto.id_cenario());
  if (proto_a_atualizar == nullptr) {
    LOG(ERROR) << "Sub cenario " << novo_proto.id_cenario() << " nao existe";
    return;
  }
  int tam_x_velho = proto_a_atualizar->largura();
  int tam_y_velho = proto_a_atualizar->altura();
  proto_a_atualizar->mutable_luz_ambiente()->CopyFrom(novo_proto.luz_ambiente());
  proto_a_atualizar->mutable_luz_direcional()->CopyFrom(novo_proto.luz_direcional());
  proto_a_atualizar->set_largura(novo_proto.largura());
  proto_a_atualizar->set_altura(novo_proto.altura());
  proto_a_atualizar->set_desenha_grade(novo_proto.desenha_grade());
  proto_a_atualizar->set_aplicar_luz_ambiente_textura_ceu(novo_proto.aplicar_luz_ambiente_textura_ceu());
  if (novo_proto.has_descricao_cenario()) {
    proto_a_atualizar->set_descricao_cenario(novo_proto.descricao_cenario());
  } else {
    proto_a_atualizar->clear_descricao_cenario();
  }
  if (novo_proto.has_nevoa()) {
    proto_a_atualizar->mutable_nevoa()->CopyFrom(novo_proto.nevoa());
  } else {
    proto_a_atualizar->clear_nevoa();
  }
  AtualizaTexturas(novo_proto);
  CorrigeTopologiaAposMudancaTamanho(
      tam_x_velho, tam_y_velho, proto_a_atualizar->largura(), proto_a_atualizar->altura(), proto_a_atualizar->mutable_ponto_terreno());
  RegeraVboTabuleiro();
}

void Tabuleiro::DeserializaRelevoCenario(const TabuleiroProto& novo_proto) {
  // Copia para poder remover uns lixos.
  auto* cenario = BuscaSubCenario(novo_proto.id_cenario());
  if (cenario == nullptr) {
    LOG(WARNING) << "Cenario invalido " << novo_proto.id_cenario() << " para atualizacao de relevo";
    return;
  }
  *cenario->mutable_ponto_terreno() = novo_proto.ponto_terreno();
  RegeraVboTabuleiro();
}

ntf::Notificacao* Tabuleiro::SerializaTabuleiro(const std::string& nome) {
  auto* notificacao = new ntf::Notificacao;
  try {
    notificacao->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    auto* t = notificacao->mutable_tabuleiro();
    t->CopyFrom(proto_);
    std::vector<TabuleiroProto*> cenarios;
    cenarios.push_back(t);
    for (auto& sub_cenario : *t->mutable_sub_cenario()) {
      cenarios.push_back(&sub_cenario);
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
    EstadoInicial(true);
  }
  if (notificacao.has_erro()) {
    auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
    ne->set_erro(std::string("Erro ao deserializar tabuleiro: ") + notificacao.erro());
    central_->AdicionaNotificacao(ne);
    auto* n = ntf::NovaNotificacao(ntf::TN_DESCONECTAR);
    central_->AdicionaNotificacao(n);
    return;
  }
  // Cria os sub cenarios dummy para atualizacao de textura funcionar,
  // caso contrario ela dira que o sub cenario nao existe e nao funcionara.
  for (auto& sub_cenario : tabuleiro.sub_cenario()) {
    auto* cenario_dummy = proto_.add_sub_cenario();
    cenario_dummy->set_id_cenario(sub_cenario.id_cenario());
  }
  AtualizaTexturasIncluindoSubCenarios(tabuleiro);
  proto_.CopyFrom(tabuleiro);
  if (proto_.has_camera_inicial()) {
    ReiniciaCamera();
  }
  proto_.clear_manter_entidades();  // Os clientes nao devem receber isso.
  proto_.clear_entidade();  // As entidades serao armazenadas abaixo.
  proto_.clear_id_cliente();
  RegeraVboTabuleiro();
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
    auto* e = NovaEntidade(ep, texturas_, m3d_, central_);
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

  bool entidades_do_jogador = n.entidade().selecionavel_para_jogador();
  // Hack para entidades aparecerem visiveis e selecionaveis caso necessario.
  bool modo_mestre_anterior = modo_mestre_;
  if (entidades_do_jogador) {
    modo_mestre_ = false;
  }
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

TabuleiroProto* Tabuleiro::BuscaSubCenario(int id_cenario) {
  if (id_cenario == CENARIO_PRINCIPAL) {
    return &proto_;
  }
  for (auto& sub_cenario : *proto_.mutable_sub_cenario()) {
    if (sub_cenario.id_cenario() == id_cenario) {
      return &sub_cenario;
    }
  }
  return nullptr;
}

void Tabuleiro::CriaSubCenarioNotificando(const ntf::Notificacao& notificacao) {
  int id_cenario = notificacao.tabuleiro().id_cenario();
  if (BuscaSubCenario(id_cenario) != nullptr) {
    LOG(ERROR) << "Cenario ja existe";
    return;
  }
  auto* cenario = proto_.add_sub_cenario();
  cenario->set_id_cenario(id_cenario);
  if (notificacao.tabuleiro().has_luz_ambiente()) {
    // Notificacao possui campos de tabuleiro, deserializa.
    DeserializaPropriedades(notificacao.tabuleiro());
  } else {
    // Padrao, apenas reinicia a iluminacao para nao ficar tudo escuro.
    ReiniciaIluminacao(cenario);
  }
  LOG(INFO) << "Cenario criado";
  if (!notificacao.local()) {
    return;
  }
  // Envia para clientes.
  central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
  // Para desfazer.
  AdicionaNotificacaoListaEventos(notificacao);
}

void Tabuleiro::RemoveSubCenarioNotificando(const ntf::Notificacao& notificacao) {
  int id_cenario = notificacao.tabuleiro().has_id_cenario() ? notificacao.tabuleiro().id_cenario() : proto_corrente_->id_cenario();
  if (id_cenario == CENARIO_PRINCIPAL) {
    LOG(ERROR) << "Nao eh possivel remover o cenario principal.";
    return;
  }
  if (proto_corrente_->id_cenario() == id_cenario) {
    // Carrega o cenario principal antes de remover o corrente.
    LOG(INFO) << "Carregando cenario principal porque o removido eh o corrente.";
    // Dificil saber para onde voltar, entao volta para camera principal do cenario principal.
    CarregaSubCenario(CENARIO_PRINCIPAL, proto_.camera_inicial().alvo());
  }
  LOG(INFO) << "Tam sub cenario antes: " << proto_.sub_cenario_size();
  bool removeu = false;
  TabuleiroProto cenario_para_desfazer;
  for (int i = 0; i < proto_.sub_cenario_size(); ++i) {
    const auto& sub_cenario = proto_.sub_cenario(i);
    if (sub_cenario.id_cenario() == id_cenario) {
      // Descarrega as texturas.
      cenario_para_desfazer.CopyFrom(sub_cenario);
      TabuleiroProto dummy;
      dummy.set_id_cenario(id_cenario);
      AtualizaTexturas(dummy);
      proto_.mutable_sub_cenario()->DeleteSubrange(i, 1);
      LOG(INFO) << "Tam sub cenario depois: " << proto_.sub_cenario_size();
      removeu = true;
      break;
    }
  }
  if (!removeu) {
    LOG(INFO) << "Sub cenario nao encontrado";
    return;
  }
  LOG(INFO) << "Cenario removido";
  if (!notificacao.local()) {
    // A remocao das entidades vira pela rede.
    return;
  }
  central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));

  // Remove entidades do cenario.
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (const auto& par_id_ent : entidades_) {
    const auto* e = par_id_ent.second.get();
    if (e->Pos().id_cenario() != id_cenario ||
        (e->Proto().has_destino() && (e->Proto().destino().id_cenario() != id_cenario))) {
      // Se a entidade estiver em outro cenario, ou estiver indo para outro cenario, nao remove.
      // O destino pode estar setado e a posical estar em outro cenario ainda se a entidade ainda nao foi atualizada.
      // Isso acontece durante desfazer.
      continue;
    }
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_REMOVER_ENTIDADE);
    n->mutable_entidade()->CopyFrom(e->Proto());
  }
  TrataNotificacao(grupo_notificacoes);

  // Para desfazer.
  auto* n = grupo_notificacoes.add_notificacao();
  n->set_tipo(ntf::TN_REMOVER_CENARIO);
  n->mutable_tabuleiro()->Swap(&cenario_para_desfazer);
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
}

void Tabuleiro::DeserializaOpcoes(const ent::OpcoesProto& novo_proto) {
  opcoes_.CopyFrom(novo_proto);
#if !__APPLE__ || !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    gl::Habilita(GL_MULTISAMPLE);
  } else {
    gl::Desabilita(GL_MULTISAMPLE);
  }
#endif
  SalvaConfiguracoes(opcoes_);
  V_ERRO("erro deserializando GL");
}

void Tabuleiro::CarregaSubCenario(int id_cenario, const Posicao& camera) {
  cenario_corrente_ = id_cenario;
  TabuleiroProto* cenario = BuscaSubCenario(id_cenario);
  if (cenario == nullptr) {
    LOG(ERROR) << "Cenario " << id_cenario << " nao existe";
    return;
  }
  // Deseleciona entidades que nao transitaram.
  std::vector<unsigned int> ids_a_deselecionar;
  for (auto id : ids_entidades_selecionadas_) {
    auto* e = BuscaEntidade(id);
    if (e == nullptr ||
        ((e->Pos().id_cenario() != id_cenario) && (e->Destino().id_cenario() != id_cenario))) {
      ids_a_deselecionar.push_back(e->Id());
    }
  }
  for (auto id : ids_a_deselecionar) {
    DeselecionaEntidade(id);
  }
  proto_corrente_ = cenario;
  RegeraVboTabuleiro();
  // A caixa do ceu nao precisa porque o objeto dela eh fixo.

  olho_.mutable_alvo()->CopyFrom(camera);
  olho_.clear_destino();
  AtualizaOlho(0, true  /*forcar*/);
}

Entidade* Tabuleiro::BuscaEntidade(unsigned int id) {
  auto it = entidades_.find(id);
  return (it != entidades_.end()) ? it->second.get() : nullptr;
}

const Entidade* Tabuleiro::BuscaEntidade(unsigned int id) const {
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
    if (e == nullptr || e->Tipo() == TE_ENTIDADE) {
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
  if (num_entidades <= 1) {
    VLOG(1) << "Nenhuma (ou uma) entidade valida para agrupar";
    return;
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

void Tabuleiro::TrataMovimentoEntidadesSelecionadas(bool frente_atras, float valor) {
  Posicao vetor_camera;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &vetor_camera);
  // angulo da camera em relacao ao eixo X.
  float rotacao_graus = VetorParaRotacaoGraus(vetor_camera.x(), vetor_camera.y());
  float dx = 0;
  float dy = 0;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    Vector2 vetor_movimento;
    if (frente_atras) {
      vetor_movimento = Vector2(vetor_camera.x(), vetor_camera.y());
    } else {
      RodaVetor2d(-90.0f, &vetor_camera);
      vetor_movimento = Vector2(vetor_camera.x(), vetor_camera.y());
    }
    vetor_movimento.normalize() *= valor;
    dx = vetor_movimento.x;
    dy = vetor_movimento.y;
  } else {
    if (rotacao_graus > -45.0f && rotacao_graus <= 45.0f) {
      // Camera apontando para x positivo.
      if (frente_atras) {
        dx = TAMANHO_LADO_QUADRADO * valor;
      } else {
        dy = TAMANHO_LADO_QUADRADO * -valor;
      }
    } else if (rotacao_graus > 45.0f && rotacao_graus <= 135) {
      // Camera apontando para y positivo.
      if (frente_atras) {
        dy = TAMANHO_LADO_QUADRADO * valor;
      } else {
        dx = TAMANHO_LADO_QUADRADO * valor;
      }
    } else if (rotacao_graus > 135 || rotacao_graus < -135) {
      // Camera apontando para x negativo.
      if (frente_atras) {
        dx = TAMANHO_LADO_QUADRADO * -valor;
      } else {
        dy = TAMANHO_LADO_QUADRADO * valor;
      }
    } else {
      // Camera apontando para y negativo.
      if (frente_atras) {
        dy = TAMANHO_LADO_QUADRADO * -valor;
      } else {
        dx = TAMANHO_LADO_QUADRADO * -valor;
      }
    }
  }
  // TODO direito com eventos.
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  std::unordered_set<unsigned int> ids;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    ids.insert(id_camera_presa_);
  } else {
    ids = ids_entidades_selecionadas_;
  }
  for (unsigned int id : ids) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    VLOG(2) << "Movendo entidade " << id << ", dx: " << dx << ", dy: " << dy;
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_MOVER_ENTIDADE);
    auto* e = n->mutable_entidade();
    e->set_id(id);
    float nx = entidade_selecionada->X() + dx;
    float ny = entidade_selecionada->Y() + dy;
    auto* p = e->mutable_destino();
    p->set_x(nx);
    p->set_y(ny);
    float z_antes = entidade_selecionada->Z();
    float z_chao_antes = ZChao(entidade_selecionada->X(), entidade_selecionada->Y());
    float z_chao_depois = ZChao(nx, ny);
    bool manter_chao = (z_antes - z_chao_antes) < 0.001f;
    if (manter_chao) {
      p->set_z(z_chao_depois);
    } else {
      p->set_z(std::max(z_chao_depois, entidade_selecionada->Z()));
    }
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

void Tabuleiro::TrataTranslacaoZ(float delta) {
  if (ModoClique() == MODO_TERRENO) {
    TrataDeltaTerreno(delta * TAMANHO_LADO_QUADRADO);
  } else {
    ntf::Notificacao grupo_notificacoes;
    grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
    for (unsigned int id : ids_entidades_selecionadas_) {
      auto* entidade_selecionada = BuscaEntidade(id);
      if (entidade_selecionada == nullptr) {
        continue;
      }
      // Salva para desfazer.
      auto* n = grupo_notificacoes.add_notificacao();
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      e->set_id(entidade_selecionada->Id());
      e->mutable_pos()->CopyFrom(entidade_selecionada->Pos());
      // Altera a translacao em Z.
      entidade_selecionada->IncrementaZ(delta * TAMANHO_LADO_QUADRADO);
      e->mutable_destino()->CopyFrom(entidade_selecionada->Pos());
    }
    // Nop mas envia para os clientes.
    TrataNotificacao(grupo_notificacoes);
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
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
      // TODO inverter a ordem das notificacoes.
      for (const auto& n : n_original.notificacao()) {
        n_inversa.add_notificacao()->CopyFrom(InverteNotificacao(n));
      }
      break;
    case ntf::TN_ATUALIZAR_RELEVO_TABULEIRO:
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
      n_inversa.mutable_tabuleiro()->set_id_cenario(n_original.tabuleiro_antes().id_cenario());
      *n_inversa.mutable_tabuleiro()->mutable_ponto_terreno() = n_original.tabuleiro_antes().ponto_terreno();
      break;
    case ntf::TN_CRIAR_CENARIO: {
      if (!n_original.tabuleiro().has_id_cenario()) {
        LOG(ERROR) << "Nao eh possivel inverter TN_CRIAR_CENARIO sem id de cenario";
        break;
      }
      n_inversa.set_tipo(ntf::TN_REMOVER_CENARIO);
      n_inversa.mutable_tabuleiro()->CopyFrom(n_original.tabuleiro());
      break;
    }
    case ntf::TN_REMOVER_CENARIO: {
      if (!n_original.has_tabuleiro()) {
        LOG(ERROR) << "Nao eh possivel inverter TN_REMOVER_CENARIO sem tabuleiro";
        break;
      }
      n_inversa.set_tipo(ntf::TN_CRIAR_CENARIO);
      n_inversa.mutable_tabuleiro()->CopyFrom(n_original.tabuleiro());
      break;
    }
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
        LOG(ERROR) << "Impossivel inverter ntf::TN_MOVER_ENTIDADE sem a posicao original ou ID: "
                   << n_original.entidade().ShortDebugString();
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
  VLOG(1) << "Notificacao inversa: " << n_inversa.ShortDebugString();
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

void Tabuleiro::SelecionaTudo(bool fixas) {
  std::vector<unsigned int> ids;
  for (const auto& id_ent : entidades_) {
    const Entidade* entidade = id_ent.second.get();
    if (entidade == nullptr ||
        entidade->IdCenario() != proto_corrente_->id_cenario() ||
        (!fixas && entidade->Fixa()) ||
        (!EmModoMestre(true) && !entidade->SelecionavelParaJogador())) {
      continue;
    }
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

#define BITS_CLIENTE 4
unsigned int Tabuleiro::GeraIdEntidade(int id_cliente) {
  unsigned int count = gl::NumeroMaximoEntidades();
  while (count-- > 0) {
    unsigned int id = (id_cliente << (32 - BITS_CLIENTE)) | proximo_id_entidade_;
    proximo_id_entidade_ = ((proximo_id_entidade_ + 1) % gl::IdMaximoEntidade());
    auto it = entidades_.find(id);
    if (it == entidades_.end()) {
      return id;
    }
  }
  throw std::logic_error("Limite de entidades alcancado para cliente.");
}

int Tabuleiro::GeraIdTabuleiro() {
  const int max_id_cliente = (1 << BITS_CLIENTE) - 1;
  int count = max_id_cliente;
  while (count-- > 0) {
    int id_tab = proximo_id_cliente_;
    auto it = clientes_.find(id_tab);
    // O id zero esta sempre reservado para o mestre.
    proximo_id_cliente_ = ((proximo_id_cliente_) % max_id_cliente) + 1;
    if (it == clientes_.end()) {
      LOG(INFO) << "GeraIdTabuleiro retornando id para cliente: " << id_tab;
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
    nl->add_info_textura()->CopyFrom(*velho_proto);
    central->AdicionaNotificacao(nl);
  }
  // Carrega textura se houver e for diferente da antiga.
  if (novo_tem && novo_proto.id() != velho_proto->id()) {
    VLOG(2) << "Carregando textura: " << novo_proto.id();
    auto* nc = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    nc->add_info_textura()->CopyFrom(novo_proto);
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

void Tabuleiro::AtualizaTexturasIncluindoSubCenarios(const ent::TabuleiroProto& proto_principal) {
  AtualizaTexturas(proto_principal);
  for (const auto& sub_cenario : proto_principal.sub_cenario()) {
    AtualizaTexturas(sub_cenario);
  }
}

void Tabuleiro::AtualizaTexturas(const TabuleiroProto& novo_proto) {
  TabuleiroProto* proto_a_atualizar = BuscaSubCenario(novo_proto.id_cenario());
  if (proto_a_atualizar == nullptr) {
    LOG(ERROR) << "Sub cenario " << novo_proto.id_cenario() << " nao existe para atualizacao de texturas";
    return;
  }
  if (ent::AtualizaTexturas(novo_proto.has_info_textura(), novo_proto.info_textura(),
                            proto_a_atualizar->has_info_textura(), proto_a_atualizar->mutable_info_textura(),
                            central_)) {
    proto_a_atualizar->set_ladrilho(novo_proto.ladrilho());
    proto_a_atualizar->set_textura_mestre_apenas(novo_proto.textura_mestre_apenas());
  } else {
    proto_a_atualizar->clear_info_textura();
    proto_a_atualizar->clear_ladrilho();
    proto_a_atualizar->clear_textura_mestre_apenas();
  }
  if (!ent::AtualizaTexturas(novo_proto.has_info_textura_ceu(), novo_proto.info_textura_ceu(),
                             proto_a_atualizar->has_info_textura_ceu(), proto_a_atualizar->mutable_info_textura_ceu(),
                             central_)) {
    proto_a_atualizar->clear_info_textura_ceu();
  }
}

void Tabuleiro::DesenhaLuzes() {
  // Entidade de referencia para camera presa.
  parametros_desenho_.clear_nevoa();
  auto* entidade_referencia = BuscaEntidade(id_camera_presa_);
  if (parametros_desenho_.desenha_nevoa() && parametros_desenho_.tipo_visao() == VISAO_ESCURO &&
      (!VisaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    gl::Habilita(GL_FOG);
    float pos[4] = { 0, 0, 0, 1 };
    const Posicao& epos = entidade_referencia->Pos();
    pos[0] = epos.x();
    pos[1] = epos.y();
    pos[2] = epos.z();
    float alcance_visao = entidade_referencia->Proto().has_alcance_visao() ? entidade_referencia->Proto().alcance_visao() : 18.0f;
    ConfiguraNevoa(alcance_visao, alcance_visao + 0.1f, 0, 0, 0, pos, &parametros_desenho_);
    parametros_desenho_.clear_iluminacao();
    gl::Desabilita(GL_LIGHTING);
    return;
  }

  GLfloat cor_luz_ambiente[] = { proto_corrente_->luz_ambiente().r(),
                                 proto_corrente_->luz_ambiente().g(),
                                 proto_corrente_->luz_ambiente().b(),
                                 proto_corrente_->luz_ambiente().a()};
  if (VisaoMestre() && !opcoes_.iluminacao_mestre_igual_jogadores()) {
    // Adiciona luz pro mestre ver melhor.
    cor_luz_ambiente[0] = std::max(0.65f, cor_luz_ambiente[0]);
    cor_luz_ambiente[1] = std::max(0.65f, cor_luz_ambiente[1]);
    cor_luz_ambiente[2] = std::max(0.65f, cor_luz_ambiente[2]);
  } else if (parametros_desenho_.tipo_visao() == VISAO_BAIXA_LUMINOSIDADE) {
    cor_luz_ambiente[0] = std::min(1.0f, cor_luz_ambiente[0] * parametros_desenho_.multiplicador_visao_penumbra());
    cor_luz_ambiente[1] = std::min(1.0f, cor_luz_ambiente[1] * parametros_desenho_.multiplicador_visao_penumbra());
    cor_luz_ambiente[2] = std::min(1.0f, cor_luz_ambiente[2] * parametros_desenho_.multiplicador_visao_penumbra());
  }
  gl::LuzAmbiente(cor_luz_ambiente[0], cor_luz_ambiente[1], cor_luz_ambiente[2]);

  // Iluminação distante direcional.
  {
    gl::MatrizEscopo salva_matriz(false);
    //gl::CarregaIdentidade();
    // O vetor inicial esta no leste (origem da luz). O quarte elemento indica uma luz no infinito.
    GLfloat pos_luz[] = { 1.0, 0.0f, 0.0f, 0.0f };
    // Roda no eixo Z (X->Y) em direcao a posicao entao inclina a luz no eixo -Y (de X->Z).
    gl::Roda(proto_corrente_->luz_direcional().posicao_graus(), 0.0f, 0.0f, 1.0f, false);
    gl::Roda(proto_corrente_->luz_direcional().inclinacao_graus(), 0.0f, -1.0f, 0.0f);
    // A cor da luz direcional.
    GLfloat cor_luz[] = {
        proto_corrente_->luz_direcional().cor().r(),
        proto_corrente_->luz_direcional().cor().g(),
        proto_corrente_->luz_direcional().cor().b(),
        proto_corrente_->luz_direcional().cor().a() };
    if (parametros_desenho_.tipo_visao() == VISAO_BAIXA_LUMINOSIDADE) {
      cor_luz[0] = std::min(1.0f, cor_luz[0] * parametros_desenho_.multiplicador_visao_penumbra());
      cor_luz[1] = std::min(1.0f, cor_luz[1] * parametros_desenho_.multiplicador_visao_penumbra());
      cor_luz[2] = std::min(1.0f, cor_luz[2] * parametros_desenho_.multiplicador_visao_penumbra());
    }
    gl::LuzDirecional(pos_luz, cor_luz[0], cor_luz[1], cor_luz[2]);
    gl::Habilita(GL_LIGHT0);
  }

  if (parametros_desenho_.desenha_nevoa() && proto_corrente_->has_nevoa() &&
      (!VisaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    gl::Habilita(GL_FOG);
    float pos[4] = { olho_.pos().x(), olho_.pos().y(), olho_.pos().z(), 1 };
    if (entidade_referencia != nullptr) {
      // So funciona com shader.
      const Posicao& epos = entidade_referencia->Pos();
      pos[0] = epos.x();
      pos[1] = epos.y();
      pos[2] = epos.z();
    }
    ConfiguraNevoa(proto_corrente_->nevoa().minimo(), proto_corrente_->nevoa().maximo(),
                   cor_luz_ambiente[0], cor_luz_ambiente[1], cor_luz_ambiente[2], pos, &parametros_desenho_);
  } else {
    gl::Desabilita(GL_FOG);
  }

  // Posiciona as luzes dinâmicas.
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    auto* e = it->second.get();
    if (e == nullptr || e->IdCenario() != proto_corrente_->id_cenario()) {
      continue;
    }
    e->DesenhaLuz(&parametros_desenho_);
  }
}

void Tabuleiro::DesenhaCaixaCeu() {
  gl::TipoShader tipo_anterior = gl::TipoShaderCorrente();
  gl::UsaShader(gl::TSH_CAIXA_CEU);
  GLuint id_textura = texturas_->Textura(proto_corrente_->info_textura_ceu().id());
  GLenum tipo_textura = texturas_->TipoTextura(proto_corrente_->info_textura_ceu().id());
  if (!proto_corrente_->aplicar_luz_ambiente_textura_ceu()) {
    MudaCor(COR_BRANCA);
  } else {
    MudaCor(proto_corrente_->luz_ambiente());
  }

  gl::MatrizEscopo salva_mv(GL_MODELVIEW, false);
  gl::Translada(olho_.pos().x(), olho_.pos().y(), olho_.pos().z(), false);

  //gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  //gl::DesligaEscritaProfundidadeEscopo desliga_escrita_escopo;
  gl::FuncaoProfundidade(GL_LEQUAL);  // O shader vai escrever pro mais longe.
  gl::FaceNula(GL_FRONT);
  gl::UnidadeTextura(tipo_textura == GL_TEXTURE_CUBE_MAP ? GL_TEXTURE2 : GL_TEXTURE0);
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(tipo_textura);
    gl::LigacaoComTextura(tipo_textura, id_textura);
    vbo_caixa_ceu_.forca_texturas(true);
  } else {
    gl::Desabilita(tipo_textura);
    gl::LigacaoComTextura(tipo_textura, 0);
    vbo_caixa_ceu_.forca_texturas(false);
  }
  gl::DesenhaVbo(vbo_caixa_ceu_);
  gl::LigacaoComTextura(tipo_textura, 0);
  gl::Desabilita(tipo_textura);
  gl::UnidadeTextura(GL_TEXTURE0);
  // Religa luzes.
  gl::FaceNula(GL_BACK);
  gl::FuncaoProfundidade(GL_LESS);
  gl::UsaShader(tipo_anterior);
}

void Tabuleiro::DesenhaGrade() {
  gl::DesligaEscritaProfundidadeEscopo desliga_escrita_escopo;
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  //gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
  MudaCor(COR_PRETA);
  //gl::DesvioProfundidade(OFFSET_GRADE_ESCALA_DZ, OFFSET_GRADE_ESCALA_R);
  gl::DesenhaVbo(vbo_grade_, GL_TRIANGLES);
}

void Tabuleiro::DesenhaListaPontosVida() {
  if (lista_pontos_vida_.empty() && !modo_dano_automatico_) {
    return;
  }
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  // Modo 2d: eixo com origem embaixo esquerda.
  int raster_x = 0, raster_y = 0;
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  raster_y = altura_ - altura_fonte;
  raster_x = largura_ - 2;
  PosicionaRaster2d(raster_x, raster_y, largura_, altura_);

  MudaCor(COR_BRANCA);
  std::string titulo("Lista PV");
  gl::DesenhaStringAlinhadoDireita(titulo);
  raster_y -= (altura_fonte + 2);
  if (modo_dano_automatico_) {
    PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
    raster_y -= (altura_fonte + 2);
    MudaCor(COR_BRANCA);
    const auto* entidade = EntidadeSelecionada();
    std::string valor = "AUTO";
    if (entidade != nullptr) {
      const std::string s = entidade->StringValorParaAcao(entidade->Acao(AcoesPadroes()));
      if (s.empty()) {
        valor += ": SEM ACAO";
      } else {
        valor += ": " + s;
      }
    } else if (ids_entidades_selecionadas_.size() > 1) {
      valor += ": VARIOS";
    } else {
      valor += ": NENHUM";
    }
    gl::DesenhaStringAlinhadoDireita(valor);
  } else {
    for (int pv : lista_pontos_vida_) {
      PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
      raster_y -= (altura_fonte + 2);
      MudaCor(pv >= 0 ? COR_VERDE : COR_VERMELHA);
      char str[4];
      snprintf(str, 4, "%d", abs(pv));
      gl::DesenhaStringAlinhadoDireita(str);
    }
  }
}

void Tabuleiro::DesenhaListaJogadores() {
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  int raster_x = 0, raster_y = 0;
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  // Pula o FPS.
  raster_y = altura_ - ((altura_fonte * escala) * 2 + 4);
  raster_x = 2;
  if (!parametros_desenho_.has_picking_x()) {
    PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
  }
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  MudaCor(COR_BRANCA);
  if (!parametros_desenho_.has_picking_x()) {
    std::string titulo("Lista Jogadores");
    gl::DesenhaStringAlinhadoEsquerda(titulo);
  }
  raster_y -= (altura_fonte + 2);
  // Lista de objetos.
  for (const auto& par : clientes_) {
    if (!parametros_desenho_.has_picking_x()) {
      PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
    }
    char rotulo[101];
    snprintf(rotulo, 100, "%u->%s%s", par.first, par.second.c_str(),
             mestres_secundarios_.find(par.first) == mestres_secundarios_.end() ? "" : " (M)");
    gl::TipoEscopo tipo(OBJ_CONTROLE_VIRTUAL);
    try {
      gl::CarregaNome(CONTROLE_JOGADORES + par.first);
    } catch (...) {
      continue;
    }
    {
      gl::MatrizEscopo salva(GL_PROJECTION);
      gl::CarregaIdentidade();
      if (parametros_desenho_.has_picking_x()) {
        gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
      }
      gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
      gl::MatrizEscopo salva_2(GL_MODELVIEW);
      gl::CarregaIdentidade(false);
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x, raster_y, raster_x + (strlen(rotulo) * largura_fonte), raster_y + altura_fonte);
    }
    MudaCor(COR_AZUL);
    if (!parametros_desenho_.has_picking_x()) {
      gl::DesenhaStringAlinhadoEsquerda(rotulo);
    }
    raster_y -= (altura_fonte + 2);
  }
}

void Tabuleiro::DesenhaListaObjetos() {
  std::vector<const Entidade*> entidades_cenario;
  for (const auto& it : entidades_) {
    const auto* e = it.second.get();
    if (e->IdCenario() != cenario_corrente_) {
      continue;
    }
    entidades_cenario.push_back(e);
  }

  const int n_objetos = entidades_cenario.size();
  const int objs_por_pagina = 10;
  const int num_paginas = (n_objetos / objs_por_pagina) + ((n_objetos % objs_por_pagina > 0) ? 1 : 0);
  const int pagina_corrente = (pagina_lista_objetos_ >= num_paginas) ? num_paginas : pagina_lista_objetos_;
  const int objeto_inicial = pagina_corrente * objs_por_pagina;
  const int objeto_final = ((pagina_corrente == num_paginas - 1) || (num_paginas == 0)) ?
      n_objetos : objeto_inicial + objs_por_pagina;  // exclui do ultimo.
  if (pagina_lista_objetos_ > num_paginas - 1) {
    pagina_lista_objetos_ = std::max(num_paginas - 1, 0);
  }

  // Modo 2d: eixo com origem embaixo esquerda.
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  int raster_x = 0, raster_y = 0;
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  raster_y = altura_ - (altura_fonte * escala);
  raster_x = largura_ - 2;
  if (!parametros_desenho_.has_picking_x()) {
    PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
  }
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  MudaCor(COR_BRANCA);
  if (!parametros_desenho_.has_picking_x()) {
    std::string titulo("Lista Objetos");
    gl::DesenhaStringAlinhadoDireita(titulo);
  }
  raster_y -= (altura_fonte + 2);
  // Paginacao inicial.
  if (pagina_corrente > 0) {
    gl::TipoEscopo tipo(OBJ_CONTROLE_VIRTUAL);
    gl::CarregaNome(CONTROLE_PAGINACAO_LISTA_OBJETOS_CIMA);
    {
      gl::MatrizEscopo salva(GL_PROJECTION);
      gl::CarregaIdentidade();
      if (parametros_desenho_.has_picking_x()) {
        gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
      }
      gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
      gl::MatrizEscopo salva_2(GL_MODELVIEW);
      gl::CarregaIdentidade(false);
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x - (3 * largura_fonte), raster_y, raster_x, raster_y + altura_fonte);
    }
    if (!parametros_desenho_.has_picking_x()) {
      MudaCor(COR_AZUL);
      PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
      std::string page_up("^^^");
      gl::DesenhaStringAlinhadoDireita(page_up);
    }
  }
  // Pula independente de ter paginacao pra ficar fixa a posicao dos objetos.
  raster_y -= (altura_fonte + 2);

  // Lista de objetos.
  for (int i = objeto_inicial; i < objeto_final; ++i) {
    const auto* e = entidades_cenario[i];
    if (!parametros_desenho_.has_picking_x()) {
      PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
    }
    char rotulo[10];
    snprintf(rotulo, 10, "%s%s", e->Proto().has_rotulo() ? ":" : "", e->Proto().rotulo().c_str());
    char str[100];
    snprintf(str, 100, "%d%s->%s:%s",
             e->Id(), rotulo,
             TipoEntidade_Name(e->Proto().tipo()).c_str(),
             e->Proto().tipo() == TE_FORMA ? TipoForma_Name(e->Proto().sub_tipo()).c_str() : "-");
    gl::TipoEscopo tipo(OBJ_ENTIDADE_LISTA);
    try {
      gl::CarregaNome(e->Id());
    } catch (...) {
      continue;
    }
    {
      gl::MatrizEscopo salva(GL_PROJECTION);
      gl::CarregaIdentidade();
      if (parametros_desenho_.has_picking_x()) {
        gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
      }
      gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
      gl::MatrizEscopo salva_2(GL_MODELVIEW);
      gl::CarregaIdentidade(false);
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x - (strlen(str) * largura_fonte), raster_y, raster_x, raster_y + altura_fonte);
    }
    MudaCor(COR_AZUL);
    if (!parametros_desenho_.has_picking_x()) {
      gl::DesenhaStringAlinhadoDireita(str);
    }
    raster_y -= (altura_fonte + 2);
  }

  // Paginacao final.
  if (pagina_corrente < (num_paginas - 1)) {
    gl::TipoEscopo tipo(OBJ_CONTROLE_VIRTUAL);
    gl::CarregaNome(CONTROLE_PAGINACAO_LISTA_OBJETOS_BAIXO);
    {
      gl::MatrizEscopo salva(GL_PROJECTION);
      gl::CarregaIdentidade();
      if (parametros_desenho_.has_picking_x()) {
        gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
      }
      gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
      gl::MatrizEscopo salva_2(GL_MODELVIEW);
      gl::CarregaIdentidade(false);
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x - (3 * largura_fonte), raster_y, raster_x, raster_y + altura_fonte);
    }
    if (!parametros_desenho_.has_picking_x()) {
      MudaCor(COR_AZUL);
      PosicionaRaster2d(raster_x, raster_y, largura_, altura_);
      std::string page_down("vvv");
      gl::DesenhaStringAlinhadoDireita(page_down);
    }
    raster_y -= (altura_fonte + 2);
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
      id_acao.assign(entidade->Acao(AcoesPadroes()));
      achou = true;
    } else if (id_acao != entidade->Acao(AcoesPadroes())) {
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
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  PosicionaRaster2d(largura_ / 2, altura_ - altura_fonte, largura_, altura_);
  MudaCor(COR_BRANCA);
  gl::DesenhaString(id_acao);
}

void Tabuleiro::DesenhaCoordenadas() {
  if (!VisaoMestre() || (estado_ != ETAB_QUAD_PRESSIONADO && estado_ != ETAB_QUAD_SELECIONADO)) {
    return;
  }
  char coordenadas[101] = { '\0' };
  float x, y, z;
  CoordenadaQuadrado(quadrado_selecionado_, &x, &y, &z);
  char descricao[51] = { '\0' };
  if (!proto_corrente_->descricao_cenario().empty()) {
    snprintf(descricao, 50, " (%s)", proto_corrente_->descricao_cenario().c_str());
  }
  snprintf(coordenadas, 100, "cenario: %d%s, x: %.1f, y: %.1f, z: %.1f", proto_corrente_->id_cenario(), descricao, x, y, z);

  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  // Modo 2d: eixo com origem embaixo esquerda.
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  gl::CarregaIdentidade();
  {
    gl::MatrizEscopo salva_matriz(GL_PROJECTION);
    gl::CarregaIdentidade();
    gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
    int largura_fonte, altura_fonte, escala;
    gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
    largura_fonte *= escala;
    altura_fonte *= escala;
    int raster_y = altura_ - (2 * altura_fonte);
    int raster_x = largura_ / 2;
    gl::PosicaoRaster(raster_x, raster_y);
  }

  {
    MudaCor(COR_BRANCA);
    gl::DesenhaString(coordenadas);
  }
}

// Chamado pelo TimerEscopo no tabuleiro.h.
void Tabuleiro::DesenhaTempoRenderizacao() {
  auto passou_ms = timer_.elapsed().wall / 1000000ULL;
  if (tempos_renderizacao_.size() == kMaximoTamTemposRenderizacao) {
    tempos_renderizacao_.pop_back();
  }
  tempos_renderizacao_.push_front(passou_ms);
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
  gl::DesligaEscritaProfundidadeEscopo profundidade_escopo;
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(largura_, altura_, &largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;

  std::string tempo_str = net::to_string(maior_tempo_ms);
  while (tempo_str.size() < 4) {
    tempo_str.insert(0, "0");
  }

  // Modo 2d.
  {
    gl::MatrizEscopo salva_matriz_pr(GL_PROJECTION);
    gl::CarregaIdentidade();
    gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
    MudaCor(COR_PRETA);
    gl::MatrizEscopo salva_matriz_mv(GL_MODELVIEW);
    gl::CarregaIdentidade();
    gl::Retangulo(0.0f, altura_ - altura_fonte - 2.0f, tempo_str.size() * largura_fonte + 2.0f, altura_);
  }
  // Eixo com origem embaixo esquerda.
  PosicionaRaster2d(2, altura_ - altura_fonte - 2, largura_, altura_);
  MudaCor(COR_BRANCA);
  gl::DesenhaStringAlinhadoEsquerda(tempo_str);
  V_ERRO("tempo de renderizacao");
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
  int cenario_origem =  (entidade_origem != nullptr) ? entidade_origem->IdCenario() : cenario_corrente_;
  std::vector<const Entidade*> entidades_cenario;
  for (const auto& id_entidade_destino : entidades_) {
    auto* entidade = id_entidade_destino.second.get();
    if (entidade->IdCenario() == cenario_origem) {
      entidades_cenario.push_back(entidade);
    }
  }
  if (acao.tipo() == ACAO_DISPERSAO) {
    switch (acao.geometria()) {
      case ACAO_GEO_ESFERA: {
        const Posicao pos_para_computar = pos_destino.has_x() ? pos_destino : pos_tabuleiro;
        for (const auto* entidade_destino : entidades_cenario) {
          Posicao pos_entidade(entidade_destino->Pos());
          pos_entidade.set_z(pos_entidade.z() + entidade_destino->Z());
          float d2 = DistanciaQuadrado(pos_para_computar, pos_entidade);
          if (d2 <= powf(acao.raio_area() * TAMANHO_LADO_QUADRADO, 2)) {
            VLOG(1) << "Adicionando id: " << entidade_destino->Id();
            ids_afetados.push_back(entidade_destino->Id());
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
        for (const auto* entidade_destino : entidades_cenario) {
          if (PontoDentroDePoligono(entidade_destino->Pos(), vertices)) {
            VLOG(1) << "Adicionando id: " << entidade_destino->Id();
            ids_afetados.push_back(entidade_destino->Id());
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
    for (const auto* entidade_destino : entidades_cenario) {
      if (entidade_destino != entidade_origem && PontoDentroDePoligono(entidade_destino->Pos(), vertices)) {
        VLOG(1) << "Adicionando id: " << entidade_destino->Id();
        ids_afetados.push_back(entidade_destino->Id());
      }
    }
  } else {
    LOG(WARNING) << "Tipo de acao nao reconhecido: " << acao.tipo();
  }

  return ids_afetados;
}

Entidade* Tabuleiro::EntidadePrimeiraPessoaOuSelecionada() {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(id_camera_presa_);
  } else {
    return EntidadeSelecionada();
  }
}

const Entidade* Tabuleiro::EntidadePrimeiraPessoaOuSelecionada() const {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(id_camera_presa_);
  } else {
    return EntidadeSelecionada();
  }
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

const Entidade* Tabuleiro::EntidadeSelecionada() const {
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

std::vector<const Entidade*> Tabuleiro::EntidadesSelecionadas() const {
  std::vector<const Entidade*> entidades;
  for (const auto& id : ids_entidades_selecionadas_) {
    const Entidade* e = nullptr;
    if ((e = BuscaEntidade(id)) == nullptr) {
      continue;
    }
    entidades.push_back(e);
  }
  return entidades;
}

void Tabuleiro::AlternaModoDebug() {
  gl::AlternaModoDebug();
  //modo_debug_ = !modo_debug_;
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
  if (!EmModoMestre(true)) {
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
    for (const auto& evento_antes : entidade->Proto().evento()) {
      int rodadas = evento_antes.rodadas();
      if (rodadas > 0) {
        --rodadas;
      }
      auto* evento_depois = proto_depois.add_evento();
      *evento_depois = evento_antes;
      evento_depois->set_rodadas(rodadas);
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
  if (!EmModoMestre(true)) {
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
  if (modo_clique_ == MODO_ACAO) {
    modo_clique_ = MODO_NORMAL;
  } else {
    modo_clique_ = MODO_ACAO;
  }
}

void Tabuleiro::AlternaModoTransicao() {
  if (modo_clique_ == MODO_TRANSICAO) {
    modo_clique_ = MODO_NORMAL;
  } else {
    modo_clique_ = MODO_TRANSICAO;
  }
}

void Tabuleiro::AlternaModoRegua() {
  if (modo_clique_ == MODO_REGUA) {
    modo_clique_ = MODO_NORMAL;
  } else {
    modo_clique_ = MODO_REGUA;
  }
}

void Tabuleiro::AlternaModoTerreno() {
  if (!EmModoMestre(true  /*secundario*/)) {
    return;
  }
  if (modo_clique_ == MODO_TERRENO) {
    modo_clique_ = MODO_NORMAL;
  } else {
    modo_clique_ = MODO_TERRENO;
  }
}

void Tabuleiro::EntraModoClique(modo_clique_e modo) {
  central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_REFRESCAR_MENU));
  if (modo_clique_ == MODO_ROTACAO && modo != MODO_ROTACAO) {
    // A rotacao eh diferente pq eh sem clique.
    estado_ = estado_anterior_;
  }
  modo_clique_ = modo;
}

void Tabuleiro::SalvaCameraInicial() {
  proto_.mutable_camera_inicial()->CopyFrom(olho_);
  proto_.mutable_camera_inicial()->mutable_pos()->set_id_cenario(proto_corrente_->id_cenario());
  // Destino é para movimento.
  proto_.mutable_camera_inicial()->clear_destino();
}

void Tabuleiro::ReiniciaCamera() {
  // Vou ser conservador aqui e voltar para a camera de perspectiva. Caso contrario, posso correr o risco de um jogador
  // ficar preso em uma entidade que nao eh a dele (por exemplo, carregando o tabuleiro sem manter as entidades, a entidade
  // presa deixa de existir).
  camera_ = CAMERA_PERSPECTIVA;
  camera_presa_ = false;
  if (proto_.has_camera_inicial()) {
    if (proto_.camera_inicial().pos().has_id_cenario() && proto_.camera_inicial().pos().id_cenario() != proto_corrente_->id_cenario()) {
      CarregaSubCenario(proto_.camera_inicial().pos().id_cenario(), proto_.camera_inicial().alvo());
    }
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
  AtualizaOlho(0, true  /*forcar*/);
}

void Tabuleiro::ReiniciaIluminacao(TabuleiroProto* sub_cenario) {
  // Iluminacao ambiente inicial.
  sub_cenario->mutable_luz_ambiente()->set_r(0.5f);
  sub_cenario->mutable_luz_ambiente()->set_g(0.5f);
  sub_cenario->mutable_luz_ambiente()->set_b(0.5f);
  // Iluminacao direcional inicial.
  sub_cenario->mutable_luz_direcional()->mutable_cor()->set_r(0.5f);
  sub_cenario->mutable_luz_direcional()->mutable_cor()->set_g(0.5f);
  sub_cenario->mutable_luz_direcional()->mutable_cor()->set_b(0.5f);
  // Vinda de 45 graus leste.
  sub_cenario->mutable_luz_direcional()->set_posicao_graus(0.0f);
  sub_cenario->mutable_luz_direcional()->set_inclinacao_graus(45.0f);
}

void Tabuleiro::AlternaCameraIsometrica() {
  if (camera_ == CAMERA_ISOMETRICA) {
    camera_ = CAMERA_PERSPECTIVA;
  } else {
    camera_= CAMERA_ISOMETRICA;
  }
}

void Tabuleiro::AlternaCameraPrimeiraPessoa() {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    LOG(INFO) << "Camera perspectiva";
    camera_ = CAMERA_PERSPECTIVA;
  } else {
    if (!camera_presa_) {
      AlternaCameraPresa();
    }
    if (!camera_presa_) {
      return;
    }
    camera_= CAMERA_PRIMEIRA_PESSOA;
    LOG(INFO) << "Camera primeira pessoa";
  }
  AtualizaOlho(0, true);
}

void Tabuleiro::AlternaCameraPresa() {
  if (camera_presa_) {
    camera_presa_ = false;
    id_camera_presa_ = Entidade::IdInvalido;
    LOG(INFO) << "Camera solta.";
    if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
      AlternaCameraPrimeiraPessoa();
    }
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
  if (!EmModoMestre()) {
    return;
  }
  watchdog_.Para();
#endif
}

void Tabuleiro::ReativaWatchdog() {
#if USAR_WATCHDOG
  if (!EmModoMestre()) {
    return;
  }
  watchdog_.Reinicia();
#endif
}

float Tabuleiro::ZChao(float x, float y) const {
  if (proto_corrente_->ponto_terreno_size() == 0) {
    return 0.0f;
  }
  return Terreno::ZChao(x, y, TamanhoX(), TamanhoY(), proto_corrente_->ponto_terreno().data());
}

float Tabuleiro::AlturaPonto(int x_quad, int y_quad) const {
  if (proto_corrente_->ponto_terreno_size() == 0) {
    return 0.0f;
  }
  try {
    return Terreno::AlturaPonto(x_quad, y_quad, TamanhoX(), TamanhoY(), proto_corrente_->ponto_terreno().data());
  } catch (...) {
    return 0.0f;
  }
}

}  // namespace ent
