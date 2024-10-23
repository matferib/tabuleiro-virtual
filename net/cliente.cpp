#include <absl/strings/str_format.h>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <vector>
#include <string>

//#include "ent/constantes.h"
#include "goog/stringprintf.h"
// para depurar android e ios.
//#define VLOG_NIVEL 1
#include "log/log.h"
#include "net/cliente.h"
#include "net/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {
namespace {
}  // namespace

Cliente::Cliente(Sincronizador* sincronizador, ntf::CentralNotificacoes* central) {
  sincronizador_ = sincronizador;
  central_ = central;
  central_->RegistraReceptor(this);
  buffer_tamanho_.resize(4);
  // TODO fazer alguma verificacao disso.
  // tamanho maximo da mensagem: 1MB.
  buffer_.resize(1 * 1024 * 1024);
  timer_descobrimento_.stop();
}

bool Cliente::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    int n = 0;
    if (socket_descobrimento_.get() != nullptr) {
      auto passou_ms = timer_descobrimento_.elapsed().wall / 1000000ULL;
      if (passou_ms > 9000) {
        socket_descobrimento_->Fecha();
        timer_descobrimento_.stop();
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
      Conecta(notificacao.id_rede(), notificacao.endereco(), notificacao.has_porta_local() ? notificacao.porta_local() : 0);
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_DESCONECTAR) {
    Desconecta("");
    return true;
  } else if (notificacao.tipo() == ntf::TN_HACK_ANDROID) {
#if 0 && ANDROID
    sincronizador_->AlternaHackAndroid();
#endif
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
    central_->AdicionaNotificacao(
        ntf::NovaNotificacaoErroTipada(ntf::TN_RESPOSTA_CONEXAO, "Já há um descobrimento em curso."));
    return;
  }
  //LOG(INFO) << "recriando socket";
  socket_descobrimento_.reset(new SocketUdp(sincronizador_, PortaAnuncio()));
  if (!socket_descobrimento_->Aberto()) {
    socket_descobrimento_.reset();
    central_->AdicionaNotificacao(
        ntf::NovaNotificacaoErroTipada(ntf::TN_RESPOSTA_CONEXAO, "Nao consegui abrir socket de auto conexao."));
    return;
  }
  buffer_descobrimento_.resize(10);
  socket_descobrimento_->Recebe(
      &buffer_descobrimento_,
      &endereco_descoberto_,
      [this, id] (const Erro& erro, std::size_t num_bytes) {
        socket_descobrimento_.reset();
        if (erro) {
          std::string erro_str = "Tempo de espera expirado para autoconexão";
          central_->AdicionaNotificacao(
            ntf::NovaNotificacaoErroTipada(ntf::TN_RESPOSTA_CONEXAO, erro_str));
          LOG(ERROR) << erro_str;
          return;
        }
        LOG(INFO) << "RECEBI de: " << endereco_descoberto_
                  << ", anuncio: " << std::string(buffer_descobrimento_.begin(), buffer_descobrimento_.end());
        std::string endereco_str(absl::StrFormat("[%s]", endereco_descoberto_.c_str()));
        if (num_bytes > 0 && num_bytes < 10) {
          endereco_str = absl::StrFormat("%s:%s",
              endereco_str.c_str(),
              std::string(buffer_descobrimento_.begin(), buffer_descobrimento_.begin() + num_bytes).c_str());
        }
        LOG(INFO) << "CONECTANDO EM: " << endereco_str;
        Conecta(id, endereco_str, /*porta_local=*/0);
      }
  );
  //LOG(INFO) << "zerando timer";
  timer_descobrimento_.start();
}

