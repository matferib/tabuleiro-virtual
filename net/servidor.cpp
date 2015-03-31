#include <algorithm>
#include <boost/asio.hpp>
#include <boost/asio/error.hpp>

#include "ent/constantes.h"
#include "log/log.h"
#include "net/servidor.h"
#include "net/util.h"
#include "ntf/notificacao.h"
#include "ntf/notificacao.pb.h"

namespace net {

Servidor::Servidor(boost::asio::io_service* servico_io, ntf::CentralNotificacoes* central) {
  servico_io_ = servico_io;
  central_ = central;
  central_->RegistraReceptor(this);
}

Servidor::~Servidor() {
}

bool Servidor::TrataNotificacao(const ntf::Notificacao& notificacao) {
  if (notificacao.tipo() == ntf::TN_TEMPORIZADOR) {
    ++timer_anuncio_;
    if (timer_anuncio_ * INTERVALO_NOTIFICACAO_MS >= 1000) {
      timer_anuncio_ = 0;
      if (anunciante_.get() != nullptr) {
        std::string porta(to_string(PortaPadrao()));
        anunciante_->async_send_to(
            boost::asio::buffer(porta),
            boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("255.255.255.255"), 11224),
            [] (const boost::system::error_code& error, std::size_t bytes_transferred) {});
      }
    }
    if (Ligado()) {
      servico_io_->poll_one();
    }
    return true;
  } else if (notificacao.tipo() == ntf::TN_INICIAR) {
    Liga();
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
      cliente_pendente->socket->close();
      return true;
    }
    VLOG(1) << "Enviando primeira notificacao para cliente pendente";
    EnviaDadosCliente(cliente_pendente->socket.get(), ns);
    clientes_.insert(cliente_pendente);
  } else {
    for (auto* c : clientes_) {
      if (notificacao.has_id_rede() && c->id != notificacao.id_rede()) {
        VLOG(1) << "Dropando notificacao pq id cliente diferente: " << notificacao.id_rede() << " x " << c->id;
        continue;
      }
      VLOG(1) << "Enviando notificacao para cliente";
      EnviaDadosCliente(c->socket.get(), ns);
    }
  }

  return true;
}

bool Servidor::Ligado() const {
  return aceitador_ != nullptr;
}

void Servidor::Liga() {
  VLOG(1) << "Ligando servidor.";
  try {
    proximo_cliente_.reset(new Cliente(new boost::asio::ip::tcp::socket(*servico_io_)));
    aceitador_.reset(new boost::asio::ip::tcp::acceptor(
        *servico_io_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PortaPadrao())));
    central_->RegistraReceptorRemoto(this);
    EsperaCliente();
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
  anunciante_.reset(new boost::asio::ip::udp::socket(*servico_io_));
  boost::system::error_code erro;
  anunciante_->open(boost::asio::ip::udp::v4(), erro);
  boost::asio::socket_base::broadcast option(true);
  anunciante_->set_option(option);
}

void Servidor::Desliga() {
  VLOG(1) << "Desligando servidor.";
  if (!Ligado()) {
    LOG(ERROR) << "Servidor ja está desligado.";
    return;
  }
  central_->DesregistraReceptorRemoto(this);
  aceitador_.reset();
  anunciante_.reset();
  for (auto* c : clientes_) {
    delete c;
  }
  for (auto* c : clientes_pendentes_) {
    delete c;
  }
  VLOG(1) << "Servidor desligado.";
}

void Servidor::EsperaCliente() {
  aceitador_->async_accept(*proximo_cliente_->socket.get(), [this](boost::system::error_code ec) {
    if (!ec) {
      VLOG(1) << "Recebendo cliente...";
      Cliente* cliente_pendente = proximo_cliente_.release();
      clientes_pendentes_.insert(cliente_pendente);
      proximo_cliente_.reset(new Cliente(new boost::asio::ip::tcp::socket(*servico_io_)));
      RecebeDadosCliente(cliente_pendente);
      EsperaCliente();
    } else {
      LOG(ERROR) << "Recebendo erro..." << ec.message();
    }
  });
}

void Servidor::EnviaDadosCliente(boost::asio::ip::tcp::socket* cliente, const std::string& dados) {
  std::vector<char> dados_codificados(CodificaDados(dados));
  //size_t bytes_enviados = cliente->send(boost::asio::buffer(dados_codificados));
  try {
    size_t bytes_enviados = boost::asio::write(*cliente, boost::asio::buffer(dados_codificados));
    if (bytes_enviados != dados_codificados.size()) {
      LOG(ERROR) << "Erro enviando dados, enviados: " << bytes_enviados << " de " << dados_codificados.size();
    } else {
      VLOG(2) << "Enviei " << dados.size() << " bytes pro cliente.";
    }
  } catch (const std::exception& e) {
    // Faz nada aqui que provavalmente o receive vai receber o erro.
    LOG(ERROR) << "Erro enviando dados: " << e.what();
  }
}

// deve ser usada apenas na funcao de RecebeDadosCliente.
void Servidor::DesconectaCliente(Cliente* cliente) {
  // Notifica interessados na desconexao.
  std::unique_ptr<ntf::Notificacao> n(ntf::NovaNotificacao(ntf::TN_DESCONECTADO));
  n->set_id_rede(cliente->id);
  central_->AdicionaNotificacao(n.release());

  // Apaga o cliente.
  clientes_.erase(cliente);
  clientes_pendentes_.erase(cliente);
  delete cliente;
}

