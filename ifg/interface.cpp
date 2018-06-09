#include <algorithm>
#include <functional>

#include "arq/arquivo.h"
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


namespace ifg {

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
    case ntf::TN_ABRIR_DIALOGO_ESCOLHER_PERICIA: {
      TrataEscolherPericia(notificacao);
      return true;
    }
    case ntf::TN_INFO:
    case ntf::TN_ERRO: {
      TrataMostraMensagem(notificacao.tipo() == ntf::TN_ERRO, notificacao.erro());
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
  std::vector<std::string> nomes_pericias;
  for (const auto& pericia : pericias_entidade) {
    nomes_pericias.push_back(tabelas_.Pericia(pericia.id()).nome());
  }
  EscolheItemLista(
      "Escolha a pericia", nomes_pericias,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPericia,
          this, notificacao,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPericia(ntf::Notificacao notificacao, bool ok, unsigned int indice_pericia) {
  if (ok && indice_pericia <= notificacao.entidade().info_pericias_size()) {
    const auto& entidade = notificacao.entidade();
    const auto& pericia = entidade.info_pericias(indice_pericia);
    const auto& pericia_tabelada = tabelas_.Pericia(pericia.id());
    const bool treinado = pericia.pontos() > 0;
    auto n = NovaNotificacao(ntf::TN_ADICIONAR_ACAO);
    auto* a = n->mutable_acao();
    auto* pd = a->mutable_pos_entidade();
    pd->set_x(entidade.pos().x());
    pd->set_y(entidade.pos().y());
    pd->set_z(entidade.pos().z());
    pd->set_id_cenario(entidade.pos().id_cenario());
    a->set_tipo(ent::ACAO_DELTA_PONTOS_VIDA);
    a->set_local_apenas(false);
    std::string texto;
    if (treinado || pericia_tabelada.sem_treinamento()) {
      const int bonus = ent::BonusTotal(pericia.bonus());
      const int dado = ent::RolaDado(20);
      texto = google::protobuf::StringPrintf("%s: d%d + %d = %d", pericia_tabelada.nome().c_str(), dado, bonus, dado + bonus);
    } else {
      texto = google::protobuf::StringPrintf("Pericia %s requer treinamento", pericia_tabelada.nome().c_str());
    }
    a->set_texto(texto);
    central_->AdicionaNotificacao(n.release());
  }
  tabuleiro_->ReativaWatchdogSeMestre();
}

//----------------
// Escolher Pocao.
//----------------
void InterfaceGrafica::TrataEscolherPocao(const ntf::Notificacao& notificacao) {
  tabuleiro_->DesativaWatchdogSeMestre();
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  if (pocoes_entidade.size() == 1) {
    VoltaEscolherPocao(notificacao, true, 0);
    return;
  }

  std::vector<std::string> nomes_pocoes;
  for (const auto& pocao : pocoes_entidade) {
    nomes_pocoes.push_back(pocao.nome().empty() ? tabelas_.Pocao(pocao.id()).nome() : pocao.nome());
  }
  EscolheItemLista(
      "Escolha a poção", nomes_pocoes,
      std::bind(
          &ifg::InterfaceGrafica::VoltaEscolherPocao,
          this, notificacao,
          _1, _2));
}

void InterfaceGrafica::VoltaEscolherPocao(ntf::Notificacao notificacao, bool ok, unsigned int indice_pocao) {
  const auto& pocoes_entidade = notificacao.entidade().tesouro().pocoes();
  if (!ok || indice_pocao >= pocoes_entidade.size()) {
    VoltaEscolherEfeito(notificacao, 0, false, 0);
    return;
  }
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

void InterfaceGrafica::VoltaEscolherEfeito(ntf::Notificacao notificacao, unsigned int indice_pocao, bool ok, unsigned int indice_efeito) {
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
          this, notificacao.entidade().has_modelo_3d(),_1));
}

void InterfaceGrafica::VoltaSalvarTabuleiro(
    bool modelo_3d, const std::string& nome) {
  auto n = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
  n->set_endereco(nome);
  if (modelo_3d) {
    n->mutable_entidade()->mutable_modelo_3d();
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
  tabuleiro_->SelecionaModeloEntidade(nome);
  tabuleiro_->ReativaWatchdogSeMestre();
}

//-----------------
// Escolher feitico
//-----------------
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
  std::vector<std::pair<int, int>> items;
  int nivel_gasto = fc.feiticos_por_nivel().size() - 1;
  if (ClassePrecisaMemorizar(tabelas_, id_classe)) {
    // Monta lista de feiticos para lancar do nivel.
    const auto& fn = fc.feiticos_por_nivel(nivel_gasto);
    for (int indice = 0; indice < fn.para_lancar().size(); ++indice) {
      const auto& pl = fn.para_lancar(indice);
      if (pl.usado()) continue;
      const auto& c = ent::FeiticoConhecido(
          id_classe, pl.nivel_conhecido(), pl.indice_conhecido(), notificacao.entidade());
      lista.push_back(google::protobuf::StringPrintf("nivel %d[%d]: %s", nivel_gasto, indice, c.nome().c_str()));
      items.push_back(std::make_pair(nivel_gasto, indice));
    }
    if (lista.empty()) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(google::protobuf::StringPrintf("Nao ha magia de nivel %d para gastar", nivel_gasto)));
      return;
    }
  } else {
    // Monta lista de feiticos conhecidos ate o nivel.
    int indice_gasto = ent::IndiceFeiticoDisponivel(id_classe, nivel_gasto, notificacao.entidade());
    if (indice_gasto == -1) {
      central_->AdicionaNotificacao(
          ntf::NovaNotificacaoErro(google::protobuf::StringPrintf("Nao ha magia de nivel %d para gastar", nivel_gasto)));
      return;
    }
    for (int nivel = fc.feiticos_por_nivel().size() - 1; nivel >= 0; --nivel) {
      const auto& fn = fc.feiticos_por_nivel(nivel);
      for (int indice = 0; indice < fn.conhecidos().size(); ++indice) {
        const auto& c = fn.conhecidos(indice);
        lista.push_back(google::protobuf::StringPrintf("nivel %d[%d]: %s", nivel, indice, c.nome().c_str()));
        // Gasta do nivel certo.
        items.push_back(std::make_pair(nivel_gasto, indice_gasto));
      }
    }
  }

  EscolheItemLista("Escolha o Feitiço", lista,
      [this, notificacao, id_classe, items](bool ok, int indice_lista) {
    if (!ok) {
      LOG(INFO) << "Nao usando feitico";
      return;
    }
    // Consome o feitico.
    int nivel;
    int indice;
    std::tie(nivel, indice) = items[indice_lista];
    auto n = ent::NotificacaoAlterarFeitico(
        id_classe, nivel, indice, true /*usado*/, notificacao.entidade());
    tabuleiro_->AdicionaNotificacaoListaEventos(*n);
    central_->AdicionaNotificacao(n.release());
  });
}

}  // namespace ifg
