#include <cstring>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include <set>
#include <stdexcept>
#if __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
  #elif TARGET_OS_MAC
    #define MAC_OSX 1
  #endif
#endif
//#define VLOG_NIVEL 1
#include "log/log.h"

#include "arq/arquivo.h"
//#include "ent/entidade.h"
#include "ent/entidade.pb.h"
#include "gltab/gl.h"
#include "ntf/notificacao.pb.h"
#include "tex/lodepng.h"
#include "tex/texturas.h"

namespace tex {

namespace {

/** Realiza a leitura da imagem de um caminho, preenchendo dados com conteudo do arquivo no caminho.
* Caso local, a textura sera local ao jogador. Caso contrario, eh uma textura global (da aplicacao).
* @throws std::exception em caso de erro na leitura.
*/
void LeImagem(bool global, const std::string& arquivo, std::vector<unsigned char>* dados) {
  boost::filesystem::path caminho(arquivo);
  std::string dados_str;
  try {
    arq::LeArquivo(global ? arq::TIPO_TEXTURA : arq::TIPO_TEXTURA_LOCAL, caminho.filename().string(), &dados_str);
  } catch (const std::exception&) {
    if (global) {
      // Fallback de texturas baixadas.
      try {
        VLOG(1) << "Tentando fallback de " << arquivo << ", global";
        arq::LeArquivo(arq::TIPO_TEXTURA_BAIXADA, caminho.filename().string(), &dados_str);
      } catch (...) {
        LOG(ERROR) << "Falha lendo arquivo " << arquivo << ", global";
        throw;
      }
    } else {
      LOG(ERROR) << "Falha lendo arquivo " << arquivo << ", nao global";
      throw;
    }
  }
  dados->assign(dados_str.begin(), dados_str.end());
}
void LeImagem(arq::tipo_e tipo, const std::string& nome, std::vector<unsigned char>* dados) {
  std::string dados_str;
  try {
    arq::LeArquivo(tipo, nome, &dados_str);
  } catch (const std::exception&) {
    if (tipo == arq::TIPO_TEXTURA) {
      // Fallback de texturas baixadas.
      try {
        VLOG(1) << "Tentando fallback de " << nome << ", global";
        arq::LeArquivo(arq::TIPO_TEXTURA_BAIXADA, nome, &dados_str);
      } catch (...) {
        LOG(ERROR) << "Falha lendo arquivo " << nome << ", global";
        throw;
      }
    } else {
      LOG(ERROR) << "Falha lendo arquivo " << nome << ", nao global";
      throw;
    }
  }
  dados->assign(dados_str.begin(), dados_str.end());
}

/** Decodifica os dados crus de info_textura, devolvendo altura, largura e os bits decodificados. */
void DecodificaImagem(
    const std::string& dados_entrada, unsigned int* plargura, unsigned int* paltura, std::vector<unsigned char>* bits,
    bool inverte_y = false, bool inverte_x = false) {
  std::vector<unsigned char> dados_crus(dados_entrada.begin(), dados_entrada.end());
  std::vector<unsigned char> dados;
  lodepng::State estado;
  unsigned int largura = 0, altura = 0;
  unsigned int error = lodepng::decode(dados, largura, altura, estado, dados_crus);
  *plargura = largura;
  *paltura = altura;
  if (error != 0) {
    throw std::logic_error(std::string("Erro decodificando: ") + lodepng_error_text(error));
  }
  const LodePNGColorMode& color = estado.info_png.color;
  VLOG(2) << "Color type: " << color.colortype;
  VLOG(2) << "Bit depth: " << color.bitdepth;
  VLOG(2) << "Bits per pixel: " << lodepng_get_bpp(&color);
  VLOG(2) << "Channels per pixel: " << lodepng_get_channels(&color);
  VLOG(2) << "Is greyscale type: " << lodepng_is_greyscale_type(&color);
  VLOG(2) << "Can have alpha: " << lodepng_can_have_alpha(&color);
  VLOG(2) << "Palette size: " << color.palettesize;
  VLOG(2) << "Has color key: " << color.key_defined;
  if (color.key_defined) {
    VLOG(2) << "Color key r: " << color.key_r;
    VLOG(2) << "Color key g: " << color.key_g;
    VLOG(2) << "Color key b: " << color.key_b;
  }
  if (inverte_y) {
    const int bytes_per_pixel = 4;  // RGBA
    int largura_linha_bytes = largura * bytes_per_pixel;
    std::vector<unsigned char> t(largura_linha_bytes);
    //LOG(ERROR) << "bytes per pixel: " << bytes_per_pixel << ", largura linha bytes: " << largura_linha_bytes;
    for (unsigned int linha = 0; linha < altura / 2; ++linha) {
      memcpy(&t[0], &dados[linha * largura_linha_bytes], largura_linha_bytes);
      memcpy(&dados[linha * largura_linha_bytes], &dados[(altura - linha - 1) * largura_linha_bytes], largura_linha_bytes);
      memcpy(&dados[(altura - linha - 1) * largura_linha_bytes], &t[0], largura_linha_bytes);
    }
  }
  if (inverte_x) {
    const int bytes_per_pixel = 4;  // RGBA
    int largura_linha_bytes = largura * bytes_per_pixel;
    std::vector<unsigned char> t(4);
    for (unsigned int linha = 0; linha < altura; ++linha) {
      for (unsigned int coluna = 0; coluna < largura / 2; ++coluna) {
        memcpy(&t[0], &dados[linha * largura_linha_bytes + coluna * 4], 4);
        memcpy(&dados[linha * largura_linha_bytes + coluna * 4], &dados[linha * largura_linha_bytes + (largura - coluna - 1) * 4], 4);
        memcpy(&dados[linha * largura_linha_bytes + (largura - coluna - 1) * 4], &t[0], 4);
      }
    }
  }

  bits->clear();
  bits->insert(bits->begin(), dados.begin(), dados.end());
}

/** Retorna o formato OpenGL de uma imagem, por exemplo: GL_BGRA. */
int FormatoImagem() {
  return GL_RGBA;
}

/** Retorna o tipo OpenGL de uma imagem, por exemplo: GL_UNSIGNED_INT_8_8_8_8_REV. */
int TipoImagem() {
  return GL_UNSIGNED_BYTE;
}

}  // namespace

class Texturas::InfoTexturaInterna {
 public:
  explicit InfoTexturaInterna(const std::string& id_mapa, bool global) : global_(global), contador_(1), id_(GL_INVALID_VALUE), formato_(FormatoImagem()) {
    VLOG(1) << "InfoTexturaInterna falsa criada: id: '" << id_mapa << "'";
    imagem_.set_id(id_mapa);
  }

