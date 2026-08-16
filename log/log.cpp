#include "log/log.h"

namespace meulog {
ABSL_ATTRIBUTE_NOINLINE
void Inicializa(int& argc, char**& argv) {
#if USAR_GLOG
// Isso aqui é para quando não funcionava o parsing de flags. Mas o gemini brilhou e deu a seguinte resposta:
// Passo 1:
//   Adicionar a biblioteca nas dependências adicionaisNo seu Visual Studio, clique com o botão direito no seu Projeto e selecione Propriedades.
//   No menu esquerdo, vá em Propriedades de Configuração > Linker(Vinculador) > Entrada.Na linha Dependências Adicionais, adicione o seguinte arquivo ao início da lista : absl_log_flags.lib;
// Passo 2: Forçar o carregamento completo(O segredo do MSVC)
//   Como o compilador tenta otimizar e remover o que julga "não usado", você precisa dizer ao Linker para trazer todo o conteúdo da biblioteca usando a diretiva / WHOLEARCHIVE :
//  Ainda nas propriedades do projeto, vá em Linker(Vinculador) > Linha de Comando.Na caixa de texto Opções Adicionais(na parte inferior), cole exatamente o seguinte comando : text / WHOLEARCHIVE : absl_log_flags.lib
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
  absl::InitializeLog();
  absl::ParseCommandLine(argc, argv);
#endif
}
}  // namespace meulog