#include <algorithm>
#include <boost/filesystem.hpp>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "arq/arquivo.h"
#include "ent/acoes.h"
#include "ent/acoes.pb.h"
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
//#define VLOG_NIVEL 3
#include "log/log.h"
#include "net/util.h"  // hack to_string
#include "ntf/notificacao.pb.h"


namespace ent {

// Esta operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador e a
// profundidade de quem o acertou.
void Tabuleiro::EncontraHits(int x, int y, unsigned int* numero_hits, unsigned int* buffer_hits) {
#if !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    // Se estiver ligado, desliga aqui para desenhar mapas.
    // Colocando dentro do if para evitar erro de opengl com plataformas que nao suportam GL_MULTISAMPLE.
    gl::Desabilita(GL_MULTISAMPLE);
  }
#endif

  gl::FramebufferEscopo framebuffer_escopo(dfb_colisao_.framebuffer);

  gl::UsaShader(gl::TSH_PICKING);
  //gl::Viewport(0, 0, (GLint)1, (GLint)1);
  // inicia o buffer de picking (selecao)
  gl::BufferSelecao(100, buffer_hits);
  // entra no modo de selecao e limpa a pilha de nomes e inicia com 0
  gl::ModoRenderizacao(gl::MR_SELECT);

  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  //gl::MatrizEscopo salva_proj(gl::MATRIZ_PROJECAO);
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  LOG(INFO) << "largura: " << viewport[2] << ", altura: " << viewport[3];
  gl::CarregaIdentidade();
  // Hack para testar projecao menor.
  //if (!parametros_desenho_.projecao().has_largura_m()) {
    gl::MatrizPicking(x, y, 1.0, 1.0, viewport);
  //}
  //float glm[16];
  //gl::Le(GL_PROJECTION_MATRIX, glm);
  //Matrix4 prm(glm);
  //gl::Le(GL_MODELVIEW_MATRIX, glm);
  //Matrix4 mvm(glm);
  //LOG(INFO) << "Matriz projecao: \n" << prm << "\n" << "Matriz Modelview: \n" << mvm << "\n\n";

  // desenha a cena sem firulas.
  parametros_desenho_.set_picking_x(x);
  parametros_desenho_.set_picking_y(y);
  parametros_desenho_.set_iluminacao(false);
  parametros_desenho_.set_desenha_texturas(false);
  parametros_desenho_.set_desenha_grade(false);
  parametros_desenho_.set_desenha_fps(false);
  parametros_desenho_.set_desenha_aura(false);
  parametros_desenho_.set_desenha_sombras(false);
  parametros_desenho_.set_desenha_mapa_sombras(false);
  parametros_desenho_.clear_desenha_mapa_oclusao();
  parametros_desenho_.set_limpa_fundo(false);
  parametros_desenho_.set_usar_transparencias(false);
  parametros_desenho_.set_desenha_acoes(false);
  parametros_desenho_.set_desenha_lista_pontos_vida(false);
  parametros_desenho_.set_desenha_quadrado_selecao(false);
  parametros_desenho_.set_desenha_rastro_movimento(false);
  parametros_desenho_.set_desenha_forma_selecionada(false);
  parametros_desenho_.set_desenha_rosa_dos_ventos(false);
  parametros_desenho_.set_desenha_nevoa(false);
  parametros_desenho_.set_desenha_info_geral(false);
  parametros_desenho_.set_desenha_detalhes(false);
  parametros_desenho_.set_desenha_eventos_entidades(false);
  parametros_desenho_.set_desenha_efeitos_entidades(false);
  parametros_desenho_.set_desenha_grade(false);
  parametros_desenho_.set_desenha_ligacao_agarrar(false);
  // Aplica opcoes do jogador.
  parametros_desenho_.set_desenha_lista_objetos(opcoes_.mostra_lista_objetos());
  parametros_desenho_.set_texturas_sempre_de_frente(opcoes_.texturas_sempre_de_frente());

  // Com todos os parametros setados, configura a projecao (por exemplo, plano de corte pode
  // variar com ou sem nevoa.
  ConfiguraProjecao();

  gl::Desabilita(GL_BLEND);
  DesenhaCena();

  // Volta pro modo de desenho, retornando quanto pegou no SELECT.
  *numero_hits = gl::ModoRenderizacao(gl::MR_RENDER);
  //gl::Viewport(0, 0, (GLint)largura_, (GLint)altura_);
  //auto e = glGetError();
  //if (e != GL_NO_ERROR) {
  //  LOG(ERROR) << "Erro de picking: " << gluErrorString(e);
  //}

  // Aqui restaura a projecao sem picking. Nao da pra salvar a projecao no inicio, porque ela eh diferente
  // da projecao usada pelo picking. Entao a responsa eh de quem chama.
  gl::MudaModoMatriz(gl::MATRIZ_PROJECAO);
  gl::CarregaIdentidade();
  ConfiguraProjecao();

#if !USAR_OPENGL_ES
  if (opcoes_.anti_aliasing()) {
    gl::Habilita(GL_MULTISAMPLE);
  }
#endif
}

