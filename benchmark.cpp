/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <boost/timer/timer.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <GL/gl.h>
#include <GL/glut.h>

#include "ent/tabuleiro.h"
#include "gltab/gl.h"
#include "ntf/notificacao.h"
#include "log/log.h"
#include "tex/texturas.h"

using namespace std;

int main(int argc, char** argv) {
  gl::IniciaGl(&argc, argv);
  ent::Tabuleiro::InicializaGL();

  glutInit(&argc, argv);
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(0, 0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_STENCIL | GLUT_DEPTH);
  glutCreateWindow("benchmark");
  ntf::CentralNotificacoes central;
  tex::Texturas texturas(&central);
  ent::Tabuleiro tabuleiro(&texturas, &central);
  tabuleiro.TrataRedimensionaJanela(300, 300);

  // Carrega castelo.
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    n.set_endereco("tabuleiros_salvos/castelo.binproto");
    tabuleiro.TrataNotificacao(n);
  }
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_TEMPORIZADOR);
    tabuleiro.TrataNotificacao(n);
  }

  // Desenha 100 vezes.
  boost::timer::auto_cpu_timer timer;
  for (int i = 0; i < 500; ++i) {
    tabuleiro.Desenha();
    glutSwapBuffers();
  }

  return 0;
}
