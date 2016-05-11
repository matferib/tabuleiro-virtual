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
#include "ent/tabuleiro_interface.h"
#include "ent/controle_virtual.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.pb.h"


namespace ent {

namespace {

const char* ROTULO_PADRAO = "-";
const char* TEXTURA_VAZIA = "";
const char* TEXTURAS_DIFEREM = "~";

// Retorna a textura das entidades. Se nao houver entidade ou se houver mas nao tiver textura, retorna TEXTURA_VAZIA.
// Se houver mais de uma e elas diferirem, retorna "~".
std::string TexturaEntidade(const std::vector<const Entidade*>& entidades) {
  if (entidades.empty()) {
    return TEXTURA_VAZIA;
  }
  const std::string& textura = entidades[0]->Proto().info_textura().id();
  for (const auto& e : entidades) {
    if (e->Proto().info_textura().id() != textura) {
      return TEXTURAS_DIFEREM;
    }
  }
  return textura;
}

// As funcoes abaixo retorna a proxima e a anterior do conjunto ordenado de texturas. O conjunto eh circular.
const std::string ProximoRotuloTextura(const std::string& corrente, const std::set<std::string>& texturas) {
  std::string chave = (corrente == TEXTURAS_DIFEREM || corrente == TEXTURA_VAZIA) ? ROTULO_PADRAO : corrente;
  auto it = texturas.find(chave);
  if (it == texturas.end()) {
    return ROTULO_PADRAO;
  }
  ++it;
  return (it == texturas.end()) ? *texturas.begin() : *it;
}

const std::string RotuloTexturaAnterior(const std::string& corrente, const std::set<std::string>& texturas) {
  std::string chave = (corrente == TEXTURAS_DIFEREM || corrente == TEXTURA_VAZIA) ? ROTULO_PADRAO : corrente;
  auto it = texturas.find(chave);
  if (it == texturas.end()) {
    return ROTULO_PADRAO;
  }
  return (it == texturas.begin()) ? *(--texturas.end()) : *(--it);
}

}  // namespace

void Tabuleiro::CarregaControleVirtual() {
  const char* ARQUIVO_CONTROLE_VIRTUAL = "controle_virtual.asciiproto";
  try {
    arq::LeArquivoAsciiProto(arq::TIPO_DADOS, ARQUIVO_CONTROLE_VIRTUAL, &controle_virtual_);
  } catch (const std::logic_error& erro) {
    LOG(ERROR) << "Erro carregando controle virtual: " << erro.what();
    return;
  }
  auto* n = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
  for (const auto& p : controle_virtual_.pagina()) {
    for (const auto& db : p.dados_botoes()) {
      if (!db.textura().empty()) {
        n->add_info_textura()->set_id(db.textura());
      }
      if (mapa_botoes_controle_virtual_.find(db.id()) == mapa_botoes_controle_virtual_.end()) {
        mapa_botoes_controle_virtual_[db.id()] = &db;
      }
    }
  }
  for (const auto& db : controle_virtual_.fixo().dados_botoes()) {
    if (!db.textura().empty()) {
      n->add_info_textura()->set_id(db.textura());
    }
    if (mapa_botoes_controle_virtual_.find(db.id()) == mapa_botoes_controle_virtual_.end()) {
      mapa_botoes_controle_virtual_[db.id()] = &db;
    }
  }
  for (const auto& par_id_acao : mapa_acoes_) {
    if (!par_id_acao.second->textura().empty()) {
      n->add_info_textura()->set_id(par_id_acao.second->textura());
    }
  }
  central_->AdicionaNotificacao(n);

  texturas_entidades_.insert(ROTULO_PADRAO);
  try {
    std::vector<std::string> texturas = arq::ConteudoDiretorio(arq::TIPO_TEXTURA, FiltroTexturaEntidade);
    // insere.
    texturas_entidades_.insert(texturas.begin(), texturas.end());
  } catch (const std::logic_error& e) {
    LOG(ERROR) << "Erro carregando lista de texturas do controle virtual: " << e.what();
  }

  for (const auto& modelo : mapa_modelos_) {
    modelos_entidades_.insert(modelo.first);
  }
}

void Tabuleiro::LiberaControleVirtual() {
  auto* n = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
  for (const auto& pagina : controle_virtual_.pagina()) {
    for (const auto& db : pagina.dados_botoes()) {
      n->add_info_textura()->set_id(db.textura());
    }
  }
  for (const auto& par_id_acao : mapa_acoes_) {
    if (!par_id_acao.second->textura().empty()) {
      n->add_info_textura()->set_id(par_id_acao.second->textura());
    }
  }
  central_->AdicionaNotificacao(n);
}

void Tabuleiro::PickingControleVirtual(int x, int y, bool alterna_selecao, int id) {
  contador_pressao_por_controle_[IdBotao(id)]++;
  switch (id) {
    case CONTROLE_NOP: {
      break;
    }
    case CONTROLE_INTERFACE_GRAFICA: {
      if (gui_ != nullptr) {
        gui_->Picking(x, y);
      }
      break;
    }
    case CONTROLE_GERAR_TERRENO_ALEATORIO: {
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_GERAR_TERRENO_ALEATORIO);
      TrataNotificacao(n);
      break;
    }
    case CONTROLE_GERAR_MONTANHA: {
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_GERAR_MONTANHA);
      TrataNotificacao(n);
      break;
    }
    case CONTROLE_ACAO:
      if (modo_clique_ != MODO_NORMAL) {
        EntraModoClique(MODO_NORMAL);
        return;
      } else {
        EntraModoClique(MODO_ACAO);
      }
      break;
    case CONTROLE_AJUDA:
      if (modo_clique_ == MODO_AJUDA) {
        EntraModoClique(MODO_NORMAL);
        return;
      }
      EntraModoClique(MODO_AJUDA);
      break;
    case CONTROLE_DANO_AUTOMATICO: {
      AlternaDanoAutomatico();
      break;
    }
    case CONTROLE_TRANSICAO:
      AlternaModoTransicao();
      break;
    case CONTROLE_CAMERA_ISOMETRICA:
      AlternaCameraIsometrica();
      break;
    case CONTROLE_CAMERA_PRESA:
      AlternaCameraPresa();
      break;
    case CONTROLE_CAMERA_PRIMEIRA_PESSOA:
      AlternaCameraPrimeiraPessoa();
      break;
    case CONTROLE_VISAO_ESCURO:
      AlternaVisaoEscuro();
      break;
    case CONTROLE_REGUA:
      AlternaModoRegua();
      break;
    case CONTROLE_MODO_TERRENO:
      AlternaModoTerreno();
      break;
    case CONTROLE_CIMA:
      TrataMovimentoEntidadesSelecionadas(true, 1.0f);
      break;
    case CONTROLE_CIMA_VERTICAL:
      TrataTranslacaoZ(1.0f);
      break;
    case CONTROLE_BAIXO:
      TrataMovimentoEntidadesSelecionadas(true, -1.0f);
      break;
    case CONTROLE_BAIXO_VERTICAL:
      TrataTranslacaoZ(-1.0f);
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
    case CONTROLE_PAGINACAO_LISTA_OBJETOS_CIMA:
      --pagina_lista_objetos_;
      break;
    case CONTROLE_PAGINACAO_LISTA_OBJETOS_BAIXO:
      ++pagina_lista_objetos_;
      break;
    case CONTROLE_PAGINACAO_ANTERIOR:
      if (controle_virtual_.pagina_corrente() > 0) {
        controle_virtual_.set_pagina_corrente(controle_virtual_.pagina_corrente() - 1);
        contador_pressao_por_controle_[CONTROLE_PAGINACAO_ANTERIOR] = 0;
      }
      break;
    case CONTROLE_PAGINACAO_PROXIMO:
      if (controle_virtual_.pagina_corrente() < controle_virtual_.pagina_size() - 1) {
        controle_virtual_.set_pagina_corrente(controle_virtual_.pagina_corrente() + 1);
        contador_pressao_por_controle_[CONTROLE_PAGINACAO_PROXIMO] = 0;
      }
      break;
    case CONTROLE_DESENHO_LIVRE:
    case CONTROLE_DESENHO_RETANGULO:
    case CONTROLE_DESENHO_CIRCULO:
    case CONTROLE_DESENHO_ESFERA:
    case CONTROLE_DESENHO_PIRAMIDE:
    case CONTROLE_DESENHO_TRIANGULO:
    case CONTROLE_DESENHO_CUBO:
    case CONTROLE_DESENHO_CILINDRO:
    case CONTROLE_DESENHO_CONE: {
      const std::map<IdBotao, TipoForma> mapa_id_tipo_forma = {
        { CONTROLE_DESENHO_LIVRE, TF_LIVRE},
        { CONTROLE_DESENHO_RETANGULO, TF_RETANGULO},
        { CONTROLE_DESENHO_CIRCULO, TF_CIRCULO},
        { CONTROLE_DESENHO_ESFERA, TF_ESFERA},
        { CONTROLE_DESENHO_PIRAMIDE, TF_PIRAMIDE},
        { CONTROLE_DESENHO_TRIANGULO, TF_TRIANGULO},
        { CONTROLE_DESENHO_CUBO, TF_CUBO},
        { CONTROLE_DESENHO_CILINDRO, TF_CILINDRO},
        { CONTROLE_DESENHO_CONE, TF_CONE},
      };
      auto it = mapa_id_tipo_forma.find(IdBotao(id));
      if (it == mapa_id_tipo_forma.end()) {
        LOG(ERROR) << "Id invalido: " << id;
        return;
      }
      TipoForma tf = it->second;
      if (modo_clique_ == MODO_DESENHO && forma_selecionada_ == tf) {
        modo_clique_ = MODO_NORMAL;
        return;
      }
      SelecionaFormaDesenho(it->second);
      EntraModoClique(MODO_DESENHO);
      break;
    }
    case CONTROLE_TEXTURA_ENTIDADE_ANTERIOR: {
      std::string rotulo_anterior = RotuloTexturaAnterior(TexturaEntidade(EntidadesSelecionadas()), texturas_entidades_);
      AlteraTexturaEntidadesSelecionadasNotificando(rotulo_anterior == ROTULO_PADRAO ? "" : rotulo_anterior);
      break;
    }
    case CONTROLE_TEXTURA_ENTIDADE_PROXIMA: {
      std::string proximo_rotulo = ProximoRotuloTextura(TexturaEntidade(EntidadesSelecionadas()), texturas_entidades_);
      AlteraTexturaEntidadesSelecionadasNotificando(proximo_rotulo == ROTULO_PADRAO ? "" : proximo_rotulo);
      break;
    }
    case CONTROLE_MODELO_ENTIDADE_ANTERIOR: {
      if (modelos_entidades_.size() < 2) {
        return;
      }
      auto it = modelos_entidades_.find(modelo_selecionado_.first);
      if (it == modelos_entidades_.begin()) {
        SelecionaModeloEntidade(*--modelos_entidades_.end());
      } else {
        SelecionaModeloEntidade(*--it);
      }
      break;
    }
    case CONTROLE_MODELO_ENTIDADE_PROXIMA: {
      if (modelos_entidades_.size() < 2) {
        return;
      }
      auto it = modelos_entidades_.find(modelo_selecionado_.first);
      if (it == modelos_entidades_.end() || it == --modelos_entidades_.end()) {
        SelecionaModeloEntidade(*modelos_entidades_.begin());
      } else {
        SelecionaModeloEntidade(*++it);
      }
      break;
    }
    case CONTROLE_APAGA_ENTIDADES: {
      ntf::Notificacao n;
      n.set_tipo(ntf::TN_REMOVER_ENTIDADE);
      TrataNotificacao(n);
      break;
    }
    case CONTROLE_DESENHO_AGRUPAR:
      AgrupaEntidadesSelecionadas();
      break;
    case CONTROLE_DESENHO_DESAGRUPAR:
      DesagrupaEntidadesSelecionadas();
      break;
    case CONTROLE_ULTIMA_ACAO_0:
    case CONTROLE_ULTIMA_ACAO_1:
    case CONTROLE_ULTIMA_ACAO_2: {
      SelecionaAcaoExecutada(id - CONTROLE_ULTIMA_ACAO_0);
      break;
    }
    case CONTROLE_DESENHO_COR_VERMELHO:
    case CONTROLE_DESENHO_COR_VERDE:
    case CONTROLE_DESENHO_COR_AZUL:
    case CONTROLE_DESENHO_COR_AMARELO:
    case CONTROLE_DESENHO_COR_MAGENTA:
    case CONTROLE_DESENHO_COR_CIANO:
    case CONTROLE_DESENHO_COR_BRANCO:
    case CONTROLE_DESENHO_COR_PRETO:
    {
      Cor c;
      c.set_r(id == CONTROLE_DESENHO_COR_VERMELHO ||
              id == CONTROLE_DESENHO_COR_MAGENTA ||
              id == CONTROLE_DESENHO_COR_AMARELO ||
              id == CONTROLE_DESENHO_COR_BRANCO ? 1.0f : 0);
      c.set_g(id == CONTROLE_DESENHO_COR_VERDE ||
              id == CONTROLE_DESENHO_COR_CIANO ||
              id == CONTROLE_DESENHO_COR_AMARELO ||
              id == CONTROLE_DESENHO_COR_BRANCO ? 1.0f : 0);
      c.set_b(id == CONTROLE_DESENHO_COR_AZUL ||
              id == CONTROLE_DESENHO_COR_CIANO ||
              id == CONTROLE_DESENHO_COR_MAGENTA ||
              id == CONTROLE_DESENHO_COR_BRANCO ? 1.0f : 0);
      if (!ids_entidades_selecionadas_.empty()) {
        AlteraCorEntidadesSelecionadasNotificando(c);
      } else {
        SelecionaCorDesenho(c);
      }
      break;
    }
    case CONTROLE_ACAO_SINALIZACAO: {
      SelecionaSinalizacao();
      break;
    }
    case CONTROLE_COPIAR: {
      CopiaEntidadesSelecionadas();
      break;
    }
    case CONTROLE_COLAR: {
      ColaEntidadesSelecionadas();
      break;
    }
    case CONTROLE_SALVAR: {
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO_SE_NECESSARIO_OU_SALVAR_DIRETO));
      break;
    }
    case CONTROLE_SALVAR_COMO: {
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO));
      break;
    }
    case CONTROLE_ABRIR: {
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ABRIR_TABULEIRO));
      break;
    }
    default:
      if (id >= CONTROLE_JOGADORES) {
        ntf::Notificacao n;
        n.set_tipo(ntf::TN_ALTERAR_MODO_MESTRE_SECUNDARIO);
        n.mutable_entidade()->set_id(id - CONTROLE_JOGADORES);
        TrataNotificacao(n);
      } else {
        LOG(WARNING) << "Controle invalido: " << id;
      }
  }
}