// Operacoes de picking neste modulo.
void Tabuleiro::BuscaHitMaisProximo(
    int x, int y, unsigned int* id, unsigned int* tipo_objeto, float* profundidade) {
  GLuint buffer_hits[100] = {0};
  GLuint numero_hits = 0;
  EncontraHits(x, y, &numero_hits, buffer_hits);
  // Cada hit ocupa pelo menos 4 inteiros do buffer. Na pratica, por causa da pilha vao ocupar ate mais.
  if (numero_hits > 25) {
    LOG(WARNING) << "Muitos hits para a posicao: " << numero_hits << ", tamanho de buffer de selecao invalido.";
    *tipo_objeto = 0;
    *id = 0;
    return;
  }

  // Busca o hit mais próximo em buffer_hits. Cada posicao do buffer (hit record):
  // - 0: tipo_objeto de nomes (numero de nomes empilhados);
  // - 1: profundidade minima.
  // - 2: profundidade maxima.
  // - 3: nomes empilhados (1 para cada pos pilha).
  // Dado o hit mais proximo, retorna o identificador, a posicao da pilha e a
  // profundidade do objeto (normalizado 0..1.0).
  VLOG(2) << "numero de hits no buffer de picking: " << numero_hits;
  GLuint* ptr_hits = buffer_hits;
  // valores do hit mais proximo.
  GLuint menor_z = 0xFFFFFFFF;
  GLuint tipo_objeto_menor = 0;
  GLuint id_menor = 0;

  // Busca o hit mais proximo.
  for (GLuint i = 0; i < numero_hits; ++i) {
    GLuint pos_pilha_corrente = *ptr_hits;
    ++ptr_hits;
    if (pos_pilha_corrente != 2) {
      LOG(ERROR) << "Tamanho da pilha diferente de 2: " << pos_pilha_corrente;
      *tipo_objeto = 0;
      *id = 0;
      return;
    }
    GLuint z_corrente = *ptr_hits;
    ptr_hits += 2;  // pula maximo.
    // Tipo do objeto do hit.
    GLuint tipo_corrente = *ptr_hits;
    ++ptr_hits;
    // Id do objeto.
    GLuint id_corrente = *ptr_hits;
    ++ptr_hits;

    if (z_corrente <= menor_z) {
      VLOG(3) << "tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
      menor_z = z_corrente;
      tipo_objeto_menor = tipo_corrente;
      id_menor = id_corrente;
    } else {
      VLOG(3) << "Pulando objeto, tipo_corrente: " << tipo_corrente
              << ", z_corrente: " << z_corrente
              << ", id_corrente: " << id_corrente;
    }
  }
  *tipo_objeto = tipo_objeto_menor;
  *id = id_menor;
  float menor_profundidade = 0.0f;
  // Converte profundidade de inteiro para float.
  // No OpenGL ES a profundidade retornada vai ser sempre zero. Se nao houver hit, menor_z vai ser 0xFFFFFFFF
  // e a profundidade maxima sera retornada.
  menor_profundidade = static_cast<float>(menor_z) / static_cast<float>(0xFFFFFFFF);
  if (profundidade != nullptr) {
    *profundidade = menor_profundidade;
  }
  VLOG(1) << "Retornando menor profundidade: " << menor_profundidade
          << ", tipo_objeto: " << tipo_objeto_menor
          << ", id: " << id_menor;
}

bool Tabuleiro::MousePara3d(int x, int y, float* x3d, float* y3d, float* z3d) {
  GLuint id;
  GLuint tipo_objeto;
  float profundidade;
  BuscaHitMaisProximo(x, y, &id, &tipo_objeto, &profundidade);
  if (profundidade == 1.0f) {
    return false;
  }
  return MousePara3dComProfundidade(x, y, profundidade, x3d, y3d, z3d);
}

bool Tabuleiro::MousePara3dParaleloZero(int x, int y, float* x3d, float* y3d, float* z3d) {
  // TODO: computar a matriz de modelagem e projecao aqui de acordo com a camera, para
  // evitar que qualquer efeito colateral nelas cause problemas de arrastar.
  // Intersecao de reta com plano z=0.
  GLfloat modelview[16], projection[16];
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  float p1x, p1y, p1z;
  gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
  float p2x, p2y, p2z;
  gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
  if (p2z - p1z == 0) {
    LOG(ERROR) << "Retornando lixo";
    return false;
  }
  float mult = (parametros_desenho_.offset_terreno() - p1z) / (p2z - p1z);
  //LOG(INFO) << "mult: " << mult << ", x: " << x << ", y: " << y << ", p1z " << p1z << ", p2z " << p2z << " offset: " << parametros_desenho_.offset_terreno();
  //LOG(INFO) << "mvm: " << Matrix4(modelview);
  //LOG(INFO) << "prj: " << Matrix4(projection);
  //LOG(INFO) << "viewport: " << viewport[0] << " " << viewport[1] << " " << viewport[2] << " " << viewport[3];

  *x3d = p1x + (p2x - p1x) * mult;
  *y3d = p1y + (p2y - p1y) * mult;
  *z3d = parametros_desenho_.offset_terreno();
  VLOG(2) << "Retornando tabuleiro: " << *x3d << ", " << *y3d << ", " << *z3d;
  return true;
}

