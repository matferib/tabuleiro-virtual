#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include <boost/timer/timer.hpp>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "ent/acoes.pb.h"
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "gltab/gl_vbo.h"

#define VBO_COM_MODELAGEM 0

namespace m3d {
class Modelos3d;
}  // namespace m3d
namespace ntf {
class CentralNotificacoes;
}  // namespace ntf
namespace ent {

class Entidade;
class Tabuleiro;
class DesenhoBase;
class IluminacaoDirecional;

/** Interface de texturas para entidades. */
class Texturas {
 public:
  /** Retorna o id da textura ou GL_INVALID_VALUE. */
  virtual unsigned int Textura(const std::string& id) const = 0;
  /** Retorna o tipo da textura: GL_TEXTURE_2D ou GL_TEXTURE_CUBE_MAP. */
  virtual unsigned int TipoTextura(const std::string& id) const = 0;
};

/** Constroi uma entidade de acordo com o proto passando, inicializando-a. */
Entidade* NovaEntidade(
    const EntidadeProto& proto,
    const Tabelas& tabelas, const Tabuleiro* tabuleiro, const Texturas* texturas, const m3d::Modelos3d* m3d,
    ntf::CentralNotificacoes* central, const ParametrosDesenho* pd);
inline Entidade* NovaEntidadeParaTestes(const EntidadeProto& proto, const Tabelas& tabelas) {
  return NovaEntidade(proto, tabelas, nullptr, nullptr, nullptr, nullptr, nullptr);
}
inline Entidade* NovaEntidadeFalsa(const Tabelas& tabelas) {
  return NovaEntidadeParaTestes(EntidadeProto::default_instance(), tabelas);
}

/** classe base para entidades.
* Toda entidade devera possuir um identificador unico.
*/
class Entidade {
 public:
  /** Inicializa a entidade, recebendo seu proto diretamente. */
  void Inicializa(const EntidadeProto& proto);

  /** Atualiza a entidade usando apenas alguns campos do proto passado. Nao atualiza posicao. */
  void AtualizaProto(const EntidadeProto& novo_proto);

  /** Atualiza a posição da entidade em direção a seu destino. Ao alcançar o destino, o limpa. */
  void Atualiza(int intervalo_ms);

  /** Retorna true se a entidade tiver luz (ou por proto, ou acao). */
  bool TemLuz() const;
  /** Retorna o raio da luz (assume que esta ligada). */
  float RaioLuzMetros() const { return proto_.luz().has_raio_m() ? proto_.luz().raio_m() : 6.0f; }
  /** Liga a iluminacao por acao da entidade, tipo quando da um tiro. */
  void AtivaLuzAcao(const IluminacaoPontual& luz);

  /** faz alvo fumegar. */
  void AtivaFumegando(int duracao_ms);

  /** Faz o alvo soltar bolhas, como nausea. */
  void AtivaBolhas(int duracao_ms, const float* cor);

  /** Destroi a entidade. */
  ~Entidade();

  /** @return o identificador da entidade que deve ser unico globalmente. */
  unsigned int Id() const { return proto_.has_id() ? proto_.id() : IdInvalido; }

  /** Retorna a cor da entidade. */
  const Cor& CorDesenho() const { return proto_.cor(); }

  /** Retorna o tipo das entidade: TE_ENTIDADE, TE_FORMA, TE_COMPOSTA. */
  TipoEntidade Tipo() const { return proto_.tipo(); }
  // Retorna true se qualquer um dos tipo DND da entidade bater com tipo.
  bool TemTipoDnD(TipoDnD tipo) const;
  bool TemSubTipoDnD(SubTipoDnD sub_tipo) const;

  bool Achatar() const {
    return Achatar(proto_, parametros_desenho_);
  }

  // Personagem esta indefeso (helpless).
  bool Indefeso() const;

  /** Exporta o VBO ja extraido.
  * @throw caso nao haja ainda (por exemplo, carregando modelo 3d).
  */
  const gl::VbosGravados* VboExtraido() const {
    if (vd_.vbos_gravados.Vazio()) {
      throw std::logic_error("vbo vazio");
    }
    return &vd_.vbos_gravados;
  }

