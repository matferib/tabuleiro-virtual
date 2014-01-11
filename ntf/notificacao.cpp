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
  for (auto* n : notificacoes_) {
    delete n;
  }
  for (auto* n : notificacoes_remotas_) {
    delete n;
  }
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
  VLOG(1) << "Adicionando: " << notificacao->ShortDebugString();
  notificacoes_.push_back(notificacao);
}

void CentralNotificacoes::AdicionaNotificacaoRemota(Notificacao* notificacao) {
  VLOG(1) << "Adicionando notificacao remota: " << notificacao->ShortDebugString();
  notificacoes_remotas_.push_back(notificacao);
}

void CentralNotificacoes::Notifica() {
  // Realiza a copia pq pode haver novas notificacoes durante o loop.
  std::vector<Notificacao*> copia_notificacoes;
  copia_notificacoes.swap(notificacoes_);
  for (auto* n : copia_notificacoes) {
    if (n->tipo() != ntf::TN_TEMPORIZADOR) {
      LOG(INFO) << "Despachando local: " << n->ShortDebugString();
    }
    for (auto* r : receptores_) {
      r->TrataNotificacao(*n);
    }
    delete n;
  }
  copia_notificacoes.clear();
  copia_notificacoes.swap(notificacoes_remotas_);
  for (auto* n : copia_notificacoes) {
    LOG(INFO) << "Despachando remota: " << n->ShortDebugString();
    for (auto* r : receptores_remotos_) {
      r->TrataNotificacaoRemota(*n);
    }
    delete n;
  }
}

}  // namespace ntf
