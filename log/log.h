/** Macros de log. */

#if USAR_GLOG
#include <gflags/gflags.h>
#include <glog/logging.h>
#else
// TODO dar um jeito de anular essas macros se nao tiver log.
#include <iostream>
#define LOG(X) if (false) std::cout
#define VLOG(X) if (false) std::cout
#endif

namespace meulog {

inline void Inicializa(int* argc, char*** argv) {
#if USAR_GLOG
  google::ParseCommandLineFlags(argc, argv, true);
  google::InitGoogleLogging((*argv)[0]);
#endif
}

}  // namespace meulog
