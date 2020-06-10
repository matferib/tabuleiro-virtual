#include <QSound>
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

class Gerenciador {
 public:
  ~Gerenciador() {
    while (!lista_.empty()) {
      lista_.front()->wait();
      lista_.pop_front();
    }
  }
  void Dispara(QThread* thread) {
    while (!lista_.empty() && lista_.front()->isFinished()) {
      lista_.pop_front();
    }
    thread->start();
    lista_.emplace_back(thread);
  }

 private:
  std::list<std::unique_ptr<QThread>> lista_;
};

void Handler(std::string s) {
  QSound::play(QString::fromStdString(s));
}

std::unique_ptr<Gerenciador> g_gerenciador;

}  // namespace

void Inicia() {
  g_gerenciador = std::make_unique<Gerenciador>();
}

void Finaliza() {
  g_gerenciador.reset();
}

void Toca(const std::string& nome) {
  VLOG(1) << "tocando: " << StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), nome.c_str());
  auto* ts = QThread::create(Handler, StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), nome.c_str()));
  g_gerenciador->Dispara(ts);
}

}  // namespace som
