/** Macros de log. */

#if USAR_GLOG
#include <glog/logging.h>
#else
// TODO dar um jeito de anular essas macros se nao tiver log.
#include <iostream>
#define LOG(X) std::cout
#define VLOG(X) std::cout
#endif

namespace meulog {

inline void Inicializa(const char* argv) {
#if USAR_GLOG
  google::InitGoogleLogging(argv);
#endif
}

}  // namespace meulog
