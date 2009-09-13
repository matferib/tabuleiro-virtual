#include <cstring>
#include "ent/parametrosdesenho.h"

using namespace ent;

class ParametrosDesenho::Dados {
public:
	Dados() {
		memset(this, 0, sizeof(this));
		corTabuleiroSelecionado_[0] = 1.0;
		corEntidadeNaoSelecionada_[1] = 1.0; // verde
		corEntidadeSelecionada_[0]    = 1.0; // vermelho
	}
	~Dados() {}

	double corTabuleiroSelecionado_[3];
	double corTabuleiroNaoSelecionado_[3];
	double corEntidadeSelecionada_[3];
	double corEntidadeNaoSelecionada_[3];
};

ParametrosDesenho::ParametrosDesenho() : dpd_(new Dados) { }

ParametrosDesenho::~ParametrosDesenho() { delete dpd_; }

double* ParametrosDesenho::corTabuleiroSelecionado() const {
	return dpd_->corTabuleiroSelecionado_; 
}

double* ParametrosDesenho::corTabuleiroNaoSelecionado() const {
	return dpd_->corTabuleiroNaoSelecionado_;
}

double* ParametrosDesenho::corEntidadeSelecionada() const {
	return dpd_->corEntidadeSelecionada_; 
}

double* ParametrosDesenho::corEntidadeNaoSelecionada() const {
	return dpd_->corEntidadeNaoSelecionada_;
}

