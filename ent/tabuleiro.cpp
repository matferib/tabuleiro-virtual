#include <vector>
#include <map>
#include <stdexcept>
#include <GL/gl.h>
#include <iostream>
#include "ent/tabuleiro.h"
#include "ent/parametrosdesenho.h"
#include "ent/movel.h"
#include "ntf/notificacao.h"

using namespace ent;
using namespace std;

namespace {
	/** tamanho do lado do quadrado no 3D. */
	const double TAMANHO_GL = 1.5;
	const double TAMANHO_GL_2 = (TAMANHO_GL / 2.0);

	/** instancia unica do tabuleiro. */
	Tabuleiro* instancia_ = NULL;

	/** representa um quadrado no tabuleiro. Cada quadrado tem lado 1,5m. */
	class Quadrado {
	public:

		/** desenha o quadrado. */
		void desenha(const ParametrosDesenho& pd, GLuint id) {
			// desenha o quadrado
			glLoadName(id);
			glColor3dv(selecionado_ ? pd.corTabuleiroSelecionado() : pd.corTabuleiroNaoSelecionado());
			glRectf(0.2, 0.2, (TAMANHO_GL-0.2), (TAMANHO_GL-0.2));
		}


		/** seleciona ou deseleciona o quadrado. */
		void seleciona(bool val) { 
			selecionado_ = val; 
		}

	private:
		/** indica que o quadrado esta selecionado. */
		bool selecionado_;
	};

}

namespace ent {
	enum etab_t {
		ETAB_NORMAL,
		ETAB_ADICIONANDO_ENTIDADE,
		ETAB_REMOVENDO_ENTIDADE
	};

	typedef map<int, Entidade*> MapaEntidades;
	typedef vector<Entidade*> VetorEntidades;

	/** dados privados do tabuleiro. */
	class DadosTabuleiro {
	public:
		explicit DadosTabuleiro(int tamanho) : 
			tamanho_(tamanho), 
			entidadesPorQuadrado_(tamanho_*tamanho_, NULL), entidadeSelecionada_(NULL), 
			quadradoSelecionado_(NULL), 
			estado_(ETAB_NORMAL), proximoId_(0)
		{
			quadrados_ = new vector<Quadrado>[tamanho];
			for (int i = 0; i < tamanho; ++i) {
				quadrados_[i].resize(tamanho);
			}
		}

		/** destroi entidades. */
		~DadosTabuleiro() {
			delete[] quadrados_;
			for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
				delete it->second;
			}
		}

		// tamanho
		inline int tamanhoX() const { return tamanho_; }
		inline int tamanhoY() const { return tamanho_; }

		/** adiciona entidade ao quadrado e a colecao de entidades. */
		void adicionaEntidade(Entidade* entidade, int idQuadrado) {
			entidades_.insert(make_pair(entidade->id(), entidade));
			entidadesPorQuadrado_[idQuadrado] = entidade;
		}

		void removeEntidade(int id) {
			MapaEntidades::iterator resFind = entidades_.find(id);
			if (resFind == entidades_.end()) {
				return;
			}
			entidades_.erase(resFind);
			Entidade* entidade = resFind->second;
			for (VetorEntidades::iterator it = entidadesPorQuadrado_.begin(); it != entidadesPorQuadrado_.end(); ++it) {
				if (*it == entidade) {
					*it = NULL;
				}
			}
			if (entidadeSelecionada_ == entidade) {
				entidadeSelecionada_ = NULL;
			}
			delete entidade;
		}

		/** seleciona a entidade pelo id. */
		void selecionaEntidade(int id) {
			//if (entidadeSelecionada_ != NULL) { entidadeSelecionada_->seleciona(false); }
			Entidade* e = entidades_.find(id)->second;
			entidadeSelecionada_ = e; 
			//entidadeSelecionada_->seleciona(true);
		}

		// quadrados
		/** @return o quadrado identificado por id no desenho. */
		inline Quadrado* quadrado(int id) { return &quadrados_[id / tamanhoY()][ id % tamanhoX()]; }
		
		/** seleciona o quadrado. */
		void selecionaQuadrado(int idQuadrado) {
			if (quadradoSelecionado_ != NULL) { quadradoSelecionado_->seleciona(false); }
			Quadrado* q = quadrado(idQuadrado);
			quadradoSelecionado_ = q; 
			quadradoSelecionado_->seleciona(true);
		}

	private:
		/** tamanho do tabuleiro (tamanho_ x tamanho_). */
		int tamanho_;

	public:
		/** mapa geral de entidades, por id. */
		MapaEntidades entidades_;

		/** entidades por quadrado. */
		VetorEntidades entidadesPorQuadrado_;

		/** a entidade selecionada. */
		Entidade* entidadeSelecionada_;

		/** quadrados */
		vector<Quadrado>* quadrados_;

		/** quadrado selecionado. */
		Quadrado* quadradoSelecionado_;