  InfoTexturaInterna(const std::string& id_mapa, bool global, const ent::InfoTextura& info_textura)
      : global_(global), contador_(1), id_(GL_INVALID_VALUE), formato_(FormatoImagem()) {
    imagem_ = info_textura;
    imagem_.set_id(id_mapa);
    // Decodifica.
    try {
      DecodificaImagem(info_textura);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Textura inválida: " << e.what() << ", textura: " << info_textura.id() << ", cubo: " << info_textura.textura_cubo();
      return;
    }
    // Cria a textura openGL.
    try {
      CriaTexturaOpenGl();
    } catch (...) {
      imagem_.Clear();
      LOG(ERROR) << "Erro gerando nome para textura";
      return;
    }
    VLOG(1) << "InfoTexturaInterna criada: id: '" << id_mapa << "', id OpenGL: '" << id_
            << "', " << largura_ << "x" << altura_
            << ", format: " << FormatoImagem();
  }

  ~InfoTexturaInterna() {
    if (id_ == GL_INVALID_VALUE) {
      return;
    }
    ApagaTexturaOpengl();
  }

  // Retorna id opengl da textura.
  GLuint Id() const { return id_; }

  // Retorna o tipo de textura.
  GLenum Tipo() const { return imagem_.textura_cubo() ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D; }

