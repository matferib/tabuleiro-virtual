#if USAR_QT
#include <QApplication>
#include <QClipboard>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/text_format.h>
#else
#endif
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

#if USAR_OPENGL_ES
#define USAR_MAPEAMENTO_SOMBRAS_OPENGLES 1
#else
#define USAR_MAPEAMENTO_SOMBRAS_OPENGLES 0
#endif

#if USAR_MAPEAMENTO_SOMBRAS_OPENGLES
#ifndef GL_TEXTURE_COMPARE_MODE_EXT
// Estes dois nao estao presentes no jni android 17 (4.4.2).
// Como o lollipop da pau de rede, prefiro fazer isso.
#define GL_TEXTURE_COMPARE_MODE_EXT 0x884C
#define GL_COMPARE_REF_TO_TEXTURE_EXT 0x884E
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif
#endif

#define TAM_MAPA_OCLUSAO 1024
#define APENAS_MESTRE_CRIA_FORMAS 0

#define ORDENAR_ENTIDADES 0

using google::protobuf::RepeatedField;

namespace ent {

namespace {

/** expessura da linha da grade do tabuleiro. */
const float EXPESSURA_LINHA = 0.1f;
const float EXPESSURA_LINHA_2 = EXPESSURA_LINHA / 2.0f;
/** velocidade do olho. */
const float VELOCIDADE_OLHO_M_S = TAMANHO_LADO_QUADRADO * 10.0f;

/** tamanho maximo da lista de eventos para desfazer. */
const unsigned int TAMANHO_MAXIMO_LISTA = 50;

/** Diferencas menores que essa farao o tabuleiro usar o dado mais preciso. */
const float PRECISAO_APOIO = 0.3f;
/** Verifica a profundidade ate este maximo de distancia. */
const float MAXIMA_PROFUNDIDADE_PARA_VERIFICACAO = 10 * TAMANHO_LADO_QUADRADO;

// Os offsets servem para evitar zfight. Eles adicionam à profundidade um valor
// dz * escala + r * unidades, onde dz eh grande dependendo do angulo do poligono em relacao
// a camera e r eh o menor offset que gera diferenca no zbuffer.
// Valores positivos afastam, negativos aproximam.
const float OFFSET_TERRENO_ESCALA_DZ = 1.0f;
const float OFFSET_TERRENO_ESCALA_R  = 2.0f;
const float OFFSET_GRADE_ESCALA_DZ   = 0.5f;
const float OFFSET_GRADE_ESCALA_R    = 1.0f;

const std::string StringEstado(ent::etab_t estado) {
  switch (estado) {
    case ent::ETAB_OCIOSO:
      return "ETAB_OCIOSO";
    case ent::ETAB_ROTACAO:
      return "ETAB_ROTACAO";
    case ent::ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA:
      return "ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA";
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

void SalvaConfiguracoes(const OpcoesProto& proto) {
  try {
    arq::EscreveArquivoAsciiProto(arq::TIPO_CONFIGURACOES, "configuracoes.asciiproto", proto);
    LOG(INFO) << "Salvando opcoes em arquivo.";
  } catch (const std::logic_error& e) {
    LOG(ERROR) << "Falha salvando opcoes: " << e.what();
  }
}

// Usado pelas funcoes de timer para enfileiras os tempos.
void EnfileiraTempo(const boost::timer::cpu_timer& timer, std::list<uint64_t>* tempos) {
  constexpr static unsigned int kMaximoTamTemposRenderizacao = 10;
  auto passou_ms = timer.elapsed().wall / 1000000ULL;
  if (tempos->size() == kMaximoTamTemposRenderizacao) {
    tempos->pop_back();
  }
  tempos->push_front(passou_ms);
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
               << ", translacoes_rotacoes_escalas_antes_.size() == " << translacoes_rotacoes_escalas_antes_.size()
               << ", lista_eventos_.size() == " << lista_eventos_.size()
               << ", processando_grupo_: " << processando_grupo_;

    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_SERIALIZAR_TABULEIRO);
    notificacao.set_endereco("dinamico://tabuleiro_watchdog.binproto");
    this->TrataNotificacao(notificacao);
  });
#endif
}

Tabuleiro::~Tabuleiro() {
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

Tabuleiro::DadosFramebuffer::~DadosFramebuffer() {
  gl::ApagaFramebuffers(1, &framebuffer);
  gl::ApagaTexturas(1, &textura);
  gl::ApagaRenderbuffers(1, &renderbuffer);
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
  temporizador_detalhamento_ms_ = 0;
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
  // Modo de acao.
  modo_clique_ = MODO_NORMAL;
  // Lista objetos.
  pagina_lista_objetos_ = 0;
  info_geral_.clear();

  // iniciativa.
  indice_iniciativa_ = -1;
  iniciativas_.clear();

  if (reiniciar_grafico) {
    LiberaTextura();
    IniciaGL();
    // Atencao V_ERRO so pode ser usado com contexto grafico.
    V_ERRO("estado inicial pos grafico");
  }
}

void Tabuleiro::EscreveInfoGeral(const std::string& info_geral) {
  info_geral_ = info_geral;
  temporizador_info_geral_ms_ = TEMPO_DETALHAMENTO_MS;
}

void Tabuleiro::ConfiguraProjecaoMapeamentoSombras() {
  float val = std::max(TamanhoX(), TamanhoY()) * TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO;
  gl::Ortogonal(-val, val, -val, val,
                0.0 /*DISTANCIA_PLANO_CORTE_PROXIMO*/, 200.0f);
  gl::AtualizaMatrizes();
}

void Tabuleiro::ConfiguraProjecaoMapeamentoOclusaoLuzes() {
  gl::Perspectiva(90.0f, 1.0f,
                  DISTANCIA_PLANO_CORTE_PROXIMO_PRIMEIRA_PESSOA,
                  DISTANCIA_PLANO_CORTE_DISTANTE);
  gl::PlanoDistanteOclusao(DISTANCIA_PLANO_CORTE_DISTANTE);
  gl::AtualizaMatrizes();
}

void Tabuleiro::ConfiguraProjecao() {
  if (parametros_desenho_.projecao().tipo_camera() == CAMERA_ISOMETRICA) {
    const Posicao& alvo = olho_.alvo();
    // o tamanho do vetor
    float dif_z = alvo.z() - olho_.pos().z();
    float distancia_min = parametros_desenho_.projecao().plano_corte_proximo_m();
    float distancia_max = parametros_desenho_.projecao().has_plano_corte_distante_m()
        ? parametros_desenho_.projecao().plano_corte_distante_m() : TAMANHO_LADO_QUADRADO + fabs(dif_z);
    const float largura = parametros_desenho_.projecao().has_largura_m()
        ? parametros_desenho_.projecao().largura_m() / 2.0f : distancia_max * Aspecto() / 2.0f;
    const float altura = parametros_desenho_.projecao().has_altura_m()
        ? parametros_desenho_.projecao().altura_m() / 2.0f : distancia_max / 2.0f;
    //LOG_EVERY_N(INFO, 30) << "Distancia: " << distancia_max;
    gl::Ortogonal(-largura, largura, -altura, altura,
                  distancia_min, distancia_max * 1.2f);  // 1.2 necessario?
  } else {
    gl::Perspectiva(parametros_desenho_.projecao().has_campo_visao_vertical_graus()
                        ? parametros_desenho_.projecao().campo_visao_vertical_graus()
                        : camera_ == CAMERA_PRIMEIRA_PESSOA ? angulo_visao_vertical_graus_ : CAMPO_VISAO_PADRAO,
                    parametros_desenho_.projecao().has_razao_aspecto()
                        ? parametros_desenho_.projecao().razao_aspecto() : Aspecto(),
                    parametros_desenho_.projecao().tipo_camera() == CAMERA_PRIMEIRA_PESSOA
                        ? DISTANCIA_PLANO_CORTE_PROXIMO_PRIMEIRA_PESSOA : DISTANCIA_PLANO_CORTE_PROXIMO,
                    parametros_desenho_.projecao().has_plano_corte_distante_m()
                        ? parametros_desenho_.projecao().plano_corte_distante_m() : DISTANCIA_PLANO_CORTE_DISTANTE);
  }
  gl::AtualizaMatrizes();
}

void Tabuleiro::ConfiguraOlhar() {
  // Desenho normal, tem que configurar as matrizes de sombra e oclusao.
  if (!parametros_desenho_.desenha_mapa_sombras() &&
      !parametros_desenho_.has_desenha_mapa_oclusao() &&
      !parametros_desenho_.has_desenha_mapa_luzes()) {
    // Mapa de sombras.
    if (MapeamentoSombras() && !parametros_desenho_.has_picking_x()) {
      gl::MudaModoMatriz(gl::MATRIZ_SOMBRA);
      ConfiguraOlharMapeamentoSombras();
      gl::MudaModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
    }
    if (MapeamentoOclusao() && !parametros_desenho_.has_picking_x()) {
      gl::MudaModoMatriz(gl::MATRIZ_OCLUSAO);
      ConfiguraOlharMapeamentoOclusao();
      gl::MudaModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
    }
    if (!parametros_desenho_.has_picking_x()) {
      gl::MudaModoMatriz(gl::MATRIZ_LUZ);
      ConfiguraOlharMapeamentoLuzes();
      gl::MudaModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
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
      Vector3 up;
      if (olho_.pos().x() != alvo.x() || olho_.pos().y() != alvo.y()) {
        up.z = 1.0f;
      } else {
        up.y = 1.0f;
      }
      gl::OlharPara(
          // from.
          olho_.pos().x(), olho_.pos().y(), olho_.pos().z(),
          // to.
          alvo.x(), alvo.y(), alvo.z(),
          // up
          up.x, up.y, up.z);
    }
    return;
  }

  if (MapeamentoSombras() && parametros_desenho_.desenha_mapa_sombras()) {
    ConfiguraOlharMapeamentoSombras();
    return;
  }
  if (MapeamentoOclusao() && parametros_desenho_.has_desenha_mapa_oclusao()) {
    ConfiguraOlharMapeamentoOclusao();
    return;
  }
  if (parametros_desenho_.has_desenha_mapa_luzes()) {
    ConfiguraOlharMapeamentoLuzes();
    return;
  }
}

void Tabuleiro::ConfiguraOlharMapeamentoSombras() {
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
}

void Tabuleiro::ConfiguraOlharMapeamentoOclusao() {
  const auto* entidade_referencia = BuscaEntidade(IdCameraPresa());
  if (entidade_referencia == nullptr) {
    LOG(ERROR) << "nao ha entidade de referencia!";
    return;
  }
  const auto& pos = entidade_referencia->Pos();
  float altura_olho = entidade_referencia->ZOlho();
  Vector3 delta_alvo;
  Vector3 up;
  int face = parametros_desenho_.desenha_mapa_oclusao();
  if (!parametros_desenho_.has_desenha_mapa_oclusao()) {
    delta_alvo.x = 1.0f;
    // No desenho normal, a gente nao inverte o vetor vertical.
    up.z = 1.0f;
    face = -1;
  }
  // No desenho da textura, o eixo vertical é invertido por causa da orientação do opengl.
  switch (face) {
    case 0:  // GL_TEXTURE_CUBE_MAP_POSITIVE_X, right
      delta_alvo.y = -1.0f;
      up.z = -1.0f;
      break;
    case 1:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left
      delta_alvo.y = 1.0f;
      up.z = -1.0f;
      break;
    case 2:  // GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top
      delta_alvo.z = 1.0f;
      up.x = -1.0f;
      break;
    case 3:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom
      delta_alvo.z = -1.0f;
      up.x = 1.0f;
      break;
    case 4:  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back
      delta_alvo.x = -1.0f;
      up.z = -1.0f;
      break;
    case 5:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front
      delta_alvo.x = 1.0f;
      up.z = -1.0f;
      break;
    default:
      //LOG(ERROR) << "Face do mapa de oclusao invalida";
      ;
  }
  gl::OlharPara(
      // from.
      pos.x(), pos.y(), altura_olho,
      // to.
      pos.x() + delta_alvo.x, pos.y() + delta_alvo.y, altura_olho + delta_alvo.z,
      // up
      up.x, up.y, up.z);
}

void Tabuleiro::ConfiguraOlharMapeamentoLuzes() {
  if (luzes_pontuais_.empty()) {
    return;
  }
  const auto& pos = luzes_pontuais_[0].pos;
  Vector3 delta_alvo;
  Vector3 up;
  int face = parametros_desenho_.desenha_mapa_luzes();
  if (!parametros_desenho_.has_desenha_mapa_luzes()) {
    delta_alvo.x = 1.0f;
    // No desenho normal, a gente nao inverte o vetor vertical.
    up.z = 1.0f;
    face = -1;
  }
  // No desenho da textura, o eixo vertical é invertido por causa da orientação do opengl.
  switch (face) {
    case 0:  // GL_TEXTURE_CUBE_MAP_POSITIVE_X, right
      delta_alvo.y = -1.0f;
      up.z = -1.0f;
      break;
    case 1:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left
      delta_alvo.y = 1.0f;
      up.z = -1.0f;
      break;
    case 2:  // GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top
      delta_alvo.z = 1.0f;
      up.x = -1.0f;
      break;
    case 3:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom
      delta_alvo.z = -1.0f;
      up.x = 1.0f;
      break;
    case 4:  // GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back
      delta_alvo.x = -1.0f;
      up.z = -1.0f;
      break;
    case 5:  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front
      delta_alvo.x = 1.0f;
      up.z = -1.0f;
      break;
    default:
      //LOG(ERROR) << "Face do mapa de luzes invalida";
      ;
  }
  gl::OlharPara(
      // from.
      pos.x(), pos.y(), pos.z(),
      // to.
      pos.x() + delta_alvo.x, pos.y() + delta_alvo.y, pos.z() + delta_alvo.z,
      // up
      up.x, up.y, up.z);
}

void Tabuleiro::DesenhaMapaOclusao() {
  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  // Zera as coisas nao usadas durante oclusao.
  parametros_desenho_.set_desenha_acoes(false);
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_transparencias(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_iniciativas(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_info_geral(false);
  parametros_desenho_.set_desenha_detalhes(false);
  parametros_desenho_.set_desenha_eventos_entidades(false);
  parametros_desenho_.set_desenha_efeitos_entidades(false);
  parametros_desenho_.set_nao_desenha_entidades_translucidas(true);  // nao devem afetar o mapa oclusao.
  parametros_desenho_.set_nao_desenha_entidades_selecionaveis(true);  // nao devem afetar o mapa oclusao.
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
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  parametros_desenho_.set_desenha_controle_virtual(false);
  parametros_desenho_.set_desenha_pontos_rolagem(false);
  parametros_desenho_.mutable_projecao()->set_tipo_camera(CAMERA_PERSPECTIVA);

  gl::UsaShader(gl::TSH_PONTUAL);

  gl::UnidadeTextura(GL_TEXTURE3);
  gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, 0);
  gl::Viewport(0, 0, TAM_MAPA_OCLUSAO, TAM_MAPA_OCLUSAO);
  parametros_desenho_.set_desenha_mapa_oclusao(0);
  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  ConfiguraProjecaoMapeamentoOclusaoLuzes();
  //LOG(INFO) << "DesenhaMapaOclusao";
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb_oclusao_.framebuffer);

  V_ERRO("LigacaoComFramebufferOclusao");

  for (int i = 0; i < 6; ++i) {
    parametros_desenho_.set_desenha_mapa_oclusao(i);
#if ORDENAR_ENTIDADES
    parametros_desenho_.set_ordena_entidades_apos_configura_olhar(i == 0);
#endif
    gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dfb_oclusao_.textura, 0);
    V_ERRO("TexturaFramebufferOclusao");
#if VBO_COM_MODELAGEM
    DesenhaCenaVbos();
#else
    DesenhaCena();
#endif
  }

  V_ERRO("LigacaoComFramebufferOclusao");
}

void Tabuleiro::DesenhaMapaLuz() {
  if (luzes_pontuais_.empty()) {
    return;
  }

  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  // Zera as coisas nao usadas durante luzes.
  parametros_desenho_.set_desenha_acoes(false);
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_transparencias(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_iniciativas(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_info_geral(false);
  parametros_desenho_.set_desenha_detalhes(false);
  parametros_desenho_.set_desenha_eventos_entidades(false);
  parametros_desenho_.set_desenha_efeitos_entidades(false);
  parametros_desenho_.set_nao_desenha_entidades_translucidas(true);  // nao devem afetar o mapa oclusao.
  parametros_desenho_.set_nao_desenha_entidades_selecionaveis(true);  // nao devem afetar o mapa oclusao.
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
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  parametros_desenho_.set_desenha_controle_virtual(false);
  parametros_desenho_.set_desenha_pontos_rolagem(false);
  parametros_desenho_.mutable_projecao()->set_tipo_camera(CAMERA_PERSPECTIVA);

  gl::UsaShader(gl::TSH_PONTUAL);

  gl::UnidadeTextura(GL_TEXTURE4);
  gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, 0);
  gl::Viewport(0, 0, TAM_MAPA_OCLUSAO, TAM_MAPA_OCLUSAO);
  parametros_desenho_.set_desenha_mapa_luzes(0);
  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  ConfiguraProjecaoMapeamentoOclusaoLuzes();
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb_luzes_[0].framebuffer);

  V_ERRO("LigacaoComFramebufferOclusao");

  for (int i = 0; i < 6; ++i) {
    //LOG(INFO) << "DesenhaMapaLuzes " << i;
    parametros_desenho_.set_desenha_mapa_luzes(i);
#if ORDENAR_ENTIDADES
    parametros_desenho_.set_ordena_entidades_apos_configura_olhar(i == 0);
#endif
    gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dfb_luzes_[0].textura, 0);
    V_ERRO("TexturaFramebufferOclusao");
#if VBO_COM_MODELAGEM
    DesenhaCenaVbos();
#else
    DesenhaCena();
#endif
  }
  V_ERRO("LigacaoComFramebufferOclusao");
}

