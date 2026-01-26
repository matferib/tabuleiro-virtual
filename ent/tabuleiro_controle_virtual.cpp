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

#include "absl/strings/str_format.h"
#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabelas.h"
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
  } catch (const arq::ParseProtoException& erro) {
    LOG(ERROR) << "Erro carregando controle virtual: " << erro.what();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErro(erro.what()));
  } catch (const std::logic_error& erro) {
    LOG(ERROR) << "Erro carregando controle virtual: " << erro.what();
    return;
  }
  auto n = ntf::NovaNotificacao(ntf::TN_CARREGAR_TEXTURA);
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
  for (const auto& arma : tabelas_.todas().tabela_armas().armas()) {
    if (arma.acao().has_icone()) {
      n->add_info_textura()->set_id(arma.acao().icone());
    }
  }
  for (const auto& feitico : tabelas_.todas().tabela_feiticos().armas()) {
    if (feitico.acao().has_icone()) {
      n->add_info_textura()->set_id(feitico.acao().icone());
    }
  }

  central_->AdicionaNotificacao(n.release());

  texturas_entidades_.insert(ROTULO_PADRAO);
  try {
    std::vector<std::string> texturas = arq::ConteudoDiretorio(arq::TIPO_TEXTURA, FiltroTexturaEntidade);
    // insere.
    texturas_entidades_.insert(texturas.begin(), texturas.end());
  } catch (const std::logic_error& e) {
    LOG(ERROR) << "Erro carregando lista de texturas do controle virtual: " << e.what();
  }

  for (const auto& modelo : tabelas_.TodosModelosEntidades().modelo()) {
    modelos_entidades_.insert(modelo.id());
  }
  for (const auto& par : tabelas_.TodosItensMenu()) {
    itens_menu_.insert(par.first);
  }
}

void Tabuleiro::LiberaControleVirtual() {
  auto n = ntf::NovaNotificacao(ntf::TN_DESCARREGAR_TEXTURA);
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
  for (const auto& arma : tabelas_.todas().tabela_armas().armas()) {
    if (arma.acao().has_icone()) {
      n->add_info_textura()->set_id(arma.acao().icone());
    }
  }
  for (const auto& feitico : tabelas_.todas().tabela_feiticos().armas()) {
    if (feitico.acao().has_icone()) {
      n->add_info_textura()->set_id(feitico.acao().icone());
    }
  }
  central_->AdicionaNotificacao(n.release());
}

void AbreDialogoForcarDado(int nfaces, ntf::CentralNotificacoes& central) {
  auto n(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_FORCAR_DADO));
  n->set_id_generico(nfaces);
  central.AdicionaNotificacao(n.release());
}