		/** estado do tabuleiro. */
		etab_t estado_;

		/** proximo id de entidades. */
		unsigned int proximoId_;
	};
}

Tabuleiro::Tabuleiro(int tamanho) : dt_(new DadosTabuleiro(tamanho)) {
	if (instancia_ != NULL) {
		throw logic_error("tabuleiro ja existe");
	}
	instancia_ = this;
}

bool Tabuleiro::haInstancia() {
	return (instancia_ != NULL);
}

Tabuleiro& Tabuleiro::instancia() {
	return *instancia_;
}

void Tabuleiro::cria(int tamanho) {
	delete instancia_;
	instancia_ = new Tabuleiro(tamanho);
}

Tabuleiro::~Tabuleiro() {
	delete dt_;
}


int Tabuleiro::tamanhoX() const {
	return dt_->tamanhoX();
}

int Tabuleiro::tamanhoY() const {
	return dt_->tamanhoY();
}

void Tabuleiro::desenha(const ParametrosDesenho& pd) {
	glPushMatrix();
	// desenha de baixo pra cima
	double deltaX = -dt_->tamanhoX() * TAMANHO_GL;
	double deltaY = -dt_->tamanhoY() * TAMANHO_GL;
	glTranslatef(deltaX / 2.0, deltaY / 2.0, 0);

	GLuint id = 0;
	Entidade* entidade;
	for (int y = 0; y < dt_->tamanhoY(); ++y) {
		vector<Quadrado>& vq = dt_->quadrados_[y];
		for (vector<Quadrado>::iterator itx = vq.begin(); itx != vq.end(); ++itx) {
			Quadrado& q = *itx;
			// desenha quadrado
			q.desenha(pd, id);
			// desenha a entidade no quadrado
			glPushMatrix();
			glTranslated(TAMANHO_GL_2, TAMANHO_GL_2, 0);
			if ((entidade = dt_->entidadesPorQuadrado_[id]) != NULL) {
				glPushName(entidade->id());
				entidade->desenha(pd);
				glPopName();
			}
			glPopMatrix();
			glTranslated(TAMANHO_GL, 0, 0);

			++id;
		}
		glTranslated(deltaX, TAMANHO_GL, 0);
	}
	glPopMatrix();
}

void Tabuleiro::selecionaQuadrado(int idQuadrado) {
	dt_->selecionaQuadrado(idQuadrado);
}

void Tabuleiro::selecionaEntidade(int id) {
	dt_->selecionaEntidade(id);
}

void Tabuleiro::adicionaEntidade(tipoent_t tipoEntidade, DadosCriacao* dc, int idQuadrado) {
	Entidade* entidade;
	switch (tipoEntidade) {
		case TIPOENT_MOVEL:
			entidade = new Movel(dt_->proximoId_++);
		break;
		default:
			throw logic_error("tipo invalido de entidade");
	}
	dt_->adicionaEntidade(entidade, idQuadrado);
}

void Tabuleiro::removeEntidade(int id) {
	dt_->removeEntidade(id);
}

void Tabuleiro::trataClique(unsigned int numeroHits, unsigned int* bufferHits) {
	cout << "numero de hits: " << (unsigned int)numeroHits << endl << endl;
	GLuint* ptrHits = bufferHits;
	GLuint id = 0, posPilha = 0;
	for (GLuint i = 0; i < numeroHits; ++i) {
		posPilha = *ptrHits;
		cout << "posicao pilha: " << (unsigned int)(posPilha) << endl;
		// pula ele mesmo, profundidade e ids anteriores na pilha
		ptrHits += posPilha + 2;
		id = *ptrHits;
		cout << "id: " << (unsigned int)(id) << endl << endl;
		++ptrHits;
	}
	if (posPilha == 1) {
		// tabuleiro
		if (dt_->estado_ == ETAB_NORMAL) {
			selecionaQuadrado(id);
		}
		else if (dt_->estado_ == ETAB_ADICIONANDO_ENTIDADE) {
			adicionaEntidade(TIPOENT_MOVEL, NULL, id);
			dt_->estado_ = ETAB_NORMAL;
		}
	}
	else if (posPilha > 1) {
		if (dt_->estado_ == ETAB_NORMAL) {
			selecionaEntidade(id);
		}
		else if (dt_->estado_ == ETAB_REMOVENDO_ENTIDADE) {
			removeEntidade(id);
			dt_->estado_ = ETAB_NORMAL;
		}
	}

}

void Tabuleiro::trataNotificacao(const ntf::Notificacao& notificacao) {
	switch (notificacao.tipo()) {
		case ntf::TN_ADICIONAR_ENTIDADE:
			dt_->estado_ = ETAB_ADICIONANDO_ENTIDADE;
		break;
		case ntf::TN_REMOVER_ENTIDADE:
			dt_->estado_ = ETAB_REMOVENDO_ENTIDADE;
		break;
		default:
			;
	}
}



