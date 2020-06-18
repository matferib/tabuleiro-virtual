#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//#include "ent/constantes.h"
//#define VLOG_NIVEL 1
#include "goog/stringprintf.h"
#include "log/log.h"
#include "net/servidor.h"
#include "net/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {
namespace {
using ::google::protobuf::StringPrintf;
}  // namespace

Servidor::Servidor(Sincronizador* sincronizador, ntf::CentralNotificacoes* central) {
  sincronizador_ = sincronizador;
  aceitador_.reset(new Aceitador(sincronizador));
  central_ = central;
  central_->RegistraReceptor(this);
}

Servidor::~Servidor() {
}

bool Servidor::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    ++timer_anuncio_;
    if (timer_anuncio_ * INTERVALO_ANUNCIO_MS >= 1000) {
      timer_anuncio_ = 0;
      if (anunciante_.get() != nullptr) {
        VLOG(3) << "ANUNCIO ENVIADO";
        anunciante_->Envia(PortaAnuncio(), buffer_porta_,
                           [] (const Erro& erro, std::size_t bytes_transferred) {});
      }
    }
    if (Ligado()) {
      auto n = sincronizador_->Roda();
      if (n > 0) {
        VLOG(2) << "Rodei " << n << " eventos";
      } else {
        VLOG(3) << "Nada rodado";
      }
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_INICIAR) {
    Liga();
    return true;
  } else if (notificacao.tipo() == ntf::TN_CONECTAR_PROXY) {
    ConectaProxy(notificacao.endereco());
    return true;
  } else if (notificacao.tipo() == ntf::TN_SAIR) {
    Desliga();
    return true;
  }
  return false;
}

bool Servidor::TrataNotificacaoRemota(const ntf::Notificacao& notificacao) {
  const std::string ns = notificacao.SerializeAsString();
  if (notificacao.clientes_pendentes()) {
    // Envia o tabuleiro para o cliente correto.
    Cliente* cliente_pendente = nullptr;
    for (auto* c : clientes_pendentes_) {
      if (c->id == notificacao.id_rede()) {
        cliente_pendente = c;
      }
    }
    if (cliente_pendente == nullptr) {
      LOG(ERROR) << "Nao encontrei cliente pendente: '" << notificacao.id_rede() << "'";
      return true;
    }
    if (notificacao.tipo() == ntf::TN_ERRO) {
      LOG(ERROR) << "Conexao com cliente rejeitada, provavelmente servidor nao conseguiu gerar id.";
      cliente_pendente->socket->Fecha();
      return true;
    }
    VLOG(1) << "Enviando primeira notificacao para cliente pendente";
    EnviaDadosCliente(cliente_pendente, ns);
    clientes_.insert(cliente_pendente);
  } else {
    for (auto* c : clientes_) {
      if (notificacao.has_id_rede() && c->id != notificacao.id_rede()) {
        LOG(INFO) << "Dropando notificacao por causa id cliente diferente. Destino: " << notificacao.id_rede() << " x cliente: " << c->id;
        continue;
      }
      VLOG(1) << "Enviando notificacao para cliente, tam: " << ns.size();
      EnviaDadosCliente(c, ns);
    }
  }

  return true;
}

bool Servidor::Ligado() const {
  return aceitador_->Ligado();
}

void Servidor::ConectaProxy(const std::string& endereco_str) {
  std::vector<std::string> endereco_porta;
  boost::split(endereco_porta, endereco_str, boost::algorithm::is_any_of(":"));
  if (endereco_porta.size() == 0) {
    // Endereco padrao.
    LOG(ERROR) << "Nunca deveria chegar aqui: conexao sem endereco nem porta";
    endereco_porta.push_back("localhost");
  } else if (endereco_porta[0].empty()) {
    endereco_porta[0] = "localhost";
  }
  if (endereco_porta.size() == 1) {
    // Porta padrao.
    endereco_porta.push_back(to_string(11225));
  }
  try {
    socket_proxy_.reset(new Socket(sincronizador_));
    socket_proxy_->Conecta(endereco_porta[0], endereco_porta[1]);
    endereco_proxy_ = endereco_porta[0];
    porta_proxy_ = endereco_porta[1];
    // Agora aguarda clientes no proxy.
    buffer_proxy_.resize(4);
    AguardaClientesProxy();
    VLOG(1) << "Conexão no proxy bem sucedida";
  } catch (std::exception& e) {
    socket_proxy_.reset();
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ERRO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
    VLOG(1) << "Falha de conexão com proxy " << endereco_porta[0] << ":" << endereco_porta[1];
    return;
  }
}

