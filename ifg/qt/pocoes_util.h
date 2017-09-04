#ifndef IFG_QT_POCOES_UTIL_H
#define IFG_QT_POCOES_UTIL_H

#include <QComboBox>
#include <QItemDelegate>
#include "ent/tabelas.h"

namespace ifg {
namespace qt {

// Responsavel por tratar a edicao do tipo de efeito.
class PocaoDelegate : public QItemDelegate {
 public:
  PocaoDelegate(const ent::Tabelas& tabelas, QListWidget* lista, ent::EntidadeProto* proto)
      : QItemDelegate(lista), tabelas_(tabelas), lista_(lista), proto_(proto) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraComboPocoes(new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr em setEditorData";
      return;
    }
    combo->setCurrentIndex(combo->findData(IdPocaoCorrenteDoProto()));
  }

  // Salva o valor do combo no modelo.
  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    const int indice_proto = lista_->currentRow();
    if (indice_proto < 0 || indice_proto >= proto_->tesouro().pocoes_size()) {
      LOG(ERROR) << "indice invalido em setEditorData: " << indice_proto;
      return;
    }
    proto_->mutable_tesouro()->mutable_pocoes(indice_proto)->set_id(IdPocaoCorrenteDoCombo(qobject_cast<QComboBox*>(editor)));
  }

  // O tamanho padrao da linha nao cabe o combo de edicao.
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    auto s = QItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 4);
    return s;
  }

 private:
  // Retorna o id da pocao corrente do combo.
  const char* IdPocaoCorrenteDoCombo(QComboBox* combo) const {
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr";
      return "";
    }
    const int indice_pocao = combo->currentIndex();
    return combo->itemData(indice_pocao).toString().toStdString().c_str();
  }

  // Retorna o id da pocao do item corrente.
  const char* IdPocaoCorrenteDoProto() const {
    const int indice_proto = lista_->currentRow();
    if (indice_proto < 0 || indice_proto >= proto_->tesouro().pocoes_size()) {
      LOG(ERROR) << "indice invalido em IdPocao: " << indice_proto;
      return "";
    }
    return proto_->tesouro().pocoes(indice_proto).id().c_str();
  }

  const char* NomePocaoCorrente() const {
    return tabelas_.Pocao(IdPocaoCorrenteDoProto()).nome().c_str();
  }

  QComboBox* PreencheConfiguraComboPocoes(QComboBox* combo) const {
    std::map<std::string, std::string> itens_ordenados;
    for (const auto& pp : tabelas_.todas().tabela_pocoes().pocoes()) {
      itens_ordenados.insert(std::make_pair(pp.nome(), pp.id()));
    }
    for (const auto& par : itens_ordenados) {
      combo->addItem(QString::fromUtf8(par.first.c_str()), QString(par.second.c_str()));
    }
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo] () {
      // Tem que tirar o const aqui, pois quando for executado, o this nao sera const.
      auto* thiz = const_cast<PocaoDelegate*>(this);
      emit thiz->commitData(combo);
      emit thiz->closeEditor(combo);
      // Aqui eh so para trigar o itemChanged.
      emit lista_->model()->setData(lista_->currentIndex(), QString::fromUtf8(NomePocaoCorrente()));
    });
    return combo;
  }

  const ent::Tabelas& tabelas_;
  QListWidget* lista_;
  ent::EntidadeProto* proto_;
};

}  // namespace qt
}  // namespace ifg

#endif