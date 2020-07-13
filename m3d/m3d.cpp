#include <string>
#include <set>
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

std::vector<std::string> Modelos3d::ModelosDisponiveis(bool global) {
  if (global) {
    std::vector<std::string> ret = arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D, ent::FiltroModelo3d);
    ret.push_back("builtin:esfera");
    ret.push_back("builtin:cilindro");
    ret.push_back("builtin:piramide");
    ret.push_back("builtin:cubo");
    return ret;
  } else {
    return arq::ConteudoDiretorio(arq::TIPO_MODELOS_3D_BAIXADOS, ent::FiltroModelo3d);
  }
}

struct Modelos3d::Interno {
  std::unordered_map<std::string, Modelo3d> modelos;
};

Modelos3d::Modelos3d(ntf::CentralNotificacoes* central) : interno_(new Interno), central_(central) {
  central_->RegistraReceptor(this);
}

Modelos3d::~Modelos3d() {
}

bool Modelos3d::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_ENVIAR_IDS_TABELAS_TEXTURAS_E_MODELOS_3D) {
    if (!notificacao.local()) {
      LOG(ERROR) << "TN_ENVIAR_IDS_TABELAS_TEXTURAS_E_MODELOS_3D deve ser sempre local";
      return false;
    }
    // Notificacao local: envia os ids de texturas locais para o servidor.
    auto n = ntf::NovaNotificacao(ntf::TN_REQUISITAR_MODELOS_3D);
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
    central_->AdicionaNotificacaoRemota(n.release());
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
    auto n = ntf::NovaNotificacao(ntf::TN_ENVIAR_MODELOS_3D);
    for (const auto& id : ids_faltantes) {
      std::vector<unsigned char> dados;
      try {
        LeModelo3d(id, &dados);
      } catch (...) {
        continue;
      }
      auto* info = n->add_info_modelo_3d();
      info->mutable_bits_crus()->append(dados.begin(), dados.end());
      info->set_id(id);
    }
    n->set_id_rede(notificacao.id_rede());
    VLOG(1) << "Enviando modelos faltantes a cliente " << n->id_rede();
    central_->AdicionaNotificacaoRemota(n.release());
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
    return true;
  } else if (notificacao.tipo() == ntf::TN_CARREGAR_MODELO_3D) {
    CarregaModelo3d(notificacao.entidade().modelo_3d().id());
  } else if (notificacao.tipo() == ntf::TN_DESCARREGAR_MODELO_3D) {
    DescarregaModelo3d(notificacao.entidade().modelo_3d().id());
  }
  return false;
}

const Modelo3d* Modelos3d::Modelo(const std::string& id) const {
  auto it = interno_->modelos.find(id);
  if (it == interno_->modelos.end()) {
    VLOG(3) << "nao achei: " << id;
    return nullptr;
  } else {
    return &(it->second);
  }
}

void Modelos3d::Recarrega() {
  // Percorre arquivos globais e baixados.
  VLOG(1) << "Recarregando modelos 3d";
  for (auto& it : interno_->modelos) {
    auto& modelo = it.second;
    if (modelo.contador > 0) {
      VLOG(1) << "Recarregando modelo " << it.first;
      modelo.contador = 0;
      CarregaModelo3d(it.first);
    }
  }
}

void Modelos3d::CarregaModelo3d(const std::string& id_interno) {
  auto it = interno_->modelos.find(id_interno);
  if (it != interno_->modelos.end() && it->second.contador > 0) {
    ++interno_->modelos[id_interno].contador;
    VLOG(1) << "CarregaModelo3d apenas incrementou contador: " << interno_->modelos[id_interno].contador;
    return;
  }
  // Ja cria, mesmo que invalido para evitar ficar lendo toda hora se der erro.
  interno_->modelos[id_interno].contador = 1;
  ntf::Notificacao n;
  if (id_interno.find("builtin:") == 0) {
    VLOG(1) << "Carregando builtin " << id_interno;
    gl::VboNaoGravado vbo_nao_gravado;
    if (id_interno == "builtin:esfera") {
      vbo_nao_gravado = gl::VboEsferaSolida(0.75f, 12, 12);
    } else if (id_interno == "builtin:cilindro") {
      vbo_nao_gravado = gl::VboCilindroSolido(0.75f, 1.0f, 12, 1);
    } else if (id_interno == "builtin:piramide") {
      vbo_nao_gravado = gl::VboPiramideSolida(0.75f, 1.0f);
    } else if (id_interno == "builtin:cubo") {
      vbo_nao_gravado = gl::VboCuboSolido(1.0f);
    } else {
      LOG(ERROR) << "modelo desconhecido: " << id_interno;
      return;
    }
    vbo_nao_gravado.AtribuiCor(1.0f, 1.0f, 1.0f, 1.0f);
    interno_->modelos[id_interno].vbos_nao_gravados = std::move(vbo_nao_gravado);
#if !VBO_COM_MODELAGEM
    interno_->modelos[id_interno].vbos_gravados.Grava(interno_->modelos[id_interno].vbos_nao_gravados);
#endif
  } else {
    try {
      std::string nome_arquivo = id_interno + ".binproto";
      VLOG(1) << "Carregando modelo 3d " << id_interno << " (" << nome_arquivo << ")";
      LeModelo3d(nome_arquivo, &n);
      if (n.tabuleiro().entidade_size() == 0) {
        LOG(ERROR) << "Falha abrindo arquivo, nao ha entidades";
        return;
      }
      n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_x();
      n.mutable_tabuleiro()->mutable_entidade(0)->mutable_pos()->clear_y();
      VLOG(2) << n.DebugString();
      // Extrai com mundo, por causa da escala. A posicao XY ja foi limpa.
      interno_->modelos[id_interno].vbos_nao_gravados =
        ent::Entidade::ExtraiVbo(n.tabuleiro().entidade(0), &ent::ParametrosDesenho::default_instance(), true  /*mundo*/);
#if !VBO_COM_MODELAGEM
      interno_->modelos[id_interno].vbos_gravados.Grava(interno_->modelos[id_interno].vbos_nao_gravados);
#endif
    } catch (...) {
      return;
    }
  }
}

void Modelos3d::DescarregaModelo3d(const std::string& id_interno) {
  if (interno_->modelos.find(id_interno) == interno_->modelos.end() ||
      interno_->modelos[id_interno].contador <= 0) {
    LOG(ERROR) << "Erro descarregando modelo 3d " << id_interno << ", "
        << (interno_->modelos.find(id_interno) == interno_->modelos.end() ? "nao existe" : "contador invalido");
    return;
  }
  if (--interno_->modelos[id_interno].contador == 0) {
    VLOG(1) << "Descarregando modelo 3d " << id_interno;
    interno_->modelos.erase(id_interno);
  } else {
    VLOG(1) << "Descarregando modelo 3d apenas decrementou contador: " << interno_->modelos[id_interno].contador ;
  }
}

}  // namespace m3d
