#ifndef ENT_TABULEIRO_H
#define ENT_TABULEIRO_H

#include <map>
#include <vector>
#include "ent/parametrosdesenho.h"

namespace ntf {
	class Notificacao;
}

namespace ent {

class Entidade;
class DadosTabuleiro;
class DadosCriacao;

/** tipos de entidade que podem ser criadas pelo tabuleiro. */
enum tipoent_e {
	TIPOENT_MOVEL
};

/** botoes do mouse. */
enum botao_e {
	BOTAO_NENHUM,
	BOTAO_ESQUERDO,
	BOTAO_DIREITO,
	BOTAO_MEIO
};

enum etab_t {
	ETAB_NORMAL,
	ETAB_ADICIONANDO_ENTIDADE,
	ETAB_REMOVENDO_ENTIDADE,
	ETAB_ROTACAO,
	ETAB_ANIMACAO
};

typedef std::map<int, Entidade*> MapaEntidades;
typedef std::vector<Entidade*> VetorEntidades;

/** singleton responsavel por tudo que eh do jogo. */
class Tabuleiro {
private:
	/** cria o tabuleiro com o tamanho de grid passado (tamanho x tamanho). 
	* O identificador do tabuleiro sempre eh 0. 
	*/
	explicit Tabuleiro(int tamanho);
public:
	/** @return true se a instancia nao for NULL. */
	static bool haInstancia();

	/** @return a instancia unica do tabuleiro. */
	static Tabuleiro& instancia();

	/** destroi a instancia (se houver) e cria uma nova. */
	static void cria(int tamanho);

	/** libera os recursos do tabuleiro, inclusive entidades. */
	~Tabuleiro();

	/** @return tamanho do tabuleiro em X. */
	int tamanhoX() const;

	/** @return tamanho do tabuleiro em Y. */
	int tamanhoY() const;

	/** adiciona a entidade ao tabuleiro, no quadrado passado.
	* @param tipoEntidade tipo da entidade.
	* @param dc dados de criacao da entidade.
	* @param idQuadrado do quadrado no desenho.
	*/
	void adicionaEntidade(tipoent_e tipoEntidade, DadosCriacao* dc, int idQuadrado);

	/** remove entidade do tabuleiro, pelo id da entidade passado. 
	* @param id da entidade.
	*/
	void removeEntidade(int id);

	/** desenha o mundo. */
	void desenha();

	/** trata a notificacao de inicio de jogo. */
	void trataNotificacao(const ntf::Notificacao& notificacao);

	/** trata evento de rodela de mouse. */
	void trataRodela(int delta);

	/** trata movimento do mouse (y ja em coordenadas opengl). */
	void trataMovimento(int x, int y);

	/** trata o botao do mouse liberado. */
	void trataBotaoLiberado();

	/** trata o botao pressionado, recebendo x, y (ja em coordenadas opengl) e a razao de aspecto da janela. */
	void trataBotaoPressionado(botao_e botao, int x, int y, double aspecto);

	/** trata a redimensao da janela. */
	void trataRedimensionaJanela(int largura, int altura);

	/** inicializa os parametros do openGL. */
	static void inicializaGL();

private:
	/** funcao que desenha a cena independente do modo. */
	void desenhaCena();

	/** trata o clique, recebendo o numero de hits e o buffer de hits do opengl. */
	void trataClique(unsigned int numeroHits, unsigned int* bufferHits);

	/** seleciona a entidade pelo ID. */ 
	void selecionaEntidade(int id);

	/** seleciona o quadrado pelo ID. */
	void selecionaQuadrado(int idQuadrado);

	/** retorna as coordenadas do quadrado. */
	void coordenadaQuadrado(double& x, double& y, double& z, int idQuadrado);

private:
	/** tamanho do tabuleiro (tamanho_ x tamanho_). */
	int tamanho_;

	/** mapa geral de entidades, por id. */
	MapaEntidades entidades_;

	/** a entidade selecionada. */
	Entidade* entidadeSelecionada_;

	/** quadrado selecionado (pelo id de desenho). */
	int quadradoSelecionado_;

	/** estado do tabuleiro. */
	etab_t estado_;

	/** proximo id de entidades. */
	unsigned int proximoId_;

	/** parametros de desenho da cena. */
	ent::ParametrosDesenho parametrosDesenho_;

	/** dados (X) para calculo de rotacao de mouse. */
	int rotacaoUltimoX_; 
	/** dados (Y) para calculo de rotacao de mouse. */
	int rotacaoUltimoY_; 

	// para onde o olho olha
	double olhoX_;
	double olhoY_;
	double olhoZ_;
	// de onde o olho olha
	double olhoDeltaRotacao_;
	double olhoAltura_;
	double olhoRaio_;

	// elimina copia
	Tabuleiro(const Tabuleiro& t);
	Tabuleiro& operator=(Tabuleiro&);
};

}

#endif