  // Incremento e decremento de contador de refencia.
  int Ref() { return ++contador_; }
  int Deref() { return --contador_; }

  // Rele a textura se for global (locais nao sao recarregadas).
  void Rele() {
    if (!global_) {
      // So as globais sao relidas.
      return;
    }
    // Textura global.
    VLOG(1) << "Relendo textura global, id: '" << imagem_.id() << "'.";
    ent::InfoTextura info_lido;
    try {
      std::vector<unsigned char> lido;
      LeImagem(true  /*global*/, imagem_.id(), &lido);
      imagem_.mutable_bits_crus()->resize(lido.size());
      imagem_.mutable_bits_crus()->assign((char*)lido.data(), (char*)(lido.data() + lido.size()));
      //imagem_.mutable_bits_crus()->insert(imagem_.bits_crus().begin(), lido.begin(), lido.end());
      if (FormatoImagem() == -1) {
        throw std::logic_error("formato invalido");
      }
      DecodificaImagem(imagem_);
    } catch (const std::exception& e) {
      LOG(ERROR) << "Textura inválida: " << imagem_.ShortDebugString() << " para releitura, excecao: " << e.what();
    }
  }

  // Apaga a textura OpenGL. Publica para recarregar.
  void ApagaTexturaOpengl() {
    VLOG(1) << "Apagando textura para: " << imagem_.id() << ", id openGL: " << id_;
    gl::ApagaTexturas(1, &id_);
    id_ = GL_INVALID_VALUE;
  }

  // Cria a textura openGL. Publica para Recarregar poder acessar tambem.
  void CriaTexturaOpenGl() {
    if (!imagem_.textura_cubo()) {
      CriaTexturaOpenGlNormal();
    } else {
      CriaTexturaOpenGlCubo();
    }
  }

  void CriaTexturaOpenGlNormal() {
    // So cria se a textura tiver bits.
    if (bits_.empty()) {
      return;
    }
    gl::GeraTexturas(1, &id_);
    V_ERRO("GeraTexturas");
    VLOG(1) << "Criando textura para: " << imagem_.id() << ", id openGL: " << id_;
    if (id_ == GL_INVALID_VALUE) {
      throw std::logic_error("Erro criando textura (glGenTextures)");
    }
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_);
    gl::HabilitaMipmapAniso(GL_TEXTURE_2D);
    V_ERRO("Ligacao");
    // Carrega a textura.
    gl::ImagemTextura2d(GL_TEXTURE_2D,
                        0, GL_RGBA,
                        largura_, altura_,
                        0, FormatoImagem(), TipoImagem(),
                        bits_.data());
    V_ERRO("Imagem");
#if WIN32 || MAC_OSX || TARGET_OS_IPHONE || (__linux__ && !ANDROID)
    // TODO wrapper para outros...
    gl::GeraMipmap(GL_TEXTURE_2D);
#endif
    gl::LigacaoComTextura(GL_TEXTURE_2D, 0);
    gl::DesabilitaMipmapAniso(GL_TEXTURE_2D);
    V_ERRO("CriaTexturaOpenGl id: " << imagem_.id());
  }

