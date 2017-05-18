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
// Os botoes tem largura e altura baseados no tamanho da fonte * multiplicador.
const float MULTIPLICADOR_LARGURA = 3.0f;
const float MULTIPLICADOR_ALTURA = 2.5f;


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

// Retorna a cor de um botao.
void CorBotao(const DadosBotao& db, float cor[3]) {
  const static float cor_padrao[3] = { 0.9f, 0.9f, 0.9f};
  if (db.has_cor_fundo()) {
    cor[0] = db.cor_fundo().r();
    cor[1] = db.cor_fundo().g();
    cor[2] = db.cor_fundo().b();
  } else {
    cor[0] = cor_padrao[0];
    cor[1] = cor_padrao[1];
    cor[2] = cor_padrao[2];
  }
}

}  // namespace

void Tabuleiro::IniciaGlControleVirtual() {
  for (auto& pagina : *controle_virtual_.mutable_pagina()) {
    for (auto& db : *pagina.mutable_dados_botoes()) {
      db.clear_id_textura();
    }
  }
  for (auto& db : *controle_virtual_.mutable_fixo()->mutable_dados_botoes()) {
    db.clear_id_textura();
  }
}

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
    if (!par_id_acao.second->icone().empty()) {
      n->add_info_textura()->set_id(par_id_acao.second->icone());
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

  for (const auto& modelo : mapa_modelos_com_parametros_) {
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
  for (const auto& db : controle_virtual_.fixo().dados_botoes()) {
    if (!db.textura().empty()) {
      n->add_info_textura()->set_id(db.textura());
    }
  }
  for (const auto& par_id_acao : mapa_acoes_) {
    if (!par_id_acao.second->icone().empty()) {
      n->add_info_textura()->set_id(par_id_acao.second->icone());
    }
  }
  central_->AdicionaNotificacao(n);
}

void Tabuleiro::PickingControleVirtual(int x, int y, bool alterna_selecao, bool duplo, int id) {
  VLOG(1) << "picking id: " << id;
  contador_pressao_por_controle_[IdBotao(id)]++;
  switch (id) {
    case CONTROLE_ROLAR_D20:
      AlternaModoD20();
      break;
    case CONTROLE_APAGAR_INICIATIVAS:
      LimpaIniciativasNotificando();
      break;
    case CONTROLE_ROLAR_INICIATIVA:
      RolaIniciativasNotificando();
      break;
    case CONTROLE_INICIAR_INICIATIVA_PARA_COMBATE:
      IniciaIniciativaParaCombate();
      break;
    case CONTROLE_PROXIMA_INICIATIVA:
      ProximaIniciativa();
      break;
    case CONTROLE_ANGULO_VISAO_MAIS:
      AlteraAnguloVisao(angulo_visao_vertical_graus_ + 5.0f);
      break;
    case CONTROLE_ANGULO_VISAO_MENOS:
      AlteraAnguloVisao(angulo_visao_vertical_graus_ - 5.0f);
      break;
    case CONTROLE_ANGULO_VISAO_ORIGINAL:
      AlteraAnguloVisao(CAMPO_VISAO_PADRAO);
      break;
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
    case CONTROLE_ILUMINACAO_MESTRE: {
      TrataBotaoAlternarIluminacaoMestre();
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
    case CONTROLE_BONUS_ATAQUE_NEGATIVO: {
      bonus_ataque_negativo_ = !bonus_ataque_negativo_;
      break;
    }
    case CONTROLE_BONUS_DANO_NEGATIVO: {
      bonus_dano_negativo_ = !bonus_dano_negativo_;
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
    case CONTROLE_CAMERA_PRESA_PROXIMO:
      MudaEntidadeCameraPresa();
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
    case CONTROLE_BAIXO:
      TrataMovimentoEntidadesSelecionadas(true, -1.0f);
      break;
    case CONTROLE_CIMA_VERTICAL:
      TrataTranslacaoZ(1.0f);
      break;
    case CONTROLE_BAIXO_VERTICAL:
      TrataTranslacaoZ(-1.0f);
      break;
    case CONTROLE_CIMA_VERTICAL_TERRENO:
      TrataDeltaTerreno(1.0f * TAMANHO_LADO_QUADRADO);
      break;
    case CONTROLE_BAIXO_VERTICAL_TERRENO:
      TrataDeltaTerreno(-1.0f * TAMANHO_LADO_QUADRADO);
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
      AlteraUltimoPontoVidaListaPontosVida(1);
      break;
    case CONTROLE_ADICIONA_5:
      AlteraUltimoPontoVidaListaPontosVida(5);
      break;
    case CONTROLE_ADICIONA_10:
      AlteraUltimoPontoVidaListaPontosVida(10);
      break;
    case CONTROLE_CONFIRMA_DANO:
      // Padrao dano.
      AcumulaPontosVida({{-1/*dano/cura*/, "0" /*valor*/}});
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
    case CONTROLE_SELECIONAVEL:
      if (EmModoMestreIncluindoSecundario()) {
        AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_SELECIONAVEL);
      }
      break;

    case CONTROLE_FURTIVO:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_FURTIVO);
      break;
    case CONTROLE_SURPRESO:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_SURPRESO);
      break;
    case CONTROLE_ATAQUE_MAIS_1:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MAIS_1);
      break;
    case CONTROLE_ATAQUE_MAIS_2:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MAIS_2);
      break;
    case CONTROLE_ATAQUE_MAIS_4:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MAIS_4);
      break;
    case CONTROLE_ATAQUE_MAIS_8:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MAIS_8);
      break;
    case CONTROLE_ATAQUE_MENOS_1:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MENOS_1);
      break;
    case CONTROLE_ATAQUE_MENOS_2:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MENOS_2);
      break;
    case CONTROLE_ATAQUE_MENOS_4:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MENOS_4);
      break;
    case CONTROLE_ATAQUE_MENOS_8:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MENOS_8);
      break;

    case CONTROLE_DANO_MAIS_1:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_1);
      break;
    case CONTROLE_DANO_MAIS_2:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_2);
      break;
    case CONTROLE_DANO_MAIS_4:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_4);
      break;
    case CONTROLE_DANO_MAIS_8:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_8);
      break;
    case CONTROLE_DANO_MAIS_16:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_16);
      break;
    case CONTROLE_DANO_MAIS_32:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MAIS_32);
      break;

    case CONTROLE_DANO_MENOS_1:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MENOS_1);
      break;
    case CONTROLE_DANO_MENOS_2:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MENOS_2);
      break;
    case CONTROLE_DANO_MENOS_4:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MENOS_4);
      break;
    case CONTROLE_DANO_MENOS_8:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_DANO_MENOS_8);
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
    case CONTROLE_PAGINACAO_LISTA_LOG_CIMA:
      --pagina_log_eventos_;
      break;
    case CONTROLE_PAGINACAO_LISTA_LOG_BAIXO:
      ++pagina_log_eventos_;
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
    case CONTROLE_MODELO_ENTIDADE: {
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_MODELO_ENTIDADE));
      break;
    }
    case CONTROLE_MODELO_ENTIDADE_ANTERIOR: {
      if (modelos_entidades_.size() < 2) {
        return;
      }
      auto it = modelos_entidades_.find(modelo_selecionado_com_parametros_.first);
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
      auto it = modelos_entidades_.find(modelo_selecionado_com_parametros_.first);
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
    case CONTROLE_DESENHO_COR_PERSONALIZADA:
    {
      if (duplo || alterna_selecao) {
        auto* n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_COR_PERSONALIZADA);
        *n->mutable_tabuleiro()->mutable_luz_ambiente() = cor_personalizada_;
        central_->AdicionaNotificacao(n);
        break;
      }
      if (!ids_entidades_selecionadas_.empty()) {
        AlteraCorEntidadesSelecionadasNotificando(cor_personalizada_);
      } else {
        SelecionaCorDesenho(cor_personalizada_);
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
      ColaEntidadesSelecionadas(alterna_selecao  /*ref_camera*/);
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

bool Tabuleiro::AtualizaBotaoControleVirtual(
    DadosBotao* db, const std::unordered_map<int, std::function<bool(const Entidade* entidade)>>& mapa_botoes, const Entidade* entidade) {
  if (!db->textura().empty() && !db->has_id_textura()) {
    unsigned int id_tex = texturas_->Textura(db->textura());
    if (id_tex != GL_INVALID_VALUE) {
      db->set_id_textura(id_tex);
    }
  }
  if (db->id() == CONTROLE_DESENHO_COR_PERSONALIZADA) {
    *db->mutable_cor_fundo() = cor_personalizada_;
  }
  const auto& it = mapa_botoes.find(db->id());
  if (it != mapa_botoes.end()) {
    return it->second(entidade);
  }
  auto res = contador_pressao_por_controle_.find(db->id());
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
    case Tabuleiro::MODO_D20:         return CONTROLE_ROLAR_D20;
    case Tabuleiro::MODO_ROTACAO:     return CONTROLE_MODO_ROTACAO;
    case Tabuleiro::MODO_TERRENO:     return CONTROLE_MODO_TERRENO;
    default:                          return CONTROLE_AJUDA;
  }
}
}  // namespace

