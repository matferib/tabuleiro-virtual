/** @file main.cpp o inicio de tudo. Responsavel por instanciar a interface grafica principal. */

#include <boost/timer/timer.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

#if __APPLE__
  // Other kinds of Mac OS
  #include <GLUT/glut.h>
#endif

#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "gltab/gl.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.h"
#include "log/log.h"
#include "tex/texturas.h"

#include <setjmp.h>
#include <GL/glut.h>

using namespace std;

DEFINE_string(tabuleiro, "castelo.binproto", "Tabuleiro de benchmark.");
DEFINE_int32(num_desenhos, 1000, "Numero de vezes que o tabuleiro sera renderizado.");
DEFINE_int32(tam_janela, 1000, "Altura e largura da janela quadrada");

ent::Tabuleiro* g_tabuleiro = nullptr;
int g_counter = 0;
jmp_buf g_buf;

void render() {
  g_tabuleiro->Desenha();
  g_tabuleiro->TrataRotacaoPorDelta(0.01f);
  glutSwapBuffers();
  if (++g_counter >= FLAGS_num_desenhos) {
    LOG(INFO) << "saindo!!";
    longjmp(g_buf, 1);
  }
}

void reshape(int width, int height) {
  g_tabuleiro->TrataRedimensionaJanela(width, height);
}

int main(int argc, char** argv) {
  meulog::Inicializa(&argc, &argv);
  // luz por pixel e mapa sombras.
  glutInitWindowSize(FLAGS_tam_janela, FLAGS_tam_janela);
  glutInitWindowPosition(0, 0);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_STENCIL | GLUT_DEPTH);
  glutInit(&argc, argv);
  glutCreateWindow("benchmark");
  gl::IniciaGl(true);  // inicia o glut.
  ntf::CentralNotificacoes central;
  tex::Texturas texturas(&central);
  ent::OpcoesProto opcoes;
  opcoes.set_iluminacao_por_pixel(true);
  opcoes.set_mapeamento_sombras(true);
  m3d::Modelos3d modelos(&central);
  ent::Tabuleiro tabuleiro(opcoes, &texturas, &modelos, &central);
  g_tabuleiro = &tabuleiro;
  tabuleiro.IniciaGL();
  tabuleiro.TrataRedimensionaJanela(FLAGS_tam_janela, FLAGS_tam_janela);

  // Carrega tabuleiro.
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
    n.set_endereco("estatico://" + FLAGS_tabuleiro);
    tabuleiro.TrataNotificacao(n);
  }
  {
    ntf::Notificacao n;
    n.set_tipo(ntf::TN_TEMPORIZADOR);
    tabuleiro.TrataNotificacao(n);
    for (int i = 0; i < 1000; ++i) {
      // Carrega.
      central.Notifica();
    }
  }

  // Desenha varias vezes.
  glutReshapeFunc(reshape);
  glutIdleFunc(render);
  glutDisplayFunc(render);
  boost::timer::auto_cpu_timer timer;
  if (!setjmp(g_buf)) {
    glutMainLoop();
  }

  return 0;
}