void Cliente::Conecta(const std::string& id, const std::string& endereco_str, int porta_local) {
  VLOG(1) << "Tentando conectar como " << id << " em " << endereco_str;
  if (socket_descobrimento_.get() != nullptr) {
    central_->AdicionaNotificacao(
        ntf::NovaNotificacaoErroTipada(ntf::TN_RESPOSTA_CONEXAO, "Já há um descobrimento em curso."));
    return;
  }
  std::string endereco_parseado;
  std::string porta_parseada;
  if (endereco_str.empty()) {
    endereco_parseado = "localhost";
  } else if (endereco_str[0] == '[') {
    // IPV6 com []
    // Mesmo que use token compress, o primeiro [ gerara um token vazio.
    std::vector<std::string> endereco_tokenizado;
    std::string endereco_sem_primeiro = endereco_str.substr(1);
    boost::split(endereco_tokenizado, endereco_sem_primeiro, boost::algorithm::is_any_of("]"));
    if (endereco_tokenizado.empty()) {
      endereco_parseado = "localhost";
      porta_parseada = to_string(PortaPadrao());
    } else {
      endereco_parseado = endereco_tokenizado[0];
      if (endereco_tokenizado.size() < 2 || endereco_tokenizado[1].size() < 2) {
        porta_parseada = to_string(PortaPadrao());
      } else {
        porta_parseada = endereco_tokenizado[1].substr(1);
      }
    }
  } else if (std::count(endereco_str.begin(), endereco_str.end(), ':') > 1) {
    // IPV6 sem [] e sem porta.
    endereco_parseado = endereco_str;
    porta_parseada = to_string(PortaPadrao());
  } else {
    // host[:porta].
    std::vector<std::string> endereco_tokenizado;
    boost::split(endereco_tokenizado, endereco_str, boost::algorithm::is_any_of(":"));
    if (endereco_tokenizado.empty()) {
      endereco_parseado = "localhost";
      porta_parseada = to_string(PortaPadrao());
    } else if (endereco_tokenizado.size() == 1) {
      endereco_parseado = endereco_tokenizado[0];
      porta_parseada = to_string(PortaPadrao());
    } else {
      endereco_parseado = endereco_tokenizado[0];
      porta_parseada = endereco_tokenizado[1];
    }
  }
  try {
    socket_.reset(new Socket(sincronizador_));
    if (porta_local > 1024) {
      socket_->PortaLocal(porta_local);
    }
    LOG(INFO) << "Pronto para conectar, servidor: " << endereco_parseado << ", porta: " << porta_parseada;
    socket_->Conecta(endereco_parseado, porta_parseada);
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

    // Handler de leitura. Envia a notificacao localmente.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_RESPOSTA_CONEXAO);
    notificacao->set_id_rede(id);
    central_->AdicionaNotificacao(notificacao);
    central_->RegistraEmissorRemoto(this);
    // Com o emissor remoto registrado, envia a notificacao remota.
    auto* copia = new ntf::Notificacao(*notificacao);
    central_->AdicionaNotificacaoRemota(copia);
    RecebeDados();
    VLOG(1) << "Conexão bem sucedida";
  } catch (std::exception& e) {
    socket_.reset();
    central_->AdicionaNotificacao(ntf::NovaNotificacaoErroTipada(ntf::TN_RESPOSTA_CONEXAO, e.what()));
    VLOG(1) << "Falha de conexão com " << endereco_parseado << ":" << porta_parseada;
    return;
  }
}

void Cliente::Desconecta(const std::string& erro) {
  if (!Ligado()) {
    return;
  }
  socket_->Fecha();
  socket_.reset();
  auto notificacao = ntf::NovaNotificacao(ntf::TN_DESCONECTADO);
  if (!erro.empty()) {
    notificacao->set_erro(erro);
  }
  central_->AdicionaNotificacao(notificacao.release());
  central_->DesregistraEmissorRemoto(this);
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
        erro_str = "Erro recebendo dados: conexao fechada pelo servidor";
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
                 to_string((int)buffer_.size());
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
        erro_str = "Erro recebendo mensagem do servidor: conexao fechada.";
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