void Servidor::AguardaClientesProxy() {
  VLOG(1) << "Servidor::AguardaClientesProxy";
  // Recebe o tamanho e chama recebe dados.
  socket_proxy_->Recebe(
      &buffer_proxy_,
      [this] (const Erro& erro, std::size_t bytes_recebidos) {
    VLOG(1) << "lambda funcao_recebe_tamanho";
    if (erro || (bytes_recebidos < 4)) {
      std::string erro_str;
      if (erro.ConexaoFechada()) {
        erro_str = "Erro recebendo mensagem do proxy: conexao fechada.";
      } else {
        erro_str = "Erro recebendo tamanho de dados do proxy: msg menor que 4.";
      }
      LOG(ERROR) << erro_str << ", bytes_recebidos: " << bytes_recebidos;
      socket_proxy_->Fecha();
      return;
    }
    VLOG(1) << "Recebi do proxy: " << buffer_proxy_;
    // Nova conexao, conecta no proxy.
    std::unique_ptr<Socket> socket_mestre_cliente(new Socket(sincronizador_));
    try {
      socket_mestre_cliente->Conecta(endereco_proxy_, to_string(atoi(porta_proxy_.c_str()) + 1));
    } catch (const std::exception& e) {
      socket_mestre_cliente.reset();
      auto* notificacao = new ntf::Notificacao;
      notificacao->set_tipo(ntf::TN_ERRO);
      notificacao->set_erro(e.what());
      central_->AdicionaNotificacao(notificacao);
      VLOG(1) << "Falha de conexão com proxy para cliente " << endereco_proxy_ << ":" << porta_proxy_;
      return;
    }
    VLOG(1) << "Criando novo cliente de proxy";
    Cliente* cliente_pendente = new Cliente(socket_mestre_cliente.release());
    clientes_pendentes_.insert(cliente_pendente);
    RecebeDadosCliente(cliente_pendente);
    AguardaClientesProxy();
  });
}

void Servidor::Liga() {
  VLOG(1) << "Ligando servidor.";
  try {
    proximo_cliente_.reset(new Cliente(new Socket(sincronizador_)));
    central_->RegistraEmissorRemoto(this);
    aceitador_->Liga(PortaPadrao(), proximo_cliente_->socket.get(),
                     [this](const Erro& erro) -> Socket* {
      if (erro) {
        LOG(ERROR) << "Recebendo erro..." << erro.mensagem();
        return nullptr;
      }
      // Cria cliente pendente.
      VLOG(1) << "Recebendo cliente...";
      Cliente* cliente_pendente = proximo_cliente_.release();
      clientes_pendentes_.insert(cliente_pendente);
#if 0
      boost::asio::socket_base::receive_buffer_size option;
      cliente_pendente->socket->get_option(option);
      LOG(INFO) << "Buffer recepcao: " << option.value();
      boost::asio::socket_base::receive_low_watermark option2;
      cliente_pendente->socket->get_option(option2);
      LOG(INFO) << "Buffer recepcao watermark: " << option2.value();
      boost::asio::socket_base::send_low_watermark option3(1);
      cliente_pendente->socket->set_option(option3);
      cliente_pendente->socket->get_option(option3);
      LOG(INFO) << "Buffer envio watermark: " << option3.value();
#endif
      // Proximo cliente.
      proximo_cliente_.reset(new Cliente(new Socket(sincronizador_)));
      RecebeDadosCliente(cliente_pendente);
      return proximo_cliente_->socket.get();
    });
    VLOG(1) << "Servidor ligado.";
  } catch (const std::exception& e) {
    LOG(ERROR) << "Erro ligando servidor: " << e.what();
    // TODO fazer o tipo de erro e tratar notificacao em algum lugar.
    auto* notificacao = new ntf::Notificacao;
    notificacao->set_tipo(ntf::TN_ERRO);
    notificacao->set_erro(e.what());
    central_->AdicionaNotificacao(notificacao);
    return;
  }

  // Aqui eh so pro anunciante do jogo.
  buffer_porta_ = to_string(PortaPadrao());
  anunciante_.reset(new SocketUdp(sincronizador_));
  try {
    anunciante_->Abre();
  } catch (const std::exception& e) {
    anunciante_.reset();
    LOG(ERROR) << "Falha abrindo socket de broadcast: " << e.what();
  }
}

