#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <stdexcept>

#include "arq/arquivo.h"
#include "log/log.h"

namespace arq {

namespace {
AAssetManager* g_aman = nullptr;
std::string g_dir_dados;
}  // namespace

void Inicializa(JNIEnv* env, jobject assets, const std::string& dir_dados) {
  g_aman = AAssetManager_fromJava(env, assets);
  g_dir_dados = dir_dados;
  interno::CriaDiretoriosUsuario();
}

namespace plat {
// Leitura.
void LeArquivoAsset(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  std::string caminho_asset(interno::CaminhoArquivo(tipo, nome_arquivo));
  AAsset* asset = nullptr;
  try {
    asset = AAssetManager_open(g_aman, caminho_asset.c_str(), AASSET_MODE_BUFFER);
    if (asset == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "falha abrindo asset: %s", caminho_asset.c_str());
      throw 1;
    }
    off_t tam = AAsset_getLength(asset);
    if (tam <= 0) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "falha com tamanho do asset: %ld", tam);
      throw 2;
    }
    __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "asset lido '%s', tamnho '%ld'", nome_arquivo.c_str(), tam);
    std::vector<char> vetor_dados;
    vetor_dados.resize(tam);
    memcpy(vetor_dados.data(), AAsset_getBuffer(asset), tam);
    dados->assign(vetor_dados.begin(), vetor_dados.end());
  } catch (...) {
    throw std::logic_error(std::string("Falha lendo asset: ") + nome_arquivo);
  }
  if (asset != nullptr) {
    AAsset_close(asset);
  }
}

const std::vector<std::string> ConteudoDiretorioAsset(tipo_e tipo) {
  std::vector<std::string> ret;
  std::string caminho_asset(Diretorio(tipo));
  AAssetDir* asset_dir = nullptr;
  try {
    asset_dir = AAssetManager_openDir(g_aman, caminho_asset.c_str());
    if (asset_dir == nullptr) {
      __android_log_print(ANDROID_LOG_ERROR, "Tabuleiro", "falha abrindo asset dir: %s", caminho_asset.c_str());
      throw 1;
    }
    for (const char* nome = AAssetDir_getNextFileName(asset_dir);
         nome != nullptr;
         nome = AAssetDir_getNextFileName(asset_dir)) {
      ret.push_back(nome);
    }
  } catch (...) {
  }
  if (asset_dir != nullptr) {
    AAssetDir_close(asset_dir);
  }
  return ret;
}

const std::string DiretorioAppsUsuario() {
  return g_dir_dados + "/";
}

const std::string DiretorioAssets() {
  return "";
}

}  // namespace plat.

}  // namespace arq
