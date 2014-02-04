#include "net/util.h"

namespace net {

const std::vector<char> CodificaDados(const std::string& dados) {
  size_t tam_dados = dados.size();
  std::vector<char> ret(4);
  ret[0] = static_cast<char>(tam_dados & 0xFF);
  ret[1] = static_cast<char>((tam_dados & 0xFF00) >> 8);
  ret[2] = static_cast<char>((tam_dados & 0xFF0000) >> 16);
  ret[3] = static_cast<char>((tam_dados & 0xFF000000) >> 24);
  ret.insert(ret.end(), dados.begin(), dados.end());
  return ret;
}

int DecodificaTamanho(const std::vector<char>& buffer) {
  return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

}  // namespace net
