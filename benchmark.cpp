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

DEFINE_string(tabuleiro, "tabuleiros_salvos/douren.binproto", "Tabuleiro de benchmark.");
DEFINE_int32(num_desenhos, 1500, "Numero de vezes que o tabuleiro sera renderizado.");

int main(int argc, char** argv) {
  meulog::Inicializa(&argc, &argv);
  glutInit(&argc, argv);
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(0, 0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_STENCIL | GLUT_DEPTH);
  gl::IniciaGl(&argc, argv);
  glutCreateWindow("benchmark");
  ntf::CentralNotificacoes central;
  tex::Texturas texturas(&central);
  ent::Tabuleiro tabuleiro(&texturas, &central);
  tabuleiro.IniciaGL();
  tabuleiro.TrataRedimensionaJanela(300, 300);

  // Carrega tabuleiro.
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    n.set_endereco(FLAGS_tabuleiro);
    tabuleiro.TrataNotificacao(n);
  }
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_TEMPORIZADOR);
    tabuleiro.TrataNotificacao(n);
  }

  // Desenha varias vezes.
  boost::timer::auto_cpu_timer timer;
  for (int i = 0; i < FLAGS_num_desenhos; ++i) {
    tabuleiro.Desenha();
    tabuleiro.TrataRotacaoPorDelta(0.01f);
    glutSwapBuffers();
  }

  return 0;
}