void Servidor::RecebeDadosCliente(Cliente* cliente) {
  cliente->socket->async_receive(
    boost::asio::buffer(cliente->buffer),
    [this, cliente](boost::system::error_code ec, std::size_t bytes_recebidos) {
      if (ec) {
        // remove o cliente.
        auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
        std::string erro(std::string("Erro recebendo dados do cliente '") + cliente->id + "': ");
        if (ec.value() == boost::asio::error::eof) {
          erro += "Conexao fechada pela outra ponta.";
        } else {
          erro += to_string(ec.value()) + ": " + ec.message();
        }
        n->set_erro(erro);
        central_->AdicionaNotificacao(n);
        DesconectaCliente(cliente);
        return;
      }
      VLOG(2) << "Recebi " << bytes_recebidos << " bytes do cliente " << cliente->id;
      auto buffer_inicio = cliente->buffer.begin();
      auto buffer_fim = buffer_inicio + bytes_recebidos;
      std::size_t bytes_faltando = bytes_recebidos;
      do {
        if (cliente->a_receber_ == 0) {
          if (bytes_faltando < 4) {
            std::string erro(std::string("Erro recebendo dados do cliente '") + cliente->id + "' , msg menor que tamanho.");
            LOG(ERROR) << erro
                       << ", bytes_recebidos: " << bytes_recebidos
                       << ", bytes_faltando: " << bytes_faltando;
            auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
            n->set_erro(erro);
            central_->AdicionaNotificacao(n);
            DesconectaCliente(cliente);
            return;
          }
          cliente->a_receber_ = DecodificaTamanho(buffer_inicio);
          if (cliente->a_receber_ > 1024 * 1024) {
            LOG(ERROR) << "recebendo mensagem maior que 1MB, impossivel. Algum problema podera acontecer. "
                << "Cliente: " << cliente->id << " "
                << ", a_receber: " << cliente->a_receber_
                << ", bytes_recebidos: " << bytes_recebidos
                << ", bytes_faltando: " << bytes_faltando;
          }
          buffer_inicio += 4;
          bytes_faltando -= 4;
        }
        if ((buffer_fim - buffer_inicio) >= cliente->a_receber_) {
          VLOG(2) << "Recebendo notificacao inteira de "
                  << cliente->id << " "
                  << ", buffer_fim: " << (int)(buffer_fim - cliente->buffer.begin())
                  << ", buffer_inicio: " << (int)(buffer_inicio - cliente->buffer.begin())
                  << ", a_receber: " << cliente->a_receber_
                  << ", bytes_recebidos: " << bytes_recebidos
                  << ", bytes_faltando: " << bytes_faltando;

          // Quantidade de dados recebida eh maior ou igual ao esperado (por exemplo, ao receber duas mensagens juntas).
          cliente->buffer_notificacao.insert(cliente->buffer_notificacao.end(), buffer_inicio, buffer_inicio + cliente->a_receber_);
          // Decodifica mensagem e poe na central.
          std::unique_ptr<ntf::Notificacao> notificacao(new ntf::Notificacao);
          if (!notificacao->ParseFromString(cliente->buffer_notificacao)) {
            std::string erro(std::string("Erro ParseFromString recebendo dados do cliente '") + cliente->id + "'");
            LOG(ERROR) << erro
                       << ", buffer_fim: " << (int)(buffer_fim - cliente->buffer.begin())
                       << ", buffer_inicio: " << (int)(buffer_inicio - cliente->buffer.begin())
                       << ", a_receber: " << cliente->a_receber_
                       << ", bytes_recebidos: " << bytes_recebidos
                       << ", bytes_faltando: " << bytes_faltando;
            auto* n = ntf::NovaNotificacao(ntf::TN_ERRO);
            n->set_erro(erro);
            central_->AdicionaNotificacao(n);
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
              auto* erro = ntf::NovaNotificacao(ntf::TN_ERRO);
              erro->set_erro(std::string("Id de cliente repetido: '") + notificacao->id_rede() + "'");
              central_->AdicionaNotificacao(erro);
              return;
            }
            cliente->id = notificacao->id_rede();
            auto* resposta = ntf::NovaNotificacao(ntf::TN_SERIALIZAR_TABULEIRO);
            resposta->set_clientes_pendentes(true);
            resposta->set_id_rede(cliente->id);
            central_->AdicionaNotificacao(resposta);
          }
          // Envia a notificacao para os outros clientes.
          for (auto* c : clientes_) {
            if (c == cliente) {
              // Nao envia para o cliente original.
              continue;
            }
            EnviaDadosCliente(c->socket.get(), cliente->buffer_notificacao);
          }
          // Processa localmente.
          notificacao->set_local(false);
          central_->AdicionaNotificacao(notificacao.release());
          cliente->buffer_notificacao.clear();
          buffer_inicio += cliente->a_receber_;
          bytes_faltando -= cliente->a_receber_;
          cliente->a_receber_ = 0;
        } else {
          VLOG(2) << "Recebendo notificacao parcial de "
                  << cliente->id << " "
                  << ", buffer_fim: " << (int)(buffer_fim - cliente->buffer.begin())
                  << ", buffer_inicio: " << (int)(buffer_inicio - cliente->buffer.begin())
                  << ", a_receber: " << cliente->a_receber_
                  << ", bytes_recebidos: " << bytes_recebidos
                  << ", bytes_faltando: " << bytes_faltando;
          // Quantidade de dados recebida eh menor que o esperado. Poe no buffer
          // e sai.
          cliente->buffer_notificacao.insert(cliente->buffer_notificacao.end(), buffer_inicio, buffer_fim);
          cliente->a_receber_ -= (buffer_fim - buffer_inicio);
          buffer_inicio = buffer_fim;
        }
      } while (buffer_inicio != buffer_fim);
      VLOG(2) << "Tudo recebido de cliente " << cliente->id;
      RecebeDadosCliente(cliente);
    }
  );
}

}  // namespace net
