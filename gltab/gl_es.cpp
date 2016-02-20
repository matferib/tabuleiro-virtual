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

void IniciaGl(bool luz_por_pixel, bool mapeamento_sombras) {
  interno::IniciaComum(luz_por_pixel, mapeamento_sombras, BuscaContexto());
}

namespace interno {
Contexto* BuscaContexto() {
  static Contexto* g_contexto = new Contexto(new ContextoEs);
  return g_contexto;
}

}  // namespace interno


}  // namespace gl
