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
#else
  return "";
#endif
}

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

}  // namespace

#if ANDROID

namespace {
AAssetManager* g_aman = nullptr;
}  // namespace

void Inicializa(JNIEnv* env, jobject assets) {
  g_aman = AAssetManager_fromJava(env, assets);;
}

// Escrita.
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& dados) {
  throw std::logic_error(std::string("Não implementado"));
}

void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  throw std::logic_error(std::string("Não implementado"));
}

void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  throw std::logic_error(std::string("Não implementado"));
}

// Leitura.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
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

#else

void Inicializa() {
  // TODO cria os diretorios locais.
  std::string dir_apps_usuario(DiretorioAppsUsuario());
  try {
    boost::filesystem::create_directory(dir_apps_usuario);
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_BAIXADA));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_LOCAL));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TABULEIRO));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_ENTIDADES));
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha ao criar diretorio de usuario: " << e.what();
  }
}

const std::vector<std::string> ConteudoDiretorio(tipo_e tipo) {
  std::vector<std::string> ret;
  for (boost::filesystem::directory_iterator it(Diretorio(tipo)); it != boost::filesystem::directory_iterator(); ++it) {
    ret.push_back(it->path().filename().string());
  }
  return ret;
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

#endif

}  // namespace arq
