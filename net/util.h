#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <vector>

namespace net {

const std::vector<char> CodificaDados(const std::string& dados);

unsigned int DecodificaTamanho(const std::vector<char>::iterator& buffer);

}  // namespace net

#endif
