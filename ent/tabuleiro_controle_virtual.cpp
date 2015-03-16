// Coisas relacionadas a controle virtual.
#include <algorithm>
#include <boost/filesystem.hpp>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <functional>
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

// Constantes do controle virtual.
const int CONTROLE_ACAO = 1;
const int CONTROLE_ACAO_ANTERIOR = 2;
const int CONTROLE_ACAO_PROXIMA = 3;
const int CONTROLE_ADICIONA_1 = 4;
const int CONTROLE_ADICIONA_5 = 5;
const int CONTROLE_ADICIONA_10 = 6;
const int CONTROLE_CONFIRMA_DANO = 7;
const int CONTROLE_APAGA_DANO = 8;
const int CONTROLE_ALTERNA_CURA = 9;
const int CONTROLE_DESFAZER = 10;
const int CONTROLE_VOO = 11;
const int CONTROLE_VISIBILIDADE = 12;
const int CONTROLE_QUEDA = 13;
const int CONTROLE_LUZ = 14;
const int CONTROLE_RODADA = 15;
const int CONTROLE_CAMERA_ISOMETRICA = 16;
const int CONTROLE_CAMERA_PRESA = 17;
const int CONTROLE_CIMA = 18;
const int CONTROLE_BAIXO = 19;
const int CONTROLE_ESQUERDA = 20;
const int CONTROLE_DIREITA = 21;

// Texturas do controle virtual.
const char* TEXTURA_ACAO = "icon_sword.png";
const char* TEXTURA_VOO = "icon_feather.png";
const char* TEXTURA_VISIBILIDADE = "icon_hide.png";
const char* TEXTURA_LUZ = "icon_light.png";
const char* TEXTURA_QUEDA = "icon_slide.png";
const char* TEXTURA_CAMERA_ISOMETRICA = "icon_isometric_camera.png";
const char* TEXTURA_CAMERA_PRESA = "icon_tracking_camera.png";
const char* TEXTURA_DESFAZER = "icon_undo.png";
const std::vector<std::string> g_texturas = { TEXTURA_ACAO, TEXTURA_VOO, TEXTURA_VISIBILIDADE, TEXTURA_LUZ, TEXTURA_QUEDA,
                                              TEXTURA_CAMERA_ISOMETRICA, TEXTURA_CAMERA_PRESA, TEXTURA_DESFAZER };

// Para botoes sem estado.
bool RetornaFalse() {
  return false;
}

}  // namespace.

void Tabuleiro::CarregaTexturasControleVirtual() {
  for (const std::string& textura : g_texturas) {
    auto* n = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
    n->mutable_info_textura()->set_id(textura);
    central_->AdicionaNotificacao(n);
  }
}

void Tabuleiro::LiberaTexturasControleVirtual() {
  for (const std::string& textura : g_texturas) {
    auto* n = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
    n->mutable_info_textura()->set_id(textura);
    central_->AdicionaNotificacao(n);
  }
}

