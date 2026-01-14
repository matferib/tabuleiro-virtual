#ifndef IFG_QT_ARMAS_UTIL_H
#define IFG_QT_ARMAS_UTIL_H

#include <QtWidgets/QComboBox>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QToolTip>
#include "absl/strings/str_format.h"
#include "ent/tabelas.h"
#include "log/log.h"
#include "ifg/qt/util.h"

namespace ifg {
namespace qt {

// Retorna as armas da tabela.
const google::protobuf::RepeatedPtrField<ent::ArmaProto>& ArmasTabela(
    const ent::Tabelas& tabelas);

// Retorna a string de descricao da arma.
std::string DescricaoParaLista(
    const ent::Tabelas& tabelas, const ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem& arma_pc);

// Responsavel por tratar a edicao do tipo de efeito.
class ArmaDelegate : public QItemDelegate {
 public:
   ArmaDelegate(
      const ent::Tabelas& tabelas,
      QListWidget* lista, ent::EntidadeProto* proto)
      : QItemDelegate(lista), tabelas_(tabelas), lista_(lista), proto_(proto) {}

  __declspec(noinline) QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraCombo(new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr em setEditorData";
      return;
    }
    combo->setCurrentIndex(combo->findData(IdCorrenteDoProto()));
  }

  // Salva o valor do combo no modelo (proto).
   void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    const int indice_selecionado = lista_->currentRow();
    int indice  = indice_selecionado;
    auto* armas_pc = ArmasPersonagemMutavel();
    //if (indice < 0) {
    //  LOG(ERROR) << "indice invalido em setEditorData: " << indice;
    //  return;
    //}
    //indice -= armas_pc->size();
    const auto& at = ArmasTabela(tabelas_);
    if (indice < 0 || indice >= at.size()) {
      LOG(ERROR) << "indice invalido em setEditorData: " << indice;
      return;
    }
    auto* arma_pc = armas_pc->Mutable(indice_selecionado);
    arma_pc->set_id_tabela(IdCorrenteDoCombo(qobject_cast<QComboBox*>(editor)));
    GeraNomeArma(tabelas_, *arma_pc);
  }

  // O tamanho padrao da linha nao cabe o combo de edicao.
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    auto s = QItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 4);
    return s;
  }

  bool helpEvent(
      QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option,
      const QModelIndex & index) override {
    if (event == nullptr || event->type() != QEvent::ToolTip || view == nullptr) {
      return false;
    }
    QListWidget* parent = qobject_cast<QListWidget*>(view);
    if (parent == nullptr) {
      return false;
    }
    int indice = parent->row(parent->itemAt(event->pos()));
    if (indice < 0) {
      return false;
    }
    QToolTip::showText(
        event->globalPos(),
        QString::fromUtf8(DescricaoParaLista(tabelas_, ArmaDoProto(indice)).c_str()));
    return true;
  }

 private:
  // Retorna o id do item corrente do combo.
  std::string IdCorrenteDoCombo(QComboBox* combo) const {
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr";
      return "";
    }
    const int indice = combo->currentIndex();
    return combo->itemData(indice).toString().toStdString().c_str();
  }

  const google::protobuf::RepeatedPtrField<ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem>& ArmasPersonagem() const {
    return ent::ArmasArmadurasOuEscudosProto(ent::TT_ARMA, *proto_);
  }

  google::protobuf::RepeatedPtrField<ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem>* ArmasPersonagemMutavel() const {
    return ent::ArmasArmadurasOuEscudosProtoMutavel(ent::TT_ARMA, proto_);
  }

  // Retorna o item do personagem.
  __declspec(noinline) const ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem& ArmaCorrenteDoProto() const {
    return ArmaDoProto(lista_->currentRow());
  }

  __declspec(noinline) const ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem& ArmaDoProto(int indice_proto) const {
    const auto& armas = ArmasPersonagem();
    if (indice_proto < 0 || indice_proto >= armas.size()) {
      LOG(ERROR) << "indice invalido em ArmaDoPRoto: " << indice_proto;
      return ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem::default_instance();
    }
    return armas.Get(indice_proto);
  }

  // Retorna o id do item corrente.
  const char* IdCorrenteDoProto() const {
    return ArmaCorrenteDoProto().id_tabela().c_str();
  }

  // Retorna o proprio combo por conveniencia. Preenche com os itens da tabela, ordenado por nome.
  // O dado de cada linha sera o id do item. Configura o combo para fechar e submeter os dados quando
  // alterado o item corrente.
  QComboBox* PreencheConfiguraCombo(QComboBox* combo) const {
    std::map<QString, std::string> armas_ordenadas;
    //for (const auto& apc : ArmasPersonagem()) {
    //  QString nome_traduzido = tr(apc.nome().c_str());
    //  itens_ordenados.insert(std::make_pair(nome_traduzido, apc.id()));
    //}
    for (const auto& at : ArmasTabela(tabelas_)) {
      QString nome_traduzido = tr(at.nome().c_str());
      armas_ordenadas.insert(std::make_pair(nome_traduzido, at.id()));
    }
    for (const auto& [id, nome] : armas_ordenadas) {
      combo->addItem(id, QString(nome.c_str()));
    }
    ExpandeComboBox(combo);
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo] () {
      // Tem que tirar o const aqui, pois quando for executado, o this nao sera const.
      auto* thiz = const_cast<ArmaDelegate*>(this);
      // Aqui chama o setModelData do modelo.
      emit thiz->commitData(combo);
      emit thiz->closeEditor(combo);
      // Aqui eh so para trigar o itemChanged.
      emit lista_->model()->setData(lista_->currentIndex(), QString::fromUtf8(ArmaCorrenteDoProto().nome().c_str()));
    });
    return combo;
  }

  const ent::Tabelas& tabelas_;
  QListWidget* lista_;
  ent::EntidadeProto* proto_;
};

inline const google::protobuf::RepeatedPtrField<ent::ArmaProto>& ArmasTabela(
    const ent::Tabelas& tabelas) {
  return tabelas.todas().tabela_armas().armas();
}

inline std::string DescricaoParaLista(
  const ent::Tabelas& tabelas, const ent::EntidadeProto::ArmaArmaduraOuEscudoPersonagem& arma_pc) {
  const auto& arma_tabela = ent::ArmaTabela(tabelas, arma_pc.id_tabela());
  std::string nome = absl::StrFormat(
      "%s [%d PO]",
      arma_tabela.nome().c_str(),
      ent::PrecoArmaPo(arma_pc));
  if (arma_tabela.nome().empty()) { nome = "---"; }
  return nome.c_str();
}

}  // namespace qt
}  // namespace ifg

#endif
