#include <stdexcept>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/movel.h"

using namespace ent;
using namespace std;

namespace {
	// o id 0 eh do tabuleiro
	int proximoId = 0;
}

Entidade* ent::novaEntidade(tipoent_t tipo) {
	switch (tipo) {
		case TIPOENT_MOVEL:
			return new Movel(proximoId++);
		break;
		default:
			throw logic_error("tipo invalido de entidade");
	}
}

