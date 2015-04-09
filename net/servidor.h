#ifndef NET_SERVIDOR_H
#define NET_SERVIDOR_H

#include <memory>
#include <queue>
#include <set>
#include "ntf/notificacao.h"
#include "net/socket.h"

namespace net {

// Protocolo:
// - Servidor ligado faz broadcast na rede local, porta PortaAnuncio. O protocolo de anuncio eh
// descrito no cliente.cpp e nao interfere na conexao aqui, serve apenas para descobrimento do endereco do
// servidor por parte do cliente.
// - Servidor espera conexao HTTP na porta PortaPadrao. Servidor espera na PortaPadrao de forma assincrona
// na funcao EsperaCliente.
// - Cliente conecta na PortaPadrao e envia imediatamente uma notificacao TN_RESPOSTA_CONEXAO contendo seu
// identificador local (cliente_bla). Notificacao eh enviada localmente tambem para que o id do cliente seja
// conhecido pelo tabuleiro.
// - Servidor na funcao EsperaCliente cria o cliente como cliente pendente e espera novos clientes. Dados do
// cliente serao recebidos na funcao RecebeDadosCliente.
// - Servidor na funcao RecebeDadosCliente recebe TN_RESPOSTA_CONEXAO e atribui o id do cliente na camada net.
// - Servidor cria no mesmo lugar TN_SERIALIZAR_TABULEIRO marcando a notificacao como para cliente pendente e
// com o id net do cliente. Envia localmente.
// - Tabuleiro recebe a notificacao local TN_SERIALIZAR_TABULEIRO e serializa o jogo criando
// TN_DESERIALIZAR_TABULEIRO com um novo id de tabuleiro e marcando-a como de cliente pendente. Se der erro,
// provavelmente servidor nao conseguiu criar id do tabuleiro e o cliente eh dropado.
// - Servidor em TrataNotificacaoRemota intercepta a mensagem por ser de cliente pendente e a envia para o
// o cliente correto. Cliente eh removido dos pendentes e adicionado nos clientes_ de verdade.
//
// Quando o cliente cai ou eh rejeitado, o fluxo eh:
// - Servidor na funcao RecebeDadosCliente desconecta o cliente (em outros lugares, deve-se fechar o socket
// do cliente para a RecebeDadosCliente tratar o fechamento).
// - Servidor na funcao DesconectaCliente o retira do mapa que estiver e deleta o ponteiro.
// - Servidor envia TN_DESCONECTADO localmente.
// - Tabuleiro trata a mensagem atualizando mapa de clientes.
class Servidor : public ntf::Receptor, public ntf::ReceptorRemoto {
 public:
  // Nao possui os parametros.
  Servidor(Sincronizador* sincronizador, ntf::CentralNotificacoes* central);
  virtual ~Servidor();

  virtual bool TrataNotificacao(const ntf::Notificacao& notificacao) override;
  virtual bool TrataNotificacaoRemota(const ntf::Notificacao& notificacao) override;

 private:
  struct Cliente {
    // tamanho maximo da mensagem: 1MB.
    Cliente(Socket* socket) : socket(socket), buffer_tamanho(4, '\0') {}
    Cliente() : Cliente(nullptr) {}
    std::string id;
    std::unique_ptr<Socket> socket;
    // Buffer para receber tamanho dos dados.
    std::string buffer_tamanho;
    // Buffer de recepcao dos dados.
    std::string buffer_recepcao;
    // Dados para enviar.
    std::queue<std::string> fifo_envio;
  };

  // Liga o aceitador para receber clientes de forma assincrona e renova automaticamente sempre que um cliente aparece.
  // Para cada cliente, chama RecebeDadosCliente e nao bloca.
  void Liga();
  // Desliga o servidor.
  void Desliga();

  // Retorna se o servidor esta ligado.
  bool Ligado() const;

  // Chama a funcao de recepcao de dados de forma assincrona para o cliente.
  void RecebeDadosCliente(Cliente* cliente);
  // Envia ou enfileira dados a serem enviados para um cliente. Assincrono. Se sem dados for true, envia a primeira mensagem enfileirada.
  void EnviaDadosCliente(Cliente* cliente, const std::string& dados, bool sem_dados = false);
  // Desconecta um cliente do servidor, efetivamente destruindo sua estrutura e deletando o ponteiro.
  void DesconectaCliente(Cliente* cliente);

  ntf::CentralNotificacoes* central_;
  Sincronizador* sincronizador_;
  std::unique_ptr<Aceitador> aceitador_;
  std::string buffer_porta_;  // usado para anunciar a porta do servidor.
  std::unique_ptr<SocketUdp> anunciante_;
  int timer_anuncio_ = 0;
  std::unique_ptr<Cliente> proximo_cliente_;
  std::set<Cliente*> clientes_pendentes_;
  std::set<Cliente*> clientes_;
};

}  // namespace net

#endif
