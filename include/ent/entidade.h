#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

namespace ent {

	/** gera um identificador unico para a entidade. */
	int geraId();

	/** parametros de desenho dos objetos. */
	class ParametrosDesenho {};

	/** classe base para entidades. Deve implementar as interfaces de desenho e clique.
	* Toda entidade devera possuir um identificador unico. 
	*/
	class Entidade : public ifg::gl::Desenhavel, public ifg::gl::Clicavel {
	protected:
		explicit Entidade(int id);

		/** @return o identificador da entidade que deve ser unico. */
		int id() const { return id_; }

		/** @return true se o objeto foi clicado. */
		virtual bool clicado(int x, int y) = 0;

		/** desenha o objeto, recebendo os parametros de desenho. */
		virtual void desenha(const ParametrosDesenho& pd) = 0;

	private:
		int id_;
	};

}

#endif

