#ifndef M3D_M3D_H
#define M3D_M3D_H

#include <memory>

namespace gl {
class VboNaoGravado;
}  // namespace gltab

namespace m3d {

class Modelos3d {
 public:
  Modelos3d();
  ~Modelos3d();

  const gl::VboNaoGravado* Modelo(const std::string& id) const;

 private:
  struct Interno;
  std::unique_ptr<Interno> interno_;
};

}  // namespace m3d

#endif
