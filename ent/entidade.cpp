#include <stdexcept>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"

using namespace ent;
using namespace std;

Entidade::Entidade(int id, int pontosVida, double x, double y, double z) : id_(id), maximoPontosVida_(pontosVida), pontosVida_(pontosVida) {
	movePara(x, y, z);
}

Entidade::~Entidade() {}

int Entidade::id() const { return id_; }

void Entidade::movePara(double x, double y, double z) { 
	x_= x; y_ = y; z_ = z; 
}

void Entidade::move(double x, double y, double z) { 
	x_ += x; y_+= y; z_ += z; 
}

int Entidade::pontosVida() const {
	return pontosVida_;
}

void Entidade::danoCura(int pontosVida) {
	pontosVida_ += pontosVida;
	if (pontosVida_ > maximoPontosVida_) {
		pontosVida_ = maximoPontosVida_;
	}
}

double Entidade::x() const { 
	return x_; 
}
double Entidade::y() const { 
	return y_; 
}
double Entidade::z() const { 
	return z_; 
}
