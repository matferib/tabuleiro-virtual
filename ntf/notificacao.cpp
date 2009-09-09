/** @file not/Notificacao.cpp implementacao das funcoes basicas de notificacao. */

#include "ntf/notificacao.h"

#include <istream>
#include <ostream>
#include <stdint.h>

using namespace ntf;

/*
Notificacao* Notificacao::nova(std::istream& stream) {
	uint8_t ts[4];
	stream.read((char*)ts, 4);
	tipontf_e tipoLido = static_cast<tipontf_e>(
			ts[0] | (ts[1] << 8) | (ts[2] << 16) | (ts[3] << 24)
	);

	Notificacao* ret = NULL;
	switch (tipoLido) {
	default:
		ret = new Notificacao(tipoLido);
	}
	ret->deserializa(stream);
	return ret;
} 
*/

Notificacao::Notificacao(tipontf_e tipo) : _tipo(tipo) {
}

Notificacao::~Notificacao(){
}

void Notificacao::serializa(std::ostream& stream) const {
	uint8_t ts[4];
 	ts[0] = static_cast<uint8_t>(_tipo & 0xFF);
 	ts[1] = static_cast<uint8_t>((_tipo & 0xFF00) >> 8);
 	ts[2] = static_cast<uint8_t>((_tipo & 0xFF0000) >> 16);
 	ts[3] = static_cast<uint8_t>((_tipo & 0xFF000000) >> 24);
	stream.write((char*)ts, 4);
}

void Notificacao::deserializa(std::istream& stream) { 
	// nao faz nada, tipo ja deserializado
}

