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
std::unique_ptr<QSoundEffect> g_fx;
const ent::OpcoesProto* g_opcoes = nullptr;

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  g_media_player = std::make_unique<QMediaPlayer>();
  g_fx = std::make_unique<QSoundEffect>();
  g_opcoes = &opcoes;
}

void Finaliza() {
  g_media_player.reset();
  g_fx.reset();
  g_opcoes = nullptr;
}

void Toca(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  QString qs = QFileInfo(QString::fromStdString(StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), nome.c_str()))).absoluteFilePath();
  g_fx->setSource(QUrl::fromLocalFile(qs));
  g_fx->setLoopCount(1);
  //LOG(INFO) << "status: "  << fx.status();
  g_fx->play();
  //g_media_player->setMedia(QUrl::fromLocalFile(qs));
  //g_media_player->setVolume(100);
  //g_media_player->play();
  //VLOG(1)
  //    << "tentando tocar " << qs.toStdString() << " vol: " << g_media_player->volume() << ", error: " << static_cast<int>(g_media_player->error())
  //    << ", availability: " << static_cast<int>(g_media_player->availability()) << ", media status: " << (int)g_media_player->mediaStatus();
}

}  // namespace som