  /** Extrai o VBO da entidade na posicao do mundo. Se desejavel a posicao
  * de modelagem, usar CorrigeVboRaiz.
  */
  gl::VbosNaoGravados ExtraiVbo(const ParametrosDesenho* pd, bool mundo) const { return ExtraiVbo(Proto(), vd_, pd, mundo); }
  // essa versao eh pra quem nao tem objeto mas tem o proto e quer criar vbos. m3d por exemplo.
  static gl::VbosNaoGravados ExtraiVbo(const EntidadeProto& proto, const ParametrosDesenho* pd, bool mundo);

  /** Move a entidade para o ponto especificado. Limpa destino. */
  void MovePara(float x, float y, float z = 0);
  void MovePara(const Posicao& pos);

  /** Move a entidade uma quantidade em cada eixo. Limpa destino. */
  void MoveDelta(float dx, float dy, float dz);

  /** @return o destino da entidade. */
  const Posicao& Destino() const { return proto_.destino(); }

  /** Atribui um destino a entidade. A cada atualizacao ela se movera em direcao ao destino. */
  void Destino(const Posicao& pos);

  /** Incrementa Z da entidade por um delta. */
  void IncrementaZ(float delta_z);

  /** Incrementa a rotacao em Z da entidade. */
  void IncrementaRotacaoZGraus(float delta_rotacao_graus);

  /** Altera a rotacao Z graus de uma entidade para o valor passado. */
  void AlteraRotacaoZGraus(float rotacao_graus);

  float RotacaoZGraus() const { return proto_.rotacao_z_graus(); }

  int PontosVida() const { return ent::PontosVida(proto_); }
  int MaximoPontosVida() const { return proto_.max_pontos_vida() - proto_.niveis_negativos() * 5 + proto_.pontos_vida_temporarios(); }
  int PontosVidaTemporarios() const { return proto_.pontos_vida_temporarios(); }
  int DanoNaoLetal() const { return proto_.dano_nao_letal(); }

  bool Morta() const { return proto_.morta(); }
  bool Inconsciente() const { return proto_.inconsciente(); }

  /** @return o total dos niveis das classes. */
  int NivelPersonagem() const;
  /** @return o nivel do personagem para a classe. */
  int NivelClasse(const std::string& id_classe) const;
  /** @return o nivel de conjurador do personagem. */
  int NivelConjurador(const std::string& classe) const;
  /** Bonus base de ataque total do personagem. */
  int BonusBaseAtaque() const;
  /** Modificador do atributo de conjuracao. Exemplo: sab para clérigos. */
  int ModificadorAtributoConjuracao() const;
  /** Retorna o modificador de um atributo do personagem, com todos modificadores. */
  int ModificadorAtributo(TipoAtributo atributo) const;
  /** Retorna true se a entidade tiver mais de duas pernas. */
  bool MaisDeDuasPernas() const { return proto_.mais_de_duas_pernas(); }

  /** Retorna o tipo de transicao do objeto. Considera codigo legado tambem. */
  EntidadeProto::TipoTransicao TipoTransicao() const;
  /** Retorna -2 caso nao haja. */
  int TransicaoCenario() const;
  /** Retorna a posicao de transicao do cenario. */
  const Posicao& PosTransicao() const;

  /** @return a coordenada (x). */
  float X() const;

  /** @return a coordenada (y). */
  float Y() const;

  /** @return a coordenada (z). Se delta voo for true, inclui o delta de voo tb. */
  float Z(bool delta_voo = false) const;
  float ZOlho() const;
  /** Diferentemente da altura, considera apenas a altura do olho, sem deslocamento da entidade. */
  float AlturaOlho() const;

  /** Retorna um valor de -1.0f ate 1.0f referente a espiada. */
  float Espiada() const { return vd_.progresso_espiada_; }

  float ZAntesVoo() const { return proto_.z_antes_voo(); }
  void AtribuiZAntesVoo(float z) { proto_.set_z_antes_voo(z); }

  /** @return o id de cenario da entidade. */
  int IdCenario() const;

  /** Retorna as coordenadas do objeto como posicao. */
  const Posicao& Pos() const { return proto_.pos(); }

  /** Retorna uma string de uma linha com o resumo dos eventos sobre a entidade. */
  std::string ResumoEventos() const;

  /** Mata a entidade, ligando os bits de queda, morte e desligando voo e destino. */
  void MataEntidade();

  /** Atualiza os pontos de vida da entidade para a quantidade passada, assim como dano nao letal. */
  void AtualizaPontosVida(int pontos_vida, int dano_nao_letal);

