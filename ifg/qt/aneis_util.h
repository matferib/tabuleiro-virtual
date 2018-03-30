#ifndef IFG_QT_ANEIS_UTIL_H
#define IFG_QT_ANEIS_UTIL_H

#include <QComboBox>
#include <QItemDelegate>
#include "ent/tabelas.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "ifg/qt/util.h"

namespace ifg {
namespace qt {

enum class TipoItem {
  TIPO_ANEL,
  TIPO_MANTO,
};

// Retorna o item da tabela por tipo.
const ent::ItemMagicoProto& ItemTabela(
    const ent::Tabelas& tabelas, TipoItem tipo, const std::string& id);
// Retorna os itens da tabela.
const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensTabela(
    const ent::Tabelas& tabelas, TipoItem tipo);

// Retorna o nome do item seguido por 'em uso' ou 'não usado'.
std::string NomeParaLista(
    const ent::Tabelas& tabelas, TipoItem tipo, const ent::ItemMagicoProto& item_pc);

// Responsavel por tratar a edicao do tipo de efeito.
class ItemMagicoDelegate : public QItemDelegate {
 public:
  ItemMagicoDelegate(
      const ent::Tabelas& tabelas, TipoItem tipo,
      QListWidget* lista, ent::EntidadeProto* proto)
      : QItemDelegate(lista), tabelas_(tabelas), lista_(lista), proto_(proto), tipo_(tipo) {}

  QWidget* createEditor(
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

  // Salva o valor do combo no modelo.
  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    const int indice_proto = lista_->currentRow();
    auto* itens = ItensPersonagemMutavel();
    if (indice_proto < 0 || indice_proto >= itens->size()) {
      LOG(ERROR) << "indice invalido em setEditorData: " << indice_proto;
      return;
    }
    itens->Mutable(indice_proto)->set_id(IdCorrenteDoCombo(qobject_cast<QComboBox*>(editor)));
  }

  // O tamanho padrao da linha nao cabe o combo de edicao.
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    auto s = QItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 4);
    return s;
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

  const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensPersonagem() const {
    switch (tipo_) {
      case TipoItem::TIPO_ANEL: return proto_->tesouro().aneis();
      case TipoItem::TIPO_MANTO: return proto_->tesouro().mantos();
    }
    LOG(ERROR) << "Tipo de item invalido (" << (int)tipo_ << "), retornando anel";
    return proto_->tesouro().aneis();
  }

  google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>* ItensPersonagemMutavel() const {
    switch (tipo_) {
      case TipoItem::TIPO_ANEL: return proto_->mutable_tesouro()->mutable_aneis();
      case TipoItem::TIPO_MANTO: return proto_->mutable_tesouro()->mutable_mantos();
    }
    LOG(ERROR) << "Tipo de item invalido (" << (int)tipo_ << "), retornando nullptr";
    return nullptr;
  }

  // Retorna o item do personagem.
  const ent::ItemMagicoProto& ItemCorrenteDoProto() const {
    const int indice_proto = lista_->currentRow();
    const auto& itens = ItensPersonagem();
    if (indice_proto < 0 || indice_proto >= itens.size()) {
      LOG(ERROR) << "indice invalido em ItemCorrenteDoProto: " << indice_proto;
      return ent::ItemMagicoProto::default_instance();
    }
    return itens.Get(indice_proto);
  }

  // Retorna o id do item corrente.
  const char* IdCorrenteDoProto() const {
    return ItemCorrenteDoProto().id().c_str();
  }

  std::string NomeCorrente() const {
    return NomeParaLista(tabelas_, tipo_, ItemCorrenteDoProto());
  }

  // Retorna o proprio combo por conveniencia. Preenche com os itens da tabela, ordenado por nome.
  // O dado de cada linha sera o id do item. Configura o combo para fechar e submeter os dados quando
  // alterado o item corrente.
  QComboBox* PreencheConfiguraCombo(QComboBox* combo) const {
    std::map<QString, std::string> itens_ordenados;
    for (const auto& pp : ItensTabela(tabelas_, tipo_)) {
      QString nome_traduzido = tr(pp.nome().c_str());
      itens_ordenados.insert(std::make_pair(nome_traduzido, pp.id()));
    }
    for (const auto& par : itens_ordenados) {
      combo->addItem(par.first, QString(par.second.c_str()));
    }
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo] () {
      // Tem que tirar o const aqui, pois quando for executado, o this nao sera const.
      auto* thiz = const_cast<ItemMagicoDelegate*>(this);
      // Aqui chama o setModelData do modelo.
      emit thiz->commitData(combo);
      emit thiz->closeEditor(combo);
      // Aqui eh so para trigar o itemChanged.
      emit lista_->model()->setData(lista_->currentIndex(), QString::fromUtf8(NomeCorrente().c_str()));
    });
    return combo;
  }

  const ent::Tabelas& tabelas_;
  QListWidget* lista_;
  ent::EntidadeProto* proto_;
  TipoItem tipo_;
};

inline const ent::ItemMagicoProto& ItemTabela(
    const ent::Tabelas& tabelas, TipoItem tipo, const std::string& id) {
  switch (tipo) {
    case TipoItem::TIPO_ANEL: return tabelas.Anel(id);
    case TipoItem::TIPO_MANTO: return tabelas.Manto(id);
  }
  return ent::ItemMagicoProto::default_instance();
}

inline const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensTabela(
    const ent::Tabelas& tabelas, TipoItem tipo) {
  switch (tipo) {
    case TipoItem::TIPO_ANEL: return tabelas.todas().tabela_aneis().aneis();
    case TipoItem::TIPO_MANTO: return tabelas.todas().tabela_mantos().mantos();
  }
  LOG(ERROR) << "Tipo invalido (" << (int)tipo << ") para ItensTabela, retornando aneis";
  return tabelas.todas().tabela_aneis().aneis();
}

// Retorna o nome do item seguido por em uso ou nao usado.
inline std::string NomeParaLista(
    const ent::Tabelas& tabelas, TipoItem tipo, const ent::ItemMagicoProto& item_pc) {
  const auto& item_tabela = ItemTabela(tabelas, tipo, item_pc.id());
  return google::protobuf::StringPrintf(
      "%s%s",
      item_tabela.nome().c_str(),
      item_pc.em_uso() ? " (em uso)" : " (não usado)");
}

}  // namespace qt
}  // namespace ifg

#endif
