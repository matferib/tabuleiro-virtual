#include <boost/asio.hpp>
#include <memory>

namespace net {

class Servidor {
 public:
   explicit Servidor(int porta);

   void Liga();
   void Desliga();

 private:
   struct Dados;
   std::unique_ptr<Dados> dados_;
};

}  // namespace net