  /** Atualiza apenas os campos presentes no proto para a entidade. */
  void AtualizaParcial(const EntidadeProto& proto_parcial);

  /** Altera o indice de feitico da entidade para usado ou nao. */
  void AlteraFeitico(const std::string& id_classe, int nivel, int indice, bool usado);
  void AlteraTodosFeiticos(const EntidadeProto& proto_parcial);

  using MapaIdAcao = std::unordered_map<std::string, std::unique_ptr<AcaoProto>>;
  // Retorna true se entidade possui acao propria.
  bool AcaoAnterior();
  bool ProximaAcao();
  /** Atualiza a acao realizada pela entidade nos comandos de acao. */
  void AtualizaAcao(const std::string& id_acao);
  void AtualizaAcaoPorGrupo(const std::string& grupo);
  /** Retorna a acao mais recente da entidade. Caso nao haja, proto vazio. */
  AcaoProto Acao() const;

  /** Atualiza a acao da entidade para o indice passado. */
  void AdicionaAcaoExecutada(const std::string& id_acao);
  /** Retorna a acao executada pela entidade ou uma acao padrao caso a entidade nao possua a acao. */
  std::string TipoAcaoExecutada(int indice_acao, const std::vector<std::string>& acoes_padroes) const;

  /** @return a posicao das acoes da entidade, ja modificador pelas matrizes de transformacao. */
  const Posicao PosicaoAcao() const;
  const Posicao PosicaoAcaoSemTransformacoes() const;
  const Posicao PosicaoAcaoSecundariaSemTransformacoes() const;
  /** @return a posicao de algo a uma altura do personagem, dada por fator * ALTURA ja transformada pelas marizes de transformacao. */
  const Posicao PosicaoAltura(float fator) const;
  const Posicao PosicaoAlturaSemTransformacoes(float fator) const;

  /** As luzes devem ser desenhadas primeiro, portanto há uma função separada para elas. */
  void DesenhaLuz(ParametrosDesenho* pd);

  /** desenha o objeto de forma solida. Pode alterar os parametros de desenho. */
  void Desenha(ParametrosDesenho* pd);

  /** Desenha a unidade de forma translucida. */
  void DesenhaTranslucido(ParametrosDesenho* pd);

  /** Desenha aura da entidade. */
  void DesenhaAura(ParametrosDesenho* pd);

  /** Retorna o proto da entidade. */
  const EntidadeProto& Proto() const { return proto_; }

  /** Retorna se a entidade eh selecionavel para jogador. */
  bool SelecionavelParaJogador() const { return proto_.selecionavel_para_jogador(); }

  /** Retorna se a entidade eh fixa (ou seja, nem o mestre pode mover). */
  bool Fixa() const { return proto_.fixa(); }

  bool Apoiada() const { return proto_.apoiada(); }
  void Apoia(bool apoiada) { proto_.set_apoiada(apoiada); }

  /** @return true se a entidade tiver iniciativa. */
  bool TemIniciativa() const { return proto_.has_iniciativa(); }
  /** @return a iniciativa da entidade. */
  int Iniciativa() const { return proto_.iniciativa(); }
  int ModificadorIniciativa() const { return proto_.modificador_iniciativa(); }

  /** @return true se a entidade tiver o efeito. */
  bool PossuiEfeito(TipoEfeito id_efeito) const;
  bool PossuiUmDosEfeitos(const std::vector<TipoEfeito>& ids_efeitos) const;

  bool PossuiTalento(const std::string& talento, const std::optional<std::string>& complemento = std::nullopt) const;

  bool PossuiVisaoEscuro() const { return (proto_.tipo_visao() & VISAO_ESCURO) != 0; }
  bool PossuiVisaoBaixaLuminosidade() const { return (proto_.tipo_visao() & VISAO_BAIXA_LUMINOSIDADE) != 0; }

  // Acesso simplificado a alinhamento parcial.
  bool Boa() const;
  bool Ma() const;
  bool Caotica() const;
  bool Ordeira() const;

