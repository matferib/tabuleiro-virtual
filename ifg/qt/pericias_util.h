#ifndef IFG_QT_DIALOGO_PERICIAS_UTIL_H
#define IFG_QT_DIALOGO_PERICIAS_UTIL_H

#include <QTableView>
#include "ent/entidade.pb.h"
#include "ent/tabelas.h"
#include "ent/util.h"
#include "ifg/qt/util.h"
#include "log/log.h"

namespace ifg {
namespace qt {

// Modelo de talento para ser usado pelos views de tabela.
class ModeloPericias : public QAbstractTableModel {
  Q_OBJECT
 public:
  using InfoPericia = ent::EntidadeProto::InfoPericia;

  ModeloPericias(const ent::Tabelas& tabelas, const ent::EntidadeProto& proto, QTableView* tabela)
      : QAbstractTableModel(tabela), tabelas_(tabelas), proto_(proto) {
    Recomputa();
  }

  // Numero de linhas da tabela.
  int rowCount(const QModelIndex& parent =  QModelIndex()) const override {
    return (int)modelo_.size();
  }

  // 0: id pericia. 1. Atributo. 2. Ranks. 3. De classe. 4. Complemento. 5. Bonus.
  int columnCount(const QModelIndex& parent = QModelIndex()) const override {
    return 6;
  }

  // Os cabeçalhos.
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
    if (orientation == Qt::Horizontal && role == Qt::SizeHintRole && section == 0) {
      // Hack pra diminuir o tamanho da primeira coluna.
      QSize qs = QAbstractTableModel::headerData(0, Qt::Horizontal, Qt::SizeHintRole).toSize();
      qs.setWidth(qs.width() / 2);
      return QVariant(qs);
    }

    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return QVariant();
    }
    switch (section) {
      case 0: return QVariant(QString::fromUtf8("Perícia"));
      case 1: return QVariant(QString::fromUtf8("Atr"));
      case 2: return QVariant(QString::fromUtf8("Pts"));
      case 3: return QVariant(QString::fromUtf8("De Classe"));
      case 4: return QVariant(QString::fromUtf8("Complemento"));
      case 5: return QVariant(QString::fromUtf8("Total"));
      default: return QVariant("Desconhecido");
    }
  }

  // Dado de cada celula.
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::BackgroundRole) {
      return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= modelo_.size()) return QVariant();
    const int column = index.column();
    switch (column) {
      case 0: {
        const auto& pt = tabelas_.Pericia(modelo_[row].id());
        if (role == Qt::DisplayRole) {
          return QVariant(QString::fromUtf8(pt.nome().c_str()));
        } else if (role == Qt::BackgroundRole) {
          bool pode_usar = pt.sem_treinamento();
          if (modelo_[row].pontos() > 0) {
            pode_usar = true;
          }
          return pode_usar ? QVariant() : QVariant(QBrush(Qt::red));
        } else {
          // Nao editavel.
          return QVariant();
        }
      }
      case 1: return QVariant(ent::TipoAtributo_Name(tabelas_.Pericia(modelo_[row].id()).atributo()).substr(3, 3).c_str());
      case 2: return QVariant(modelo_[row].pontos());
      case 3: {
        if (role == Qt::DisplayRole) {
          return QVariant(PericiaDeClasse(tabelas_, modelo_[row].id(), proto_));
        } else {
          // Nao editavel.
          return QVariant();
        }
      }
      case 4: return QVariant(QString::fromUtf8(modelo_[row].complemento().c_str()));
      case 5: {
        if (role == Qt::DisplayRole) {
          return QVariant(BonusTotal(modelo_[row].bonus()));
        } else {
          // Nao editavel.
          return QVariant();
        }
      }
      default:
        if (role == Qt::BackgroundRole) {
          return QVariant();
        }
    }
    // Nunca deveria chegar aqui.
    LOG(INFO) << "Coluna invalida: " << column;
    return QVariant();
  }

  bool setData(const QModelIndex& index, const QVariant& value, int role) override {
    if (role != Qt::EditRole) {
      return false;
    }

    const int row = index.row();
    if (row < 0 || row >= modelo_.size()) {
      LOG(INFO) << "Linha invalida " << row;
      return false;
    }
    const int column = index.column();
    switch (column) {
      // id
      case 0: return false;
      case 1: return false;
      // pontos
      case 2: {
        modelo_[row].set_pontos(value.toInt());
        emit dataChanged(index, index);
        return true;
      }
      // de classe.
      case 3: return false;
      // complemento.
      case 4: {
        modelo_[row].set_complemento(value.toString().toUtf8().constData());
        emit dataChanged(index, index);
        return true;
      }
      // total.
      case 5: return false;
      default: ;
    }
    return false;
  }

  Qt::ItemFlags flags(const QModelIndex & index) const override {
    switch (index.column()) {
      case 0:
      case 1:
      case 3:
      case 5:
        return Qt::ItemIsEnabled;
      default:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
  }

  google::protobuf::RepeatedPtrField<InfoPericia> Converte() const {
    google::protobuf::RepeatedPtrField<InfoPericia> ret(modelo_.begin(), modelo_.end());
    return ret;
  }

  // Recomputa o modelo apos atualizacoes.
  void Recomputa() {
    modelo_.clear();
    // Mapa da tabela.
    std::map<std::string, const ent::PericiaProto*> mapa_pericias;
    for (const auto& pp : tabelas_.todas().tabela_pericias().pericias()) {
      mapa_pericias.insert(std::make_pair(pp.nome(), &pp));
    }
    // Mapa do proto.
    std::unordered_map<std::string, const InfoPericia*> mapa_pericias_proto;
    for (const auto& ip : proto_.info_pericias()) {
      mapa_pericias_proto[ip.id()] = &ip;
    }
    // Preenche o modelo.
    for (const auto& par_nome_pericia : mapa_pericias) {
      const auto& pp = *par_nome_pericia.second;
      InfoPericia pericia;
      pericia.set_id(pp.id());
      auto it = mapa_pericias_proto.find(pp.id());
      if (it != mapa_pericias_proto.end()) {
        pericia = *it->second;
      }
      //LOG(INFO) << "Pericia:" << pericia.ShortDebugString();
      modelo_.push_back(pericia);
    }
  }

 private:
  const ent::Tabelas& tabelas_;
  const ent::EntidadeProto& proto_;
  std::vector<InfoPericia> modelo_;
};

}  // namespace qt
}  // namespace ifg

#endif
