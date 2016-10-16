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

void Entidade::InicializaForma(const ent::EntidadeProto& proto, VariaveisDerivadas* vd) {
}

void Entidade::AtualizaProtoForma(
    const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd) {
#if 0
  if (proto_novo.sub_tipo() == TF_LIVRE) {
    if (!vd->vbos.empty()) {
      // Extrai o VBO da forma livre.
      try {
        vd->vbos = ExtraiVboForma(proto_novo, *vd, &ParametrosDesenho::default_instance());
        CorrigeVboRaiz(proto_novo, vd);
      } catch (...) {
        LOG(WARNING) << "Falha atualizando VBO de forma LIVRE, renderizacao sera custosa.";
        // sem VBO, vai desenhar na marra.
      }
    }
  }
#endif
}

gl::VbosNaoGravados Entidade::ExtraiVboForma(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo) {
  gl::VboNaoGravado vbo;
  switch (proto.sub_tipo()) {
    case TF_CIRCULO: {
      vbo = gl::VboDisco(0.5f, 12);
    }
    break;
    case TF_CILINDRO: {
      vbo = gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12  /*fatias*/, 6  /*tocos*/);
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
      vbo = gl::VboConeSolido(0.5f/*raio*/, 1.0f  /*altura*/, 12  /*fatias*/, 6  /*tocos*/);
      {
        gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
        vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
        vbo.Concatena(vbo_disco);
      }
    }
    break;
    case TF_CUBO: {
      vbo = gl::VboCuboSolido(1.0f);
      vbo.Translada(0, 0, 0.5f);
    }
    break;
    case TF_PIRAMIDE: {
      vbo = gl::VboPiramideSolida(1.0f, 1.0f);
    }
    break;
    case TF_RETANGULO: {
      vbo = gl::VboRetangulo(1.0f);
    }
    break;
    case TF_TRIANGULO: {
      vbo = gl::VboTriangulo(1.0f);
    }
    break;
    case TF_ESFERA: {
      vbo = gl::VboEsferaSolida(0.5f, 24, 12);
    }
    break;
    case TF_LIVRE: {
      // Livre eh um pouco diferente por causa da escala. Isso vai dar problema de se concatenar com
      // outros tipos de objeto. Por enquanto, fica assim.
      std::vector<std::pair<float, float>> v;
      for (const auto& p : proto.ponto()) {
        v.push_back(std::make_pair(p.x(), p.y()));
      }
      vbo = std::move(gl::VboLivre(v, TAMANHO_LADO_QUADRADO * proto.escala().z()));
      const auto& c = proto.cor();
      vbo.AtribuiCor(c.r(), c.g(), c.b(), c.a());
      if (mundo) {
        vbo.RodaX(proto.rotacao_x_graus());
        vbo.RodaY(proto.rotacao_y_graus());
        vbo.RodaZ(proto.rotacao_z_graus());
        vbo.Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
      }
      return gl::VbosNaoGravados(std::move(vbo));
    }
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";
      throw std::logic_error("Forma de desenho invalida");
  }
  const auto& c = proto.cor();
  vbo.AtribuiCor(c.r(), c.g(), c.b(), c.a());
  if (mundo) {
    vbo.Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
    vbo.RodaX(proto.rotacao_x_graus());
    vbo.RodaY(proto.rotacao_y_graus());
    vbo.RodaZ(proto.rotacao_z_graus());
    vbo.Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
  }
  return gl::VbosNaoGravados(std::move(vbo));
}

bool TipoForma2d(TipoForma tipo) {
  switch (tipo) {
    case TF_LIVRE:
    case TF_RETANGULO:
    case TF_CIRCULO:
    case TF_TRIANGULO:
      return true;
    default:
      return false;
  }
}

