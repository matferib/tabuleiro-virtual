#include <cstring>
#include "ent/parametrosdesenho.h"

using namespace ent;

class ParametrosDesenho::Dados {
public:
	Dados() {
		memset(this, 0, sizeof(this));
		corTabuleiroSelecionado_[0] = 1.0;
		corObjetoNaoSelecionado_[1] = 1.0;
	}
	~Dados() {}

	double corTabuleiroSelecionado_[3];
	double corTabuleiroNaoSelecionado_[3];
	double corObjetoSelecionado_[3];
	double corObjetoNaoSelecionado_[3];
};

ParametrosDesenho::ParametrosDesenho() : dpd_(new Dados) { }

ParametrosDesenho::~ParametrosDesenho() { delete dpd_; }

double* ParametrosDesenho::corTabuleiroSelecionado() const {
	return dpd_->corTabuleiroSelecionado_; 
}

double* ParametrosDesenho::corTabuleiroNaoSelecionado() const {
	return dpd_->corTabuleiroNaoSelecionado_;
}

double* ParametrosDesenho::corObjetoSelecionado() const {
	return dpd_->corObjetoSelecionado_; 
}

double* ParametrosDesenho::corObjetoNaoSelecionado() const {
	return dpd_->corObjetoNaoSelecionado_;
}