void Tabuleiro::PickingControleVirtual(int x, int y, bool alterna_selecao, bool duplo, int id) {
  VLOG(1) << "picking id: " << id << ", duplo: " << duplo << ", alterna selecao: " << alterna_selecao;
  contador_pressao_por_controle_[IdBotao(id)]++;
  switch (id) {
    case CONTROLE_DESCANSAR_PERSONAGEM:
      DescansaPersonagemNotificando();
      break;
    case CONTROLE_DERRUBAR:
      AlternaAtaqueDerrubar();
      break;
    case CONTROLE_DESARMAR:
      AlternaAtaqueDesarmar();
      break;
    case CONTROLE_USAR_FEITICO_0:
    case CONTROLE_USAR_FEITICO_1:
    case CONTROLE_USAR_FEITICO_2:
    case CONTROLE_USAR_FEITICO_3:
    case CONTROLE_USAR_FEITICO_4:
    case CONTROLE_USAR_FEITICO_5:
    case CONTROLE_USAR_FEITICO_6:
    case CONTROLE_USAR_FEITICO_7:
    case CONTROLE_USAR_FEITICO_8:
    case CONTROLE_USAR_FEITICO_9:
      TrataBotaoUsarFeitico(alterna_selecao, id - CONTROLE_USAR_FEITICO_0);
      break;
    case CONTROLE_CLASSE_FEITICO_ATIVA:
      TrataMudarClasseFeiticoAtiva();
      break;
    case CONTROLE_ROLAR_D2:
      AlternaModoDado(2);
      break;
    case CONTROLE_ROLAR_D3:
      AlternaModoDado(3);
      break;
    case CONTROLE_ROLAR_D4:
      AlternaModoDado(4);
      break;
    case CONTROLE_ROLAR_D6:
      AlternaModoDado(6);
      break;
    case CONTROLE_ROLAR_D8:
      AlternaModoDado(8);
      break;
    case CONTROLE_ROLAR_D10:
      AlternaModoDado(10);
      break;
    case CONTROLE_ROLAR_D12:
      AlternaModoDado(12);
      break;
    case CONTROLE_ROLAR_D20:
      AlternaModoDado(20);
      break;
    case CONTROLE_ROLAR_D100:
      AlternaModoDado(100);
      break;
    case CONTROLE_DADOS:
      mostrar_dados_ = !mostrar_dados_;
      break;
    case CONTROLE_FORCAR_D2:
      if (alterna_selecao) LimpaDadosAcumulados(2);
      else AbreDialogoForcarDado(2, *central_);
      break;
    case CONTROLE_FORCAR_D3:
      if (alterna_selecao) LimpaDadosAcumulados(3);
      else AbreDialogoForcarDado(3, *central_);
      break;
    case CONTROLE_FORCAR_D4:
      if (alterna_selecao) LimpaDadosAcumulados(4);
      else AbreDialogoForcarDado(4, *central_);
      break;
    case CONTROLE_FORCAR_D6:
      if (alterna_selecao) LimpaDadosAcumulados(6);
      else AbreDialogoForcarDado(6, *central_);
      break;
    case CONTROLE_FORCAR_D8:
      if (alterna_selecao) LimpaDadosAcumulados(8);
      else AbreDialogoForcarDado(8, *central_);
      break;
    case CONTROLE_FORCAR_D10:
      if (alterna_selecao) LimpaDadosAcumulados(10);
      else AbreDialogoForcarDado(10, *central_);
      break;
    case CONTROLE_FORCAR_D12:
      if (alterna_selecao) LimpaDadosAcumulados(12);
      else AbreDialogoForcarDado(12, *central_);
      break;
    case CONTROLE_FORCAR_D20:
      if (alterna_selecao) LimpaDadosAcumulados(20);
      else AbreDialogoForcarDado(20, *central_);
      break;
    case CONTROLE_FORCAR_D100:
      if (alterna_selecao) LimpaDadosAcumulados(100);
      else AbreDialogoForcarDado(100, *central_);
      break;
    case CONTROLE_DADOS_FORCADOS:
      mostrar_dados_forcados_ = !mostrar_dados_forcados_;
      break;
    case CONTROLE_ROLAR_PERICIA: {
      auto n(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ESCOLHER_PERICIA));
      std::vector<const Entidade*> entidades = EntidadesSelecionadas();
      for (const auto* entidade : entidades) {
        auto* e = n->add_notificacao()->mutable_entidade();
        e->set_id(entidade->Id());
        *e->mutable_info_classes() = entidade->Proto().info_classes();
        *e->mutable_info_pericias() = entidade->Proto().info_pericias();
        *e->mutable_atributos() = entidade->Proto().atributos();
      }
      if (entidades.size() == 1 && alterna_selecao && !entidades[0]->UltimaPericia().empty()) {
        EntraModoPericia(entidades[0]->UltimaPericia(), *n);
      } else {
        central_->AdicionaNotificacao(n.release());
      }
      break;
    }
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
    case CONTROLE_MODO_ESQUIVA:
      if (alterna_selecao) {
        DesligaEsquivaNotificando();
      } else {
        AlternaModoEsquiva();
      }
      break;
    case CONTROLE_FURIA:
      AlternaFuria();
      break;
    case CONTROLE_DEFESA_TOTAL:
      AlternaDefesaTotal();
      break;
    case CONTROLE_LUTA_DEFENSIVA:
      AlternaLutaDefensiva();
      break;
    case CONTROLE_ATAQUE_PODEROSO:
      AlternaAtaquePoderoso();
      break;
    case CONTROLE_CIMA:
      TrataMovimentoEntidadesSelecionadasOuCamera(true, 1.0f);
      break;
    case CONTROLE_BAIXO:
      TrataMovimentoEntidadesSelecionadasOuCamera(true, -1.0f);
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
      TrataMovimentoEntidadesSelecionadasOuCamera(false, -1.0f);
      break;
    case CONTROLE_DIREITA:
      TrataMovimentoEntidadesSelecionadasOuCamera(false, 1.0f);
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
    case CONTROLE_ALTERNAR_FLANQUEANDO:
      AlternaFlanqueandoEntidadesSelecionadasNotificando();
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
    case CONTROLE_ALTERNAR_EM_CORPO_A_CORPO:
      AlternaEmCorpoACorpoNotificando();
      break;
    case CONTROLE_SURPRESO:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_SURPRESO);
      break;
    case CONTROLE_INVESTIDA:
      AlternaInvestida();
      break;
    case CONTROLE_MODO_MONTAR:
      AlternaMontar();
      break;
    case CONTROLE_ALTERNAR_MODELOS_DESLIGAVEIS_ENTIDADE:
      AlternaModelosDesligaveisNotificando();
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
    case CONTROLE_ATAQUE_MAIS_16:
      AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_ATAQUE_MAIS_16);
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
    case CONTROLE_BEBER_POCAO: {
      const auto* e = EntidadePrimeiraPessoaOuSelecionada();
      if (e == nullptr || (e->Proto().tesouro().pocoes().empty() && e->Proto().tesouro().itens_mundanos().empty())) return;
      auto n(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ESCOLHER_POCAO));
      n->mutable_entidade()->set_id(e->Id());
      *n->mutable_entidade()->mutable_tesouro()->mutable_pocoes() = e->Proto().tesouro().pocoes();
      *n->mutable_entidade()->mutable_tesouro()->mutable_itens_mundanos() = e->Proto().tesouro().itens_mundanos();
      central_->AdicionaNotificacao(n.release());
      break;
    }
    case CONTROLE_USAR_PERGAMINHO_ARCANO:
    case CONTROLE_USAR_PERGAMINHO_DIVINO: {
      const auto* e = EntidadePrimeiraPessoaOuSelecionada();
      if (e == nullptr) return;
      if (id == CONTROLE_USAR_PERGAMINHO_ARCANO && e->Proto().tesouro().pergaminhos_arcanos().empty()) return;
      if (id == CONTROLE_USAR_PERGAMINHO_DIVINO && e->Proto().tesouro().pergaminhos_divinos().empty()) return;
      auto n(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ESCOLHER_PERGAMINHO));
      n->mutable_entidade()->set_id(e->Id());
      *n->mutable_entidade()->mutable_info_classes() = e->Proto().info_classes();
      if (id == CONTROLE_USAR_PERGAMINHO_ARCANO) {
        *n->mutable_entidade()->mutable_tesouro()->mutable_pergaminhos_arcanos() = e->Proto().tesouro().pergaminhos_arcanos();
      } else {
        *n->mutable_entidade()->mutable_tesouro()->mutable_pergaminhos_divinos() = e->Proto().tesouro().pergaminhos_divinos();
      }
      central_->AdicionaNotificacao(n.release());
      break;
    }
    case CONTROLE_FALHA_20:
    case CONTROLE_FALHA_50:
    case CONTROLE_FALHA_NEGATIVO:
      AlternaBitsEntidadeNotificando(
          id == CONTROLE_FALHA_20
            ? ent::Tabuleiro::BIT_FALHA_20
            : id == CONTROLE_FALHA_50
                ? ent::Tabuleiro::BIT_FALHA_50
                : ent::Tabuleiro::BIT_FALHA_NEGATIVO);
      break;
    case CONTROLE_VISIBILIDADE: {
      if (alterna_selecao) {
        RemoveEfeitoInvisibilidadeEntidadesNotificando();
      } else {
        AlternaBitsEntidadeNotificando(ent::Tabuleiro::BIT_VISIBILIDADE);
      }
      break;
    }
    case CONTROLE_AGARRANDO:
      DesagarraEntidadesSelecionadasNotificando();
      break;
    case CONTROLE_ALTERAR_FORMA:
      AlteraFormaEntidadeNotificando();
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
        if (!duplo) {
          auto grupo = NovoGrupoNotificacoes();
          PreenchePassaUmaRodada(/*passar_para_todos=*/true, grupo.get(), nullptr, /*expira_eventos_zerados=*/false);
          TrataNotificacao(*grupo);
          AdicionaNotificacaoListaEventos(*grupo);
        } else {
          // O clique duplo tera passado uma rodada já.
          // Tem um bug aqui que precisara de 2 desfazer para desfazer o duplo clique, mas ok.
          auto grupo = NovoGrupoNotificacoes();
          auto grupo_desfazer = NovoGrupoNotificacoes();
          for (int i = 0; i < 9; ++i) {
            auto grupo = NovoGrupoNotificacoes();
            PreenchePassaUmaRodada(/*passar_para_todos=*/true, grupo.get(), grupo_desfazer.get(), /*expira_eventos_zerados=*/false);
            // Tem que ir fazendo rodada a rodada para as entidades irem sendo atualizadas corretamente.
            TrataNotificacao(*grupo);
          }
          AdicionaNotificacaoListaEventos(*grupo_desfazer);
        }
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
    case CONTROLE_PAGINACAO_LISTA_LOG_ESQUERDA:
      pagina_horizontal_log_eventos_ = std::max(0, pagina_horizontal_log_eventos_ - 1);
      break;
    case CONTROLE_PAGINACAO_LISTA_LOG_DIREITA:
      pagina_horizontal_log_eventos_ = std::min(10, pagina_horizontal_log_eventos_ + 1);
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
    case CONTROLE_DESENHO_CONE:
    case CONTROLE_DESENHO_HEMISFERIO: {
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
        { CONTROLE_DESENHO_HEMISFERIO, TF_HEMISFERIO},
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
      if (itens_menu_.size() < 2) {
        return;
      }
      auto it = itens_menu_.find(item_selecionado_.id);
      if (it == itens_menu_.end()) {
        it = itens_menu_.begin();
        ++it;
      }
      SelecionaModelosEntidades(tabelas_.ItemMenu(*(it == itens_menu_.begin() ? --itens_menu_.end() : --it)).id());
      break;
    }
    case CONTROLE_MODELO_ENTIDADE_PROXIMA: {
      if (itens_menu_.size() < 2) {
        return;
      }
      auto it = itens_menu_.find(item_selecionado_.id);
      if (it != itens_menu_.end()) {
        ++it;
      }
      SelecionaModelosEntidades(tabelas_.ItemMenu(*(it == itens_menu_.end() ? itens_menu_.begin() : it)).id());
      break;
    }
    case CONTROLE_ADICIONA_ENTIDADE: {
      EntraModoClique(MODO_ADICAO_ENTIDADE);
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
    case CONTROLE_MODO_REMOVER_DE_GRUPO:
      AlternaModoRemocaoDeGrupo();
      break;
    case CONTROLE_MODO_MINECRAFT:
      AlternaModoMinecraft();
      break;
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
        auto n = ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_COR_PERSONALIZADA);
        *n->mutable_tabuleiro()->mutable_luz_ambiente() = cor_personalizada_;
        central_->AdicionaNotificacao(n.release());
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
    case CONTROLE_MODO_MOSTRAR_IMAGEM: {
      central_->AdicionaNotificacao(ntf::NovaNotificacao(ntf::TN_ABRIR_DIALOGO_ESCOLHER_IMAGEM));
      // Para teste.
      // EntraModoMostrarImagem();
      break;
    }
    default:
      if (id >= CONTROLE_JOGADORES) {
        // Isso acontece pela UI. O mestre aperta a tecla J, que mostra todos os jogadores como
        // controles e entao ele clica em um dos botoes de jogadores, transformando o jogador em
        // mestre tambem.
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
  // Por quantos frames um botao fica pressionado. 240ms.
  const auto atualizacoes_botao_pressionado = static_cast<int>(opcoes_.fps()) * 0.240f;
  if (num_frames > atualizacoes_botao_pressionado) {
    // Ficou suficiente, volta no proximo.
    num_frames = 0;
  }
  // O botao pode estar pressionado ou ativado.
  return true;
}