  // Retorna nullptr caso nao haja.
  const DadosAtaque* DadoCorrente(bool ignora_ataques_na_rodada = false) const;
  const DadosAtaque& DadoCorrenteNaoNull(bool ignora_ataques_na_rodada = false) const {
    auto* da = DadoCorrente(ignora_ataques_na_rodada);
    return da == nullptr ? DadosAtaque::default_instance() : *da;
  }
  // Retorna o dado corrente com a mao secundaria, se houver. Implica ataque de duas armas.
  const DadosAtaque* DadoCorrenteSecundario() const;
  const DadosAtaque* DadoAtaque(const std::string& grupo, int indice_ataque) const;
  const DadosAtaque* DadoAgarrar() const;
  const DadosAtaque& DadoAgarrarNaoNull() const {
    if (const auto* da = DadoAgarrar(); da != nullptr) { return *da; } else { return DadosAtaque::default_instance(); }
  }
  // Funcoes retornam AtaqueCaInvalido o se nao possuirem.
  int BonusAtaque() const;
  // Retorna modificadores para ataques de toque.
  int BonusAtaqueToque() const;
  int BonusAtaqueToqueDistancia() const;
  std::string TipoAtaque() const;
  int MargemCritico() const;
  int MultiplicadorCritico() const;
  // Retorna o alcance do ataque em m. Negativo se nao tiver.
  float AlcanceAtaqueMetros() const;
  float AlcanceMinimoAtaqueMetros() const;
  // Retorna quantos incrementos o ataque permite.
  int IncrementosAtaque() const;
  enum TipoCA {
    CA_NORMAL,
    CA_TOQUE,
    CA_SURPRESO  // Nao faz sentido, coisa do defensor.
  };
  // Retorna a CA da entidade, contra um atacante e um tipo de CA.
  int CA(const Entidade& atacante, TipoCA tipo, bool vs_oportunidade = false) const;
  // Retorna 10 + modificador tamanho + destreza.
  int CAReflexos() const;
  bool ImuneCritico() const;
  bool ImuneFurtivo(const Entidade& atacante) const;
  bool ImuneAcaoMental() const;
  bool ImuneEfeito(TipoEfeito efeito) const;
  void ProximoAtaque() { vd_.ataques_na_rodada++; vd_.ultimo_ataque_ms = 0; }
  void AtaqueAnterior() {
    vd_.ataques_na_rodada = std::max(0, vd_.ataques_na_rodada-1); vd_.ultimo_ataque_ms = 0;
  }
  int IndiceAtaque() const { return vd_.ataques_na_rodada; }
  // A chance de falha ao atacar.
  int ChanceFalhaAtaque() const;
  // A chance de um inimigo falhar um ataque contra esta entidade.
  int ChanceFalhaDefesa(const DadosAtaque& da = DadosAtaque::default_instance()) const;
  // Retorna se a entidade, ao atacar, ignora a chance de falha do oponente.
  bool IgnoraChanceFalha() const;

  /** Verifica se o ponto em pos, ao se mover na direcao, ira colidir com o objeto.
  * Caso haja colisao, retorna true e altera a direcao para o que sobrou apos a colisao.
  */
  bool Colisao(const Posicao& pos, Vector3* direcao) const { return Colisao(proto_, pos, direcao); }
  static bool Colisao(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao);
  static bool ColisaoComposta(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao);
  static bool ColisaoForma(const EntidadeProto& proto, const Posicao& pos, Vector3* direcao);

  /** Retorna o multiplicador de tamanho para a entidade de acordo com seu tamanho. Por exemplo, retorna
  * 1.0f para entidades medias. Multiplicado pelo tamanho do quadrado da o tamanho da entidade.
  */
  float MultiplicadorTamanho() const;
  TamanhoEntidade Tamanho() const { return proto_.tamanho(); }

  /** O espaco da entidade, baseado no seu tamanho. */
  float Espaco() const;

  /** Limpa a proxima salvacao para a entidade. */
  void AtualizaProximaSalvacao(ResultadoSalvacao rs) { proto_.set_proxima_salvacao(rs); }
  void LimpaProximaSalvacao() { proto_.clear_proxima_salvacao(); }

  /** Retorna a proxima salvacao para a entidade. */
  ResultadoSalvacao ProximaSalvacao() const { return static_cast<ResultadoSalvacao>(proto_.proxima_salvacao()); }
  bool TemProximaSalvacao() const { return proto_.has_proxima_salvacao(); }
  /** Retorna o bonus de salvacao de um tipo para entidade. */
  int Salvacao(const Entidade& atacante, TipoSalvacao tipo) const;
  int SalvacaoFeitico(const Entidade& atacante, const DadosAtaque& da) const;
  int SalvacaoSemAtacante(TipoSalvacao tipo) const;
  int SalvacaoVeneno() const;

  bool ImuneVeneno() const;
  bool ImuneDoenca() const;

