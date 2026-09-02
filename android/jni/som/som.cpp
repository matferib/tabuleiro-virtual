#include "som/som.h"

#include <aaudio/AAudio.h>
#include <cstdio>
#include <memory>
#include <vector>

#include "arq/arquivo.h"
#include "log/log.h"

namespace som {
namespace {

const ent::OpcoesProto* g_opcoes = nullptr;

// Estrutura para rastrear o estado da reprodução
struct AudioPlaybackContext {
    std::vector<int16_t> pcmData;
    size_t playHead = 0; // Posição atual da reprodução (em amostras)
};

// Esta função roda em uma thread dedicada em background do AAudio
aaudio_data_callback_result_t AudioCallback(AAudioStream *stream, void *contexto_cru, void *saida_audio_cru, int32_t frames) {
  auto contexto = std::unique_ptr<AudioPlaybackContext>(static_cast<AudioPlaybackContext*>(contexto_cru));
  auto* saida_audio = static_cast<int16_t*>(saida_audio_cru);

  int32_t channels = AAudioStream_getChannelCount(stream);
  int32_t samples_necessarios = frames * channels;
  int32_t samples_disponiveis = contexto->pcmData.size() - contexto->playHead;

  // Se o áudio acabou, preenche com silêncio e para
  if (samples_disponiveis <= 0) {
    std::fill_n(saida_audio, samples_necessarios, 0);
    return AAUDIO_CALLBACK_RESULT_STOP;
  }

  int32_t samples = std::min(samples_necessarios, samples_disponiveis);

  // Copia os dados do arquivo para o buffer de saída do hardware
  std::copy_n(contexto->pcmData.begin() + contexto->playHead, samples, saida_audio);
  contexto->playHead += samples;

  // Dados acabaram no meio deste bloco, limpa o resto com silêncio
  if (samples < samples_necessarios) {
    std::fill_n(saida_audio + samples, samples_necessarios - samples, 0);
    return AAUDIO_CALLBACK_RESULT_STOP;
  }
  contexto.release();  // ainda não mata, vamos precisar dele de novo.
  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  g_opcoes = &opcoes;
}

void Toca(const std::string& nome) {
  //if (g_opcoes->desativar_som()) return;

  std::string dados;
  try {
    arq::LeArquivo(arq::TIPO_SOM, nome, &dados);
    if (dados.size() < 44) throw std::logic_error("arquivo de som sem cabeçalho.");
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha ao tocar: " << nome << ": " << e.what();
    return;
  }

  // Read and parse simple WAV header (44 bytes standard)
  char* header = static_cast<char*>(dados.data());
  int sample_rate = *reinterpret_cast<int*>(&header[24]);
  short channels = *reinterpret_cast<short*>(&header[22]);

  // 2. Alocar o contexto e carregar todo o arquivo na memória
  auto contexto = std::make_unique<AudioPlaybackContext>();
  int16_t* body = reinterpret_cast<int16_t*>(dados.data() + 44);
  int frames = (dados.size() - 44) / sizeof(int16_t);

  contexto->pcmData.resize(frames);
  std::copy_n(body, frames, contexto->pcmData.data());

  // 3. Configurar o Stream Builder do AAudio
  AAudioStreamBuilder* builder = nullptr;
  AAudio_createStreamBuilder(&builder);
  if (builder == nullptr) {
    LOG(ERROR) << "Falha ao tocar: " << nome << ": builder nullptr.";
    return;
  }
  AAudioStreamBuilder_setSampleRate(builder, sample_rate);
  AAudioStreamBuilder_setChannelCount(builder, channels);
  AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
  AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
  AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);

  // Define a função de callback e passa os dados de áudio
  AAudioStreamBuilder_setDataCallback(builder, AudioCallback, contexto.get());

  // 4. Abrir e iniciar o Stream
  AAudioStream* stream = nullptr;
  if (AAudioStreamBuilder_openStream(builder, &stream) == AAUDIO_OK) {
    AAudioStream_requestStart(stream);
    contexto.release();  // tudo certo, não mata o contexto.
  }
  AAudioStreamBuilder_delete(builder);
}

void TocaSomFundo(const std::string& nome) {

void ParaSomFundo(const std::string& nome) {


void Finaliza() {}

}  // namespace som

