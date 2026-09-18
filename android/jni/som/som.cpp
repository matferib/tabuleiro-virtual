#include "som/som.h"

#include <aaudio/AAudio.h>
#include <cstdio>
#include <memory>
#include <vector>
#include <unordered_map>

#include "absl/strings/match.h"
#include "arq/arquivo.h"
#include "ent/util.h"
#include "log/log.h"

// Include inline (declarações e definições). O arquivo não é compilado separadamente.
#include "som/stb_vorbis.cpp"

namespace som {
namespace {

const ent::OpcoesProto* g_opcoes = nullptr;
struct AudioPlaybackContext;
std::unordered_map<std::string, AudioPlaybackContext*>* g_sons_fundo;

enum ModoTocar {
  UMA_VEZ,
  LOOP
};

// Estrutura para rastrear o estado da reprodução
struct AudioPlaybackContext {
  std::vector<int16_t> pcmData;
  size_t playHead = 0; // Posição atual da reprodução (em amostras)
  bool forcar_fim = false;
  ModoTocar modo = ModoTocar::UMA_VEZ;
};

// Esta função roda em uma thread dedicada em background do AAudio
aaudio_data_callback_result_t AudioCallback(AAudioStream *stream, void *contexto_cru, void *saida_audio_cru, int32_t frames) {
  auto contexto = std::unique_ptr<AudioPlaybackContext>(static_cast<AudioPlaybackContext*>(contexto_cru));
  auto* saida_audio = static_cast<int16_t*>(saida_audio_cru);
  int32_t channels = AAudioStream_getChannelCount(stream);
  int32_t samples_necessarios = frames * channels;
  int32_t samples_disponiveis = contexto->pcmData.size() - contexto->playHead;

  if (contexto->forcar_fim) {
    std::fill_n(saida_audio, samples_necessarios, 0);
    return AAUDIO_CALLBACK_RESULT_STOP;
  }

  // Se o áudio acabou, preenche com silêncio e para
  if (samples_disponiveis <= 0) {
    std::fill_n(saida_audio, samples_necessarios, 0);
    if (contexto->modo == ModoTocar::LOOP) {
      contexto->playHead = 0;
      contexto.release();  // ainda não mata, vamos precisar dele de novo.
      return AAUDIO_CALLBACK_RESULT_CONTINUE;
    } else {
      return AAUDIO_CALLBACK_RESULT_STOP;
    }
  }

  int32_t samples = std::min(samples_necessarios, samples_disponiveis);

  // Copia os dados do arquivo para o buffer de saída do hardware
  std::copy_n(contexto->pcmData.begin() + contexto->playHead, samples, saida_audio);
  contexto->playHead += samples;

  // Dados acabaram no meio deste bloco, limpa o resto com silêncio
  if (samples < samples_necessarios) {
    std::fill_n(saida_audio + samples, samples_necessarios - samples, 0);
    if (contexto->modo == ModoTocar::LOOP) {
      contexto->playHead = 0;
      contexto.release();  // ainda não mata, vamos precisar dele de novo.
      return AAUDIO_CALLBACK_RESULT_CONTINUE;
    } else {
      return AAUDIO_CALLBACK_RESULT_STOP;
    }
  }
  contexto.release();  // ainda não mata, vamos precisar dele de novo.
  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void DisparaOggEmBackground(const std::string& nome, ModoTocar modo) {
  std::string dados;
  try {
    arq::LeArquivo(arq::TIPO_SOM, nome, &dados);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Falha ao ler: " << nome << ": " << e.what();
    return;
  }

  // Variables to hold audio metadata
  int channels = 0;
  int sample_rate = 0;
  short* output_buffer = nullptr;

  // Decode the entire file into a 16-bit signed integer buffer (interleaved channels)
  int num_samples = stb_vorbis_decode_memory(reinterpret_cast<uint8_t*>(dados.data()), dados.size(), &channels, &sample_rate, &output_buffer);

  // CRITICAL: stb_vorbis allocates memory via standard C 'malloc'.
  // You must free it using C 'free' to avoid memory leaks.
  ent::RodaNoRetorno r([output_buffer]() {
    free(output_buffer);
  });

  // Error checking
  if (num_samples < 0) {
    LOG(ERROR) << "Falha ao tocar: " << nome << ": numero de samples: " << num_samples;
    return;
  }

  size_t frames = static_cast<size_t>(num_samples) * channels;

  // 2. Alocar o contexto e carregar todo o arquivo na memória
  auto contexto = std::make_unique<AudioPlaybackContext>();

  contexto->pcmData.resize(frames);
  std::copy_n(output_buffer, frames, contexto->pcmData.data());

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
    if (modo == ModoTocar::LOOP) {
      // Para sons em loop, precisamos salvar em g_sons_fundo para conseguir parar.
      auto& som_fundo = (*g_sons_fundo)[nome];
      som_fundo = contexto.get();
      som_fundo->modo = modo;
    }
    AAudioStream_requestStart(stream);
    contexto.release();  // tudo certo, não mata o contexto.
  }
  AAudioStreamBuilder_delete(builder);

  // You can now feed 'pcmData' directly into your audio engine (OpenAL, SDL_Audio, etc.)
  return;
}

// Le o arquivo WAV, preenche os buffers e dispara o playback em outra thread, que sera chamada
// continuamente ate retornar AAUDIO_CALLBACK_RESULT_STOP.
void DisparaWavEmBackground(const std::string& nome, ModoTocar modo) {
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
    if (modo == ModoTocar::LOOP) {
      // Para sons em loop, precisamos salvar em g_sons_fundo para conseguir parar.
      auto& som_fundo = (*g_sons_fundo)[nome];
      som_fundo = contexto.get();
      som_fundo->modo = modo;
    }
    AAudioStream_requestStart(stream);
    contexto.release();  // tudo certo, não mata o contexto.
  }
  AAudioStreamBuilder_delete(builder);
}
}  // namespace

void Inicia(const ent::OpcoesProto& opcoes) {
  g_opcoes = &opcoes;
  g_sons_fundo = new std::unordered_map<std::string, AudioPlaybackContext*>();
}

void Toca(const std::string& nome) {
  LOG(INFO) << "Toca: " << nome;
  if (g_opcoes->desativar_som() || nome.empty()) return;
  DisparaWavEmBackground(nome, ModoTocar::UMA_VEZ);
}

void TocaSomFundo(const std::string& nome) {
  LOG(INFO) << "TocaSomFundo: " << nome;
  if (g_opcoes->desativar_som() || nome.empty()) return;
  if (absl::EndsWith(nome, ".wav")) {
    DisparaWavEmBackground(nome, ModoTocar::LOOP);
  } else {
    DisparaOggEmBackground(nome, ModoTocar::LOOP);
  }
}

void ParaSomFundo(const std::string& nome) {
  LOG(INFO) << "ParaSomFundo: " << nome;
  if (g_opcoes->desativar_som() || nome.empty() || !g_sons_fundo->contains(nome)) return;
  (*g_sons_fundo)[nome]->forcar_fim = true;
}

void Finaliza() {
  delete g_sons_fundo;
  g_sons_fundo = nullptr;
}

}  // namespace som

