#include <fstream>
#include <stdexcept>
#include "arq/arquivo.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>

namespace arq {

namespace {

// Converte um tipo para o diretorio correto.
const std::string TipoParaDiretorio(tipo_e tipo) {
  switch (tipo) {
    case TIPO_TEXTURA: return "texturas";
    case TIPO_TEXTURA_LOCAL: return "texturas_locais";
    case TIPO_TABULEIRO: return "tabuleiros_salvos";
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
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& valor) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  if (!arquivo) {
    throw std::logic_error(std::string("Erro escrevendo arquivo: ") + caminho_arquivo);
  }
  arquivo.write(valor.c_str(), valor.size());
}

void EscreveAsciiArquivo(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ofstream arquivo(caminho_arquivo, std::ios::out | std::ios::binary);
  google::protobuf::io::OstreamOutputStream zos(&arquivo);
  if (!google::protobuf::TextFormat::Print(mensagem, &zos)) {
    throw std::logic_error(std::string("Erro escrevendo arquivo: ") + caminho_arquivo);
  }
}

void EscreveBinArquivo(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem) {
  std::string mensagem_string(mensagem.SerializeAsString());
  EscreveArquivo(tipo, nome_arquivo, mensagem_string);
}

// Leitura.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* valor) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ifstream arquivo(caminho_arquivo, std::ios::in | std::ios::binary);
  if (!arquivo) {
    throw std::logic_error(std::string("Erro lendo arquivo: ") + caminho_arquivo);
  }
  arquivo.seekg(0, std::ios::end);
  valor->resize(arquivo.tellg());
  arquivo.seekg(0, std::ios::beg);
  arquivo.read(&(*valor)[0], valor->size());
}

void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem) {
  std::string caminho_arquivo(CaminhoArquivo(tipo, nome_arquivo));
  std::ifstream arquivo(nome_arquivo);
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

}  // namespace arq
