/** Macros de log. */

#if USAR_GLOG
#include <gflags/gflags.h>
#include <glog/logging.h>

#elif __ANDROID__

// TODO converter para android log.
//#include <android/log.h>
#include <iostream>
#define LOG(X) if (false) std::cout
#define VLOG(X) if (false) std::cout
// __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");

#else
// TODO dar um jeito de anular essas macros se nao tiver log.
#include <iosfwd>
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
