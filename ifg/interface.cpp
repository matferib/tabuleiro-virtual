#include <algorithm>
#include <functional>

#include "arq/arquivo.h"
#include "ent/acoes.pb.h"
#include "ent/tabelas.h"
#include "ent/tabuleiro.h"
#include "goog/stringprintf.h"
#include "ifg/interface.h"
#include "ifg/modelos.pb.h"
#include "ifg/tecladomouse.h"
#include "log/log.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;
using google::protobuf::StringPrintf;

namespace ifg {

namespace {

std::string StringDuracao(ent::ModificadorRodadas mr) {
  switch (mr) {
    case ent::MR_NENHUM: return "nenhum";
    case ent::MR_RODADAS_NIVEL: return "rodadas/nível";
    case ent::MR_MINUTOS_NIVEL: return "minutos/nível";
    case ent::MR_10_MINUTOS_NIVEL: return "10 minutos/nível";
    case ent::MR_HORAS_NIVEL: return "horas/nível";
    case ent::MR_2_HORAS_NIVEL: return "2 horas/nível";
    case ent::MR_1_RODADA_A_CADA_3_NIVEIS_MAX_6: return "1 rodada/ 3 níveis (max 6)";
    case ent::MR_10_RODADAS_MAIS_UMA_POR_NIVEL_MAX_15: return "10 rodadas +1/nível (max 15)";
    case ent::MR_HORAS_NIVEL_MAX_15: return "horas/nivel (max 15)";
    case ent::MR_2_MINUTOS_NIVEL: return "2 minutos/nivel";
    case ent::MR_DIAS_POR_NIVEL: return "dias/nivel";
    case ent::MR_PALAVRA_PODER_ATORDOAR: return "variável por PV: max 4d4 rodadas";
    case ent::MR_PALAVRA_PODER_CEGAR: return "variável por PV: max permanente";
    case ent::MR_CONTINUO: return "continuo";
  }
  // Nao deveria chegar aqui.
  return "desconhecida";
}

std::string StringArea(const ent::AcaoProto& acao) {
  std::string str_geo;
  switch (acao.geometria()) {
    case ent::ACAO_GEO_CONE:
      str_geo = StringPrintf("cone %f (q)", acao.distancia_quadrados());
      break;
    case ent::ACAO_GEO_CILINDRO:
      str_geo = StringPrintf("cilindro raio %.1f (q)", acao.raio_quadrados());
      break;
    case ent::ACAO_GEO_ESFERA:
      str_geo = StringPrintf("esfera raio %.1f (q)", acao.raio_quadrados());
      break;
    case ent::ACAO_GEO_CUBO:
      // Isso é mais para modelar objetos geometricos, como flecha acida.
      break;
  }
  return str_geo;
}

}  // namespace

void MisturaProtosMenu(const MenuModelos& entrada, MenuModelos* saida) {
  for (const auto& m : entrada.modelo()) {
    saida->add_modelo()->CopyFrom(m);
  }
  for (const auto& sub_entrada : entrada.sub_menu()) {
    MenuModelos* sub_saida = nullptr;
    for (auto& esta_sub_saida : *saida->mutable_sub_menu()) {
      if (esta_sub_saida.id() == sub_entrada.id()) {
        sub_saida = &esta_sub_saida;
        break;
      }
    }
    if (sub_saida == nullptr) {
      sub_saida = saida->add_sub_menu();
      sub_saida->set_id(sub_entrada.id());
    }
    MisturaProtosMenu(sub_entrada, sub_saida);
  }
}

bool InterfaceGrafica::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
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
    case ntf::TN_ABRIR_DIALOGO_SALVAR_TABULEIRO: {
      TrataSalvarTabuleiro(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_MODELO_ENTIDADE: {
      TrataEscolherModeloEntidade(notificacao);
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_POCAO: {
      TrataEscolherPocao(notificacao);
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
    default:
      break;
  }
  return false;
}

//-----------------
// Escolher Pericia
//-----------------
void InterfaceGrafica::TrataEscolherPericia(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const auto& pericias_entidade = notificacao.entidade().info_pericias();
  std::map<std::string, int> mapa_nomes;
  int indice = 0;
  for (const auto& pericia : pericias_entidade) {
    std::string nome = tabelas_.Pericia(pericia.id()).nome();
    mapa_nomes.insert(std::make_pair(nome, indice++));
  }
  std::map<int, int> mapa_pericias;
  std::vector<std::string> nomes_pericias;
  indice = 0;
  for (const auto& it : mapa_nomes) {
    nomes_pericias.push_back(it.first);
    mapa_pericias.insert(std::make_pair(indice++, it.second));
  }
  EscolheItemLista(
      "Escolha a pericia", nomes_pericias,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPericia,
          this, notificacao, mapa_pericias,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPericia(ntf::Notificacao notificacao, std::map<int, int> mapa_pericias, bool ok, int indice_selecao) {
  if (ok && indice_selecao >= 0 && indice_selecao <= notificacao.entidade().info_pericias_size()) {
    int indice_pericia = mapa_pericias[indice_selecao];
    tabuleiro_->TrataRolarPericiaNotificando(indice_pericia, notificacao.entidade());
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

struct IndiceQuantidadeNivel {
  int indice = 0;
  int quantidade = 0;
  int nivel = 0;
  std::string link;
  std::string duracao;

  void PreencheIncrementando(int indice, const ent::ArmaProto& feitico, const ent::InfoClasse& ic = ent::InfoClasse::default_instance()) {
    this->indice = indice;
    ++quantidade;
    nivel = ic.has_id() ? ent::NivelMagia(feitico, ic) :  ent::NivelMaisAltoMagia(feitico);
    link = feitico.link();
    if (feitico.acao().efeitos_adicionais().size() == 1) {
      duracao = feitico.acao().efeitos_adicionais(0).has_modificador_rodadas()
          ? StringDuracao(feitico.acao().efeitos_adicionais(0).modificador_rodadas())
          : "-";
    } else {
      duracao = "-";
    }
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
    std::string texto = StringPrintf(
        "%s%s, nivel: %d, duração: %s %s",
        nome.c_str(),
        quantidade > 1 ? StringPrintf(" (x%d)", quantidade).c_str() : "",
        nivel,
        it.second.duracao.c_str(),
        link.empty() ? "" : StringPrintf("<a href='%s'>link</a>", link.c_str()).c_str());
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
    mapa_nomes_para_indices[nome].PreencheIncrementando(i++, tabelas_.Feitico(pergaminho.id()), ic);
  }

  std::vector<int> mapa_indices;
  std::vector<std::string> nomes;
  std::tie(nomes, mapa_indices) = PreencheNomesEMapaIndices(mapa_nomes_para_indices);
  EscolheItemLista(
      "Escolha o pergaminho", nomes,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPergaminho,
          this, notificacao, mapa_indices,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPergaminho(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_pergaminho) {
  ent::RodaNoRetorno([this] () {
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
void InterfaceGrafica::TrataEscolherPocao(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  std::map<std::string, IndiceQuantidadeNivel> mapa_nomes_para_indices;
  int i = 0;
  for (const auto& pocao : pocoes_entidade) {
    const auto& pocao_tabelada = tabelas_.Pocao(pocao.id());
    const std::string& nome = pocao.nome().empty() ? pocao_tabelada.nome() : pocao.nome();
    mapa_nomes_para_indices[nome].PreencheIncrementando(
        i++, tabelas_.Feitico(pocao_tabelada.has_id_feitico() ? pocao_tabelada.id_feitico() : pocao_tabelada.id()));
  }

  std::vector<std::string> nomes;
  std::vector<int> mapa_indices;
  std::tie(nomes, mapa_indices) = PreencheNomesEMapaIndices(mapa_nomes_para_indices);
  EscolheItemLista(
      "Escolha a poção", nomes,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPocao,
          this, notificacao, mapa_indices,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPocao(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_selecionado) {
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  int indice_pocao = mapa_indices[indice_selecionado];
  if (!ok || indice_pocao >= pocoes_entidade.size()) {
    VoltaEscolherEfeito(notificacao, 0, false, 0);
    return;
  }
  // Mapaeia o nome para indice. As repeticoes mapearam sempre para o mesmo, mas isso nao importa.
  const auto& pocao = tabelas_.Pocao(pocoes_entidade.Get(indice_pocao).id());
  if (pocao.tipo_efeito_size() == 1 || pocao.combinacao_efeitos() != ent::COMB_EXCLUSIVO) {
    VoltaEscolherEfeito(notificacao, indice_pocao, true, 0);
    return;
  }
  std::vector<std::string> efeitos;
  for (auto tipo_efeito : pocao.tipo_efeito()) {
    efeitos.push_back(ent::TipoEfeito_Name((ent::TipoEfeito)tipo_efeito));
  }
  EscolheItemLista(
      "Escolha o efeito", efeitos,
      std::bind(
        &ifg::InterfaceGrafica::VoltaEscolherEfeito,
        this, notificacao, indice_pocao,
        _1, _2));
}

void InterfaceGrafica::VoltaEscolherEfeito(const ntf::Notificacao notificacao, unsigned int indice_pocao, bool ok, unsigned int indice_efeito) {
  if (ok) {
    tabuleiro_->BebePocaoNotificando(notificacao.entidade().id(), indice_pocao, indice_efeito);
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----
// Cor
//----
void InterfaceGrafica::TrataEscolheCor(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const ent::Cor& c = notificacao.tabuleiro().luz_ambiente();
  EscolheCor(c.r(), c.g(), c.b(), c.a(), std::bind(&InterfaceGrafica::VoltaEscolheCor, this, _1, _2, _3, _4, _5));
}

void InterfaceGrafica::VoltaEscolheCor(bool ok, float r, float g, float b, float a) {
  if (ok) {
    tabuleiro_->SelecionaCorPersonalizada(r, g, b, a);
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

//--------------
// EscolheModelo
//--------------
void InterfaceGrafica::TrataEscolherModeloEntidade(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const char* ARQUIVO_MENU_MODELOS = "menumodelos.asciiproto";
  const char* ARQUIVO_MENU_MODELOS_NAO_SRD = "menumodelos_nao_srd.asciiproto";
  const char* ARQUIVO_MENU_MODELOS_FEITICOS = "menumodelosfeiticos.asciiproto";
  const std::string arquivos_menu_modelos[] = {
      ARQUIVO_MENU_MODELOS, ARQUIVO_MENU_MODELOS_NAO_SRD, ARQUIVO_MENU_MODELOS_FEITICOS };
  std::vector<ent::EntidadeProto*> entidades;
  MenuModelos menu_modelos_proto;
  for (const std::string& nome_arquivo_menu_modelo : arquivos_menu_modelos) {
    MenuModelos este_menu_modelos_proto;
    try {
      arq::LeArquivoAsciiProto(arq::TIPO_DADOS, nome_arquivo_menu_modelo, &este_menu_modelos_proto);
      MisturaProtosMenu(este_menu_modelos_proto, &menu_modelos_proto);
    } catch (const std::logic_error& erro) {
      LOG(ERROR) << erro.what();
      continue;
    }
  }
  EscolheModeloEntidade(
      menu_modelos_proto,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherModeloEntidade,
          this, _1));
}

void InterfaceGrafica::VoltaEscolherModeloEntidade(
    const std::string& nome) {
  ent::Tabuleiro::ModelosComPesos modelos;
  modelos.ids_com_peso.emplace_back(nome);
  tabuleiro_->SelecionaModelosEntidades(modelos);
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
std::string NomeFeitico(const ent::EntidadeProto::InfoConhecido& c, const ent::Tabelas& tabelas) {
  if (!c.id().empty()) {
    const auto& feitico = tabelas.Feitico(c.id());
    if (!feitico.nome().empty()) {
      std::string str_area;
      std::string str_duracao;
      if (feitico.has_acao()) {
        const auto& acao = feitico.acao();
        str_area = acao.efeito_area() ? StringPrintf(", %s", StringArea(acao).c_str()) : "";
        if (acao.efeitos_adicionais().size() == 1) {
          const auto& ed = acao.efeitos_adicionais(0);
          str_duracao = ed.has_modificador_rodadas()
              ? StringPrintf(", %s", StringDuracao(ed.modificador_rodadas()).c_str())
              : "";
        }
      }
      return StringPrintf("%s, alcance: %d (q)%s%s",
          feitico.nome().c_str(),
          feitico.alcance_quadrados(),
          str_area.c_str(),
          str_duracao.c_str()
      );
    }
  }
  if (!c.nome().empty()) return c.nome();
  return c.id();
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
  EscolheItemLista("Parâmetros de Lancamento", lista_parametros, [this, notificacao, lista_parametros, ids] (bool ok_decisao, int indice_decisao) {
    ent::RodaNoRetorno([this]() {
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
      notificacao.entidade().feiticos_classes(0).feiticos_por_nivel().empty()) {
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
  int nivel_gasto = fc.feiticos_por_nivel().size() - 1;
  if (ClassePrecisaMemorizar(tabelas_, id_classe)) {
    // Monta lista de feiticos para lancar do nivel.
    const auto& fn = fc.feiticos_por_nivel(nivel_gasto);
    for (int indice = 0; indice < fn.para_lancar().size(); ++indice) {
      const auto& pl = fn.para_lancar(indice);
      if (pl.usado()) continue;
      const auto& c = ent::FeiticoConhecido(
          id_classe, pl.nivel_conhecido(), pl.indice_conhecido(), notificacao.entidade());
      const auto& feitico = tabelas_.Feitico(c.id());
      std::string link = feitico.link().empty() ? "" : StringPrintf("<a href='%s'>link</a>", feitico.link().c_str());
      lista.push_back(StringPrintf("nivel %d[%d]: %s %s", nivel_gasto, indice, NomeFeitico(c, tabelas_).c_str(), link.c_str()));
      items.emplace_back(c.id(), nivel_gasto, indice, indice);
    }
    if (lista.empty()) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(StringPrintf("Nao ha magia de nivel %d para gastar", nivel_gasto)));
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
          ntf::NovaNotificacaoErro(StringPrintf("Nao ha magia de nivel %d para gastar", nivel_gasto)));
      return;
    }
    for (int nivel = fc.feiticos_por_nivel().size() - 1; nivel >= 0; --nivel) {
      const auto& fn = fc.feiticos_por_nivel(nivel);
      for (int indice = 0; indice < fn.conhecidos().size(); ++indice) {
        const auto& c = fn.conhecidos(indice);
        const auto& feitico = tabelas_.Feitico(c.id());
        std::string link = feitico.link().empty() ? "" : StringPrintf("<a href='%s'>link</a>", feitico.link().c_str());
        lista.push_back(StringPrintf("nivel %d[%d]: %s %s", nivel, indice, NomeFeitico(c, tabelas_).c_str(), link.c_str()));
        // Gasta do nivel certo.
        items.emplace_back(c.id(), nivel, indice, indice_gasto);
      }
    }
  }
  bool conversao_espontanea = fc.conversao_espontanea();

  // Esse lambda rodada no final da UI.
  auto funcao_final = [this, notificacao, conversao_espontanea, id_classe, nivel_gasto, items] (int ok, int indice_lista) {
    ent::RodaNoRetorno([this]() {
      this->tabuleiro_->ReativaWatchdogSeMestre();
    });
    if (!ok) {
      LOG(INFO) << "Nao usando feitico";
      return;
    }
    // Consome o feitico.
    const auto& item = items[indice_lista];

    ntf::Notificacao grupo_notificacao;
    grupo_notificacao.set_tipo(ntf::TN_GRUPO_NOTIFICACOES);

    const auto* entidade = tabuleiro_->BuscaEntidade(notificacao.entidade().id());
    if (entidade == nullptr) {
      LOG(INFO) << "Erro, entidade nao existe";
      return;
    }
    if (ent::NotificacaoConsequenciaFeitico(
        tabelas_, id_classe, conversao_espontanea, item.nivel, item.indice, *entidade, &grupo_notificacao)) {
      tabuleiro_->EntraModoClique(ent::Tabuleiro::MODO_ACAO);
    }

    auto n_alteracao_feitico = ent::NotificacaoAlterarFeitico(
        id_classe, nivel_gasto, item.indice_gasto, /*usado=*/true, notificacao.entidade());
    *grupo_notificacao.add_notificacao() = *n_alteracao_feitico;

    tabuleiro_->AdicionaNotificacaoListaEventos(grupo_notificacao);
    tabuleiro_->TrataNotificacao(grupo_notificacao);
    VLOG(1) << "gastando feitico nivel: " << nivel_gasto << ", indice: " << item.indice_gasto;
  };

  tabuleiro_->DesativaWatchdogSeMestre();
  EscolheItemLista("Escolha o Feitiço", lista, funcao_final);
}

void InterfaceGrafica::EscolheVersaoTabuleiro(const std::string& titulo, std::function<void(int)> funcao_volta) {
  std::vector<std::string> items;
  for (int i = 0; i < tabuleiro_->Proto().versoes().size(); ++i) {
    const std::string& descricao = tabuleiro_->Proto().versoes(i).descricao();
    if (descricao.empty()) {
      items.push_back(StringPrintf("versão %d", i + 1));
    } else {
      items.push_back(descricao);
    }
  }
  EscolheItemLista(titulo, items, [this, funcao_volta](bool aceito, int indice) {
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
      items.push_back(StringPrintf("versão %d", i + 1));
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

}  // namespace ifg
