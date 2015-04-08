#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <vector>

namespace net {

const std::string CodificaDados(const std::string& dados);

// Porta para aceitar conexoes http.
inline int PortaPadrao() { return 22113; }

// Porta de anuncio (broadcast) do endereco do servidor.
inline int PortaAnuncio() { return 22114; }

unsigned int DecodificaTamanho(const std::string::iterator& string);

// Conversao de numero para string independente de plataforma.
const std::string to_string(int numero);

}  // namespace net

#endif
