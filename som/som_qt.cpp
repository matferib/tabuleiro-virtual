#include <absl/strings/str_format.h>
#include <QFileInfo>
#include <QSoundEffect>
#include <QThread>
#include <list>
#include <memory>
#include "arq/arquivo.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "som/som.h"

namespace som {
namespace {

std::unordered_map<std::string, std::unique_ptr<QSoundEffect>> g_fxs;
const ent::OpcoesProto* g_opcoes = nullptr;

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  std::vector<std::string> sons = arq::ConteudoDiretorio(arq::TIPO_SOM);
  for (const std::string& som : sons) {
    auto fx = std::make_unique<QSoundEffect>();
    QString qs = QFileInfo(QString::fromStdString(absl::StrFormat("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), som.c_str()))).absoluteFilePath();
    fx->setSource(QUrl::fromLocalFile(qs));
    fx->setLoopCount(1);
    g_fxs[som] = std::move(fx);
  }
  g_opcoes = &opcoes;
}

void Finaliza() {
  g_fxs.clear();
  g_opcoes = nullptr;
}

void Toca(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  if (auto it = g_fxs.find(nome); it == g_fxs.end()) return;
  else it->second->play();
}

}  // namespace som
