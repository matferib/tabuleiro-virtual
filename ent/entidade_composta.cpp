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

gl::VboNaoGravado Entidade::ExtraiVboComposta(const ent::EntidadeProto& proto) {
  gl::VboNaoGravado vbo;
  gl::VboNaoGravado sub_vbo;
  for (const auto& sub : proto.sub_forma()) {
    if (sub.tipo() == TE_COMPOSTA) {
      sub_vbo = std::move(ExtraiVboComposta(sub));
    } else if (sub.tipo() == TE_FORMA) {
      sub_vbo = std::move(ExtraiVboForma(sub));
    }
    vbo.Concatena(sub_vbo);
  }
  vbo.RodaX(proto.rotacao_x_graus());
  vbo.RodaY(proto.rotacao_y_graus());
  vbo.RodaZ(proto.rotacao_z_graus());
  vbo.Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  // Mundo.
  vbo.Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
  return vbo;
}

void Entidade::AtualizaTexturasEntidadesCompostasProto(
    const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central) {
  // Libera todas.
  if (novo_proto.sub_forma_size() != proto_atual->sub_forma_size()) {
    // Libera todos antigos e deixa do mesmo tamanho do novo.
    EntidadeProto dummy;
    for (auto& forma_velha : *proto_atual->mutable_sub_forma()) {
      VLOG(2) << "Liberando textura de sub forma para entidade composta";
      AtualizaTexturasProto(dummy, &forma_velha, central);
    }
    proto_atual->clear_sub_forma();
    for (int i = 0; i < novo_proto.sub_forma_size(); ++i) {
      proto_atual->add_sub_forma();
    }
  }
  for (int i = 0; i < novo_proto.sub_forma_size(); ++i) {
    VLOG(2) << "Atualizando textura de sub forma para entidade composta";
    AtualizaTexturasProto(novo_proto.sub_forma(i), proto_atual->mutable_sub_forma(i), central);
  }
}

void Entidade::DesenhaObjetoCompostoProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  gl::MatrizEscopo salva_matriz;
  if (matriz_shear != nullptr) {
    gl::MultiplicaMatriz(matriz_shear);
  }
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.pos().z() + 0.01f);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f);
  gl::Roda(proto.rotacao_y_graus(), 0, 1.0f, 0);
  gl::Roda(proto.rotacao_x_graus(), 1.0f, 0.0f, 0);
  gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  for (const auto& forma : proto.sub_forma()) {
    DesenhaObjetoProto(forma, vd, pd, nullptr);
  }
}

}  // namespace ent