  void CriaTexturaOpenGlCubo() {
    // So cria se a textura tiver bits.
    if (bits_direita_.empty() || bits_frente_.empty() || bits_cima_.empty() ||
        bits_esquerda_.empty() || bits_atras_.empty() || bits_baixo_.empty()) {
      return;
    }
    gl::GeraTexturas(1, &id_);
    V_ERRO("GeraTexturasCubo");
    VLOG(1) << "Criando textura cubo para: " << imagem_.id() << ", id openGL: " << id_;
    if (id_ == GL_INVALID_VALUE) {
      throw std::logic_error("Erro criando textura cubo (glGenTextures)");
    }
    gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, id_);
    gl::HabilitaMipmapAniso(GL_TEXTURE_CUBE_MAP);
    V_ERRO("Ligacao Cubo");
    // Carrega a textura.
    struct DadosTextura {
      GLenum alvo;
      std::vector<unsigned char>* bits;
    };
    // Seguindo a convencao do OpenGL que Y pra cima e -Z eh pra frente. Isso sera feito no shader de caixa de ceu.
    std::vector<DadosTextura> dados_texturas = {
      { GL_TEXTURE_CUBE_MAP_POSITIVE_X, &bits_direita_},
      { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, &bits_cima_},
      { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, &bits_atras_},
      { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, &bits_esquerda_ },
      { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, &bits_baixo_ },
      { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, &bits_frente_ },
    };
    for (const auto& dado_textura : dados_texturas) {
      gl::ImagemTextura2d(
          dado_textura.alvo,
          0, GL_RGBA,
          largura_, altura_,
          0, FormatoImagem(), TipoImagem(),
          dado_textura.bits->data());
      V_ERRO("ImagemCubo");
    }
#if WIN32 || MAC_OSX || TARGET_OS_IPHONE || (__linux__ && !ANDROID)
    // TODO wrapper para outros...
    gl::GeraMipmap(GL_TEXTURE_CUBE_MAP);
#endif
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if !USAR_OPENGL_ES
    gl::ParametroTextura(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

    gl::LigacaoComTextura(GL_TEXTURE_CUBE_MAP, 0);
    gl::DesabilitaMipmapAniso(GL_TEXTURE_CUBE_MAP);
    V_ERRO("CriaTexturaOpenGlCubo");
  }

 private:
  /** Decodifica os dados_crus, preenchendo info_textura. */
  void DecodificaImagem(const ent::InfoTextura& info_textura) {
    if (!info_textura.textura_cubo()) {
      tex::DecodificaImagem(info_textura.bits_crus(), &largura_, &altura_, &bits_);
    } else {
      unsigned int largura, altura;
      tex::DecodificaImagem(info_textura.bits_crus_direita(), &largura_, &altura_, &bits_direita_, false, false);
      tex::DecodificaImagem(info_textura.bits_crus_frente(), &largura, &altura, &bits_frente_, false, false);
      if (largura_ != largura) throw std::logic_error("tamanho invalido de textura");
      tex::DecodificaImagem(info_textura.bits_crus_cima(), &largura, &altura, &bits_cima_, true, true);
      if (largura_ != largura) throw std::logic_error("tamanho invalido de textura");
      tex::DecodificaImagem(info_textura.bits_crus_esquerda(), &largura, &altura, &bits_esquerda_, false, false);
      if (largura_ != largura) throw std::logic_error("tamanho invalido de textura");
      tex::DecodificaImagem(info_textura.bits_crus_atras(), &largura, &altura, &bits_atras_, false, false);
      if (largura_ != largura) throw std::logic_error("tamanho invalido de textura");
      tex::DecodificaImagem(info_textura.bits_crus_baixo(), &largura, &altura, &bits_baixo_, true, true);
      if (largura_ != largura) throw std::logic_error("tamanho invalido de textura");
    }
  }

 private:
  bool global_;  // indica se a textura eh global ou local (para recarregamento).
  // Contador de referencia.
  int contador_;

  ent::InfoTextura imagem_;
  GLuint id_;
  unsigned int largura_;
  unsigned int altura_;
  int formato_;
  std::vector<unsigned char> bits_;
  std::vector<unsigned char> bits_direita_;
  std::vector<unsigned char> bits_esquerda_;
  std::vector<unsigned char> bits_frente_;
  std::vector<unsigned char> bits_atras_;
  std::vector<unsigned char> bits_cima_;
  std::vector<unsigned char> bits_baixo_;
};

Texturas::Texturas(ntf::CentralNotificacoes* central) {
  central_ = central;
  central_->RegistraReceptor(this);
  //central_->RegistraReceptorRemoto(this);
}

Texturas::~Texturas() {}

