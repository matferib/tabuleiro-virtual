#if ANDROID
#include <condition_variable>
#include <errno.h>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>
#include <sys/select.h>
// TODO remove
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include <sys/socket.h>
#include <sys/types.h>
#include "log/log.h"
#include "net/socket.h"
#include "net/util.h"

namespace net {

namespace {
struct ScopedPrint {
  explicit ScopedPrint(const std::string& p) {
    LOG(INFO) << "Entrando " << p;
    p_ = p;
  }
  ~ScopedPrint() {
    LOG(INFO) << "Saindo " << p_;
  }

  std::string p_;
};

struct DadosParaEnviarTcp {
  int desc;
  std::string dados;
  Socket::CallbackEnvio callback;
  Erro erro;
};

// O que deve ser recebido.
struct DadosParaReceberTcp {
  int desc;
  std::string* dados;
  Erro erro;
  Socket::CallbackRecepcao callback;
};

// UDP.
// Envio broadcast para uma porta.
struct DadosParaBroadcastUdp {
  int desc;
  int porta;
  std::string dados;
  SocketUdp::CallbackEnvio callback;
};

struct DadosParaReceberUdp {
  std::string* endereco;
  std::string* dados;
  int socket_cliente;
  Erro erro;
  SocketUdp::CallbackRecepcao callback;
};

struct DadosParaConexao {
  Socket* socket_servidor;
  Socket* socket_cliente;
  Aceitador::CallbackConexaoCliente callback;
};

}  // namespace

//-----
// Erro
//-----
struct Erro::Interno {
  boost::system::error_code ec;
};

const Erro ConverteErro(const boost::system::error_code& ec) {
  return Erro((void*)&ec);
}

Erro::Erro(void* dependente_plataforma) : Erro() {
  interno_->ec = *((boost::system::error_code*)dependente_plataforma);
  erro_ = *interno_->ec;
  msg_ = interno_->ec.message();
}
Erro::Erro(const std::string& msg) : Erro(true) {
  msg_ = msg;
}
Erro::Erro(bool erro) : interno_(new Interno), erro_(erro) {
}

bool Erro::ConexaoFechada() const {
  return interno_->ec.value() == boost::asio::error::eof;
}

//--------------
// Sincronizador
//--------------
struct Sincronizador::Interno {
 public:
  Interno(void* depende_plataforma) : servico_io((boost::asio::io_service*)depende_plataforma) {
    thread_ = std::move(std::thread(Sincronizador::Interno::Loop, this));
  }
  ~Interno() {
    terminar_ = true;
    thread_.join();
  }

  // Loop do sincronizador.
  static void Loop(Interno* thiz);
  static void LoopRecepcaoTcp(Interno* thiz, std::vector<int>* a_remover);
  static void LoopRecepcaoUdp(Interno* thiz, std::vector<int>* a_remover);
  static void LoopEnvioTcp(Interno* thiz);


  int Roda() {
    //ScopedPrint sp("Roda");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!recebidos_udp_.empty()) {
      auto* recebido = recebidos_udp_.front();
      LOG(INFO) << "Retornando erro udp? '" << recebido->erro.mensagem() << "', endereco: " << *recebido->endereco;
      recebido->callback(recebido->erro, recebido->erro ? 0 : recebido->dados->size());
      recebidos_udp_.pop();
      delete recebido;
    }
    if (!recebidos_tcp_.empty()) {
      auto* recebido = recebidos_tcp_.front();
      LOG(INFO) << "Retornando erro tcp? " << ((bool)recebido->erro)
                << ", mensagem: '" << recebido->erro.mensagem() << "'";
      recebido->callback(recebido->erro, recebido->erro ? 0 : recebido->dados->size());
      recebidos_tcp_.pop();
      delete recebido;
    }
    if (!enviados_tcp_.empty()) {
      auto* enviado = enviados_tcp_.front();
      LOG(INFO) << "Retornando erro envio tcp? " << ((bool)enviado->erro)
                << "', mensagem: " << enviado->erro.mensagem();
      enviado->callback(enviado->erro, enviado->erro ? 0 : enviado->dados.size());
      enviados_tcp_.pop();
      delete enviado;
    }
    return 1;
  }