bool Tabuleiro::AtualizaBotaoControleVirtual(IdBotao id, const std::map<int, std::function<bool()>>& mapa_botoes) {
  const auto& it = mapa_botoes.find(id);
  if (it != mapa_botoes.end()) {
    return it->second();
  }
  auto res = contador_pressao_por_controle_.find(id);
  if (res == contador_pressao_por_controle_.end()) {
    return false;
  }
  int& num_frames = res->second;
  bool pressionado = (num_frames > 0);
  if (!pressionado) {
    return false;
  }
  ++num_frames;
  if (num_frames > ATUALIZACOES_BOTAO_PRESSIONADO) {
    // Ficou suficiente, volta no proximo.
    num_frames = 0;
  }
  // O botao pode estar pressionado ou ativado.
  return true;
}

namespace {
// Funcao para retornar o id de botao que melhor representa o estado do clique com a forma selecionada.
IdBotao ModoCliqueParaId(Tabuleiro::modo_clique_e mc, TipoForma tf) {
  switch (mc) {
    case Tabuleiro::MODO_DESENHO: {
      switch (tf) {
        case TF_LIVRE:     return CONTROLE_DESENHO_LIVRE;
        case TF_RETANGULO: return CONTROLE_DESENHO_RETANGULO;
        case TF_CIRCULO:   return CONTROLE_DESENHO_CIRCULO;
        case TF_ESFERA:    return CONTROLE_DESENHO_ESFERA;
        case TF_PIRAMIDE:  return CONTROLE_DESENHO_PIRAMIDE;
        case TF_TRIANGULO: return CONTROLE_DESENHO_TRIANGULO;
        case TF_CUBO:      return CONTROLE_DESENHO_CUBO;
        case TF_CILINDRO:  return CONTROLE_DESENHO_CILINDRO;
        case TF_CONE:      return CONTROLE_DESENHO_CONE;
        default:           return CONTROLE_DESENHO_LIVRE;
      }
    }
    case Tabuleiro::MODO_AJUDA:       return CONTROLE_AJUDA;
    case Tabuleiro::MODO_SINALIZACAO: return CONTROLE_ACAO_SINALIZACAO;
    case Tabuleiro::MODO_TRANSICAO:   return CONTROLE_TRANSICAO;
    case Tabuleiro::MODO_REGUA:       return CONTROLE_REGUA;
    case Tabuleiro::MODO_ROTACAO:     return CONTROLE_MODO_ROTACAO;
    case Tabuleiro::MODO_TERRENO:     return CONTROLE_MODO_TERRENO;
    default:                          return CONTROLE_AJUDA;
  }
}
}  // namespace