// static
Matrix4 Entidade::MontaMatrizModelagemForma(
    bool queda,
    bool transladar_z,  // nao usado.
    const EntidadeProto& proto,
    const VariaveisDerivadas& vd,
    const ParametrosDesenho* pd,
    bool posicao_mundo) {

  Matrix4 matrix;
  switch (proto.sub_tipo()) {
    case TF_CIRCULO:
      matrix.scale(proto.escala().x(), proto.escala().y(), 1.0f);
    break;
    case TF_CILINDRO: {
      // Aqui ignoro as transformacoes para os circulos que fecham o cilindro.
      matrix.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    }
    break;
    case TF_CONE: {
      // Mesma coisa que cilindro.
      matrix.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    }
    break;
    case TF_CUBO: {
      matrix.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    }
    break;
    case TF_PIRAMIDE: {
      matrix.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    }
    break;
    case TF_RETANGULO: {
      matrix.scale(proto.escala().x(), proto.escala().y(), 1.0f);
    }
    break;
    case TF_TRIANGULO: {
      matrix.scale(proto.escala().x(), proto.escala().y(), 1.0f);
      matrix.translate(0.0f, -proto.escala().y() / 2.0f, 0.0f);
    }
    break;
    case TF_ESFERA: {
      matrix.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    }
    break;
    case TF_LIVRE:
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";

  }

  if (pd != nullptr && pd->has_translacao_efeito()) {
    const auto& te = pd->translacao_efeito();
    matrix.translate(te.x(), te.y(), te.z());
  }
  if (pd != nullptr && pd->has_rotacao_efeito()) {
    const auto& re = pd->rotacao_efeito();
    if (re.has_x()) {
      matrix.rotateX(re.x());
    } else if (re.has_y()) {
      matrix.rotateY(re.y());
    } else if (re.has_z()) {
      matrix.rotateZ(re.z());
    }
  }

  bool computar_queda = queda && (vd.angulo_disco_queda_graus > 0);
  if (computar_queda) {
    // Roda sobre o eixo X negativo para cair com a face para cima.
    matrix.rotateX(- vd.angulo_disco_queda_graus);
    // Roda pra direcao de queda.
    const auto& dq = proto.direcao_queda();
    if (dq.x() != 0.0f || dq.y() != 0) {
      // Como a queda é sobre o eixo X, subtrai 90 para a direcao ficar certa.
      float direcao_queda_graus = VetorParaRotacaoGraus(dq) - 90.0f;
      matrix.rotateZ(direcao_queda_graus);
    }
  }

  if (posicao_mundo) {
    float multiplicador = CalculaMultiplicador(proto.tamanho());
    if (pd != nullptr && pd->has_escala_efeito()) {
      const auto& ee = pd->escala_efeito();
      matrix.scale(ee.x(), ee.y(), ee.z());
    }
    matrix.scale(multiplicador);

    if (!computar_queda) {
      matrix.rotateX(proto.rotacao_x_graus());
      matrix.rotateY(proto.rotacao_y_graus());
      matrix.rotateZ(proto.rotacao_z_graus());
    }
    const auto& pos = proto.pos();
    matrix.translate(pos.x(), pos.y(), pos.z());
  }
  return matrix;
}

void Entidade::DesenhaObjetoFormaProto(const EntidadeProto& proto,
                                       const VariaveisDerivadas& vd,
                                       ParametrosDesenho* pd) {
  bool usar_stencil = false;
  if (proto.sub_tipo() == TF_LIVRE) {
    usar_stencil = !pd->desenha_mapa_sombras() && !pd->desenha_mapa_oclusao() && !pd->has_picking_x();
    if (usar_stencil) {
      LigaStencil();
    }
  }
#if !VBO_COM_MODELAGEM
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);
  Matrix4 m = MontaMatrizModelagemForma(true, true, proto, vd, pd);
  gl::MultiplicaMatriz(m.get());
#endif
  AlteraBlendEscopo blend_escopo(pd, proto.cor().a());
  GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
  }
  if (vd.vbos_gravados.Vazio()) {
    // Para formas sendo desenhada, nao ha um VBO ainda.
    gl::VbosNaoGravados vbo(ExtraiVboForma(proto, vd, pd, VBO_COM_MODELAGEM));
    vbo.Desenha();
  } else {
    vd.vbos_gravados.Desenha();
  }
  gl::Desabilita(GL_TEXTURE_2D);
  if (usar_stencil) {
    float xi, yi, xs, ys;
    LimitesLinha3d(proto.ponto(), TAMANHO_LADO_QUADRADO * proto.escala().z(), &xi, &yi, &xs, &ys);
    //LOG_EVERY_N(INFO, 100) << "Limites: xi: " << xi << ", yi: " << yi << ", xs: " << xs << ", ys: " << ys;
#if VBO_COM_MODELAGEM
    gl::MatrizEscopo salva_matriz(false);
    gl::MultiplicaMatriz(MontaMatrizModelagemForma(false, false, proto, vd, pd).get());
#endif
    float cor[] = { proto.cor().r(), proto.cor().g(), proto.cor().b(), proto.cor().a() };
    DesenhaStencil3d(xi, yi, xs, ys, cor);
  }
}

bool Entidade::ColisaoForma(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao) {
  switch (proto.sub_tipo()) {
    case TF_CUBO: {
      break;
    }
    default:
      ;
  }
  return false;
}

}  // namespace ent
