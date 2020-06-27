#ifndef IFG_QT_ITEMS_MAGICOS_UTIL_H
#define IFG_QT_ITEMS_MAGICOS_UTIL_H

#include <QComboBox>
#include <QItemDelegate>
#include <QToolTip>
#include "ent/tabelas.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "ifg/qt/util.h"

namespace ifg {
namespace qt {

// Retorna o maximo de itens em uso pelo tipo.
int MaximoEmUso(ent::TipoItem tipo);

// Retorna os itens da tabela.
const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensTabela(
    const ent::Tabelas& tabelas, ent::TipoItem tipo);

// Retorna a string de descricao do item.
std::string DescricaoParaLista(
    const ent::Tabelas& tabelas, ent::TipoItem tipo, const ent::ItemMagicoProto& item_pc);

// Retorna o nome do item seguido por 'em uso' ou 'não usado'.
std::string NomeParaLista(
    const ent::Tabelas& tabelas, ent::TipoItem tipo, const ent::ItemMagicoProto& item_pc);

// Responsavel por tratar a edicao do tipo de efeito.
class ItemMagicoDelegate : public QItemDelegate {
 public:
  ItemMagicoDelegate(
      const ent::Tabelas& tabelas, ent::TipoItem tipo,
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
        QString::fromUtf8(DescricaoParaLista(tabelas_, tipo_, ItemDoProto(indice)).c_str()));
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

  const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensPersonagem() const {
    return ent::ItensProto(tipo_, *proto_);
  }

  google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>* ItensPersonagemMutavel() const {
    return ent::ItensProtoMutavel(tipo_, proto_);
  }

  // Retorna o item do personagem.
  const ent::ItemMagicoProto& ItemCorrenteDoProto() const {
    return ItemDoProto(lista_->currentRow());
  }

  const ent::ItemMagicoProto& ItemDoProto(int indice_proto) const {
    const auto& itens = ItensPersonagem();
    if (indice_proto < 0 || indice_proto >= itens.size()) {
      LOG(ERROR) << "indice invalido em ItemDoPRoto: " << indice_proto;
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
    ExpandeComboBox(combo);
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
  ent::TipoItem tipo_;
};

inline const google::protobuf::RepeatedPtrField<ent::ItemMagicoProto>& ItensTabela(
    const ent::Tabelas& tabelas, ent::TipoItem tipo) {
  switch (tipo) {
    case ent::TipoItem::TIPO_ITEM_MUNDANO: return tabelas.todas().tabela_itens_mundanos().itens();
    case ent::TipoItem::TIPO_ANEL: return tabelas.todas().tabela_aneis().aneis();
    case ent::TipoItem::TIPO_MANTO: return tabelas.todas().tabela_mantos().mantos();
    case ent::TipoItem::TIPO_LUVAS: return tabelas.todas().tabela_luvas().luvas();
    case ent::TipoItem::TIPO_BRACADEIRAS: return tabelas.todas().tabela_bracadeiras().bracadeiras();
    case ent::TipoItem::TIPO_POCAO: return tabelas.todas().tabela_pocoes().pocoes();
    case ent::TipoItem::TIPO_AMULETO: return tabelas.todas().tabela_amuletos().amuletos();
    case ent::TipoItem::TIPO_BOTAS: return tabelas.todas().tabela_botas().botas();
    case ent::TipoItem::TIPO_CHAPEU: return tabelas.todas().tabela_chapeus().chapeus();
    case ent::TipoItem::TIPO_PERGAMINHO_ARCANO: return tabelas.todas().tabela_pergaminhos().pergaminhos_arcanos();
    case ent::TipoItem::TIPO_PERGAMINHO_DIVINO: return tabelas.todas().tabela_pergaminhos().pergaminhos_divinos();
    case ent::TipoItem::TIPO_VARINHA: return tabelas.todas().tabela_varinhas().varinhas();
    default: ;
  }
  LOG(ERROR) << "Tipo invalido (" << (int)tipo << ") para ItensTabela, retornando aneis";
  return tabelas.todas().tabela_aneis().aneis();
}

inline std::string NomeParaLista(
    const ent::Tabelas& tabelas, ent::TipoItem tipo, const ent::ItemMagicoProto& item_pc) {
  const auto& item_tabela = ent::ItemTabela(tabelas, tipo, item_pc.id());
  const std::string nivel = item_tabela.has_nivel_conjurador() ? google::protobuf::StringPrintf(", nv conj: %d", item_tabela.nivel_conjurador()) : "";
  std::string nome = google::protobuf::StringPrintf("%s%s [%s]", item_tabela.nome().c_str(), nivel.c_str(), ent::PrecoItem(item_tabela).c_str());
  if (item_tabela.nome().empty()) { nome = "---"; }
  return tipo == ent::TipoItem::TIPO_PERGAMINHO_ARCANO || tipo == ent::TipoItem::TIPO_PERGAMINHO_DIVINO  || tipo == ent::TipoItem::TIPO_POCAO || tipo == ent::TipoItem::TIPO_ITEM_MUNDANO || tipo == ent::TipoItem::TIPO_VARINHA
      ? nome
      : google::protobuf::StringPrintf(
          "%s%s",
          nome.c_str(),
          item_pc.em_uso() ? " (em uso)" : " (não usado)");
}

inline std::string DescricaoParaLista(
    const ent::Tabelas& tabelas, ent::TipoItem tipo, const ent::ItemMagicoProto& item_pc) {
  const auto& item_tabela = ent::ItemTabela(tabelas, tipo, item_pc.id());
  return item_tabela.descricao().c_str();
}


inline int MaximoEmUso(ent::TipoItem tipo) {
  switch (tipo) {
    case ent::TipoItem::TIPO_ANEL: return 2;
    default: return 1;
  }
}

}  // namespace qt
}  // namespace ifg

#endif