// Retorna o id da textura para uma determinada acao.
unsigned int Tabuleiro::TexturaBotao(const DadosBotao& db) const {
  if (!db.textura().empty()) {
    return texturas_->Textura(db.textura());
  }
  auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
  switch (db.id()) {
    case CONTROLE_ACAO: {
      if (modo_clique_ == MODO_NORMAL || modo_clique_ == MODO_ACAO) {
        unsigned int textura_espada = texturas_->Textura("icon_sword.png");
        if (entidade == nullptr || entidade->Acao(AcoesPadroes()).empty()) {
          return textura_espada;
        }
        const auto& it = mapa_acoes_.find(entidade->Acao(AcoesPadroes()));
        if (it == mapa_acoes_.end()) {
          return textura_espada;
        }
        unsigned int textura = texturas_->Textura(it->second->textura());
        return textura == GL_INVALID_VALUE ? textura_espada : textura;
      } else {
        auto it = mapa_botoes_controle_virtual_.find(ModoCliqueParaId(modo_clique_, forma_selecionada_));
        if (it != mapa_botoes_controle_virtual_.end()) {
          return it->second->textura().empty() ? GL_INVALID_VALUE : texturas_->Textura(it->second->textura());
        }
      }
    }
    case CONTROLE_ULTIMA_ACAO_0:
    case CONTROLE_ULTIMA_ACAO_1:
    case CONTROLE_ULTIMA_ACAO_2:
    {
      int indice_acao = db.id() - CONTROLE_ULTIMA_ACAO_0;
      if (entidade == nullptr) {
        return texturas_->Textura(AcaoPadrao(indice_acao).textura());
      }
      return texturas_->Textura(AcaoDoMapa(entidade->AcaoExecutada(indice_acao, AcoesPadroes())).textura());
    }
    default:
      ;
  }
  return GL_INVALID_VALUE;
}

