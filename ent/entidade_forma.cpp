#include <algorithm>
#include <cmath>
#include <memory>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"


namespace ent {

void AjustaCor(const EntidadeProto& proto, const ParametrosDesenho* pd);

void Entidade::DesenhaObjetoFormaProto(const EntidadeProto& proto,
                                       const VariaveisDerivadas& vd,
                                       ParametrosDesenho* pd,
                                       const float* matriz_shear) {
  bool transparencias = pd->transparencias() &&
                        ((pd->has_alfa_translucidos() && pd->alfa_translucidos() < 1.0f) || (proto.cor().a() < 1.0f));
  AjustaCor(proto, pd);
  gl::MatrizEscopo salva_matriz;
  if (matriz_shear != nullptr) {
    gl::MultiplicaMatriz(matriz_shear);
  }
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.translacao_z() + 0.01f);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f);
  gl::Roda(proto.rotacao_y_graus(), 0, 1.0f, 0);
  switch (proto.sub_tipo()) {
    case TF_CIRCULO: {
      if (matriz_shear != nullptr) {
        break;
      }
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(-1.0f, -40.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), 1.0f);
      gl::DesenhaVbo(g_vbos[VBO_DISCO], GL_TRIANGLE_FAN);
    }
    break;
    case TF_CILINDRO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::DesenhaVbo(g_vbos[VBO_CILINDRO]);
      {
        gl::MatrizEscopo salva;
        gl::Escala(-1.0f, 1.0f, -1.0f);
        gl::DesenhaVbo(g_vbos[VBO_DISCO], GL_TRIANGLE_FAN);
      }
      gl::Translada(0.0f, 0.0f, 1.0f);
      gl::DesenhaVbo(g_vbos[VBO_DISCO], GL_TRIANGLE_FAN);
    }
    break;
    case TF_CONE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::DesenhaVbo(g_vbos[VBO_CONE]);
    }
    break;
    case TF_CUBO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Translada(0, 0, proto.escala().z() / 2.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::DesenhaVbo(g_vbos[VBO_CUBO]);
    }
    break;
    case TF_PIRAMIDE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::DesenhaVbo(g_vbos[VBO_PIRAMIDE]);
    }
    break;
    case TF_RETANGULO: {
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(-1.0f, -40.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), 1.0f);
      GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
          vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
      if (id_textura != GL_INVALID_VALUE) {
        gl::Habilita(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, id_textura);
      }
      gl::DesenhaVbo(g_vbos[VBO_RETANGULO], GL_TRIANGLE_FAN);
      gl::Desabilita(GL_TEXTURE_2D);
    }
    break;
    case TF_ESFERA: {
      // Usar x como base para achatamento.
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::DesenhaVbo(g_vbos[VBO_ESFERA]);
    }
    break;
    case TF_LIVRE: {
      if (matriz_shear != nullptr) {
        break;
      }
      if (transparencias) {
        LigaStencil();
      }
      {
        // Durante preenchimento do stencil nao pode usar o offset pois ele se aplicara ao retangulo da tela toda.
        // Portanto escopo deve terminar aqui.
        gl::HabilitaEscopo offset_escopo(GL_POLYGON_OFFSET_FILL);
        gl::DesvioProfundidade(-1.0, -40.0f);
        DesenhaLinha3d(proto.ponto(), TAMANHO_LADO_QUADRADO * proto.escala().z());
      }
      if (transparencias) {
        DesenhaStencil();
      }
    }
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";
  }
}

}  // namespace ent
