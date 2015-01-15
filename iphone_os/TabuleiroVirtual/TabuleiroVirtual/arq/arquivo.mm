#include <fstream>
#include <stdexcept>
#include "arq/arquivo.h"
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
#if !ANDROID
    // Android nao tem textura local.
    case TIPO_TEXTURA_LOCAL: return "texturas_locais";
#endif
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

}  // namespace

// Escrita.
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

}  // namespace arq
