#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/tabelas.h"
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

EntidadeProto Entidade::RemoveSubForma(int indice) {
  if (indice < 0 || indice >= proto_.sub_forma_size()) return EntidadeProto::default_instance();
  EntidadeProto sub_forma = proto_.sub_forma(indice);
  proto_.mutable_sub_forma()->DeleteSubrange(indice, 1);
  DecompoeFilho(MatrizDecomposicaoPai(proto_), &sub_forma);
  return sub_forma;
}

gl::VbosNaoGravados Entidade::ExtraiVboComposta(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo) {
  gl::VbosNaoGravados vbos;
  gl::VbosNaoGravados sub_vbos;
  for (const auto& sub : proto.sub_forma()) {
    // As subformas sempre extraem com modelagem de mundo, porque elas sao relativas ao pai.
    if (sub.tipo() == TE_COMPOSTA) {
      sub_vbos = ExtraiVboComposta(sub, vd, pd, true  /*mundo*/);
    } else if (sub.tipo() == TE_FORMA) {
      sub_vbos = ExtraiVboForma(sub, vd, pd, proto.info_textura().respeitar_texturas_sub_objetos(), true   /*mundo*/);
    }
    vbos.Concatena(&sub_vbos);
  }
  if (proto.has_cor()) {
    const auto& c = proto.cor();
    vbos.MesclaCores(c.r(), c.g(), c.b(), c.a());
  } else {
    vbos.MesclaCores(1.0f, 1.0f, 1.0f, 1.0f);
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
  if (pd->has_desenha_objeto_desmembrado() && pd->desenha_objeto_desmembrado() == proto.id()) {
    // Usado em picking para decomposicao de objetos.
    pd->clear_desenha_objeto_desmembrado();
    pd->set_regera_vbo(true);

    // Transformacoes do objeto pai.
    Matrix4 m_pai = MatrizDecomposicaoPai(proto);
    for (int i = 0; i < proto.sub_forma_size(); ++i) {
      auto sub_forma = proto.sub_forma(i);
      DecompoeFilho(m_pai, &sub_forma);
      std::unique_ptr<Entidade> s(NovaEntidade(sub_forma, Tabelas::Unica(), /*tabuleiro=*/nullptr, vd.texturas, vd.m3d, nullptr, pd));
      s->Atualiza(0);
      gl::CarregaNome(i);
      s->DesenhaObjeto(pd);
    }
    pd->set_desenha_objeto_desmembrado(proto.id());
    pd->clear_regera_vbo();
    return;
  }

  std::unique_ptr<MisturaPreNevoaEscopo> blend_escopo;
  if (proto.cor().has_r() || proto.cor().a() < 1.0f || pd->has_alfa_translucidos() || pd->entidade_selecionada()) {
    blend_escopo.reset(new MisturaPreNevoaEscopo(pd->entidade_selecionada() ? CorRealcada(proto.cor()) : proto.cor(), pd));
  }

  //gl::MatrizEscopo salva_matriz(gl::MATRIZ_MODELAGEM);
  //gl::MultiplicaMatriz(vd.matriz_modelagem.get());
  GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    gl::TexturaBump(proto.info_textura().textura_bump());
    // Para num pegar lixo de outros objetos.
    gl::MatrizEscopo salva_matriz_textura(gl::MATRIZ_AJUSTE_TEXTURA);
    gl::AtualizaMatrizes();
  }
  vd.vbos_gravados.Desenha();
  gl::TexturaBump(false);
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
