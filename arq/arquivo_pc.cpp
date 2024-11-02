// Implementacao das funcoes especificas de plataforma para PC (mac windows linux).
#if __APPLE__
#include "TargetConditionals.h"
#endif

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <stdexcept>
#if WIN32
#include <codecvt>
#include <stdio.h>
#include <shlobj.h>
#include <objbase.h>
#include <locale>
#endif

#include "arq/arquivo.h"

#include "log/log.h"

namespace arq {

// Caminho do executavel.
std::string g_x_path;

void Inicializa(const std::string& x_path) {
  interno::CriaDiretoriosUsuario();
  g_x_path = x_path;
}

namespace plat {

const std::string DiretorioAppsUsuario() {
#if __APPLE__ && TARGET_OS_MAC
  auto* home_ptr = getenv("HOME");
  if (home_ptr == nullptr || strlen(home_ptr) == 0) {
    return "./";
  }
  std::string home(home_ptr);
  return home + "/Library/Application Support/TabuleiroVirtual/";
#elif WIN32
  PWSTR path = nullptr;
  HRESULT r = SHGetKnownFolderPath(FOLDERID_SavedGames, KF_FLAG_CREATE, NULL, &path);
  if (path != nullptr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::string path_str = converter.to_bytes(path);
    CoTaskMemFree(path);
    return path_str + "/TabuleiroVirtual/";
  }
	// Fallback para localappdata.
  std::string appdata(getenv("localappdata"));
  if (appdata.empty()) {
    return "";
  }
	// Fallback para diretorio local.
  return appdata + "/TabuleiroVirtual/";
#else
  std::string home;
  auto* xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home == nullptr || strlen(xdg_config_home) == 0) {
    auto* home_ptr = getenv("HOME");
    if (home_ptr == nullptr || strlen(home_ptr) == 0) {
      return "./";
    }
    home = std::string(home_ptr) + "/.config";
  } else {
    home = xdg_config_home;
  }
  return home + "/TabuleiroVirtual/";
#endif
}

bool RodandoDeBundle() {
#if __APPLE__ && TARGET_OS_MAC
  return g_x_path.find("Contents/MacOS") != std::string::npos;
#endif
  return false;
}

const std::string DiretorioAssets() {
#if __APPLE__ && TARGET_OS_MAC
  return RodandoDeBundle() ? g_x_path + "/../Resources/" : "";
#endif
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