void Servidor::Desliga() {
  VLOG(1) << "Desligando servidor.";
  if (!Ligado()) {
    LOG(ERROR) << "Servidor ja está desligado.";
    return;
  }
  central_->DesregistraEmissorRemoto(this);
  aceitador_->Desliga();
  anunciante_.reset();
  for (auto* c : clientes_) {
    delete c;
  }
  for (auto* c : clientes_pendentes_) {
    delete c;
  }
  VLOG(1) << "Servidor desligado.";
}

void Servidor::EnviaDadosCliente(Cliente* cliente, const std::string& dados, bool sem_dados) {
  if (!sem_dados) {
    cliente->fifo_envio.push(CodificaDados(dados));
    if (cliente->fifo_envio.size() > 1) {
      VLOG(1) << "Enfileirando dados, fila nao vazia";
      return;
    }
  }
  try {
    cliente->socket->Envia(
        cliente->fifo_envio.front(),
        [this, cliente] (const Erro& erro, std::size_t bytes_enviados) {
      if (erro) {
        // Importante nao usar cliente aqui, pois o ponteiro pode estar dangling.
        LOG(ERROR) << "Erro enviando dados, mensagem: " << erro.mensagem();
        return;
      }
      cliente->fifo_envio.pop();
      VLOG(1) << "Enviei " << bytes_enviados << " bytes pro cliente.";
      if (!cliente->fifo_envio.empty()) {
        EnviaDadosCliente(cliente, "", true  /*sem_dados*/);
      }
    });
 } catch (const std::exception& e) {
   // Faz nada aqui que provavalmente o receive vai receber o erro.
   LOG(ERROR) << "Erro enviando dados: " << e.what();
 }
}

// deve ser usada apenas na funcao de RecebeDadosCliente.
void Servidor::DesconectaCliente(Cliente* cliente) {
  // Notifica interessados na desconexao.
  auto n(ntf::NovaNotificacao(ntf::TN_DESCONECTADO));
  n->set_id_rede(cliente->id);
  central_->AdicionaNotificacao(n.release());

  // Apaga o cliente.
  clientes_.erase(cliente);
  clientes_pendentes_.erase(cliente);
  delete cliente;
}

