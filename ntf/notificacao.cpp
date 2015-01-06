/** @file not/Notificacao.cpp implementacao das funcoes basicas de notificacao. */

#include "log/log.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace ntf {

Notificacao* NovaNotificacao(ntf::Tipo tipo) {
  auto* n = new ntf::Notificacao;
  n->set_tipo(tipo);
  return n;
}

CentralNotificacoes::CentralNotificacoes() {}
CentralNotificacoes::~CentralNotificacoes() {
}

void CentralNotificacoes::RegistraReceptor(Receptor* receptor) {
  receptores_.push_back(receptor);
}

void CentralNotificacoes::RegistraReceptorRemoto(ReceptorRemoto* receptor) {
  receptores_remotos_.push_back(receptor);
}

void CentralNotificacoes::DesregistraReceptor(const Receptor* receptor) {
  for (auto it = receptores_.begin(); it != receptores_.end(); ++it) {
    if (*it == receptor) {
      receptores_.erase(it);
      break;
    }
  }
}

void CentralNotificacoes::DesregistraReceptorRemoto(const ReceptorRemoto* receptor) {
  for (auto it = receptores_remotos_.begin(); it != receptores_remotos_.end(); ++it) {
    if (*it == receptor) {
      receptores_remotos_.erase(it);
      break;
    }
  }
}

void CentralNotificacoes::AdicionaNotificacao(Notificacao* notificacao) {
  VLOG(3) << "Adicionando: " << notificacao->ShortDebugString();
  if (notificacao->tipo() == TN_GRUPO_NOTIFICACOES) {
    LOG(ERROR) << "Nao se deve adicionar GRUPO a notificacoes da central.";
  }
  notificacoes_.push_back(std::unique_ptr<ntf::Notificacao>(notificacao));
}

void CentralNotificacoes::AdicionaNotificacaoRemota(Notificacao* notificacao) {
  if (receptores_remotos_.empty()) {
    delete notificacao;
    return;
  }
  VLOG(3) << "Adicionando notificacao remota: " << notificacao->ShortDebugString();
  if (notificacao->tipo() == TN_GRUPO_NOTIFICACOES) {
    LOG(ERROR) << "Nao se deve adicionar GRUPO a notificacoes da central.";
  }
  notificacoes_remotas_.push_back(std::unique_ptr<ntf::Notificacao>(notificacao));
}

void CentralNotificacoes::Notifica() {
  // Realiza a copia pq pode haver novas notificacoes durante o loop.
  std::vector<std::unique_ptr<Notificacao>> copia_notificacoes;
  copia_notificacoes.swap(notificacoes_);
  // Copia receptores caso algum queira se desregistrar durante o loop.
  std::vector<Receptor*> copia_receptores(receptores_);
  for (const auto& n : copia_notificacoes) {
    if (n->tipo() != ntf::TN_TEMPORIZADOR) {
      VLOG(1) << "Despachando local: " << n->ShortDebugString();
    } else {
      VLOG(2) << "Despachando local: " << n->ShortDebugString();
    }
    for (auto* r : copia_receptores) {
      r->TrataNotificacao(*n);
    }
  }
  copia_notificacoes.clear();
  copia_notificacoes.swap(notificacoes_remotas_);
  // Copia receptores caso algum queira se deregistrar durante o loop. Observe que a copia deve ser feita
  // aqui, caso contrario se alguem se registrar no loop acima, nao sera visto aqui e a mensagem se perdera.
  std::vector<ReceptorRemoto*> copia_receptores_remotos(receptores_remotos_);
  for (const auto& n : copia_notificacoes) {
    VLOG(1) << "Despachando remota: " << n->ShortDebugString();
    for (auto* r : copia_receptores_remotos) {
      r->TrataNotificacaoRemota(*n);
    }
  }
}

}  // namespace ntf