namespace {

int TranslacaoX(const DadosBotao& db, const GLint* viewport, float unidade_largura) {
  int coluna = db.coluna();
  if (db.alinhamento_horizontal() == ALINHAMENTO_DIREITA) {
    return viewport[2] + coluna * unidade_largura;
  } else if (db.alinhamento_horizontal() == ALINHAMENTO_CENTRO) {
    return (viewport[2] / 2) + coluna * unidade_largura;
  } else {
    return coluna * unidade_largura;
  }
}

float TranslacaoY(const DadosBotao& db, const GLint* viewport, float unidade_altura) {
  int linha = db.linha();
  if (db.alinhamento_vertical() == ALINHAMENTO_CIMA) {
    return viewport[3] + linha * unidade_altura;
  } else if (db.alinhamento_vertical() == ALINHAMENTO_CENTRO) {
    return (viewport[3] / 2) + linha * unidade_altura;
  } else {
    return linha * unidade_altura;
  }
}

}  // namespace

void Tabuleiro::DesenhaBotaoControleVirtual(
    const DadosBotao& db, const GLint* viewport, float padding, float unidade_largura, float unidade_altura) {
  if ((db.picking_apenas() && !parametros_desenho_.has_picking_x()) ||
      (db.mestre_apenas() && !EmModoMestre(true))) {
    return;
  }
  gl::CarregaNome(db.id());
  float xi, xf, yi, yf;
  xi = TranslacaoX(db, viewport, unidade_largura);
  float largura_botao = db.has_tamanho() ? db.tamanho() : db.largura();
  float altura_botao = db.has_tamanho() ? db.tamanho() : db.altura();
  xf = xi + largura_botao * unidade_largura;
  yi = TranslacaoY(db, viewport, unidade_altura);
  yf = yi + altura_botao * unidade_altura;
  gl::MatrizEscopo salva(false);
  if (db.num_lados_botao() == 4) {
    float trans_x = (db.translacao_x() * unidade_largura);
    float trans_y = (db.translacao_y() * unidade_altura);
    unsigned int id_textura = TexturaBotao(db);
    if (parametros_desenho_.desenha_texturas() && id_textura != GL_INVALID_VALUE) {
      gl::Habilita(GL_TEXTURE_2D);
      gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    }
    float tam_x = xf - (2.0f * padding) - xi;
    float tam_y = yf - (2.0f * padding) - yi;
    gl::Translada(xi + padding + trans_x + (tam_x / 2.0f), yi + padding + trans_y + (tam_y / 2.0f), 0.0f, false);
    gl::Escala(tam_x, tam_y, 1.0f, false);
    gl::Retangulo(1.0f);
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  } else {
    gl::Translada(((xi + xf) / 2.0f) + (db.translacao_x() * unidade_largura),
        ((yi + yf) / 2.0f) + (db.translacao_y() * unidade_altura), 0.0f, false);
    gl::Roda(db.rotacao_graus(), 0.0f, 0.0f, 1.0f, false);
    if (db.num_lados_botao() == 3) {
      gl::Triangulo(xf - xi);
    } else {
      gl::Disco((xf - xi) / 2.0f, db.num_lados_botao());
    }
  }
}

