#include <string>
#define VLOG_NIVEL 1
#include <unordered_set>
#include <unordered_map>
#include "arq/arquivo.h"
#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "gltab/gl_vbo.h"
#include "log/log.h"
#include "m3d/m3d.h"
#include "ntf/notificacao.pb.h"

namespace m3d {

namespace {

// Le o conteudo cru de um arquivo de modelo.
void LeModelo3d(const std::string& nome_arquivo, std::vector<unsigned char>* dados) {
  std::string dados_str;
  try {
    arq::LeArquivo(arq::TIPO_MODELOS_3D, nome_arquivo, &dados_str);
  } catch (...) {
    VLOG(1) << "Arquivo global de modelo 3d não encontrado: " << nome_arquivo << ", tentando fallback baixado";
    try {
      arq::LeArquivo(arq::TIPO_MODELOS_3D_BAIXADOS, nome_arquivo, &dados_str);
    } catch (...) {
      LOG(ERROR) << "Falha lendo arquivo de modelo 3d: " << nome_arquivo;
      throw;
    }
  }
  dados->assign(dados_str.begin(), dados_str.end());
}

// Le o proto de um arquivo de modelo.
void LeModelo3d(const std::string& nome_arquivo, ntf::Notificacao* n) {
  std::vector<unsigned char> dados;
  LeModelo3d(nome_arquivo, &dados);
  std::string dados_string(dados.begin(), dados.end());
  if (!n->ParseFromString(dados_string)) {
    throw std::logic_error(std::string("Erro de parse do arquivo de modelo 3d") + nome_arquivo);
  }
}

}  // namespace

struct Modelos3d::Interno {
  std::unordered_map<std::string, gl::VboNaoGravado> vbos;
};

Modelos3d::Modelos3d(ntf::CentralNotificacoes* central) : interno_(new Interno), central_(central) {
  central_->RegistraReceptor(this);
  Recarrega();
}

Modelos3d::~Modelos3d() {
}

bool Modelos3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_ENVIAR_ID_TEXTURAS_E_MODELOS_3D) {
    if (!notificacao.local()) {
      LOG(ERROR) << "TN_ENVIAR_ID_TEXTURAS_E_MODELOS_3D deve ser sempre local";
      return false;
    }
    // Notificacao local: envia os ids de texturas locais para o servidor.
    auto* n = ntf::NovaNotificacao(ntf::TN_REQUISITAR_MODELOS_3D);
    // Percorre arquivos globais.
    std::vector<std::string> globais(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D, ent::FiltroModelo3d));
    for (const std::string& id : globais) {
      n->add_info_modelo_3d()->set_id(id);
    }
    // Percorre arquivos baixados.
    std::vector<std::string> baixados(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D_BAIXADOS, ent::FiltroModelo3d));
    for (const std::string& id : baixados) {
      n->add_info_modelo_3d()->set_id(id);
    }
    n->set_id_rede(notificacao.id_rede());
    n->set_servidor_apenas(true);
    // Envia para o servidor.
    VLOG(1) << "Enviando remoto TN_REQUISITAR_MODELOS_3D: " << n->DebugString();
    central_->AdicionaNotificacaoRemota(n);
    return true;
  } else if (notificacao.tipo() == ntf::TN_REQUISITAR_MODELOS_3D) {
    // Servidor recebendo de cliente.
    if (notificacao.local()) {
      LOG(ERROR) << "TN_REQUISITAR_MODELOS_3D deve ser apenas remota";
      return false;
    }
    VLOG(1) << "Recebendo de cliente TN_REQUISITAR_MODELOS_3D: " << notificacao.DebugString();
    std::unordered_set<std::string> ids;
    // Percorre arquivos globais.
    std::vector<std::string> globais(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D, ent::FiltroModelo3d));
    ids.insert(globais.begin(), globais.end());
    // Percorre arquivos baixados.
    std::vector<std::string> baixados(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D_BAIXADOS, ent::FiltroModelo3d));
    ids.insert(baixados.begin(), baixados.end());
    std::set<std::string> ids_cliente;
    std::vector<std::string> ids_faltantes;
    for (const auto& info : notificacao.info_modelo_3d()) {
      ids_cliente.insert(info.id());
    }
    for (const auto& id : ids) {
      if (ids_cliente.find(id) == ids_cliente.end()) {
        VLOG(1) << "Faltando modelo para cliente: " << id;
        ids_faltantes.push_back(id);
      }
    }
    if (ids_faltantes.empty()) {
      VLOG(1) << "Cliente tem todos os modelos.";
      return true;
    }
    auto* n = ntf::NovaNotificacao(ntf::TN_ENVIAR_MODELOS_3D);
    for (const auto& id : ids_faltantes) {
      auto* info = n->add_info_modelo_3d();
      std::vector<unsigned char> dados;
      LeModelo3d(id, &dados);
      info->mutable_bits_crus()->append(dados.begin(), dados.end());
      info->set_id(id);
    }
    n->set_id_rede(notificacao.id_rede());
    VLOG(1) << "Enviando modelos faltantes a cliente " << n->id_rede();
    central_->AdicionaNotificacaoRemota(n);
    return true;
  } else if (notificacao.tipo() == ntf::TN_ENVIAR_MODELOS_3D) {
    if (notificacao.local()) {
      LOG(ERROR) << "Notificacao TN_ENVIAR_MODELOS_3D so pode ser remota";
      return false;
    }
    VLOG(1) << "Recebendo TN_MODELOS_3D do servidor";
    for (const auto& info : notificacao.info_modelo_3d()) {
      // Salva bits crus do modelo 3d.
      arq::EscreveArquivo(arq::TIPO_MODELOS_3D_BAIXADOS, info.id(), info.bits_crus());
    }
    Recarrega();
    return true;
  }
  return false;
}

const gl::VboNaoGravado* Modelos3d::Modelo(const std::string& id) const {
  auto it = interno_->vbos.find(id);
  if (it == interno_->vbos.end()) {
    return nullptr;
  } else {
    return &(it->second);
  }
}

void Modelos3d::Recarrega() {
  // Percorre arquivos globais e baixados.
  interno_->vbos.clear();
  std::vector<std::string> globais(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D, ent::FiltroModelo3d));
  std::vector<std::string> baixados(arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D_BAIXADOS, ent::FiltroModelo3d));
  std::set<std::string> todos_modelos(globais.begin(), globais.end());
  todos_modelos.insert(baixados.begin(), baixados.end());
  if (todos_modelos.empty()) {
    VLOG(1) << "Modelos 3d vazio!";
    return;
  }
  for (const std::string& id : todos_modelos) {
    std::string id_interno(id.substr(0, id.find(".binproto")));
    try {
      ntf::Notificacao n;
      LeModelo3d(id, &n);
      n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_x();
      n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_y();
      VLOG(1) << "Carregando modelo 3d " << id_interno << "( " << id << ") : ";
      VLOG(2) << n.DebugString();
      interno_->vbos[id_interno] = std::move(ent::Entidade::ExtraiVbo(n.tabuleiro().entidade(0))[0]);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Falha carregando modelo 3d: " << id << ": " << e.what();
    }
  }
}

}  // namespace m3d