  // TCP.
  void EnfileiraDadosEnvioTcp(const DadosParaEnviarTcp& d) {
    ScopedPrint sp("EnfileiraDadosEnvioTcp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      auto* cd = new DadosParaEnviarTcp;
      cd->desc = d.desc;
      cd->dados = d.dados;
      cd->callback = d.callback;
      a_enviar_tcp_.push(cd);
    }
    cond_.notify_one();
  }

  void EnfileiraDadosRecepcaoTcp(const DadosParaReceberTcp &dtcp) {
    ScopedPrint sp("EnfileiraDadosRecepcaoTcp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      DadosSocket ds;
      ds.dados_tcp.reset(new DadosParaReceberTcp);
      ds.dados_tcp->dados = dtcp.dados;
      ds.dados_tcp->desc = dtcp.desc;
      ds.dados_tcp->callback = dtcp.callback;
      sockets_.insert(std::make_pair(dtcp.desc, std::move(ds)));
    }
    cond_.notify_one();
  }

  // UDP.
  void EnfileiraDadosBroadcastUdp(const DadosParaBroadcastUdp& d) {
    d.callback(Erro("NAO IMPLEMENTADO"), 0);
  }

  void EnfileiraDadosRecepcaoUdp(const DadosParaReceberUdp& dudp) {
    ScopedPrint sp("EnfileiraDadosRecepcaoUdp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      DadosSocket ds;
      ds.dados_udp.reset(new DadosParaReceberUdp);
      ds.dados_udp->endereco = dudp.endereco;
      ds.dados_udp->dados = dudp.dados;
      ds.dados_udp->socket_cliente = dudp.socket_cliente;
      ds.dados_udp->callback = dudp.callback;
      sockets_.insert(std::make_pair(dudp.socket_cliente, std::move(ds)));
    }
    cond_.notify_one();
  }

  // Aceitador.

  boost::asio::io_service* servico_io = nullptr;
  std::thread thread_;
  bool terminar_ = false;

 private:
  std::queue<DadosParaEnviarTcp*> a_enviar_tcp_;
  std::queue<DadosParaEnviarTcp*> enviados_tcp_;

  std::queue<DadosParaReceberTcp*> recebidos_tcp_;
  std::queue<DadosParaReceberUdp*> recebidos_udp_;

  std::condition_variable_any cond_;
  std::recursive_mutex mutex_;

  // Para select.
  struct DadosSocket {
    std::unique_ptr<DadosParaReceberTcp> dados_tcp;
    std::unique_ptr<DadosParaReceberUdp> dados_udp;
  };
  std::map<int, DadosSocket> sockets_;  // para recepcao.
};


Sincronizador::Sincronizador(void* depende_plataforma) : interno_(new Interno(depende_plataforma)) {}
Sincronizador::~Sincronizador() {}

int Sincronizador::Roda() {
  return interno_->Roda();
}


//-------
// Socket
//-------
struct Socket::Interno {
  explicit Interno(void* nao_usado) {
    socket_ = socket(PF_INET, SOCK_STREAM, 0);
  }

  int socket_ = -1;
};

Socket::Socket(Sincronizador* sincronizador)
    : sincronizador_(sincronizador), interno_(new Interno(nullptr)) {
}

Socket::~Socket() {}

void Socket::Ouve(int porta) {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
  getaddrinfo(NULL, to_string(porta).c_str(), &hints, &res);
  if (bind(interno_->socket_, res->ai_addr, res->ai_addrlen) == -1) {
    throw std::logic_error("Erro no bind TCP");
  }
  freeaddrinfo(res);
  if (listen(interno_->socket_, 10  /*num conexoes pendentes*/) == -1) {
    throw std::logic_error("Erro no listen tcp");
  }
}

void Socket::Conecta(const std::string& endereco, const std::string& porta) {
  struct addrinfo hints, *res;
  int sockfd;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(endereco.c_str(), porta.c_str(), &hints, &res) != 0) {
    throw std::logic_error(std::string("Nao consegui converter endereco: ") + endereco + ":" + porta);
  }
  bool conectou = false;
  for (auto* rp = res; rp != nullptr; rp = rp->ai_next) {
    if (connect(interno_->socket_, rp->ai_addr, rp->ai_addrlen) == -1) {
      continue;
    }
    conectou = true;
  }
  freeaddrinfo(res);
  if (!conectou) {
    throw std::logic_error(std::string("Falha ao conectar ao servidor em: ") + endereco + ":" + porta);
  }
  LOG(INFO) << "CONECTADO AO SERVIDOR";
}

