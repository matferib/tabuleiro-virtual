#include <QFileInfo>
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

}  // namespace

void Inicia() {
  g_media_player = std::make_unique<QMediaPlayer>();
}

void Finaliza() {
  g_media_player.reset();
}

void Toca(const std::string& nome) {
  QString qs = QFileInfo(QString::fromStdString(s)).absoluteFilePath();
  g_media_player->setMedia(QUrl::fromLocalFile(qs));
  g_media_player->setVolume(100);
  g_media_player->play();
  VLOG(1)
      << "tentando tocar " << s << " vol: " << g_media_player->volume() << ", error: " << static_cast<int>(g_media_player->error())
      << ", availability: " << static_cast<int>(g_media_player->availability()) << ", media status: " << (int)g_media_player->mediaStatus();
}

}  // namespace som
