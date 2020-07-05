#if 0 && ANDROID
#include <algorithm>
#include <cstring>
#include <condition_variable>
#include <errno.h>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <queue>
#include <sys/select.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
//#define VLOG_NIVEL 1
#include "log/log.h"
#include "net/socket.h"
#include "net/util.h"

namespace net {

namespace {
struct ScopedPrint {
  explicit ScopedPrint(const std::string& p) {
    VLOG(1) << "Entrando " << p;
    p_ = p;
  }
  ~ScopedPrint() {
    VLOG(1) << "Saindo " << p_;
  }

  std::string p_;
};

// Estado do envio TCP.
struct DadosParaEnviarTcp {
  int desc;
  std::string dados;
  Socket::CallbackEnvio callback;
  Erro erro;
};

// Estado da recepcao TCP.
struct DadosParaReceberTcp {
  int desc;
  int recebido;
  std::string* dados;
  Erro erro;
  Socket::CallbackRecepcao callback;
};

// Estado da recepcao UDP.
struct DadosParaReceberUdp {
  int desc;
  std::string* endereco;
  std::string* dados;
  Erro erro;
  SocketUdp::CallbackRecepcao callback;
};

}  // namespace

//-----
// Erro
//-----
struct Erro::Interno {
  bool conexao_fechada = false;
};

Erro::Erro(void* dependente_plataforma) : Erro() {
}

Erro::Erro(const std::string& msg) : Erro(true) {
  msg_ = msg;
}

Erro::Erro(bool erro) : interno_(new Interno), erro_(erro) {
}

bool Erro::ConexaoFechada() const {
  return interno_->conexao_fechada;
}

//--------------
// Sincronizador
//--------------
struct Sincronizador::Interno {
 public:
  Interno(void* depende_plataforma) {
    thread_ = std::move(std::thread(Sincronizador::Interno::Loop, this));
  }
  ~Interno() {
    terminar_ = true;
    thread_.join();
  }

  // Loop do sincronizador.
  static void Loop(Interno* thiz);
  static void LoopRecepcaoTcp(Interno* thiz);
  static void LoopRecepcaoUdp(Interno* thiz);
  static void LoopEnvioTcp(Interno* thiz);


  int Roda() {
    //ScopedPrint sp("Roda");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!recebidos_udp_.empty()) {
      auto* recebido = recebidos_udp_.front().get();
      VLOG(1) << "Retornando erro udp? " << ((bool)recebido->erro)
              << ", mensagem: '" << recebido->erro.mensagem() << "', endereco: " << *recebido->endereco;
      recebido->callback(recebido->erro, recebido->erro ? 0 : recebido->dados->size());
      recebidos_udp_.pop();
    }
    if (!recebidos_tcp_.empty()) {
      auto* recebido = recebidos_tcp_.front().get();
      VLOG(1) << "Retornando erro tcp? " << ((bool)recebido->erro)
              << ", mensagem: '" << recebido->erro.mensagem() << "', tam: " << recebido->dados->size();
      recebido->callback(recebido->erro, recebido->erro ? 0 : recebido->dados->size());
      recebidos_tcp_.pop();
    }
    if (!enviados_tcp_.empty()) {
      auto* enviado = enviados_tcp_.front().get();
      VLOG(1) << "Retornando erro envio tcp? " << ((bool)enviado->erro)
              << ", mensagem: " << enviado->erro.mensagem();
      enviado->callback(enviado->erro, enviado->erro ? 0 : enviado->dados.size());
      enviados_tcp_.pop();
    }
    return 1;
  }

