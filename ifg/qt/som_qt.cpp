#include "som/som.h"

#include <QtCore/QFileInfo>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QSoundEffect>
#include <QtCore/QThread>
#include <QtCore/QUrl>

#include <list>
#include <memory>

#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "arq/arquivo.h"
#include "log/log.h"

namespace som {
namespace {

std::unordered_map<std::string, std::unique_ptr<QSoundEffect>> g_fxs;
const ent::OpcoesProto* g_opcoes = nullptr;
std::unique_ptr<QMediaPlayer> g_mediaplayer;
std::unique_ptr<QAudioOutput> g_audio_output;

QUrl QUrlSom(const std::string& nome) {
  QString qs = QFileInfo(
      QString::fromStdString(absl::StrFormat("%s/%s",
                                             arq::Diretorio(arq::TIPO_SOM).c_str(),
                                             nome.c_str()))).absoluteFilePath();
  return QUrl::fromLocalFile(qs);
}

std::unique_ptr<QSoundEffect> CarregaSomUnico(const std::string& nome) {
  auto fx = std::make_unique<QSoundEffect>();
  fx->setSource(QUrlSom(nome));
  fx->setLoopCount(1);
  return fx;
}

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  g_audio_output = std::make_unique<QAudioOutput>();
  g_mediaplayer = std::make_unique<QMediaPlayer>();
  g_mediaplayer->setAudioOutput(g_audio_output.get());
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
    if (absl::EndsWith(som, ".wav")) {
      g_fxs[som] = CarregaSomUnico(som);
    }
  }
  g_opcoes = &opcoes;
}

void Finaliza() {
  g_fxs.clear();
  g_mediaplayer.reset();
  g_audio_output.reset();
  g_opcoes = nullptr;
}

void Toca(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  if (auto it = g_fxs.find(nome); it != g_fxs.end()) {
    it->second->play();
  } else {
    g_mediaplayer->setSource(QUrlSom(nome));
    g_mediaplayer->play();
  }
}

void TocaSomFundo(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  if (auto it = g_fxs.find(nome); it != g_fxs.end()) {
    VLOG(1) << "tocando wav " << nome;
    it->second->setLoopCount(QSoundEffect::Infinite);
    it->second->play();
  } else {
    VLOG(1) << "tocando ogg " << nome;
    g_mediaplayer->setSource(QUrlSom(nome));
    g_mediaplayer->setLoops(QMediaPlayer::Infinite);
    g_mediaplayer->play();
  }
}

void ParaSomFundo(const std::string& nome) {
  if (g_opcoes->desativar_som()) return;
  if (auto it = g_fxs.find(nome); it != g_fxs.end()) {
    VLOG(1) << "parando wav " << nome;
    it->second->setLoopCount(1);
    it->second->stop();
  } else {
    VLOG(1) << "parando ogg " << nome;
    g_mediaplayer->stop();
  }
}

}  // namespace som
