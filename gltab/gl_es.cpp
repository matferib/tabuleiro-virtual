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
  //int max_pilha_mv = 0.0f;
  //int max_pilha_pj = 0.0f;
};
}  // namespace interno

namespace {

interno::Contexto g_contexto(new interno::ContextoEs);
interno::ContextoEs* g_contexto_interno = nullptr;

}  // namespace

void IniciaGl(int* argcp, char** argv) {
  g_contexto_interno = reinterpret_cast<interno::ContextoEs*>(g_contexto.interno.get());
  interno::IniciaComum(interno::LuzPorVertice(argcp == nullptr ? 0 : *argcp, argv), &g_contexto);
}

namespace interno {
Contexto* BuscaContexto() {
  return &g_contexto;
}

}  // namespace interno


}  // namespace gl