  // TCP.
  void EnfileiraDadosEnvioTcp(DadosParaEnviarTcp* dtcp) {
    //ScopedPrint sp("EnfileiraDadosEnvioTcp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      a_enviar_tcp_.push(std::unique_ptr<DadosParaEnviarTcp>(dtcp));
    }
    cond_.notify_one();
  }

  void EnfileiraDadosRecepcaoTcp(DadosParaReceberTcp* dtcp) {
    //ScopedPrint sp("EnfileiraDadosRecepcaoTcp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      sockets_tcp_.insert(std::make_pair(dtcp->desc, std::unique_ptr<DadosParaReceberTcp>(dtcp)));
    }
    cond_.notify_one();
  }

  // UDP.
  void EnfileiraDadosRecepcaoUdp(DadosParaReceberUdp* dudp) {
    //ScopedPrint sp("EnfileiraDadosRecepcaoUdp");
    {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      sockets_udp_.insert(std::make_pair(dudp->desc, std::unique_ptr<DadosParaReceberUdp>(dudp)));
    }
    cond_.notify_one();
  }

  void AlternaHackAndroid() { hack_android = !hack_android; }

  // Aceitador.

  std::thread thread_;
  bool terminar_ = false;

 private:
  std::queue<std::unique_ptr<DadosParaEnviarTcp>> a_enviar_tcp_;
  std::queue<std::unique_ptr<DadosParaEnviarTcp>> enviados_tcp_;

  // Para select.
  std::map<int, std::unique_ptr<DadosParaReceberTcp>> sockets_tcp_;
  std::queue<std::unique_ptr<DadosParaReceberTcp>> recebidos_tcp_;
  // Para recepcao udp.
  std::map<int, std::unique_ptr<DadosParaReceberUdp>> sockets_udp_;
  std::queue<std::unique_ptr<DadosParaReceberUdp>> recebidos_udp_;

  // Sincronizacao.
  std::condition_variable_any cond_;
  std::recursive_mutex mutex_;

  bool hack_android = false;
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
  throw std::logic_error("NAO IMPLEMENTADO");
#if 0
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
#endif
}

void Socket::PortaLocal(int porta) {
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
}

void Socket::Conecta(const std::string& endereco, const std::string& porta) {
  struct addrinfo hints, *res;
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
  VLOG(1) << "Conectado ao servidor";
}

void Socket::Fecha() {
  close(interno_->socket_);
  interno_->socket_ = -1;
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
  auto* dtcp = new DadosParaEnviarTcp;
  dtcp->desc = interno_->socket_;
  dtcp->dados = dados;
  dtcp->callback = callback_envio_cliente;
  sincronizador_->interno_->EnfileiraDadosEnvioTcp(dtcp);
}

void Socket::Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente) {
  auto* dtcp = new DadosParaReceberTcp;
  dtcp->recebido = 0;
  dtcp->desc = interno_->socket_;
  dtcp->dados = dados;
  dtcp->callback = callback_recepcao_cliente;
  sincronizador_->interno_->EnfileiraDadosRecepcaoTcp(dtcp);
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
  callback_envio_cliente(Erro("NAO IMPLEMENTADO"), 0);
}

void SocketUdp::Recebe(
    std::string* dados, std::string* endereco, CallbackRecepcao callback_recepcao_cliente) {
  auto* dudp = new DadosParaReceberUdp;
  dudp->dados = dados;
  dudp->endereco = endereco;
  dudp->callback = callback_recepcao_cliente;
  dudp->desc = interno_->socket_;
  sincronizador_->interno_->EnfileiraDadosRecepcaoUdp(dudp);
}

//----------
// Aceitador
//----------
struct Aceitador::Interno {
  explicit Interno(Sincronizador* sincronizador) : socket_(sincronizador) {}
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
    auto* de = thiz->a_enviar_tcp_.front().get();
    unsigned int total_enviado = 0;
    VLOG(1) << "Enviando TCP";
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
    VLOG(1) << "Enviado TCP: " << total_enviado;
    thiz->enviados_tcp_.push(std::move(thiz->a_enviar_tcp_.front()));
    thiz->a_enviar_tcp_.pop();
  }
}