// Retorna o id da textura para uma determinada acao.
unsigned int Tabuleiro::TexturaBotao(const DadosBotao& db, const Entidade* entidade) const {
  if (!db.textura().empty()) {
    return db.has_id_textura() ? db.id_textura() : GL_INVALID_VALUE;
    //return texturas_->Textura(db.textura());
  }
  switch (db.id()) {
    case CONTROLE_ACAO: {
      if (modo_clique_ == MODO_NORMAL || modo_clique_ == MODO_ACAO) {
        unsigned int textura_espada = texturas_->Textura("icon_sword.png");
        if (entidade == nullptr) {
          return textura_espada;
        }
        unsigned int textura = texturas_->Textura(entidade->Acao(mapa_acoes_).icone());
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
        return texturas_->Textura(AcaoPadrao(indice_acao).icone());
      }
      return texturas_->Textura(AcaoDoMapa(entidade->AcaoExecutada(indice_acao, AcoesPadroes())).icone());
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
    const DadosBotao& db, const GLint* viewport, float padding, float unidade_largura, float unidade_altura, const Entidade* entidade) {
  if ((db.picking_apenas() && !parametros_desenho_.has_picking_x()) ||
      (db.mestre_apenas() && !VisaoMestre()) ||
      db.forma() == FORMA_NULA) {
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
  gl::MatrizEscopo salva;
  if (db.forma() == FORMA_RETANGULO) {
    float trans_x = (db.translacao_x() * unidade_largura);
    float trans_y = (db.translacao_y() * unidade_altura);
    unsigned int id_textura = TexturaBotao(db, entidade);
    if (parametros_desenho_.desenha_texturas() && id_textura != GL_INVALID_VALUE) {
      gl::Habilita(GL_TEXTURE_2D);
      gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    }
    float tam_x = xf - (2.0f * padding) - xi;
    float tam_y = yf - (2.0f * padding) - yi;
    Matrix4 m;
    m.scale(tam_x, tam_y, 1.0f);
    m.translate(xi + padding + trans_x + (tam_x / 2.0f), yi + padding + trans_y + (tam_y / 2.0f), 0.0f);
    gl::MultiplicaMatriz(m.get());
    gl::AtualizaMatrizes();
    gl::RetanguloUnitario();
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::Desabilita(GL_TEXTURE_2D);
  } else {
    Matrix4 m;
    float transx = ((xi + xf) / 2.0f) + (db.translacao_x() * unidade_largura);
    float transy = ((yi + yf) / 2.0f) + (db.translacao_y() * unidade_altura);
    if (db.forma() == FORMA_TRIANGULO) {
      unsigned int id_textura = TexturaBotao(db, entidade);
      if (parametros_desenho_.desenha_texturas() && id_textura != GL_INVALID_VALUE) {
        gl::Habilita(GL_TEXTURE_2D);
        gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
      }
      m.rotateZ(db.rotacao_graus());
      m.scale(xf - xi, xf - xi, 1.0f);
      m.translate(transx, transy, 0.0f);
      gl::MultiplicaMatriz(m.get());
      gl::AtualizaMatrizes();
      gl::TrianguloUnitario();
      gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
      gl::Desabilita(GL_TEXTURE_2D);
    } else {
      m.rotateZ(db.rotacao_graus());
      m.scale((xf - xi), (xf - xi), 1.0f);
      m.translate(transx, transy, 0.0f);
      gl::MultiplicaMatriz(m.get());
      gl::AtualizaMatrizes();
      gl::DiscoUnitario();
    }
  }
}

bool Tabuleiro::EstadoBotao(IdBotao id) const {
  switch (id) {
    case CONTROLE_DANO_AUTOMATICO:
      return modo_dano_automatico_;
    case CONTROLE_BONUS_ATAQUE_NEGATIVO:
      return bonus_ataque_negativo_;
    case CONTROLE_BONUS_DANO_NEGATIVO:
      return bonus_dano_negativo_;
    default:
      return false;
  }
}

bool Tabuleiro::BotaoVisivel(const DadosBotao& db) const {
  if (db.has_visibilidade()) {
    for (const auto& ref : db.visibilidade().referencia()) {
      bool parcial = EstadoBotao(ref.id());
      if (ref.tipo() == VIS_INVERSO_DE) {
        parcial = !parcial;
      }
      if (!parcial) {
        return false;
      }
    }
  }
  return true;
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
      std::string rotulo = modelo_selecionado_com_parametros_.first;
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
  std::string dica = StringSemUtf8(db.dica());
  const float tam_dica_2_px = (dica.size() * fonte_x) / 2.0f;
  float delta_x = 0;
  if ((x_meio - tam_dica_2_px) < 0.0f) {
    delta_x += -(x_meio - tam_dica_2_px);
  } else if (x_meio + tam_dica_2_px > static_cast<float>(viewport[2])) {
    delta_x -= (x_meio + tam_dica_2_px - viewport[2]);
  }
  if (yf + fonte_y > viewport[3]) {
    yf = viewport[3] - fonte_y;
  }
  PosicionaRaster2d(x_meio + delta_x, yf);
  MudaCor(COR_PRETA);
  gl::Retangulo(x_meio + delta_x - tam_dica_2_px - 2, yf - 2,
                x_meio + delta_x + tam_dica_2_px + 2, yf + fonte_y + 2);
  std::function<void(const std::string&, bool)> funcao_desenho;
  MudaCor(COR_AMARELA);
  gl::DesenhaString(dica, false);
}

void Tabuleiro::DesenhaRotuloBotaoControleVirtual(
    const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding,
    float unidade_largura, float unidade_altura, const Entidade* entidade) {
  unsigned int id_textura = TexturaBotao(db, entidade);
  if (parametros_desenho_.has_picking_x() || id_textura != GL_INVALID_VALUE || (db.mestre_apenas() && !VisaoMestre())) {
    return;
  }
  std::string rotulo = StringSemUtf8(RotuloBotaoControleVirtual(db));
  if (rotulo.empty()) {
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
  PosicionaRaster2d(x_meio, y_base);
  gl::DesenhaString(rotulo.substr(0, max_caracteres));
}

void Tabuleiro::DesenhaIniciativas() {
  if (indice_iniciativa_ == -1 || iniciativas_.empty() || indice_iniciativa_ >= (int)iniciativas_.size()) {
    return;
  }
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);

  int raster_x = 0, raster_y = 0;
  largura_fonte *= escala;
  altura_fonte *= escala;
  raster_y = altura_ - altura_fonte;
  raster_x = (opcoes_.mostra_fps() ? largura_fonte  * 9 : 0) + 2;
  PosicionaRaster2d(raster_x, raster_y);

  MudaCor(COR_AMARELA);
  char titulo[100] = { '\0' };
  snprintf(titulo, 99, "Iniciativa: %d/%d",
           iniciativas_[indice_iniciativa_].iniciativa, iniciativas_[indice_iniciativa_].modificador);
  gl::DesenhaStringAlinhadoEsquerda(titulo);
  MudaCor(COR_BRANCA);
  raster_y -= (altura_fonte + 2);
  int num_desenhadas = 0;

  std::vector<DadosIniciativa> entidades_na_ordem_desenho;
  entidades_na_ordem_desenho.reserve(iniciativas_.size());
  entidades_na_ordem_desenho.insert(entidades_na_ordem_desenho.end(), iniciativas_.begin() + indice_iniciativa_, iniciativas_.end());
  entidades_na_ordem_desenho.insert(entidades_na_ordem_desenho.end(), iniciativas_.begin(), iniciativas_.begin() + indice_iniciativa_);

  for (const auto& di : entidades_na_ordem_desenho) {
    Entidade* entidade = BuscaEntidade(di.id);
    if (entidade == nullptr ||
        (!VisaoMestre() && !entidade->SelecionavelParaJogador() && !entidade->Proto().visivel())) {
      continue;
    }
    PosicionaRaster2d(raster_x, raster_y);
    raster_y -= (altura_fonte + 2);
    char str[51] = {'\0'};
    std::string rotulo;
    const auto& proto = entidade->Proto();
    if (proto.has_rotulo()) {
      rotulo = proto.rotulo();
    } else if (!proto.modelo_3d().id().empty()) {
      rotulo = proto.modelo_3d().id();
    } else if (!proto.info_textura().id().empty()) {
      rotulo = proto.info_textura().id().substr(0, proto.info_textura().id().find_last_of("."));
    }
    if (entidade->Morta()) {
      rotulo += " (morta)";
    }
    if (entidade->IdCenario() != cenario_corrente_) {
      rotulo += " (outro cenario)";
    }
    snprintf(str, 50, "%d-%s", entidade->Iniciativa(), StringSemUtf8(rotulo).c_str());
    gl::DesenhaStringAlinhadoEsquerda(str);
    const int MAX_INICIATIVAS_DESENHADAS = 10;
    if (++num_desenhadas > MAX_INICIATIVAS_DESENHADAS) {
      break;
    }
  }
}

void Tabuleiro::DesenhaListaPontosVida() {
  if (lista_pontos_vida_.empty() && !modo_dano_automatico_) {
    return;
  }
  int largura_fonte, altura_fonte, escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);
  const float largura_botao = static_cast<float>(largura_fonte) * MULTIPLICADOR_LARGURA * escala;

  int raster_x = 0, raster_y = 0;
  largura_fonte *= escala;
  altura_fonte *= escala;
  raster_y = altura_ - altura_fonte;
  raster_x = largura_ - (VisaoMestre() ? 3.0f : 0.0f) * largura_botao - 2;
  PosicionaRaster2d(raster_x, raster_y);

  MudaCor(COR_BRANCA);
  std::string titulo("Lista PV");
  gl::DesenhaStringAlinhadoDireita(titulo);
  raster_y -= (altura_fonte + 2);
  if (modo_dano_automatico_) {
    PosicionaRaster2d(raster_x, raster_y);
    raster_y -= (altura_fonte + 2);
    MudaCor(COR_BRANCA);
    const auto* entidade = EntidadeSelecionada();
    std::string valor = "AUTO";
    if (entidade != nullptr) {
      const std::string s = StringSemUtf8(entidade->DetalhesAcao());
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
    for (const auto& sinal_valor : lista_pontos_vida_) {
      PosicionaRaster2d(raster_x, raster_y);
      raster_y -= (altura_fonte + 2);
      MudaCor(sinal_valor.first >= 0 ? COR_VERDE : COR_VERMELHA);
      char str[20];
      snprintf(str, 19, "%s", sinal_valor.second.c_str());
      gl::DesenhaStringAlinhadoDireita(str);
    }
  }
}

void Tabuleiro::DesenhaControleVirtual() {
  int fonte_x_int, fonte_y_int, escala;
  gl::TamanhoFonte(&fonte_x_int, &fonte_y_int, &escala);
  fonte_x_int *= escala;
  fonte_y_int *= escala;
  const float fonte_x = fonte_x_int;
  const float fonte_y = fonte_y_int;
  const float altura_botao = fonte_y * MULTIPLICADOR_ALTURA;
  const float largura_botao = fonte_x * MULTIPLICADOR_LARGURA;
  //const float largura_botao = altura_botao;
  const float padding = parametros_desenho_.has_picking_x() ? 0 : fonte_x / 4;

  // Mapeia id do botao para a funcao de estado.
  static const std::unordered_map<int, std::function<bool(const Entidade* entidade)>> mapa_botoes = {
    { CONTROLE_ACAO,              [this] (const Entidade* entidade) { return modo_clique_ != MODO_NORMAL; } },
    { CONTROLE_AJUDA,             [this] (const Entidade* entidade) { return modo_clique_ == MODO_AJUDA; } },
    { CONTROLE_TRANSICAO,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_TRANSICAO; } },
    { CONTROLE_REGUA,             [this] (const Entidade* entidade) { return modo_clique_ == MODO_REGUA; } },
    { CONTROLE_ROLAR_D20,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_D20; } },
    { CONTROLE_MODO_TERRENO,      [this] (const Entidade* entidade) { return modo_clique_ == MODO_TERRENO; } },
    { CONTROLE_CAMERA_ISOMETRICA, [this] (const Entidade* entidade) { return camera_ == CAMERA_ISOMETRICA; } },
    { CONTROLE_INICIAR_INICIATIVA_PARA_COMBATE, [this] (const Entidade* entidade) { return indice_iniciativa_ != -1; } },
    { CONTROLE_CAMERA_PRESA,      [this] (const Entidade* entidade) {
      return (entidade != nullptr && IdPresoACamera(entidade->Id())) || (camera_presa_ && entidade == nullptr);
    } },
    { CONTROLE_CAMERA_PRIMEIRA_PESSOA,      [this] (const Entidade* entidade) { return camera_ == CAMERA_PRIMEIRA_PESSOA; } },
    { CONTROLE_VISAO_ESCURO,      [this] (const Entidade* entidade) { return visao_escuro_; } },
    { CONTROLE_INICIAR_INICIATIVA_PARA_COMBATE,  [this] (const Entidade* entidade) {
      return indice_iniciativa_ != -1;
    } },
    { CONTROLE_LUZ,                [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().has_luz();
    } },
    { CONTROLE_QUEDA,              [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().caida();
    } },
    { CONTROLE_SELECIONAVEL,       [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().selecionavel_para_jogador();
    } },
    { CONTROLE_FURTIVO,            [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().furtivo();
    } },
    { CONTROLE_SURPRESO,           [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().surpreso();
    } },
    { CONTROLE_ILUMINACAO_MESTRE,  [this] (const Entidade* entidade) {
      return !opcoes_.iluminacao_mestre_igual_jogadores();
    } },
    // Ataque.
    { CONTROLE_ATAQUE_MAIS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_mais_1();
    } },
    { CONTROLE_ATAQUE_MAIS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_mais_2();
    } },
    { CONTROLE_ATAQUE_MAIS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_mais_4();
    } },
    { CONTROLE_ATAQUE_MAIS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_mais_8();
    } },

    { CONTROLE_ATAQUE_MENOS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_menos_1();
    } },
    { CONTROLE_ATAQUE_MENOS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_menos_2();
    } },
    { CONTROLE_ATAQUE_MENOS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_menos_4();
    } },
    { CONTROLE_ATAQUE_MENOS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().ataque_menos_8();
    } },

    // Dano.
    { CONTROLE_DANO_MAIS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_1();
    } },
    { CONTROLE_DANO_MAIS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_2();
    } },
    { CONTROLE_DANO_MAIS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_4();
    } },
    { CONTROLE_DANO_MAIS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_8();
    } },
    { CONTROLE_DANO_MAIS_16,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_16();
    } },
    { CONTROLE_DANO_MAIS_32,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_mais_32();
    } },

    { CONTROLE_DANO_MENOS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_menos_1();
    } },
    { CONTROLE_DANO_MENOS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_menos_2();
    } },
    { CONTROLE_DANO_MENOS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_menos_4();
    } },
    { CONTROLE_DANO_MENOS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_globais().dano_menos_8();
    } },

    { CONTROLE_VOO,          [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().voadora();
    } },
    { CONTROLE_VISIBILIDADE, [this] (const Entidade* entidade) {
      return entidade != nullptr && !entidade->Proto().visivel();
    } },
    { CONTROLE_DESENHO_LIVRE, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_LIVRE;
    }, },
    { CONTROLE_DESENHO_RETANGULO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_RETANGULO;
    }, },
    { CONTROLE_DESENHO_CIRCULO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CIRCULO;
    }, },
    { CONTROLE_DESENHO_ESFERA, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_ESFERA;
    }, },
    { CONTROLE_DESENHO_PIRAMIDE, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_PIRAMIDE;
    }, },
    { CONTROLE_DESENHO_TRIANGULO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_TRIANGULO;
    }, },
    { CONTROLE_DESENHO_CUBO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CUBO;
    }, },
    { CONTROLE_DESENHO_CILINDRO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CILINDRO;
    }, },
    { CONTROLE_DESENHO_CONE, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_CONE;
    }, },
    { CONTROLE_DANO_AUTOMATICO, [this] (const Entidade* entidade) {
      return modo_dano_automatico_;
    }, },
    { CONTROLE_BONUS_ATAQUE_NEGATIVO, [this] (const Entidade* entidade) {
      return bonus_ataque_negativo_;
    }, },
    { CONTROLE_BONUS_DANO_NEGATIVO, [this] (const Entidade* entidade) {
      return bonus_dano_negativo_;
    }, },
  };

  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  gl::MatrizEscopo salva_matriz_2(GL_MODELVIEW);

  {
    int pagina_corrente = controle_virtual_.pagina_corrente();
    if (pagina_corrente < 0 || pagina_corrente >= controle_virtual_.pagina_size()) {
      return;
    }
    // Todos botoes, mapeados por id.
    std::vector<DadosBotao*> botoes;
    botoes.reserve(controle_virtual_.pagina(pagina_corrente).dados_botoes_size() + controle_virtual_.fixo().dados_botoes_size());
    for (auto& db : *controle_virtual_.mutable_pagina(pagina_corrente)->mutable_dados_botoes()) {
      if (!BotaoVisivel(db)) continue;
      botoes.push_back(&db);
    }
    for (auto& db : *controle_virtual_.mutable_fixo()->mutable_dados_botoes()) {
      if (!BotaoVisivel(db)) continue;
      botoes.push_back(&db);
    }
    auto* entidade = EntidadeSelecionadaOuPrimeiraPessoa();
    for (auto* db : botoes) {
      float ajuste = AtualizaBotaoControleVirtual(db, mapa_botoes, entidade) ? 0.5f : 1.0f;
      float cor[3];
      CorBotao(*db, cor);
      cor[0] *= ajuste;
      cor[1] *= ajuste;
      cor[2] *= ajuste;
      gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
      DesenhaBotaoControleVirtual(*db, viewport, padding, largura_botao, altura_botao, entidade);
      //LOG(INFO) << "timer: " << ((int)(timer_uma_renderizacao_completa_.elapsed().wall / 1000000ULL)) << ", botao: " << db->dica();
    }

    // Rotulos dos botoes.
    for (const auto* db : botoes) {
      DesenhaRotuloBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao, entidade);
    }
    // Dicas.
    if (tipo_entidade_detalhada_ == OBJ_CONTROLE_VIRTUAL) {
      for (const auto* db : botoes) {
        DesenhaDicaBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao);
      }
    }
  }

  // Informacao da entidade primeira pessoa. Uma barra na esquerda, com número abaixo.
  // Barra de pontos de vida.
  // aka DesenhaInfoPrimeiraPessoa.
  if (camera_presa_) {
    DesenhaInfoCameraPresa();
  }

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }
  if (parametros_desenho_.desenha_iniciativas()) {
    DesenhaIniciativas();
  }
  V_ERRO("desenhando lista pontos de vida");

  // So volta a luz se havia iluminacao antes.
  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
  }
  gl::Habilita(GL_DEPTH_TEST);
}