void Socket::Fecha() {
  close(interno_->socket_);
  interno_->socket_ = -1;
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
  DadosParaEnviarTcp d;
  d.desc = interno_->socket_;
  d.dados = dados;
  d.callback = callback_envio_cliente;
  sincronizador_->interno_->EnfileiraDadosEnvioTcp(d);
}

void Socket::Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente) {
  DadosParaReceberTcp d;
  d.desc = interno_->socket_;
  d.dados = dados;
  d.callback = callback_recepcao_cliente;
  sincronizador_->interno_->EnfileiraDadosRecepcaoTcp(d);
}


//----------
// SocketUdp
//----------
struct SocketUdp::Interno {
  Interno() {}
  Interno(int s) : socket_(s) {}
  ~Interno() { if (socket_ != -1) { close(socket_); socket_ = -1;} }

  int socket_ = -1;
};

SocketUdp::SocketUdp(Sincronizador* sincronizador)
    : sincronizador_(sincronizador), interno_(new Interno()) {
}

SocketUdp::SocketUdp(Sincronizador* sincronizador, int porta)
    : sincronizador_(sincronizador), interno_(new Interno()) {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;  //AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  if (getaddrinfo(NULL, to_string(porta).c_str(), &hints, &res) != 0) {
    LOG(WARNING) << "Nao consegui pegar endereco udp local porta " << porta;
    return;
  }
  int s = socket(PF_INET, SOCK_DGRAM, 0);
  if (s == -1) {
    freeaddrinfo(res);
    LOG(WARNING) << "Nao consegui abrir socket";
    return;
  }
  bool bind_funcionou = false;
  for (auto* rp = res; rp != nullptr; rp = rp->ai_next) {
    if (bind(s, rp->ai_addr, rp->ai_addrlen) != 0) {
      LOG(WARNING) << "Nao consegui fazer bind udp local porta " << porta;
      continue;
    }
    bind_funcionou = true;
    LOG(WARNING) << "Bind udp ok porta " << porta;
    break;
  }
  if (!bind_funcionou) {
    close(s);
    s = -1;
  }
  freeaddrinfo(res);
  interno_ .reset(new Interno(s));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  int broadcast = 1;
  if (setsockopt(interno_->socket_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
    interno_->socket_ = -1;
    throw std::logic_error("Erro abrindo socket broadcast UDP");
  }
}

bool SocketUdp::Aberto() const { return interno_->socket_ != -1; }

void SocketUdp::Fecha() {
  if (interno_->socket_ != -1) {
    close(interno_->socket_);
    interno_->socket_ = -1;
  }
}

void SocketUdp::Envia(int porta, const std::string& dados, CallbackEnvio callback_envio_cliente) {
  DadosParaBroadcastUdp d;
  d.desc = interno_->socket_;
  d.porta = porta;
  d.dados = dados;
  d.callback = callback_envio_cliente;
  sincronizador_->interno_->EnfileiraDadosBroadcastUdp(d);
}

void SocketUdp::Recebe(
    std::string* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente) {
  DadosParaReceberUdp dudp;
  dudp.dados = dados;
  dudp.endereco = endereco;
  dudp.callback = callback_recepcao_cliente;
  dudp.socket_cliente = interno_->socket_;
  sincronizador_->interno_->EnfileiraDadosRecepcaoUdp(dudp);
}

//----------
// Aceitador
//----------
struct Aceitador::Interno {
  explicit Interno(Sincronizador* sincronizador) : socket_(sincronizador) {}
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador;
  Socket socket_;
};
Aceitador::Aceitador(Sincronizador* sincronizador)
    : sincronizador_(sincronizador), interno_(new Interno(sincronizador)) {}

Aceitador::~Aceitador() {}

bool Aceitador::Liga(int porta,
                     Socket* socket_cliente,
                     CallbackConexaoCliente callback_conexao_cliente) {
  return false;
  /*
  interno_->socket_.Ouve(porta);

  // now accept an incoming connection.
  DadosParaConexao d;
  d.socket_cliente = socket_cliente;
  d.socket_servidor = &interno_->socket_;
  d.callback = callback_conexao_cliente;
  return true;*/
}

