#ifndef ENT_PARAMETROSDESENHO_H
#define ENT_PARAMETROSDESENHO_H

namespace ent {
	/** parametros de desenho, como cores, tamanho de traco etc... */
	class ParametrosDesenho {
	public:
		ParametrosDesenho();
		~ParametrosDesenho();

		double* corTabuleiroSelecionado() const;
		double* corTabuleiroNaoSelecionado() const;
		double* corEntidadeSelecionada() const;
		double* corEntidadeNaoSelecionada() const;

	private:
		class Dados;
		Dados* dpd_;
	};
}

#endif