void Tabuleiro::PickingControleVirtual(bool alterna_selecao, int id) {
  contador_pressao_por_controle_[id]++;
  switch (id) {
    case CONTROLE_ACAO:
      AlternaModoAcao();
      break;
    case CONTROLE_CAMERA_ISOMETRICA:
      AlternaCameraIsometrica();
      break;
    case CONTROLE_CAMERA_PRESA:
      AlternaCameraPresa();
      break;
    case CONTROLE_CIMA:
      TrataMovimentoEntidadesSelecionadas(true, 1.0f);
      break;
    case CONTROLE_BAIXO:
      TrataMovimentoEntidadesSelecionadas(true, -1.0f);
      break;
    case CONTROLE_ESQUERDA:
      TrataMovimentoEntidadesSelecionadas(false, -1.0f);
      break;
    case CONTROLE_DIREITA:
      TrataMovimentoEntidadesSelecionadas(false, 1.0f);
      break;
    case CONTROLE_ACAO_ANTERIOR:
      AcaoAnterior();
      break;
    case CONTROLE_ACAO_PROXIMA:
      ProximaAcao();
      break;
    case CONTROLE_ADICIONA_1:
      AlteraUltimoPontoVidaListaPontosVida(modo_acao_cura_ ? 1 : -1);
      break;
    case CONTROLE_ADICIONA_5:
      AlteraUltimoPontoVidaListaPontosVida(modo_acao_cura_ ? 5 : -5);
      break;
    case CONTROLE_ADICIONA_10:
      AlteraUltimoPontoVidaListaPontosVida(modo_acao_cura_ ? 10 : -10);
      break;
    case CONTROLE_CONFIRMA_DANO:
      AcumulaPontosVida({0});
      break;
    case CONTROLE_APAGA_DANO:
      LimpaUltimoListaPontosVida();
      break;
    case CONTROLE_ALTERNA_CURA:
      AlternaUltimoPontoVidaListaPontosVida();
      break;
    case CONTROLE_LUZ:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ILUMINACAO);
      break;
    case CONTROLE_QUEDA:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_CAIDA);
      break;
    case CONTROLE_VOO:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VOO);
      break;
    case CONTROLE_VISIBILIDADE:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VISIBILIDADE);
      break;
    case CONTROLE_DESFAZER:
      if (!alterna_selecao) {
        TrataComandoDesfazer();
      } else {
        TrataComandoRefazer();
      }
      break;
    case CONTROLE_RODADA:
      if (!alterna_selecao) {
        PassaUmaRodadaNotificando();
      } else {
        ZeraRodadasNotificando();
      }
      break;
    default:
      LOG(WARNING) << "Controle invalido: " << id;
  }
}