  bool PodeMover() const;
  // Retorna se pode agir e caso nao possa, a razao.
  std::pair<bool, std::string> PodeAgir() const;

  /** Por padrao, apenas entidades podem ser afetadas por acao. */
  inline bool PodeSerAfetadoPorAcoes() const {
    return proto_.has_pode_ser_afetada_por_acao()
               ? proto_.pode_ser_afetada_por_acao()
               : Tipo() == TE_ENTIDADE;
  }

  /** Atribui a direcao de queda da entidade. */
  void AtualizaDirecaoDeQueda(float x, float y, float z);

  // Acesso a tendencia.
  bool Bom() const { return ::ent::Bom(proto_); }
  bool Mau() const { return ::ent::Mau(proto_); }
  bool Ordeiro() const { return ::ent::Ordeiro(proto_); }
  bool Caotico() const { return ::ent::Caotico(proto_); }

  /** Retorna o valor automatico de uma acao, se houver. Retorna zero se nao houver. A string eh a descricao. */
  std::pair<std::tuple<int, std::string>, std::optional<std::tuple<int, std::string>>>
      ValorParaAcao(const std::string& id_acao, const EntidadeProto& alvo) const;
  /** Retorna a string de dano para a acao corrente para o alvo: '1d8+3'. */
  std::pair<std::string, std::optional<std::string>> StringDanoParaAcao(const EntidadeProto& alvo) const;
  /** Retorna a string de CA para a acao corrente (normal, toque): '(esc+surp) 15, tq: 12. */
  std::string StringCAParaAcao() const;
  /** Retorna alguns detalhes da acao: rotulo, string dano. */
  std::string DetalhesAcao() const;

  /** Desenha um objeto a partir de seu proto. Usado para desenhar de forma simples objetos (por exemplo, formas sendo adicionadas).
  * Implementado em entidade_desenha.cpp.
  */
  static void DesenhaObjetoProto(
      const EntidadeProto& proto, ParametrosDesenho* pd);

  /** Carrega modelos usados pelas entidades. */
  static void IniciaGl(ntf::CentralNotificacoes* central);

  Matrix4 MontaMatrizModelagem(const ParametrosDesenho* pd = nullptr) const;

  // Reinicia os dados de ataque da entidade.
  void ReiniciaAtaque();

  /** Remove e retorna a sub forma da entidade. */
  EntidadeProto RemoveSubForma(int indice);

  /** Realiza as notificacoes referentes a modelos 3d. */
  void AtualizaModelo3d(const EntidadeProto& novo_proto);

  /** Realiza as chamadas de notificacao para as texturas. */
  void AtualizaTexturas(const EntidadeProto& novo_proto);

  /** Retorna true se a entidade segue o solo ao se mover. */
  bool RespeitaSolo() const;

  /** Atualiza a matriz de acao da mão principal. */
  void AtualizaMatrizAcaoPrincipal(const Matrix4& matriz);
  /** Atualiza a matriz de acao da mão secundaria. */
  void AtualizaMatrizAcaoSecundaria(const Matrix4& matriz);

  /** Atualizacao que sera executada na proxima chamada de atualizacao. */
  void DeixaAtualizacaoPendente(const EntidadeProto& atualizacao_pendente) { atualizacao_pendente_ = atualizacao_pendente; }

  // Id de entidade invalido.
  static constexpr unsigned int IdInvalido = 0xFFFFFFFF;
  // Valor de ataque ou ca invalido.
  static constexpr int AtaqueCaInvalido = -100;

 protected:
  friend Entidade* NovaEntidade(
      const EntidadeProto& proto, const Tabelas& tabelas, const Tabuleiro* tabuleiro, const Texturas*, const m3d::Modelos3d*,
      ntf::CentralNotificacoes*, const ParametrosDesenho* pd);
  Entidade(
      const Tabelas& tabelas, const Tabuleiro* tabuleiro, const Texturas* texturas, const m3d::Modelos3d* m3d,
      ntf::CentralNotificacoes* central, const ParametrosDesenho* pd);

 private:
  // Numero maximo de acoes de uma entidade.
  static constexpr unsigned int MaxNumAcoes = 3;

