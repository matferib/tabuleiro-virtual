// OpenGL ES.
// Varias funcoes copiadas do GLUES: https://code.google.com/p/glues/.

#if __APPLE__
#include "TargetConditionals.h"
#endif
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include "gltab/gl_interno.h"
#include "gltab/glues.h"
#include "log/log.h"

using gl::interno::TSH_LUZ;
using gl::interno::TSH_SIMPLES;

namespace gl {

bool ImprimeSeErro(const char* mais);

namespace interno {
struct ContextoEs : public ContextoDependente {
  // Mapeia um ID para a cor RGB em 21 bits (os dois mais significativos sao para a pilha).
  //std::unordered_map<unsigned int, unsigned int> ids;
  //unsigned int proximo_id = 0;
  // O bit da pilha em tres bits (valor de [0 a 7]).
  //unsigned int bit_pilha = 0;
  //modo_renderizacao_e modo_renderizacao = MR_RENDER;
  //GLuint* buffer_selecao = nullptr;
  //GLuint tam_buffer = 0;
  int max_pilha_mv = 0.0f;
  int max_pilha_pj = 0.0f;
  bool* depurar_selecao_por_cor;
};
}  // namespace interno

namespace {

interno::Contexto g_contexto(new interno::ContextoEs);
interno::ContextoEs* g_contexto_interno = nullptr;

// Gera um proximo ID.
void MapeiaId(unsigned int id, GLubyte rgb[3]) {
  unsigned int id_mapeado = g_contexto.proximo_id | (g_contexto.bit_pilha << 21);
  g_contexto.ids.insert(std::make_pair(id_mapeado, id));
  if (g_contexto.proximo_id == ((1 << 21) - 1)) {
    LOG(ERROR) << "Limite de ids alcancado";
  } else {
    if (*g_contexto_interno->depurar_selecao_por_cor) {
      // Mais facil de ver.
      g_contexto.proximo_id += 5;
    } else {
      ++g_contexto.proximo_id;
    }
  }
  rgb[0] = (id_mapeado & 0xFF);
  rgb[1] = ((id_mapeado >> 8) & 0xFF);
  rgb[2] = ((id_mapeado >> 16) & 0xFF);
}

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto_interno = reinterpret_cast<interno::ContextoEs*>(g_contexto.interno.get());
  g_contexto_interno->depurar_selecao_por_cor = &g_contexto.depurar_selecao_por_cor;
#if 0
  gl::Le(GL_MAX_PROJECTION_STACK_DEPTH, &g_contexto_interno->max_pilha_pj);
  gl::Le(GL_MAX_MODELVIEW_STACK_DEPTH, &g_contexto_interno->max_pilha_mv);
  LOG(INFO) << "Max pilha mv: " << g_contexto_interno->max_pilha_mv;
  LOG(INFO) << "Max pilha pj: " << g_contexto_interno->max_pilha_pj;
#endif
  interno::IniciaComum(interno::LuzPorVertice(argcp == nullptr ? 0 : *argcp, argv), &g_contexto);
}

void FinalizaGl() {
  interno::FinalizaShaders(g_contexto.shaders[TSH_LUZ]);
  interno::FinalizaShaders(g_contexto.shaders[TSH_SIMPLES]);
}

void InicioCena() {
  if (g_contexto.UsarSelecaoPorCor()) {
    g_contexto.proximo_id = 0;
    g_contexto.bit_pilha = 0;
    g_contexto.ids.clear();
  }
}

void Habilita(GLenum cap) {
  if (g_contexto.UsarSelecaoPorCor()) {
    if (cap == GL_LIGHTING) {
      return;
    }
  } else {
#if USAR_SHADER
    interno::HabilitaComShader(&g_contexto, cap);
#else
    glEnable(cap);
#endif
  }
}

void Desabilita(GLenum cap) {
#if USAR_SHADER
  interno::DesabilitaComShader(&g_contexto, cap);
  V_ERRO((std::string("desabilitando es cap: ") + std::to_string((int)cap)).c_str());
#else
  glDisable(cap);
  V_ERRO("acola");
#endif
}

GLint ModoRenderizacao(modo_renderizacao_e modo) {
  if (g_contexto.modo_renderizacao == modo) {
    VLOG(1) << "Nao houve mudanca no modo de renderizacao";
    return 0;
  }
  g_contexto.modo_renderizacao = modo;
  switch (modo) {
    case MR_SELECT:
      return 0;
    case MR_RENDER: {
      glFlush();
      glFinish();
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      GLubyte pixel[4] = { 0 };
      glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
      int erro = glGetError();
      if (erro != 0) {
        VLOG(1) << "Erro pos glReadPixels: " << erro;
      }
      // Usar void* para imprimir em hexa.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      VLOG(2) << "Pixel: " << (void*)pixel[0] << " " << (void*)pixel[1] << " " << (void*)pixel[2] << " " << (void*)pixel[3];
      unsigned int id_mapeado = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);
      VLOG(1) << "Id mapeado: " << (void*)id_mapeado;
      unsigned int tipo_objeto = id_mapeado >> 21;
      VLOG(1) << "Tipo objeto: " << tipo_objeto;
      if (tipo_objeto > 7) {
        LOG(ERROR) << "Tipo objeto invalido: " << tipo_objeto;
        return 0;
      }
      auto it = g_contexto.ids.find(id_mapeado);
      if (it == g_contexto.ids.end()) {
        LOG(ERROR) << "Id nao mapeado: " << (void*)id_mapeado;
        return 0;
      }
#pragma GCC diagnostic pop
      unsigned int id_original = it->second;
      VLOG(1) << "Id original: " << id_original;
      GLuint* ptr = g_contexto.buffer_selecao;
      ptr[0] = 2;  // Sempre 2: 1 para tipo, outro para id.
      ptr[1] = 0;  // zmin.
      ptr[2] = 0;  // zmax
      ptr[3] = tipo_objeto;
      ptr[4] = id_original;
      g_contexto.buffer_selecao = nullptr;
      g_contexto.tam_buffer = 0;
      return 1;  // Numero de hits: so pode ser 0 ou 1.
    }
    default:
      return 0;
  }
}