std::string Tabuleiro::RotuloBotaoControleVirtual(const DadosBotao& db) const {
  if (db.has_rotulo()) {
    return db.rotulo();
  }
  switch (db.id()) {
    case CONTROLE_RODADA:
      return net::to_string(proto_.contador_rodadas());
    case CONTROLE_TEXTURA_ENTIDADE: {
      std::string rotulo = TexturaEntidade(EntidadesSelecionadas());
      return rotulo.empty() ? "-" : rotulo;
    }
    case CONTROLE_MODELO_ENTIDADE: {
      std::string rotulo = modelo_selecionado_.first;
      return rotulo.empty() ? "-" : rotulo;
    }
    default:

      ;
  }
  return "";
}

void Tabuleiro::DesenhaDicaBotaoControleVirtual(
    const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding, float unidade_largura, float unidade_altura) {
  if (id_entidade_detalhada_ != db.id() || db.dica().empty()) {
    return;
  }
  float largura_botao = db.has_largura() ? db.largura() : db.tamanho();
  float altura_botao = db.has_altura() ? db.altura() : db.tamanho();
  float xi, xf, yi, yf;
  xi = TranslacaoX(db, viewport, unidade_largura);
  xf = xi + largura_botao * unidade_largura;
  yi = TranslacaoY(db, viewport, unidade_altura);
  yf = yi + altura_botao * unidade_altura;
  float x_meio = (xi + xf) / 2.0f;
  MudaCor(COR_AMARELA);
  std::string dica = StringSemUtf8(db.dica());
  const float tam_dica_2_px = (dica.size() * fonte_x) / 2.0f;
  float delta_x = 0;
  if ((x_meio - tam_dica_2_px) < 0.0f) {
    delta_x += -(x_meio - tam_dica_2_px);
  } else if (x_meio + tam_dica_2_px > static_cast<float>(viewport[2])) {
    delta_x -= (x_meio + tam_dica_2_px - viewport[2]);
  }
  PosicionaRaster2d(x_meio + delta_x, yf, viewport[2], viewport[3]);
  std::function<void(const std::string&, bool)> funcao_desenho;
  gl::DesenhaString(dica, false);
}

