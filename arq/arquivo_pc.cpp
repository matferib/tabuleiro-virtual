// Implementacao das funcoes especificas de plataforma para PC (mac windows linux).
#if __APPLE__
#include "TargetConditionals.h"
#endif

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <stdexcept>
#include "arq/arquivo.h"

#include "log/log.h"

namespace arq {

void Inicializa() {
  interno::CriaDiretoriosUsuario();
}

namespace plat {

const std::string DiretorioAppsUsuario() {
#if __APPLE__ && TARGET_OS_MAC
  std::string home(getenv("HOME"));
  if (home.empty()) {
    return "";
  }
  return home + "/Library/Application Support/TabuleiroVirtual/";
#elif WIN32
  std::string appdata(getenv("localappdata"));
  if (appdata.empty()) {
    return "";
  }
  return appdata + "/TabuleiroVirtual/";
#else
  return "./";
#endif
}

const std::string DiretorioAssets() {
  return "";
}

void LeArquivoAsset(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  std::string caminho_asset(interno::CaminhoArquivo(tipo, nome_arquivo));
  interno::LeArquivoNormal(caminho_asset, dados);
}

const std::vector<std::string> ConteudoDiretorioAsset(tipo_e tipo) {
  return interno::ConteudoDiretorioNormal(Diretorio(tipo));
}

}  // namespace plat.


} // namespace