/*static*/
void Sincronizador::Interno::LoopRecepcaoUdp(Interno* thiz) {
  std::vector<int> a_remover;
  for (auto& par : thiz->sockets_udp_) {
    DadosParaReceberUdp* dudp = par.second.get();
    VLOG(1) << "Recebendo UDP";
    char buf[100];
    struct sockaddr from;
    socklen_t from_len = sizeof(from);
    ssize_t ret = recvfrom(par.first, buf, sizeof(buf), MSG_DONTWAIT, &from, &from_len);
    auto tipo_erro = errno;
    if (ret == -1 && (tipo_erro == EAGAIN || tipo_erro == EWOULDBLOCK)) {
      continue;
    }
    a_remover.push_back(par.first);
    if (ret == -1) {
      LOG(ERROR) << "Recebi erro UDP";
      dudp->erro = Erro("Erro recebendo UDP");
    } else {
      VLOG(1) << "Recebi dados UDP, tam: " << dudp->dados->size();
      dudp->endereco->assign(inet_ntoa(((sockaddr_in*)&from)->sin_addr));
      dudp->dados->assign(buf, ret);
    }
    thiz->recebidos_udp_.push(std::move(par.second));
  }
  std::for_each(a_remover.begin(), a_remover.end(), [thiz] (int i) { thiz->sockets_udp_.erase(i); });
}

/*static*/
void Sincronizador::Interno::LoopRecepcaoTcp(Interno* thiz) {
  fd_set conjunto_tcp;
  FD_ZERO(&conjunto_tcp);
  int maior = -1;
  for (auto& par : thiz->sockets_tcp_) {
    FD_SET(par.first, &conjunto_tcp);
    maior = std::max(maior, par.first);
  }
  if (maior == -1) {
    VLOG(2) << "Nada a receber TCP";
    return;
  }
  VLOG(1) << "Maior: " << maior;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10000;
  if (thiz->hack_android) {
    for (int i = 0; i < 1000; ++i) {
      // so pra acordar o processador.
      ;
    }
  }
  int select_ret = select(maior + 1, &conjunto_tcp, nullptr, nullptr, &tv);
  if (select_ret == -1) {
    LOG(ERROR) << "Erro no select: " << strerror(select_ret);
    return;
  }
  if (select_ret == 0) {
    VLOG(1) << "Timeout select";
    return;
  }
  VLOG(1) << "Select ret: " << select_ret;
  std::vector<int> a_remover;
  for (auto& par : thiz->sockets_tcp_) {
    auto* dtcp = par.second.get();
    if (!FD_ISSET(par.first, &conjunto_tcp)) {
      continue;
    }
    int a_receber = dtcp->dados->size() - dtcp->recebido;
    VLOG(1) << "Recebendo TCP pos select, esperando " << a_receber;
    ssize_t ret = recv(par.first, &(*dtcp->dados)[dtcp->recebido], a_receber, 0);
    auto tipo_erro = errno;
    if (ret == -1) {
      LOG(ERROR) << "Erro recebendo TCP pos select: " << strerror(tipo_erro);
      dtcp->erro = Erro("Erro recebendo TCP");
    } else if (ret >  a_receber) {
      LOG(ERROR) << "Erro recebendo TCP pos select: dados maior que buffer";
      dtcp->erro = Erro("Erro recebendo TCP");
    } else if (ret < a_receber) {
      VLOG(1) << "Recebido pos select parcial, tam: " << ret;
      dtcp->recebido += ret;
      continue;
    } else {
      VLOG(1) << "Recebido pos select, tam: " << ret;
    }
    thiz->recebidos_tcp_.push(std::unique_ptr<DadosParaReceberTcp>(par.second.release()));
    a_remover.push_back(par.first);
  }
  std::for_each(a_remover.begin(), a_remover.end(), [thiz] (int i) { thiz->sockets_tcp_.erase(i); } );
}

