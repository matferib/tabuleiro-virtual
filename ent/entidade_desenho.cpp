/** Implementacao dos varios tipos de desenho da classe Entidade. */

#include <algorithm>
#include <cmath>
#include <memory>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "m3d/m3d.h"

namespace gl {
bool ImprimeSeErro();
}  // namespace gl

#define V_ERRO() do { gl::ImprimeSeErro(); } while (0)

namespace ent {

void AjustaCor(const EntidadeProto& proto, const ParametrosDesenho* pd) {
  const auto& cp = proto.cor();
  float cor[4] = { cp.r(), cp.g(), cp.b(), 1.0f };
  if (pd->has_alfa_translucidos()) {
    cor[3] = cp.a() * pd->alfa_translucidos();
  }
  if (pd->entidade_selecionada()) {
    RealcaCor(cor);
  }
  if (proto.morta()) {
    EscureceCor(cor);
  }
  MudaCorAlfa(cor);
}

void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, ParametrosDesenho* pd, const float* matriz_shear) {
  DesenhaObjetoProto(proto, VariaveisDerivadas(), pd, matriz_shear);
}

void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  switch (proto.tipo()) {
    case TE_ENTIDADE:
      DesenhaObjetoEntidadeProto(proto, vd, pd, matriz_shear);
      return;
    case TE_FORMA:
      DesenhaObjetoFormaProto(proto, vd, pd, matriz_shear);
      return;
    case TE_COMPOSTA: {
      DesenhaObjetoCompostoProto(proto, vd, pd, matriz_shear);
      return;
    }
    return;
  }
}

void Entidade::DesenhaObjetoEntidadeProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  AjustaCor(proto, pd);
  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  const auto& pos = proto.pos();
  if (!proto.has_info_textura() && !proto.has_modelo_3d()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*em_voo*/, true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
    gl::DesenhaVbo(g_vbos[VBO_PEAO]);
    return;
  }

  // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
  if (!proto.morta()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*em_voo*/, true  /*queda*/,
                (vd.altura_voo == 0.0f)  /*z*/,  // so desloca tijolo se nao estiver voando.
                proto, vd, pd, matriz_shear);
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 2);
    gl::Escala(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
    if (pd->entidade_selecionada()) {
      gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    }
    gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
  }

  if (proto.has_modelo_3d()) {
    const auto* vbo = vd.m3d->Modelo(proto.modelo_3d().id());
    if (vbo != nullptr) {
      // TODO vbo gravado
      gl::MatrizEscopo salva_matriz;
      MontaMatriz(true  /*em_voo*/, true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
      gl::DesenhaVbo(*vbo);
      return;
    } else {
      LOG(INFO) << "Modelo3d invalido: " << proto.modelo_3d().id();
    }
  }

  // Moldura da textura.
  bool achatar = (pd->desenha_texturas_para_cima() || proto.achatado()) && !proto.caida();
  gl::MatrizEscopo salva_matriz;
  MontaMatriz(true  /*em_voo*/, true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
  // Tijolo da moldura: nao roda selecionado (comentado).
  if (achatar) {
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10);
    //if (pd->entidade_selecionada()) {
    //  gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    //}
    gl::Roda(90.0f, -1.0f, 0.0f, 0.0f);
    gl::Escala(0.8f, 1.0f, 0.8f);
  } else {
    // Moldura da textura: acima do tijolo de base e achatado em Y (longe da camera).
    gl::Translada(0, 0, TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO_10);
    float angulo = 0;
    // So desenha a textura de frente pra entidades nao caidas.
    if (pd->texturas_sempre_de_frente() && !proto.caida()) {
      double dx = pos.x() - pd->pos_olho().x();
      double dy = pos.y() - pd->pos_olho().y();
      double r = sqrt(pow(dx, 2) + pow(dy, 2));
      angulo = (acosf(dx / r) * RAD_PARA_GRAUS);
      if (dy < 0) {
        // A funcao asin tem dois resultados mas sempre retorna o positivo [0, PI].
        // Se o vetor estiver nos quadrantes de baixo, inverte o angulo.
        angulo = -angulo;
      }
      gl::Roda(angulo - 90.0f, 0, 0, 1.0);
    }
    gl::MatrizEscopo salva_matriz;
    gl::Escala(1.0f, 0.1f, 1.0f);
    gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
  }

  // Tela onde a textura será desenhada face para o sul (nao desenha para sombra).
  GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (matriz_shear == nullptr && id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id_textura);
    gl::Normal(0.0f, -1.0f, 0.0f);
    Cor c;
    c.set_r(1.0f);
    c.set_g(1.0f);
    c.set_b(1.0f);
    c.set_a(pd->has_alfa_translucidos() ? pd->alfa_translucidos() : 1.0f);
    MudaCor(proto.morta() ? EscureceCor(c) : c);
    gl::DesenhaVbo(g_vbos[VBO_TELA_TEXTURA], GL_TRIANGLE_FAN);
    gl::Desabilita(GL_TEXTURE_2D);
  }
}

}  // namespace ent
