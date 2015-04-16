#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <vector>

namespace net {

// Codifica tamanho da mensagem em 4 bytes e usa de prefixo.
const std::string CodificaDados(const std::string& dados);
// Decodifica tamanho (lendo 4 bytes do iterador).
unsigned int DecodificaTamanho(const std::string::iterator& string);

// Porta para aceitar conexoes http.
//inline int PortaPadrao() { return 22113; }
inline int PortaPadrao() { return 11223; }

// Porta de anuncio (broadcast) do endereco do servidor.
//inline int PortaAnuncio() { return 22114; }
inline int PortaAnuncio() { return 11224; }


// Conversao de numero para string independente de plataforma.
const std::string to_string(int numero);

}  // namespace net

#endif
