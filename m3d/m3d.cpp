#include <string>
#include <unordered_map>
#include "arq/arquivo.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "log/log.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.pb.h"

namespace m3d {

struct Modelos3d::Interno {
  std::unordered_map<std::string, gl::VboNaoGravado> vbos;
};

Modelos3d::Modelos3d() : interno_(new Interno) {
  try {
    ntf::Notificacao n;
    arq::LeArquivoBinProto(arq::TIPO_MODELO_3D, "phaerimm.binproto", &n);
    n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_x();
    n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_y();
    LOG(INFO) << "phaerimm: " << n.DebugString();
    interno_->vbos["phaerimm"] = std::move(ent::Entidade::ExtraiVbo(n.tabuleiro().entidade(0))[0]);
    n.Clear();

    arq::LeArquivoBinProto(arq::TIPO_MODELO_3D, "orc.binproto", &n);
    //LOG(INFO) << "orc: " << n.DebugString();
    interno_->vbos["orc"] = std::move(ent::Entidade::ExtraiVbo(n.tabuleiro().entidade(0))[0]);
    //LOG(INFO) << interno_->vbos["orc"].ParaString();
    n.Clear();

    arq::LeArquivoBinProto(arq::TIPO_MODELO_3D, "geo.binproto", &n);
    //LOG(INFO) << "geo: " << n.DebugString();
    interno_->vbos["geo"] = std::move(ent::Entidade::ExtraiVbo(n.tabuleiro().entidade(0))[0]);
    n.Clear();
    //LOG(INFO) << interno_->vbos["geo"].ParaString();
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha carregando orc ou geo: " << e.what();
  }
}

Modelos3d::~Modelos3d() {
}

const gl::VboNaoGravado* Modelos3d::Modelo(const std::string& id) const {
  auto it = interno_->vbos.find(id);
  if (it == interno_->vbos.end()) {
    return nullptr;
  } else {
    return &(it->second);
  }
}

}  // namespace m3d
