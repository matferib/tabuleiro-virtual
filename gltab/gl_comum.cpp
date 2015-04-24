#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include "gltab/gl.h"
#include "log/log.h"

// Comum.
namespace gl {

bool ImprimeSeErro() {
  auto erro = glGetError();
  if (erro != GL_NO_ERROR) {
#if USAR_OPENGL_ES
    LOG(ERROR) << "OpenGL Erro: " << erro;
#else
    LOG(ERROR) << "OpenGL Erro: " << gluErrorString(erro);
#endif
    return true;
  }
  return false;
}
#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)

namespace interno {

const std::vector<std::string> QuebraString(const std::string& entrada, char caractere_quebra) {
  std::vector<std::string> ret;
  if (entrada.empty()) {
    return ret;
  }
  auto it_inicio = entrada.begin();
  auto it = it_inicio;
  while (it != entrada.end()) {
    if (*it == caractere_quebra) {
      ret.push_back(std::string(it_inicio, it));
      it_inicio = ++it;
    } else {
      ++it;
    }
  }
  ret.push_back(std::string(it_inicio, it));
  return ret;
}

// Implementado diferente em cada um.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical);

}  // interno

// Sao funcoes iguais dos dois lados que dependem de implementacoes diferentes.
void DesenhaString(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 0, inverte_vertical);
}

void DesenhaStringAlinhadoEsquerda(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, -1, inverte_vertical);
}

void DesenhaStringAlinhadoDireita(const std::string& str, bool inverte_vertical) {
  interno::DesenhaStringAlinhado(str, 1, inverte_vertical);
}

}  // namespace gl
