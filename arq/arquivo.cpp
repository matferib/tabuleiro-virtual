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
#include "goog/stringprintf.h"
#include "log/log.h"

namespace arq {

// ---------------------------------------------------------
// Declaracao de funcoes internas especificas de plataforma.
// ---------------------------------------------------------
namespace plat {

// Le arquivos readonly.
void LeArquivoAsset(tipo_e tipo, const std::string& nome_arquivo, std::string* dados);

// Diretorio de dados de aplicacao do usuario, incluindo / terminal.
const std::string DiretorioAppsUsuario();

// Diretorio que contem os assets da aplicacao, incluindo / terminal, se nao for vazio.
const std::string DiretorioAssets();

// Retorna o conteudo do tipo passado.
const std::vector<std::string> ConteudoDiretorioAsset(tipo_e tipo);

}  // namespace plat


// ------------------------
// Funcoes internas comums.
// ------------------------
namespace interno {

const std::string TipoParaDiretorio(tipo_e tipo) {
  switch (tipo) {
    case TIPO_MODELOS_3D: return "modelos3d";
    case TIPO_MODELOS_3D_BAIXADOS: return "modelos3d_baixados";
    case TIPO_TEXTURA: return "texturas";
    case TIPO_TEXTURA_BAIXADA: return "texturas_baixadas";
    case TIPO_TEXTURA_LOCAL: return "texturas_locais";
    case TIPO_TABULEIRO: return "tabuleiros_salvos";
    case TIPO_TABULEIRO_ESTATICO: return "tabuleiros_salvos";
    case TIPO_DADOS: return "dados";
    case TIPO_CONFIGURACOES: return "configuracoes";
    case TIPO_SHADER: return "shaders";
    case TIPO_ENTIDADES: return "entidades_salvas";
    case TIPO_FONTES: return "fontes";
    case TIPO_SOM: return "sons";
    case TIPO_TESTE: return "teste";
    default:
      throw std::logic_error("Tipo de arquivo invalido.");
  }
}

bool EhAsset(tipo_e tipo) {
  return tipo == TIPO_TABULEIRO_ESTATICO ||
         tipo == TIPO_TEXTURA ||
         tipo == TIPO_DADOS ||
         tipo == TIPO_SHADER ||
         tipo == TIPO_FONTES ||
         tipo == TIPO_SOM ||
         tipo == TIPO_MODELOS_3D;
}

const std::string CaminhoArquivo(tipo_e tipo, const std::string& arquivo) {
  return Diretorio(tipo) + "/" + arquivo;
}

void CriaDiretoriosUsuario() {
  std::string dir_apps_usuario(plat::DiretorioAppsUsuario());
  try {
    boost::filesystem::create_directory(dir_apps_usuario);
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_BAIXADA));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TEXTURA_LOCAL));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_MODELOS_3D_BAIXADOS));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_TABULEIRO));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_ENTIDADES));
    boost::filesystem::create_directory(dir_apps_usuario + "/" + TipoParaDiretorio(TIPO_CONFIGURACOES));
    LOG(INFO) << "Diretorios de usuario criados em " << dir_apps_usuario;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha ao criar diretorio de usuario '" << dir_apps_usuario << "': " << e.what();
    throw;
  }
}

void EscreveArquivoNormal(const std::string& nome_arquivo, const std::string& dados) {
  std::ofstream arquivo(nome_arquivo, std::ios::out | std::ios::binary);
  if (!arquivo) {
    throw std::logic_error(std::string("Arquivo invalido para escrita: ") + nome_arquivo);
  }
  arquivo.write(dados.data(), dados.size());
  arquivo.close();
}

void LeArquivoNormal(const std::string& nome_arquivo, std::string* dados) {
  std::ifstream arquivo(nome_arquivo, std::ios::in | std::ios::binary);
  if (!arquivo) {
    throw std::logic_error(std::string("Arquivo invalido: ") + nome_arquivo);
  }
  dados->assign(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
}

const std::vector<std::string> ConteudoDiretorioNormal(const std::string& diretorio) {
  std::vector<std::string> ret;
  for (boost::filesystem::directory_iterator it(diretorio); it != boost::filesystem::directory_iterator(); ++it) {
    ret.push_back(it->path().filename().string());
  }
  return ret;
}

}  // namespace interno

