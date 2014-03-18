/** Macros de log. */

#if USAR_GLOG
#include <gflags/gflags.h>
#include <glog/logging.h>

#elif __ANDROID__

// TODO converter para android log.
#include <android/log.h>
#include <string>
#include <sstream>

class StringLogger {
 public:
  StringLogger(const char* file, int line) {
    std::ostringstream oss;
    oss << file << ", " << line << ": ";
    str_ = oss.str();
  }

  const std::string& str() const { return str_; }

 private:
  std::string str_;
};

template <class T>
inline const StringLogger& operator<<(const StringLogger& logger, const T& msg) {
  return logger;
}

template <>
inline const StringLogger& operator<<(const StringLogger& logger, const std::string& msg) {
  __android_log_print(ANDROID_LOG_INFO, "Tabuleiro", "%s%s", logger.str().c_str(), msg.c_str());
  return logger;
}

#define LOG(X) if (false) StringLogger(__FILE__, __LINE__)
#define VLOG(X) if (false && X <= 1) StringLogger(__FILE__, __LINE__)
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