void Texturas::CarregaTexturas(const ntf::Notificacao& notificacao) {
  for (const auto& info_textura : notificacao.info_textura()) {
    CarregaTextura(info_textura);
  }
}

void Texturas::DescarregaTexturas(const ntf::Notificacao& notificacao) {
  for (const auto& info_textura : notificacao.info_textura()) {
    DescarregaTextura(info_textura);
  }
}

// Interface de ent::Texturas.
bool Texturas::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_CARREGAR_TEXTURA: {
      CarregaTexturas(notificacao);
      return true;
    }
    case ntf::TN_DESCARREGAR_TEXTURA: {
      DescarregaTexturas(notificacao);
      return true;
    }
    case ntf::TN_ENVIAR_IDS_TABELAS_TEXTURAS_E_MODELOS_3D: {
      // Primeira notificacao eh local;
      if (!notificacao.local()) {
        return false;
      }
      // Notificacao local: envia os ids de texturas locais para o servidor.
      auto n = ntf::NovaNotificacao(ntf::TN_REQUISITAR_TEXTURAS);
      // Percorre arquivos globais.
      std::vector<std::string> globais(arq::ConteudoDiretorio(arq::TIPO_TEXTURA));
      for (const std::string& id : globais ) {
        if (id.size() < 4 || id.find(".png") == std::string::npos) {
          continue;
        }
        n->add_info_textura()->set_id(id);
      }
      // Percorre arquivos baixados.
      std::vector<std::string> baixadas(arq::ConteudoDiretorio(arq::TIPO_TEXTURA_BAIXADA));
      for (const std::string& id : baixadas) {
        n->add_info_textura()->set_id(id);
      }
      n->set_id_rede(notificacao.id_rede());
      n->set_servidor_apenas(true);
      // Envia para o servidor.
      VLOG(1) << "Enviando remoto TN_REQUISITAR_TEXTURAS: " << n->DebugString();
      central_->AdicionaNotificacaoRemota(n.release());
      return true;
    }
    case ntf::TN_REQUISITAR_TEXTURAS: {
      // Servidor recebendo de cliente.
      if (notificacao.local()) {
        return false;
      }
      VLOG(1) << "Recebendo de cliente TN_REQUISITAR_TEXTURAS: " << notificacao.DebugString();
      std::unordered_set<std::string> ids;
      // Percorre arquivos globais.
      std::vector<std::string> globais(arq::ConteudoDiretorio(arq::TIPO_TEXTURA));
      ids.insert(globais.begin(), globais.end());
      // Percorre arquivos baixados.
      std::vector<std::string> baixadas(arq::ConteudoDiretorio(arq::TIPO_TEXTURA_BAIXADA));
      ids.insert(baixadas.begin(), baixadas.end());
      std::set<std::string> ids_cliente;
      std::vector<std::string> ids_faltantes;
      for (const auto& info : notificacao.info_textura()) {
        ids_cliente.insert(info.id());
      }
      for (const auto& id : ids) {
        if (id.size() < 4 || id.find(".png") == std::string::npos) {
          continue;
        }
        if (ids_cliente.find(id) == ids_cliente.end()) {
          VLOG(1) << "Faltando textura para cliente: " << id;
          ids_faltantes.push_back(id);
        }
      }
      if (ids_faltantes.empty()) {
        VLOG(1) << "Cliente tem todas as texuras.";
        return true;
      }
      // Compara os arquivos recebidos com os baixados.
      // Envia para o servidor.
      auto n = ntf::NovaNotificacao(ntf::TN_ENVIAR_TEXTURAS);
      for (const auto& id : ids_faltantes) {
        auto* info = n->add_info_textura();
        std::vector<unsigned char> dados;
        LeImagem(true  /*global*/, id, &dados);
        info->mutable_bits_crus()->append(dados.begin(), dados.end());
        info->set_id(id);
      }
      n->set_id_rede(notificacao.id_rede());
      VLOG(1) << "Enviando texturas faltantes a cliente.";
      central_->AdicionaNotificacaoRemota(n.release());
      return true;
    }
    case ntf::TN_ENVIAR_TEXTURAS: {
      // Cliente recebendo texturas de servidor.
      if (notificacao.local()) {
        return false;
      }
      VLOG(1) << "Recebendo TN_ENVIAR_TEXTURAS do servidor";
      for (const auto& info : notificacao.info_textura()) {
        // Salva bits crus em texturas_baixadas com id da textura.
        arq::EscreveArquivo(arq::TIPO_TEXTURA_BAIXADA, info.id(), info.bits_crus());
      }
      Recarrega(true  /*rele*/);
      return true;
    }
    default: ;
  }
  return false;
}

