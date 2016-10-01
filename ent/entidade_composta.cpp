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
  try {
    vd->vbos = std::move(ExtraiVbo(proto, *vd, &ParametrosDesenho::default_instance()));
    CorrigeVboRaiz(proto, vd);
  } catch (...) {
    LOG(WARNING) << "Nao consegui extrair VBO de objeto composto: " << proto.id() << ", vai falhar ao desenhar";
    vd->vbos.clear();
  }
}

void Entidade::AtualizaProtoComposta(
    const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd) {
}

std::vector<gl::VboNaoGravado> Entidade::ExtraiVboComposta(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd) {
  std::vector<gl::VboNaoGravado> vbos(1);
  std::vector<gl::VboNaoGravado> sub_vbos;
  int indice_corrente = 0;  // qual vbo esta sendo concatenado.
  for (const auto& sub : proto.sub_forma()) {
    if (sub.tipo() == TE_COMPOSTA) {
      sub_vbos = std::move(ExtraiVboComposta(sub, vd, pd));
    } else if (sub.tipo() == TE_FORMA) {
      sub_vbos = std::move(ExtraiVboForma(sub, vd, pd));
    }
    for (const auto& svbo : sub_vbos) {
      try {
        vbos[indice_corrente].Concatena(svbo);
      } catch (...) {
        LOG(INFO) << "Objeto grande, criando outro VBO para ele.";
        vbos[indice_corrente].Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
        vbos[indice_corrente].RodaX(proto.rotacao_x_graus());
        vbos[indice_corrente].RodaY(proto.rotacao_y_graus());
        vbos[indice_corrente].RodaZ(proto.rotacao_z_graus());
        vbos[indice_corrente].Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
        ++indice_corrente;
        vbos.push_back(svbo);
      }
    }
  }
  vbos[indice_corrente].Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  vbos[indice_corrente].RodaX(proto.rotacao_x_graus());
  vbos[indice_corrente].RodaY(proto.rotacao_y_graus());
  vbos[indice_corrente].RodaZ(proto.rotacao_z_graus());
  vbos[indice_corrente].Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
  return vbos;
}

void Entidade::DesenhaObjetoCompostoProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd) {
  gl::MatrizEscopo salva_matriz(false);
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.pos().z() + 0.01f, false);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f, false);
  gl::Roda(proto.rotacao_y_graus(), 0, 1.0f, 0, false);
  gl::Roda(proto.rotacao_x_graus(), 1.0f, 0.0f, 0, false);
  gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z(), false);
  if (!vd.vbos.empty()) {
    AlteraBlendEscopo blend_escopo(pd, proto.cor().a());
    for (const auto& vbo : vd.vbos) {
      GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() && vbo.tem_texturas() ?
        vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
      if (id_textura != GL_INVALID_VALUE) {
        gl::Habilita(GL_TEXTURE_2D);
        gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
      }
      gl::DesenhaVbo(vbo);
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
      gl::Desabilita(GL_TEXTURE_2D);
    }
  }
}

bool Entidade::ColisaoComposta(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao) {
  return false;
}

}  // namespace ent
