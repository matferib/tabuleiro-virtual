#ifndef IFG_GL_DESENHAVEL_H
#define IFG_GL_DESENHAVEL_H

namespace ifg {
namespace gl {

	// fwd
	class ParametrosDesenho;

	/** classe base dos objetos desenhaveis. Pode conter outros objetos e delegar a funcao de desenho. */
	class Desenhavel {
		/** desenha o objeto, recebendo os parametros de desenho. */
		virtual void desenha(const ParametrosDesenho& pd) = 0;
	};

}
}

#endif