bool Aceitador::Ligado() const { return interno_->socket_.interno_->socket_ != -1; }

void Aceitador::Desliga() {
}

// Impls.
/*static*/
void Sincronizador::Interno::LoopEnvioTcp(Interno* thiz) {
  while (!thiz->a_enviar_tcp_.empty()) {
    auto* de = thiz->a_enviar_tcp_.front();
    int total_enviado = 0;
    LOG(INFO) << "Enviando TCP";
    while (total_enviado < de->dados.size()) {
      int enviado = send(de->desc, de->dados.c_str() + total_enviado, de->dados.size() - total_enviado, 0);
      int tipo_erro = errno;
      if (enviado == -1) {
        LOG(ERROR) << "Erro enviando TCP: " << strerror(tipo_erro);
        de->erro = Erro("Erro enviando TCP");
        break;
      }
      total_enviado += enviado;
    }
    LOG(INFO) << "Enviado TCP: " << total_enviado;
    thiz->enviados_tcp_.push(de);
    thiz->a_enviar_tcp_.pop();
  }
}

/*static*/
void Sincronizador::Interno::LoopRecepcaoUdp(Interno* thiz, std::vector<int>* a_remover) {
  int nudp = 0;
  for (auto& par : thiz->sockets_) {
    DadosSocket& ds = par.second;
    if (ds.dados_udp.get() != nullptr) {
      LOG(INFO) << "Recebendo UDP";
      ++nudp;
      char buf[100];
      struct sockaddr from;
      socklen_t from_len = sizeof(from);
      ssize_t ret = recvfrom(par.first, buf, sizeof(buf), MSG_DONTWAIT, &from, &from_len);
      auto tipo_erro = errno;
      if (ret == -1 && (tipo_erro == EAGAIN || tipo_erro == EWOULDBLOCK)) {
        continue;
      }
      a_remover->push_back(par.first);
      if (ret == -1) {
        LOG(INFO) << "Recebi erro UDP";
        ds.dados_udp->erro = Erro("Erro recebendo UDP");
      } else {
        LOG(INFO) << "Recebi dados UDP, tam: " << ds.dados_udp->dados->size();
        auto* dudp = ds.dados_udp.get();
        dudp->endereco->assign(inet_ntoa(((sockaddr_in*)&from)->sin_addr));
        dudp->dados->assign(buf, ret);
      }
      thiz->recebidos_udp_.push(ds.dados_udp.release());
    }
  }
}

/*static*/
void Sincronizador::Interno::LoopRecepcaoTcp(Interno* thiz, std::vector<int>* a_remover) {
  fd_set conjunto_tcp;
  FD_ZERO(&conjunto_tcp);
  int maior = -1;
  int ntcp = 0;
  for (auto& par : thiz->sockets_) {
    DadosSocket& ds = par.second;
    if (ds.dados_tcp.get() != nullptr) {
      ++ntcp;
      FD_SET(par.first, &conjunto_tcp);
      maior = std::max(maior, par.first);
    }
  }
  if (maior == -1) {
    LOG(INFO) << "Nada a receber TCP";
    return;
  }
  LOG(INFO) << "Maior: " << maior;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int select_ret = select(maior + 1, &conjunto_tcp, nullptr, nullptr, &tv);
  if (select_ret == -1) {
    LOG(INFO) << "Erro no select: " << strerror(select_ret);
    return;
  }
  if (select_ret == 0) {
    LOG(INFO) << "Timeout select";
    return;
  }
  LOG(INFO) << "Select ret: " << select_ret;
  for (auto& par : thiz->sockets_) {
    DadosSocket& ds = par.second;
    if (ds.dados_tcp.get() != nullptr && FD_ISSET(par.first, &conjunto_tcp)) {
      a_remover->push_back(par.first);
      LOG(INFO) << "Recebendo TCP pos select";
      char buf[2000];
      ssize_t ret = recv(par.first, buf, sizeof(buf), 0);
      auto tipo_erro = errno;
      if (ret == -1) {
        LOG(ERROR) << "Erro recebendo TCP pos select: " << strerror(tipo_erro);
        ds.dados_tcp->erro = Erro("Erro recebendo TCP");
      } else {
        LOG(INFO) << "Recebido pos select, tam: " << ret;
        auto* dtcp = ds.dados_tcp.get();
        dtcp->dados->assign(buf, ret);
      }
      thiz->recebidos_tcp_.push(ds.dados_tcp.release());
    }
  }
}