void Tabuleiro::DesenhaRotuloBotaoControleVirtual(
    const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding, float unidade_largura, float unidade_altura) {
  unsigned int id_textura = TexturaBotao(db);
  std::string rotulo = StringSemUtf8(RotuloBotaoControleVirtual(db));
  if (rotulo.empty() || id_textura != GL_INVALID_VALUE) {
    return;
  }
  float largura_botao = db.has_largura() ? db.largura() : db.tamanho();
  float altura_botao = db.has_altura() ? db.altura() : db.tamanho();
  float xi, xf, yi, yf;
  xi = TranslacaoX(db, viewport, unidade_largura);
  xf = xi + largura_botao * unidade_largura;
  yi = TranslacaoY(db, viewport, unidade_altura);
  yf = yi + altura_botao * unidade_altura;
  float x_meio = (xi + xf) / 2.0f;
  float y_meio = (yi + yf) / 2.0f;
  float y_base = y_meio - (fonte_y / 4.0f);
  if (db.cor_rotulo().has_r() || db.cor_rotulo().has_g() || db.cor_rotulo().has_b()) {
    gl::MudaCor(db.cor_rotulo().r(), db.cor_rotulo().g(), db.cor_rotulo().b(), 1.0f);
  } else {
    gl::MudaCor(0.0f, 0.0f, 0.0f, 1.0f);
  }
  // Adiciona largura de um botao por causa do paginador inicial.
  int max_caracteres = (largura_botao * unidade_largura) / fonte_x;
  PosicionaRaster2d(x_meio, y_base, viewport[2], viewport[3]);
  gl::DesenhaString(rotulo.substr(0, max_caracteres));
}

