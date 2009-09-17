#include <stdexcept>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"

using namespace ent;
using namespace std;

Entidade::Entidade(int id, double x, double y, double z) : id_(id) {
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

double Entidade::x() const { 
	return x_; 
}
double Entidade::y() const { 
	return y_; 
}
double Entidade::z() const { 
	return z_; 
}
