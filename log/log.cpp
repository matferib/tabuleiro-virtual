#include "log/log.h"

namespace meulog {
inline void Inicializa(int& argc, char**& argv) {
#if USAR_GLOG
#if 0 && WIN32
  // Como nao consigo fazer o log do windows funcionar por flags, faço parsing na mão
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--stderrthreshold=0") == 0 || strcmp(argv[i], "--stderrthreshold=1") == 0) {
      for (int j = i + 1; j < argc; ++j) {
        argv[j - 1] = argv[j];
      }
      argc -= 1;
      absl::SetStderrThreshold(strcmp(argv[i], "--stderrthreshold=0") == 0 ? absl::LogSeverityAtLeast::kInfo : absl::LogSeverityAtLeast::kWarning);
      --i;
    }
    else if (strncmp(argv[i], "--vmodule=", 10) == 0) {
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