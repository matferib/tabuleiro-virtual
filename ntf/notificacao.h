#ifndef NOTIFICACAO_H
#define NOTIFICACAO_H

/** @file include/ifg/ntf/Notificacao.h declaracao da interface de notificacao. */

#include <iosfwd>

namespace ntf {
	
/** enumeracao com todos os tipos de notificacoes. */
enum tipontf_e { 
	TN_SAIR, 
	TN_INICIAR, 
	TN_ADICIONAR_JOGADOR, 
	TN_ADICIONAR_ENTIDADE, 
	TN_REMOVER_ENTIDADE, 
	TN_ENTIDADE_ADICIONADA,
	TN_ENTIDADE_REMOVIDA,
	TN_NUM 
};

/** classe base das notificacoes, com funcoes para serializacao e deserializacao. */
class Notificacao {
public:
	/** cria uma notificacao de um determinado tipo. */
	explicit Notificacao(tipontf_e tipo);

	/** destrutor virtual de classe base. */
	virtual ~Notificacao();

	/** retorna o tipo de notificacao. */
	tipontf_e tipo() const { return _tipo; }

	/** serializa a notificacao para o stream de saida. */
	virtual void serializa(std::ostream& stream) const;

	/** deserializa a notificacao, ja tendo lido o cabecalho.
	* Utilizado pela factory.
	*/
	virtual void deserializa(std::istream& stream);

private:
	/** tipo da notificacao. */
	tipontf_e _tipo;
};

/** uma notificacao que pode ser feita, refeita e desfeita. */
class NotificacaoAcao : public Notificacao {
	explicit NotificacaoAcao(tipontf_e tipo);
	virtual ~NotificacaoAcao();

	/** executa a notificacao. */
	virtual void faz() = 0;
	/** reexecuta a notificacao. */
	virtual void refaz() = 0;
	/** desfaz a notificacao. */
	virtual void desfaz() = 0;
};


} // namespace ntf


#endif
