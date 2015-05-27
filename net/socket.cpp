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
struct DadosParaEnviar {
  int desc;
  std::string dados;
  Socket::CallbackEnvio callback;
};

// O que deve ser recebido.
struct DadosParaReceber {
  int desc;
  std::string* dados;
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

  int Roda() {
    static int contador = 99;
    if (++contador == 100) {
      LOG(INFO) << "RODA!!!!!!!!!";
      contador = 0;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!recebidos_udp_.empty()) {
      auto* recebido = recebidos_udp_.front();
      LOG(INFO) << "Retornando erro? '" << recebido->erro.mensagem() << "', endereco: " << *recebido->endereco;
      recebido->callback(recebido->erro, recebido->erro ? 0 : recebido->dados->size());
      recebidos_udp_.pop();
      delete recebido;
    }
    return 1;
  }

  // TCP.
  void EnfileiraDadosEnvio(const DadosParaEnviar& d) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      a_enviar_tcp_.push(d);
    }
    cond_.notify_one();
  }

  void EnfileiraDadosRecepcao(const DadosParaReceber &d) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      a_receber_tcp_.push(d);
    }
    cond_.notify_one();
  }

  // UDP.
  void EnfileiraDadosBroadcastUdp(const DadosParaBroadcastUdp& d) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      broadcast_udp_.push(d);
    }
    cond_.notify_one();
  }

  void EnfileiraDadosRecepcaoUdp(const DadosParaReceberUdp& dudp) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
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
  void EnfileiraDadosParaConexao(const DadosParaConexao& d) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      a_receber_conexao_.push(d);
    }
    cond_.notify_one();
  }

  boost::asio::io_service* servico_io = nullptr;
  std::thread thread_;
  bool terminar_ = false;

 private:
  std::queue<DadosParaEnviar> a_enviar_tcp_;
  std::queue<DadosParaEnviar> enviados_tcp_;

  std::queue<DadosParaReceber> a_receber_tcp_;
  std::queue<DadosParaReceber> recebidos_tcp_;

  std::queue<DadosParaBroadcastUdp> broadcast_udp_;

  std::queue<DadosParaReceberUdp*> recebidos_udp_;

  std::queue<DadosParaConexao> a_receber_conexao_;
  std::condition_variable cond_;
  std::mutex mutex_;

  // Para select.
  struct DadosSocket {
    std::unique_ptr<DadosParaReceber> dados_tcp;
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
  int erro = connect(interno_->socket_, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  if (erro == -1) {
    throw std::logic_error("Falha ao conectar ao servidor");
  }
}

void Socket::Fecha() {
  close(interno_->socket_);
  interno_->socket_ = -1;
}

void Socket::Envia(const std::string& dados, CallbackEnvio callback_envio_cliente) {
  callback_envio_cliente(Erro("NAO IMPLEMENTADO"), 0);
  return;
  DadosParaEnviar d;
  d.desc = interno_->socket_;
  d.dados = dados;
  d.callback = callback_envio_cliente;
  sincronizador_->interno_->EnfileiraDadosEnvio(d);
}

void Socket::Recebe(std::string* dados, CallbackRecepcao callback_recepcao_cliente) {
  callback_recepcao_cliente(Erro("NAO IMPLEMENTADO"), 0);
  return;
  DadosParaReceber d;
  d.desc = interno_->socket_;
  d.dados = dados;
  d.callback = callback_recepcao_cliente;
  sincronizador_->interno_->EnfileiraDadosRecepcao(d);
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
  interno_->socket_.Ouve(porta);

  // now accept an incoming connection.
  DadosParaConexao d;
  d.socket_cliente = socket_cliente;
  d.socket_servidor = &interno_->socket_;
  d.callback = callback_conexao_cliente;
  sincronizador_->interno_->EnfileiraDadosParaConexao(d);
  return true;
}

bool Aceitador::Ligado() const { return interno_->socket_.interno_->socket_ != -1; }

void Aceitador::Desliga() {
}

// Impls.
/*static*/
void Sincronizador::Interno::Loop(Interno* thiz) {
  LOG(INFO) << "LOOP INICIADO!!!!!!!!!";
  std::unique_lock<std::mutex> ulock(thiz->mutex_);
  while (!thiz->terminar_) {
    thiz->cond_.wait_for(ulock, std::chrono::milliseconds(100));
    LOG(INFO) << "Num sockets: " << thiz->sockets_.size();
    std::vector<int> a_remover;
    for (auto& par : thiz->sockets_) {
      DadosSocket& ds = par.second;
      if (ds.dados_tcp.get() != nullptr) {
        // TODO Leu dado TCP.
      } else {
        char buf[100];
        struct sockaddr from;
        socklen_t from_len;
        ssize_t ret = recvfrom(par.first, buf, sizeof(buf), MSG_DONTWAIT, &from, &from_len);
        auto tipo_erro = errno;
        if (ret == -1 && (tipo_erro == EAGAIN || tipo_erro == EWOULDBLOCK)) {
          continue;
        }
        a_remover.push_back(par.first);
        if (ret == -1) {
          LOG(INFO) << "RECEBENDO ERRO";
          ds.dados_udp->erro = Erro("Erro recebendo UDP");
        } else {
          LOG(INFO) << "RECEBENDO DADO";
          auto* dudp = ds.dados_udp.get();
          dudp->endereco->assign(inet_ntoa(((sockaddr_in*)&from)->sin_addr));
          dudp->dados->assign(buf, ret);
        }
        thiz->recebidos_udp_.push(ds.dados_udp.release());
      }
    }
    for_each(a_remover.begin(), a_remover.end(), [thiz] (int chave) { thiz->sockets_.erase(chave); } );
  }
#if 0
    if (!thiz->a_enviar_.empty()) {
      // TODO tratar isso no nivel do net/servidor e net/cliente ou tirar do callback.
      const auto& de = thiz->a_enviar_.front();

      int total_sent = 0;
      while (total_sent < de.dados.size()) {
        int sent = send(de.desc, de.dados.c_str() + total_sent, de.dados.size() - total_sent, 0);
        if (sent == -1) {
          de.callback(Erro("Erro enviando"), 0);
          break;
        }
        total_sent += sent;
      }
      de.callback(Erro(false), total_sent);
      de.callback(Erro("NAO IMPLEMENTADO"), 0);
      thiz->a_enviar_.pop();
    }
    if (!thiz->a_receber_.empty()) {
      const auto& dr = thiz->a_receber_.front();
      // TODO
      dr.callback(Erro("NAO IMPLEMENTADO"), 0);
      thiz->a_receber_.pop();
    }
    if (!thiz->broadcast_udp_.empty()) {
      const auto& de = thiz->broadcast_udp_.front();
      struct hostent* he = gethostbyname("255.255.255.255");
      struct sockaddr_in their_addr;
      their_addr.sin_family = AF_INET;     // host byte order
      their_addr.sin_port = htons(de.porta); // short, network byte order
      their_addr.sin_addr = *((struct in_addr *)he->h_addr);
      memset(their_addr.sin_zero, '\0', sizeof(their_addr.sin_zero));
      int numbytes = 0;
      if ((numbytes = sendto(de.desc, de.dados.c_str(), de.dados.size(), 0,
                             (struct sockaddr *)&their_addr, sizeof(their_addr))) == -1) {
        de.callback(Erro("Erro broadcast"), 0);
      } else {
        de.callback(Erro(false), de.dados.size());
      }
      free(he);
      // TODO
      thiz->broadcast_udp_.pop();
    }
    if (!thiz->a_receber_udp_.empty()) {
      const auto& dr = thiz->a_receber_udp_.front();
      dr.callback(Erro("NAO IMPLEMENTADO"), 0);
      // TODO
      thiz->a_receber_udp_.pop();
    }
    if (!thiz->a_receber_conexao_.empty()) {
      auto& dc = thiz->a_receber_conexao_.front();
      struct sockaddr addr;
      socklen_t addrlen;
      int ret = accept(dc.socket_servidor->interno_->socket_, &addr, &addrlen);
      if (ret == -1 && errno != EWOULDBLOCK) {
        // TODO
        dc.callback(Erro("Erro conectando recebendo conexao"));
        thiz->a_receber_conexao_.pop();
      } else if (ret > 0) {
         dc.socket_cliente->interno_->socket_ = ret;
         dc.socket_cliente = dc.callback(Erro(false));
      }
      dc.callback(Erro("NAO IMPLEMENTADO"));
    }
#endif
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
