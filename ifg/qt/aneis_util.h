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

// Retorna o nome do Anel seguido por em uso ou nao usado.
inline std::string NomeAnelParaLista(const ent::Tabelas& tabelas, const ent::ItemMagicoProto& anel_pc) {
  const auto& anel_tabela = tabelas.Anel(anel_pc.id());
  return google::protobuf::StringPrintf(
      "%s%s",
      anel_tabela.nome().c_str(),
      anel_pc.em_uso() ? " (em uso)" : " (não usado)");
}

// Responsavel por tratar a edicao do tipo de efeito.
class AnelDelegate : public QItemDelegate {
 public:
  AnelDelegate(const ent::Tabelas& tabelas, QListWidget* lista, ent::EntidadeProto* proto)
      : QItemDelegate(lista), tabelas_(tabelas), lista_(lista), proto_(proto) {}

  QWidget* createEditor(
      QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    return PreencheConfiguraComboAneis(new QComboBox(parent));
  }

  // Escreve o valor do combo.
  void setEditorData(QWidget* editor, const QModelIndex& index) const override {
    auto* combo = qobject_cast<QComboBox*>(editor);
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr em setEditorData";
      return;
    }
    combo->setCurrentIndex(combo->findData(IdAnelCorrenteDoProto()));
  }

  // Salva o valor do combo no modelo.
  void setModelData(
      QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
    const int indice_proto = lista_->currentRow();
    if (indice_proto < 0 || indice_proto >= proto_->tesouro().aneis_size()) {
      LOG(ERROR) << "indice invalido em setEditorData: " << indice_proto;
      return;
    }
    const std::string id_anel = IdAnelCorrenteDoCombo(qobject_cast<QComboBox*>(editor));
    proto_->mutable_tesouro()->mutable_aneis(indice_proto)->set_id(id_anel);
  }

  // O tamanho padrao da linha nao cabe o combo de edicao.
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
    auto s = QItemDelegate::sizeHint(option, index);
    s.setHeight(s.height() + 4);
    return s;
  }

 private:
  // Retorna o id da anel corrente do combo.
  std::string IdAnelCorrenteDoCombo(QComboBox* combo) const {
    if (combo == nullptr) {
      LOG(ERROR) << "combo == nullptr";
      return "";
    }
    const int indice_anel = combo->currentIndex();
    return combo->itemData(indice_anel).toString().toStdString().c_str();
  }

  // Retorna o anel do personagem.
  const ent::ItemMagicoProto& AnelCorrenteDoProto() const {
    const int indice_proto = lista_->currentRow();
    if (indice_proto < 0 || indice_proto >= proto_->tesouro().aneis_size()) {
      LOG(ERROR) << "indice invalido em IdAnel: " << indice_proto;
      return ent::ItemMagicoProto::default_instance();
    }
    return proto_->tesouro().aneis(indice_proto);
  }

  // Retorna o id do anel corrente.
  const char* IdAnelCorrenteDoProto() const {
    return AnelCorrenteDoProto().id().c_str();
  }

  std::string NomeAnelCorrente() const {
    const auto& anel_pc = AnelCorrenteDoProto();
    return NomeAnelParaLista(tabelas_, anel_pc);
  }

  // Retorna o proprio combo por conveniencia. Preenche com os aneis da tabela, ordenado por nome.
  // O dado de cada linha sera o id do anel. Configura o combo para fechar e submeter os dados quando
  // alterado o item corrente.
  QComboBox* PreencheConfiguraComboAneis(QComboBox* combo) const {
    std::map<QString, std::string> itens_ordenados;
    for (const auto& pp : tabelas_.todas().tabela_aneis().aneis()) {
      QString nome_traduzido = tr(pp.nome().c_str());
      itens_ordenados.insert(std::make_pair(nome_traduzido, pp.id()));
    }
    for (const auto& par : itens_ordenados) {
      combo->addItem(par.first, QString(par.second.c_str()));
    }
    //connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor()));
    lambda_connect(combo, SIGNAL(currentIndexChanged(int)), [this, combo] () {
      // Tem que tirar o const aqui, pois quando for executado, o this nao sera const.
      auto* thiz = const_cast<AnelDelegate*>(this);
      // Aqui chama o setModelData do modelo.
      emit thiz->commitData(combo);
      emit thiz->closeEditor(combo);
      // Aqui eh so para trigar o itemChanged.
      emit lista_->model()->setData(lista_->currentIndex(), QString::fromUtf8(NomeAnelCorrente().c_str()));
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
