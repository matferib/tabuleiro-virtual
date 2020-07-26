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

using gl::interno::BuscaContexto;

namespace gl {

bool ImprimeSeErro(const char* mais);

namespace interno {
struct ContextoEs : public ContextoDependente {
};
}  // namespace interno

namespace {
interno::Contexto* g_contexto = nullptr;
}  // namespace


void IniciaGl(TipoLuz tipo_luz, float escala) {
  g_contexto = new interno::Contexto(escala, new interno::ContextoEs);
  interno::IniciaComum(tipo_luz, g_contexto->escala, BuscaContexto());
}

namespace interno {
Contexto* BuscaContexto() {
  return g_contexto;
}

}  // namespace interno


}  // namespace gl
