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

#if ANDROID
#include <cstring>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/native_activity.h>
#endif

#include "log/log.h"

namespace arq {

namespace {

// Converte um tipo para o diretorio correto.
const std::string TipoParaDiretorio(tipo_e tipo) {
  switch (tipo) {
    case TIPO_TEXTURA: return "texturas";
    case TIPO_TEXTURA_BAIXADA: return "texturas_baixadas";
#if !ANDROID
    // Android nao tem textura local.
    case TIPO_TEXTURA_LOCAL: return "texturas_locais";
#endif
    case TIPO_TABULEIRO: return "tabuleiros_salvos";
    case TIPO_DADOS: return "dados";
    case TIPO_SHADER: return "shaders";
    case TIPO_ENTIDADES: return "entidades_salvas";
    default:
      throw std::logic_error("Tipo de arquivo invalido.");
  }
}

// Diretorio de dados de aplicacao do usuario, incluindo / terminal se houver.
const std::string DiretorioAppsUsuario();

// Retorna o diretorio do tipo passado, sem a "/" final.
const std::string Diretorio(tipo_e tipo) {
  std::string diretorio;
  // TODO fazer isso com tabuleiros salvos e entidades salvas.
  if ((tipo == TIPO_TEXTURA_BAIXADA) ||
      (tipo == TIPO_TEXTURA_LOCAL)) {
    diretorio.assign(DiretorioAppsUsuario());
  }
  return diretorio + TipoParaDiretorio(tipo);
}

// Retorna o caminho para um tipo de arquivo.
const std::string CaminhoArquivo(tipo_e tipo, const std::string& arquivo) {
  return Diretorio(tipo) + "/" + arquivo;
}

// Cria a estrutura de diretorios para conteudo do usuario.
void CriaDiretoriosUsuario() {
  std::string dir_apps_usuario(DiretorioAppsUsuario());
  try {
    boost::filesystem::create_directory(dir_apps_usuario);
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_BAIXADA));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_LOCAL));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TABULEIRO));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_ENTIDADES));
    LOG(INFO) << "Diretorios de usuario criados em " << dir_apps_usuario;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha ao criar diretorio de usuario: " << e.what();
  }
}

}  // namespace

#if ANDROID

namespace {
AAssetManager* g_aman = nullptr;
std::string g_dir_dados;
}  // namespace

void Inicializa(JNIEnv* env, jobject assets, const std::string& dir_dados) {
  g_aman = AAssetManager_fromJava(env, assets);
  g_dir_dados = dir_dados;
  CriaDiretoriosUsuario();
}

// Retorna true se o tipo de arquivo eh asset (ou seja, READ ONLY).
bool EhAsset(tipo_e tipo) {
  return tipo != TIPO_TEXTURA_BAIXADA;
}

// Escrita.
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& dados) {
  if (EhAsset(tipo)) {
    throw std::logic_error(std::string("Não implementado"));
  }
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  arquivo.write(dados.data(), dados.size());
  arquivo.close();
}

void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  EscreveArquivo(tipo, nome_arquivo, mensagem.DebugString());
}

void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  EscreveArquivo(tipo, nome_arquivo, mensagem.SerializeAsString());
}

// Leitura.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  if (EhAsset(tipo)) {
    std::string caminho_asset(CaminhoArquivo(tipo, nome_arquivo));
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
    }
    if (asset != nullptr) {
      AAsset_close(asset);
    }
  } else {
    std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
    std::ifstream arquivo(caminho_arquivo, std::ios::in | std::ios::binary);
    dados->assign(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
  }
}

void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string dados;
  LeArquivo(tipo, nome_arquivo, &dados);
  if (!google::protobuf::TextFormat::ParseFromString(dados, mensagem)) {
    throw std::logic_error(std::string("Erro de parse do arquivo ") + nome_arquivo);
  }
}

void LeArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string dados;
  LeArquivo(tipo, nome_arquivo, &dados);
  if (!mensagem->ParseFromString(dados)) {
    throw std::logic_error(std::string("Erro de parse do arquivo ") + nome_arquivo);
  }
}

const std::vector<std::string> ConteudoDiretorio(tipo_e tipo) {
  std::vector<std::string> ret;
  if (EhAsset(tipo)) {
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
  } else {
    for (boost::filesystem::directory_iterator it(Diretorio(tipo)); it != boost::filesystem::directory_iterator(); ++it) {
      ret.push_back(it->path().filename().string());
    }
  }
  return ret;
}

#else

void Inicializa() {
  CriaDiretoriosUsuario();
}

// Escrita.
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& dados) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  arquivo.write(dados.data(), dados.size());
  arquivo.close();
}

void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  google::protobuf::io::OstreamOutputStream zos(&arquivo);
  if (!google::protobuf::TextFormat::Print(mensagem, &zos)) {
    throw std::logic_error(std::string("Erro escrevendo arquivo: ") + caminho_arquivo);
  }
}

void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  if (!mensagem.SerializeToOstream(&arquivo)) {
    throw std::logic_error(std::string("Erro escrevendo arquivo: ") + caminho_arquivo);
  }
}

// Leitura.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  //LOG(INFO) << "Lendo: " << caminho_arquivo;
  std::ifstream arquivo(caminho_arquivo, std::ios::in | std::ios::binary);
  dados->assign(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
}

void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ifstream arquivo(caminho_arquivo);
  google::protobuf::io::IstreamInputStream zis(&arquivo);
  if (!google::protobuf::TextFormat::Parse(&zis, mensagem)) {
    throw std::logic_error(std::string("Erro lendo arquivo: ") + caminho_arquivo);
  }
}

void LeArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ifstream arquivo(caminho_arquivo, std::ios::in | std::ios::binary);
  if (!mensagem->ParseFromIstream(&arquivo)) {
    throw std::logic_error(std::string("Erro lendo arquivo: ") + caminho_arquivo);
  }
}

const std::vector<std::string> ConteudoDiretorio(tipo_e tipo) {
  std::vector<std::string> ret;
  for (boost::filesystem::directory_iterator it(Diretorio(tipo)); it != boost::filesystem::directory_iterator(); ++it) {
    ret.push_back(it->path().filename().string());
  }
  return ret;
}

#endif

namespace {
const std::string DiretorioAppsUsuario() {
  // TODO.
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
#elif ANDROID
  return g_dir_dados + "/";
#else
  return "";
#endif
}
}  // namespace

}  // namespace arq
