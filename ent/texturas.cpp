#include "ent/texturas.h"
#include "log/log.h"
#include "ntf/notificacao.pb.h"

namespace ent {

struct Texturas::InfoTextura {
  InfoTextura() : contador(1), textura(nullptr) {}
  ~InfoTextura() { /*delete textura;*/ }
  int contador;
  void* textura;
};

Texturas::Texturas(ntf::CentralNotificacoes* central) {
  central_ = central;
  central_->RegistraReceptor(this);
}

Texturas::~Texturas() {}

bool Texturas::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_CARREGAR_TEXTURA:
      CarregaTextura(notificacao.endereco());
      return true;
    case ntf::TN_LIBERAR_TEXTURA:
      DescarregaTextura(notificacao.endereco());
      return true;
    default: ;
  }
  return false;
}

const void* Texturas::Textura(const std::string& id) const {
  const InfoTextura* info = Info(id);
  if (info == nullptr) {
    return nullptr;
  }
  return info->textura;
}

Texturas::InfoTextura* Texturas::Info(const std::string& id) {
  auto it = texturas_.find(id);
  if (it == texturas_.end()) {
    return nullptr;
  }
  return it->second;
}

const Texturas::InfoTextura* Texturas::Info(const std::string& id) const {
  auto it = texturas_.find(id);
  if (it == texturas_.end()) {
    return nullptr;
  }
  return it->second;
}

void Texturas::CarregaTextura(const std::string& id) {
  auto* info = Info(id);
  if (info == nullptr) {
    VLOG(1) << "Textura criada:" << id;
    info = new InfoTextura;
    texturas_.insert(make_pair(id, info));
  } else {
    ++info->contador;
    VLOG(1) << "Textura '" << id << " incrementada para " << info->contador;
  }
}

void Texturas::DescarregaTextura(const std::string& id) {
  auto* info = Info(id);
  if (info == nullptr) {
    LOG(WARNING) << "Textura nao existente: " << id;
  } else {
    if (--info->contador == 0) {
      VLOG(1) << "Textura liberada: " << id;
      delete info;
      texturas_.erase(id);
    } else {
      VLOG(1) << "Textura '" << id << " decrementada para " << info->contador;
    }
  }
}

}  // namespace ent
