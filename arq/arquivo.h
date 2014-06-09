#include <string>

// As funcoes abaixo servem de wrapper sobre a camada de arquivos dos diversos dispositivos.
// Por exemplo, no Mac os arquivos devem ser escritos no diretorio do usuario e lidos do diretorio
// do bundle. No Linux le do diretorio da aplicacao. No windows tambem, mas deveria usar o registro.

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google


namespace arq {

enum tipo_e {
  TIPO_TEXTURA,
  TIPO_TEXTURA_LOCAL,
  TIPO_DADOS,
  TIPO_TABULEIRO
};

// Escreve no arquivo.
void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem);
void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem);

// Le um arquivo do diretorio.
void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem);
void LeArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem);

}  // namespace arq
