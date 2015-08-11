#include <cstdio>
#include "net/util.h"

namespace net {

const std::string CodificaDados(const std::string& dados) {
  unsigned int tam_dados = static_cast<unsigned int>(dados.size());
  std::string ret(4, '\0');
  ret[0] = static_cast<char>(tam_dados & 0xFFUL);
  ret[1] = static_cast<char>((tam_dados & 0xFF00UL) >> 8);
  ret[2] = static_cast<char>((tam_dados & 0xFF0000UL) >> 16);
  ret[3] = static_cast<char>((tam_dados & 0xFF000000UL) >> 24);
  ret.insert(ret.end(), dados.begin(), dados.end());
  return ret;
}

unsigned int DecodificaTamanho(const std::string::iterator& buffer) {
  // Type casts para as operacoes nao criarem 1s a esquerda.
  return static_cast<unsigned char>(*buffer) |
         (static_cast<unsigned char>(*(buffer + 1)) << 8) |
         (static_cast<unsigned char>(*(buffer + 2)) << 16) |
         (static_cast<unsigned char>(*(buffer + 3)) << 24);
}

const std::string to_string(int numero) {
#if WIN3o2 || ANDROID
  char buffer[51];
  snprintf(buffer, 51, "%d", numero);
  return std::string(buffer);
#else
  return std::to_string(numero);
#endif
}

}  // namespace net