void Tabuleiro::DesenhaControleVirtual() {
  gl::Desabilita(GL_LIGHTING);
  gl::Desabilita(GL_DEPTH_TEST);
  float cor_padrao[3];
  cor_padrao[0] = 0.8f;
  cor_padrao[1] = 0.8f;
  cor_padrao[2] = 0.8f;

  int fonte_x_int, fonte_y_int, escala;
  gl::TamanhoFonte(&fonte_x_int, &fonte_y_int, &escala);
  fonte_x_int *= escala;
  fonte_y_int *= escala;
  const float fonte_x = fonte_x_int;
  const float fonte_y = fonte_y_int;
  const float altura_botao = fonte_y * 2.5f;
  const float largura_botao = fonte_x * 3.0f;
  //const float largura_botao = altura_botao;
  const float padding = parametros_desenho_.has_picking_x() ? 0 : fonte_x / 4;

  // Mapeia id do botao para a funcao de estado.
  static const std::map<int, std::function<bool()>> mapa_botoes = {
    { CONTROLE_ACAO,              [this] () { return modo_clique_ != MODO_NORMAL; } },
    { CONTROLE_AJUDA,             [this] () { return modo_clique_ == MODO_AJUDA; } },
    { CONTROLE_TRANSICAO,         [this] () { return modo_clique_ == MODO_TRANSICAO; } },
    { CONTROLE_REGUA,             [this] () { return modo_clique_ == MODO_REGUA; } },
    { CONTROLE_MODO_TERRENO,      [this] () { return modo_clique_ == MODO_TERRENO; } },
    { CONTROLE_CAMERA_ISOMETRICA, [this] () { return camera_ == CAMERA_ISOMETRICA; } },
    { CONTROLE_CAMERA_PRESA,      [this] () { return camera_presa_; } },
    { CONTROLE_CAMERA_PRIMEIRA_PESSOA,      [this] () { return camera_ == CAMERA_PRIMEIRA_PESSOA; } },
    { CONTROLE_VISAO_ESCURO,      [this] () { return visao_escuro_; } },
    { CONTROLE_LUZ,               [this]() {
      auto* e = EntidadePrimeiraPessoaOuSelecionada();
      return e != nullptr && e->Proto().has_luz();
    } },
    { CONTROLE_QUEDA,        [this]() {
      auto* e = EntidadePrimeiraPessoaOuSelecionada();
      return e != nullptr && e->Proto().caida();
    } },
    { CONTROLE_VOO,          [this]() {
      auto* e = EntidadePrimeiraPessoaOuSelecionada();
      return e != nullptr && e->Proto().voadora();
    } },
    { CONTROLE_VISIBILIDADE, [this]() {
      auto* e = EntidadePrimeiraPessoaOuSelecionada();
      return e != nullptr && !e->Proto().visivel();
    } },
    { CONTROLE_DESENHO_LIVRE, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_LIVRE;
    }, },
    { CONTROLE_DESENHO_RETANGULO, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_RETANGULO;
    }, },
    { CONTROLE_DESENHO_CIRCULO, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CIRCULO;
    }, },
    { CONTROLE_DESENHO_ESFERA, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_ESFERA;
    }, },
    { CONTROLE_DESENHO_PIRAMIDE, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_PIRAMIDE;
    }, },
    { CONTROLE_DESENHO_TRIANGULO, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_TRIANGULO;
    }, },
    { CONTROLE_DESENHO_CUBO, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CUBO;
    }, },
    { CONTROLE_DESENHO_CILINDRO, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CILINDRO;
    }, },
    { CONTROLE_DESENHO_CONE, [this]() {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CONE;
    }, },
    { CONTROLE_DANO_AUTOMATICO, [this]() {
      return modo_dano_automatico_;
    }, },
  };
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);

  // Desenha em duas passadas por causa da limitacao de projecao do nexus 7.
  // Desenha apenas os botoes.
  {
    // Modo 2d: eixo com origem embaixo esquerda.
    gl::MatrizEscopo salva_matriz(GL_PROJECTION);
    gl::CarregaIdentidade(false);
    if (parametros_desenho_.has_picking_x()) {
      // Modo de picking faz a matriz de picking para projecao ortogonal.
      gl::MatrizPicking(parametros_desenho_.picking_x(), parametros_desenho_.picking_y(), 1.0, 1.0, viewport);
    }
    gl::Ortogonal(0, largura_, 0, altura_, 0, 1);
    gl::MatrizEscopo salva_matriz_2(GL_MODELVIEW);
    gl::CarregaIdentidade();
    int pagina_corrente = controle_virtual_.pagina_corrente();
    if (pagina_corrente < 0 || pagina_corrente >= controle_virtual_.pagina_size()) {
      return;
    }
    std::vector<const DadosBotao*> botoes;
    for (const auto& db : controle_virtual_.pagina(pagina_corrente).dados_botoes()) {
      botoes.push_back(&db);
    }
    for (const auto& db : controle_virtual_.fixo().dados_botoes()) {
      botoes.push_back(&db);
    }
    for (const auto* db : botoes) {
      float cor[3];
      if (db->has_cor_fundo()) {
        cor[0] = db->cor_fundo().r();
        cor[1] = db->cor_fundo().g();
        cor[2] = db->cor_fundo().b();
      } else {
        cor[0] = cor_padrao[0];
        cor[1] = cor_padrao[1];
        cor[2] = cor_padrao[2];
      }
      float ajuste = AtualizaBotaoControleVirtual(db->id(), mapa_botoes) ? 0.5f : 1.0f;
      cor[0] *= ajuste;
      cor[1] *= ajuste;
      cor[2] *= ajuste;
      gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
      DesenhaBotaoControleVirtual(*db, viewport, padding, largura_botao, altura_botao);
    }

    // Rotulos dos botoes.
    for (const auto* db : botoes) {
      DesenhaRotuloBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao);
    }
    // Dicas.
    if (tipo_entidade_detalhada_ == OBJ_CONTROLE_VIRTUAL) {
      for (const auto* db : botoes) {
        DesenhaDicaBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao);
      }
    }
  }

  // So volta a luz se havia iluminacao antes.
  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
  }
  gl::Habilita(GL_DEPTH_TEST);
}

}  // namespace ent
