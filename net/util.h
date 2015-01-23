#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <vector>

namespace net {

const std::vector<char> CodificaDados(const std::string& dados);

// Porta para aceitar conexoes http.
inline int PortaPadrao() { return 11223; }

// Porta de anuncio (broadcast) do endereco do servidor.
inline int PortaAnuncio() { return 11224; }

unsigned int DecodificaTamanho(const std::vector<char>::iterator& buffer);

// Conversao de numero para string independente de plataforma.
const std::string to_string(int numero);

}  // namespace net

#endif
