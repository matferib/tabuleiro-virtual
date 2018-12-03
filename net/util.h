#ifndef NET_UTIL_H
#define NET_UTIL_H

#include <string>
#include <vector>
#if USAR_GFLAGS
#include <gflags/gflags.h>
#endif

namespace net {

// Codifica tamanho da mensagem em 4 bytes e usa de prefixo.
const std::string CodificaDados(const std::string& dados);
// Decodifica tamanho (lendo 4 bytes do iterador).
unsigned int DecodificaTamanho(const std::string::iterator& string);

// Porta para aceitar conexoes http.
int PortaPadrao();

// Porta de anuncio (broadcast) do endereco do servidor.
//inline int PortaAnuncio() { return 22114; }
inline int PortaAnuncio() { return 11224; }


// Conversao de numero para string independente de plataforma.
const std::string to_string(int numero);
inline const std::string to_string(unsigned int numero) { return to_string((int)numero); }
const std::string to_string(float numero);

}  // namespace net

#endif