/*static*/
void Sincronizador::Interno::Loop(Interno* thiz) {
  std::unique_lock<std::recursive_mutex> ulock(thiz->mutex_);
  while (!thiz->terminar_) {
    thiz->cond_.wait_for(ulock, std::chrono::milliseconds(20));
    //ScopedPrint sp("Loop");
    LoopRecepcaoUdp(thiz);
    LoopRecepcaoTcp(thiz);
    LoopEnvioTcp(thiz);
  }
}

void Sincronizador::AlternaHackAndroid() {
  interno_->AlternaHackAndroid();
}

}  // namespace net

#else
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include "goog/stringprintf.h"

#include "log/log.h"
#include "net/socket.h"
#include "net/util.h"

namespace net {
namespace {
using google::protobuf::StringPrintf;
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
  erro_ = interno_->ec.value() != boost::system::errc::success;
  msg_ = interno_->ec.message();
  msg_ += ", cod: " + to_string(interno_->ec.value());
}
Erro::Erro(const std::string& msg) : Erro(true) {
  msg_ = msg;
}
Erro::Erro(bool erro) : interno_(new Interno), erro_(erro) {}

bool Erro::ConexaoFechada() const {
  // O valor 2 esta sendo recebido quando a conexao eh fechada (eof).
  return interno_->ec == boost::system::errc::connection_aborted || interno_->ec == boost::system::errc::connection_reset || interno_->ec.value() == boost::asio::error::eof;
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
  boost::asio::ip::udp::endpoint endereco(boost::asio::ip::udp::v6(), porta);
  interno_.reset(new Interno(new boost::asio::ip::udp::socket(*sincronizador->interno_->servico_io, endereco)));
}

SocketUdp::~SocketUdp() {}

void SocketUdp::Abre() {
  boost::system::error_code erro;
  interno_->socket->open(boost::asio::ip::udp::v6(), erro);
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

  std::string IpString() const {
    return socket == nullptr ? "null" : socket->remote_endpoint().address().to_string();
  }

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

void Socket::PortaLocal(int porta_local) {
  interno_->socket->open(boost::asio::ip::tcp::v6());
  boost::system::error_code ec;
  interno_->socket->bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), porta_local), ec);
  if (ec) {
    throw std::logic_error(StringPrintf("Erro no bind do cliente na porta %d", porta_local));
  }
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
#if 1
  // Write envia tudo em uma so chamada.
  boost::asio::async_write(
      *interno_->socket.get(),
      boost::asio::buffer(dados),
      [callback_envio_cliente, &dados] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    VLOG(1) << "TCP Enviados " << bytes_enviados << ", buffer: " << dados.size() << ", erro? " << ec.message();
    callback_envio_cliente(ConverteErro(ec), bytes_enviados);
 });
#else
  // Send tem que enviar cada parte.
  int* enviado = new int(0);
  auto* callback_send = new std::function<void(const boost::system::error_code& ec, std::size_t bytes_enviados)>;
  (*callback_send) = [this, callback_envio_cliente, &dados, enviado, callback_send] (const boost::system::error_code& ec, std::size_t bytes_enviados) {
    *enviado += bytes_enviados;
    if (*enviado < dados.size() && !ec) {
      VLOG(1) << "TCP parcialmente enviado: " << *enviado << " de buffer tamanho: " << dados.size();
      interno_->socket->async_send(boost::asio::buffer(dados.data() + (*enviado), dados.size() - *enviado), *callback_send);
    } else {
      VLOG(1) << "TCP finalizado, enviado " << *enviado << " de buffer: " << dados.size() << ", erro? " << ec.message();
      callback_envio_cliente(ConverteErro(ec), bytes_enviados);
      delete callback_send;
      delete enviado;
    }
  };
  interno_->socket->async_send(boost::asio::buffer(dados), *callback_send);
#endif
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

std::string Socket::IpString() const {
  return interno_->IpString();
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
      *sincronizador_->interno_->servico_io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), porta)));
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
