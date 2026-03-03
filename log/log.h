/** Macros de log. */
#ifndef LOG_LOG_H
#define LOG_LOG_H

#if USAR_GLOG

#include "absl/flags/parse.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#if WIN32
#include <vector>
#include "absl/strings/str_split.h"
#endif

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

inline void Inicializa(int& argc, char**& argv) {
#if USAR_GLOG
#if WIN32
  // Como nao consigo fazer o log do windows funcionar por flags, faço parsing na mão
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--stderrthreshold=0") == 0 || strcmp(argv[i], "--stderrthreshold=1") == 0) {
      for (int j = i + 1; j < argc; ++j) {
        argv[j - 1] = argv[j];
      }
      argc -= 1;
      absl::SetStderrThreshold(strcmp(argv[i], "--stderrthreshold=0") == 0 ? absl::LogSeverityAtLeast::kInfo : absl::LogSeverityAtLeast::kWarning);
      --i;
    } else if (strncmp(argv[i], "--vmodule=", 10) == 0) {
      std::vector<std::string> flag_module_level = absl::StrSplit(argv[i], "=");
      if (flag_module_level.size() == 3) {
        absl::SetVLogLevel(flag_module_level[1], atoi(flag_module_level[2].c_str()));
      }
      for (int j = i + 1; j < argc; ++j) {
        argv[j - 1] = argv[j];
      }
      argc -= 1;
      --i;
    }
  }
#endif
  absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();
#endif
}

}  // namespace meulog

#endif