void Servidor::RecebeDadosCliente(Cliente* cliente) {
  // Funcao que recebe o dado de tamanho certinho.
  std::function<void(const Erro& erro, std::size_t bytes_recebidos)> funcao_recebe_dados =
      [this, cliente] (const Erro& erro, std::size_t bytes_recebidos) {
    if (erro || (bytes_recebidos < cliente->buffer_recepcao.size())) {
      // remove o cliente.
      auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
      std::string erro_str;
      if (erro.ConexaoFechada()) {
        erro_str = StringPrintf("Conexão fechada por cliente '%s'.", cliente->id.c_str());
        LOG(INFO) << erro_str << ", msg: " << erro.mensagem().c_str();
      } else {
        erro_str = StringPrintf("Erro recebendo mensagem de cliente: '%s'. Erro: %s. Esperava: %d, recebi: %d",
            cliente->id.c_str(), erro.mensagem().c_str(), (int)cliente->buffer_recepcao.size(), (int)bytes_recebidos);
        LOG(INFO) << erro_str;
      }
      n->set_erro(erro_str);
      central_->AdicionaNotificacao(n.release());
      DesconectaCliente(cliente);
      return;
    }
    VLOG(1) << "Recebi " << bytes_recebidos << " bytes do cliente " << cliente->id;

    // Decodifica mensagem e poe na central.
    std::unique_ptr<ntf::Notificacao> notificacao(new ntf::Notificacao);
    if (!notificacao->ParseFromString(cliente->buffer_recepcao)) {
      std::string erro_str(std::string("Erro ParseFromString recebendo dados do cliente '") + cliente->id + "'");
      LOG(ERROR) << erro_str << ", bytes_recebidos: " << bytes_recebidos;
      auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
      n->set_erro(erro_str);
      central_->AdicionaNotificacao(n.release());
      DesconectaCliente(cliente);
      return;
    }
    // Notificacao de identificacao eh tratada neste nivel tambem. Aqui eh o unico local onde se tem o objeto do cliente
    // e a notificacao.
    if (notificacao->tipo() == ntf::TN_RESPOSTA_CONEXAO) {
      bool ja_existe = false;
      for (const auto& c : clientes_pendentes_) {
        if (c->id == notificacao->id_rede()) {
          ja_existe = true;
        }
      }
      if (ja_existe) {
        DesconectaCliente(cliente);
        auto erro = ntf::NovaNotificacao(ntf::TN_ERRO);
        erro->set_erro(std::string("Id de cliente repetido: '") + notificacao->id_rede() + "'");
        central_->AdicionaNotificacao(erro.release());
        return;
      }
      LOG(INFO) << "Recebi TN_RESPOSTA_CONEXAO de cliente: " << notificacao->id_rede();
      cliente->id = notificacao->id_rede();
      auto resposta = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
      resposta->set_clientes_pendentes(true);
      resposta->set_id_rede(cliente->id);
      central_->AdicionaNotificacao(resposta.release());
    }
    // Envia a notificacao para os outros clientes.
    if (!notificacao->servidor_apenas()) {
      for (auto* c : clientes_) {
        if (c == cliente) {
          // Nao envia para o cliente original.
          continue;
        }
        EnviaDadosCliente(c, cliente->buffer_recepcao);
      }
    }
    // Processa localmente.
    notificacao->set_local(false);
    central_->AdicionaNotificacao(notificacao.release());
    VLOG(1) << "Tudo recebido de cliente " << cliente->id;
    RecebeDadosCliente(cliente);
  };

  // Recebe o tamanho dos dados e chama recebe dados.
  std::function<void(const Erro& erro, std::size_t bytes_recebidos)> funcao_recebe_tamanho =
      [this, cliente, funcao_recebe_dados] (const Erro& erro, std::size_t bytes_recebidos) {
    if (erro || (bytes_recebidos < 4)) {
      std::string erro_str(std::string("Erro recebendo tamanho de dados do cliente '") + cliente->id + "': ");
      if (erro.ConexaoFechada()) {
        erro_str += std::string("Conexao fechada.");
      } else {
        erro_str += std::string("msg menor que 4. msg: ") + erro.mensagem();
      }
      LOG(ERROR) << erro_str << ", bytes_recebidos: " << bytes_recebidos;
      auto n = ntf::NovaNotificacao(ntf::TN_ERRO);
      n->set_erro(erro_str);
      central_->AdicionaNotificacao(n.release());
      DesconectaCliente(cliente);
      return;
    }
    unsigned int tamanho = DecodificaTamanho(cliente->buffer_tamanho.begin());
    // TODO verificar tamanho.
    if (tamanho > 50 * 1024 * 1024) {
      LOG(WARNING) << "TAMANHO GIGANTE!! " << tamanho;
    }
    VLOG(1) << "Vou Receber: " << tamanho << " bytes";
    cliente->buffer_recepcao.resize(tamanho);
    cliente->socket->Recebe(
        &cliente->buffer_recepcao,
        funcao_recebe_dados);
  };

  // Aqui comeca tudo. Chama o recebe tamanho.
  cliente->socket->Recebe(&cliente->buffer_tamanho, funcao_recebe_tamanho);
}

}  // namespace net