// Funcao para retornar o id de botao que melhor representa o estado do clique com a forma selecionada.
IdBotao Tabuleiro::ModoCliqueParaId(Tabuleiro::modo_clique_e mc, TipoForma tf) const {
  switch (mc) {
    case Tabuleiro::MODO_DESENHO: {
      switch (tf) {
        case TF_LIVRE:     return CONTROLE_DESENHO_LIVRE;
        case TF_RETANGULO: return CONTROLE_DESENHO_RETANGULO;
        case TF_CIRCULO:   return CONTROLE_DESENHO_CIRCULO;
        case TF_ESFERA:
          return CONTROLE_DESENHO_ESFERA;
        case TF_PIRAMIDE:  return CONTROLE_DESENHO_PIRAMIDE;
        case TF_TRIANGULO: return CONTROLE_DESENHO_TRIANGULO;
        case TF_CUBO:      return CONTROLE_DESENHO_CUBO;
        case TF_CILINDRO:  return CONTROLE_DESENHO_CILINDRO;
        case TF_CONE:      return CONTROLE_DESENHO_CONE;
        case TF_HEMISFERIO:return CONTROLE_DESENHO_HEMISFERIO;
        default:           return CONTROLE_DESENHO_LIVRE;
      }
    }
    case Tabuleiro::MODO_AJUDA:       return CONTROLE_AJUDA;
    case Tabuleiro::MODO_SINALIZACAO: return CONTROLE_ACAO_SINALIZACAO;
    case Tabuleiro::MODO_TRANSICAO:   return CONTROLE_TRANSICAO;
    case Tabuleiro::MODO_REGUA:       return CONTROLE_REGUA;
    case Tabuleiro::MODO_ROLA_DADO:   return faces_dado_ == 20 ? CONTROLE_ROLAR_D20 : CONTROLE_ROLAR_D100;
    case Tabuleiro::MODO_ROTACAO:     return CONTROLE_MODO_ROTACAO;
    case Tabuleiro::MODO_TERRENO:     return CONTROLE_MODO_TERRENO;
    case Tabuleiro::MODO_ESQUIVA:     return CONTROLE_MODO_ESQUIVA;
    case Tabuleiro::MODO_MONTAR:      return CONTROLE_MODO_MONTAR;
    case Tabuleiro::MODO_AGUARDANDO:  return CONTROLE_MODO_AGUARDANDO;
    case Tabuleiro::MODO_PERICIA:     return CONTROLE_ROLAR_PERICIA;
    case Tabuleiro::MODO_ADICAO_ENTIDADE: return CONTROLE_ADICIONA_ENTIDADE;
    case Tabuleiro::MODO_MOSTRAR_IMAGEM: return CONTROLE_MODO_MOSTRAR_IMAGEM;
    default:                          return CONTROLE_AJUDA;
  }
}

