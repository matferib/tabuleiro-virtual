/** Macros de log. */

#if USAR_GLOG
#include <gflags/gflags.h>
#include <glog/logging.h>

#elif ANDROID

#include <android/log.h>
#include <cstring>
#include <string>
#include <sstream>

class StringLogger {
 public:
  StringLogger(const char* file, int line) {
    oss_ << file << ", " << line << ": ";
  }

  ~StringLogger() {
    __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "%s", oss_.str().c_str());
  }

  template <class T>
  inline StringLogger& operator<<(const T& msg) {
    oss_ << msg;
    return *this;
  }

 private:
  std::ostringstream oss_;
};

#define SHORT_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(X) if (true) StringLogger(SHORT_FILE, __LINE__)
#define VLOG(X) if (true && X <= 1) StringLogger(SHORT_FILE, __LINE__)
// __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "nativePause");

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
