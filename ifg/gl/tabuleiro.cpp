#include <GL/gl.h>

#include "ifg/gl/tabuleiro.h"
#include "ifg/gl/parametrosdesenho.h"

using namespace ifg::gl;

void Tabuleiro::desenha(const ParametrosDesenho& parametrosDesenho) {
	glColor3f(1.0, 0, 0);
	glRecti(50.0, 50.0, -50.0, -50.0);
}