// ---------------------
// Funcoes da interface.
// ---------------------

const std::string Diretorio(tipo_e tipo) {
  if (interno::EhAsset(tipo)) {
    return plat::DiretorioAssets() + interno::TipoParaDiretorio(tipo);
  } else {
    return plat::DiretorioAppsUsuario() + interno::TipoParaDiretorio(tipo);
  }
}

const std::vector<std::string> ConteudoDiretorio(tipo_e tipo, std::function<bool(const std::string&)> filtro) {
  std::vector<std::string> ret;
  if (interno::EhAsset(tipo)) {
    ret = plat::ConteudoDiretorioAsset(tipo);
  } else {
    ret = interno::ConteudoDiretorioNormal(Diretorio(tipo));
  }
  ret.erase(std::remove_if(ret.begin(), ret.end(), filtro), ret.end());
  return ret;
}

// Escrita: funciona para todas as plataformas, desde que a funcao caminho arquivo funcione.
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& dados) {
  if (interno::EhAsset(tipo)) {
    throw std::logic_error(std::string("Não implementado"));
  }
  std::string caminho_arquivo(interno::CaminhoArquivo(tipo, nome_arquivo));
  interno::EscreveArquivoNormal(caminho_arquivo, dados);
}

void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  // TODO: deveria ser TextFormat::PrintToString???
  EscreveArquivo(tipo, nome_arquivo, mensagem.DebugString());
}

void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  EscreveArquivo(tipo, nome_arquivo, mensagem.SerializeAsString());
}

// Leitura: a parte de assets eh especifica de plataforma. Caminho arquivo tem que funcionar tambem.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  if (interno::EhAsset(tipo)) {
    plat::LeArquivoAsset(tipo, nome_arquivo, dados);
  } else {
    interno::LeArquivoNormal(interno::CaminhoArquivo(tipo, nome_arquivo), dados);
  }
}

// Esse log handler imprime filename como o fonte que esta processando o arquivo e nao o arquivo sendo lido :(
void LogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message);

struct ScopedLogHandler {
 public:
  ScopedLogHandler(const std::string& nome_arquivo) {
    old = google::protobuf::SetLogHandler(&LogHandler);
    g_nome_arquivo = nome_arquivo;
  }
  ~ScopedLogHandler() {
    google::protobuf::SetLogHandler(old);
  }

  static std::string g_nome_arquivo;
  google::protobuf::LogHandler* old;
};
std::string ScopedLogHandler::g_nome_arquivo;

void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string dados;
  LeArquivo(tipo, nome_arquivo, &dados);

  ScopedLogHandler slh(nome_arquivo);
  google::protobuf::TextFormat::ParseFromString(dados, mensagem);
}

void LeArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string dados;
  LeArquivo(tipo, nome_arquivo, &dados);

  ScopedLogHandler slh(nome_arquivo);
  mensagem->ParseFromString(dados);
}

void LogHandler(google::protobuf::LogLevel level, const char* filename, int line, const std::string& message) {
  if (level >= google::protobuf::LOGLEVEL_ERROR) {
    LOG(ERROR) << "erro arquivo: " << ScopedLogHandler::g_nome_arquivo << ": " << message;
    throw ParseProtoException(google::protobuf::StringPrintf("%s", message.c_str()));
  } else if (level == google::protobuf::LOGLEVEL_WARNING) {
    LOG(WARNING) << "erro arquivo: " << ScopedLogHandler::g_nome_arquivo << ": " << message;
  } else if (level == google::protobuf::LOGLEVEL_INFO) {
    LOG(INFO) << "erro arquivo: " << ScopedLogHandler::g_nome_arquivo << ": " << message;
  }
}

}  // namespace arq
