#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include "ent/entidade.h"

namespace ent {

	/** responsavel pelo terreno de jogo. */
	class Tabuleiro : public Entidade {
	public:
		/** cria o tabuleiro com o tamanho de grid passado (tamanho x tamanho). 
		* O identificador do tabuleiro sempre eh 0. 
		*/
		explicit Tabuleiro(int tamanho);

		// implementacao da interface virtual 
		void desenha(const ParametrosDesenho* pd);
		void clicado(int x, int y);
	};
}

#endif
