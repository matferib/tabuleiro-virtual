#include "som/som.h"

#include <QtCore/QFileInfo>
#include <QtMultimedia/QSoundEffect>
#include <QtCore/QThread>

#include <list>
#include <memory>

#include "absl/strings/str_format.h"
#include "arq/arquivo.h"
#include "log/log.h"

namespace som {
namespace {

std::unordered_map<std::string, std::unique_ptr<QSoundEffect>> g_fxs;
const ent::OpcoesProto* g_opcoes = nullptr;

std::unique_ptr<QSoundEffect> CarregaSomUnico(const std::string& nome) {
  auto fx = std::make_unique<QSoundEffect>();
  QString qs = QFileInfo(
      QString::fromStdString(absl::StrFormat("%s/%s",
                                             arq::Diretorio(arq::TIPO_SOM).c_str(),
                                             nome.c_str()))).absoluteFilePath();
  fx->setSource(QUrl::fromLocalFile(qs));
  fx->setLoopCount(1);
  return fx;
}

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  {
    LOG(INFO) << "Forçando sistema de som a iniciar...";
    // Toca na inicialização para forçar loading do sistema de som.
    if (auto fx = CarregaSomUnico("nothing.wav"); fx != nullptr) {
      fx->play();
      LOG(INFO) << "Som forçado!!!";
    }
  }

  std::vector<std::string> sons = arq::ConteudoDiretorio(arq::TIPO_SOM);
  for (const std::string& som : sons) {
    g_fxs[som] = CarregaSomUnico(som);
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
