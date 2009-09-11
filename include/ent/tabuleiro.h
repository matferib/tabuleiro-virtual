#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include "ent/entidade.h"

namespace ent {

	class DadosTabuleiro;

	/** responsavel pelo terreno de jogo. */
	class Tabuleiro {
	public:
		/** cria o tabuleiro com o tamanho de grid passado (tamanho x tamanho). 
		* O identificador do tabuleiro sempre eh 0. 
		*/
		explicit Tabuleiro(int tamanho);
		~Tabuleiro();

		/** seleciona um quadrado do tabuleiro. 
		* @param id do quadrado no desenho.
		*/
		void selecionaQuadrado(int id);

		/** adiciona a entidade ao quadrado passado.
		* @param id do quadrado no desenho.
		*/
		void adicionaEntidade(Entidade* ent, int id);

		/** desenha o tabuleiro apenas. */
		void desenha(const ParametrosDesenho& pd);

		/** desenha as entidades no tabuleiro. */
		void desenhaEntidades(const ParametrosDesenho& pd);

	private:
		/** dados do tabuleiro. */
		DadosTabuleiro* dt_;
	};
}

#endif
