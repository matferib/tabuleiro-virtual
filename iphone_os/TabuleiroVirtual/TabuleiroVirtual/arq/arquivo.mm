#include <boost/filesystem.hpp>
#include <fstream>
#include <stdexcept>
#include "arq/arquivo.h"
#include "log/log.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

#if __APPLE__
#include "TargetConditionals.h"
#endif

#import <Foundation/Foundation.h>

namespace arq {

namespace {

// Converte um tipo para o diretorio correto.
const std::string TipoParaDiretorio(tipo_e tipo) {
  switch (tipo) {
    case TIPO_TEXTURA: return "texturas";
    case TIPO_TEXTURA_LOCAL: return "texturas_locais";
    case TIPO_TABULEIRO: return "tabuleiros_salvos";
    case TIPO_DADOS: return "dados";
    case TIPO_SHADER: return "shaders";
    default:
      throw std::logic_error("Tipo de arquivo invalido.");
  }
}

// Retorna o caminho para um tipo de arquivo.
const std::string CaminhoArquivo(tipo_e tipo, const std::string& arquivo) {
  std::string diretorio(TipoParaDiretorio(tipo));
  return diretorio + "/" + arquivo;
}

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

// Retorna true se o tipo de arquivo eh asset (ou seja, READ ONLY).
bool EhAsset(tipo_e tipo) {
  return tipo != TIPO_TEXTURA_BAIXADA;
}

}  // namespace

void Inicializa() {
  CriaDiretoriosUsuario();
}

// Listagem.
const std::vector<std::string> ConteudoDiretorio(tipo_e tipo) {
  return std::vector<std::string>();
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
  std::string caminho_arquivo = [[[NSBundle mainBundle] resourcePath] cStringUsingEncoding : NSASCIIStringEncoding];
  caminho_arquivo += "/" + CaminhoArquivo(tipo, nome_arquivo);
  std::ifstream arquivo(caminho_arquivo, std::ios::in | std::ios::binary);
  dados->assign(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
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

namespace {
const std::string DiretorioAppsUsuario() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *base_path = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
  return base_path != nil ? [base_path UTF8String] : "";
}
}  // namespace

}  // namespace arq
