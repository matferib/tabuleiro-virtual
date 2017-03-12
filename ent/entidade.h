#ifndef ENT_ENTIDADE_H
#define ENT_ENTIDADE_H

#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "ent/entidade.pb.h"
#include "ent/util.h"
#include "gltab/gl_vbo.h"
#include "m3d/m3d.h"

#define VBO_COM_MODELAGEM 0

namespace ntf {
class CentralNotificacoes;
}  // namespace ntf
namespace ent {

class Entidade;
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
Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd);

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

  /** Destroi a entidade. */
  ~Entidade();

  /** @return o identificador da entidade que deve ser unico globalmente. */
  unsigned int Id() const { return proto_.id(); }

  /** Retorna a cor da entidade. */
  const Cor& CorDesenho() const { return proto_.cor(); }

  /** Retorna o tipo das entidade: TE_ENTIDADE, TE_FORMA, TE_COMPOSTA. */
  TipoEntidade Tipo() const { return proto_.tipo(); }

  bool Achatar() const {
    return Achatar(proto_, parametros_desenho_);
  }

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
  static gl::VbosNaoGravados ExtraiVbo(const ent::EntidadeProto& proto, const ParametrosDesenho* pd, bool mundo);

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

  /** @return o HP da unidade. */
  int PontosVida() const;
  int MaximoPontosVida() const;

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

  /** Adiciona um evento para acontecer em rodadas. */
  void AdicionaEvento(int rodadas, const std::string& descricao) { proto_.add_evento()->set_rodadas(rodadas); }

  /** Mata a entidade, ligando os bits de queda, morte e desligando voo e destino. */
  void MataEntidade();

  /** Atualiza os pontos de vida da entidade para a quantidade passada. */
  void AtualizaPontosVida(int pontos_vida);

  /** Atualiza apenas os campos presentes no proto para a entidade. */
  void AtualizaParcial(const EntidadeProto& proto_parcial);

  // Retorna true se entidade possui acao propria.
  bool AcaoAnterior();
  bool ProximaAcao();
  /** Atualiza a acao realizada pela entidade nos comandos de acao. */
  void AtualizaAcao(const std::string& id_acao);
  /** Retorna a acao mais recente da entidade. Caso nao haja, retorna a primeira acao padrao. */
  std::string Acao(const std::vector<std::string>& acoes_padroes) const;

  /** Atualiza a acao da entidade para o indice passado. */
  void AdicionaAcaoExecutada(const std::string& id_acao);
  /** Retorna a acao executada pela entidade ou uma acao padrao caso a entidade nao possua a acao. */
  std::string AcaoExecutada(int indice_acao, const std::vector<std::string>& acoes_padroes) const;

  /** @return a posicao das acoes da entidade. */
  const Posicao PosicaoAcao() const;

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

  const EntidadeProto::DadosAtaque* DadoCorrente() const;
  // Funcoes retornam AtaqueCaInvalido o se nao possuirem.
  int BonusAtaque() const;
  std::string TipoAtaque() const;
  int MargemCritico() const;
  int MultiplicadorCritico() const;
  // Retorna o alcance do ataque em m. Negativo se nao tiver.
  float AlcanceAtaqueMetros() const;
  // Retorna quantos incrementos o ataque permite.
  int IncrementosAtaque() const;
  enum TipoCA {
    CA_NORMAL,
    CA_TOQUE,
    CA_SURPRESO
  };
  int CA(TipoCA tipo = CA_NORMAL) const;
  bool ImuneCritico() const;
  void ProximoAtaque() { vd_.ataques_na_rodada++; vd_.ultimo_ataque_ms = 0; }

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

  /** O espaco da entidade, baseado no seu tamanho. */
  float Espaco() const;

  /** Limpa a proxima salvacao para a entidade. */
  void AtualizaProximaSalvacao(ResultadoSalvacao rs);

  /** Retorna a proxima salvacao para a entidade. */
  ResultadoSalvacao ProximaSalvacao() const { return static_cast<ResultadoSalvacao>(proto_.proxima_salvacao()); }

  /** Atribui a direcao de queda da entidade. */
  void AtualizaDirecaoDeQueda(float x, float y, float z);

  /** Retorna o valor automatico de uma acao, se houver. Retorna zero se nao houver. */
  int ValorParaAcao(const std::string& id_acao) const;
  /** Retorna a string de dano para a acao corrente: '1d8+3'. */
  std::string StringDanoParaAcao() const;
  /** Retorna alguns detalhes da acao: rotulo, string dano. */
  std::string DetalhesAcao() const;

  /** Desenha um objeto a partir de seu proto. Usado para desenhar de forma simples objetos (por exemplo, formas sendo adicionadas).
  * Implementado em entidade_desenha.cpp.
  */
  static void DesenhaObjetoProto(
      const EntidadeProto& proto, ParametrosDesenho* pd);

  /** Carrega modelos usados pelas entidades. */
  static void IniciaGl();

  Matrix4 MontaMatrizModelagem(const ParametrosDesenho* pd = nullptr) const;


  // Id de entidade invalido.
  static constexpr unsigned int IdInvalido = 0xFFFFFFFF;
  // Valor de ataque ou ca invalido.
  static constexpr int AtaqueCaInvalido = -100;

 protected:
  friend Entidade* NovaEntidade(const EntidadeProto& proto, const Texturas*, const m3d::Modelos3d*, ntf::CentralNotificacoes*, const ParametrosDesenho* pd);
  Entidade(const Texturas* texturas, const m3d::Modelos3d* m3d, ntf::CentralNotificacoes* central, const ParametrosDesenho* pd);

 private:
  // Numero maximo de acoes de uma entidade.
  static constexpr unsigned int MaxNumAcoes = 3;

  // Nome dos buffers de VBO.
  constexpr static unsigned short NUM_VBOS = 15;
  constexpr static unsigned short VBO_PEAO = 0, VBO_TIJOLO = 1, VBO_TELA_TEXTURA = 2, VBO_CUBO = 3, VBO_ESFERA = 4, VBO_PIRAMIDE = 5, VBO_CILINDRO = 6, VBO_DISCO = 7, VBO_RETANGULO = 8, VBO_TRIANGULO = 9, VBO_CONE = 10, VBO_CONE_FECHADO = 11, VBO_CILINDRO_FECHADO = 12, VBO_BASE_PECA = 13, VBO_MOLDURA_PECA = 14;
  static std::vector<gl::VboGravado> g_vbos;

  // Alguns efeitos tem complementos.
  struct ComplementoEfeito {
    int quantidade = 0;  // quantidade de imagens, por exemplo.
    std::vector<float> posicoes;
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
    // Efeitos da criatura e algum complemento.
    std::unordered_map<int, ComplementoEfeito> complementos_efeitos;
    // Alguns efeitos podem fazer com que o desenho nao seja feito (piscar por exemplo).
    bool nao_desenhar = false;
    // Numero de ataques realizado na rodada.
    int ataques_na_rodada = 0;
    unsigned int ultimo_ataque_ms = 0;

    // Alguns tipos de entidade possuem VBOs. (no caso de VBO_COM_MODELAGEM, todas).
    gl::VbosNaoGravados vbos_nao_gravados;  // se vazio, ainda nao foi carregado.
    gl::VbosGravados vbos_gravados;
    Matrix4 matriz_modelagem;
    // Para entidades com textura.
    Matrix4 matriz_modelagem_tijolo_base;
    Matrix4 matriz_modelagem_tijolo_tela;
    Matrix4 matriz_modelagem_tela_textura;
    Matrix4 matriz_deslocamento_textura;

    // As texturas da entidade.
    const Texturas* texturas = nullptr;
    // Modelo 3d para entidades que o possuem.
    const m3d::Modelos3d* m3d = nullptr;
  };

  // Correcao de VBO: corrige o VBO da entidade raiz. As transformadas do objeto raiz devem ser desfeitas
  // apos a extracao, pois elas serao reaplicadas durante o desenho da entidade.
  static void CorrigeVboRaiz(const ent::EntidadeProto& proto, VariaveisDerivadas* vd);

  /** Retorna um VBO que representa a entidade (valido para FORMAS e COMPOSTAS). */
  static gl::VbosNaoGravados ExtraiVbo(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  // Extracao de VBO por tipo.
  static gl::VbosNaoGravados ExtraiVboEntidade(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  static gl::VbosNaoGravados ExtraiVboForma(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);
  static gl::VbosNaoGravados ExtraiVboComposta(const ent::EntidadeProto& proto, const VariaveisDerivadas& vd, const ParametrosDesenho* pd, bool mundo);

  // Inicializacao por tipo.
  static void InicializaForma(const ent::EntidadeProto& proto, VariaveisDerivadas* vd);
  static void InicializaComposta(const ent::EntidadeProto& proto, VariaveisDerivadas* vd);

  // Atualizacao por tipo.
  static void AtualizaProtoForma(
      const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd);
  static void AtualizaProtoComposta(
      const ent::EntidadeProto& proto_original, const ent::EntidadeProto& proto_novo, VariaveisDerivadas* vd);

  /** Atualiza os efeitos para o frame. */
  void AtualizaEfeitos();
  void AtualizaEfeito(efeitos_e id_efeito, ComplementoEfeito* complemento);

  /** Realiza as chamadas de notificacao para as texturas. */
  void AtualizaTexturas(const EntidadeProto& novo_proto);
  static void AtualizaTexturasProto(const EntidadeProto& novo_proto, EntidadeProto* proto_atual, ntf::CentralNotificacoes* central);

  /** Realiza as notificacoes referentes a modelos 3d. */
  void AtualizaModelo3d(const EntidadeProto& novo_proto);

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

  /** Desenha os efeitos do objeto. Sera chamado uma vez para solido e outra para translucido. Cada efeito devera
  * saber o que e quando desenhar (usando pd->alfa_translucidos para diferenciar).
  */
  void DesenhaEfeitos(ParametrosDesenho* pd);
  /** Desenha o efeito de uma entidade. */
  void DesenhaEfeito(ParametrosDesenho* pd, const EntidadeProto::Evento& evento, const ComplementoEfeito& complemento);

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
  VariaveisDerivadas vd_;
  const ParametrosDesenho* parametros_desenho_ = nullptr;  // nao eh dono.

  // A central é usada apenas para enviar notificacoes de textura ja que as entidades nao sao receptoras.
  ntf::CentralNotificacoes* central_;
};

}  // namespace ent

#endif
