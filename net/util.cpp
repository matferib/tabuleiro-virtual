#include <cstdio>
#include "net/util.h"

namespace net {

const std::vector<char> CodificaDados(const std::string& dados) {
  unsigned int tam_dados = static_cast<unsigned int>(dados.size());
  std::vector<char> ret(4);
  ret[0] = static_cast<char>(tam_dados & 0xFFUL);
  ret[1] = static_cast<char>((tam_dados & 0xFF00UL) >> 8);
  ret[2] = static_cast<char>((tam_dados & 0xFF0000UL) >> 16);
  ret[3] = static_cast<char>((tam_dados & 0xFF000000UL) >> 24);
  ret.insert(ret.end(), dados.begin(), dados.end());
  return ret;
}

unsigned int DecodificaTamanho(const std::vector<char>::iterator& buffer) {
  const auto& ubuffer = reinterpret_cast<const std::vector<unsigned char>::iterator&>(buffer);
  return ubuffer[0] | (ubuffer[1] << 8) | (ubuffer[2] << 16) | (ubuffer[3] << 24);
}

const std::string to_string(int numero) {
#if WIN32
  char buffer[51];
  snprintf(buffer, 51, "%d", numero);
  return std::string(buffer);
#else
  return std::to_string(numero);
#endif
}

}  // namespace net
