#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include "ent/entidade.h"

namespace ntf {
	class Notificacao;
}

namespace ent {

	class DadosTabuleiro;
	class DadosCriacao;

	enum tipoent_t {
		TIPOENT_MOVEL
	};

	/** singleton responsavel por tudo que eh do jogo. */
	class Tabuleiro {
	private:
		/** cria o tabuleiro com o tamanho de grid passado (tamanho x tamanho). 
		* O identificador do tabuleiro sempre eh 0. 
		*/
		explicit Tabuleiro(int tamanho);
	public:
		/** @return true se a instancia nao for NULL. */
		static bool haInstancia();

		/** @return a instancia unica do tabuleiro. */
		static Tabuleiro& instancia();

		/** destroi a instancia (se houver) e cria uma nova. */
		static void cria(int tamanho);

		/** libera os recursos do tabuleiro, inclusive entidades. */
		~Tabuleiro();

		/** @return tamanho do tabuleiro em X. */
		int tamanhoX() const;

		/** @return tamanho do tabuleiro em Y. */
		int tamanhoY() const;

		/** adiciona a entidade ao tabuleiro, no quadrado passado.
		* @param tipoEntidade tipo da entidade.
		* @param dc dados de criacao da entidade.
		* @param idQuadrado do quadrado no desenho.
		*/
		void adicionaEntidade(tipoent_t tipoEntidade, DadosCriacao* dc, int idQuadrado);

		/** remove entidade do tabuleiro, pelo id da entidade passado. 
		* @param id da entidade.
		*/
		void removeEntidade(int id);

		/** desenha o mundo. */
		void desenha(const ParametrosDesenho& pd);

		/** trata o resultado de um clique. Buffer no formato do openGL. */
		void trataClique(unsigned int numHits, unsigned int* bufferHits);

		/** trata a notificacao de inicio de jogo. */
		void trataNotificacao(const ntf::Notificacao& notificacao);

		/** le as coordenadas do objeto selecionado.
		* @return true se houver selecao. Se nao houver, nao altera x e y.
		*/
		bool coordenadasSelecao(double &x, double& y);

	private:
		/** dados do tabuleiro. */
		DadosTabuleiro* dt_;

		// elimina copia
		Tabuleiro(const Tabuleiro& t);
		Tabuleiro& operator=(Tabuleiro&);
	};
}

#endif
