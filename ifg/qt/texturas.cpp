#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QImage>
#include <GL/gl.h>

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
  InfoTexturaInterna() : contador(1) {
  }
  ~InfoTexturaInterna() {
  }
  QImage qimage;
  int contador;
  ent::InfoTextura info;
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

const ent::InfoTextura* Texturas::Textura(const std::string& id) const {
  const InfoTexturaInterna* info_interna = InfoInterna(id);
  if (info_interna == nullptr) {
    return nullptr;
  }
  return &info_interna->info;
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
    info_interna = new InfoTexturaInterna;
    info_interna->qimage = imagem;
    info_interna->info.altura = imagem.height();
    info_interna->info.largura = imagem.width();
    info_interna->info.dados =  imagem.constBits();
    info_interna->info.formato = FormatoImagem(imagem);
    info_interna->info.tipo = TipoImagem(imagem);
    texturas_.insert(make_pair(id, info_interna));
    VLOG(1) << "Textura criada: " << id
            << "', " << info_interna->info.largura << "x" << info_interna->info.altura
            << ", bpp: " << imagem.depth() << ", format: " << (int)imagem.format();
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