  // Nome dos buffers de VBO.
  constexpr static unsigned short NUM_VBOS = 25;
  constexpr static unsigned short
      VBO_PEAO = 0, VBO_TIJOLO = 1, VBO_TELA_TEXTURA = 2, VBO_CUBO = 3,
      VBO_ESFERA = 4, VBO_PIRAMIDE = 5, VBO_CILINDRO = 6, VBO_DISCO = 7,
      VBO_RETANGULO = 8, VBO_TRIANGULO = 9, VBO_CONE = 10, VBO_CONE_FECHADO = 11,
      VBO_CILINDRO_FECHADO = 12, VBO_BASE_PECA = 13, VBO_MOLDURA_PECA = 14,
      VBO_HEMISFERIO = 15, VBO_PIRAMIDE_FECHADA = 16;
  static std::vector<gl::VboGravado> g_vbos;

  // Alguns efeitos tem complementos.
  struct ComplementoEfeito {
    int quantidade = 0;  // quantidade de imagens, por exemplo.
    std::vector<float> posicoes;
  };

  // Dados de uma emissao, pode ser nuvem, bolha etc.
  struct DadosUmaEmissao {
    // Vetor de direcao da fumaca. Unitario.
    Vector3 direcao;
    Vector3 pos;
    float escala = 1.0f;
    int duracao_ms = 0;
    float velocidade_m_s = 0.0f;
    float incremento_escala_s = 0.0f;
    float cor[4] = {1.0f, 1.0f, 1.0f, 1.0};  // Cor de uma emissao.
  };

  // Os dados da emissao toda.
  struct DadosEmissao {
    // Ao chegar a zero, para de emitir.
    int duracao_ms = 0;
    // Intervalo entre emissoes.
    int intervalo_emissao_ms = 0;
    // Ao chegar a zero, realizara nova emissao.
    int proxima_emissao_ms = 0;
    // Quanto tempo vive uma nuvem.
    int duracao_nuvem_ms = 0;
    // Dados de cada nuvem.
    std::vector<DadosUmaEmissao> emissoes;
    // O vbo da fumaca.
    gl::VbosNaoGravados vbo;
    // Cor base da emissao.
    float cor[3] = {0};
  };

  // Para luzes temporarias, como disparo de arma de fogo.
  struct DadosLuzAcao {
    int tempo_desde_inicio_ms = 0;
    int duracao_ms = 0;
    // Ativa se inicio.raio_m existir.
    IluminacaoPontual inicio;
    IluminacaoPontual corrente;  // funcao de luz inicio e duracao.
  };

  // Variaveis locais nao sao compartilhadas pela rede, pois sao computadas a partir de outras.
  struct VariaveisDerivadas {
    VariaveisDerivadas() { }
    // Como esse estado é local e não precisa ser salvo, fica aqui.
    float angulo_disco_selecao_graus = 0.0f;
    // Para entidades com texturas sempre de frente, o angulo para rodar a modulra da textura.
    float angulo_rotacao_textura_graus = 0.0f;
    // Qual a altura do voo da entidade.
    float altura_voo = 0.0f;
    // Entidades em voo oscilam sobre a altura do voo. A oscilacao eh baseada no seno deste angulo.
    float angulo_disco_voo_rad = 0.0f;
    // Entidades em queda caem progressivamente ate 90 graus.
    float angulo_disco_queda_graus = 0.0f;
    // Oscilacao da luz.
    float angulo_disco_luz_rad = 0.0f;
    // Usado para escala da seta da iniciativa.
    float angulo_disco_iniciativa_rad = 0.0f;
    // Usado para inclinar o personagem, de -1.0f a 1.0f.
    float progresso_espiada_ = 0.0f;
    // Para texturas que se movem.
    float deslocamento_textura = 0.0f;
    // Efeitos da criatura e algum complemento.
    std::unordered_map<int, ComplementoEfeito> complementos_efeitos;
    // Alguns efeitos podem fazer com que o desenho nao seja feito (piscar por exemplo).
    bool nao_desenhar = false;
    // Numero de ataques realizado na rodada.
    int ataques_na_rodada = 0;
    unsigned int ultimo_ataque_ms = 0;
    DadosEmissao fumaca;
    DadosEmissao bolhas;
    DadosLuzAcao luz_acao;