unsigned int Texturas::Textura(const std::string& id) const {
  const InfoTexturaInterna* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    return GL_INVALID_VALUE;
  }
  return info_interna->Id();
}

unsigned int Texturas::TipoTextura(const std::string& id) const {
  const InfoTexturaInterna* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    return GL_TEXTURE_2D;
  }
  return info_interna->Tipo();
}

// Fim da interface ent::Texturas.

void Texturas::Recarrega(bool rele) {
  // Apaga todas as texturas primeiro para garantir que nao havera repeticoes de ids.
  for (auto& cv : texturas_) {
    LOG(INFO) << "Apagando: " << cv.first;
    cv.second->ApagaTexturaOpengl();
  }
  // Agora recria.
  for (auto& cv : texturas_) {
    LOG(INFO) << "Recriando: " << cv.first;
    if (rele) {
      cv.second->Rele();
    }
    cv.second->CriaTexturaOpenGl();
  }
}

Texturas::InfoTexturaInterna* Texturas::InfoInterna(const std::string& id) {
  auto it = texturas_.find(id);
  if (it == texturas_.end()) {
    return nullptr;
  }
  return it->second;
}

const Texturas::InfoTexturaInterna* Texturas::InfoInterna(const std::string& id) const {
  auto it = texturas_.find(id);
  if (it == texturas_.end()) {
    return nullptr;
  }
  return it->second;
}

void Texturas::CarregaTextura(const ent::InfoTextura& info_textura_const) {
  ent::InfoTextura info_textura(info_textura_const);
  if (info_textura.id().find(".cube") != std::string::npos) {
    info_textura.set_textura_cubo(true);
  }
  auto* info_interna = InfoInterna(info_textura.id());
  if (info_interna != nullptr) {
    int contador = info_interna->Ref();
    VLOG(1) << "Textura '" << info_textura.id() << "' incrementada para " << contador;
    return;
  }
  // Carrega a textura.
  if (info_textura.has_bits_crus()) {
    VLOG(1) << "Carregando textura local com bits crus, id: '" << info_textura.id() << "'.";
    texturas_.insert(make_pair(info_textura.id(), new InfoTexturaInterna(info_textura.id(), false  /*global*/, info_textura)));
  } else {
    if (!info_textura.textura_cubo()) {
      // Textura global.
      VLOG(1) << "Carregando textura global, id: '" << info_textura.id() << "'.";
      try {
        ent::InfoTextura info_lido;
        std::vector<unsigned char> lido;
        LeImagem(true  /*global*/, info_textura.id(), &lido);
        info_lido.set_id(info_textura.id());
        info_lido.mutable_bits_crus()->resize(lido.size());
        info_lido.mutable_bits_crus()->assign((char*)lido.data(), (char*)(lido.data() + lido.size()));
        if (FormatoImagem() == -1) {
          throw std::logic_error("formato invalido");
        }
        texturas_.insert(make_pair(info_textura.id(), new InfoTexturaInterna(info_textura.id(), true  /*global*/, info_lido)));
      } catch (const std::exception& e) {
        LOG(ERROR) << "Textura inválida: {" << info_textura.ShortDebugString() << "}, excecao: " << e.what();
        // Cria textura fake.
        texturas_.insert(make_pair(info_textura.id(), new InfoTexturaInterna(info_textura.id(), true  /*global*/)));
      }
    } else {
      // Textura cubo global.
      VLOG(1) << "Carregando textura cubo global, id: '" << info_textura.id() << "'.";
      struct DadosTextura {
        std::string sufixo;
        std::string* bits_crus;
      };
      ent::InfoTextura info_lido;
      const std::vector<DadosTextura> dados_texturas = {
        { "esquerda.png", info_lido.mutable_bits_crus_esquerda() },
        { "atras.png", info_lido.mutable_bits_crus_atras() },
        { "baixo.png", info_lido.mutable_bits_crus_baixo() },
        { "direita.png", info_lido.mutable_bits_crus_direita() },
        { "frente.png", info_lido.mutable_bits_crus_frente() },
        { "cima.png", info_lido.mutable_bits_crus_cima() }
      };
      try {
        std::string prefixo = info_textura.id();
        prefixo.replace(prefixo.find(".cube"), 5, "");
        for (const auto& dados_textura : dados_texturas) {
          std::vector<unsigned char> lido;
          LeImagem(true  /*global*/, prefixo + dados_textura.sufixo, &lido);
          info_lido.set_id(info_textura.id());
          info_lido.set_textura_cubo(true);
          dados_textura.bits_crus->resize(lido.size());
          dados_textura.bits_crus->assign((char*)lido.data(), (char*)(lido.data() + lido.size()));
          if (FormatoImagem() == -1) {
            throw std::logic_error("formato invalido");
          }
        }
      } catch (const std::exception& e) {
        LOG(ERROR) << "Textura inválida: " << e.what() << ", info_textura: " << info_textura.ShortDebugString();
        // Cria textura fake.
        texturas_.insert(make_pair(info_textura.id(), new InfoTexturaInterna(info_textura.id(), true  /*global*/)));
      }
      texturas_.insert(make_pair(info_textura.id(), new InfoTexturaInterna(info_textura.id(), true  /*global*/, info_lido)));
    }
  }
}

