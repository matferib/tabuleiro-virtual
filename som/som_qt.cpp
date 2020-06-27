#include <QFileInfo>
#include <QSoundEffect>
#include <QtMultimedia/QMediaPlayer>
#include <QThread>
#include <list>
#include <memory>
#include "arq/arquivo.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "som/som.h"

using google::protobuf::StringPrintf;

namespace som {
namespace {

std::unique_ptr<QMediaPlayer> g_media_player;
std::unordered_map<std::string, std::unique_ptr<QSoundEffect>> g_fxs;
const ent::OpcoesProto* g_opcoes = nullptr;

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  std::vector<std::string> sons = arq::ConteudoDiretorio(arq::TIPO_SOM);
  for (const std::string& som : sons) {
    auto fx = std::make_unique<QSoundEffect>();
    QString qs = QFileInfo(QString::fromStdString(StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), som.c_str()))).absoluteFilePath();
    fx->setSource(QUrl::fromLocalFile(qs));
    fx->setLoopCount(1);
    g_fxs[som] = std::move(fx);
  }
  g_media_player = std::make_unique<QMediaPlayer>();
  g_opcoes = &opcoes;
}

void Finaliza() {
  g_media_player.reset();
  g_fxs.clear();
  g_opcoes = nullptr;
}

void Toca(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  if (auto it = g_fxs.find(nome); it == g_fxs.end()) return;
  else it->second->play();
}

}  // namespace som