    // Alguns tipos de entidade possuem VBOs. (no caso de VBO_COM_MODELAGEM, todas).
    gl::VbosNaoGravados vbos_nao_gravados;  // se vazio, ainda nao foi carregado.
    gl::VbosGravados vbos_gravados;
    Matrix4 matriz_modelagem;
    // Essa matriz é aplicada à acao sendo realizada.
    Matrix4 matriz_acao_principal;
    Matrix4 matriz_acao_secundaria;
    // Para entidades com textura.
    Matrix4 matriz_modelagem_tijolo_base;
    Matrix4 matriz_modelagem_tijolo_tela;
    Matrix4 matriz_modelagem_tela_textura;
    Matrix4 matriz_deslocamento_textura;

    // Abaixo sao variaveis que valem para todos (como se fossem globais).
    // As texturas.
    const Texturas* texturas = nullptr;
    // Modelo 3d.
    const m3d::Modelos3d* m3d = nullptr;
  };

  // Apos o intervalo de emissao, emite nova bolha ou nuvem.
  void EmiteNovaBolha();
  void EmiteNovaNuvem();
  /** Atualiza os dados da emissao, baseado no intervalo. Remove as emissoes mortas, atualiza as existentes. */
  void RemoveAtualizaEmissoes(unsigned int intervalo_ms, DadosEmissao* dados_emissao) const;
  /** Recria os VBOs da emissao. */
  void RecriaVboEmissoes(const std::function<const gl::VboNaoGravado()> gera_vbo_f, DadosEmissao* dados_emissao) const;

  // Correcao de VBO: corrige o VBO da entidade raiz. As transformadas do objeto raiz devem ser desfeitas
  // apos a extracao, pois elas serao reaplicadas durante o desenho da entidade.
  static void CorrigeVboRaiz(const EntidadeProto& proto, VariaveisDerivadas* vd);

