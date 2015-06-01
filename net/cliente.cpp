#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>
#include <string>

#include "ent/constantes.h"
// para depurar android e ios.
#define VLOG_NIVEL 1
#include "log/log.h"
#include "net/cliente.h"
#include "net/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {

Cliente::Cliente(Sincronizador* sincronizador, ntf::CentralNotificacoes* central) {
  sincronizador_ = sincronizador;
  central_ = central;
  central_->RegistraReceptor(this);
  buffer_tamanho_.resize(4);
  // TODO fazer alguma verificacao disso.
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
}

bool Cliente::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    int n = 0;
    if (socket_descobrimento_.get() != nullptr) {
      if (++timer_descobrimento_ * INTERVALO_NOTIFICACAO_MS > 3000) {
        //LOG(INFO) << "Timer: " << timer_descobrimento_;
        socket_descobrimento_->Fecha();
      }
      n = sincronizador_->Roda();
    } else if (Ligado()) {
      n = sincronizador_->Roda();
    }
    if (n > 0) {
      VLOG(2) << "polled: " << n;
    } else {
      VLOG(3) << "polled: 0";
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_CONECTAR) {
    if (notificacao.endereco().empty()) {
      AutoConecta(notificacao.id_rede());
    } else {
      Conecta(notificacao.id_rede(), notificacao.endereco());
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_DESCONECTAR) {
    Desconecta("");
    return true;
  }
  return false;
}

bool Cliente::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  EnviaDados(notificacao.SerializeAsString());
  return true;
}

void Cliente::EnviaDados(const std::string& dados, bool sem_dados) {
  if (!sem_dados) {
    fifo_envio_.push(CodificaDados(dados));
    if (fifo_envio_.size() > 1) {
      return;
    }
  }
  socket_->Envia(fifo_envio_.front(), [this] (const Erro& erro, std::size_t bytes_enviados) {
    // Importante nao usar o socket aqui em caso de erro, pode estar dangling.
    if (erro) {
      LOG(ERROR) << "Erro enviando: " << erro.mensagem() << ", enviado: " << bytes_enviados;
      return;
    }
    fifo_envio_.pop();
    VLOG(1) << "Enviei " << bytes_enviados << " bytes pro servidor.";
    if (!fifo_envio_.empty()) {
      EnviaDados("", true);
    }
  });
}

void Cliente::AutoConecta(const std::string& id) {
  VLOG(1) << "Tentando auto conectar como " << id;
  if (socket_descobrimento_.get() != nullptr) {
    auto* n = ntf::NovaNotificacao(ntf::TN_RESPOSTA_CONEXAO);
    n->set_erro("Já há um descobrimento em curso.");
    central_->AdicionaNotificacao(n);
    return;
  }
  //LOG(INFO) << "recriando socket";
  socket_descobrimento_.reset(new SocketUdp(sincronizador_, PortaAnuncio()));
  if (!socket_descobrimento_->Aberto()) {
    socket_descobrimento_.reset();
    auto* n = ntf::NovaNotificacao(ntf::TN_RESPOSTA_CONEXAO);
    n->set_erro("Nao consegui abrir socket de auto conexao.");
    central_->AdicionaNotificacao(n);
    return;
  }
  buffer_descobrimento_.resize(10);
  socket_descobrimento_->Recebe(
      &buffer_descobrimento_,
      &endereco_descoberto_,
      [this, id] (const Erro& erro, std::size_t num_bytes) {
        socket_descobrimento_.reset();
        if (erro) {
          std::string erro_str("Tempo de espera expirado para autoconexão");
          auto* n = ntf::NovaNotificacao(ntf::TN_RESPOSTA_CONEXAO);
          n->set_erro(erro_str);
          central_->AdicionaNotificacao(n);
          LOG(ERROR) << erro_str;
          return;
        }
        LOG(INFO) << "RECEBI de: " << endereco_descoberto_
                  << ", anuncio: " << std::string(buffer_descobrimento_.begin(), buffer_descobrimento_.end());
        std::string endereco_str(endereco_descoberto_);
        if (num_bytes > 0) {
          endereco_str.append(":");
          endereco_str.append(buffer_descobrimento_.begin(), buffer_descobrimento_.begin() + num_bytes);
        }
        Conecta(id, endereco_str);
      }
  );
  //LOG(INFO) << "zerando timer";
  timer_descobrimento_ = 0;
}