void Tabuleiro::DesenhaControleVirtual() {
  gl::Desabilita(GL_LIGHTING);
  gl::Desabilita(GL_DEPTH_TEST);
  float cor_padrao[3];
  float cor_ativa[3];
  cor_padrao[0] = 0.8f;
  cor_padrao[1] = 0.8f;
  cor_padrao[2] = 0.8f;
  cor_ativa[0] = 0.4f;
  cor_ativa[1] = 0.4f;
  cor_ativa[2] = 0.4f;

  int fonte_x_int, fonte_y_int;
  gl::TamanhoFonte(&fonte_x_int, &fonte_y_int);
  const float fonte_x = fonte_x_int;
  const float fonte_y = fonte_y_int;
  const float botao_y = fonte_y * 2.5f;
  //const float botao_x = botao_y;  // botoes quadrados. Era: fonte_x * 3.0f;
  const float botao_x = fonte_x * 3.0f;
  const float padding = parametros_desenho_.has_picking_x() ? 0 : fonte_x / 2;

  // Todos os botoes tem tamanho baseado no tamanho da fonte.
  struct DadosBotao {
    int tamanho;  // 1 eh base, 2 eh duas vezes maior.
    int linha;    // Em qual linha esta a base do botao (0 ou 1)
    int coluna;   // Em qual coluna esta a esquerda do botao.
    std::string rotulo;
    const float* cor_rotulo;   // cor do rotulo.
    std::string textura;  // Se o botao tiver icone.
    int id;  // Identifica o que o botao faz, ver pos_pilha == 4 para cada id.
    std::function<bool()> estado_botao;  // Funcao que retorna o estado botao (true para apertado).
    int num_lados_botao;  // numero de lados do botao,.
    float rotacao_graus;  // Rotacao do botao.
    float translacao_x;  // Translacao do desenho em fator de escala da fonte (botao_x * translacao_x)
    float translacao_y;  // Translacao do desenho em fator de escala da fonte (botao_y * translacao_x).
  };
  const std::vector<DadosBotao> dados_botoes = {
    // Botoes grandes.
    // Acao.
    { 2, 0, 0, "A", nullptr, TEXTURA_ACAO, CONTROLE_ACAO, [this] () { return modo_acao_; } , 4, 0.0f, 0.0f, 0.0f },
    // Linha de cima.
    // Alterna acao para tras.
    { 1, 1, 2, "<", nullptr, "", CONTROLE_ACAO_ANTERIOR, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Alterna acao para frente.
    { 1, 1, 3, ">", nullptr, "", CONTROLE_ACAO_PROXIMA, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Alterna cura.
    { 1, 1, 4, "+-", modo_acao_cura_ ? COR_VERMELHA : COR_VERDE, "", CONTROLE_ALTERNA_CURA, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Linha de baixo
    // Adiciona dano +1.
    { 1, 0, 2, "1", nullptr, "", CONTROLE_ADICIONA_1, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Adiciona dano +5
    { 1, 0, 3, "5", nullptr, "", CONTROLE_ADICIONA_5, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Adiciona dano +10.
    { 1, 0, 4, "10", nullptr, "", CONTROLE_ADICIONA_10, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Confirma dano.
    { 1, 0, 5, "v", COR_AZUL, "", CONTROLE_CONFIRMA_DANO, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    // Apaga dano.
    { 1, 0, 6, "x", nullptr, "", CONTROLE_APAGA_DANO, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },

    // Status.
    { 1, 0, 8, "L", COR_AMARELA, TEXTURA_LUZ, CONTROLE_LUZ, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    { 1, 0, 9, "Q", nullptr, TEXTURA_QUEDA, CONTROLE_QUEDA, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    { 1, 1, 8, "Vo", nullptr, TEXTURA_VOO, CONTROLE_VOO, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },
    { 1, 1, 9, "Vi", nullptr, TEXTURA_VISIBILIDADE, CONTROLE_VISIBILIDADE, RetornaFalse, 4, 0.0f, 0.0f, 0.0f },

    // Setas.
    { 1, 1, 11, "", nullptr, "", CONTROLE_CIMA,     RetornaFalse, 3, 0.0f,   0.0f,  -0.2f },
    { 1, 0, 11, "", nullptr, "", CONTROLE_BAIXO,    RetornaFalse, 3, 180.0f, 0.0f,  0.2f },
    { 1, 0, 10, "", nullptr, "", CONTROLE_ESQUERDA, RetornaFalse, 3, 90.0f,  0.4f,  0.5f },
    { 1, 0, 12, "", nullptr, "", CONTROLE_DIREITA,  RetornaFalse, 3, -90.0f, -0.4f, 0.5f },

    // Cameras.
    { 1, 0, 13, "Is", nullptr, TEXTURA_CAMERA_ISOMETRICA, CONTROLE_CAMERA_ISOMETRICA, [this] () { return this->camera_isometrica_; }, 4, 0.0f, 0.0f, 0.0f },
    { 1, 1, 13, "Pr", nullptr, TEXTURA_CAMERA_PRESA,      CONTROLE_CAMERA_PRESA,      [this] () { return this->camera_presa_; },      4, 0.0f, 0.0f, 0.0f },

    // Desfazer.
    { 2, 0, 14, "<=", COR_VERMELHA, TEXTURA_DESFAZER, CONTROLE_DESFAZER, RetornaFalse, 4, 30.0f, 0.0f, 0.0f },

    // Contador de rodadas.
    { 2, 0, 16, net::to_string(proto_.contador_rodadas()), nullptr, "", CONTROLE_RODADA, RetornaFalse, 8, 0.0f, 0.0f, 0.0f },
  };
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  // Desenha em duas passadas por causa da limitacao de projecao do nexus 7.
  // Desenha apenas os botoes.
  {
    // Modo 2d: eixo com origem embaixo esquerda.
    gl::MatrizEscopo salva_matriz(GL_PROJECTION);
    gl::CarregaIdentidade();
    if (parametros_desenho_.has_picking_x()) {
      // Modo de picking faz a matriz de picking para projecao ortogonal.
      gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
    }
    gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
    gl::MatrizEscopo salva_matriz_2(GL_MODELVIEW);
    gl::CarregaIdentidade();
    for (const DadosBotao& db : dados_botoes) {
      gl::CarregaNome(db.id);
      auto res = contador_pressao_por_controle_.find(db.id);
      bool pressionado = false;
      if (res != contador_pressao_por_controle_.end()) {
        int& num_frames = res->second;
        pressionado = (num_frames > 0);
        if (pressionado) {
          ++num_frames;
          // Os lambdas estao retornando nullptr aqui.
          auto* funcao_estado = (db.estado_botao.target<bool(*)()>());
          if (num_frames > ATUALIZACOES_BOTAO_PRESSIONADO) {
            // Ficou suficiente, volta no proximo.
            num_frames = 0;
          } else if ((funcao_estado == nullptr) || (*funcao_estado != RetornaFalse)) {
            num_frames = 0;
            pressionado = false;
          }
        }
      }
      bool estado_ativo = db.estado_botao();
      float* cor = estado_ativo || pressionado ? cor_ativa : cor_padrao;
      gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
      float xi, xf, yi, yf;
      xi = db.coluna * botao_x;
      xf = xi + db.tamanho * botao_x;
      yi = db.linha * botao_y;
      yf = yi + db.tamanho * botao_y;
      gl::MatrizEscopo salva;
      if (db.num_lados_botao == 4 || parametros_desenho_.has_picking_x()) {
        float trans_x = (db.translacao_x * botao_x);
        float trans_y = (db.translacao_y * botao_y);
        InfoVerticeTabuleiro vertice_controle_virtual[] = {
          { xi + padding + trans_x, yi + padding + trans_y, 0.0f, 1.0f },
          { xf - padding + trans_x, yi + padding + trans_y, 1.0f, 1.0f },
          { xf - padding + trans_x, yf - padding + trans_y, 1.0f, 0.0f },
          { xi + padding + trans_x, yf - padding + trans_y, 0.0f, 0.0f },
        };
        unsigned short ponteiro_vertices[] = { 0, 1, 2, 3 };
        unsigned int id_textura = db.textura.empty() ? GL_INVALID_VALUE : texturas_->Textura(db.textura);
        if (parametros_desenho_.desenha_texturas() && id_textura != GL_INVALID_VALUE) {
          gl::Habilita(GL_TEXTURE_2D);
          gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
          glBindTexture(GL_TEXTURE_2D, id_textura);
          gl::PonteiroVerticesTexturas(2, GL_FLOAT, sizeof(InfoVerticeTabuleiro), (void*)&vertice_controle_virtual[0].s0);
        }
        gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
        gl::PonteiroVertices(2, GL_FLOAT, sizeof(InfoVerticeTabuleiro), (void*)vertice_controle_virtual);
        gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, (void*)ponteiro_vertices);
        gl::Desabilita(GL_TEXTURE_2D);
        gl::DesabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
        gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
      } else {
        gl::Translada(((xi + xf) / 2.0f) + (db.translacao_x * botao_x),
                      ((yi + yf) / 2.0f) + (db.translacao_y * botao_y), 0.0f);
        gl::Roda(db.rotacao_graus, 0.0f, 0.0f, 1.0f);
        if (db.num_lados_botao == 3) {
          gl::Triangulo(xf - xi);
        } else {
          gl::Disco((xf - xi) / 2.0f, db.num_lados_botao);
        }
      }
    }
  }
  // Desenha os labels para quem tiver e nao tiver textura.
  if (!parametros_desenho_.has_picking_x() && !modo_debug_) {
    for (const DadosBotao& db : dados_botoes) {
      unsigned int id_textura = db.textura.empty() ? GL_INVALID_VALUE : texturas_->Textura(db.textura);
      if (db.rotulo.empty() || id_textura != GL_INVALID_VALUE) {
        continue;
      }
      float xi, xf, yi, yf;
      xi = db.coluna * botao_x;
      xf = xi + db.tamanho * botao_x;
      yi = db.linha * botao_y;
      yf = yi + db.tamanho * botao_y;
      float x_meio = (xi + xf) / 2.0f;
      float y_meio = (yi + yf) / 2.0f;
      float y_base = y_meio - (fonte_y / 4.0f);
      if (db.cor_rotulo != nullptr) {
        gl::MudaCor(db.cor_rotulo[0], db.cor_rotulo[1], db.cor_rotulo[2], 1.0f);
      } else {
        gl::MudaCor(0.0f, 0.0f, 0.0f, 1.0f);
      }
      PosicionaRaster2d(x_meio, y_base, viewport[2], viewport[3]);
      gl::DesenhaString(db.rotulo);
    }
  }

  // So volta a luz se havia iluminacao antes.
  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
  }
  gl::Habilita(GL_DEPTH_TEST);
}



}  // namespace ent
