#include <vector>
#include <stdexcept>
#include <GL/gl.h>
#include "ent/tabuleiro.h"

using namespace ent;
using namespace std;

namespace {

	/** representa um quadrado no tabuleiro. Cada quadrado tem lado 1,5m. */
	class Quadrado {
	public:
		/** tamanho do lado do quadrado no 3D. */
		static const double TAMANHO_GL = 1.0;

		/** desenha o quadrado. */
		void desenha(const ParametrosDesenho& pd, GLuint id) {
			glLoadName(id);
			glColor3f(selecionado_ ? 1.0 : 0.0, 0, 0);
			glRectf(0.2, 0.2, (TAMANHO_GL-0.2), (TAMANHO_GL-0.2));
		}

		/** seleciona ou deseleciona o quadrado. */
		void seleciona(bool val) { selecionado_ = val; }

	private:
		/** indica que o quadrado esta selecionado. */
		bool selecionado_;
	};

}

namespace ent {
	class DadosTabuleiro {
	public:
		explicit DadosTabuleiro(int tamanho) : tamanho_(tamanho) {
			quadrados_ = new vector<Quadrado>[tamanho];
			for (int i = 0; i < tamanho; ++i) {
				quadrados_[i].resize(tamanho);
			}
			selecionado_ = NULL;
		}

		~DadosTabuleiro() {
			delete[] quadrados_;
		}

		inline int tamanhoX() const { return tamanho_; }
		inline int tamanhoY() const { return tamanho_; }
		inline vector<Quadrado>& quadrados(int y) { return quadrados_[y]; }
		// selecao de quadrados
		bool haQuadradoSelecionado() const { return selecionado_ != NULL; }
		void deselecionaQuadrado() { selecionado_ = NULL; }
		void selecionaQuadrado(Quadrado& q) { 
			if (selecionado_ != NULL) { selecionado_->seleciona(false); }
			selecionado_ = &q; 
			selecionado_->seleciona(true);
		}
		Quadrado& quadradoSelecionado() { 
			if (selecionado_ == NULL) {
				throw logic_error("nao ha quadrado selecionado");
			}
		}

	private:
		/** tamanho do tabuleiro (tamanho_ x tamanho_). */
		int tamanho_;

		/** quadrados */
		vector<Quadrado>* quadrados_;

		/** quadrado selecionado. */
		Quadrado* selecionado_;
	};
}

Tabuleiro::Tabuleiro(int tamanho) : Entidade(0), dt_(new DadosTabuleiro(tamanho)) {
}

Tabuleiro::~Tabuleiro() {
	delete dt_;
}

void Tabuleiro::desenha(const ParametrosDesenho& pd) {
	glPushMatrix();
	// desenha de baixo pra cima
	double deltaX = -dt_->tamanhoX() * Quadrado::TAMANHO_GL;
	double deltaY = -dt_->tamanhoY() * Quadrado::TAMANHO_GL;
	glTranslatef(deltaX / 2.0, deltaY / 2.0, 0);

	GLuint id = 0;
	for (int y = 0; y < dt_->tamanhoY(); ++y) {
		vector<Quadrado>& vq = dt_->quadrados(y);
		for (vector<Quadrado>::iterator itx = vq.begin(); itx != vq.end(); ++itx) {
			Quadrado& q = *itx;
			q.desenha(pd, id++);
			glTranslatef(Quadrado::TAMANHO_GL, 0, 0);
		}
		glTranslatef(deltaX, Quadrado::TAMANHO_GL, 0);
	}
	glPopMatrix();
}

void Tabuleiro::clique(int id) {
	dt_->selecionaQuadrado(dt_->quadrados(id / dt_->tamanhoY())[ id % dt_->tamanhoX()]);
}


