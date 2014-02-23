#ifndef ENT_FORMAS_H
#define ENT_FORMAS_H

#include <vector>
#include "ent/entidade.pb.h"

namespace ent {

/** Representa uma forma de desenho. */
class Forma {
 public:
  explicit Forma(const EntidadeProto& proto);
  ~Forma();
  void Desenha(const ParametrosDesenho& pd);
  void DesenhaTranslucido(const ParametrosDesenho& pd);
  unsigned int Id() { return proto_.id(); }

 private:
  EntidadeProto proto_;
};

}  // namespace ent

#endif
