#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

namespace ent {

	class Entidade;

	/** tipos de entidade que podem ser construidas. */
	enum tipoent_t {
		TIPOENT_MOVEL
	};	

	/** gera um identificador unico para a entidade. */
	Entidade* novaEntidade(tipoent_t tipo);

	/** parametros de desenho dos objetos. */
	class ParametrosDesenho;

	/** classe base para entidades.
	* Toda entidade devera possuir um identificador unico. 
	*/
	class Entidade {
	protected:
		explicit Entidade(int id) { id_ = id; }
	public:
		virtual ~Entidade(){}

	public:
		/** @return o identificador da entidade que deve ser unico. */
		int id() const { return id_; }

		/** seleciona o objeto.
		* @param valor da selecao.
		*/
		void seleciona(bool valor) { selecionado_ = valor; }

		/** @return true se o objeto estiver selecionado. */
		bool selecionado() const { return selecionado_; }

		/** desenha o objeto, recebendo os parametros de desenho. */
		virtual void desenha(const ParametrosDesenho& pd) = 0;

	private:
		int id_;
		bool selecionado_;
	};

}

#endif