  /** Retorna um VBO que representa a entidade (valido para FORMAS e COMPOSTAS). */
  static gl::VbosNaoGravados ExtraiVbo(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  // Extracao de VBO por tipo.
  static gl::VbosNaoGravados ExtraiVboEntidade(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  static gl::VbosNaoGravados ExtraiVboForma(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  static gl::VbosNaoGravados ExtraiVboComposta(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);

  // Inicializacao por tipo.
  static void InicializaForma(const EntidadeProto& proto, VariaveisDerivadas* vd);
  static void InicializaComposta(const EntidadeProto& proto, VariaveisDerivadas* vd);

  // Atualizacao por tipo.
  static void AtualizaProtoForma(
      const EntidadeProto& proto_original, const EntidadeProto& proto_novo, VariaveisDerivadas* vd);
  static void AtualizaProtoComposta(
      const EntidadeProto& proto_original, const EntidadeProto& proto_novo, VariaveisDerivadas* vd);

  /** Atualiza os efeitos para o frame. */
  void AtualizaEfeitos();
  void AtualizaEfeito(TipoEfeito id_efeito, ComplementoEfeito* complemento);
  /** Atualiza a fumaca da entidade. Parametro intervalo_ms representa o tempo passado desde a ultima atualizacao. */
  void AtualizaFumaca(int intervalo_ms);
  /** Atualiza as bolhas da entidade. Parametro intervalo_ms representa o tempo passado desde a ultima atualizacao. */
  void AtualizaBolhas(int intervalo_ms);

  /** Atualiza a iluminacao por acao. Parametro intervalo_ms representa o tempo passado desde a ultima atualizacao. */
  void AtualizaLuzAcao(int intervalo_ms);

  static void AtualizaTexturasProto(const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central);

  /** Atualiza o VBO da entidade. Deve ser chamado sempre que houver algo que mude a posicao, orientacao ou forma do objeto.
  * Teoricamente deveria sempre receber pd, mas se for nullptr vai usar valor padrao (o que implica em olho em 0,0).
  */
  void AtualizaVbo(const ParametrosDesenho* pd);

  /** A oscilacao de voo nao eh um movimento real (nao gera notificacoes). Esta funcao retorna o delta. */
  static float DeltaVoo(const VariaveisDerivadas& vd);

  /** Realiza o desenho do objeto com as decoracoes, como disco de selecao e barra de vida (de acordo com pd). */
  void DesenhaObjetoComDecoracoes(ParametrosDesenho* pd);

  /** Realiza o desenho do objeto. */
  void DesenhaObjeto(ParametrosDesenho* pd);

  /** Desenha as decoracoes do objeto (pontos de vida, disco de selecao). */
  void DesenhaDecoracoes(ParametrosDesenho* pd);
  void DesenhaArmas(ParametrosDesenho* pd);

  /** Desenha os efeitos do objeto. Sera chamado uma vez para solido e outra para translucido. Cada efeito devera
  * saber o que e quando desenhar (usando pd->alfa_translucidos para diferenciar).
  */
  void DesenhaEfeitos(ParametrosDesenho* pd);
  /** Desenha o efeito de uma entidade. */
  void DesenhaEfeito(ParametrosDesenho* pd, const EntidadeProto::Evento& evento, const ComplementoEfeito& complemento);

  void RecomputaDependencias();

  struct MatrizesDesenho {
    Matrix4 modelagem;
    Matrix4 tijolo_base;
    Matrix4 tijolo_tela;
    Matrix4 tela_textura;
    Matrix4 deslocamento_textura;
  };
  static MatrizesDesenho GeraMatrizesDesenho(const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd);

  /** Atualiza as matrizes do objeto. */
  void AtualizaMatrizes();

  /** @return true se a entidade deve ser achatada. */
  static bool Achatar(const EntidadeProto& proto, const ParametrosDesenho* pd) {
    return !proto.has_modelo_3d() && !proto.info_textura().id().empty() && (pd->desenha_texturas_para_cima() || proto.achatado()) && !proto.caida();
  }

  /** Retorna se a peca de base deve ser desenhada. */
  static bool DesenhaBase(const EntidadeProto& proto);

  /** Tipo de translacao em Z desejado ao montar a matriz. */
  enum translacao_z_e {
    TZ_NENHUMA,  // sem translacao em Z.
    TZ_SEM_VOO,  // translada em Z sem voo.
    TZ_COMPLETA  // translada em Z incluindo voo.
  };
  /** Auxiliar para montar a matriz de desenho do objeto.
  * @param queda se verdeiro, roda o eixo para desenhar a entidade caida.
  * @param translacao_z se verdadeiro, transladar para posicao vertical do objeto.
  * @param proto da entidade.
  * @param pd os parametros de desenho.
  */
  static void MontaMatriz(bool queda,
                          bool transladar_z,
                          const EntidadeProto& proto,
                          const VariaveisDerivadas& vd,
                          const ParametrosDesenho* pd = nullptr,
                          bool posicao_mundo = true);

  static Matrix4 MontaMatrizModelagem(
      bool queda, translacao_z_e tz, const EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd = nullptr,
      bool posicao_mundo = true);
  static Matrix4 MontaMatrizModelagem(
      bool queda, bool transladar_z, const EntidadeProto& proto, const VariaveisDerivadas& vd,
      const ParametrosDesenho* pd = nullptr, bool posicao_mundo = true) {
    return MontaMatrizModelagem(queda, transladar_z ? TZ_COMPLETA : TZ_NENHUMA, proto, vd, pd, posicao_mundo);
  }
  static Matrix4 MontaMatrizModelagemForma(
      bool queda, bool transladar_z, const EntidadeProto& proto, const VariaveisDerivadas& vd,
      const ParametrosDesenho* pd = nullptr, bool posicao_mundo = true);
  static Matrix4 MontaMatrizModelagemComposta(
      bool queda, bool transladar_z, const EntidadeProto& proto, const VariaveisDerivadas& vd,
      const ParametrosDesenho* pd = nullptr, bool posicao_mundo = true);

  static void DesenhaObjetoProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd);
  static void DesenhaObjetoFormaProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd);
  static void DesenhaObjetoEntidadeProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd);
  static void DesenhaObjetoEntidadeProtoComMatrizes(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd,
      const Matrix4& modelagem, const Matrix4& tijolo_base, const Matrix4& tijolo_tela, const Matrix4& tela_textura, const Matrix4& deslocamento_textura);
  static void DesenhaObjetoCompostoProto(
      const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd);

  /** Calcula o multiplicador para um determinado tamanho. */
  static float CalculaMultiplicador(TamanhoEntidade tamanho);

 private:
  EntidadeProto proto_;
  std::optional<EntidadeProto> atualizacao_pendente_;  // para efeitos que alternam a forma.
  const Tabelas& tabelas_;
  const Tabuleiro* tabuleiro_ = nullptr;
  VariaveisDerivadas vd_;
  const ParametrosDesenho* parametros_desenho_ = nullptr;  // nao eh dono.

  // A central é usada apenas para enviar notificacoes de textura ja que as entidades nao sao receptoras.
  ntf::CentralNotificacoes* central_ = nullptr;
};

}  // namespace ent

#endif