void Tabuleiro::DesenhaInfoCameraPresa() {
  const auto* entidade = BuscaEntidade(IdCameraPresa());
  if (entidade == nullptr || entidade->MaximoPontosVida() == 0) {
    return;
  }
  gl::TipoEscopo tipo_escopo(OBJ_ENTIDADE_LISTA);
  gl::CarregaNome(entidade->Id());

  int fonte_x_int, fonte_y_int, escala;
  gl::TamanhoFonte(&fonte_x_int, &fonte_y_int, &escala);
  fonte_x_int *= escala;
  fonte_y_int *= escala;
  const float fonte_x = fonte_x_int;
  const float fonte_y = fonte_y_int;
  const float altura_botao = fonte_y * MULTIPLICADOR_ALTURA;
  const float largura_botao = fonte_x * MULTIPLICADOR_LARGURA;

  float top_y = altura_botao * 7.0f;
  float bottom_y = altura_botao * 4.0f;
  float altura_maxima = (altura_botao * 7.0f) - (altura_botao * 4.0f);
  float altura_pv_entidade = 0;
  if (entidade->PontosVida() > 0) {
    float fator = static_cast<float>(entidade->PontosVida()) / entidade->MaximoPontosVida();
    altura_pv_entidade = std::min(altura_maxima, altura_maxima * fator);
  }
  gl::MudaCor(1.0f, 0.0f, 0.0f, 1.0f);
  gl::Retangulo(largura_botao, bottom_y, 2.0f * largura_botao, top_y);
  gl::MudaCor(0.0f, 1.0f, 0.0f, 1.0f);
  gl::Retangulo(largura_botao, bottom_y, 2.0f * largura_botao, bottom_y + altura_pv_entidade);
  if (parametros_desenho_.has_picking_x()) {
    return;
  }

  MudaCor(COR_AMARELA);
  PosicionaRaster2d(largura_botao, bottom_y - (fonte_y * 1.1f));
  gl::DesenhaStringAlinhadoEsquerda(net::to_string(entidade->PontosVida()) + "/" + net::to_string(entidade->MaximoPontosVida()), true  /*inverte vertical*/);
  if (!entidade->Proto().rotulo().empty()) {
    PosicionaRaster2d(largura_botao, top_y);
    gl::DesenhaStringAlinhadoEsquerda(StringSemUtf8(entidade->Proto().rotulo()));
  }
  //PosicionaRaster2d(largura_botao, top_y + (fonte_y * 0.5f));
  //gl::DesenhaStringAlinhadoEsquerda(std::string("mov: ") + net::to_string(quadrados_movimentados_), true  /*inverte vertical*/);
}

void Tabuleiro::SelecionaCorPersonalizada(float r, float g, float b, float a) {
  cor_personalizada_.set_r(r);
  cor_personalizada_.set_g(g);
  cor_personalizada_.set_b(b);
  cor_personalizada_.set_a(a);
}

}  // namespace ent
