#ifndef IFG_INTERFACE_H
#define IFG_INTERFACE_H

#include <functional>
#include <string>

#include "arq/arquivo.h"
#include "ent/tabuleiro.h"
#include "ntf/notificacao.h"

namespace ent {
class Tabelas;
class Tabuleiro;
}  // namespace ent
namespace ifg {

class MenuModelos;
class TratadorTecladoMouse;

// Funcao auxiliar para misturar protos de menu de tipos diferentes (srd e nao srd).
void MisturaProtosMenu(const MenuModelos& entrada, MenuModelos* saida);

class InterfaceGrafica : public ntf::Receptor {
 public:
  InterfaceGrafica(
      const ent::Tabelas& tabelas, TratadorTecladoMouse* teclado_mouse, ent::Tabuleiro* tabuleiro, ntf::CentralNotificacoes* central)
      : tabelas_(tabelas), teclado_mouse_(teclado_mouse), tabuleiro_(tabuleiro), central_(central) {
    central_->RegistraReceptor(this);
  }

  virtual ~InterfaceGrafica() {
    central_->DesregistraReceptor(this);
  }

  bool TrataNotificacao(const ntf::Notificacao& notificacao) override;

 protected:
  // Mostra um dialogo de erro ou informacao.
  virtual void MostraMensagem(bool erro, const std::string& mensagem, std::function<void()> funcao_volta) = 0;

  // Funcao generica para retorno da escolha de um item da lista. funcao_volta eh chamada com false
  // em caso de cancelamento, ou com o indice escolhido caso contrario.
  virtual void EscolheItemLista(
      const std::string& titulo,
      const std::vector<std::string>& lista,
      std::function<void(bool, int)> funcao_volta) = 0;

  // Funcao generica para retorno da escolha de um ou mais items da lista. funcao_volta eh chamada com false
  // em caso de cancelamento, ou com os indices escolhidos caso contrario.
  virtual void EscolheItemsLista(
      const std::string& titulo,
      const std::vector<std::string>& lista,
      std::function<void(bool, std::vector<int>)> funcao_volta) = 0;

  // Mostra dialogo para escolher um item entre tab_estaticos e tab_dinamicos, chamando
  // a funcao de volta ao terminar.
  virtual void EscolheArquivoAbrirTabuleiro(
      const std::vector<std::string>& tab_estaticos,
      const std::vector<std::string>& tab_dinamicos,
      std::function<void(const std::string& nome, arq::tipo_e tipo)> funcao_volta) = 0;

  // Mostra dialogo para salvar tabuleiro, chamando a funcao de volta ao terminar.
  virtual void EscolheArquivoSalvarTabuleiro(
      std::function<void(const std::string& nome)> funcao_volta) = 0;

  // Dialogo com as versões do tabuleiro.
  void EscolheVersaoTabuleiro(const std::string& titulo, std::function<void(int)> funcao_volta);
  void EscolheVersoesTabuleiro(const std::string& titulo, std::function<void(const std::vector<int>&)> funcao_volta);

  // Mostra o dialogo para escolher um modelo de entidade.
  virtual void EscolheModeloEntidade(
      const MenuModelos& modelos,
      std::function<void(const std::string& nome)> funcao_volta) = 0;

  // Mostra o dialogo de selecao de cor. A funcao de volta recebe se houve selecao de cor e caso positivo,
  // os componentes rgba. A versao aqui apresentada nao faz nada, pois no android e IOS nao implementei.
  // No QT ela eh overriden.
  virtual void EscolheCor(
      float r, float g, float b, float a,
      std::function<void(bool, float, float, float, float)> funcao_volta) {
    funcao_volta(true, r, g, b, a);
  }

 protected:
  const ent::Tabelas& tabelas_;
  TratadorTecladoMouse* teclado_mouse_ = nullptr;
  ent::Tabuleiro* tabuleiro_ = nullptr;
  ntf::CentralNotificacoes* central_ = nullptr;

 private:
  void TrataEscolherAliados(const ntf::Notificacao& notificacao);

  void TrataEscolherDecisaoLancamento(const ntf::Notificacao& notificacao);

  void TrataEscolherPericia(const ntf::Notificacao& notificacao);
  void VoltaEscolherPericia(const ntf::Notificacao notificacao, std::vector<std::string> mapa_indice_id, bool ok, int indice_pericia);

  void TrataEscolherTipoTesouro(const ntf::Notificacao& notificacao);
  void VoltaEscolherTipoTesouro(const ntf::Notificacao notificacao, std::vector<ent::TipoTesouro> mapa_indice_id, bool ok, int indice_tipo);

  void TrataEscolherPocao(const ntf::Notificacao& notificacao);
  void VoltaEscolherPocao(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_selecionado);
  void VoltaEscolherEfeito(const ntf::Notificacao notificacao, unsigned int indice_pocao, bool ok, unsigned int indice_efeito);

  void TrataEscolherPergaminho(const ntf::Notificacao& notificacao);
  void VoltaEscolherPergaminho(const ntf::Notificacao notificacao, const std::vector<int> mapa_indices, bool ok, int indice_selecionado);

  void TrataEscolherFeitico(const ntf::Notificacao& notificacao);

  void TrataEscolheCor(const ntf::Notificacao& notificacao);
  void VoltaEscolheCor(bool ok, float r, float g, float b, float a);

  void TrataMostraMensagem(bool erro, const std::string& mensagem);

  void TrataAbrirTabuleiro(const ntf::Notificacao& notificacao);
  // A saida sera sempre TIPO_TABULEIRO_ESTATICO ou TIPO_TABULEIRO_DINAMICO, mesmo quando a entrada eh modelo3d.
  void VoltaAbrirTabuleiro(
    bool manter_entidades, bool modelo_3d, const std::string& nome, arq::tipo_e tipo_retornado);

  void TrataSalvarTabuleiro(const ntf::Notificacao& notificacao);
  void VoltaSalvarTabuleiro(bool modelo_3d, bool versionar, const std::string& nome);

  void TrataEscolherModeloEntidade(const ntf::Notificacao& notificacao);
  void VoltaEscolherModeloEntidade(const std::string& nome);

  void TrataEscolherVersao();
  void TrataEscolherVersaoParaRemocao();
};

}  // namespace ifg

#endif