/*static*/
void Sincronizador::Interno::Loop(Interno* thiz) {
  ScopedPrint sp("Loop");
  std::unique_lock<std::recursive_mutex> ulock(thiz->mutex_);
  while (!thiz->terminar_) {
    thiz->cond_.wait_for(ulock, std::chrono::milliseconds(100));
    std::vector<int> a_remover;
    LoopRecepcaoUdp(thiz, &a_remover);
    LoopRecepcaoTcp(thiz, &a_remover);
    for_each(a_remover.begin(), a_remover.end(), [thiz] (int chave) { thiz->sockets_.erase(chave); } );
    LoopEnvioTcp(thiz);
  }
}


}  // namespace net

#else
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include "log/log.h"
#include "net/socket.h"
#include "net/util.h"

namespace net {

//-----
// Erro
//-----
struct Erro::Interno {
  boost::system::error_code ec;
};

const Erro ConverteErro(const boost::system::error_code& ec) {
  return Erro((void*)&ec);
}

Erro::Erro(void* dependente_plataforma) : Erro() {
  interno_->ec = *((boost::system::error_code*)dependente_plataforma);
  erro_ = *interno_->ec;
  msg_ = interno_->ec.message();
}
Erro::Erro(const std::string& msg) : Erro(true) {
  msg_ = msg;
}
Erro::Erro(bool erro) : interno_(new Interno), erro_(erro) {
}

bool Erro::ConexaoFechada() const {
  return interno_->ec.value() == boost::asio::error::eof;
}

//--------------
// Sincronizador
//--------------
struct Sincronizador::Interno {
  Interno(void* depende_plataforma) : servico_io((boost::asio::io_service*)depende_plataforma) {}

  boost::asio::io_service* servico_io = nullptr;
};
Sincronizador::Sincronizador(void* depende_plataforma) : interno_(new Interno(depende_plataforma)) {}
Sincronizador::~Sincronizador() {}

int Sincronizador::Roda() {
  return interno_->servico_io->poll();
}

//----------
// SocketUdp
//----------
struct SocketUdp::Interno {
  Interno(boost::asio::ip::udp::socket* socket) : socket(socket) {}

  std::unique_ptr<boost::asio::ip::udp::socket> socket;
};

SocketUdp::SocketUdp(Sincronizador* sincronizador)
    : interno_(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io))) {}

SocketUdp::SocketUdp(Sincronizador* sincronizador, int porta) {
  boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v4(), porta);
  interno_.reset(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io, endereco)));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  boost::system::error_code erro;
  interno_->socket->open(boost::asio::ip::udp::v4(), erro);
  boost::asio::socket_base::broadcast option(true);
  interno_->socket->set_option(option);
}

bool SocketUdp::Aberto() const { return interno_->socket->is_open(); }

void SocketUdp::Fecha() {
  boost::system::error_code ec;
  interno_->socket->close(ec);
}

void SocketUdp::Envia(int porta, const std::string& dados, CallbackEnvio callback_envio_cliente) {
  interno_->socket->async_send_to(
      boost::asio::buffer(dados),
      boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), porta),
      [callback_envio_cliente, &dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(2) << "UDP Enviados: " << bytes_enviados << ", buffer: " << dados.size() << ", erro? " << ec.message();
    callback_envio_cliente(ConverteErro(ec), bytes_enviados);
  });
}

void SocketUdp::Recebe(
    std::string* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente) {
  auto* endpoint = new boost::asio::ip::udp::endpoint;
  interno_->socket->async_receive_from(
      boost::asio::buffer(&(*dados)[0], dados->size()),
      *endpoint,
      [endpoint, endereco, callback_recepcao_cliente, dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(2) << "UDP Recebidos: " << bytes_enviados << ", buffer: " << dados->size() << ", erro? " << ec.message();
    endereco->assign(endpoint->address().to_string());
    delete endpoint;
    callback_recepcao_cliente(ConverteErro(ec), bytes_enviados);
  });
}