void BufferSelecao(GLsizei tam_buffer, GLuint* buffer) {
  g_contexto.buffer_selecao = buffer;
  g_contexto.tam_buffer = tam_buffer;
}

// Nomes
void IniciaNomes() {
}

void EmpilhaNome(GLuint id) {
  if (!g_contexto.UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (id > 7) {
    LOG(ERROR) << "Bit da pilha passou do limite superior.";
    return;
  }
  g_contexto.bit_pilha = id;
  VLOG(1) << "Empilhando bit pilha: " << g_contexto.bit_pilha;
}

void CarregaNome(GLuint id) {
  if (!g_contexto.UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  GLubyte rgb[3];
  MapeiaId(id, rgb);
  VLOG(2) << "Mapeando " << id << ", bit pilha " << g_contexto.bit_pilha
          << " para " << (int)rgb[0] << ", " << (int)rgb[1] << ", " << (int)rgb[2];
  // Muda a cor para a mapeada.
#if USAR_SHADER
  glVertexAttrib4f(interno::BuscaShader().atr_gltab_cor, rgb[0] / 255.0f, rgb[1] / 255.0f, rgb[2] / 255.0f, 1.0f);
#else
  glColor4ub(rgb[0], rgb[1], rgb[2], 255);
#endif
}

void DesempilhaNome() {
  if (!g_contexto.UsarSelecaoPorCor()) {
    // So muda no modo de selecao.
    return;
  }
  if (g_contexto.bit_pilha == 0) {
    // No jeito novo, isso nao eh mais erro.
    //LOG(ERROR) << "Bit da pilha passou do limite inferior.";
    return;
  }
  VLOG(1) << "Desempilhando bit pilha: " << g_contexto.bit_pilha;
  g_contexto.bit_pilha = 0;
}

void MudaCor(float r, float g, float b, float a) {
  if (g_contexto.UsarSelecaoPorCor()) {
    // So muda no modo de renderizacao pra nao estragar o picking por cor.
    return;
  }
  //GLfloat cor[4] = { r, g, b, a };
  // Segundo manual do OpenGL ES, nao se pode definir o material separadamente por face.
  //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cor);
#if USAR_SHADER
  glVertexAttrib4f(interno::BuscaShader().atr_gltab_cor, r, g, b, a);
#else
  glColor4f(r, g, b, a);
#endif
}

void Limpa(GLbitfield mascara) {
  if (g_contexto.UsarSelecaoPorCor()) {
    if ((mascara & GL_COLOR_BUFFER_BIT) != 0) {
      // Preto nao eh valido no color picking.
      glClearColor(0, 0, 0, 1.0f);
    }
  }
  glClear(mascara);
}

void AlternaModoDebug() {
  *g_contexto_interno->depurar_selecao_por_cor = !*g_contexto_interno->depurar_selecao_por_cor;
}

GLint Uniforme(const char* id) {
#if USAR_SHADER
  GLint ret = glGetUniformLocation(g_contexto.shaders[TSH_LUZ].programa, id);
  if (ret == -1) {
    LOG(INFO) << "Uniforme nao encontrada: " << id;
  }
  return ret;
#else
  return -1;
#endif
}

namespace interno {
Contexto* BuscaContexto() {
  return &g_contexto;
}

}  // namespace interno


}  // namespace gl
