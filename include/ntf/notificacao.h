#ifndef NOTIFICACAO_H
#define NOTIFICACAO_H

/** @file include/ifg/ntf/Notificacao.h declaracao da interface de notificacao. */

#include <iosfwd>

namespace ntf {
	
	/** enumeracao com todos os tipos de notificacoes. */
	enum tipontf_e { TN_SAIR, TN_NUM };

	/** classe base das notificacoes, com funcoes para serializacao e deserializacao. */
	class Notificacao {
	public:
		/** cria uma notificacao de um determinado tipo. */
		explicit Notificacao(tipontf_e tipo);

		/** factory de Notificacao: deserializa a partir de stream. */
		//static Notificacao* nova(std::istream& stream);

		/** retorna o tipo de notificacao. */
		tipontf_e tipo() const { return _tipo; }

		/** destrutor virtual de classe base. */
		virtual ~Notificacao();
	
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


} // namespace ntf


#endif
