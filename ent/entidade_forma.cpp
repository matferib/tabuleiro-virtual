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

gl::VboNaoGravado Entidade::ExtraiVboForma(const ent::EntidadeProto& proto) {
  gl::VboNaoGravado vbo;
  switch (proto.sub_tipo()) {
    case TF_CIRCULO: {
      vbo = std::move(gl::VboDisco(0.5f, 12));
    }
    break;
    case TF_CILINDRO: {
      vbo = std::move(gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12  /*fatias*/, 6  /*tocos*/));
      {
        gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
        vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
        vbo.Concatena(vbo_disco);
      }
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Translada(0.0f, 0.0f, 1.0f);
      vbo.Concatena(vbo_disco);
    }
    break;
    case TF_CONE: {
      vbo = std::move(gl::VboConeSolido(0.5f, 1.0f, 12, 6));
    }
    break;
    case TF_CUBO: {
      vbo = std::move(gl::VboCuboSolido(1.0f));
      vbo.Translada(0, 0, 0.5f);
    }
    break;
    case TF_PIRAMIDE: {
      vbo = std::move(gl::VboPiramideSolida(1.0f, 1.0f));
    }
    break;
    case TF_RETANGULO: {
      vbo = std::move(gl::VboRetangulo(1.0f));
    }
    break;
    case TF_ESFERA: {
      vbo = std::move(gl::VboEsferaSolida(0.5f, 24, 12));
    }
    break;
    case TF_LIVRE: {
    }
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";
  }
  const auto& c = proto.cor();
  vbo.AtribuiCor(c.r(), c.g(), c.b(), c.a());
  vbo.Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  vbo.RodaX(proto.rotacao_x_graus());
  vbo.RodaY(proto.rotacao_y_graus());
  vbo.RodaZ(proto.rotacao_z_graus());
  // Mundo.
  vbo.Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());

  return vbo;
}

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
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.pos().z() + 0.01f);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f);
  gl::Roda(proto.rotacao_y_graus(), 0, 1.0f, 0);
  gl::Roda(proto.rotacao_x_graus(), 1.0, 0.0f, 0);
  switch (proto.sub_tipo()) {
    case TF_CIRCULO: {
      if (matriz_shear != nullptr) {
        break;
      }
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(-1.0f, -40.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), 1.0f);
      gl::DesenhaVbo(g_vbos[VBO_DISCO]);
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
