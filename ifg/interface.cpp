#include <algorithm>
#include <functional>

#include "absl/strings/str_format.h"
#include "arq/arquivo.h"
#include "ent/acoes.pb.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "ifg/interface.h"
#include "ifg/modelos.pb.h"
#include "ifg/tecladomouse.h"
#include "log/log.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using std::placeholders::_6;
using std::placeholders::_7;

namespace ifg {

namespace {

std::string StringDuracao(ent::TipoDuracao td) {
  switch (td) {
    case ent::TD_RODADAS_NIVEL: return "rodadas/nível";
    case ent::TD_MINUTOS_NIVEL: return "minutos/nível";
    case ent::TD_HORAS_NIVEL: return "horas/nível";
    case ent::TD_2_HORAS_NIVEL: return "2 horas/nível";
    case ent::TD_10_MINUTOS_NIVEL: return "10 minutos/nível";
    default: ;
  }
  // Nao deveria chegar aqui.
  return "-";
}


std::string StringDuracao(ent::ModificadorRodadas mr) {
  switch (mr) {
    case ent::MR_NENHUM: return "nenhum";
    case ent::MR_RODADAS_NIVEL: return "rodadas/nível";
    case ent::MR_MINUTOS_NIVEL: return "minutos/nível";
    case ent::MR_10_MINUTOS_NIVEL: return "10 minutos/nível";
    case ent::MR_HORAS_NIVEL: return "horas/nível";
    case ent::MR_2_HORAS_NIVEL: return "2 horas/nível";
    case ent::MR_1_RODADA_A_CADA_3_NIVEIS_MAX_6: return "1 rodada/3 níveis (max 6)";
    case ent::MR_10_RODADAS_MAIS_UMA_POR_NIVEL_MAX_15: return "10 rodadas +1/nível (max 15)";
    case ent::MR_HORAS_NIVEL_MAX_15: return "horas/nivel (max 15)";
    case ent::MR_2_MINUTOS_NIVEL: return "2 minutos/nivel";
    case ent::MR_DIAS_POR_NIVEL: return "dias/nivel";
    case ent::MR_PALAVRA_PODER_ATORDOAR: return "variável por PV: max 4d4 rodadas";
    case ent::MR_PALAVRA_PODER_CEGAR: return "variável por PV: max permanente";
    case ent::MR_CONTINUO: return "continuo";
    case ent::MR_MOD_CARISMA: return "modificador de carisma";
    case ent::MR_DRENAR_FORCA_VITAL: return "10 minutos/nivel do alvo";
    case ent::MR_1_RODADA_A_CADA_2_NIVEIS: return "1 rodada/2 níveis";
  }
  // Nao deveria chegar aqui.
  return "desconhecida";
}

std::string StringArea(const ent::AcaoProto& acao) {
  std::string str_geo;
  switch (acao.geometria()) {
    case ent::ACAO_GEO_CONE:
      str_geo = absl::StrFormat("cone %.1f (q)", acao.distancia_quadrados());
      break;
    case ent::ACAO_GEO_CILINDRO:
      str_geo = absl::StrFormat("cilindro raio %.1f (q)", acao.raio_quadrados());
      break;
    case ent::ACAO_GEO_ESFERA:
      str_geo = absl::StrFormat("esfera raio %.1f (q)", acao.raio_quadrados());
      break;
    case ent::ACAO_GEO_CUBO:
      // Isso é mais para modelar objetos geometricos, como flecha acida.
      break;
    default:
      ;
  }
  return str_geo;
}

}  // namespace

bool InterfaceGrafica::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_TIPO_TESOURO:
      TrataEscolherTipoTesouro(notificacao);
      return true;
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_ALIADOS_INIMIGOS:
      TrataEscolherAliados(notificacao);
      return true;
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_DECISAO_LANCAMENTO:
      TrataEscolherDecisaoLancamento(notificacao);
      return true;
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_FEITICO:
      TrataEscolherFeitico(notificacao);
      return true;
    case ntf::TN_ABRIR_DIALOGO_COR_PERSONALIZADA:
      TrataEscolheCor(notificacao);
      return true;
    case ntf::TN_ABRIR_DIALOGO_ABRIR_TABULEIRO: {
      TrataAbrirTabuleiro(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_IMAGEM: {
      TrataAbrirImagem(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO: {
      TrataSalvarTabuleiro(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_MODELO_ENTIDADE: {
      TrataEscolherModeloEntidade(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_POCAO: {
      TrataEscolherPocaoOuAntidoto(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_PERGAMINHO: {
      TrataEscolherPergaminho(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_PERICIA: {
      TrataEscolherPericia(notificacao);
      return true;
    }
    case ntf::TN_INFO:
    case ntf::TN_ERRO: {
      TrataMostraMensagem(notificacao.tipo() == ntf::TN_ERRO, notificacao.erro());
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_ABRIR_VERSAO: {
      TrataEscolherVersao();
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_REMOVER_VERSAO: {
      TrataEscolherVersaoParaRemocao();
      break;
    }
    case ntf::TN_ABRIR_DIALOGO_FORCAR_DADO:
      TrataForcarDado(notificacao);
      return true;
    default:
      break;
  }
  return false;
}

std::string NomeTesouro(ent::TipoTesouro tipo) {
  if (ent::TipoItem_IsValid(tipo)) {
    return ent::TipoItem_Name(static_cast<ent::TipoItem>(tipo));
  }
  switch (tipo) {
    case ent::TT_ARMA: return "Arma";
    case ent::TT_ARMADURA: return "Armadura";
    case ent::TT_ESCUDO: return "Escudo";
    default: return absl::StrFormat("Inválido: %d", tipo);
  }
}

//----------------------
// Escolher Tipo Tesouro
//----------------------
void InterfaceGrafica::TrataEscolherTipoTesouro(const ntf::Notificacao& notificacao) {
  std::vector<ent::TipoTesouro> mapa_indice_tipo;
  std::vector<std::string> nomes_itens;
  for (ent::TipoTesouro tipo : { ent::TT_ANEL, ent::TT_MANTO, ent::TT_LUVAS, ent::TT_BRACADEIRAS,
                              ent::TT_POCAO, ent::TT_AMULETO, ent::TT_BOTAS, ent::TT_CHAPEU,
                              ent::TT_PERGAMINHO_ARCANO, ent::TT_PERGAMINHO_DIVINO, ent::TT_ITEM_MUNDANO,
                              ent::TT_ARMA, ent::TT_ARMADURA, ent::TT_ESCUDO, ent::TT_VARINHA }) {
    if (ent::TipoItem_IsValid(tipo)) {
      const auto& itens = ent::ItensProto(static_cast<ent::TipoItem>(tipo), notificacao.entidade());
      if (itens.empty()) continue;
      nomes_itens.push_back(NomeTesouro(tipo));
      mapa_indice_tipo.push_back(tipo);
    } else {
      const auto& itens = ArmasArmadurasOuEscudosProto(tipo, notificacao.entidade());
      if (itens.empty()) continue;
      nomes_itens.push_back(NomeTesouro(tipo));
      mapa_indice_tipo.push_back(tipo);
    }
  }
  if (nomes_itens.empty()) return;
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheItemLista(
      "Escolha o tipo de item", std::nullopt, nomes_itens,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherTipoTesouro,
          this, notificacao, mapa_indice_tipo,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherTipoTesouro(const ntf::Notificacao notificacao, std::vector<ent::TipoTesouro> mapa_indice_tipo, bool ok, int indice_tipo) {
  ent::RodaNoRetorno r([this] () {
    tabuleiro_->ReativaWatchdogSeMestre();
  });
  if (!ok) {
    return;
  }
  if (indice_tipo < 0 || indice_tipo >= (int)mapa_indice_tipo.size()) {
    // TODO erro
    return;
  }
  const auto* receptor = tabuleiro_->BuscaEntidade(notificacao.id_referencia());
  const auto* doador = tabuleiro_->BuscaEntidade(notificacao.entidade().id());
  if (receptor == nullptr || doador == nullptr) {
    LOG(ERROR) << (receptor == nullptr && doador == nullptr
        ? "receptor e doador são nulos"
        : (receptor == nullptr ? "receptor é nulo" : "doador é nulo"));
    // TODO dar mensagem de erro.
    return;
  }
  ent::TipoTesouro tipo = mapa_indice_tipo[indice_tipo];
  auto grupo = ent::NovoGrupoNotificacoes();
  ent::PreencheNotificacoesTransicaoUmTipoTesouro(tabelas_, tipo, *doador, *receptor, grupo.get(), /*n_desfazer=*/nullptr);
  tabuleiro_->TrataNotificacao(*grupo);
  tabuleiro_->AdicionaNotificacaoListaEventos(*grupo);
}

//-----------------
// Escolher Pericia
//-----------------
void InterfaceGrafica::TrataEscolherPericia(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  std::vector<std::tuple<ent::TipoAtributo, std::string, std::string>> atributos = {
          { ent::TA_FORCA, " atributo: força", "forca" },
          { ent::TA_DESTREZA, " atributo: destreza", "destreza" },
          { ent::TA_CONSTITUICAO, " atributo: constituição", "constituicao" },
          { ent::TA_INTELIGENCIA, " atributo: inteligência", "inteligencia" },
          { ent::TA_SABEDORIA, " atributo: sabedoria", "sabedoria" },
          { ent::TA_CARISMA, " atributo: carisma", "carisma" } };
  std::map<std::string, std::string> mapa_nomes;
  for (const auto& pericia : tabelas_.todas().tabela_pericias().pericias()) {
    if (notificacao.notificacao().size() == 1) {
      for (const auto& [atributo, nome, id] : atributos) {
        mapa_nomes.insert(std::make_pair(
            absl::StrFormat("%s: %+d", nome.c_str(), ModificadorAtributo(atributo, notificacao.notificacao(0).entidade())),
            id));
      }
      if (PodeUsarPericia(tabelas_, pericia.id(), notificacao.notificacao(0).entidade())) {
        const auto& p = Pericia(pericia.id(), notificacao.notificacao(0).entidade());
        mapa_nomes.insert(std::make_pair(absl::StrFormat("%s: %+d", pericia.nome().c_str(), BonusTotal(p.bonus())), pericia.id()));
      }
    } else {
      for (const auto& [atributo, nome, id] : atributos) {
        VLOG(1) << "compilador feliz: " << (int)atributo;
        mapa_nomes.insert(std::make_pair(nome, id));
      }
      mapa_nomes.insert(std::make_pair(pericia.nome(), pericia.id()));
    }
  }
  std::vector<std::string> nomes_pericias;
  std::vector<std::string> mapa_indice_id;
  for (auto& it : mapa_nomes) {
    nomes_pericias.push_back(it.first);
    mapa_indice_id.push_back(it.second);
  }
  EscolheItemLista(
      "Escolha a pericia", "Usar Perícia", nomes_pericias,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPericia,
          this, notificacao, mapa_indice_id,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPericia(
    const ntf::Notificacao notificacao, std::vector<std::string> mapa_indice_id,
    bool ok, int indice_selecao) {
  if (ok && indice_selecao >= 0 && indice_selecao <= (int)mapa_indice_id.size()) {
    tabuleiro_->EntraModoPericia(mapa_indice_id[indice_selecao], notificacao);
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

std::string StringDuracao(const ent::Tabelas& tabelas, const ent::ArmaProto& feitico_tabelado) {
  if (!feitico_tabelado.has_acao()) {
    return "não implementado";
  }
  const auto& acao = feitico_tabelado.acao();
  const auto& acao_tabelada = tabelas.Acao(acao.id());
  if (acao.tipo() == ent::ACAO_CRIACAO_ENTIDADE || acao_tabelada.tipo() == ent::ACAO_CRIACAO_ENTIDADE) {
    // Busca a entidade criada.
    std::string id_modelo;
    if (acao.has_id_modelo_entidade()) {
      id_modelo = acao.id_modelo_entidade();
    } else if (acao.parametros_lancamento().consequencia() == ent::AcaoProto::CP_ATRIBUI_MODELO_ENTIDADE &&
               !acao.parametros_lancamento().parametros().empty()) {
      id_modelo = acao.parametros_lancamento().parametros(0).id_modelo_entidade();
    }
    const auto& modelo = tabelas.ModeloEntidade(id_modelo);
    return StringDuracao(modelo.parametros().tipo_duracao());
  } else if (acao.efeitos_adicionais().size() == 1) {
    return  acao.efeitos_adicionais(0).has_modificador_rodadas()
        ? StringDuracao(feitico_tabelado.acao().efeitos_adicionais(0).modificador_rodadas())
        : "-";
  } else {
    return "-";
  }
}

struct IndiceQuantidadeNivel {
  int indice = 0;
  int quantidade = 0;
  int nivel = 0;
  std::string link;
  std::string duracao;

  void PreencheIncrementando(
      const ent::Tabelas& tabelas, int indice, const ent::ArmaProto& feitico_tabelado = ent::ArmaProto::default_instance(), const ent::InfoClasse& ic = ent::InfoClasse::default_instance()) {
    this->indice = indice;
    ++quantidade;
    nivel = ic.has_id() ? ent::NivelMagia(feitico_tabelado, ic) :  ent::NivelMaisAltoMagia(feitico_tabelado);
    link = feitico_tabelado.link();
    duracao = StringDuracao(tabelas, feitico_tabelado);
  }
};

std::pair<std::vector<std::string>, std::vector<int>>
PreencheNomesEMapaIndices(const std::map<std::string, IndiceQuantidadeNivel>& mapa_nomes_para_indices) {
  std::vector<std::string> nomes;
  std::vector<int> mapa_indices;
  for (auto it : mapa_nomes_para_indices) {
    const std::string& nome = it.first;
    const std::string& link = it.second.link;
    int quantidade =  it.second.quantidade;
    int nivel = it.second.nivel;
    std::string texto = absl::StrFormat(
        "%s%s, nivel: %d, duração: %s%s",
        nome.c_str(),
        quantidade > 1 ? absl::StrFormat(" (x%d)", quantidade).c_str() : "",
        nivel,
        it.second.duracao.c_str(),
        link.empty() ? "" : absl::StrFormat(", <a href='%s'>link</a>", link.c_str()).c_str());
    nomes.push_back(texto);
    int indice = it.second.indice;
    mapa_indices.push_back(indice);
  }
  return std::make_pair(nomes, mapa_indices);
}

//--------------------
// Escolher Pergaminho
//--------------------
void InterfaceGrafica::TrataEscolherPergaminho(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const bool arcano = notificacao.entidade().tesouro().pergaminhos_divinos().empty();
  const auto& pergaminhos_entidade = arcano
      ? notificacao.entidade().tesouro().pergaminhos_arcanos()
      : notificacao.entidade().tesouro().pergaminhos_divinos();

  // Mapaeia o nome para indice. As repeticoes mapearam sempre para o mesmo, mas isso nao importa.
  std::map<std::string, IndiceQuantidadeNivel> mapa_nomes_para_indices;
  std::vector<std::string> nomes_pergaminhos;
  int i = 0;
  for (const auto& pergaminho : pergaminhos_entidade) {
    const std::string& nome = pergaminho.nome().empty()
      ? (arcano ? tabelas_.PergaminhoArcano(pergaminho.id()).nome() : tabelas_.PergaminhoDivino(pergaminho.id()).nome())
      : pergaminho.nome();
    // Classe para conjurar pergaminho.
    const auto& ic = ent::ClasseParaLancarPergaminho(
        tabelas_, arcano ? ent::TM_ARCANA: ent::TM_DIVINA, pergaminho.id(), notificacao.entidade());
    if (ic.id().empty()) {
      LOG(WARNING) << "Nao achei classe de conjuracao para pergaminho: " << pergaminho.id();
    }
    mapa_nomes_para_indices[nome].PreencheIncrementando(tabelas_, i++, tabelas_.Feitico(pergaminho.id()), ic);
  }

  std::vector<int> mapa_indices;
  std::vector<std::string> nomes;
  std::tie(nomes, mapa_indices) = PreencheNomesEMapaIndices(mapa_nomes_para_indices);
  EscolheItemLista(
      "Escolha o pergaminho", "Usar Pergaminho", nomes,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPergaminho,
          this, notificacao, mapa_indices,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPergaminho(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_pergaminho) {
  ent::RodaNoRetorno r([this] () {
    tabuleiro_->ReativaWatchdogSeMestre();
  });
  if (indice_pergaminho < 0 || indice_pergaminho >= (int)mapa_indices.size()) {
    if (ok) {
      LOG(ERROR) << "indice de pergaminho invalido: " << indice_pergaminho << ", tam mapa: " << mapa_indices.size();
    }
    return;
  }

  const bool arcano = notificacao.entidade().tesouro().pergaminhos_divinos().empty();
  const auto& pergaminhos_entidade = arcano
      ? notificacao.entidade().tesouro().pergaminhos_arcanos()
      : notificacao.entidade().tesouro().pergaminhos_divinos();

  int indice_real = mapa_indices[indice_pergaminho];
  if (indice_real < 0 || indice_real >= (int)pergaminhos_entidade.size()) {
    if (ok) {
      LOG(ERROR) << "indice real de pergaminho invalido: " << indice_real << ", tam pergaminhos: " << pergaminhos_entidade.size();
    }
    return;
  }
  if (!ok) {
    return;
  }
  tabuleiro_->UsaPergaminhoNotificando(notificacao.entidade().id(), arcano ? ent::TM_ARCANA : ent::TM_DIVINA, indice_real);
}

//----------------
// Escolher Pocao.
//----------------
void InterfaceGrafica::TrataEscolherPocaoOuAntidoto(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  std::map<std::string, IndiceQuantidadeNivel> mapa_nomes_para_indices;
  int i = 0;
  for (const auto& pocao : pocoes_entidade) {
    const auto& pocao_tabelada = tabelas_.Pocao(pocao.id());
    const std::string& nome = pocao.nome().empty() ? pocao_tabelada.nome() : pocao.nome();
    mapa_nomes_para_indices[nome].PreencheIncrementando(
        tabelas_, i++, tabelas_.Feitico(pocao_tabelada.has_id_feitico() ? pocao_tabelada.id_feitico() : pocao_tabelada.id()));
  }
  for (const auto& item : notificacao.entidade().tesouro().itens_mundanos()) {
    // por enquanto ta hardcoded para antidoto apenas com poções, mas outros itens mundanos eventualmente podem entrar aqui.
    if (item.id() != "antidoto") {
      ++i;
      continue;
    }
    const auto& item_tabelado = tabelas_.ItemMundano(item.id());
    const std::string& nome = item.nome().empty() ? item_tabelado.nome() : item.nome();
    auto& iqn = mapa_nomes_para_indices[nome];
    iqn.PreencheIncrementando(tabelas_, i++);
    iqn.duracao = absl::StrCat(item_tabelado.duracao_rodadas());
  }
  std::vector<std::string> nomes;
  std::vector<int> mapa_indices;
  std::tie(nomes, mapa_indices) = PreencheNomesEMapaIndices(mapa_nomes_para_indices);
  EscolheItemLista(
      "Escolha a poção", "Beber", nomes,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPocaoOuAntidoto,
          this, notificacao, mapa_indices,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPocaoOuAntidoto(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_selecionado) {
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  const auto& items_entidade = notificacao.entidade().tesouro().itens_mundanos();
  int indice_pocao_ou_antidoto = mapa_indices[indice_selecionado];
  if (!ok || indice_pocao_ou_antidoto >= pocoes_entidade.size() + items_entidade.size()) {
    LOG(ERROR) << "poção ou antidoto invalido, indice: " << indice_pocao_ou_antidoto << ", pocoes: " << pocoes_entidade.size() << ", itens: " << items_entidade.size();
    VoltaEscolherEfeito(notificacao, 0, false, 0);
    return;
  }
  if (indice_pocao_ou_antidoto < pocoes_entidade.size()) {
    // Mapaeia o nome para indice. As repeticoes mapearam sempre para o mesmo, mas isso nao importa.
    const auto& pocao = tabelas_.Pocao(pocoes_entidade.Get(indice_pocao_ou_antidoto).id());
    if (pocao.tipo_efeito_size() == 1 || pocao.combinacao_efeitos() != ent::COMB_EXCLUSIVO) {
      VoltaEscolherEfeito(notificacao, indice_pocao_ou_antidoto, true, 0);
      return;
    }
    std::vector<std::string> efeitos;
    for (auto tipo_efeito : pocao.tipo_efeito()) {
      efeitos.push_back(ent::TipoEfeito_Name((ent::TipoEfeito)tipo_efeito));
    }
    EscolheItemLista(
        "Escolha o efeito", std::nullopt, efeitos,
        std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherEfeito,
          this, notificacao, indice_pocao_ou_antidoto,
          _1, _2, /*eh_antidoto=*/false));
  } else {
    // antidoto.
    indice_pocao_ou_antidoto -= pocoes_entidade.size();
    const auto& antidoto = tabelas_.ItemMundano(items_entidade.Get(indice_pocao_ou_antidoto).id());
    if (antidoto.id() != "antidoto") {
      LOG(ERROR) << "antidoto invalido: " << antidoto.ShortDebugString();
      VoltaEscolherEfeito(notificacao, 0, false, 0, /*eh_antidoto=*/true);
      return;
    }
    VoltaEscolherEfeito(notificacao, indice_pocao_ou_antidoto, /*ok=*/true, /*indice_efeito=*/0, /*eh_antidoto=*/true);
    return;
  }
}

void InterfaceGrafica::VoltaEscolherEfeito(const ntf::Notificacao notificacao, unsigned int indice_pocao, bool ok, unsigned int indice_efeito, bool eh_antidoto) {
  if (ok) {
    if (eh_antidoto) {
      tabuleiro_->BebeAntidotoNotificando(notificacao.entidade().id(), indice_pocao);
    } else {
     tabuleiro_->BebePocaoNotificando(notificacao.entidade().id(), indice_pocao, indice_efeito);
    }
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----
// Cor
//----
void InterfaceGrafica::TrataEscolheCor(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const ent::Cor* c = nullptr;
  TipoCor tc = TipoCor::COR_GENERICA;
  if (notificacao.has_cor_generica()) {
    c = &notificacao.cor_generica();
  } else if (notificacao.tabuleiro().has_luz_ambiente()) {
    c = &notificacao.tabuleiro().luz_ambiente();
    tc = TipoCor::COR_LUZ_AMBIENTE;
  } else if (notificacao.tabuleiro().has_luz_direcional()) {
    c = &notificacao.tabuleiro().luz_direcional().cor();
    tc = TipoCor::COR_LUZ_DIRECIONAL;
  }
  EscolheCor(tc, notificacao.tabuleiro().has_id_cenario() ? std::make_optional<int>(notificacao.tabuleiro().id_cenario()) : std::nullopt,
             c->r(), c->g(), c->b(), c->a(), std::bind(&InterfaceGrafica::VoltaEscolheCor, this, _1, _2, _3, _4, _5, _6, _7));
}

void InterfaceGrafica::VoltaEscolheCor(bool ok, TipoCor tc, std::optional<int> id_cenario, float r, float g, float b, float a) {
  if (ok) {
    if (tc == TipoCor::COR_GENERICA) {
      tabuleiro_->SelecionaCorPersonalizada(r, g, b, a);
    } else if (id_cenario.has_value()) {
      if (tc == TipoCor::COR_LUZ_AMBIENTE) {
        tabuleiro_->SelecionaLuzAmbiente(*id_cenario, r, g, b, a);
      } else if (tc == TipoCor::COR_LUZ_DIRECIONAL) {
        tabuleiro_->SelecionaLuzDirecional(*id_cenario, r, g, b, a);
      }
    }
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----------------
// Mostra Mensagem
//----------------
void InterfaceGrafica::TrataMostraMensagem(bool erro, const std::string& mensagem) {
  tabuleiro_->DesativaWatchdogSeMestre();
  MostraMensagem(erro, mensagem, [this] () { tabuleiro_->ReativaWatchdogSeMestre(); });
}

//----------------
// Abrir Tabuleiro
//----------------
void InterfaceGrafica::TrataAbrirTabuleiro(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  std::vector<std::string> tab_estaticos;
  std::vector<std::string> tab_dinamicos;
  bool tabuleiro = !notificacao.entidade().has_modelo_3d();
  try {
    tab_estaticos = arq::ConteudoDiretorio(tabuleiro ? arq::TIPO_TABULEIRO_ESTATICO : arq::TIPO_MODELOS_3D);
  }
  catch (...) {
  }
  try {
    tab_dinamicos = arq::ConteudoDiretorio(tabuleiro ? arq::TIPO_TABULEIRO : arq::TIPO_MODELOS_3D_BAIXADOS);
  }
  catch (...) {
  }

  if (tab_estaticos.size() + tab_dinamicos.size() == 0) {
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErroTipada(ntf::TN_ERRO, "Nao existem tabuleiros salvos"));
    tabuleiro_->ReativaWatchdogSeMestre();
    return;
  }
  std::sort(tab_estaticos.begin(), tab_estaticos.end());
  std::sort(tab_dinamicos.begin(), tab_dinamicos.end());
  EscolheArquivoAbrirTabuleiro(
      tab_estaticos, tab_dinamicos,
      std::bind(
          &ifg::InterfaceGrafica::VoltaAbrirTabuleiro,
          this,
          notificacao.tabuleiro().manter_entidades(),
          notificacao.entidade().has_modelo_3d(),
          _1, _2));
}

void InterfaceGrafica::VoltaAbrirTabuleiro(
    bool manter_entidades, bool modelo_3d, const std::string& nome, arq::tipo_e tipo_retornado) {
  if (!nome.empty()) {
    auto notificacao = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_TABULEIRO);
    if (modelo_3d) {
      notificacao->mutable_entidade()->mutable_modelo_3d();
    }
    notificacao->set_endereco(
        std::string(tipo_retornado == arq::TIPO_TABULEIRO_ESTATICO ? "estatico://" : "dinamico://") + nome);
    notificacao->mutable_tabuleiro()->set_manter_entidades(manter_entidades);
    central_->AdicionaNotificacao(notificacao.release());
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//-----------------
// Salvar Tabuleiro
//-----------------
void InterfaceGrafica::TrataSalvarTabuleiro(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheArquivoSalvarTabuleiro(
      std::bind(
          &ifg::InterfaceGrafica::VoltaSalvarTabuleiro,
          this, notificacao.entidade().has_modelo_3d(), !notificacao.tabuleiro().versoes().empty(), _1));
}

void InterfaceGrafica::VoltaSalvarTabuleiro(
    bool modelo_3d, bool versionar, const std::string& nome) {
  auto n = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
  n->set_endereco(nome);
  if (modelo_3d) {
    n->mutable_entidade()->mutable_modelo_3d();
  }
  if (versionar) {
    n->mutable_tabuleiro()->mutable_versoes()->Add();
  }
  central_->AdicionaNotificacao(n.release());
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----------------
// Abrir Imagem
//----------------
void InterfaceGrafica::TrataAbrirImagem(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  std::vector<std::string> texs;
  try {
    texs = arq::ConteudoDiretorio(arq::TIPO_TEXTURA_LOCAL);
  }
  catch (...) {
  }

  if (texs.empty()) {
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErroTipada(ntf::TN_ERRO, "Nao existem texturas salvas"));
    tabuleiro_->ReativaWatchdogSeMestre();
    return;
  }
  std::sort(texs.begin(), texs.end());
  EscolheArquivoAbrirImagem(
    texs,
    std::bind(
      &ifg::InterfaceGrafica::VoltaAbrirImagem,
      this, _1));
}

void InterfaceGrafica::VoltaAbrirImagem(const std::string& nome) {
  if (!nome.empty()) {
    auto notificacao = ntf::NovaNotificacao(ntf::TN_MOSTRAR_IMAGEM_CLIENTES);
    auto* textura = notificacao->add_info_textura();
    try {
      unsigned int largura, altura;
      tex::Texturas::LeDecodificaImagemTipo(arq::TIPO_TEXTURA_LOCAL, nome, textura, &largura, &altura);
    } catch (...) {
      return;
    }
    textura->set_id(nome);
    central_->AdicionaNotificacao(notificacao.release());
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//--------------
// EscolheModelo
//--------------
void InterfaceGrafica::TrataEscolherModeloEntidade(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheModeloEntidade(
      tabelas_.MenuModelos(),
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherModeloEntidade,
          this, _1));
}

void InterfaceGrafica::VoltaEscolherModeloEntidade(
    const std::string& nome) {
  tabuleiro_->SelecionaModelosEntidades(nome);
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----------------
// Escolher versao
//----------------
void InterfaceGrafica::TrataEscolherVersao() {
  if (tabuleiro_->Proto().versoes().empty()) {
    return;
  }
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheVersaoTabuleiro("Escolha versão a ser restaurada", [this](int versao) {
    if (versao >= 0 && versao < tabuleiro_->Proto().versoes().size()) {
      auto notificacao = ntf::NovaNotificacao(ntf::TN_DESERIALIZAR_VERSAO_TABULEIRO_NOTIFICANDO);
      notificacao->mutable_tabuleiro()->ParseFromString(tabuleiro_->Proto().versoes(versao).dados());
      *notificacao->mutable_tabuleiro()->mutable_versoes() = tabuleiro_->Proto().versoes();
      central_->AdicionaNotificacao(notificacao.release());
    }
    tabuleiro_->ReativaWatchdogSeMestre();
  });
}

void InterfaceGrafica::TrataEscolherVersaoParaRemocao() {
  if (tabuleiro_->Proto().versoes().empty()) {
    return;
  }
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheVersoesTabuleiro("Escolha versões a serem removida", [this](const std::vector<int>& versoes) {
    tabuleiro_->RemoveVersoes(versoes);
    tabuleiro_->ReativaWatchdogSeMestre();
  });
}

//-----------------
// Escolher feitico
//-----------------
std::string StringAlcance(const ent::Tabelas& tabelas, const ent::ArmaProto& feitico) {
  if (feitico.alcance_quadrados() > 0) {
    return absl::StrFormat("alcance %d q%s", feitico.alcance_quadrados(), feitico.ataque_toque() ? " [toque]" : "");
  } else if (FeiticoPessoalDispersao(tabelas, feitico)) {
    return "feitiço pessoal dispersão";
  } else if (FeiticoPessoal(tabelas, feitico)) {
    return "feitiço pessoal";
  } else if (!feitico.has_acao()) {
    return "";
  } else {
    return "toque";
  }
}

std::string NomeFeitico(const ent::Tabelas& tabelas, const ent::EntidadeProto::InfoConhecido& c) {
  if (!c.id().empty()) {
    const auto& feitico = tabelas.Feitico(c.id());
    if (!feitico.nome().empty()) {
      std::string str_area;
      std::string str_duracao;
      if (feitico.has_acao()) {
        const auto& acao = feitico.acao();
        str_area = acao.efeito_area() ? absl::StrFormat(", %s", StringArea(acao).c_str()) : "";
        if (acao.efeitos_adicionais().size() == 1) {
          const auto& ed = acao.efeitos_adicionais(0);
          str_duracao = ed.has_modificador_rodadas()
              ? absl::StrFormat(", %s", StringDuracao(ed.modificador_rodadas()).c_str())
              : "";
        }
      }
      return absl::StrFormat("%s, %s%s%s",
          feitico.nome().c_str(),
          StringAlcance(tabelas, feitico).c_str(),
          str_area.c_str(),
          str_duracao.c_str()
      );
    }
  }
  if (!c.nome().empty()) return c.nome();
  return c.id();
}

void InterfaceGrafica::TrataEscolherAliados(const ntf::Notificacao& notificacao) {
  if (notificacao.acao().ids_afetados().empty()) {
    LOG(INFO) << "acao com ids afetados vazio";
    return;
  }

  std::unordered_map<int, unsigned int> mapa_indice_id;
  std::vector<std::string> rotulos_entidades;
  int indice = 0;
  for (const auto id : notificacao.acao().ids_afetados()) {
    const auto* entidade = tabuleiro_->BuscaEntidade(id);
    if (entidade == nullptr) continue;
    rotulos_entidades.emplace_back(RotuloEntidade(entidade));
    mapa_indice_id[indice++] = entidade->Id();
  }
  if (rotulos_entidades.empty()) {
    LOG(INFO) << "rotulos de entidade vazio";
    return;
  }
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheItemsLista(notificacao.acao().aliados_e_inimigos_de_forma_diferente() ? "Escolha os aliados" : "Escolha entidades afetadas", rotulos_entidades,
      [this, notificacao, mapa_indice_id] (bool ok_decisao, const std::vector<int>& indices) {
    ent::RodaNoRetorno r([this]() {
      this->tabuleiro_->ReativaWatchdogSeMestre();
    });
    ent::AcaoProto acao = notificacao.acao();
    acao.clear_ids_afetados();
    for (auto& it : mapa_indice_id) {
      // Wndows reclama que indice nao eh capturado aqui usando structured binding initialisation.
      const auto indice = it.first;
      const auto id = it.second;
      if (ent::c_any(indices, indice)) {
        acao.add_ids_afetados(id);
      } else {
        acao.add_ids_afetados_inimigos(id);
      }
    }
    if (acao.ids_afetados().empty() && acao.ids_afetados_inimigos().empty()) {
      LOG(INFO) << "ids afetados vazio";
      return;
    }
    tabuleiro_->TrataAcaoUmaEntidade(
        acao.has_id_entidade_origem() ? tabuleiro_->BuscaEntidade(acao.id_entidade_origem()) : nullptr,
        acao.pos_entidade(), acao.pos_tabuleiro(),
        acao.has_id_entidade_destino() ? acao.id_entidade_destino() : ent::Entidade::IdInvalido,
        /*atraso_s=*/0.0f, &acao);
  });
}


void InterfaceGrafica::TrataEscolherDecisaoLancamento(const ntf::Notificacao& notificacao) {
  std::vector<std::string> lista_parametros;
  std::vector<std::string> ids;
  if (notificacao.acao().parametros_lancamento().consequencia() == ent::AcaoProto::CP_SELECIONA_FEITICO) {
    if (notificacao.acao().parametros_lancamento().parametros().empty()) {
      LOG(WARNING) << "feitico com CP_SELECIONA_FEITICO deve ter 1 parametro.";
      return;
    }
    const auto& parametro = notificacao.acao().parametros_lancamento().parametros(0);
    for (const auto& feitico : tabelas_.todas().tabela_feiticos().armas()) {
      if (!parametro.id_classe().empty() &&
          c_none_of(feitico.info_classes(),
            [&parametro](const ent::ArmaProto::InfoClasseParaFeitico& ic) { return ic.id() == parametro.id_classe(); })) {
        continue;
      }
      if (parametro.has_nivel_maximo() &&
        c_none_of(feitico.info_classes(),
          [&parametro](const ent::ArmaProto::InfoClasseParaFeitico& ic) { return ic.nivel() <= parametro.nivel_maximo(); })) {
        continue;
      }
      ids.push_back(feitico.id());
      lista_parametros.push_back(feitico.nome());
    }
  } else if (!notificacao.acao().parametros_lancamento().parametros().empty()) {
    for (const auto& parametro : notificacao.acao().parametros_lancamento().parametros()) {
      lista_parametros.push_back(parametro.texto());
    }
  }

  if (lista_parametros.empty()) {
    return;
  }
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheItemLista("Parâmetros de Lancamento", std::nullopt, lista_parametros, [this, notificacao, lista_parametros, ids] (bool ok_decisao, int indice_decisao) {
    ent::RodaNoRetorno r([this]() {
      this->tabuleiro_->ReativaWatchdogSeMestre();
    });
    ent::AcaoProto acao = notificacao.acao();
    auto pl = acao.parametros_lancamento();
    if (!ok_decisao || (indice_decisao < 0) || (indice_decisao >= (int)lista_parametros.size()) || (indice_decisao >= pl.parametros().size())) {
      return;
    }
    acao.clear_parametros_lancamento();
    switch (pl.consequencia()) {
      case ent::AcaoProto::CP_SELECIONA_FEITICO:
        for (auto& ed : *acao.mutable_efeitos_adicionais()) {
          ed.add_complementos_str(ids[indice_decisao]);
        }
        break;
      case ent::AcaoProto::CP_ATRIBUI_EFEITO:
        for (auto& ed : *acao.mutable_efeitos_adicionais()) {
          if (ed.has_efeito()) continue;
          ed.set_efeito(pl.parametros(indice_decisao).efeito());
        }
        break;
      case ent::AcaoProto::CP_ATRIBUI_MODELO_ENTIDADE: {
        const auto& parametros = pl.parametros(indice_decisao);
        acao.set_id_modelo_entidade(parametros.id_modelo_entidade());
        if (!parametros.quantidade().empty()) {
          acao.set_quantidade_entidades(parametros.quantidade());
        }
        break;
      }
      default:
        return;
    }
    tabuleiro_->TrataAcaoUmaEntidade(
        acao.has_id_entidade_origem() ? tabuleiro_->BuscaEntidade(acao.id_entidade_origem()) : nullptr,
        acao.pos_entidade(), acao.pos_tabuleiro(),
        acao.has_id_entidade_destino() ? acao.id_entidade_destino() : ent::Entidade::IdInvalido,
        /*atraso_s=*/0.0f, &acao);
  });
}

void InterfaceGrafica::TrataEscolherFeitico(const ntf::Notificacao& notificacao) {
  if (notificacao.entidade().feiticos_classes().empty() ||
      notificacao.entidade().feiticos_classes(0).id_classe().empty() ||
      notificacao.entidade().feiticos_classes(0).mapa_feiticos_por_nivel().empty()) {
    LOG(ERROR) << "Notificacao de escolher feitico invalida: " << notificacao.DebugString();
    return;
  }
  const auto& fc = notificacao.entidade().feiticos_classes(0);
  const auto& id_classe = fc.id_classe();
  std::vector<std::string> lista;
  // Cada item, contendo o nivel do feitico e o indice do feitico e o indice gasto.
  struct NivelIndiceIndiceGasto {
    NivelIndiceIndiceGasto(const std::string& id, int nivel, int indice, int indice_gasto)
      : id(id), nivel(nivel), indice(indice), indice_gasto(indice_gasto) {}
    std::string id;
    int nivel = 0;
    int indice = 0;
    int indice_gasto = 0;
  };
  std::vector<NivelIndiceIndiceGasto> items;
  int nivel_gasto = -1;
  for (const auto& [nivel, fn] : fc.mapa_feiticos_por_nivel()) {
    nivel_gasto = std::max(nivel, nivel_gasto);
  }
  const auto& fn = ent::FeiticosNivel(id_classe, nivel_gasto, notificacao.entidade());
  if (ClassePrecisaMemorizar(tabelas_, id_classe)) {
    std::string str_restricao = id_classe == "clerigo" ? " (dominio) " : " (especista) ";
    // Monta lista de feiticos para lancar do nivel.
    for (int indice = 0; indice < fn.para_lancar().size(); ++indice) {
      const auto& pl = fn.para_lancar(indice);
      if (pl.usado()) continue;
      const auto& c = ent::FeiticoConhecido(
          id_classe, pl.nivel_conhecido(), pl.indice_conhecido(), notificacao.entidade());
      const auto& feitico = tabelas_.Feitico(c.id());
      std::string link = feitico.link().empty() ? "" : absl::StrFormat("<a href='%s'>link</a>", feitico.link().c_str());
      lista.push_back(
          absl::StrFormat("nivel %d[%d]: %s%s, duração: %s, %s",
            nivel_gasto, indice,
            NomeFeitico(tabelas_, c).c_str(), pl.restrito() ? "" : "",
            StringDuracao(tabelas_, feitico).c_str(),
            link.c_str()));
      items.emplace_back(c.id(), nivel_gasto, indice, indice);
    }
    if (lista.empty()) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(absl::StrFormat("Nao ha magia de nivel %d para gastar", nivel_gasto)));
      return;
    }
  } else {
    // Monta lista de feiticos conhecidos ate o nivel.
    const auto* entidade = tabuleiro_->BuscaEntidade(notificacao.entidade().id());
    if (entidade == nullptr) {
      LOG(ERROR) << "entidade invalida";
      return;
    }
    int indice_gasto = IndiceFeiticoDisponivel(id_classe, nivel_gasto, entidade->Proto());
    if (indice_gasto == -1) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(absl::StrFormat("Nao ha magia de nivel %d para gastar", nivel_gasto)));
      return;
    }
    for (int nivel = nivel_gasto; nivel >= 0; --nivel) {
      const auto& fn = FeiticosNivel(id_classe, nivel, notificacao.entidade());
      for (int indice = 0; indice < fn.conhecidos().size(); ++indice) {
        const auto& c = fn.conhecidos(indice);
        const auto& feitico = tabelas_.Feitico(c.id());
        std::string link = feitico.link().empty() ? "" : absl::StrFormat("<a href='%s'>link</a>", feitico.link().c_str());
        lista.push_back(absl::StrFormat("nivel %d[%d]: %s %s", nivel, indice, NomeFeitico(tabelas_, c).c_str(), link.c_str()));
        // Gasta do nivel certo.
        items.emplace_back(c.id(), nivel, indice, indice_gasto);
      }
    }
  }
  bool conversao_espontanea = fc.conversao_espontanea();

  // Esse lambda rodada no final da UI.
  auto funcao_final = [this, notificacao, conversao_espontanea, id_classe, nivel_gasto, items] (int ok, int indice_lista) {
    ent::RodaNoRetorno r([this]() {
      this->tabuleiro_->ReativaWatchdogSeMestre();
    });
    if (!ok) {
      LOG(INFO) << "Nao usando feitico";
      return;
    }
    // Consome o feitico.
    const auto& item = items[indice_lista];

    auto grupo_notificacao = ent::NovoGrupoNotificacoes();
    const auto* entidade = tabuleiro_->BuscaEntidade(notificacao.entidade().id());
    if (entidade == nullptr) {
      LOG(INFO) << "Erro, entidade nao existe";
      return;
    }
    if (ent::NotificacaoConsequenciaFeitico(
          tabelas_, DadosIniciativaEntidade(entidade), id_classe, conversao_espontanea, item.nivel, item.indice, *entidade, grupo_notificacao.get())) {
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_ACAO);
    }

    auto n_alteracao_feitico = ent::NotificacaoAlterarFeitico(
        id_classe, nivel_gasto, item.indice_gasto, /*usado=*/true, notificacao.entidade());
    *grupo_notificacao->add_notificacao() = *n_alteracao_feitico;

    tabuleiro_->AdicionaNotificacaoListaEventos(*grupo_notificacao);
    tabuleiro_->TrataNotificacao(*grupo_notificacao);
    VLOG(1) << "gastando feitico nivel: " << nivel_gasto << ", indice: " << item.indice_gasto;
  };

  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheItemLista(conversao_espontanea ? "Converter qual feitiço" : "Escolha o Feitiço", std::nullopt, lista, funcao_final);
}

void InterfaceGrafica::EscolheVersaoTabuleiro(const std::string& titulo, std::function<void(int)> funcao_volta) {
  std::vector<std::string> items;
  for (int i = 0; i < tabuleiro_->Proto().versoes().size(); ++i) {
    const std::string& descricao = tabuleiro_->Proto().versoes(i).descricao();
    if (descricao.empty()) {
      items.push_back(absl::StrFormat("versão %d", i + 1));
    } else {
      items.push_back(descricao);
    }
  }
  EscolheItemLista(titulo, std::nullopt, items, [this, funcao_volta](bool aceito, int indice) {
    if (aceito && indice >= 0 && indice < tabuleiro_->Proto().versoes().size()) {
      funcao_volta(indice);
    } else {
      funcao_volta(-1);
    }
  });
}

void InterfaceGrafica::EscolheVersoesTabuleiro(const std::string& titulo, std::function<void(const std::vector<int>&)> funcao_volta) {
  std::vector<std::string> items;
  for (int i = 0; i < tabuleiro_->Proto().versoes().size(); ++i) {
    const std::string& descricao = tabuleiro_->Proto().versoes(i).descricao();
    if (descricao.empty()) {
      items.push_back(absl::StrFormat("versão %d", i + 1));
    } else {
      items.push_back(descricao);
    }
  }
  EscolheItemsLista(titulo, items, [this, funcao_volta](bool aceito, const std::vector<int>& indices) {
    if (aceito && !indices.empty()) {
      funcao_volta(indices);
    } else {
      funcao_volta({});
    }
  });
}

//------------
// Forcar Dado
//------------
void InterfaceGrafica::TrataForcarDado(const ntf::Notificacao& notificacao) {
  if (!notificacao.has_id_generico()) {
    return;
  }
  int nfaces = notificacao.id_generico();
  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheValorDadoForcado(absl::StrFormat("Escolha o valor do d%d", nfaces), nfaces, [this, nfaces](int valor_forcado) {
    if (valor_forcado < 1 || valor_forcado > nfaces) {
      LOG(ERROR) << "Valor invalido " << valor_forcado << " para dado de " << nfaces;
      return;
    }
    std::optional<ent::Face> face = ent::NumParaFace(nfaces, /*logar_erro=*/true);
    if (!face.has_value()) {
      LOG(ERROR) << "Numero de faces invalido: " << nfaces;
      return;
    }
    ent::AcumulaDado(*face, valor_forcado);
  });
}

}  // namespace ifg
