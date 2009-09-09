#ifndef IFG_GL_TABULEIRO_H
#define IFG_GL_TABULEIRO_H

#include "ifg/gl/desenhavel.h"

namespace ifg {
namespace gl {

	/** objeto principal de desenho. Raiz de todos desenhaveis. */
	class Tabuleiro : public Desenhavel {
	public:
		void desenha(const ParametrosDesenho& pd);
	};

}
}

#endif
