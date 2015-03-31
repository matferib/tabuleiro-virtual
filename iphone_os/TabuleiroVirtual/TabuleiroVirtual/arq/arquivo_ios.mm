// Implementacao das funcoes especificas de plataforma para IOS.
#include <boost/filesystem.hpp>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <stdexcept>
#include "arq/arquivo.h"

#include "log/log.h"

// Por ultimo para nao dar conflito TYPE_BOOL.
#import <Foundation/Foundation.h>
#include <TargetConditionals.h>


namespace arq {

void Inicializa() {
  interno::CriaDiretoriosUsuario();
}

namespace plat {

const std::string DiretorioAppsUsuario() {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
  NSString *base_path = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;
  return base_path != nil ? std::string([base_path UTF8String]) + "/" : "";
}

const std::string DiretorioAssets() {
  return std::string([[[NSBundle mainBundle] resourcePath] UTF8String]) + "/";
}

void LeArquivoAsset(tipo_e tipo, const std::string& nome_arquivo, std::string* dados) {
  //LOG(INFO) << "Lendo asset: " << interno::CaminhoArquivo(tipo, nome_arquivo);
  std::ifstream arquivo(interno::CaminhoArquivo(tipo, nome_arquivo), std::ios::in | std::ios::binary);
  if (!arquivo) {
    throw std::logic_error(std::string("Falha lendo asset: ") + nome_arquivo);
  }
  dados->assign(std::istreambuf_iterator<char>(arquivo), std::istreambuf_iterator<char>());
}

const std::vector<std::string> ConteudoDiretorioAsset(tipo_e tipo) {
  return interno::ConteudoDiretorioNormal(Diretorio(tipo));
}

}  // namespace plat.

}  // namespace arq
