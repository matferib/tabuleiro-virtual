#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImage>
#if __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ent/entidade.h"
#include "ifg/qt/constantes.h"
#include "ifg/qt/texturas.h"
#include "log/log.h"
#include "ntf/notificacao.pb.h"

namespace ifg {
namespace qt {
namespace {

int FormatoImagem(const QImage& imagem) {
  switch (imagem.format()) {
    // ffRRGGBB: retorna invertido para inverter no tipo ja que nao tem um GL_ARGB.
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
      return GL_BGRA;
    default:
      return GL_BGRA;
  }
}

int TipoImagem(const QImage& imagem) {
  switch (imagem.format()) {
    // O formato foi BGRA que invertido da ARGB.
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
      return GL_UNSIGNED_INT_8_8_8_8_REV;
    default:
      return GL_UNSIGNED_INT_8_8_8_8_REV;
  }
}

}  // namespace

struct Texturas::InfoTexturaInterna {
  InfoTexturaInterna(const QImage& imagem) : contador(1) {
    glGenTextures(1, &id);
    if (id == GL_INVALID_VALUE) {
      LOG(ERROR) << "Erro gerando nome para textura";
      return;
    }
    glBindTexture(GL_TEXTURE_2D, id);
    // Mapeamento de texels em amostragem para cima e para baixo.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Carrega a textura.
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGBA,
                 imagem.height(), imagem.width(),
                 0, FormatoImagem(imagem), TipoImagem(imagem),
                 imagem.constBits());
    qimage = imagem;
    VLOG(1) << "Textura criada: '" << id
            << "', " << imagem.width() << "x" << imagem.height()
            << ", bpp: " << imagem.depth() << ", format: " << FormatoImagem(imagem);
    glDisable(GL_TEXTURE_2D);
  }
  ~InfoTexturaInterna() {
    if (id == GL_INVALID_VALUE) {
      return;
    }
    GLuint tex_name = id;
    glDeleteTextures(1, &tex_name);
  }
  QImage qimage;
  int contador;
  GLuint id;
};

Texturas::Texturas(ntf::CentralNotificacoes* central) {
  central_ = central;
  central_->RegistraReceptor(this);
}

Texturas::~Texturas() {}

bool Texturas::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_CARREGAR_TEXTURA:
      CarregaTextura(notificacao.endereco());
      return true;
    case ntf::TN_LIBERAR_TEXTURA:
      DescarregaTextura(notificacao.endereco());
      return true;
    default: ;
  }
  return false;
}

unsigned int Texturas::Textura(const std::string& id) const {
  const InfoTexturaInterna* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    return GL_INVALID_VALUE;
  }
  return info_interna->id;
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

void Texturas::CarregaTextura(const std::string& id) {
  auto* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    QFileInfo arquivo(QDir(DIR_TEXTURAS), id.c_str());
    QImageReader leitor_imagem(arquivo.absoluteFilePath());
    QImage imagem = leitor_imagem.read();
    if (imagem.isNull()) {
      LOG(ERROR) << "Textura inválida: " << id;
      return;
    }
    texturas_.insert(make_pair(id, new InfoTexturaInterna(imagem)));
  } else {
    ++info_interna->contador;
    VLOG(1) << "Textura '" << id << "' incrementada para " << info_interna->contador;
  }
}

void Texturas::DescarregaTextura(const std::string& id) {
  auto* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    LOG(WARNING) << "Textura nao existente: " << id;
  } else {
    if (--info_interna->contador == 0) {
      VLOG(1) << "Textura liberada: " << id;
      delete info_interna;
      texturas_.erase(id);
    } else {
      VLOG(1) << "Textura '" << id << "' decrementada para " << info_interna->contador;
    }
  }
}

}  // namespace qt 
}  // namespace ifg 