// Retorna o id da textura para uma determinada acao.
// Aka IconeBotao.
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
        const auto& da = entidade->DadoCorrenteNaoNull();
        unsigned int textura = texturas_->Textura(da.icone());
        return textura == GL_INVALID_VALUE ? textura_espada : textura;
      } else {
        auto it = mapa_botoes_controle_virtual_.find(ModoCliqueParaId(modo_clique_, forma_selecionada_));
        if (it != mapa_botoes_controle_virtual_.end()) {
          return it->second->textura().empty() ? GL_INVALID_VALUE : texturas_->Textura(it->second->textura());
        }
      }
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
    return static_cast<int>(viewport[2] + coluna * unidade_largura);
  } else if (db.alinhamento_horizontal() == ALINHAMENTO_CENTRO) {
    return static_cast<int>((viewport[2] / 2) + coluna * unidade_largura);
  } else {
    return static_cast<int>(coluna * unidade_largura);
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
    case CONTROLE_DADOS:
      return mostrar_dados_;
    case CONTROLE_DADOS_FORCADOS:
      return mostrar_dados_forcados_;
    default:
      return false;
  }
}

bool Tabuleiro::BotaoVisivel(const DadosBotao& db) const {
  if (db.has_visibilidade()) {
    for (const auto& ref : db.visibilidade().referencia()) {
      // So retorna os false, por causa do encadeamento que eh AND.
      switch (ref.tipo()) {
        case VIS_INVISIVEL:
          return false;
        case VIS_CAMERA_PRESA: {
          if (!camera_presa_) return false;
          break;
        }
        case VIS_ENTIDADE_PRIMEIRA_PESSOA_OU_SELECIONADAS: {
          if (IdsPrimeiraPessoaOuEntidadesSelecionadas().empty()) return false;
          break;
        }
        case VIS_CAMERA_PRIMEIRA_PESSOA_OU_SELECIONADA: {
          if (IdsPrimeiraPessoaOuEntidadesSelecionadas().empty()) {
            return false;
          }
          break;
        }
        case VIS_CAMERA_PRIMEIRA_PESSOA: {
          if (camera_ != CAMERA_PRIMEIRA_PESSOA || EntidadePrimeiraPessoaOuSelecionada() == nullptr) {
            return false;
          }
          break;
        }
        case VIS_POCAO: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr ||
              (e->Proto().tesouro().pocoes().empty() && c_none_of(e->Proto().tesouro().itens_mundanos(), [](const ItemMagicoProto& mundano) { return mundano.id() == "antidoto"; }))) {
            return false;
          }
          break;
        }
        case VIS_PERGAMINHO_ARCANO: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || e->Proto().tesouro().pergaminhos_arcanos().empty()) {
            return false;
          }
          break;
        }
        case VIS_PERGAMINHO_DIVINO: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || e->Proto().tesouro().pergaminhos_divinos().empty()) {
            return false;
          }
          break;
        }
        case VIS_ATAQUE_FURTIVO: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || e->Proto().dados_ataque_global().dano_furtivo().empty()) {
            return false;
          }
          break;
        }
        case VIS_FORMA_ALTERNATIVA: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || e->Proto().formas_alternativas_size() <= 1) {
            return false;
          }
          break;
        }
        case VIS_ESQUIVA: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || !PossuiTalento("esquiva", e->Proto())) {
            return false;
          }
          break;
        }
        case VIS_FURIA: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || Nivel("barbaro", e->Proto()) == 0) {
            return false;
          }
          break;
        }
        case VIS_INVERSO_DE: {
          if (EstadoBotao(ref.id()) == true) {
            return false;
          }
          break;
        }
        case VIS_IGUAL_A: {
          if (EstadoBotao(ref.id()) == false) {
            return false;
          }
          break;
        }
        case VIS_FEITICO_0:
        case VIS_FEITICO_1:
        case VIS_FEITICO_2:
        case VIS_FEITICO_3:
        case VIS_FEITICO_4:
        case VIS_FEITICO_5:
        case VIS_FEITICO_6:
        case VIS_FEITICO_7:
        case VIS_FEITICO_8:
        case VIS_FEITICO_9: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr) return false;
          const auto& id_classe = ClasseFeiticoAtiva(e->Proto());
          if (id_classe.empty()) return false;
          const auto& fn = FeiticosNivel(id_classe, ref.tipo() - VIS_FEITICO_0, e->Proto());
          return std::any_of(fn.para_lancar().begin(), fn.para_lancar().end(),
              [] (const EntidadeProto::InfoLancar& il) {
            return !il.usado();
          });
        }
        case VIS_CLASSE_FEITICO_ATIVA: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr) return false;
          const auto& id_classe = ClasseFeiticoAtiva(e->Proto());
          return !id_classe.empty();
        }
        case VIS_ATAQUE_DERRUBAR: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr) return false;
          const auto* da = e->DadoCorrente();
          if (da == nullptr) return false;
          return tabelas_.Arma(da->id_arma()).pode_derrubar();
        }
        case VIS_ATAQUE_PODEROSO: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr || !e->PossuiTalento("ataque_poderoso")) return false;
          return true;
        }
          /*
        case VIS_ATAQUE_DESARMAR: {
          const auto* e = EntidadePrimeiraPessoaOuSelecionada();
          if (e == nullptr) return false;
          const auto* da = e->DadoCorrente();
          if (da == nullptr) return false;
          return PossuiTalento() || tabelas_.Arma(da->id_arma()).pode_desarmar();
        }
        */

        default: {
          LOG(WARNING) << "Tipo de visibilidade de botao invalido: " << ref.tipo();
        }
      }
    }
  }
  return true;
}

