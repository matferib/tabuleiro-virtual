#ifndef ARQ_ARQUIVO_H
#define ARQ_ARQUIVO_H

#include <functional>
#include <string>
#include <vector>

#if ANDROID
#include <android/asset_manager_jni.h>
#endif

// As funcoes abaixo servem de wrapper sobre a camada de arquivos dos diversos dispositivos.
// Por exemplo, no Mac os arquivos devem ser escritos no diretorio do usuario e lidos do diretorio
// do bundle. No Linux le do diretorio da aplicacao. No windows tambem, mas deveria usar o registro.

namespace google {
namespace protobuf {
class Message;
}  // namespace protobuf
}  // namespace google


namespace arq {

class ParseProtoException : std::exception {
 public:
  ParseProtoException(const std::string& reason) : std::exception(), reason_(reason) {}
  const char* what() const noexcept override {
    return reason_.c_str();
  }
 private:
  std::string reason_;
};

enum tipo_e {
  TIPO_MODELOS_3D,
  TIPO_MODELOS_3D_BAIXADOS,
  TIPO_TEXTURA,
  TIPO_TEXTURA_BAIXADA,
  TIPO_TEXTURA_LOCAL,
  TIPO_DADOS,
  TIPO_CONFIGURACOES,
  TIPO_TABULEIRO,
  TIPO_TABULEIRO_ESTATICO,  // quando vem de assets.
  TIPO_ENTIDADES,
  TIPO_SHADER,
  TIPO_FONTES,
  TIPO_SOM,
  TIPO_TESTE
};

// Cria os diretorios locais.
#if ANDROID
void Inicializa(JNIEnv* env, jobject assets, const std::string& dir_dados);
void Inicializa(JNIEnv* env, AAssetManager *asset_manager, const std::string& dir_dados);
#else
// x_path: caminho do executavel.
void Inicializa(const std::string& x_path);
#endif

// Retorna conteudo de um diretorio. Se o filtro for passado, aqueles que retornarem true serao filtrados (removidos).
const std::vector<std::string> ConteudoDiretorio(tipo_e tipo, std::function<bool(const std::string&)> filtro = [] (const std::string&) { return false; });

// Retorna o diretorio do tipo passado, sem a "/" final.
const std::string Diretorio(tipo_e tipo);

// Interface de escrita.
// @throws std::logic_error caso nao consiga escrever arquivo.
void EscreveArquivo(tipo_e tipo, const std::string& nome_arquivo, const std::string& dados);
void EscreveArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem);
void EscreveArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, const google::protobuf::Message& mensagem);

// Interface de leitura.
// @throws ParseProtoException caso nao consiga ler o arquivo.
void LeArquivo(tipo_e tipo, const std::string& nome_arquivo, std::string* dados);
void LeArquivoAsciiProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem);
void LeArquivoBinProto(tipo_e tipo, const std::string& nome_arquivo, google::protobuf::Message* mensagem);

// Funcoes internas (de arq).
namespace interno {

// Converte um tipo para o diretorio correto.
const std::string TipoParaDiretorio(tipo_e tipo);
// Retorna true se o tipo de arquivo eh asset (ou seja, READ ONLY).
bool EhAsset(tipo_e tipo);
// Retorna o caminho para um tipo de arquivo.
const std::string CaminhoArquivo(tipo_e tipo, const std::string& arquivo);
// Cria a estrutura de diretorios para conteudo do usuario.
void CriaDiretoriosUsuario();
// Escreve um arquivo com caminho completo.
void EscreveArquivoNormal(const std::string& nome_arquivo, const std::string& dados);
// Le um arquivo dado o caminho completo.
void LeArquivoNormal(const std::string& nome_arquivo, std::string* dados);
// Lista o conteudo de um diretorio.
const std::vector<std::string> ConteudoDiretorioNormal(const std::string& diretorio);

}  // namespace interno.


}  // namespace arq

#endif
