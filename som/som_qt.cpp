#include <QSound>
#include "arq/arquivo.h"
#include "goog/stringprintf.h"
#include "log/log.h"
#include "som/som.h"

using google::protobuf::StringPrintf;

namespace som {

void Toca(const std::string& nome) {
  LOG(INFO) << "tocando: " << StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), nome.c_str());
  QSound::play(QString::fromStdString(StringPrintf("%s/%s", arq::Diretorio(arq::TIPO_SOM).c_str(), nome.c_str())));
}

}  // namespace som
