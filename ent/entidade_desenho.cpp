/** Implementacao dos varios tipos de desenho da classe Entidade. */

#include <algorithm>
#include <cmath>
#include <memory>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"

namespace ent {

void AjustaCor(const EntidadeProto& proto, ParametrosDesenho* pd) {
  auto cor = proto.cor();
  if (pd->has_alfa_translucidos()) {
    cor.set_a(cor.a() * pd->alfa_translucidos());
  }
  if (pd->entidade_selecionada()) {
    RealcaCor(&cor);
  }
  if (proto.morta()) {
    EscureceCor(&cor);
  }
  MudaCor(cor);
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

void Entidade::DesenhaObjetoCompostoProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  gl::MatrizEscopo salva_matriz;
  if (matriz_shear != nullptr) {
    gl::MultiplicaMatriz(matriz_shear);
  }
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.translacao_z() + 0.01f);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f);
  gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  for (const auto& forma : proto.sub_forma()) {
    DesenhaObjetoProto(forma, vd, pd, nullptr);
  }
}

void Entidade::DesenhaObjetoEntidadeProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  AjustaCor(proto, pd);
  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  const auto& pos = proto.pos();
  if (!proto.has_info_textura()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*em_voo*/, true  /*queda*/, proto, vd, pd, matriz_shear);
    gl::ConeSolido(TAMANHO_LADO_QUADRADO_2 - 0.2, ALTURA, NUM_FACES, NUM_LINHAS);
    gl::Translada(0, 0, ALTURA);
    gl::EsferaSolida(TAMANHO_LADO_QUADRADO_2 - 0.4, NUM_FACES, NUM_FACES / 2.0f);
    return;
  }

  // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
  {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*em_voo*/, true  /*queda*/, proto, vd, pd, matriz_shear);
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 2);
    gl::Escala(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
    if (pd->entidade_selecionada()) {
      gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    }
    gl::CuboSolido(TAMANHO_LADO_QUADRADO);
  }

  bool achatar = pd->desenha_texturas_para_cima() || proto.achatado();
  gl::MatrizEscopo salva_matriz;
  MontaMatriz(true  /*em_voo*/, true  /*queda*/, proto, vd, pd, matriz_shear);
  // Tijolo da moldura: nao roda selecionado (comentado).
  if (achatar) {
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10);
    //if (pd->entidade_selecionada()) {
    //  gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    //}
    gl::Roda(-90.0f, 1.0f, 0.0f, 0.0f);
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
    gl::CuboSolido(TAMANHO_LADO_QUADRADO);
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
    const unsigned short indices[] = { 0, 1, 2, 3 };
    const float vertices[] = {
      -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, -TAMANHO_LADO_QUADRADO_2,
      TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, -TAMANHO_LADO_QUADRADO_2,
      TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, TAMANHO_LADO_QUADRADO_2,
      -TAMANHO_LADO_QUADRADO_2, -TAMANHO_LADO_QUADRADO_2 / 10.0f - 0.01f, TAMANHO_LADO_QUADRADO_2,
    };
    const float vertices_texel[] = {
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
    };
    gl::HabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
    gl::PonteiroVertices(3, GL_FLOAT, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, vertices_texel);
    gl::DesenhaElementos(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_SHORT, indices);
    gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
    gl::DesabilitaEstadoCliente(GL_TEXTURE_COORD_ARRAY);
    gl::Desabilita(GL_TEXTURE_2D);
  }
}

void Entidade::DesenhaObjetoFormaProto(const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  bool transparencias = pd->transparencias() && ((pd->has_alfa_translucidos() && pd->alfa_translucidos() < 1.0f) || (proto.cor().a() < 1.0f));
  AjustaCor(proto, pd);
  gl::MatrizEscopo salva_matriz;
  if (matriz_shear != nullptr) {
    gl::MultiplicaMatriz(matriz_shear);
  }
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.translacao_z() + 0.01f);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f);
  switch (proto.sub_tipo()) {
    case TF_CIRCULO: {
      if (matriz_shear != nullptr) {
        break;
      }
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(-1.0f, -40.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), 1.0f);
      DesenhaDisco(0.5f, 12);
    }
    break;
    case TF_CILINDRO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::CilindroSolido(0.5f  /*radius_base*/, 0.5f  /*radius_top*/, 1.0f  /*height*/, 20  /*slices*/, 20  /*stacks*/);
    }
    break;
    case TF_CONE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::ConeSolido(0.5f, 1.0f, 20  /*slices*/, 20  /*stacks*/);
    }
    break;
    case TF_CUBO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Translada(0, 0, proto.escala().z() / 2.0f);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      gl::CuboSolido(1.0f);
    }
    break;
    case TF_PIRAMIDE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x() / 2.0f, proto.escala().y() / 2.0f, proto.escala().z());
      const unsigned short indices[] = {
          0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
      };
      const float normais[] = {
        // Topo.
        0.0f, 0.0, 1.0f,
        // Face sul
        0.0f, -0.5, 0.5f,
        0.0f, -0.5, 0.5f,
        // Face leste.
        0.5f, 0.0f, 0.5f,
        0.5f, 0.0f, 0.5f,
        // Face norte.
        0.0f, 0.5f, 0.5f,
        0.0f, 0.5f, 0.5f,
        // Face Oeste.
        -0.5f, 0.0f, 0.5f,
        -0.5f, 0.0f, 0.5f,
      };
      const float vertices[] = {
        // Topo.
        0.0f, 0.0f, 1.0f,
        // Face sul.
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        // Face leste.
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        // Face norte.
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        // Face Oeste.
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
      };
      gl::HabilitaEstadoCliente(GL_VERTEX_ARRAY);
      gl::HabilitaEstadoCliente(GL_NORMAL_ARRAY);
      gl::PonteiroNormais(GL_FLOAT, normais);
      gl::PonteiroVertices(3, GL_FLOAT, vertices);
      gl::DesenhaElementos(GL_TRIANGLE_FAN, 9, GL_UNSIGNED_SHORT, indices);
      gl::DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
      gl::DesabilitaEstadoCliente(GL_VERTEX_ARRAY);
    }
    break;
    case TF_RETANGULO: {
      if (matriz_shear != nullptr) {
        break;
      }
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(-1.0f, -40.0f);
      float x = proto.escala().x() / 2.0f;
      float y = proto.escala().y() / 2.0f;
      gl::Normal(0.0f, 0.0f, 1.0f);
      gl::Retangulo(-x, -y, x, y);
    }
    break;
    case TF_ESFERA: {
      // Usar x como base para achatamento.
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
      // TODO fazer baseado nas escalas?
      int num_faces = std::max(4, static_cast<int>((proto.escala().x() + proto.escala().y()) * 2));
      int num_tocos = std::max(4, static_cast<int>(proto.escala().z() * 4));
      gl::EsferaSolida(0.5f  /*raio*/,  num_faces, num_tocos);
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