void Texturas::DescarregaTextura(const ent::InfoTextura& info_textura) {
  auto* info_interna = InfoInterna(info_textura.id());
  if (info_interna == nullptr) {
    LOG(WARNING) << "Textura nao existente para descarregar: " << info_textura.id();
  } else {
    int contador = info_interna->Deref();
    if (contador == 0) {
      VLOG(1) << "Textura liberada: " << info_textura.id();
      delete info_interna;
      texturas_.erase(info_textura.id());
    } else {
      VLOG(1) << "Textura '" << info_textura.id() << "' decrementada para " << contador;
    }
  }
}

// static
void Texturas::LeDecodificaImagem(
    bool global, const std::string& caminho, ent::InfoTextura* info_textura, unsigned int* largura, unsigned int* altura) {
  std::vector<unsigned char> dados_arquivo;
  LeImagem(global, caminho, &dados_arquivo);
  if (dados_arquivo.size() <= 0) {
    throw std::logic_error(std::string("Erro lendo imagem: ") + caminho);
  }
  info_textura->clear_bits_crus();
  info_textura->mutable_bits_crus()->append(dados_arquivo.begin(), dados_arquivo.end());
  std::vector<unsigned char> nao_usado;
  DecodificaImagem(info_textura->bits_crus(), largura, altura, &nao_usado);
}

void Texturas::LeDecodificaImagemTipo(
    arq::tipo_e tipo, const std::string& nome, ent::InfoTextura* info_textura, unsigned int* largura, unsigned int* altura) {
  std::vector<unsigned char> dados_arquivo;
  LeImagem(tipo, nome, &dados_arquivo);
  if (dados_arquivo.size() <= 0) {
    throw std::logic_error(std::string("Erro lendo imagem: ") + nome);
  }
  info_textura->clear_bits_crus();
  info_textura->mutable_bits_crus()->append(dados_arquivo.begin(), dados_arquivo.end());
  std::vector<unsigned char> nao_usado;
  DecodificaImagem(info_textura->bits_crus(), largura, altura, &nao_usado);
}

}  // namespace tex
