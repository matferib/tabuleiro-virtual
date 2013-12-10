#include <stdexcept>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"

using namespace ent;
using namespace std;

Entidade::Entidade(int id, int pontos_vida, double x, double y, double z) 
    : id_(id), maximo_pontos_vida_(pontos_vida), pontos_vida_(pontos_vida) {
	MovePara(x, y, z);
}

Entidade::~Entidade() {}

int Entidade::Id() const { return id_; }

void Entidade::MovePara(double x, double y, double z) { 
	x_= x; y_ = y; z_ = z; 
}

void Entidade::Move(double x, double y, double z) { 
	x_ += x; y_+= y; z_ += z; 
}

int Entidade::PontosVida() const {
	return pontos_vida_;
}

void Entidade::DanoCura(int pontos_vida) {
	pontos_vida_ += pontos_vida;
	if (pontos_vida_ > maximo_pontos_vida_) {
		pontos_vida_ = maximo_pontos_vida_;
	}
}

double Entidade::X() const { 
	return x_; 
}
double Entidade::Y() const { 
	return y_; 
}
double Entidade::Z() const { 
	return z_; 
}
