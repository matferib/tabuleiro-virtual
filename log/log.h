/** Macros de log. */
#ifndef LOG_LOG_H
#define LOG_LOG_H

#if USAR_GLOG

#include "absl/flags/parse.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#elif ANDROID

#include <android/log.h>
#include <cstring>
#include <string>
#include <sstream>

#ifndef INFO
#define INFO 1
#define WARNING 2
#define ERROR 3
#else
#error info defined
#endif

class StringLogger {
 public:
  StringLogger(const char* file, int line) {
    oss_ << file << ", " << line << ": ";
  }

  ~StringLogger() {
    __android_log_print(ANDROID_LOG_INFO, "TabuleiroJni", "%s", oss_.str().c_str());
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
// Arquivos querendo VLOG devem definir esse VLOG_NIVEL no android.
#ifndef VLOG_NIVEL
#define VLOG_NIVEL 0
#endif
#define LOG(X) if (X >= INFO || VLOG_NIVEL > 0) StringLogger(SHORT_FILE, __LINE__)
#define VLOG(X) if (X <= VLOG_NIVEL) StringLogger(SHORT_FILE, __LINE__)
#define LOG_EVERY_N(X, N) if (true) StringLogger(SHORT_FILE, __LINE__)

#else

// TODO dar um jeito de anular essas macros se nao tiver log.
#include <cstring>
#include <iostream>
// Arquivos querendo VLOG devem definir esse VLOG_NIVEL no android.
#ifndef VLOG_NIVEL
#define VLOG_NIVEL 0
#endif

#ifndef INFO
#define INFO 1
#define WARNING 2
#define ERROR 3
#endif

#define SHORT_FILE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(X) if (X > INFO || VLOG_NIVEL > 0) (X > 0 ? std::cerr : std::cout) << std::endl << SHORT_FILE << ":" << __LINE__ << " "
#define LOG_EVERY_N(X, N) if (X > INFO || VLOG_NIVEL > 0) std::cout << std::endl  << SHORT_FILE << ":" << __LINE__ << " "
#define VLOG(X) if (VLOG_NIVEL >= X) std::cout << std::endl

#endif

namespace meulog {

inline void Inicializa(int* argc, char*** argv) {
#if USAR_GLOG
  absl::InitializeLog();
  absl::ParseCommandLine(*argc, *argv);
#endif
}

}  // namespace meulog

#endif