void Tabuleiro::DesenhaMapaSombra() {
  if (!parametros_desenho_.desenha_sombras()) {
    return;
  }
  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  // Zera as coisas nao usadas na sombra.
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_transparencias(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_iniciativas(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_info_geral(false);
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
  parametros_desenho_.set_desenha_mapa_sombras(true);
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_modo_mestre(VisaoMestre());
  parametros_desenho_.set_desenha_controle_virtual(false);
  parametros_desenho_.set_desenha_pontos_rolagem(false);
  parametros_desenho_.mutable_projecao()->set_tipo_camera(CAMERA_ISOMETRICA);
#if ORDENAR_ENTIDADES
  parametros_desenho_.set_ordena_entidades_apos_configura_olhar(true);
#endif

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
  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  ConfiguraProjecaoMapeamentoSombras();
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb_luz_direcional_.framebuffer);
  V_ERRO("LigacaoComFramebufferSombraProjetada");
#if !USAR_MAPEAMENTO_SOMBRAS_OPENGLES
  gl::BufferDesenho(GL_NONE);
#endif
  //LOG(INFO) << "sombra projetada";
#if VBO_COM_MODELAGEM
  DesenhaCenaVbos();
#else
  DesenhaCena();
#endif
}

int Tabuleiro::Desenha() {
  V_ERRO_RET("InicioDesenha");

#if DEBUG
  glFinish();
#endif
  timer_uma_renderizacao_completa_.start();

  // Varios lugares chamam desenha cena com parametros especifico. Essa funcao
  // desenha a cena padrao, entao ela restaura os parametros para seus valores
  // default. Alem disso a matriz de projecao eh diferente para picking.
  parametros_desenho_.Clear();
  parametros_desenho_.set_tipo_visao(VISAO_NORMAL);
  gl::TipoShader tipo_shader;
  auto* entidade_referencia = BuscaEntidade(IdCameraPresa());
  if (entidade_referencia != nullptr && entidade_referencia->Proto().tipo_visao() == VISAO_ESCURO && visao_escuro_ &&
      (!VisaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    parametros_desenho_.set_tipo_visao(entidade_referencia->Proto().tipo_visao());
    parametros_desenho_.set_desenha_sombras(false);
    parametros_desenho_.set_desenha_mapa_sombras(false);
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
  parametros_desenho_.set_desenha_iniciativas(opcoes_.mostra_iniciativas());
  parametros_desenho_.set_desenha_fps(opcoes_.mostra_fps());
  parametros_desenho_.set_desenha_log_eventos(opcoes_.mostra_log_eventos());
  parametros_desenho_.set_desenha_grade(opcoes_.desenha_grade());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());
  parametros_desenho_.mutable_projecao()->set_tipo_camera(camera_);
  if (camera_ == CAMERA_ISOMETRICA) {
    const Posicao& alvo = olho_.alvo();
    float dif_z = alvo.z() - olho_.pos().z();
    float distancia_max =  TAMANHO_LADO_QUADRADO + fabs(dif_z);
    auto* proj = parametros_desenho_.mutable_projecao();
    proj->set_plano_corte_proximo_m(0.1f);
    proj->set_plano_corte_distante_m(distancia_max);
    proj->set_largura_m(distancia_max * Aspecto() / 2.0f);
    proj->set_altura_m(distancia_max / 2.0f);
  }
  // Verifica o angulo em relacao ao tabuleiro para decidir se as texturas ficarao viradas para cima.
  if (camera_ == CAMERA_ISOMETRICA ||
      (camera_ != CAMERA_PRIMEIRA_PESSOA && (olho_.altura() > (2 * olho_.raio())))) {
    parametros_desenho_.set_desenha_texturas_para_cima(true);
  } else {
    parametros_desenho_.set_desenha_texturas_para_cima(false);
  }

  if (modo_debug_) {
    tipo_shader = gl::TSH_PICKING;
    parametros_desenho_.set_iluminacao(false);
    parametros_desenho_.set_desenha_texturas(false);
    parametros_desenho_.set_desenha_grade(false);
    parametros_desenho_.set_desenha_fps(false);
    parametros_desenho_.set_desenha_aura(false);
    parametros_desenho_.set_desenha_sombras(false);
    parametros_desenho_.set_desenha_mapa_sombras(false);
    parametros_desenho_.clear_desenha_mapa_oclusao();
    parametros_desenho_.clear_desenha_mapa_luzes();
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
  V_ERRO_RET("Antes desenha sombras");

#if VBO_COM_MODELAGEM
  GeraVbosCena();
#endif

#if !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    // Se estiver ligado, desliga aqui para desenhar mapas.
    // Colocando dentro do if para evitar erro de opengl com plataformas que nao suportam GL_MULTISAMPLE.
    gl::Desabilita(GL_MULTISAMPLE);
  }
#endif

#if DEBUG
  glFinish();
#endif
  timer_renderizacao_mapas_.start();
  if (MapeamentoOclusao() && !modo_debug_) {
    GLint original;
    gl::Le(GL_FRAMEBUFFER_BINDING, &original);
    ParametrosDesenho salva_pd(parametros_desenho_);
    DesenhaMapaOclusao();
    parametros_desenho_.set_desenha_mapa_oclusao(0);
    V_ERRO_RET("Depois Mapa Oclusao");
    // Restaura os valores e usa a textura como mapa de oclusao.
    // Note que ao mudar o shader, o valor do plano distante de corte para oclusao permanecera o mesmo.
    gl::UsaShader(tipo_shader);
    gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
    gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
    gl::UnidadeTextura(GL_TEXTURE3);
    gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, dfb_oclusao_.textura);
    gl::UnidadeTextura(GL_TEXTURE0);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
    parametros_desenho_ = salva_pd;
  }

  if (parametros_desenho_.desenha_sombras() && !modo_debug_) {
    GLint original;
    gl::Le(GL_FRAMEBUFFER_BINDING, &original);
    ParametrosDesenho salva_pd(parametros_desenho_);
    DesenhaMapaSombra();

    V_ERRO_RET("Depois DesenhaMapaSombra");
    // Restaura os valores e usa a textura como sombra.
    gl::UsaShader(tipo_shader);
    gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
    // Desloca os componentes xyz do espaco [-1,1] para [0,1] que eh o formato armazenado no mapa de sombras.
    gl::MudaModoMatriz(gl::MATRIZ_PROJECAO_SOMBRA);
    gl::CarregaIdentidade();
    Matrix4 bias(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0);
    gl::MultiplicaMatriz(bias.get());
    ConfiguraProjecaoMapeamentoSombras();  // antes de parametros_desenho_.set_desenha_mapa_sombras para configurar para luz.
    gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
    gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
    gl::UnidadeTextura(GL_TEXTURE1);
    gl::LigacaoComTextura(GL_TEXTURE_2D, dfb_luz_direcional_.textura);
    gl::UnidadeTextura(GL_TEXTURE0);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
    parametros_desenho_ = salva_pd;
  } else {
    gl::UsaShader(tipo_shader);
    gl::UnidadeTextura(GL_TEXTURE0);
  }
#if DEBUG
  glFinish();
#endif
  timer_renderizacao_mapas_.stop();
  EnfileiraTempo(timer_renderizacao_mapas_, &tempos_renderizacao_mapas_);

#if !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    gl::Habilita(GL_MULTISAMPLE);
  } else {
    gl::Desabilita(GL_MULTISAMPLE);
  }
#endif

#if !USAR_MAPEAMENTO_SOMBRAS_OPENGLES
  gl::BufferDesenho(GL_BACK);
#endif
  V_ERRO_RET("MeioDesenha");
  gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  ConfiguraProjecao();
  //LOG(INFO) << "Desenha sombra: " << parametros_desenho_.desenha_sombras();
  //DesenhaCenaVbos();
#if ORDENAR_ENTIDADES
  parametros_desenho_.set_ordena_entidades_apos_configura_olhar(true);
#endif
  DesenhaCena();
  EnfileiraTempo(timer_entre_cenas_, &tempos_entre_cenas_);
#if DEBUG
  glFinish();
#endif
  timer_entre_cenas_.start();
  V_ERRO_RET("FimDesenha");
  timer_uma_renderizacao_completa_.stop();
  EnfileiraTempo(timer_uma_renderizacao_completa_, &tempos_uma_renderizacao_completa_);
  return tempos_entre_cenas_.front();
}

void Tabuleiro::AdicionaEntidadeNotificando(const ntf::Notificacao& notificacao) {
  try {
    if (notificacao.local()) {
      EntidadeProto modelo(notificacao.has_entidade() ? notificacao.entidade() : *modelo_selecionado_.second);
#if APENAS_MESTRE_CRIA_FORMAS
      if (modelo.tipo() == TE_FORMA && !EmModoMestreIncluindoSecundario()) {
        LOG(ERROR) << "Apenas o mestre pode adicionar formas.";
        return;
      }
#endif
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
        modelo.set_visivel(!EmModoMestreIncluindoSecundario());
        modelo.set_selecionavel_para_jogador(!EmModoMestreIncluindoSecundario());
        modelo.set_id(id_entidade);
      } else {
        if (BuscaEntidade(modelo.id()) != nullptr) {
          // Este caso eh raro, mas talvez possa acontecer quando estiver perto do limite de entidades.
          // Isso tem potencial de erro caso o mestre remova entidade de jogadores.
          throw std::logic_error("Id da entidade já está sendo usado.");
        }
      }
      auto* entidade = NovaEntidade(modelo, texturas_, m3d_, central_, &parametros_desenho_);
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
      // Selecao: queremos selecionar entidades criadas ou coladas, mas apenas quando nao estiver tratando comando de desfazer.
      if (!Desfazendo()) {
        // Se a entidade selecionada for TE_ENTIDADE e a entidade adicionada for FORMA, deseleciona a entidade.
        DeselecionaEntidades();
#if 0
        // Esse comportamento nao deseleciona outras formas.
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
#endif
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
      auto* entidade = NovaEntidade(notificacao.entidade(), texturas_, m3d_, central_, &parametros_desenho_);
      entidades_.insert(std::make_pair(entidade->Id(), std::unique_ptr<Entidade>(entidade)));
    }
  } catch (const std::logic_error& erro) {
    auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
    n->set_erro(erro.what());
    central_->AdicionaNotificacao(n);
    return;
  }
}

void Tabuleiro::AtualizaBitsEntidadeNotificando(int bits, bool valor) {
  if (estado_ != ETAB_ENTS_SELECIONADAS) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  bool atualizar_mapa_luzes = false;
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* n = grupo_notificacoes.add_notificacao();
    auto* entidade_selecionada = BuscaEntidade(id);
    const auto& proto_original = entidade_selecionada->Proto();
    // Para desfazer.
    auto* proto_antes = n->mutable_entidade_antes();
    auto* proto_depois = n->mutable_entidade();
    if ((bits & BIT_VISIBILIDADE) > 0 &&
        (EmModoMestreIncluindoSecundario() || proto_original.selecionavel_para_jogador())) {
      // Apenas modo mestre ou para selecionaveis.
      proto_antes->set_visivel(proto_original.visivel());
      proto_depois->set_visivel(valor);
      if (proto_original.tipo() != TE_ENTIDADE) {
        atualizar_mapa_luzes = true;
      }
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
    if (bits & BIT_SURPRESO) {
      proto_antes->set_surpreso(proto_original.surpreso());
      proto_depois->set_surpreso(valor);
    }
    if (bits & BIT_FURTIVO) {
      proto_antes->set_furtivo(proto_original.furtivo());
      proto_depois->set_furtivo(valor);
    }
    if (bits & BIT_ATAQUE_MAIS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_1(proto_original.dados_ataque_globais().ataque_mais_1());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_1(valor);
    }
    if (bits & BIT_ATAQUE_MAIS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_2(proto_original.dados_ataque_globais().ataque_mais_2());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_2(valor);
    }
    if (bits & BIT_ATAQUE_MAIS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_4(proto_original.dados_ataque_globais().ataque_mais_4());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_4(valor);
    }
    if (bits & BIT_ATAQUE_MAIS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_8(proto_original.dados_ataque_globais().ataque_mais_8());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_8(valor);
    }
    if (bits & BIT_ATAQUE_MENOS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_1(proto_original.dados_ataque_globais().ataque_menos_1());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_1(valor);
    }
    if (bits & BIT_ATAQUE_MENOS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_2(proto_original.dados_ataque_globais().ataque_menos_2());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_2(valor);
    }
    if (bits & BIT_ATAQUE_MENOS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_4(proto_original.dados_ataque_globais().ataque_menos_4());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_4(valor);
    }
    if (bits & BIT_ATAQUE_MENOS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_8(proto_original.dados_ataque_globais().ataque_menos_8());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_8(valor);
    }

    if (bits & BIT_DANO_MAIS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_1(proto_original.dados_ataque_globais().dano_mais_1());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_1(valor);
    }
    if (bits & BIT_DANO_MAIS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_2(proto_original.dados_ataque_globais().dano_mais_2());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_2(valor);
    }
    if (bits & BIT_DANO_MAIS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_4(proto_original.dados_ataque_globais().dano_mais_4());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_4(valor);
    }
    if (bits & BIT_DANO_MAIS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_8(proto_original.dados_ataque_globais().dano_mais_8());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_8(valor);
    }
    if (bits & BIT_DANO_MAIS_16) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_16(proto_original.dados_ataque_globais().dano_mais_16());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_16(valor);
    }
    if (bits & BIT_DANO_MAIS_32) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_32(proto_original.dados_ataque_globais().dano_mais_32());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_32(valor);
    }
    if (bits & BIT_DANO_MENOS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_1(proto_original.dados_ataque_globais().dano_menos_1());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_1(valor);
    }
    if (bits & BIT_DANO_MENOS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_2(proto_original.dados_ataque_globais().dano_menos_2());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_2(valor);
    }
    if (bits & BIT_DANO_MENOS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_4(proto_original.dados_ataque_globais().dano_menos_4());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_4(valor);
    }
    if (bits & BIT_DANO_MENOS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_8(proto_original.dados_ataque_globais().dano_menos_8());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_8(valor);
    }
    proto_antes->set_id(id);
    proto_depois->set_id(id);
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
  if (atualizar_mapa_luzes) {
    luzes_pontuais_.clear();
  }
}

