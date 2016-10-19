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
      vbo = gl::VboLivre(v, TAMANHO_LADO_QUADRADO * proto.escala().z());
    }
    break;
    default:
      LOG(ERROR) << "Forma de desenho invalida";
      throw std::logic_error("Forma de desenho invalida");
  }
  const auto& c = proto.cor();
  vbo.AtribuiCor(c.r(), c.g(), c.b(), c.a());
  if (mundo) {
    vbo.Multiplica(MontaMatrizModelagemForma(true, true, proto, vd, pd, true));
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
    bool transladar_z,  // nao usado, sempre true.
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
      matrix.translate(0.0f, 0.0f, 0.5f);
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
      // nao faz nada, pois nao possui escala.
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
    if (pd != nullptr && pd->has_escala_efeito()) {
      const auto& ee = pd->escala_efeito();
      matrix.scale(ee.x(), ee.y(), ee.z());
    }

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
#if VBO_COM_MODELAGEM
  bool usar_stencil = false;
  if (proto.sub_tipo() == TF_LIVRE) {
    usar_stencil = !pd->desenha_mapa_sombras() && !pd->desenha_mapa_oclusao() && !pd->has_picking_x();
    if (usar_stencil) {
      LigaStencil();
    }
  }
  Cor c;
  c.set_a(proto.cor().a());  // a gente so quer o alfa aqui.
  AlteraBlendEscopo blend_escopo(pd, c);
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
    gl::MatrizEscopo salva_matriz(false);
    gl::MultiplicaMatriz(MontaMatrizModelagemForma(false, false, proto, vd, pd).get());
    float cor[] = { proto.cor().r(), proto.cor().g(), proto.cor().b(), proto.cor().a() };
    DesenhaStencil3d(xi, yi, xs, ys, cor);
  }
#else
  AjustaCor(proto, pd);
  gl::MatrizEscopo salva_matriz(false);
  gl::MultiplicaMatriz(MontaMatrizModelagemForma(false  /*queda*/, true  /*translacao_z*/, proto, vd, pd).get());
  bool usar_textura = proto.sub_tipo() == TF_CUBO || proto.sub_tipo() == TF_CIRCULO || proto.sub_tipo() == TF_PIRAMIDE ||
                      proto.sub_tipo() == TF_RETANGULO || proto.sub_tipo() == TF_TRIANGULO;
  if (usar_textura) {
    GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
        vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
    if (id_textura != GL_INVALID_VALUE) {
      gl::Habilita(GL_TEXTURE_2D);
      gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    }
  }
  switch (proto.sub_tipo()) {
    case TF_CILINDRO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::DesenhaVbo(g_vbos[VBO_CILINDRO_FECHADO]);
    }
    break;
    case TF_CONE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::DesenhaVbo(g_vbos[VBO_CONE_FECHADO]);
    }
    break;
    case TF_CUBO: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::DesenhaVbo(g_vbos[VBO_CUBO]);
    }
    break;
    case TF_CIRCULO: {
      gl::DesenhaVbo(g_vbos[VBO_DISCO]);
    }
    break;
    case TF_PIRAMIDE: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::DesenhaVbo(g_vbos[VBO_PIRAMIDE]);
    }
    break;
    case TF_RETANGULO: {
      gl::DesenhaVbo(g_vbos[VBO_RETANGULO], GL_TRIANGLE_FAN);
    }
    break;
    case TF_TRIANGULO: {
      gl::DesenhaVbo(g_vbos[VBO_TRIANGULO], GL_TRIANGLES);
    }
    break;
    case TF_ESFERA: {
      gl::HabilitaEscopo habilita_normalizacao(GL_NORMALIZE);
      gl::DesenhaVbo(g_vbos[VBO_ESFERA]);
    }
    break;
    case TF_LIVRE: {
      // Usar stencil nos dois casos (transparente ou solido) para que a cor do AjustaCor funcione.
      // caso contrario, ao atualizar a cor do desenho livre, o VBO tera que ser regerado.
      // Para picking, deve-se ignorar o stencil tb.
      bool usar_stencil = !pd->desenha_mapa_sombras() && !pd->desenha_mapa_oclusao() && !pd->has_picking_x();
      if (usar_stencil) {
        LigaStencil();
      }
      vd.vbos_nao_gravados.Desenha();
      if (usar_stencil) {
        float xi, yi, xs, ys;
        LimitesLinha3d(proto.ponto(), TAMANHO_LADO_QUADRADO * proto.escala().z(), &xi, &yi, &xs, &ys);
        //LOG_EVERY_N(INFO, 100) << "Limites: xi: " << xi << ", yi: " << yi << ", xs: " << xs << ", ys: " << ys;
        DesenhaStencil3d(xi, yi, xs, ys);
      }
    }
    break;
    default: ;
  }
  gl::Desabilita(GL_TEXTURE_2D);
#endif
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
