#ifndef ENT_ENTIDADE_DESENHO_H
#define ENT_ENTIDADE_DESENHO_H

namespace ent {

class EntidadeProto;
class ParametrosDesenho;

DesenhoBase* NovoDesenhoBase(const EntidadeProto& proto);

class DesenhoBase {
 public:
  virtual void Desenha(ParametrosDesenho* pd) = 0;
}; 

}  // namespace ent

#endif