void Tabuleiro::AlternaBitsEntidadeNotificando(int bits) {
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  bool atualizar_mapa_luzes = false;
  for (unsigned int id : IdsEntidadesSelecionadasOuPrimeiraPessoa()) {
    auto* n = grupo_notificacoes.add_notificacao();
    auto* entidade_selecionada = BuscaEntidade(id);
    const auto& proto_original = entidade_selecionada->Proto();
    // Para desfazer.
    auto* proto_antes = n->mutable_entidade_antes();
    auto* proto_depois = n->mutable_entidade();
    if ((bits & BIT_VISIBILIDADE) > 0 &&
        (EmModoMestreIncluindoSecundario() || proto_original.selecionavel_para_jogador())) {
      // Apenas modo mestre ou para selecionaveis.
      proto_antes->set_visivel(proto_original.visivel());
      proto_depois->set_visivel(!proto_original.visivel());
      if (proto_original.tipo() != TE_ENTIDADE) {
        atualizar_mapa_luzes = true;
      }
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
      if (proto_antes->voadora()) {
        // Detecta colisao vertical.
        Vector3 vetor_movimento;
        float z_olho = entidade_selecionada->ZOlho();
        float tam_movimento = z_olho - entidade_selecionada->ZAntesVoo();
        VLOG(1) << "z_olho: " << z_olho << ", z_antes_voo: " << entidade_selecionada->ZAntesVoo();
        vetor_movimento.z = -tam_movimento;
        // como movimento eh vertical, ignora o espaco da entidade.
        auto res_colisao = DetectaColisao(*entidade_selecionada, vetor_movimento, true  /*ignora_espaco*/);
        VLOG(1) << "tam_movimento: " << tam_movimento << ", colisao: " << res_colisao.profundidade << ", vetor: " << vetor_movimento;
        if (res_colisao.profundidade < tam_movimento) {
          float novo_z = z_olho - res_colisao.profundidade;
          VLOG(1) << "colisao, novo_z: " << novo_z << ", antes: " << entidade_selecionada->ZAntesVoo();
          proto_depois->set_z_antes_voo(novo_z);
          //entidade_selecionada->AtribuiZAntesVoo(novo_z);
        }
      } else {
        proto_depois->set_z_antes_voo(entidade_selecionada->Z(true));
      }
      proto_antes->set_z_antes_voo(entidade_selecionada->ZAntesVoo());
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
    if (bits & BIT_SURPRESO) {
      proto_antes->set_surpreso(proto_original.surpreso());
      proto_depois->set_surpreso(!proto_original.surpreso());
    }
    if (bits & BIT_FURTIVO) {
      proto_antes->set_furtivo(proto_original.furtivo());
      proto_depois->set_furtivo(!proto_original.furtivo());
    }
    if (bits & BIT_ATAQUE_MAIS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_1(proto_original.dados_ataque_globais().ataque_mais_1());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_1(!proto_original.dados_ataque_globais().ataque_mais_1());
    }
    if (bits & BIT_ATAQUE_MAIS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_2(proto_original.dados_ataque_globais().ataque_mais_2());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_2(!proto_original.dados_ataque_globais().ataque_mais_2());
    }
    if (bits & BIT_ATAQUE_MAIS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_4(proto_original.dados_ataque_globais().ataque_mais_4());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_4(!proto_original.dados_ataque_globais().ataque_mais_4());
    }
    if (bits & BIT_ATAQUE_MAIS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_mais_8(proto_original.dados_ataque_globais().ataque_mais_8());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_mais_8(!proto_original.dados_ataque_globais().ataque_mais_8());
    }

    if (bits & BIT_ATAQUE_MENOS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_1(proto_original.dados_ataque_globais().ataque_menos_1());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_1(!proto_original.dados_ataque_globais().ataque_menos_1());
    }
    if (bits & BIT_ATAQUE_MENOS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_2(proto_original.dados_ataque_globais().ataque_menos_2());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_2(!proto_original.dados_ataque_globais().ataque_menos_2());
    }
    if (bits & BIT_ATAQUE_MENOS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_4(proto_original.dados_ataque_globais().ataque_menos_4());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_4(!proto_original.dados_ataque_globais().ataque_menos_4());
    }
    if (bits & BIT_ATAQUE_MENOS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_ataque_menos_8(proto_original.dados_ataque_globais().ataque_menos_8());
      proto_depois->mutable_dados_ataque_globais()->set_ataque_menos_8(!proto_original.dados_ataque_globais().ataque_menos_8());
    }

    if (bits & BIT_DANO_MAIS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_1(proto_original.dados_ataque_globais().dano_mais_1());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_1(!proto_original.dados_ataque_globais().dano_mais_1());
    }
    if (bits & BIT_DANO_MAIS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_2(proto_original.dados_ataque_globais().dano_mais_2());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_2(!proto_original.dados_ataque_globais().dano_mais_2());
    }
    if (bits & BIT_DANO_MAIS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_4(proto_original.dados_ataque_globais().dano_mais_4());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_4(!proto_original.dados_ataque_globais().dano_mais_4());
    }
    if (bits & BIT_DANO_MAIS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_8(proto_original.dados_ataque_globais().dano_mais_8());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_8(!proto_original.dados_ataque_globais().dano_mais_8());
    }
    if (bits & BIT_DANO_MAIS_16) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_16(proto_original.dados_ataque_globais().dano_mais_16());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_16(!proto_original.dados_ataque_globais().dano_mais_16());
    }
    if (bits & BIT_DANO_MAIS_32) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_mais_32(proto_original.dados_ataque_globais().dano_mais_32());
      proto_depois->mutable_dados_ataque_globais()->set_dano_mais_32(!proto_original.dados_ataque_globais().dano_mais_32());
    }

    if (bits & BIT_DANO_MENOS_1) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_1(proto_original.dados_ataque_globais().dano_menos_1());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_1(!proto_original.dados_ataque_globais().dano_menos_1());
    }
    if (bits & BIT_DANO_MENOS_2) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_2(proto_original.dados_ataque_globais().dano_menos_2());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_2(!proto_original.dados_ataque_globais().dano_menos_2());
    }
    if (bits & BIT_DANO_MENOS_4) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_4(proto_original.dados_ataque_globais().dano_menos_4());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_4(!proto_original.dados_ataque_globais().dano_menos_4());
    }
    if (bits & BIT_DANO_MENOS_8) {
      proto_antes->mutable_dados_ataque_globais()->set_dano_menos_8(proto_original.dados_ataque_globais().dano_menos_8());
      proto_depois->mutable_dados_ataque_globais()->set_dano_menos_8(!proto_original.dados_ataque_globais().dano_menos_8());
    }

    proto_antes->set_id(id);
    proto_depois->set_id(id);
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
  }
  if (grupo_notificacoes.notificacao_size() == 0) {
    VLOG(1) << "Não há entidade selecionada.";
    return;
  }
  TrataNotificacao(grupo_notificacoes);
  // Para desfazer.
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
  if (atualizar_mapa_luzes) {
    luzes_pontuais_.clear();
  }
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
    PreencheNotificacaoAtualizaoPontosVida(*entidade_selecionada,
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
  PreencheNotificacaoAtualizaoPontosVida(*entidade, delta_pontos_vida, &n);
  TrataNotificacao(n);

  // Acao de pontos de vida sem efeito.
  if (!acao.Proto().texto().empty()) {
    ntf::Notificacao na;
    na.set_tipo(ntf::TN_ADICIONAR_ACAO);
    auto* a = na.mutable_acao();
    a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
    a->add_id_entidade_destino(entidade->Id());
    a->set_afeta_pontos_vida(false);
    a->set_texto(acao.Proto().texto());
    TrataNotificacao(na);
  }

  ntf::Notificacao na;
  na.set_tipo(ntf::TN_ADICIONAR_ACAO);
  auto* a = na.mutable_acao();
  a->set_tipo(ACAO_DELTA_PONTOS_VIDA);
  a->add_id_entidade_destino(entidade->Id());
  a->set_delta_pontos_vida(delta_pontos_vida);
  a->set_afeta_pontos_vida(false);
  if (!acao.Proto().texto().empty()) {
    a->set_atraso_s(0.5f);
  }
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

void Tabuleiro::AcumulaPontosVida(const std::vector<std::pair<int, std::string>>& lista_pv) {
  for (const auto& sinal_valor : lista_pv) {
    lista_pontos_vida_.emplace_back(sinal_valor);
  }
}

void Tabuleiro::LimpaListaPontosVida() {
  lista_pontos_vida_.clear();
}

void Tabuleiro::AlteraUltimoPontoVidaListaPontosVida(int delta) {
  if (!lista_pontos_vida_.empty()) {
    AtualizaStringDadosVida(delta, &lista_pontos_vida_.back().second);
  } else {
    lista_pontos_vida_.push_back({ -1  /*padrao: dano*/, net::to_string(delta)});
  }
  modo_clique_ = MODO_ACAO;
}

void Tabuleiro::AlternaUltimoPontoVidaListaPontosVida() {
  if (!lista_pontos_vida_.empty()) {
    lista_pontos_vida_.back().first *= -1;
  }
  modo_clique_ = MODO_ACAO;
}

int Tabuleiro::LeValorListaPontosVida(const Entidade* entidade, const std::string& id_acao) {
  if (modo_dano_automatico_) {
    if (entidade == nullptr) {
      LOG(WARNING) << "entidade eh nula";
      return 0;
    }
    int delta_pontos_vida;
    std::string texto_pontos_vida;
    std::tie(delta_pontos_vida, texto_pontos_vida) = entidade->ValorParaAcao(id_acao);
    delta_pontos_vida = -delta_pontos_vida;
    VLOG(1) << "Lendo valor automatico de dano para entidade, acao: " << id_acao << ", delta: " << delta_pontos_vida;
    AdicionaLogEvento(std::string("entidade ") +
                      (entidade->Proto().rotulo().empty() ? net::to_string(entidade->Id()) : entidade->Proto().rotulo()) +
                      ": " + texto_pontos_vida);
    return delta_pontos_vida;
  } else {
    int delta_pontos_vida;
    std::vector<std::pair<int, int>> dados;
    std::tie(delta_pontos_vida, dados) = GeraPontosVida(lista_pontos_vida_.front().second);
    AdicionaLogEvento(std::string("resultado de ") + lista_pontos_vida_.front().second + ": " +
                      ent::DadosParaString(delta_pontos_vida, dados));
    delta_pontos_vida *= lista_pontos_vida_.front().first;
    lista_pontos_vida_.pop_front();

    VLOG(1) << "Lendo valor da lista de pontos de vida: " << delta_pontos_vida;
    return delta_pontos_vida;
  }
}

int Tabuleiro::LeValorAtaqueFurtivo(const Entidade* entidade) {
  if (modo_dano_automatico_) {
    if (entidade == nullptr) {
      LOG(WARNING) << "entidade eh nula";
      return 0;
    }
    if (!entidade->Proto().furtivo() || entidade->Proto().dados_ataque_globais().dano_furtivo().empty()) {
      return 0;
    }
    int total;
    std::vector<std::pair<int, int>> dados;
    std::tie(total, dados) = GeraPontosVida(entidade->Proto().dados_ataque_globais().dano_furtivo());
    total = -total;
    std::string texto_dados;
    for (const auto& fv : dados) {
      texto_dados += std::string("d") + net::to_string(fv.first) + "=" + net::to_string(fv.second) + ", ";
    }
    LOG(INFO) << "valor dos dados para furtivo. Total: " << total << ", dados: " << texto_dados;
  }
  return 0;
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
    case ntf::TN_ENVIAR_TEXTURAS: {
      // Cliente recebendo texturas de servidor.
      if (notificacao.local()) {
        return false;
      }
      // Controle virtual tem que apagar as texturas dos botoes pois ele cacheia os ids.
      IniciaGlControleVirtual();
      return true;
    }
    case ntf::TN_PROXIMA_INICIATIVA: {
      if (notificacao.entidade().id() == IniciativaCorrente()) {
        ProximaIniciativa();
      }
      return true;
    }
    case ntf::TN_ATUALIZAR_LISTA_INICIATIVA: {
      AtualizaIniciativaNotificando(notificacao);
      return true;
    }
    case ntf::TN_ENTRAR_MODO_SELECAO_TRANSICAO: {
      const Entidade* e = BuscaEntidade(notificacao.entidade().id());
      if (e == nullptr || e->TipoTransicao() != EntidadeProto::TRANS_CENARIO) {
        return true;
      }
      int id_cenario = e->TransicaoCenario();
      if (id_cenario != cenario_corrente_) {
        CarregaSubCenario(id_cenario, e->PosTransicao());
      }
      EntraModoClique(MODO_SELECAO_TRANSICAO);
      notificacao_selecao_transicao_ = notificacao;
      return true;
    }
    case ntf::TN_CONECTAR: {
      AlterarModoMestre(false);
      opcoes_.set_ultimo_endereco(notificacao.endereco());
      SalvaConfiguracoes(opcoes_);
      return true;
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
      if (notificacao.tabuleiro().has_camera_inicial()) {
        ReiniciaCamera(notificacao);
      } else {
        ReiniciaCamera();
      }
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
      // quanto passou desde a ultima atualizacao. Usa o tempo entre cenas pois este timer eh do da atualizacao.
      auto passou_ms = timer_entre_atualizacoes_.elapsed().wall / 1000000ULL;
#if DEBUG
      glFinish();
#endif
      timer_entre_atualizacoes_.start();
      timer_uma_atualizacao_.start();
      if (regerar_vbos_entidades_) {
        parametros_desenho_.set_regera_vbo(true);
      }

      AtualizaEntidades(passou_ms);
      AtualizaLuzesPontuais();

      if (indice_iniciativa_ != -1 && EmModoMestre()) {
        AtualizaIniciativas();
      }
      // Em algumas situacoes, nao se deve atualizar o olho. Por exemplo, quando se esta pressionando entidades para mover,
      // ao move-la, o olho ira se atualizar e o ponto de destino mudara, assim como as matrizes.
      if (estado_ != ETAB_ENTS_PRESSIONADAS && estado_ != ETAB_DESLIZANDO) {
        AtualizaOlho(passou_ms, false  /*forcar*/);
      }
      AtualizaAcoes(passou_ms);
#if DEBUG
      glFinish();
#endif
      timer_uma_atualizacao_.stop();
      EnfileiraTempo(timer_uma_atualizacao_, &tempos_uma_atualizacao_);
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
      if (temporizador_info_geral_ms_ > 0) {
        temporizador_info_geral_ms_ -= passou_ms;
      } else if (!info_geral_.empty()) {
        info_geral_.clear();
      }
#if USAR_WATCHDOG
      if (EmModoMestre()) {
        watchdog_.Refresca();
      }
#endif
      if (regerar_vbos_entidades_) {
        parametros_desenho_.clear_regera_vbo();
        regerar_vbos_entidades_ = false;
      }

      //auto at_passou = timer_temp.elapsed().wall / 1000000ULL;
      //LOG(INFO) << "Passou " << (int)at_passou << " atualizando";
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
      Entidade* entidade = EntidadePrimeiraPessoaOuSelecionada();
      if (entidade == nullptr) {
        auto* ne = ntf::NovaNotificacao(ntf::TN_ERRO);
        ne->set_erro("Deve haver uma entidade (e apenas uma) selecionada.");
        central_->AdicionaNotificacao(ne);
        return true;
      }
      auto* n = new ntf::Notificacao(notificacao);
      central_->AdicionaNotificacao(n);
      n->set_modo_mestre(EmModoMestreIncluindoSecundario());
      n->mutable_tabuleiro()->set_id_cliente(id_cliente_);
      n->mutable_entidade()->CopyFrom(entidade->Proto());
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
    case ntf::TN_GERAR_MONTANHA: {
      GeraMontanhaNotificando();
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
  } else if (estado_ == ETAB_ENTS_TRANSLACAO_ROTACAO || estado_ == ETAB_ESCALANDO_ROTACIONANDO_ENTIDADE_PINCA) {
    for (const auto& id_proto : translacoes_rotacoes_escalas_antes_) {
      auto* e = BuscaEntidade(id_proto.first);
      if (e == nullptr) {
        continue;
      }
      // Atualiza clientes quando delta passar de algum valor.
      auto* nr = ntf::NovaNotificacao(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
      nr->mutable_entidade()->set_id(e->Id());
      nr->mutable_entidade()->set_rotacao_z_graus(e->RotacaoZGraus());
      *nr->mutable_entidade()->mutable_escala() = e->Proto().escala();
      *nr->mutable_entidade()->mutable_pos() = e->Pos();
      central_->AdicionaNotificacaoRemota(nr);
    }
  }
}

void Tabuleiro::RefrescaTerrenoParaClientes() {
  central_->AdicionaNotificacaoRemota(SerializaRelevoCenario());
}

void Tabuleiro::LimpaIniciativasNotificando() {
  std::vector<const Entidade*> entidades;
  if (EmModoMestreIncluindoSecundario()) {
    if (estado_ == ETAB_ENTS_SELECIONADAS) {
      entidades = EntidadesSelecionadas();
    } else {
      for (const auto& di : iniciativas_) {
        const auto* entidade = BuscaEntidade(di.id);
        if (entidade != nullptr) {
          entidades.push_back(entidade);
        }
      }
    }
  } else {
    if (estado_ == ETAB_ENTS_SELECIONADAS) {
      entidades = EntidadesSelecionadas();
    } else if (IdCameraPresa() != Entidade::IdInvalido) {
      for (const unsigned int id : ids_camera_presa_) {
        auto* entidade = BuscaEntidade(id);
        if (entidade != nullptr) {
          entidades.push_back(entidade);
        }
      }
    } else {
      LOG(INFO) << "Nao ha unidade selecionada ou presa para limpar iniciativas";
      return;
    }
  }

  // desfazer.
  ntf::Notificacao grupo_rotulo;
  grupo_rotulo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (const auto* entidade : entidades) {
    if (entidade->Tipo() != TE_ENTIDADE) {
      continue;
    }
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    auto* e_antes = n->mutable_entidade_antes();
    e_antes->set_id(entidade->Id());
    if (entidade->TemIniciativa()) {
      e_antes->set_iniciativa(entidade->Iniciativa());
    } else {
      e_antes->set_iniciativa(INICIATIVA_INVALIDA);
    }
    // TODO notificar e desfazer.
    auto* e_depois = n->mutable_entidade();
    e_depois->set_id(entidade->Id());
    e_depois->set_iniciativa(INICIATIVA_INVALIDA);
    TrataNotificacao(*n);
  }
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
  TrataNotificacao(grupo_rotulo);
}

void Tabuleiro::RolaIniciativasNotificando() {
  std::vector<const Entidade*> entidades;
  if (EmModoMestreIncluindoSecundario()) {
    entidades = EntidadesSelecionadas();
  } else {
    if (estado_ == ETAB_ENTS_SELECIONADAS) {
      entidades = EntidadesSelecionadas();
    } else if (IdCameraPresa() != Entidade::IdInvalido) {
      for (const unsigned int id : ids_camera_presa_) {
        const auto* entidade = BuscaEntidade(id);
        if (entidade != nullptr) {
          entidades.push_back(entidade);
        }
      }
    } else {
      LOG(INFO) << "Nao ha unidade selecionada ou presa para rolar iniciativa";
      return;
    }
  }

  // desfazer.
  ntf::Notificacao grupo_rotulo;
  grupo_rotulo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  float atraso_rotulo_s = 0.0f;
  ntf::Notificacao grupo_notificacoes;
  grupo_notificacoes.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  for (const auto* entidade : entidades) {
    if (entidade->Tipo() != TE_ENTIDADE) {
      continue;
    }
    auto* n = grupo_notificacoes.add_notificacao();
    n->set_tipo(ntf::TN_ATUALIZAR_PARCIAL_ENTIDADE);
    auto* e_antes = n->mutable_entidade_antes();
    e_antes->set_id(entidade->Id());
    if (entidade->TemIniciativa()) {
      e_antes->set_iniciativa(entidade->Iniciativa());
    } else {
      e_antes->set_iniciativa(INICIATIVA_INVALIDA);
    }
    // TODO notificar e desfazer.
    int d20 = RolaDado(20);
    int iniciativa = d20 + entidade->ModificadorIniciativa();
    auto* e_depois = n->mutable_entidade();
    e_depois->set_id(entidade->Id());
    e_depois->set_iniciativa(iniciativa);
    TrataNotificacao(*n);
    {
      auto* n_rotulo = grupo_rotulo.add_notificacao();
      n_rotulo->set_tipo(ntf::TN_ADICIONAR_ACAO);
      auto* acao = n_rotulo->mutable_acao();
      acao->set_tipo(ACAO_DELTA_PONTOS_VIDA);
      acao->add_id_entidade_destino(entidade->Id());
      char texto[20] = {'\0'};
      snprintf(texto, 19, "%d+%d= %d", d20, entidade->ModificadorIniciativa(), iniciativa);
      acao->set_texto(texto);
      acao->set_atraso_s(atraso_rotulo_s);
      atraso_rotulo_s += 0.5f;
      if (EmModoMestreIncluindoSecundario() && !entidade->SelecionavelParaJogador()) {
        acao->set_local_apenas(true);
      }
    }
  }
  AdicionaNotificacaoListaEventos(grupo_notificacoes);
  TrataNotificacao(grupo_rotulo);
}

void Tabuleiro::IniciaIniciativaParaCombate() {
  if (!EmModoMestreIncluindoSecundario()) {
    LOG(INFO) << "Apenas mestre pode iniciar as iniciativas para combate.";
    return;
  }
  // TODO desfazer.
  std::vector<const Entidade*> entidades_com_iniciativa;
  if (indice_iniciativa_ == -1) {
    for (auto& id_ent : entidades_) {
      auto* entidade = id_ent.second.get();
      if (entidade->TemIniciativa() && !entidade->Proto().morta()) {
        entidades_com_iniciativa.push_back(entidade);
      }
    }
  }

  // Ordena as entidades por iniciativa.
  std::sort(entidades_com_iniciativa.begin(), entidades_com_iniciativa.end(), [this] (const Entidade* entidade1, const Entidade* entidade2) {
    return
      entidade1->Iniciativa() > entidade2->Iniciativa() ||
      (entidade1->Iniciativa() == entidade2->Iniciativa() && entidade1->ModificadorIniciativa() >  entidade2->ModificadorIniciativa());
  });
  ntf::Notificacao n;
  n.set_tipo(ntf::TN_ATUALIZAR_LISTA_INICIATIVA);
  SerializaIniciativas(n.mutable_tabuleiro_antes());
  {
    auto* tabuleiro = n.mutable_tabuleiro();
    for (const auto* entidade : entidades_com_iniciativa) {
      auto* e = tabuleiro->add_entidade();
      e->set_id(entidade->Id());
      e->set_iniciativa(entidade->Iniciativa());
      e->set_modificador_iniciativa(entidade->ModificadorIniciativa());
    }
    tabuleiro->set_indice_iniciativa(indice_iniciativa_ == -1 ? 0 : -1);
  }
  AdicionaNotificacaoListaEventos(n);
  TrataNotificacao(n);
}

void Tabuleiro::AtualizaIniciativaNotificando(const ntf::Notificacao& notificacao) {
  iniciativas_.clear();
  iniciativas_.reserve(notificacao.tabuleiro().entidade_size());
  for (const auto& entidade : notificacao.tabuleiro().entidade()) {
    DadosIniciativa dados;
    dados.id = entidade.id();
    dados.iniciativa = entidade.iniciativa();
    dados.modificador = entidade.modificador_iniciativa();
    iniciativas_.push_back(dados);
  }
  indice_iniciativa_ = notificacao.tabuleiro().indice_iniciativa();
  // Repassa aos outros.
  if (notificacao.local()) {
    central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
  }
}

void Tabuleiro::ProximaIniciativa() {
  if (indice_iniciativa_ == -1) {
    LOG(INFO) << "Nao eh mestre ou entidades_ordenadas_por_iniciativa_.empty";
    return;
  }
  if (!EmModoMestreIncluindoSecundario()) {
    // So permite ao jogador passar se for a vez dele.
    unsigned int id_iniciativa = IniciativaCorrente();
    if (!IdPresoACamera(id_iniciativa)) {
      LOG(INFO) << "Jogador so pode passar sua propria iniciativa.";
      return;
    }
    // Envia requisicao pro mestre passar a vez.
    auto* n = ntf::NovaNotificacao(ntf::TN_PROXIMA_INICIATIVA);
    n->set_servidor_apenas(true);
    n->mutable_entidade()->set_id(id_iniciativa);
    central_->AdicionaNotificacaoRemota(n);
    return;
  }

  ntf::Notificacao grupo;
  grupo.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);
  auto* n = grupo.add_notificacao();
  n->set_tipo(ntf::TN_ATUALIZAR_LISTA_INICIATIVA);
  SerializaIniciativas(n->mutable_tabuleiro_antes());
  SerializaIniciativas(n->mutable_tabuleiro());
  n->mutable_tabuleiro()->set_indice_iniciativa(indice_iniciativa_ + 1);
  if (indice_iniciativa_ + 1 >= (int)iniciativas_.size()) {
    n->mutable_tabuleiro()->set_indice_iniciativa(0);
    PassaUmaRodadaNotificando(&grupo);
  }
  AdicionaNotificacaoListaEventos(grupo);
  TrataNotificacao(grupo);
}

void Tabuleiro::SerializaIniciativas(TabuleiroProto* tabuleiro) const {
  for (const DadosIniciativa& di : iniciativas_) {
    SerializaIniciativaParaEntidade(di, tabuleiro->add_entidade());
  }
  tabuleiro->set_indice_iniciativa(indice_iniciativa_);
}

void Tabuleiro::SerializaIniciativaParaEntidade(const DadosIniciativa& di, EntidadeProto* e) const {
  e->set_id(di.id);
  e->set_iniciativa(di.iniciativa);
  e->set_modificador_iniciativa(di.modificador);
}

void Tabuleiro::AlteraAnguloVisao(float valor) {
  valor = std::max(valor, CAMPO_VISAO_MIN);
  valor = std::min(valor, CAMPO_VISAO_MAX);
  angulo_visao_vertical_graus_ = valor;
  EscreveInfoGeral(std::string("Fov: ") + net::to_string(angulo_visao_vertical_graus_));
}

void Tabuleiro::AlternaVisaoJogador() {
  visao_jogador_ = (visao_jogador_ > 0) ? 0 : 1;
}

void Tabuleiro::TrataBotaoAlternarIluminacaoMestre() {
  if (visao_jogador_ == 1) {
    visao_jogador_ = 2;
  } else if (visao_jogador_ == 2) {
    visao_jogador_ = 1;
  } else {
    opcoes_.set_iluminacao_mestre_igual_jogadores(!opcoes_.iluminacao_mestre_igual_jogadores());
  }
}

void Tabuleiro::TrataRedimensionaJanela(int largura, int altura) {
  gl::Viewport(0, 0, (GLint)largura, (GLint)altura);
  largura_ = largura;
  altura_ = altura;
}

void Tabuleiro::IniciaGL() {
#if !USAR_OPENGL_ES
#ifdef GL_PROGRAM_POINT_SIZE
  gl::Habilita(GL_PROGRAM_POINT_SIZE);  // deixa o shader decidir tamanho do ponto.
#else
  gl::Habilita(GL_VERTEX_PROGRAM_POINT_SIZE);  // deixa o shader decidir tamanho do ponto.
#endif
#endif
  gl::Desabilita(GL_DITHER);
  // Faz com que AMBIENTE e DIFFUSE sigam as cores.

  // Nao desenha as costas dos poligonos.
  gl::Habilita(GL_CULL_FACE);
  gl::FaceNula(GL_BACK);

  GeraVboCaixaCeu();
  GeraVboRosaDosVentos();

  RegeraVboTabuleiro();
  IniciaGlControleVirtual();
  GeraFramebuffer();
  Entidade::IniciaGl();
  regerar_vbos_entidades_ = true;

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

void Tabuleiro::SelecionaAcao(const std::string& id_acao, Entidade* entidade) {
  auto it = mapa_acoes_.find(id_acao);
  if (it == mapa_acoes_.end()) {
    LOG(ERROR) << "Id de acao inválido: " << id_acao;
    return;
  }
  entidade->AtualizaAcao(it->first);
}

void Tabuleiro::SelecionaAcao(const std::string& id_acao) {
  auto it = mapa_acoes_.find(id_acao);
  if (it == mapa_acoes_.end()) {
    LOG(ERROR) << "Id de acao inválido: " << id_acao;
    return;
  }

  for (auto id_selecionado : IdsEntidadesSelecionadasOuPrimeiraPessoa()) {
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
  Entidade* e = EntidadeSelecionadaOuPrimeiraPessoa();
  if (e == nullptr) {
    LOG(INFO) << "Nao selecionando acao pois ha 0 ou mais de uma entidade selecionada.";
    return;
  }
  std::string id_acao = e->AcaoExecutada(indice, AcoesPadroes());
  if (id_acao.empty()) {
    VLOG(1) << "Selecionando acao padrao pois id eh invalido para a entidade.";
    id_acao = AcaoPadrao(indice).id();
  }
  SelecionaAcao(id_acao, e);
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
  for (auto id_selecionado : IdsEntidadesSelecionadasOuPrimeiraPessoa()) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    if (entidade->ProximaAcao()) {
      // entidade possui acao, usa as dela.
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
  for (auto id_selecionado : IdsEntidadesSelecionadasOuPrimeiraPessoa()) {
    Entidade* entidade = BuscaEntidade(id_selecionado);
    if (entidade == nullptr) {
      continue;
    }
    if (entidade->AcaoAnterior()) {
      // entidade possui acao, usa as dela.
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
  if (!parametros_desenho_.desenha_mapa_sombras() &&
      !parametros_desenho_.has_desenha_mapa_oclusao() &&
      !parametros_desenho_.has_desenha_mapa_luzes() &&
      !parametros_desenho_.has_picking_x() &&
      (parametros_desenho_.tipo_visao() != VISAO_ESCURO) &&
       parametros_desenho_.projecao().tipo_camera() != CAMERA_ISOMETRICA) {
    desenhar_caixa_ceu = true;
  } else {
    bits_limpar |= GL_COLOR_BUFFER_BIT;
    if (parametros_desenho_.tipo_visao() == VISAO_ESCURO) {
      gl::CorLimpeza(0.0f, 0.0f, 0.0f, 1.0f);
    } else if (parametros_desenho_.has_desenha_mapa_oclusao() ||
               parametros_desenho_.has_desenha_mapa_luzes()) {
      gl::CorLimpeza(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
      gl::CorLimpeza(proto_corrente_->luz_ambiente().r(),
                     proto_corrente_->luz_ambiente().g(),
                     proto_corrente_->luz_ambiente().b(),
                     proto_corrente_->luz_ambiente().a());
    }
  }
  V_ERRO("AntesLimpa");
  gl::Limpa(bits_limpar);
  V_ERRO("Limpa");

  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  V_ERRO("desabilitando luzes");

  gl::MudaModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::CarregaIdentidade();
  ConfiguraOlhar();
#if ORDENAR_ENTIDADES
  if (parametros_desenho_.ordena_entidades_apos_configura_olhar()) {
    OrdenaEntidades();
  }
#endif

  if (!parametros_desenho_.has_pos_olho()) {
    *parametros_desenho_.mutable_pos_olho() = olho_.pos();
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

  if (MapeamentoOclusao() && !parametros_desenho_.has_desenha_mapa_oclusao()) {
    gl::Oclusao(true);
  } else {
    gl::Oclusao(false);
  }

  // Aqui podem ser desenhados objetos normalmente. Caso contrario, a caixa do ceu vai ferrar tudo.
  if (opcoes_.desenha_olho()) {
    DesenhaOlho();
  }
  V_ERRO("desenhando olho");

#if 0
  // Algumas verificacoes.
  GLint depth = 0;
  gl::Le(gl::MATRIZ_MODELAGEM_CAMERA_STACK_DEPTH, &depth);
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

  // desenha tabuleiro do sul para o norte por ultimo para economizar.
  if (parametros_desenho_.desenha_terreno()) {
    gl::TipoEscopo nomes_tabuleiro(OBJ_TABULEIRO);
    DesenhaTabuleiro();
    if (parametros_desenho_.desenha_grade() &&
        (proto_corrente_->desenha_grade() || (!VisaoMestre() && proto_corrente_->textura_mestre_apenas()))) {
      DesenhaGrade();
    }
    DesenhaQuadradoSelecionado();
  }
  V_ERRO("desenhando tabuleiro");

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
  if (parametros_desenho_.desenha_entidades() && !parametros_desenho_.nao_desenha_entidades_translucidas()) {
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

  if ((parametros_desenho_.desenha_mapa_sombras()) ||
      (MapeamentoOclusao() && parametros_desenho_.has_desenha_mapa_oclusao()) ||
       parametros_desenho_.has_desenha_mapa_luzes()) {
    return;
  }

  //-------------
  // DESENHOS 2D.
  //-------------
#if 0
  // Nao funciona mais porque o tipo da textura eh sombra, provavelmente.
  if (MapeamentoSombras() && !parametros_desenho_.has_picking_x()) {
    gl::MatrizEscopo salva_matriz_proj(GL_PROJECTION);
    gl::CarregaIdentidade();
    // Eixo com origem embaixo esquerda.
    gl::Ortogonal(0, largura_, 0, altura_, -1.0f, 1.0f);
    gl::MatrizEscopo salva_matriz_mv(gl::MATRIZ_MODELAGEM_CAMERA);
    gl::CarregaIdentidade();
    gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);

    MudaCor(COR_BRANCA);
    gl::Habilita(GL_TEXTURE_2D);
    //gl::UnidadeTextura(GL_TEXTURE1);
    //gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::UnidadeTextura(GL_TEXTURE0);
    gl::LigacaoComTextura(GL_TEXTURE_2D, textura_framebuffer_);
    gl::Retangulo(10, altura_ - 150, 110, altura_ - 50);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  }
#elif 0
  if (MapeamentoOclusao() && !parametros_desenho_.has_picking_x()) {
    gl::Oclusao(false);
    gl::MatrizEscopo salva_matriz_proj(GL_PROJECTION);
    gl::CarregaIdentidade();
    // Eixo com origem embaixo esquerda.
    gl::Ortogonal(0, largura_, 0, altura_, -1.0f, 1.0f);
    gl::MatrizEscopo salva_matriz_mv(gl::MATRIZ_MODELAGEM_CAMERA);
    gl::CarregaIdentidade();
    gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);

    MudaCor(COR_BRANCA);
    gl::Habilita(GL_TEXTURE_2D);
    gl::UnidadeTextura(GL_TEXTURE3);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::UnidadeTextura(GL_TEXTURE0);
    gl::LigacaoComTextura(GL_TEXTURE_2D, textura_framebuffer_oclusao_);
    gl::Retangulo(10, altura_ - 150, 110, altura_ - 50);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  }
#endif



  gl::Desabilita(GL_FOG);
  gl::UsaShader(gl::TSH_SIMPLES);

  gl::DesabilitaEscopo salva_depth(GL_DEPTH_TEST);
  gl::DesabilitaEscopo salva_luz(GL_LIGHTING);

  // Configura modo 2d.
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::MatrizEscopo salva_matriz_pr(GL_PROJECTION);
  gl::CarregaIdentidade();
  if (parametros_desenho_.has_picking_x()) {
    gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
  }
  gl::Ortogonal(0, largura_, 0, altura_, -1.0f, 1.0f);
  gl::AtualizaMatrizProjecao();
  gl::MatrizEscopo salva_matriz_mv(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::CarregaIdentidade();

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

  if (parametros_desenho_.desenha_info_geral()) {
    DesenhaInfoGeral();
  }
  V_ERRO("desenhando info geral");

  if (parametros_desenho_.desenha_controle_virtual() && opcoes_.desenha_controle_virtual()) {
    // Controle na quarta posicao da pilha.
    gl::TipoEscopo controle(OBJ_CONTROLE_VIRTUAL);
#if DEBUG
    glFinish();
#endif
    timer_uma_renderizacao_controle_virtual_.start();
    DesenhaControleVirtual();
#if DEBUG
    glFinish();
#endif
    timer_uma_renderizacao_controle_virtual_.stop();
    EnfileiraTempo(timer_uma_renderizacao_controle_virtual_, &tempos_uma_renderizacao_controle_virtual_);
  }
  V_ERRO("desenhando controle virtual");

  if (gui_ != nullptr) {
    gl::TipoEscopo controle(OBJ_CONTROLE_VIRTUAL);
    gui_->Desenha(&parametros_desenho_);
  }
  V_ERRO("desenhando interface grafica");

  if (parametros_desenho_.desenha_fps()) {
    DesenhaTempos();
  }

  if (parametros_desenho_.desenha_log_eventos()) {
    DesenhaLogEventos();
  }
#if DEBUG
  glFlush();
#endif
}

void Tabuleiro::DesenhaCenaVbos() {
  //if (glGetError() == GL_NO_ERROR) LOG(ERROR) << "ok!";
  V_ERRO("ha algum erro no opengl, investigue");

  gl::Habilita(GL_DEPTH_TEST);
  V_ERRO("Teste profundidade");
  int bits_limpar = GL_DEPTH_BUFFER_BIT;
  bits_limpar |= GL_COLOR_BUFFER_BIT;
  gl::CorLimpeza(1.0f, 1.0f, 1.0f, 1.0f);
  gl::Limpa(bits_limpar);
  V_ERRO("Limpa");

  for (int i = 1; i < 8; ++i) {
    gl::Desabilita(GL_LIGHT0 + i);
  }
  V_ERRO("desabilitando luzes");

  gl::MudaModoMatriz(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::CarregaIdentidade();
  ConfiguraOlhar();

  if (!parametros_desenho_.has_pos_olho()) {
    *parametros_desenho_.mutable_pos_olho() = olho_.pos();
  }
  // Verifica o angulo em relacao ao tabuleiro para decidir se as texturas ficarao viradas para cima.
  if (camera_ == CAMERA_ISOMETRICA ||
      (camera_ != CAMERA_PRIMEIRA_PESSOA && (olho_.altura() > (2 * olho_.raio())))) {
    parametros_desenho_.set_desenha_texturas_para_cima(true);
  } else {
    parametros_desenho_.set_desenha_texturas_para_cima(false);
  }
  V_ERRO("configurando olhar");

  gl::Desabilita(GL_LIGHTING);
  gl::Desabilita(GL_FOG);
  V_ERRO("desenhando luzes");

  {
    DesenhaTabuleiro();
  }
  V_ERRO("desenhando tabuleiro");

  if (parametros_desenho_.desenha_entidades()) {
    if (!parametros_desenho_.nao_desenha_entidades_selecionaveis()) {
      for (const auto& vbo : vbos_selecionaveis_cena_) {
        vbo->Desenha();
      }
    }
    for (const auto& vbo : vbos_nao_selecionaveis_cena_) {
      vbo->Desenha();
    }
  }
  V_ERRO("desenhando entidades");

  if (parametros_desenho_.desenha_acoes()) {
    // TODO
  }
  V_ERRO("desenhando acoes");
}

void Tabuleiro::GeraVbosCena() {
  vbos_nao_selecionaveis_cena_.clear();
  vbos_selecionaveis_cena_.clear();
  vbos_acoes_cena_.clear();

  //if (glGetError() == GL_NO_ERROR) LOG(ERROR) << "ok!";
  V_ERRO("ha algum erro no opengl, investigue");

  {
    // Tabuleiro ja eh Vbo.
    //DesenhaTabuleiro();
  }
  V_ERRO("desenhando tabuleiro");

  ColetaVbosEntidades();
  V_ERRO("desenhando entidades");

  if (parametros_desenho_.desenha_acoes()) {
//#error TODO
    //DesenhaAcoes();
    //std::move(std::begin(vbos_acoes), std::end(vbos_acoes), std::back_inserter(vbos_acoes_cena_));
  }
  V_ERRO("desenhando acoes");

  //if (estado_ == ETAB_DESENHANDO && parametros_desenho_.desenha_forma_selecionada()) {
  //  vbos_entidades_translucidas_cena_.emplace_back(Entidade::ExtraiVbo(forma_proto_, &parametros_desenho_)[0]);
  //}
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

  gl::VboNaoGravado vbo_disco = gl::VboDisco(kRaioRosa, 8  /*faces*/);
  vbo_disco.AtribuiCor(1.0f, 1.0f, 1.0f, 1.0f);
  gl::VboNaoGravado vbo_seta("seta");
  {
    // Deixa espaco para o N.
    // Desenha seta.
    unsigned short indices_seta[] = { 0, 1, 2 };
    vbo_seta.AtribuiIndices(indices_seta, 3);
    float coordenadas[] = {
      -kLarguraSeta, 0.0f, 0.0f,
      kLarguraSeta, 0.0f, 0.0f,
      0.0f, kTamanhoSeta, 0.0f,
    };
    vbo_seta.AtribuiCoordenadas(3, coordenadas, 9);
    vbo_seta.AtribuiCor(1.0f, 0.0f, 0.0f, 1.0f);
  }

  gl::VboNaoGravado vbo_n("n");
  unsigned short indices_n[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
  float coordenadas_n[] = {
    // Primeira perna N.
    -4.0f, 0.0f + kRaioRosa, 0.0f,
    -1.0f, 0.0f + kRaioRosa, 0.0f,
    -4.0f, 13.0f + kRaioRosa , 0.0f,
    // Segunda.
    -4.0f, 13.0f + kRaioRosa , 0.0f,
    -4.0f, 8.0f + kRaioRosa , 0.0f,
    4.0f, 0.0f + kRaioRosa , 0.0f,
    // Terceira.
    4.0f, 0.0f + kRaioRosa , 0.0f,
    4.0f, 13.0f + kRaioRosa , 0.0f,
    1.0f, 13.0f + kRaioRosa , 0.0f,
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
  gl::VboNaoGravado vbo = gl::VboCuboSolido(DISTANCIA_PLANO_CORTE_PROXIMO * 4.0);
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
  VLOG(2) << "Regerando vbo tabuleiro, pontos: " << proto_corrente_->ponto_terreno_size();
  Terreno terreno(TamanhoX(), TamanhoY(), proto_corrente_->ladrilho(),
                  Wrapper<RepeatedField<double>>(proto_corrente_->ponto_terreno()));
  terreno.Preenche(&indices_tabuleiro,
                   &coordenadas_tabuleiro,
                   &coordenadas_normais,
                   &coordenadas_textura);
  gl::VbosNaoGravados tabuleiro_nao_gravado;
  gl::VboNaoGravado tabuleiro_parcial;
  tabuleiro_parcial.AtribuiIndices(&indices_tabuleiro);
  tabuleiro_parcial.AtribuiCoordenadas(3, &coordenadas_tabuleiro);
  tabuleiro_parcial.AtribuiTexturas(&coordenadas_textura);
  tabuleiro_parcial.AtribuiNormais(&coordenadas_normais);
  tabuleiro_nao_gravado.Concatena(tabuleiro_parcial);
  V_ERRO("RegeraVboTabuleiro antes gravar");
  vbos_tabuleiro_.Grava(tabuleiro_nao_gravado);
  V_ERRO("RegeraVboTabuleiro depois gravar");

  // Regera a grade.
  gl::VbosNaoGravados grade_nao_gravada;
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
  gl::VboNaoGravado grade_vertical;
  grade_vertical.AtribuiIndices(&indices_grade);
  grade_vertical.AtribuiCoordenadas(3, &coordenadas_grade);
  grade_nao_gravada.Concatena(&grade_vertical);
  indice = 0;
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
  gl::VboNaoGravado grade_horizontal;
  grade_horizontal.AtribuiIndices(&indices_grade);
  grade_horizontal.AtribuiCoordenadas(3, &coordenadas_grade);
  grade_nao_gravada.Concatena(&grade_horizontal);
  vbos_grade_.Grava(grade_nao_gravada);
  V_ERRO("RegeraVboTabuleiro fim");
}

void Tabuleiro::GeraFramebufferColisao(int tamanho, DadosFramebuffer* dfb) {
  GLint original;
  gl::Le(GL_FRAMEBUFFER_BINDING, &original);
  gl::GeraFramebuffers(1, &dfb->framebuffer);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb->framebuffer);
  gl::GeraRenderbuffers(1, &dfb->renderbuffer);
  // Depth attachment.
  gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, dfb->renderbuffer);
  gl::ArmazenamentoRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tamanho, tamanho);
  gl::RenderbufferDeFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dfb->renderbuffer);
  // Color attachment.
  gl::GeraTexturas(1, &dfb->textura);
  gl::LigacaoComTextura(GL_TEXTURE_2D, dfb->textura);
  gl::ImagemTextura2d(GL_TEXTURE_2D, 0, GL_RGBA, tamanho, tamanho, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dfb->textura, 0);
  //gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, renderbuffer_framebuffer_colisao_[1]);
  //gl::ArmazenamentoRenderbuffer(GL_RENDERBUFFER, GL_RGBA4, 4, 4);
  //gl::RenderbufferDeFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer_framebuffer_colisao_[1]);
  auto ret = gl::VerificaFramebuffer(GL_FRAMEBUFFER);
  if (ret != GL_FRAMEBUFFER_COMPLETE) {
    LOG(ERROR) << "Framebuffer colisao incompleto: " << ret;
  } else {
    LOG(INFO) << "Framebuffer colisao completo";
  }
  // Volta estado normal.
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, 0);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
  V_ERRO("Fim da Geracao de framebuffer");
  LOG(INFO) << "framebuffer gerado";
}

// A variavel usar_sampler_sombras eh usado como input e output. Se ela for false, nem tentara usar o sampler de sombras. Se for true,
// tentara se houver a extensao, caso contrario seta pra false e prossegue.
void Tabuleiro::GeraFramebufferLocal(int tamanho, bool textura_cubo, bool* usar_sampler_sombras, DadosFramebuffer* dfb) {
  GLint original;
  gl::Le(GL_FRAMEBUFFER_BINDING, &original);
  LOG(INFO) << "gerando framebuffer cubo ? " << textura_cubo;
  gl::GeraFramebuffers(1, &dfb->framebuffer);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb->framebuffer);
  V_ERRO("LigacaoComFramebuffer");
  gl::GeraTexturas(1, &dfb->textura);
  V_ERRO("GeraTexturas");
  gl::LigacaoComTextura(textura_cubo ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, dfb->textura);
  V_ERRO("LigacaoComTextura");
  if (textura_cubo) {
    for (int i = 0; i < 6; ++i) {
      gl::ImagemTextura2d(
          GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, TAM_MAPA_OCLUSAO, TAM_MAPA_OCLUSAO, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
  } else {
#if USAR_MAPEAMENTO_SOMBRAS_OPENGLES
    if (*usar_sampler_sombras && (gl::TemExtensao("GL_OES_depth_texture") || gl::TemExtensao("GL_ARB_depth_texture"))) {
      LOG(INFO) << "Gerando framebuffer com sampler de sombras.";
      gl::ImagemTextura2d(
          GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, tamanho, tamanho, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT);
    } else {
      LOG(INFO) << "Gerando framebuffer com sampler de profundidade.";
      *usar_sampler_sombras = false;
      gl::ImagemTextura2d(GL_TEXTURE_2D, 0, GL_RGBA, tamanho, tamanho, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
#else
    gl::ImagemTextura2d(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, tamanho, tamanho, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    // Indica que vamos comparar o valor de referencia passado contra o valor armazenado no mapa de textura.
    // Nas versoes mais nova, usa-se ref.
    //gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
#endif
  }

  V_ERRO("ImagemTextura2d");
  if (textura_cubo) {
#if !USAR_MAPEAMENTO_SOMBRAS_OPENGLES
    //gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    //gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
    //gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  } else {
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::ParametroTextura(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  V_ERRO("ParametroTextura");
  if (textura_cubo) {
    for (int i = 0; i < 6; ++i) {
      gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, dfb->textura, 0);
    }
    gl::GeraRenderbuffers(1, &dfb->renderbuffer);
    gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, dfb->renderbuffer);
    gl::ArmazenamentoRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, TAM_MAPA_OCLUSAO, TAM_MAPA_OCLUSAO);
    gl::RenderbufferDeFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dfb->renderbuffer);
  } else {
#if USAR_MAPEAMENTO_SOMBRAS_OPENGLES
    if (*usar_sampler_sombras) {
      LOG(INFO) << "Textura de sombras para shader de sombras";
      gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dfb->textura, 0);
    } else {
      LOG(INFO) << "Textura de completa para shader de profundidade";
      gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dfb->textura, 0);
      gl::GeraRenderbuffers(1, &dfb->renderbuffer);
      gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, dfb->renderbuffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tamanho, tamanho);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, dfb->renderbuffer);
    }
#else
    gl::TexturaFramebuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dfb->textura, 0);
#endif
  }
  V_ERRO("TexturaFramebuffer");

  // No OSX o framebuffer fica incompleto se nao desabilitar o buffer de desenho e leitura para esse framebuffer.
#if __APPLE__ && !USAR_OPENGL_ES
  if (!textura_cubo && *usar_sampler_sombras) {
    gl::BufferDesenho(GL_NONE);
    gl::BufferLeitura(GL_NONE);
  }
#endif
  auto ret = gl::VerificaFramebuffer(GL_FRAMEBUFFER);
  if (ret != GL_FRAMEBUFFER_COMPLETE) {
    LOG(ERROR) << "Framebuffer incompleto: " << ret;
  } else {
    LOG(INFO) << "Framebuffer completo";
  }

  // Volta estado normal.
  gl::LigacaoComTextura(textura_cubo ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 0);
  gl::LigacaoComRenderbuffer(GL_RENDERBUFFER, 0);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
  V_ERRO("Fim da Geracao de framebuffer");
  LOG(INFO) << "framebuffer gerado";
}


void Tabuleiro::GeraFramebuffer() {
  GeraFramebufferColisao(TAM_BUFFER_COLISAO, &dfb_colisao_);
  GeraFramebufferLocal(
      1024, false  /*textura_cubo*/, &usar_sampler_sombras_, &dfb_luz_direcional_);
  GeraFramebufferLocal(
      1024, true  /*textura_cubo*/, &usar_sampler_sombras_, &dfb_oclusao_);
  dfb_luzes_.resize(1);
  GeraFramebufferLocal(
      1024, true  /*textura_cubo*/, &usar_sampler_sombras_, &dfb_luzes_[0]);
}

namespace {
void GeraPontoAleatorioMontanha(int x, int y, int tam_x, float altura_m, float max_porcentagem_aleatoria, TabuleiroProto* proto) {
  float altura_aleatoria_m = altura_m * (1.0f + max_porcentagem_aleatoria * Aleatorio());
  int indice = Terreno::IndicePontoTabuleiro(x, y, tam_x);
  if (proto->ponto_terreno(indice) < altura_aleatoria_m) {
    proto->set_ponto_terreno(indice, altura_aleatoria_m);
  }
}

}  // namespace

// TODO passar esse codigo pro terreno.
void Tabuleiro::GeraMontanhaNotificando() {
  if (!EmModoMestreIncluindoSecundario()) {
    LOG(INFO) << "Apenas mestre pode gerar montanha.";
    return;
  }
  if (estado_ != ETAB_QUAD_SELECIONADO) {
    LOG(INFO) << "Preciso de um quadrado selecionado.";
    return;
  }
  // Pega o ponto inicial (SW quadrado selecionado).
  int x_quad = -1, y_quad = -1;
  XYQuadrado(quadrado_selecionado_, &x_quad, &y_quad);
  if (x_quad < 0 || y_quad < 0) {
    LOG(INFO) << "Coordenadas invalidas para quadrado selecionado";
    return;
  }
  // Gera o terreno se nao houver.
  proto_corrente_->mutable_ponto_terreno()->Resize((TamanhoX() + 1) * (TamanhoY() + 1), 0);

  ntf::Notificacao n_desfazer;
  n_desfazer.set_tipo(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
  {
    auto* cenario_antes = n_desfazer.mutable_tabuleiro_antes();
    cenario_antes->set_id_cenario(proto_corrente_->id_cenario());
    *cenario_antes->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  }

  int indice = Terreno::IndicePontoTabuleiro(x_quad, y_quad, TamanhoX());
  float altura_ponto_inicial_m = proto_corrente_->ponto_terreno(indice);

  // Inclinacao e delta de altura por iteracao.
  float inclinacao_graus = 45.0f;
  float delta_h_m = TAMANHO_LADO_QUADRADO * tanf(inclinacao_graus * GRAUS_PARA_RAD);
  if (inclinacao_graus < 0 || inclinacao_graus > 85.0f) {
    LOG(INFO) << "Inclinacao invalida: " << inclinacao_graus;
    return;
  }

  // Fator aleatorio: cada ponto podera ser ate esse fator mais alto.
  float max_porcentagem_aleatoria = 0.3f;

  // Altura inicial: usa o proprio ponto se ja tiver altura suficiente. Senao, gera um de altura constante.
  float altura_m = (altura_ponto_inicial_m > delta_h_m) ? altura_ponto_inicial_m : (10.0f * (1.0f + (max_porcentagem_aleatoria * Aleatorio())));

  // Numero de iteracoes eh calculado ate o terreno chegar no nivel 0, baseado na altura inicial e o delta.
  int num_iteracoes = altura_m / delta_h_m;

  // Eleva o ponto inicial na altura.
  proto_corrente_->set_ponto_terreno(Terreno::IndicePontoTabuleiro(x_quad, y_quad, TamanhoX()), altura_m);

  // Percorre os pontos ao redor do quadrado, reduzindo a altura de acordo com a inclinacao.
  // Terminar quando chegar em 0.
  float altura_ajustada_m = altura_m;
  for (int i = 1; i <= num_iteracoes; ++i) {
    altura_ajustada_m -= delta_h_m;
    // sul e norte.
    {
      int y_base_s = y_quad - i;
      int y_base_n = y_quad + i;
      int x_w = x_quad - i;
      for (int j = 0; j < ((i * 2) + 1); ++j) {
        int x_corrente = x_w + j;
        if (x_corrente < 0 || x_corrente > TamanhoX()) {
          continue;
        }
        if (y_base_s >= 0) {
          GeraPontoAleatorioMontanha(x_corrente, y_base_s, TamanhoX(), altura_ajustada_m, max_porcentagem_aleatoria, proto_corrente_);
        }
        if (y_base_n <= TamanhoY()) {
          GeraPontoAleatorioMontanha(x_corrente, y_base_n, TamanhoX(), altura_ajustada_m, max_porcentagem_aleatoria, proto_corrente_);
        }
      }
    }
    // Oeste e leste.
    {
      int y_s = y_quad - i + 1;
      int x_base_w = x_quad - i;
      int x_base_e = x_quad + i;
      for (int j = 0; j < ((i * 2) - 1); ++j) {
        int y_corrente = y_s + j;
        if (y_corrente < 0 || y_corrente > TamanhoY()) {
          continue;
        }
        if (x_base_w >= 0) {
          GeraPontoAleatorioMontanha(x_base_w, y_corrente, TamanhoX(), altura_ajustada_m, max_porcentagem_aleatoria, proto_corrente_);
        }
        if (x_base_e <= TamanhoX()) {
          GeraPontoAleatorioMontanha(x_base_e, y_corrente, TamanhoX(), altura_ajustada_m, max_porcentagem_aleatoria, proto_corrente_);
        }
      }
    }
  }
  RegeraVboTabuleiro();
  RefrescaTerrenoParaClientes();
  {
    auto* cenario_depois = n_desfazer.mutable_tabuleiro();
    cenario_depois->set_id_cenario(proto_corrente_->id_cenario());
    *cenario_depois->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  }
  AdicionaNotificacaoListaEventos(n_desfazer);
}

void Tabuleiro::GeraTerrenoAleatorioNotificando() {
  if (!EmModoMestreIncluindoSecundario()) {
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
  ntf::Notificacao n_desfazer;
  n_desfazer.set_tipo(ntf::TN_ATUALIZAR_RELEVO_TABULEIRO);
  {
    auto* cenario_depois = n_desfazer.mutable_tabuleiro();
    cenario_depois->set_id_cenario(proto_corrente_->id_cenario());
    *cenario_depois->mutable_ponto_terreno() = proto_corrente_->ponto_terreno();
  }
  AdicionaNotificacaoListaEventos(n_desfazer);
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
  if (estado_ != ETAB_QUAD_SELECIONADO || !EmModoMestreIncluindoSecundario()) {
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
                parametros_desenho_.offset_terreno());
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
  vbos_tabuleiro_.Desenha();
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

namespace {
bool PulaEntidade(const EntidadeProto& proto, const ParametrosDesenho& pd) {
  if (!proto.faz_sombra() && pd.desenha_mapa_sombras()) {
    return true;
  }
  if (!proto.causa_colisao() && pd.desenha_apenas_entidades_colisivas()) {
    return true;
  }
  if (proto.fixa() && pd.nao_desenha_entidades_fixas()) {
    return true;
  }
  if (proto.selecionavel_para_jogador() && pd.nao_desenha_entidades_selecionaveis()) {
    return true;
  }
  if (proto.tipo() == TE_ENTIDADE && (pd.has_desenha_mapa_luzes() || pd.has_desenha_mapa_oclusao())) {
    // Para luzes pontuais e oclusao, entidades nao contam (performance).
    return true;
  }
  if (proto.fixa() && proto.cor().a() < 1.0f && pd.nao_desenha_entidades_fixas_translucidas()) {
    return true;
  }
  return false;
}
}  // namespace

void Tabuleiro::OrdenaEntidades() {
  auto MaisProximoOlho = [this] (Entidade* lhs, Entidade* rhs) {
    Vector3 lhs_pos(lhs->X(), lhs->Y(), lhs->Z());
    Vector3 rhs_pos(rhs->X(), rhs->Y(), rhs->Z());
    Vector3 olho_pos(olho_.pos().x(), olho_.pos().y(), olho_.pos().z());
    Vector3 olho_lhs = olho_pos - lhs_pos;
    Vector3 olho_rhs = olho_pos - rhs_pos;
    return olho_lhs.length() < olho_rhs.length();
  };
  std::set<Entidade*, std::function<bool(Entidade*, Entidade*)>> set_entidades(MaisProximoOlho);
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    auto* entidade = it->second.get();
    if (entidade->IdCenario() == cenario_corrente_) {
      set_entidades.insert(entidade);
    }
  }
  entidades_ordenadas_.clear();
  for (auto* entidade : set_entidades) {
    entidades_ordenadas_.push_back(entidade);
  }
}

void Tabuleiro::DesenhaEntidadesBase(const std::function<void (Entidade*, ParametrosDesenho*)>& f) {
  //LOG(INFO) << "LOOP";
#if ORDENAR_ENTIDADES
  for (Entidade* entidade : entidades_ordenadas_) {
#else
  for (auto& it : entidades_) {
    auto* entidade = it.second.get();
#endif
    if (entidade == nullptr) {
      LOG(ERROR) << "Entidade nao existe.";
      continue;
    }
    if (entidade->Pos().id_cenario() != cenario_corrente_) {
      continue;
    }
    if (PulaEntidade(entidade->Proto(), parametros_desenho_)) {
      continue;
    }
    // Nao desenha a propria entidade na primeira pessoa, apenas sua sombra.
    if (camera_presa_ &&  entidade->Id() == IdCameraPresa() && !parametros_desenho_.desenha_mapa_sombras()) {
      if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
        continue;
      }
      if (parametros_desenho_.has_desenha_mapa_oclusao() || parametros_desenho_.has_desenha_mapa_luzes()) {
        continue;
      }
    }
    // Nao roda disco se estiver arrastando.
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS &&
                                                 EntidadeEstaSelecionada(entidade->Id()));
    parametros_desenho_.set_iniciativa_corrente(indice_iniciativa_ >= 0 && indice_iniciativa_ < (int)iniciativas_.size() &&
                                                iniciativas_[indice_iniciativa_].id == entidade->Id());
    bool detalhar_tudo = !(parametros_desenho_.desenha_mapa_sombras() ||
                           parametros_desenho_.has_desenha_mapa_oclusao() ||
                           parametros_desenho_.has_desenha_mapa_luzes()) &&
                         (detalhar_todas_entidades_ || modo_clique_ == MODO_ACAO);
    bool entidade_detalhada = parametros_desenho_.desenha_detalhes() &&
                              tipo_entidade_detalhada_ == OBJ_ENTIDADE &&
                              entidade->Id() == id_entidade_detalhada_;
    parametros_desenho_.set_desenha_barra_vida(entidade_detalhada || detalhar_tudo);
    // Rotulos apenas individualmente.
    parametros_desenho_.set_desenha_rotulo(entidade_detalhada);
    parametros_desenho_.set_desenha_rotulo_especial(
        entidade_detalhada && (VisaoMestre() || entidade->SelecionavelParaJogador()));
    parametros_desenho_.set_desenha_eventos_entidades(
        !(parametros_desenho_.desenha_mapa_sombras() ||
          parametros_desenho_.has_desenha_mapa_oclusao() ||
          parametros_desenho_.has_desenha_mapa_luzes()) &&
        (VisaoMestre() || entidade->SelecionavelParaJogador()));
    //LOG(INFO) << "Desenhando: " << entidade->Id();
    f(entidade, &parametros_desenho_);
  }
  parametros_desenho_.set_entidade_selecionada(false);
  parametros_desenho_.set_desenha_barra_vida(false);
}

void Tabuleiro::ColetaVbosEntidades() {
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second.get();
    if (entidade == nullptr) {
      LOG(ERROR) << "Entidade nao existe.";
      continue;
    }
    if (entidade->Pos().id_cenario() != cenario_corrente_) {
      continue;
    }
    if (!entidade->Proto().faz_sombra() && parametros_desenho_.desenha_mapa_sombras()) {
      continue;
    }
    if (!entidade->Proto().visivel() &&
        !entidade->Proto().selecionavel_para_jogador() &&
        (!EmModoMestreIncluindoSecundario() || visao_jogador_)) {
      continue;
    }
    // Nao roda disco se estiver arrastando.
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS &&
                                                 EntidadeEstaSelecionada(entidade->Id()));
    parametros_desenho_.set_desenha_barra_vida(false);
    // Rotulos apenas individualmente.
    parametros_desenho_.set_desenha_rotulo(false);
    parametros_desenho_.set_desenha_rotulo_especial(false);
    parametros_desenho_.set_desenha_eventos_entidades(VisaoMestre() || entidade->SelecionavelParaJogador());
    std::vector<const gl::VbosGravados*>* dest = entidade->Proto().selecionavel_para_jogador()
        ? &vbos_selecionaveis_cena_ : &vbos_nao_selecionaveis_cena_;
    try {
      dest->push_back(entidade->VboExtraido());
    } catch (...) {
      // Vbo vazio, pode acontecer. Ignora.
    }
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
    if (a->IdCenario() != cenario_corrente_) {
      continue;
    }
    a->Desenha(&parametros_desenho_);
  }
}

void Tabuleiro::DesenhaFormaSelecionada() {
  // Quando alfa_translucido < 1.0f, assume-se blend ligado.
  gl::HabilitaEscopo habilita_blend(GL_BLEND);
  parametros_desenho_.set_alfa_translucidos(0.5);
  AlteraBlendEscopo blend_escopo(&parametros_desenho_, forma_proto_.cor());
  Entidade::ExtraiVbo(forma_proto_, &parametros_desenho_, true  /*mundo*/).Desenha();
  parametros_desenho_.clear_alfa_translucidos();
}

void Tabuleiro::DesenhaRosaDosVentos() {
  gl::MatrizEscopo salva_matriz_mv;
  const float kRaioRosa = 20.0f;
  // Deixa espaco para o N.
  gl::Translada(largura_ - kRaioRosa - 15.0f, kRaioRosa + 15.0f, 0.0f);
  // Roda pra posicao correta.
  Posicao direcao;
  ComputaDiferencaVetor(olho_.alvo(), olho_.pos(), &direcao);
  // A diferenca eh em relacao ao leste e o norte esta a 90 graus. Quanto maior a diferenca, mais proximo do norte (ate 90.0f).
  float diferenca_graus = 90.0f - VetorParaRotacaoGraus(direcao.x(), direcao.y());
  gl::Roda(diferenca_graus, 0.0f, 0.0f, 1.0f);
  gl::AtualizaMatrizes();
  //gl::MudaCor(1.0f, 0.0f, 0.0f, 1.0f);
  //gl::Retangulo(10.0f);
  gl::DesenhaVbo(vbo_rosa_, GL_TRIANGLES);
}

void Tabuleiro::DesenhaPontosRolagem() {
  // 4 pontos.
  MudaCor(COR_PRETA);
  gl::MatrizEscopo salva_matriz(gl::MATRIZ_MODELAGEM_CAMERA);
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

void Tabuleiro::AtualizaOlho(int intervalo_ms, bool forcar) {
  const auto* entidade_referencia = BuscaEntidade(IdCameraPresa());
  if (camera_presa_) {
    if (entidade_referencia == nullptr) {
      AlternaCameraPresa();
    } else {
      bool cenario_diferente = entidade_referencia->Pos().id_cenario() != proto_corrente_->id_cenario();
      if (cenario_diferente) {
        // Pode acontecer da entidade estar se movendo para o cenario novo.
        if ((entidade_referencia->Destino().has_id_cenario() &&
            (entidade_referencia->Destino().id_cenario() == proto_corrente_->id_cenario()))) {
          cenario_diferente = false;
        }
      }
      if (cenario_diferente) {
        // Carrega sub cenario chama AtualizaOlho.
        //CarregaSubCenario(entidade_referencia->Pos().id_cenario(), entidade_referencia->Pos());
        AlternaCameraPresa();
        LOG(WARNING) << "Nao consigo atualizar olho porque entidade presa esta em outro cenario";
        return;
      }
      if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
        olho_.clear_destino();
        olho_.mutable_pos()->set_x(entidade_referencia->X());
        olho_.mutable_pos()->set_y(entidade_referencia->Y());
        olho_.mutable_pos()->set_z(entidade_referencia->ZOlho());
        // Aqui fazemos o inverso da visao normal. Colocamos o alvo no angulo oposto da rotacao para olhar na mesma direcao
        // que a camera de perspectiva.
        olho_.mutable_alvo()->set_x(olho_.pos().x() + cosf(olho_.rotacao_rad() + M_PI));
        olho_.mutable_alvo()->set_y(olho_.pos().y() + sinf(olho_.rotacao_rad() + M_PI));
        olho_.mutable_alvo()->set_z(olho_.pos().z() - (olho_.altura() - OLHO_ALTURA_INICIAL) / 4.0f);

        if (entidade_referencia->Espiada() != 0.0f) {
          float altura_olho = entidade_referencia->AlturaOlho();
          Vector3 vetor_olho(olho_.pos().x(), olho_.pos().y(), olho_.pos().z());
          Vector3 vetor_alvo(olho_.alvo().x(), olho_.alvo().y(), olho_.alvo().z());
          Vector3 vetor_olhar = vetor_alvo - vetor_olho;
          vetor_olhar.normalize();

          const float MAXIMA_INCLINACAO_ESPIADA_GRAUS = 25.0f;
          const Vector4 vetor_up(0.0f, 0.0f, altura_olho, 1.0f);
          Matrix4 rotacao;
          rotacao.rotate(entidade_referencia->Espiada() * MAXIMA_INCLINACAO_ESPIADA_GRAUS, vetor_olhar);
          Vector4 ajuste4 = rotacao * vetor_up;
          Vector3 ajuste(ajuste4.x, ajuste4.y, ajuste4.z);

          // Posicao.
          olho_.mutable_pos()->set_x(vetor_olho.x + ajuste.x);
          olho_.mutable_pos()->set_y(vetor_olho.y + ajuste.y);
          olho_.mutable_pos()->set_z(vetor_olho.z - altura_olho + ajuste.z);
          // Alvo.
          olho_.mutable_alvo()->set_x(olho_.alvo().x() + ajuste.x);
          olho_.mutable_alvo()->set_y(olho_.alvo().y() + ajuste.y);
          olho_.mutable_alvo()->set_z(olho_.alvo().z() - altura_olho + ajuste.z);
        }
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
  boost::timer::cpu_timer timer;
#if DEBUG
  glFinish();
#endif

  for (auto& id_ent : entidades_) {
    parametros_desenho_.set_entidade_selecionada(estado_ != ETAB_ENTS_PRESSIONADAS && EntidadeEstaSelecionada(id_ent.first));
    auto* entidade = id_ent.second.get();
#if DEBUG
    glFinish();
#endif
    timer.resume();
    entidade->Atualiza(intervalo_ms, &timer);
    parametros_desenho_.clear_entidade_selecionada();
  }
  EnfileiraTempo(timer, &tempos_atualiza_parcial_);
}

void Tabuleiro::AtualizaIniciativas() {
  // Ha tres casos: adicao de nova entidade, atualizacao e remocao.
  bool atualizar_remoto = false;
  std::unordered_map<unsigned int, DadosIniciativa*> mapa_iniciativas;
  for (DadosIniciativa& di : iniciativas_) {
    mapa_iniciativas[di.id] = &di;
    di.presente = false;
  }

  std::unique_ptr<ntf::Notificacao> n(ntf::NovaNotificacao(ntf::TN_ATUALIZAR_LISTA_INICIATIVA));

  std::vector<const Entidade*> entidades_adicionar;
  for (auto& id_ent : entidades_) {
    const auto* entidade = id_ent.second.get();
    if (!entidade->TemIniciativa()) {
      continue;
    }
    auto it = mapa_iniciativas.find(entidade->Id());
    if (it == mapa_iniciativas.end()) {
      atualizar_remoto = true;  // adicao.
      entidades_adicionar.push_back(entidade);
    } else {
      if (entidade->Iniciativa() != it->second->iniciativa || entidade->ModificadorIniciativa() != it->second->modificador) {
        // Como nao esta marcada como presente, sera removida. E depois, adicionada.
        atualizar_remoto = true;  // atualizacao.
        entidades_adicionar.push_back(entidade);
      } else {
        it->second->presente = true;
      }
    }
  }
  // Remove nao presentes.
  for (int i = 0; i < (int)iniciativas_.size();) {
    DadosIniciativa& di = iniciativas_[i];
    if (!di.presente) {
      atualizar_remoto = true;  // remocao.
      if (indice_iniciativa_ > i) {
        --indice_iniciativa_;
      } else if (indice_iniciativa_ == i && i == (int)(iniciativas_.size() - 1)) {
        indice_iniciativa_ = 0;
        PassaUmaRodadaNotificando();
      }
      // Senao, ignora pq nao faz diferennca, mesmo que i == indice.
      // Agora pode remover.
      iniciativas_.erase(iniciativas_.begin() + i);
    } else {
      ++i;
    }
  }
  // Adiciona novas entidades (ou atualizadas).
  for (const auto* entidade : entidades_adicionar) {
    // Acha ponto de insercao.
    int posicao = 0;
    while (posicao < (int)iniciativas_.size() &&
           (entidade->Iniciativa() < iniciativas_[posicao].iniciativa ||
           (entidade->Iniciativa() == iniciativas_[posicao].iniciativa && entidade->ModificadorIniciativa() < iniciativas_[posicao].modificador))) {
      ++posicao;
    }
    DadosIniciativa di;
    di.id = entidade->Id();
    di.iniciativa = entidade->Iniciativa();
    di.modificador = entidade->ModificadorIniciativa();
    iniciativas_.insert(iniciativas_.begin() + posicao, di);
    if (indice_iniciativa_ >= posicao) {
      ++indice_iniciativa_;
    }
  }

  if (iniciativas_.empty()) {
    indice_iniciativa_ = -1;
  }
  if (atualizar_remoto) {
    SerializaIniciativas(n->mutable_tabuleiro());
    central_->AdicionaNotificacaoRemota(n.release());
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
      if (ap.id_entidade_destino_size() > 0 && ap.afeta_pontos_vida()) {
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

bool Tabuleiro::SelecionaEntidade(unsigned int id, bool forcar_fixa) {
  VLOG(2) << "Selecionando entidade: " << id;
  ids_entidades_selecionadas_.clear();
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    LOG(ERROR) << "Entidade invalida: " << id;
    throw std::logic_error("Entidade inválida");
  }
  if ((!forcar_fixa && entidade->Fixa()) || (!EmModoMestreIncluindoSecundario() && !entidade->SelecionavelParaJogador())) {
    DeselecionaEntidades();
    return false;
  }
  ids_entidades_selecionadas_.insert(entidade->Id());
  quadrado_selecionado_ = -1;
  if (IdPresoACamera(id) && camera_ != CAMERA_PRIMEIRA_PESSOA) {
    auto it = std::find(ids_camera_presa_.begin(), ids_camera_presa_.end(), id);
    ids_camera_presa_.splice(ids_camera_presa_.begin(), ids_camera_presa_, it);
  }
  // Nao precisa mudar porque a funcao MudaEstadoAposSelecao fara isso.
  // estado_ = ETAB_ENTS_SELECIONADAS;
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
        (!EmModoMestreIncluindoSecundario() && !entidade->SelecionavelParaJogador())) {
      continue;
    }
    ids_entidades_selecionadas_.insert(id);
  }
  if (ids_entidades_selecionadas_.size() == 1) {
    SelecionaEntidade(ids[0]);
  }
  MudaEstadoAposSelecao();
}

void Tabuleiro::AtualizaSelecaoEntidade(unsigned int id) {
  if (!EntidadeEstaSelecionada(id)) {
    return;
  }
  auto* e = BuscaEntidade(id);
  if (e == nullptr || (!EmModoMestreIncluindoSecundario() && !e->SelecionavelParaJogador())) {
    ids_entidades_selecionadas_.erase(id);
  }
  if (ids_entidades_selecionadas_.size() == 1) {
    SelecionaEntidade(*ids_entidades_selecionadas_.begin());
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
    if (ids_entidades_selecionadas_.size() == 1) {
      SelecionaEntidade(*ids_entidades_selecionadas_.begin());
    }
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
  if (ids_entidades_selecionadas_.size() == 1) {
    SelecionaEntidade(*ids_entidades_selecionadas_.begin());
  }
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

void Tabuleiro::XYQuadrado(unsigned int quadrado, int* x, int* y) {
  if (estado_ != ETAB_QUAD_SELECIONADO) {
    LOG(ERROR) << "XYQuadradoSelecionado sem quadrado selecionado";
    return;
  }
  int quad_x = quadrado % TamanhoX();
  int quad_y = quadrado / TamanhoX();
  if (quad_x >= TamanhoX()) {
    LOG(ERROR) << "XYQuadradoSelecionado excede em x: " << quad_x << ", tamanho: " << TamanhoX();
    return;
  }
  if (quad_y >= TamanhoY()) {
    LOG(ERROR) << "XYQuadradoSelecionado excede em y: " << quad_y << ", tamanho: " << TamanhoY();
    return;
  }
  *x = quad_x;
  *y = quad_y;
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
  const auto& novo_tabuleiro = notificacao.tabuleiro();
  bool manter_entidades = novo_tabuleiro.manter_entidades();
  if (manter_entidades) {
    VLOG(1) << "Deserializando tabuleiro mantendo entidades: " << novo_tabuleiro.ShortDebugString();
  } else {
    VLOG(1) << "Deserializando tabuleiro todo: " << novo_tabuleiro.ShortDebugString();
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
  for (auto& sub_cenario : novo_tabuleiro.sub_cenario()) {
    auto* cenario_dummy = proto_.add_sub_cenario();
    cenario_dummy->set_id_cenario(sub_cenario.id_cenario());
  }
  AtualizaTexturasIncluindoSubCenarios(novo_tabuleiro);
  proto_.CopyFrom(novo_tabuleiro);
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
    VLOG(1) << "Alterando id de cliente para " << novo_tabuleiro.id_cliente();
    id_cliente_ = novo_tabuleiro.id_cliente();
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
  for (EntidadeProto ep : novo_tabuleiro.entidade()) {
    if (manter_entidades) {
      // Para manter as entidades, os ids tem que ser regerados para as entidades do tabuleiro,
      // senao pode dar conflito com as que ficaram.
      ep.set_id(GeraIdEntidade(id_cliente_));
    }
    auto* e = NovaEntidade(ep, texturas_, m3d_, central_, &parametros_desenho_);
    if (!entidades_.insert(std::make_pair(e->Id(), std::unique_ptr<Entidade>(e))).second) {
      LOG(ERROR) << "Erro adicionando entidade: " << ep.ShortDebugString();
    }
  }
  VLOG(1) << "Foram adicionadas " << novo_tabuleiro.entidade_size() << " entidades";
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
      ids_a_deselecionar.push_back(id);
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
  entidades_copiadas.mutable_origem()->set_nome(proto_.nome());
  entidades_copiadas.mutable_origem()->set_id_cenario(cenario_corrente_);
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

namespace {

Posicao& operator-=(Posicao& pos, const Posicao& delta) {
  pos.set_x(pos.x() - delta.x());
  pos.set_y(pos.y() - delta.y());
  pos.set_z(pos.z() - delta.z());
  return pos;
}

Posicao operator-(const Posicao& pos, const Posicao& delta) {
  Posicao ret;
  ret.set_x(pos.x() - delta.x());
  ret.set_y(pos.y() - delta.y());
  ret.set_z(pos.z() - delta.z());
  return ret;
}

Posicao operator+(const Posicao& pos, const Posicao& delta) {
  Posicao ret;
  ret.set_x(pos.x() + delta.x());
  ret.set_y(pos.y() + delta.y());
  ret.set_z(pos.z() + delta.z());
  return ret;
}

Posicao PosicaoMedia(const std::vector<EntidadeProto*>& entidades) {
  float x_m = 0.0f;
  float y_m = 0.0f;
  float z_m = 0.0f;
  int n = 0;
  for (const auto* e : entidades) {
    const auto& ep = e->pos();
    x_m += ep.x();
    y_m += ep.y();
    z_m += ep.z();
    ++n;
  }
  Posicao pos;
  pos.set_x(x_m / n);
  pos.set_y(y_m / n);
  pos.set_z(z_m / n);
  return pos;
}

void AjustaPosicoes(const Posicao& alvo, std::vector<EntidadeProto*>* entidades) {
  Posicao media = PosicaoMedia(*entidades);
  for (auto* e : *entidades) {
    *(e->mutable_pos()) -= (media - alvo);
  }
}

}  // namespace

void Tabuleiro::ColaEntidadesSelecionadas(bool ref_camera) {
  std::vector<EntidadeProto*> entidades_coladas;
#if USAR_QT
  std::string str_entidades(QApplication::clipboard()->text().toUtf8().constData());
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
    entidades_coladas.push_back(n->mutable_entidade());
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
    entidades_coladas.push_back(n->mutable_entidade());
  }
#endif
  if (ref_camera) {
    AjustaPosicoes(olho_.alvo(), &entidades_coladas);
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
    VLOG(1) << "Estado invalido para agrupar: " << estado_;
    return;
  }
  VLOG(1) << "Agrupando entidades selecionadas.";
  EntidadeProto nova_entidade;
  nova_entidade.set_tipo(TE_COMPOSTA);
  float x_medio = 0;
  float y_medio = 0;
  float z_medio = 0;
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
    z_medio += e->Z();
    nova_entidade.add_sub_forma()->CopyFrom(e->Proto());
    ++num_entidades;
  }
  if (num_entidades <= 1) {
    VLOG(1) << "Nenhuma (ou uma) entidade valida para agrupar";
    return;
  }
  x_medio /= num_entidades;
  y_medio /= num_entidades;
  z_medio /= num_entidades;
  nova_entidade.mutable_pos()->set_x(x_medio);
  nova_entidade.mutable_pos()->set_y(y_medio);
  nova_entidade.mutable_pos()->set_z(z_medio);
  for (auto& sub_forma : *nova_entidade.mutable_sub_forma()) {
    sub_forma.mutable_pos()->set_x(sub_forma.pos().x() - x_medio);
    sub_forma.mutable_pos()->set_y(sub_forma.pos().y() - y_medio);
    sub_forma.mutable_pos()->set_z(sub_forma.pos().z() - z_medio);
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
      Vector4 vetor_pos(pos->x() * proto_composto.escala().x(),
                        pos->y() * proto_composto.escala().y(),
                        pos->z() * proto_composto.escala().z(),
                        1.0f);
      Matrix4 rotacao;
      rotacao.rotateZ(proto_composto.rotacao_z_graus());
      rotacao.rotateY(proto_composto.rotacao_y_graus());
      rotacao.rotateX(proto_composto.rotacao_x_graus());
      vetor_pos = rotacao * vetor_pos;
      pos->set_x(vetor_pos.x + proto_composto.pos().x());
      pos->set_y(vetor_pos.y + proto_composto.pos().y());
      pos->set_z(vetor_pos.z + proto_composto.pos().z());
      auto* escala = nova_entidade->mutable_escala();
      escala->set_x(escala->x() * proto_composto.escala().x());
      escala->set_y(escala->y() * proto_composto.escala().y());
      escala->set_z(escala->z() * proto_composto.escala().z());
      nova_entidade->set_rotacao_x_graus(nova_entidade->rotacao_x_graus() + proto_composto.rotacao_x_graus());
      nova_entidade->set_rotacao_y_graus(nova_entidade->rotacao_y_graus() + proto_composto.rotacao_y_graus());
      nova_entidade->set_rotacao_z_graus(nova_entidade->rotacao_z_graus() + proto_composto.rotacao_z_graus());
      ++num_adicionados;
    }
    auto* notificacao_remocao = grupo_notificacoes.add_notificacao();
    notificacao_remocao->set_tipo(ntf::TN_REMOVER_ENTIDADE);
    notificacao_remocao->mutable_entidade()->CopyFrom(proto_composto);
  }
  TrataNotificacao(grupo_notificacoes);
  AdicionaEntidadesSelecionadas(ids_adicionados_);
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

Tabuleiro::ResultadoColisao Tabuleiro::DetectaColisao(
    float x, float y, float z_olho, float espaco_entidade, const Vector3& movimento, bool ignora_espaco_entidade) {
  gl::MatrizEscopo salva_mvm(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::MatrizEscopo salva_prj(gl::MATRIZ_PROJECAO);
  float tamanho_movimento = movimento.length();
  espaco_entidade = ignora_espaco_entidade ? 0.0f : espaco_entidade;

  ParametrosDesenho pd(parametros_desenho_);
  parametros_desenho_.set_desenha_terreno(true);
  parametros_desenho_.set_desenha_entidades(true);
  parametros_desenho_.set_desenha_apenas_entidades_colisivas(true);
  parametros_desenho_.set_desenha_controle_virtual(false);
  parametros_desenho_.mutable_projecao()->set_tipo_camera(CAMERA_ISOMETRICA);
  parametros_desenho_.mutable_projecao()->set_plano_corte_proximo_m(0.0f);
  parametros_desenho_.mutable_projecao()->set_plano_corte_distante_m(tamanho_movimento + espaco_entidade + 0.01f);
  parametros_desenho_.mutable_projecao()->set_largura_m(espaco_entidade + 0.01);
  parametros_desenho_.mutable_projecao()->set_altura_m(espaco_entidade + 0.01);
  parametros_desenho_.clear_offset_terreno();

  // Configura o olhar para a direcao do movimento.
  Posicao origem_temp;
  origem_temp.set_x(x);
  origem_temp.set_y(y);
  origem_temp.set_z(z_olho);
  Posicao alvo_temp;
  alvo_temp.set_x(origem_temp.x() + movimento.x);
  alvo_temp.set_y(origem_temp.y() + movimento.y);
  alvo_temp.set_z(origem_temp.z() + movimento.z);
  olho_.mutable_pos()->Swap(&origem_temp);
  olho_.mutable_alvo()->Swap(&alvo_temp);
  VLOG(2) << "olho para colisao: " << olho_.ShortDebugString();
  gl::Viewport(0, 0, TAM_BUFFER_COLISAO, TAM_BUFFER_COLISAO);

  GLint original;
  gl::Le(GL_FRAMEBUFFER_BINDING, &original);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, dfb_colisao_.framebuffer);

  unsigned int id, tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(TAM_BUFFER_COLISAO / 2, TAM_BUFFER_COLISAO / 2, &id, &tipo_objeto, &profundidade);
  //BuscaHitMaisProximo(1, 1, &id, &tipo_objeto, &profundidade);
  // restaura valores.
  olho_.mutable_pos()->Swap(&origem_temp);
  olho_.mutable_alvo()->Swap(&alvo_temp);

  bool colisao = false;
  auto* entidade_alvo = BuscaEntidade(id);
  if ((tipo_objeto == OBJ_ENTIDADE && entidade_alvo != nullptr && entidade_alvo->Proto().causa_colisao()) ||
      (tipo_objeto == OBJ_TABULEIRO)) {
    float x3d, y3d, z3d;
    MousePara3dComProfundidade(2, 2, profundidade, &x3d, &y3d, &z3d);
    //MousePara3dComProfundidade(1, 1, profundidade, &x3d, &y3d, &z3d);
    Vector3 d(x3d, y3d, z3d);
    Vector3 o(x, y, z_olho);
    profundidade = (d - o).length();
    VLOG(2) << "colisao possivel, prof: " << profundidade;
    VLOG(2) << "espaco entidade: " << espaco_entidade;
    if (profundidade < (tamanho_movimento + espaco_entidade)) {
      colisao = true;
      if (profundidade > espaco_entidade) {
        // Anda o que pode ate a extremidade bater na parede.
        tamanho_movimento = profundidade - espaco_entidade;
        if (tamanho_movimento < TAMANHO_LADO_QUADRADO_10) {
          tamanho_movimento = 0.0f;
        }
        VLOG(2) << "reduzindo movimento para " << tamanho_movimento;
      } else {
        // nao da pra andar mais.
        tamanho_movimento = 0.0f;
        VLOG(2) << "reduzindo 2 movimento para zero";
      }
    }
  } else {
    VLOG(2) << "sem colisao, tamanho: " << tamanho_movimento;
  }
  // TODO normal.
  parametros_desenho_.Swap(&pd);
  gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
  return { tamanho_movimento, colisao, Vector3() };
}

bool Tabuleiro::Apoiado(float x, float y, float z_olho, float altura_olho) {
  if (altura_olho <= 0) {
    LOG(WARNING) << "Altura  do olho <= 0: " << altura_olho;
  }
  auto res_colisao = DetectaColisao(x, y, z_olho, 0.0f,
                                    Vector3(0.0f, 0.0f, -altura_olho - 0.5f),
                                    true  /*ignora espaco*/);
  //LOG(INFO) << "distancia apoio: " << res_colisao.profundidade << ", altura olho: " << altura_olho;
  return res_colisao.colisao;
}

Tabuleiro::ResultadoZApoio Tabuleiro::ZApoio(float x, float y, float z_olho, float altura_olho) {
  auto res_colisao = DetectaColisao(x, y, z_olho, 0.0f,
                                    Vector3(0.0f, 0.0f, -fabs(z_olho - ZChao(x, y))),
                                    true  /*ignora espaco*/);
  ResultadoZApoio res;
  VLOG(2) << "res_colisao.profundidade: " << res_colisao.profundidade << ", altura_olho: " << altura_olho;
  res.apoiado = res_colisao.colisao; //res_colisao.profundidade - altura_olho < PRECISAO_APOIO;
  VLOG(2) << "apoiado: " << res.apoiado;
  res.z_apoio = z_olho - res_colisao.profundidade;
  return res;
}

Tabuleiro::ResultadoColisao Tabuleiro::DetectaColisao(const Entidade& entidade, const Vector3& movimento, bool ignora_espaco_entidade) {
  return DetectaColisao(entidade.Pos().x(), entidade.Pos().y(), entidade.ZOlho(), entidade.Espaco(), movimento, ignora_espaco_entidade);
}

void Tabuleiro::TrataEspiada(int espiada) {
  Entidade* entidade = nullptr;
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    entidade = BuscaEntidade(IdCameraPresa());
  }
  if (entidade == nullptr) {
    return;
  }
  EntidadeProto proto;
  int novo = entidade->Proto().espiando() + espiada;
  if (novo > 1) {
    novo = 1;
  } else if (novo < -1) {
    novo = -1;
  }

  if (novo == 0) {
    proto.set_espiando(novo);
    entidade->AtualizaParcial(proto);
    return;
  }

  // Direcao do olhar.
  LOG(INFO) << "olho: " << olho_.ShortDebugString();
  Vector3 vetor_olho(olho_.pos().x(), olho_.pos().y(), olho_.pos().z());
  Vector3 vetor_alvo(olho_.alvo().x(), olho_.alvo().y(), olho_.alvo().z());
  Vector3 vetor_olhar = vetor_alvo - vetor_olho;
  vetor_olhar.normalize();

  // bloquear espiada em caso de colisao
  Vector3 vetor_up(0.0f, 0.0f, 1.0f);
  Vector3 direcao_espiada = vetor_olhar.cross(vetor_up) * novo;
  auto res_colisao  = DetectaColisao(*entidade, direcao_espiada, false  /*ignora*/);
  if (!res_colisao.colisao) {
    proto.set_espiando(novo);
    entidade->AtualizaParcial(proto);
    VLOG(1) << "Espiada ok";
  } else {
    VLOG(1) << "Espiada bloqueada";
  }
}

void Tabuleiro::Hack() {
  for (unsigned int id : ids_entidades_selecionadas_) {
    auto* entidade_selecionada = BuscaEntidade(id);
    if (entidade_selecionada == nullptr) {
      continue;
    }
    //LOG(INFO) << "antes: " << entidade_selecionada->Proto().escala().ShortDebugString();
    EntidadeProto proto;
    proto.mutable_escala()->set_z(entidade_selecionada->Proto().escala().z() + 1.5f);
    entidade_selecionada->AtualizaParcial(proto);
    //LOG(INFO) << "depois: " << entidade_selecionada->Proto().escala().ShortDebugString();
  }
}

void Tabuleiro::AdicionaLogEvento(const std::string& evento) {
  if (evento.empty()) {
    return;
  }
  log_eventos_.push_front(evento);
  if (log_eventos_.size() > 30) {
    log_eventos_.pop_back();
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
  AdicionaLogEvento(ResumoNotificacao(*this, notificacao));

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
    case ntf::TN_ATUALIZAR_LISTA_INICIATIVA:
      n_inversa.set_tipo(ntf::TN_ATUALIZAR_LISTA_INICIATIVA);
      *n_inversa.mutable_tabuleiro() = n_original.tabuleiro_antes();
      break;
    case ntf::TN_REINICIAR_CAMERA:
      n_inversa.set_tipo(ntf::TN_REINICIAR_CAMERA);
      *n_inversa.mutable_tabuleiro()->mutable_camera_inicial() = n_original.tabuleiro_antes().camera_inicial();
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
      if (!n_original.entidade_antes().has_pos() || !n_original.entidade().has_id()) {
        LOG(ERROR) << "Impossivel inverter ntf::TN_MOVER_ENTIDADE sem a posicao original ou ID: "
                   << n_original.entidade().ShortDebugString();
        break;
      }
      n_inversa.set_tipo(ntf::TN_MOVER_ENTIDADE);
      // Usa o destino.
      *n_inversa.mutable_entidade()->mutable_pos() = n_original.entidade_antes().pos();
      n_inversa.mutable_entidade()->set_id(n_original.entidade().id());
      if (n_original.entidade_antes().has_apoiada()) {
        n_inversa.mutable_entidade()->set_apoiada(n_original.entidade_antes().apoiada());
      }
      if (n_original.tabuleiro_antes().has_camera_inicial()) {
        *n_inversa.mutable_tabuleiro()->mutable_camera_inicial() = n_original.tabuleiro_antes().camera_inicial();
      }
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
        (!EmModoMestreIncluindoSecundario() && !entidade->SelecionavelParaJogador())) {
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
  bool apoiada_antes = entidade->Apoiada();
  if (proto.has_destino()) {
    entidade->Destino(proto.destino());
  } else {
    entidade->MovePara(proto.pos());
  }
  if (proto.has_apoiada()) {
    entidade->Apoia(proto.apoiada());
  }
  if (notificacao.local()) {
    central_->AdicionaNotificacaoRemota(new ntf::Notificacao(notificacao));
    // Para desfazer: salva a posicao original e destino.
    ntf::Notificacao n_desfazer;
    n_desfazer.set_tipo(ntf::TN_MOVER_ENTIDADE);
    n_desfazer.mutable_entidade()->set_id(entidade->Id());
    n_desfazer.mutable_entidade_antes()->set_id(entidade->Id());
    n_desfazer.mutable_entidade_antes()->mutable_pos()->CopyFrom(entidade->Proto().pos());
    n_desfazer.mutable_entidade()->mutable_destino()->CopyFrom(proto.pos());
    if (proto.has_apoiada()) {
      n_desfazer.mutable_entidade()->set_apoiada(entidade->Apoiada());
      n_desfazer.mutable_entidade_antes()->set_apoiada(apoiada_antes);
    }
    AdicionaNotificacaoListaEventos(n_desfazer);
  }
  if (entidade->Tipo() != TE_ENTIDADE) {
    luzes_pontuais_.clear();
  }
}

void Tabuleiro::RemoveEntidadeNotificando(unsigned int id_remocao) {
  auto* entidade = BuscaEntidade(id_remocao);
  if (entidade == nullptr) {
    return;
  }
  if (entidade->Tipo() != TE_ENTIDADE) {
    luzes_pontuais_.clear();
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

void Tabuleiro::AtualizaLuzesPontuais() {
  // Posiciona as luzes dinâmicas.
  std::vector<const Entidade*> entidades_com_luz;
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    auto* e = it->second.get();
    if (e == nullptr || e->IdCenario() != proto_corrente_->id_cenario() || !e->Proto().has_luz()) {
      continue;
    }
    entidades_com_luz.push_back(e);
  }
  if (entidades_com_luz.empty()) {
    return;
  }
  // Posicao para comparacao.
  Vector3 pos_comp;
  const auto* entidade_presa = BuscaEntidade(IdCameraPresa());
  if (entidade_presa != nullptr) {
    pos_comp = PosParaVector3(entidade_presa->Pos());
    pos_comp.z += entidade_presa->AlturaOlho();
  } else {
    pos_comp = PosParaVector3(olho_.pos());
  }
  // Ordena as luzes pela distancia.
  std::sort(entidades_com_luz.begin(), entidades_com_luz.end(), [entidade_presa, &pos_comp] (const Entidade* lhs, const Entidade* rhs) {
    if (lhs == entidade_presa) {
      return true;
    } else if (rhs == entidade_presa) {
      return false;
    }
    float ld = (PosParaVector3(lhs->Pos()) - pos_comp).length();
    float rd = (PosParaVector3(rhs->Pos()) - pos_comp).length();
    return ld < rd;
  });
  // Quando entidade estiver travada, ela tera oclusao, portanto, usa a segunda luz se houver.
  if (entidade_presa != nullptr && entidades_com_luz[0] == entidade_presa && entidades_com_luz.size() > 1) {
    std::swap(entidades_com_luz[0], entidades_com_luz[1]);
  }

  bool atualizar_mapa = false;
  if (!luzes_pontuais_.empty()) {
    Posicao pos = entidades_com_luz[0]->Pos();
    pos.set_z(entidades_com_luz[0]->ZOlho());
    Vector3 pv = PosParaVector3(pos);
    Vector3 pt = PosParaVector3(luzes_pontuais_[0].pos);
    if ((pv - pt).length() > TAMANHO_LADO_QUADRADO_10 || luzes_pontuais_[0].id != entidades_com_luz[0]->Id()) {
      atualizar_mapa = true;
    }
  } else {
    atualizar_mapa = true;
  }
  unsigned int num = std::min((unsigned int)8, (unsigned int)entidades_com_luz.size());
  luzes_pontuais_.resize(num);
  for (unsigned int i = 0; i < num; ++i) {
    luzes_pontuais_[i].id = entidades_com_luz[i]->Id();
    if (atualizar_mapa || i > 0) {
      luzes_pontuais_[i].pos = entidades_com_luz[i]->Pos();
      luzes_pontuais_[i].pos.set_z(entidades_com_luz[i]->ZOlho());
    }
  }

  if (!atualizar_mapa) {
    return;
  }
  //LOG(INFO) << "atualizando mapa de luz";
  GLint original;
  gl::Le(GL_FRAMEBUFFER_BINDING, &original);
  ParametrosDesenho salva_pd(parametros_desenho_);
  DesenhaMapaLuz();
  parametros_desenho_.set_desenha_mapa_luzes(0);
  // TODO XXX desenhar apenas as entidades fixas.
  // Restaura os valores e usa a textura como mapa de luz.
  // Note que ao mudar o shader, o valor do plano distante de corte para oclusao permanecera o mesmo.
  gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
  gl::LigacaoComFramebuffer(GL_FRAMEBUFFER, original);
  gl::UnidadeTextura(GL_TEXTURE4);
  gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, dfb_luzes_[0].textura);
  gl::UnidadeTextura(GL_TEXTURE0);
  gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
  gl::Desabilita(GL_TEXTURE_2D);
  parametros_desenho_ = salva_pd;
}

void Tabuleiro::DesenhaLuzes() {
  // Entidade de referencia para camera presa.
  parametros_desenho_.clear_nevoa();
  auto* entidade_referencia = BuscaEntidade(IdCameraPresa());
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
  if (IluminacaoMestre() && !opcoes_.iluminacao_mestre_igual_jogadores()) {
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
    gl::MatrizEscopo salva_matriz;
    // O vetor inicial esta no leste (origem da luz). O quarte elemento indica uma luz no infinito.
    GLfloat pos_luz[] = { 1.0, 0.0f, 0.0f, 0.0f };
    // Roda no eixo Z (X->Y) em direcao a posicao entao inclina a luz no eixo -Y (de X->Z).
    gl::Roda(proto_corrente_->luz_direcional().posicao_graus(), 0.0f, 0.0f, 1.0f);
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
      (!IluminacaoMestre() || opcoes_.iluminacao_mestre_igual_jogadores())) {
    gl::Habilita(GL_FOG);
    float pos[4] = { olho_.pos().x(), olho_.pos().y(), olho_.pos().z(), 1 };
    if (entidade_referencia != nullptr) {
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
  for (const auto& luz : luzes_pontuais_) {
    auto* e = BuscaEntidade(luz.id);
    if (e != nullptr) {
      e->DesenhaLuz(&parametros_desenho_);
    }
  }
}

void Tabuleiro::DesenhaCaixaCeu() {
#if 1
  gl::TipoShader tipo_anterior = gl::TipoShaderCorrente();
  gl::UsaShader(gl::TSH_CAIXA_CEU);
  GLuint id_textura = texturas_->Textura(proto_corrente_->info_textura_ceu().id());
  GLenum tipo_textura = texturas_->TipoTextura(proto_corrente_->info_textura_ceu().id());
  if (!proto_corrente_->aplicar_luz_ambiente_textura_ceu()) {
    MudaCor(COR_BRANCA);
  } else {
    MudaCor(proto_corrente_->luz_ambiente());
  }

  gl::MatrizEscopo salva_mv(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::Translada(olho_.pos().x(), olho_.pos().y(), olho_.pos().z());

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
  gl::FaceNula(GL_BACK);
  gl::FuncaoProfundidade(GL_LESS);
  gl::UsaShader(tipo_anterior);
#else
  // Hack para ver textura de cubo.
  if (!gl::OclusaoLigada()) {
    return;
  }
  gl::TipoShader tipo_anterior = gl::TipoShaderCorrente();
  gl::UsaShader(gl::TSH_CAIXA_CEU);
  gl::MatrizEscopo salva_mv(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::Translada(0.0f, 0.0f, 5.0f);

  //gl::DesabilitaEscopo profundidade_escopo(GL_DEPTH_TEST);
  //gl::DesligaEscritaProfundidadeEscopo desliga_escrita_escopo;
  gl::UnidadeTextura(GL_TEXTURE2);
  gl::Habilita(GL_TEXTURE_CUBE_MAP);
  gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, textura_framebuffer_oclusao_);
  vbo_caixa_ceu_.forca_texturas(true);

  MudaCor(COR_BRANCA);
  gl::CuboUnitario();
  gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, 0);
  gl::Desabilita(GL_TEXTURE_CUBE_MAP);
  gl::UnidadeTextura(GL_TEXTURE0);
  // Religa luzes.
  gl::UsaShader(tipo_anterior);
#endif
}

void Tabuleiro::DesenhaGrade() {
  gl::DesligaEscritaProfundidadeEscopo desliga_escrita_escopo;
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
  MudaCor(COR_PRETA);
  gl::DesvioProfundidade(OFFSET_GRADE_ESCALA_DZ, OFFSET_GRADE_ESCALA_R);
  vbos_grade_.Desenha();
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
    PosicionaRaster2d(raster_x, raster_y);
  }

  MudaCor(COR_BRANCA);
  if (!parametros_desenho_.has_picking_x()) {
    std::string titulo("Lista Jogadores");
    gl::DesenhaStringAlinhadoEsquerda(titulo);
  }
  raster_y -= (altura_fonte + 2);
  // Lista de objetos.
  for (const auto& par : clientes_) {
    if (!parametros_desenho_.has_picking_x()) {
      PosicionaRaster2d(raster_x, raster_y);
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
  raster_x = 0 + 2;
  if (!parametros_desenho_.has_picking_x()) {
    PosicionaRaster2d(raster_x, raster_y);
  }
  MudaCor(COR_BRANCA);
  if (!parametros_desenho_.has_picking_x()) {
    std::string titulo("Lista Objetos");
    gl::DesenhaStringAlinhadoEsquerda(titulo);
  }
  raster_y -= (altura_fonte + 2);
  // Paginacao inicial.
  if (pagina_corrente > 0) {
    gl::TipoEscopo tipo(OBJ_CONTROLE_VIRTUAL);
    gl::CarregaNome(CONTROLE_PAGINACAO_LISTA_OBJETOS_CIMA);
    {
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x, raster_y, raster_x + (3 * largura_fonte), raster_y + altura_fonte);
    }
    if (!parametros_desenho_.has_picking_x()) {
      MudaCor(COR_AZUL);
      PosicionaRaster2d(raster_x, raster_y);
      std::string page_up("^^^");
      gl::DesenhaStringAlinhadoEsquerda(page_up);
    }
  }
  // Pula independente de ter paginacao pra ficar fixa a posicao dos objetos.
  raster_y -= (altura_fonte + 2);

  // Lista de objetos.
  for (int i = objeto_inicial; i < objeto_final; ++i) {
    const auto* e = entidades_cenario[i];
    if (!parametros_desenho_.has_picking_x()) {
      PosicionaRaster2d(raster_x, raster_y);
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
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x, raster_y, raster_x + (strlen(str) * largura_fonte), raster_y + altura_fonte);
    }
    MudaCor(COR_AZUL);
    if (!parametros_desenho_.has_picking_x()) {
      gl::DesenhaStringAlinhadoEsquerda(str);
    }
    raster_y -= (altura_fonte + 2);
  }

  // Paginacao final.
  if (pagina_corrente < (num_paginas - 1)) {
    gl::TipoEscopo tipo(OBJ_CONTROLE_VIRTUAL);
    gl::CarregaNome(CONTROLE_PAGINACAO_LISTA_OBJETOS_BAIXO);
    {
      MudaCor(COR_BRANCA);
      gl::Retangulo(raster_x, raster_y, raster_x + (3 * largura_fonte), raster_y + altura_fonte);
    }
    if (!parametros_desenho_.has_picking_x()) {
      MudaCor(COR_AZUL);
      PosicionaRaster2d(raster_x, raster_y);
      std::string page_down("vvv");
      gl::DesenhaStringAlinhadoEsquerda(page_down);
    }
    raster_y -= (altura_fonte + 2);
  }
}

void Tabuleiro::DesenhaLogEventos() {
  // Quadrado preto de 8 linhas.
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  int largura_fonte, altura_fonte, escala;
  int kTamanhoMaximoCaracteres  = 100;
  int kNumLinhas = 8;  // uma de titulo e duas de paginacoes.
  // TODO paginacao.
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  int kTamanhoMaximoPixels = largura_fonte * kTamanhoMaximoCaracteres;

  MudaCor(COR_PRETA);
  gl::Retangulo(0, 0, kTamanhoMaximoPixels, (altura_fonte * escala) * 8);

  MudaCor(COR_AMARELA);
  int raster_x = kTamanhoMaximoPixels / 2, raster_y =  (altura_fonte * escala) * (kNumLinhas - 1);
  PosicionaRaster2d(raster_x, raster_y);
  gl::DesenhaString("Lista de Eventos");
  raster_y = (altura_fonte * escala) * (kNumLinhas - 3);
  auto it = log_eventos_.begin();
  for (int i = 0; i < (kNumLinhas - 3) && it != log_eventos_.end(); ++it) {
    PosicionaRaster2d(2, raster_y);
    char s[100];
    snprintf(s, 99, "%s", it->c_str());
    gl::DesenhaStringAlinhadoEsquerda(s);
    raster_y -= altura_fonte;
    ++i;
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

  gl::DesenhaString(id_acao);
}

void Tabuleiro::DesenhaCoordenadas() {
  if (!VisaoMestre()) {
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

  gl::DesenhaString(coordenadas);
}

void Tabuleiro::DesenhaInfoGeral() {
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);
  gl::MatrizEscopo salva_matriz_mv(gl::MATRIZ_MODELAGEM_CAMERA);
  gl::CarregaIdentidade();
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;
  int raster_y = altura_ - (altura_fonte);
  int raster_x = largura_ / 2;
  gl::PosicaoRaster(raster_x, raster_y);
  MudaCor(COR_BRANCA);
  if (!info_geral_.empty()) {
    gl::DesenhaString(info_geral_);
    return;
  }
  switch (estado_) {
    case ETAB_ENTS_SELECIONADAS:
      DesenhaIdAcaoEntidade();
      break;
    case ETAB_QUAD_PRESSIONADO:
    case ETAB_QUAD_SELECIONADO:
      DesenhaCoordenadas();
      break;
    default:
      ;
  }
}

void Tabuleiro::DesenhaTempo(int linha, const std::string& prefixo, const std::list<uint64_t>& ultimos_tempos) {
  // Acha o maior.
  uint64_t maior_tempo_ms = 0;
  for (uint64_t tempo_ms : ultimos_tempos) {
    if (tempo_ms > maior_tempo_ms) {
      maior_tempo_ms = tempo_ms;
    }
  }

  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(largura_, altura_, &largura_fonte, &altura_fonte, &escala);
  largura_fonte *= escala;
  altura_fonte *= escala;

  std::string tempo_str(prefixo);
  tempo_str.append(": ");
  tempo_str.append(net::to_string((int)maior_tempo_ms));
  while (tempo_str.size() < 4) {
    tempo_str.insert(0, "0");
  }

  int yi = altura_ - altura_fonte * (linha + 1);
  int ys = yi + altura_fonte;
  // Modo 2d.
  {
    MudaCor(COR_PRETA);
    gl::MatrizEscopo salva_matriz_mv(gl::MATRIZ_MODELAGEM_CAMERA);
    gl::CarregaIdentidade();
    gl::AtualizaMatrizes();
    gl::Retangulo(0.0f, yi, tempo_str.size() * largura_fonte + 2.0f, ys);
  }
  // Eixo com origem embaixo esquerda.
  PosicionaRaster2d(2, yi);
  MudaCor(COR_BRANCA);
  gl::DesenhaStringAlinhadoEsquerda(tempo_str);
}

void Tabuleiro::DesenhaTempos() {
  gl::DesligaEscritaProfundidadeEscopo profundidade_escopo;
  gl::DesabilitaEscopo luz_escopo(GL_LIGHTING);

  DesenhaTempo(0, "T", tempos_entre_cenas_);
  DesenhaTempo(1, "M", tempos_renderizacao_mapas_);
  DesenhaTempo(2, "R", tempos_uma_renderizacao_completa_);
  DesenhaTempo(3, "A", tempos_uma_atualizacao_);
  DesenhaTempo(4, "B", tempos_atualiza_parcial_);
  DesenhaTempo(5, "C", tempos_uma_renderizacao_controle_virtual_);
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
    ReativaWatchdog();
  } else {
    DesativaWatchdog();
  }
#endif
}

const std::vector<unsigned int> Tabuleiro::EntidadesAfetadasPorAcao(const AcaoProto& acao) {
  std::vector<unsigned int> ids_afetados;
  //const Posicao& pos_tabuleiro = acao.pos_tabuleiro();
  //const Posicao& pos_destino = acao.pos_entidade();
  const Entidade* entidade_origem = BuscaEntidade(acao.id_entidade_origem());
  int cenario_origem =  (entidade_origem != nullptr) ? entidade_origem->IdCenario() : cenario_corrente_;
  std::vector<const Entidade*> entidades_cenario;
  Posicao pos_origem;
  if (entidade_origem != nullptr) {
    pos_origem = entidade_origem->PosicaoAcao();
  }
  for (const auto& id_entidade_destino : entidades_) {
    auto* entidade = id_entidade_destino.second.get();
    if (entidade->IdCenario() == cenario_origem) {
      //entidades_cenario.push_back(entidade);
      if (Acao::PontoAfetadoPorAcao(entidade->PosicaoAcao(), pos_origem, acao)) {
        ids_afetados.push_back(id_entidade_destino.first);
      }
    }
  }
#if 0
  if (acao.tipo() == ACAO_DISPERSAO) {
    switch (acao.geometria()) {
      case ACAO_GEO_ESFERA: {
        for (const auto* entidade_destino : entidades_cenario) {
          // Usa a posicao da acao para ver se pegou.
          Posicao pos_entidade_destino(entidade_destino->PosicaoAcao());
          float d2 = DistanciaQuadrado(pos_tabuleiro, pos_entidade_destino);
          if (d2 <= powf(acao.raio_quadrados() * TAMANHO_LADO_QUADRADO, 2)) {
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
          if (entidade_destino != entidade_origem && PontoDentroDePoligono(entidade_destino->Pos(), vertices)) {
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

#endif
  return ids_afetados;
}

std::vector<unsigned int> Tabuleiro::IdsPrimeiraPessoaOuEntidadesSelecionadas() const {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return { IdCameraPresa() };
  } else {
    return std::vector<unsigned int>(ids_entidades_selecionadas_.begin(), ids_entidades_selecionadas_.end());
  }
}

std::vector<unsigned int> Tabuleiro::IdsPrimeiraPessoaIncluindoEntidadesSelecionadas() const {
  std::vector<unsigned int> ids(ids_entidades_selecionadas_.begin(), ids_entidades_selecionadas_.end());
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    ids.push_back(IdCameraPresa());
  }
  return ids;
}

std::vector<unsigned int> Tabuleiro::IdsEntidadesSelecionadasOuPrimeiraPessoa() const {
  if (ids_entidades_selecionadas_.empty()) {
    if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
      return { IdCameraPresa() };
    }
  }
  return std::vector<unsigned int>(ids_entidades_selecionadas_.begin(), ids_entidades_selecionadas_.end());
}

Entidade* Tabuleiro::EntidadePrimeiraPessoaOuSelecionada() {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(IdCameraPresa());
  } else {
    return EntidadeSelecionada();
  }
}

const Entidade* Tabuleiro::EntidadePrimeiraPessoaOuSelecionada() const {
  if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(IdCameraPresa());
  } else {
    return EntidadeSelecionada();
  }
}

const Entidade* Tabuleiro::EntidadeSelecionadaOuPrimeiraPessoa() const {
  if (ids_entidades_selecionadas_.size() == 1) {
    return EntidadeSelecionada();
  } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(IdCameraPresa());
  }
  return nullptr;
}

Entidade* Tabuleiro::EntidadeSelecionadaOuPrimeiraPessoa() {
  if (ids_entidades_selecionadas_.size() == 1) {
    return EntidadeSelecionada();
  } else if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
    return BuscaEntidade(IdCameraPresa());
  }
  return nullptr;
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

void Tabuleiro::PassaUmaRodadaNotificando(ntf::Notificacao* grupo) {
  if (!EmModoMestreIncluindoSecundario()) {
    return;
  }
  ntf::Notificacao alias_grupo;
  ntf::Notificacao& grupo_notificacoes = (grupo == nullptr) ? alias_grupo : *grupo;
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

  if (grupo == nullptr) {
    TrataNotificacao(grupo_notificacoes);
    AdicionaNotificacaoListaEventos(grupo_notificacoes);
  }
}

void Tabuleiro::ZeraRodadasNotificando() {
  if (!EmModoMestreIncluindoSecundario()) {
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

void Tabuleiro::AlternaModoD20() {
  if (modo_clique_ == MODO_D20) {
    modo_clique_ = MODO_NORMAL;
  } else {
    modo_clique_ = MODO_D20;
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
  if (!EmModoMestreIncluindoSecundario()) {
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
  // Muda para o cenario caso nao seja o corrente.
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
  if (camera_presa_) {
    AlternaCameraPresa();
  } else {
    camera_ = CAMERA_PERSPECTIVA;
    camera_presa_ = false;
  }
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

void Tabuleiro::ReiniciaCamera(const ntf::Notificacao& notificacao) {
  if (notificacao.tabuleiro().camera_inicial().alvo().id_cenario() != proto_corrente_->id_cenario()) {
    LOG(INFO) << "Carregando cenario " << notificacao.tabuleiro().camera_inicial().alvo().id_cenario();
    CarregaSubCenario(notificacao.tabuleiro().camera_inicial().alvo().id_cenario(), notificacao.tabuleiro().camera_inicial().alvo());
  } else {
    LOG(INFO) << "Mantendo cenario: " << notificacao.ShortDebugString();
  }
  olho_ = notificacao.tabuleiro().camera_inicial();
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
  olho_.set_altura(OLHO_ALTURA_INICIAL);
  AtualizaOlho(0, true);
}

void Tabuleiro::AlternaCameraPresa() {
  auto EntidadeParaAdicionar = [this] () {
    std::vector<unsigned int> entidades_a_adicionar;
    for (unsigned int id : ids_entidades_selecionadas_) {
      const Entidade* entidade = BuscaEntidade(id);
      if (entidade != nullptr && entidade->Tipo() == TE_ENTIDADE) {
        entidades_a_adicionar.push_back(id);
      }
    }
    return entidades_a_adicionar;
  };
  std::vector<unsigned int> entidades_a_adicionar = EntidadeParaAdicionar();
  if (camera_presa_ && entidades_a_adicionar.empty()) {
    camera_presa_ = false;
    ids_camera_presa_.clear();
    LOG(INFO) << "Camera solta.";
    if (camera_ == CAMERA_PRIMEIRA_PESSOA) {
      AlternaCameraPrimeiraPessoa();
    }
  } else {
    for (unsigned int id : entidades_a_adicionar) {
      if (std::find(ids_camera_presa_.begin(), ids_camera_presa_.end(), id) == ids_camera_presa_.end()) {
        ids_camera_presa_.push_back(id);
      }
    }
  }

  if (!ids_camera_presa_.empty()) {
    camera_presa_ = true;
    LOG(INFO) << "Camera presa.";
  } else {
    LOG(INFO) << "Sem entidade selecionada, nada a fazer.";
  }
}

void Tabuleiro::MudaEntidadeCameraPresa() {
  if (!camera_presa_ || ids_camera_presa_.size() <= 1) {
    LOG(INFO) << "Nao posso alternar camera, camera_presa_ " << camera_presa_
              << ", ids_entidades_selecionadas_.size(): " << ids_entidades_selecionadas_.size();
    return;
  }

  unsigned int primeiro = ids_camera_presa_.front();
  LOG(INFO) << "Alternando id camera presa de " << primeiro;
  ids_camera_presa_.pop_front();

  const Entidade* entidade = BuscaEntidade(ids_camera_presa_.front());
  for (; entidade == nullptr && !ids_camera_presa_.empty(); entidade = BuscaEntidade(ids_camera_presa_.front())) {
    LOG(INFO) << "Alternando para entidada nao existente " << ids_camera_presa_.front();
    ids_camera_presa_.pop_front();
  }
  if (entidade == nullptr) {
    LOG(INFO) << "Nao ha outra entidade para prender, retornando a primeira.";
  } else {
    if (entidade->Pos().id_cenario() != cenario_corrente_) {
      CarregaSubCenario(entidade->Pos().id_cenario(), entidade->Pos());
    }
  }
  ids_camera_presa_.push_back(primeiro);
  LOG(INFO) << "Camera presa em " << ids_camera_presa_.front();
  SelecionaEntidade(ids_camera_presa_.front());
}

void Tabuleiro::DesativaWatchdog() {
#if USAR_WATCHDOG
  watchdog_.Para();
#endif
}

void Tabuleiro::ReativaWatchdog() {
#if USAR_WATCHDOG
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