//-------
// Socket
//-------
struct Socket::Interno {
  explicit Interno(boost::asio::ip::tcp::socket* socket) : socket(socket) {}

  std::unique_ptr<boost::asio::ip::tcp::socket> socket;
};

Socket::Socket(Sincronizador* sincronizador)
    : sincronizador_(sincronizador),
      interno_(new Interno(new boost::asio::ip::tcp::socket(*sincronizador->interno_->servico_io))) {
  try {
    boost::asio::ip::tcp::no_delay option(true);
    interno_->socket->set_option(option);
    LOG(INFO) << "No delay setado com sucesso";
  } catch (const std::exception& e) {
    LOG(WARNING) << "Nao consegui setar no_delay, realtime pode ser comprometido";
  }
}

Socket::~Socket() {}

void Socket::Conecta(const std::string& endereco, const std::string& porta) {
  boost::asio::ip::tcp::resolver resolver(*sincronizador_->interno_->servico_io);
  auto endereco_resolvido = resolver.resolve({endereco, porta});
  boost::asio::connect(*interno_->socket, endereco_resolvido);
}

void Socket::Fecha() {
  interno_->socket->close();
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
  boost::asio::async_write(
      *interno_->socket.get(),
      boost::asio::buffer(dados),
      [callback_envio_cliente, &dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(1) << "TCP Enviados " << bytes_enviados << ", buffer: " << dados.size() << ", erro? " << ec.message();
    callback_envio_cliente(ConverteErro(ec), bytes_enviados);
 });
}

void Socket::Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente) {
  boost::asio::async_read(
      *interno_->socket.get(),
      boost::asio::buffer(&(*dados)[0], dados->size()),
      [callback_recepcao_cliente, dados](const boost::system::error_code& ec, std::size_t bytes_recebidos) {
    VLOG(1) << "TCP Recebidos " << bytes_recebidos << ", buffer: " << dados->size() << ", erro? " << ec.message();
    callback_recepcao_cliente(ConverteErro(ec), bytes_recebidos);
  });
}

//----------
// Aceitador
//----------
struct Aceitador::Interno {
  std::unique_ptr<boost::asio::ip::tcp::acceptor> aceitador;
};
Aceitador::Aceitador(Sincronizador* sincronizador) : sincronizador_(sincronizador), interno_(new Interno) {

}
Aceitador::~Aceitador() {}

bool Aceitador::Liga(int porta,
          Socket* socket_cliente,
          CallbackConexaoCliente callback_conexao_cliente) {
  interno_->aceitador.reset(new boost::asio::ip::tcp::acceptor(
      *sincronizador_->interno_->servico_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), porta)));
  // Funcao recursiva para ser chamada a cada conexao.
  // Primeiro a gente cria o objeto para poder passa-lo para ele mesmo no lambda.
  auto* callback_conexao = new std::function<void(boost::system::error_code, CallbackConexaoCliente)>;
  (*callback_conexao) = [this, callback_conexao] (boost::system::error_code ec,
                                                  CallbackConexaoCliente callback_conexao_cliente) {
    // Chama callback do cliente com erro recebido.
    Socket* novo_socket_cliente = callback_conexao_cliente(Erro(&ec));
    if (novo_socket_cliente != nullptr) {
      // Se recebeu socket, chama de novo.
      interno_->aceitador->async_accept(
          *novo_socket_cliente->interno_->socket.get(),
          std::bind(*callback_conexao,
                    std::placeholders::_1,  // ec, a ser preenchido.
                    callback_conexao_cliente));
    } else {
      delete callback_conexao;
    }
  };

  // Quando um cliente for recebido, CallbackConexao sera chamado.
  interno_->aceitador->async_accept(
      *socket_cliente->interno_->socket.get(),
      std::bind(*callback_conexao,
                std::placeholders::_1,  // ec, a ser preenchido.
                callback_conexao_cliente)
  );
  return true;
}

bool Aceitador::Ligado() const { return interno_->aceitador.get() != nullptr; }

void Aceitador::Desliga() {
  interno_->aceitador.reset();
}

}  // namespace net
#endif