void Cliente::Conecta(const std::string& id, const std::string& endereco_str) {
  VLOG(1) << "Tentando conectar como " << id << " em " << endereco_str;
  if (socket_descobrimento_.get() != nullptr) {
    auto* n = ntf::NovaNotificacao(ntf::TN_RESPOSTA_CONEXAO);
    n->set_erro("Já há um descobrimento em curso.");
    central_->AdicionaNotificacao(n);
    return;
  }
  std::vector<std::string> endereco_porta;
  boost::split(endereco_porta, endereco_str, boost::algorithm::is_any_of(":"));
  if (endereco_porta.size() == 0) {
    // Endereco padrao.
    LOG(ERROR) << "Nunca deveria chegar aqui: conexao sem endereco nem portal";
    endereco_porta.push_back("localhost");
  } else if (endereco_porta[0].empty()) {
    endereco_porta[0] = "localhost";
  }
  if (endereco_porta.size() == 1) {
    // Porta padrao.
    endereco_porta.push_back(to_string(PortaPadrao()));
  }
  try {
    socket_.reset(new Socket(sincronizador_));
    socket_->Conecta(endereco_porta[0], endereco_porta[1]);
#if 0
    //boost::asio::socket_base::receive_buffer_size option(50000);
    boost::asio::socket_base::receive_buffer_size option;
    //socket_->set_option(option);
    socket_->get_option(option);
    LOG(INFO) << "Valor buffer recepcao: " << option.value();
    boost::asio::socket_base::receive_low_watermark option2;
    //socket_->set_option(option);
    socket_->get_option(option2);
    LOG(INFO) << "Valor buffer recepcao watermark: " << option2.value();
    boost::asio::socket_base::send_low_watermark option3(1);
    //socket_->set_option(option3);
    socket_->get_option(option3);
    LOG(INFO) << "Valor buffer envio watermark: " << option3.value();
#endif

    // Handler de leitura.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_id_rede(id);
    central_->AdicionaNotificacao(notificacao);
    central_->RegistraReceptorRemoto(this);
    auto* copia = new ntf::Notificacao(*notificacao);
    central_->AdicionaNotificacaoRemota(copia);
    RecebeDados();
    VLOG(1) << "Conexão bem sucedida";
  } catch (std::exception& e) {
    socket_.reset();
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
    VLOG(1) << "Falha de conexão com " << endereco_porta[0] << ":" << endereco_porta[1];
    return;
  }
}

void Cliente::Desconecta(const std::string& erro) {
  if (!Ligado()) {
    return;
  }
  socket_->Fecha();
  socket_.reset();
  auto* notificacao = ntf::NovaNotificacao(ntf::TN_DESCONECTADO);
  central_->AdicionaNotificacao(notificacao);
  central_->DesregistraReceptorRemoto(this);
  if (!erro.empty()) {
    notificacao->set_erro(erro);
  }
  LOG(INFO) << "Desconectando: " << erro;
}

bool Cliente::Ligado() const {
  return socket_ != nullptr;
}

void Cliente::RecebeDados() {
  VLOG(1) << "Cliente::RecebeDados";
  // Funcao para recebimento de dados.
  std::function<void(const Erro&, std::size_t)> funcao_recebe_dados =
      [this](const Erro& erro, std::size_t bytes_recebidos) {
    VLOG(1) << "lambda funcao_recebe_dados";
    if (erro) {
      std::string erro_str;
      if (erro.ConexaoFechada()) {
        erro_str = "Erro recebendo dados: conexao fechada pela outra ponta";
      } else {
        erro_str = "Erro recebendo dados: " + erro.mensagem();
        LOG(ERROR) << erro_str;
      }
      Desconecta(erro_str);
      return;
    }
    // Quantidade de dados recebida eh maior ou igual ao esperado (por exemplo, ao receber duas mensagens juntas).
    // Decodifica mensagem e poe na central.
    auto* notificacao = new ntf::Notificacao;
    if (!notificacao->ParseFromString(buffer_)) {
      std::string erro_str;
      erro_str = "Erro ParseFromString recebendo dados do servidor. Tamanho buffer_notificacao: " +
                 to_string(buffer_.size());
      delete notificacao;
      Desconecta(erro_str);
      return;
    }
    notificacao->set_local(false);
    central_->AdicionaNotificacao(notificacao);
    VLOG(1) << "Tudo recebido";
    RecebeDados();
  };

  // Recebe o tamanho e chama recebe dados.
  buffer_tamanho_.resize(4);
  socket_->Recebe(
      &buffer_tamanho_,
      [this, funcao_recebe_dados] (const Erro& erro, std::size_t bytes_recebidos) {
    VLOG(1) << "lambda funcao_recebe_tamanho";
    if (erro || (bytes_recebidos < 4)) {
      std::string erro_str;
      if (erro.ConexaoFechada()) {
        erro_str = "Erro recebendo mensagem do servidor: conexao fechada pela outra ponta.";
      } else {
        erro_str = "Erro recebendo tamanho de dados do servidor msg menor que 4.";
        LOG(ERROR) << erro_str;
      }
      LOG(ERROR) << erro_str << ", bytes_recebidos: " << bytes_recebidos;
      Desconecta(erro_str);
      return;
    }
    unsigned int tamanho = DecodificaTamanho(buffer_tamanho_.begin());
    // TODO verificar tamanho.
    if (tamanho > 50 * 1024 * 1024) {
      LOG(WARNING) << "TAMANHO GIGANTE!! " << tamanho;
    }
    VLOG(1) << "Vou receber: " << tamanho << " bytes";
    buffer_.resize(tamanho);
    socket_->Recebe(
        &buffer_,
        funcao_recebe_dados);
  });
}

}  // namespace net