bool Tabuleiro::MousePara3dComProfundidade(int x, int y, float profundidade, float* x3d, float* y3d, float* z3d) {
  GLfloat modelview[16], projection[16];
  GLint viewport[4];
  gl::Le(GL_MODELVIEW_MATRIX, modelview);
  gl::Le(GL_PROJECTION_MATRIX, projection);
  gl::Le(GL_VIEWPORT, viewport);
  if (!gl::Desprojeta(x, y, profundidade,
                      modelview, projection, viewport,
                      x3d, y3d, z3d)) {
    LOG(ERROR) << "Falha ao projetar x y no mundo 3d.";
    return false;
  }
  VLOG(1) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}

bool Tabuleiro::MousePara3dComId(int x, int y, unsigned int id, unsigned int tipo_objeto, float* x3d, float* y3d, float* z3d) {
  // Busca mais detalhado.
  if (tipo_objeto == 1) {
    MousePara3dParaleloZero(x, y, x3d, y3d, z3d);
  } else {
    GLfloat modelview[16], projection[16];
    GLint viewport[4];
    gl::Le(GL_MODELVIEW_MATRIX, modelview);
    gl::Le(GL_PROJECTION_MATRIX, projection);
    gl::Le(GL_VIEWPORT, viewport);
    // Raio que sai do pixel.
    float p1x, p1y, p1z;
    gl::Desprojeta(x, y, -1.0f, modelview, projection, viewport, &p1x, &p1y, &p1z);
    if (camera_ == CAMERA_ISOMETRICA) {
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }

    float p2x, p2y, p2z;
    gl::Desprojeta(x, y, 1.0f, modelview, projection, viewport, &p2x, &p2y, &p2z);
    if (p2z - p1z == 0) {
      LOG(ERROR) << "Retornando lixo";
      return false;
    }
    // Equacao parametrica do raio. Substituindo os p1* por x0, y0 e z0 e os p2* por x, y e z, temos:
    // x = x0 + at
    // y = y0 + bt
    // z = z0 + ct
    float a_raio = p2x - p1x;
    float b_raio = p2y - p1y;
    float c_raio = p2z - p1z;

    auto* e = BuscaEntidade(id);
    if (e == nullptr) {
      LOG(ERROR) << "Retornando lixo porque nao achei a entidade";
      return false;
    }
    // Cria um plano perpendicular a linha de visao para o objeto e com o plano XY.
    // Equacao do olho para o objeto. a_olho_obj * x + b_olho_obj = y.
    // Equacao da perdicular: a_perpendicular * x + b_perpendicular = y.
    //                         onde a_perpendicular = -1 / a_olho_obj.
    float a_perpendicular = (fabs(olho_.pos().x() -  e->X()) < 0.0001f) ?
        0.0f : (-1.0f / (olho_.pos().y() - e->Y()) / (olho_.pos().x() - e->X()));
    float b_perpendicular = e->Y() - e->X() * a_perpendicular;

    // Valor do t da intersecao.: onde a equacao perpendicular encontra com o plano.
    // (para simplicar nomenclatura, p = a_perpendicular, q = b_perpendicular, a = a_raio, b = b_raio).
    // (x0 = p1x, y0 = p1y, z0 = p1z).
    // y = y0 + bt = px + q;
    // t = (px + q - y0) / b. (1)
    // Como: x = x0 + at,
    // entao t = (x - x0) / a. (2)
    // Igualando (1) e (2):
    // (px + q - y0) / b = (x - x0) / a;
    // apx + aq - ay0 = bx - bx0;
    // apx - bx = ay0 - aq - bx0;
    // x (ap - b) = ay0 - aq - bx0;
    // Portanto, o x da intercessao eh (tanto faz, eh so multiplicar acima por -1 dos dois lados):
    // x = (ay0 - aq - bx0) / (ap - b) ou
    // x = (aq - ay0 + bx0) / (b - ap)
    if (fabs(b_raio - a_raio * a_perpendicular) < 0.0001f) {
      LOG(WARNING) << "Projecao praticamente perpendicular, retornando valores simples";
      *x3d = p1x;
      *y3d = p1y;
      *z3d = 0.0f;
      return true;
    }
    float x_inter = (a_raio * b_perpendicular - a_raio * p1y + b_raio * p1x) / (b_raio - a_raio * a_perpendicular);
    // Valor do t para interceptar o plano perpendicular
    float t_inter = (x_inter - p1x) / a_raio;
    // Outros valores da intersecao.
    float y_inter = p1y + b_raio * t_inter;
    float z_inter = p1z + c_raio * t_inter;

    *x3d = x_inter;
    *y3d = y_inter;
    *z3d = z_inter;
  }
  // Importante para operacoes no mesmo frame nao se confundirem.
  VLOG(2) << "Retornando: " << *x3d << " " << *y3d << " " << *z3d;
  return true;
}

}  // namespace ent