std::string Tabuleiro::RotuloBotaoControleVirtual(const DadosBotao& db, const Entidade* entidade) const {
  switch (db.id()) {
    case CONTROLE_RODADA: {
      return net::to_string(proto_.contador_rodadas());
    }
    case CONTROLE_CLASSE_FEITICO_ATIVA: {
      return tabelas_.Classe(ClasseFeiticoAtiva(entidade->Proto())).nome();
    }
    case CONTROLE_TEXTURA_ENTIDADE: {
      std::string rotulo = TexturaEntidade(EntidadesSelecionadas());
      return rotulo.empty() ? "-" : rotulo;
    }
    case CONTROLE_MODELO_ENTIDADE: {
      return item_selecionado_.id.empty() ? "-" : item_selecionado_.id;
    }
    case CONTROLE_USAR_FEITICO_0:
    case CONTROLE_USAR_FEITICO_1:
    case CONTROLE_USAR_FEITICO_2:
    case CONTROLE_USAR_FEITICO_3:
    case CONTROLE_USAR_FEITICO_4:
    case CONTROLE_USAR_FEITICO_5:
    case CONTROLE_USAR_FEITICO_6:
    case CONTROLE_USAR_FEITICO_7:
    case CONTROLE_USAR_FEITICO_8:
    case CONTROLE_USAR_FEITICO_9: {
      const auto& id_classe = ClasseFeiticoAtiva(entidade->Proto());
      const auto& fn = FeiticosNivel(id_classe, db.id() - CONTROLE_USAR_FEITICO_0, entidade->Proto());
      return net::to_string((int)std::count_if(fn.para_lancar().begin(), fn.para_lancar().end(),
           [] (const ent::EntidadeProto::InfoLancar& il) {
         return !il.usado();
      }));
    }
    default:
      return db.rotulo();
  }
}

std::string DicaBotao(const DadosBotao& db, const Entidade* entidade) {
  return db.dica();
}

