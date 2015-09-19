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
    vd->vbos = std::move(ExtraiVbo(proto));
    CorrigeVboRaiz(proto, vd);
  } catch (...) {
    LOG(WARNING) << "Nao consegui extrair VBO de objeto composto: " << proto.id() << ", renderizacao sera lenta";
    vd->vbos.clear();
  }
}

void Entidade::AtualizaProtoComposta(
    const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd) {
}

const std::vector<gl::VboNaoGravado> Entidade::ExtraiVboComposta(const ent::EntidadeProto& proto) {
  std::vector<gl::VboNaoGravado> vbos(1);
  std::vector<gl::VboNaoGravado> sub_vbos;
  int indice_corrente = 0;  // qual vbo esta sendo concatenado.
  for (const auto& sub : proto.sub_forma()) {
    if (sub.tipo() == TE_COMPOSTA) {
      sub_vbos = std::move(ExtraiVboComposta(sub));
    } else if (sub.tipo() == TE_FORMA) {
      sub_vbos = std::move(ExtraiVboForma(sub));
    }
    for (const auto& svbo : sub_vbos) {
      try {
        vbos[indice_corrente].Concatena(svbo);
      } catch (...) {
        LOG(INFO) << "Objeto grande, criando outro VBO para ele.";
        vbos[indice_corrente].RodaX(proto.rotacao_x_graus());
        vbos[indice_corrente].RodaY(proto.rotacao_y_graus());
        vbos[indice_corrente].RodaZ(proto.rotacao_z_graus());
        vbos[indice_corrente].Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
        // Mundo.
        vbos[indice_corrente].Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
        ++indice_corrente;
        vbos.push_back(svbo);
      }
    }
  }
  vbos[indice_corrente].RodaX(proto.rotacao_x_graus());
  vbos[indice_corrente].RodaY(proto.rotacao_y_graus());
  vbos[indice_corrente].RodaZ(proto.rotacao_z_graus());
  vbos[indice_corrente].Escala(proto.escala().x(), proto.escala().y(), proto.escala().z());
  // Mundo.
  vbos[indice_corrente].Translada(proto.pos().x(), proto.pos().y(), proto.pos().z());
  return vbos;
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
  gl::MatrizEscopo salva_matriz(false);
  if (matriz_shear != nullptr) {
    gl::MultiplicaMatriz(matriz_shear, false);
  }
  gl::Translada(proto.pos().x(), proto.pos().y(), proto.pos().z() + 0.01f, false);
  gl::Roda(proto.rotacao_z_graus(), 0, 0, 1.0f, false);
  gl::Roda(proto.rotacao_y_graus(), 0, 1.0f, 0, false);
  gl::Roda(proto.rotacao_x_graus(), 1.0f, 0.0f, 0, false);
  gl::Escala(proto.escala().x(), proto.escala().y(), proto.escala().z(), false);
  if (!vd.vbos.empty()) {
    if (pd->has_alfa_translucidos()) {
      gl::FuncaoMistura(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    }
    for (const auto& vbo : vd.vbos) {
      gl::DesenhaVbo(vbo);
    }
    if (pd->has_alfa_translucidos()) {
      gl::FuncaoMistura(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
  } else {
    for (const auto& forma : proto.sub_forma()) {
      DesenhaObjetoProto(forma, vd, pd, nullptr);
    }
  }
}

}  // namespace ent
