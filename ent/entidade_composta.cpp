#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"

#include "log/log.h"

namespace ent {

void Entidade::InicializaComposta(const ent::EntidadeProto& proto, VariaveisDerivadas* vd) {
}

void Entidade::AtualizaProtoComposta(
    const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd) {
}

gl::VbosNaoGravados Entidade::ExtraiVboComposta(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo) {
  gl::VbosNaoGravados vbos;
  gl::VbosNaoGravados sub_vbos;
  for (const auto& sub : proto.sub_forma()) {
    if (sub.tipo() == TE_COMPOSTA) {
      sub_vbos = ExtraiVboComposta(sub, vd, pd, true  /*mundo*/);
    } else if (sub.tipo() == TE_FORMA) {
      sub_vbos = ExtraiVboForma(sub, vd, pd, true   /*mundo*/);
    }
    vbos.Concatena(&sub_vbos);
  }
  if (mundo) {
    Matrix4 m;
    m.scale(proto.escala().x(), proto.escala().y(), proto.escala().z());
    m.rotateX(proto.rotacao_x_graus());
    m.rotateY(proto.rotacao_y_graus());
    m.rotateZ(proto.rotacao_z_graus());
    m.translate(proto.pos().x(), proto.pos().y(), proto.pos().z());
    vbos.Multiplica(m);
  }
  return vbos;
}

void Entidade::DesenhaObjetoCompostoProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd) {
  AlteraBlendEscopo blend_escopo(pd, proto.cor().a());
#if !VBO_COM_MODELAGEM
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);
  MontaMatriz(true, true, proto, vd, pd);
#endif
  GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
  }
  vd.vbos_gravados.Desenha();
  gl::Desabilita(GL_TEXTURE_2D);
#if 0 && DEBUG
      // Debug de normais escala deve estar em 1.0.
      if (pd->desenha_barra_vida() && !pd->has_picking_x()) {
        try {
          auto vn = vbo.ExtraiVboNormais();
          gl::DesenhaVbo(vn, GL_LINES);
        } catch (const std::exception& e) {
          LOG_EVERY_N(INFO, 1000) << "erro vbo: " << e.what();
        }
      }
#endif
}

bool Entidade::ColisaoComposta(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao) {
  return false;
}

}  // namespace ent