void Tabuleiro::DesenhaDicaBotaoControleVirtual(
    const DadosBotao& db, const GLint* viewport, float fonte_x, float fonte_y, float padding, float unidade_largura, float unidade_altura,
    const Entidade* entidade) {
  float largura_botao = db.has_largura() ? db.largura() : db.tamanho();
  float altura_botao = db.has_altura() ? db.altura() : db.tamanho();
  float xi, xf, yi, yf;
  xi = TranslacaoX(db, viewport, unidade_largura);
  xf = xi + largura_botao * unidade_largura;
  yi = TranslacaoY(db, viewport, unidade_altura);
  yf = yi + altura_botao * unidade_altura;
  float x_meio = (xi + xf) / 2.0f;
  std::string dica = StringSemUtf8(DicaBotao(db, entidade));
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
  std::string rotulo = StringSemUtf8(RotuloBotaoControleVirtual(db, entidade));
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
  if (indice_iniciativa_ < 0 || iniciativas_.empty()) {
    //LOG(INFO) << "Nao " << indice_iniciativa_ << ", iniciativas_.size " << iniciativas_.size();
    return;
  }
  int largura_fonte, altura_fonte;
  float escala;
  gl::TamanhoFonte(&largura_fonte, &altura_fonte, &escala);

  int raster_x = 0, raster_y = 0;
  largura_fonte *= escala;
  altura_fonte *= escala;
  raster_y = altura_ - (altura_fonte * 3);  // deixa a linha livre pra nao baguncar acoes.
  raster_x = (largura_fonte  * 9) + 2;
  PosicionaRaster2d(raster_x, raster_y);

  const unsigned int indice_corrigido = static_cast<unsigned int>(indice_iniciativa_) < iniciativas_.size() ? indice_iniciativa_ : iniciativas_.size() - 1;
  MudaCor(COR_AMARELA);
  std::string titulo = iniciativa_valida_
    ? absl::StrFormat("Iniciativa: %d/%d", iniciativas_[indice_corrigido].iniciativa, iniciativas_[indice_corrigido].modificador)
    : "Iniciativa INVÁLIDA (passar para próxima)";
  gl::DesenhaStringAlinhadoEsquerda(StringSemUtf8(titulo));
  MudaCor(COR_BRANCA);
  raster_y -= (altura_fonte + 2);
  int num_desenhadas = 0;

  std::vector<DadosIniciativa> entidades_na_ordem_desenho;
  entidades_na_ordem_desenho.reserve(iniciativas_.size());
  entidades_na_ordem_desenho.insert(entidades_na_ordem_desenho.end(), iniciativas_.begin() + indice_corrigido, iniciativas_.end());
  entidades_na_ordem_desenho.insert(entidades_na_ordem_desenho.end(), iniciativas_.begin(), iniciativas_.begin() + indice_corrigido);

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
    // Nao desenha eventos: agora que ta tudo automatico isso confunde muito na tela.
    //std::string eventos = entidade->ResumoEventos();
    //if (!eventos.empty()) {
    //  rotulo += ": " + eventos;
    //}
    if (entidade->Morta()) {
      rotulo += " (morta)";
    }
    if (entidade->IdCenario() != IdCenario()) {
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
  int largura_fonte, altura_fonte;
  float escala;
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
    const auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
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
    std::vector<int> faces = {2, 3, 4, 6, 8, 10, 12, 20, 100};
    bool tem_forcado = false;
    for (int nfaces : faces) {
      std::optional<DadoTesteOuForcado> tof = TemDadoDeTesteOuForcado(nfaces);
      if (tof.has_value() && *tof == DadoTesteOuForcado::FORCADO) {
        tem_forcado = true;
        break;
      }
    }
    if (tem_forcado) {
      raster_y -= (altura_fonte + 2);
      PosicionaRaster2d(raster_x, raster_y);
      gl::DesenhaStringAlinhadoDireita("Dados forcados");
      for (int nfaces : faces) {
        std::optional<int> valor = DadoAcumulado(nfaces);
        if (valor == std::nullopt) continue;
        raster_y -= (altura_fonte + 2);
        PosicionaRaster2d(raster_x, raster_y);
        gl::DesenhaStringAlinhadoDireita(absl::StrFormat("D%d: %d", nfaces, *valor));
      }
    }
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
  int fonte_x_int, fonte_y_int;
  float escala;
  gl::TamanhoFonte(&fonte_x_int, &fonte_y_int, &escala);
  fonte_x_int *= escala;
  fonte_y_int *= escala;
  const float fonte_x = fonte_x_int;
  const float fonte_y = fonte_y_int;
  const float altura_botao = fonte_y * MULTIPLICADOR_ALTURA;
  const float largura_botao = fonte_x * MULTIPLICADOR_LARGURA;
  //const float largura_botao = altura_botao;
  const float padding = parametros_desenho_.has_picking_x() ? 0 : fonte_x / 4;

  // Mapeia id do botao para a funcao de estado. A entidade recebida vem da funcao EntidadePrimeiraPessoaOuSelecionada.
  static const std::unordered_map<int, std::function<bool(const Entidade* entidade)>> mapa_botoes = {
    { CONTROLE_DERRUBAR,          [this] (const Entidade* entidade) {
       return entidade != nullptr && entidade->DadoCorrenteNaoNull().ataque_derrubar();
    } },
    { CONTROLE_DESARMAR,          [this] (const Entidade* entidade) {
       return entidade != nullptr && entidade->DadoCorrenteNaoNull().ataque_desarmar();
    } },
    { CONTROLE_ACAO,              [this] (const Entidade* entidade) { return modo_clique_ != MODO_NORMAL; } },
    { CONTROLE_AJUDA,             [this] (const Entidade* entidade) { return modo_clique_ == MODO_AJUDA; } },
    { CONTROLE_ADICIONA_ENTIDADE, [this] (const Entidade* entidade) { return modo_clique_ == MODO_ADICAO_ENTIDADE; } },
    { CONTROLE_TRANSICAO,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_TRANSICAO; } },
    { CONTROLE_REGUA,             [this] (const Entidade* entidade) { return modo_clique_ == MODO_REGUA; } },
    { CONTROLE_ROLAR_D2,          [this](const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 2; } },
    { CONTROLE_ROLAR_D3,          [this](const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 3; } },
    { CONTROLE_ROLAR_D4,          [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 4; } },
    { CONTROLE_ROLAR_D6,          [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 6; } },
    { CONTROLE_ROLAR_D8,          [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 8; } },
    { CONTROLE_ROLAR_D10,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 10; } },
    { CONTROLE_ROLAR_D12,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 12; } },
    { CONTROLE_ROLAR_D20,         [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 20; } },
    { CONTROLE_ROLAR_D100,        [this] (const Entidade* entidade) { return modo_clique_ == MODO_ROLA_DADO && faces_dado_ == 100; } },
    { CONTROLE_MODO_TERRENO,      [this] (const Entidade* entidade) { return modo_clique_ == MODO_TERRENO; } },
    { CONTROLE_MODO_REMOVER_DE_GRUPO,      [this] (const Entidade* entidade) { return modo_clique_ == MODO_REMOCAO_DE_GRUPO; } },
    { CONTROLE_MODO_MINECRAFT,    [this] (const Entidade* entidade) { return modo_clique_ == MODO_MINECRAFT; } },
    { CONTROLE_MODO_ESQUIVA,      [this] (const Entidade* entidade) {
      if (modo_clique_ == MODO_ESQUIVA) return true;
      return entidade != nullptr && entidade->Proto().dados_defesa().has_entidade_esquiva();
    } },
    { CONTROLE_DEFESA_TOTAL,      [this] (const Entidade* entidade) {
      if (entidade == nullptr) return false;
      return EmDefesaTotal(entidade->Proto());
    } },
    { CONTROLE_LUTA_DEFENSIVA,      [this] (const Entidade* entidade) {
      if (entidade == nullptr) return false;
      return LutandoDefensivamente(entidade->Proto());
    } },
    { CONTROLE_ATAQUE_PODEROSO,      [this] (const Entidade* entidade) {
      if (entidade == nullptr || !entidade->PossuiTalento("ataque_poderoso")) return false;
      return AtacandoPoderosamente(entidade->Proto());
    } },
    { CONTROLE_FURIA,             [this] (const Entidade* entidade) {
      if (entidade == nullptr) return false;
      for (const auto& e : entidade->Proto().evento()) {
        if (e.id_efeito() == EFEITO_FURIA_BARBARO) return true;
      }
      return false;
    } },
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
    { CONTROLE_ALTERNAR_FLANQUEANDO,        [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().flanqueando();
    } },
    { CONTROLE_QUEDA,              [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().caida();
    } },
    { CONTROLE_SELECIONAVEL,       [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().selecionavel_para_jogador();
    } },
    { CONTROLE_FURTIVO,            [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().furtivo();
    } },
    { CONTROLE_ALTERNAR_EM_CORPO_A_CORPO, [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().em_corpo_a_corpo();
    } },
    { CONTROLE_SURPRESO,           [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().surpreso();
    } },
    { CONTROLE_INVESTIDA,          [this] (const Entidade* entidade) {
      return entidade != nullptr && PossuiEvento(EFEITO_INVESTIDA, entidade->Proto());
    } },
    { CONTROLE_MODO_MONTAR,        [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().has_montado_em();
    } },
    { CONTROLE_MODO_MOSTRAR_IMAGEM,        [this](const Entidade* entidade) {
      return false;  // este botão só é desenhado se não estiver no modo screenshot, então ele sempre está inativo.
    } },
    { CONTROLE_ALTERNAR_MODELOS_DESLIGAVEIS_ENTIDADE, [this] (const Entidade* entidade) {
      return entidade != nullptr && EntidadeTemModeloDesligavelLigado(tabelas_, entidade->Proto());
    } },
    { CONTROLE_ILUMINACAO_MESTRE,  [this] (const Entidade* entidade) {
      return !opcoes_.iluminacao_mestre_igual_jogadores();
    } },
    // Ataque.
    { CONTROLE_ATAQUE_MAIS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_mais_1();
    } },
    { CONTROLE_ATAQUE_MAIS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_mais_2();
    } },
    { CONTROLE_ATAQUE_MAIS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_mais_4();
    } },
    { CONTROLE_ATAQUE_MAIS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_mais_8();
    } },
    { CONTROLE_ATAQUE_MAIS_16,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_mais_16();
    } },
    { CONTROLE_ATAQUE_MENOS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_menos_1();
    } },
    { CONTROLE_ATAQUE_MENOS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_menos_2();
    } },
    { CONTROLE_ATAQUE_MENOS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_menos_4();
    } },
    { CONTROLE_ATAQUE_MENOS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().ataque_menos_8();
    } },

    // Dano.
    { CONTROLE_DANO_MAIS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_1();
    } },
    { CONTROLE_DANO_MAIS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_2();
    } },
    { CONTROLE_DANO_MAIS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_4();
    } },
    { CONTROLE_DANO_MAIS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_8();
    } },
    { CONTROLE_DANO_MAIS_16,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_16();
    } },
    { CONTROLE_DANO_MAIS_32,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_mais_32();
    } },

    { CONTROLE_DANO_MENOS_1,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_menos_1();
    } },
    { CONTROLE_DANO_MENOS_2,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_menos_2();
    } },
    { CONTROLE_DANO_MENOS_4,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_menos_4();
    } },
    { CONTROLE_DANO_MENOS_8,      [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().dano_menos_8();
    } },

    { CONTROLE_VOO,          [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().voadora();
    } },
    { CONTROLE_FALHA_20,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().chance_falha() == 20;
    } },
    { CONTROLE_FALHA_50,     [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().chance_falha() == 50;
    } },
    { CONTROLE_FALHA_NEGATIVO, [this] (const Entidade* entidade) {
      return entidade != nullptr && entidade->Proto().dados_ataque_global().chance_falha() < 0;
    } },
    { CONTROLE_VISIBILIDADE, [this] (const Entidade* entidade) {
      return entidade != nullptr && !entidade->Proto().visivel();
    } },
    { CONTROLE_AGARRANDO, [this] (const Entidade* entidade) {
      return entidade != nullptr && !entidade->Proto().agarrado_a().empty();
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
    { CONTROLE_DESENHO_HEMISFERIO, [this] (const Entidade* entidade) {
      return modo_clique_ == MODO_DESENHO && forma_selecionada_ == TF_HEMISFERIO;
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
  gl::MatrizEscopo salva_matriz_2(gl::MATRIZ_MODELAGEM);

  // Todos botoes, mapeados por id.
  std::vector<DadosBotao*> botoes;
  {

    std::unique_ptr<gl::HabilitaEscopo> blend_escopo;
    if (controle_virtual_.modo_debug()) {
      blend_escopo.reset(new gl::HabilitaEscopo(GL_BLEND));
    }
    int pagina_corrente = controle_virtual_.pagina_corrente();
    if (pagina_corrente < 0 || pagina_corrente >= controle_virtual_.pagina_size()) {
      return;
    }
    botoes.reserve(controle_virtual_.pagina(pagina_corrente).dados_botoes_size() + controle_virtual_.fixo().dados_botoes_size());
    for (auto& db : *controle_virtual_.mutable_pagina(pagina_corrente)->mutable_dados_botoes()) {
      if (!controle_virtual_.modo_debug() && !BotaoVisivel(db)) continue;
      botoes.push_back(&db);
    }
    for (auto& db : *controle_virtual_.mutable_fixo()->mutable_dados_botoes()) {
      if (!controle_virtual_.modo_debug() && !BotaoVisivel(db)) continue;
      botoes.push_back(&db);
    }
    auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
    for (auto* db : botoes) {
      float ajuste = AtualizaBotaoControleVirtual(db, mapa_botoes, entidade) ? 0.5f : 1.0f;
      float cor[3];
      CorBotao(*db, cor);
      cor[0] *= ajuste;
      cor[1] *= ajuste;
      cor[2] *= ajuste;
      if (!controle_virtual_.modo_debug()) {
        gl::MudaCor(cor[0], cor[1], cor[2], 1.0f);
      } else {
        gl::MudaCor(1.0f, 0.0f, 0.0f, 0.2f);
      }
      DesenhaBotaoControleVirtual(*db, viewport, padding, largura_botao, altura_botao, entidade);
      //LOG(INFO) << "timer: " << ((int)(timer_uma_renderizacao_completa_.elapsed().wall / DIV_NANO_PARA_SEGUNDOS)) << ", botao: " << db->dica();
    }

    // Rotulos dos botoes.
    if (!controle_virtual_.modo_debug()) {
      for (const auto* db : botoes) {
        DesenhaRotuloBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao, entidade);
      }
    }
  }

  // Informacao da entidade primeira pessoa. Uma barra na esquerda, com número abaixo.
  // Barra de pontos de vida.
  // aka DesenhaInfoPrimeiraPessoa.
  if (EntidadePrimeiraPessoaOuSelecionada()) {
    DesenhaInfoCameraPresa();
  }

  if (parametros_desenho_.desenha_lista_pontos_vida()) {
    DesenhaListaPontosVida();
  }
  if (parametros_desenho_.desenha_iniciativas()) {
    DesenhaIniciativas();
  }

  // Desenha dicas por ultimo.
  if (tipo_entidade_detalhada_ == OBJ_CONTROLE_VIRTUAL) {
    auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
    // Dicas.
    for (const auto* db : botoes) {
      if (id_entidade_detalhada_ != static_cast<unsigned int>(db->id()) || db->dica().empty()) {
        continue;
      }
      DesenhaDicaBotaoControleVirtual(*db, viewport, fonte_x, fonte_y, padding, largura_botao, altura_botao, entidade);
    }
  }

  V_ERRO("desenhando lista pontos de vida");

  // So volta a luz se havia iluminacao antes.
  if (parametros_desenho_.iluminacao()) {
    gl::Habilita(GL_LIGHTING);
  }
  gl::Habilita(GL_DEPTH_TEST);
}

void Tabuleiro::DesenhaInfoCameraPresa() {
  const auto* entidade = EntidadePrimeiraPessoaOuSelecionada();
  if (entidade == nullptr || entidade->MaximoPontosVida() == 0) {
    return;
  }
  gl::TipoEscopo tipo_escopo(OBJ_ENTIDADE_LISTA);
  gl::CarregaNome(entidade->Id());

  int fonte_x_int, fonte_y_int;
  float escala;
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
  float altura_pv_temporario = 0;
  const int pontos_vida_corrente = entidade->PontosVida();
  const int pontos_vida_temporarios = entidade->PontosVidaTemporarios();
  const int pontos_vida_sem_temporarios = pontos_vida_corrente - pontos_vida_temporarios;
  if (pontos_vida_sem_temporarios > 0) {
    const float fator = static_cast<float>(pontos_vida_sem_temporarios) / entidade->MaximoPontosVida();
    altura_pv_entidade = std::max(0.0f, std::min<float>(altura_maxima, altura_maxima * fator));
  }
  if (pontos_vida_temporarios > 0) {
    // Se a entidade estiver negativa, os pontos temporarios nao podem ser maior que o corrente.
    const int pontos_vida_temporarios_para_display = std::min(pontos_vida_corrente, pontos_vida_temporarios);
    float fator = static_cast<float>(pontos_vida_temporarios_para_display) / entidade->MaximoPontosVida();
    altura_pv_temporario = std::max<float>(0.0f, std::min(altura_maxima, altura_maxima * fator));
  }
  // Pontos de vida total incluindo temporarios.
  gl::MudaCor(1.0f, 0.0f, 0.0f, 1.0f);
  gl::Retangulo(largura_botao, bottom_y, 2.0f * largura_botao, top_y);
  // Pontos de vida correntei incluindo temporarios.
  gl::MudaCor(0.0f, 1.0f, 0.0f, 1.0f);
  gl::Retangulo(largura_botao, bottom_y, 2.0f * largura_botao, bottom_y + altura_pv_entidade);
  // Pontos de vida temporario.
  gl::MudaCor(1.0f, 1.0f, 1.0f, 1.0f);
  gl::Retangulo(largura_botao, bottom_y + altura_pv_entidade, 2.0f * largura_botao, bottom_y + altura_pv_entidade + altura_pv_temporario);

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
