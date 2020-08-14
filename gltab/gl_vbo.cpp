#include <cstdio>
#include <cstring>
#include <limits>
#include "gltab/gl_interno.h"
#include "gltab/gl_vbo.h"
#include "goog/stringprintf.h"
#include "log/log.h"

extern bool g_hack;

namespace gl {

namespace {
using google::protobuf::StringPrintf;
}  // namespace

bool ImprimeSeErro(const char* mais = nullptr);

namespace {

// retorna o angulo do vetor formado por x,y, alem de seu tamanho.
float VetorParaRotacaoGraus(float x, float y, float* tamanho) {
  float tam = sqrt(x * x + y * y);
  if (tam == 0) {
    if (tamanho != nullptr) {
      *tamanho = 0;
    }
    return 0;
  }
  float angulo = acosf(x / tam) * RAD_PARA_GRAUS;
  if (tamanho != nullptr) {
    *tamanho = tam;
  }
  return (y >= 0 ? angulo : -angulo);
}

// Matriz de rotacao do vetor up (z=1) para o vetor transformado.
#if 0
Matrix4 MatrizRotacaoParaZ(const Vector3& vn) {
  Vector3 a(0.0f, 0.0f, 1.0f);
  Vector3 b(vn.x, vn.y, vn.z);
  b.normalize();
  Vector3 axis = a.cross(b);
  if (fabs(axis.length()) == 0.0f) {
    return Matrix4();
  }
  axis.normalize();
  float cosang = a.dot(b);
  // aparentemente, nao precisa resolver o caso > PI porque o cross sempre vai retornar o menor angulo entre eles..
  return Matrix4().rotate(acosf(cosang) * RAD_PARA_GRAUS, axis);
}
#endif

enum Vbos {
  VBO_CUBO = 0,
  VBO_ESFERA,
  VBO_PIRAMIDE,
  VBO_CILINDRO,
  VBO_DISCO,
  VBO_RETANGULO,
  VBO_TRIANGULO,
  VBO_CONE,
  VBO_CONE_FECHADO,
  VBO_CILINDRO_FECHADO,
  VBO_HEMISFERIO,
  VBO_NUM
};
static std::vector<gl::VboGravado> g_vbos;

}  // namespace

namespace interno {
void IniciaVbos() {
  std::vector<VboNaoGravado> vbos_nao_gravados(VBO_NUM);
  // Cubo.
  {
    auto& vbo = vbos_nao_gravados[VBO_CUBO];
    vbo = gl::VboCuboSolido(1.0f);
    vbo.Nomeia("cubo unitario");
  }

  // Esfera.
  {
    auto& vbo = vbos_nao_gravados[VBO_ESFERA];
    vbo = gl::VboEsferaSolida(0.5f, 24, 12);
    vbo.Nomeia("Esfera unitaria");
  }

  // Hemisferio.
  {
    auto& vbo = vbos_nao_gravados[VBO_HEMISFERIO];
    vbo = gl::VboHemisferioSolido(0.5f, 24, 12);
    vbo.Nomeia("Hemisferio unitario");
  }

  // Piramide.
  {
    auto& vbo = vbos_nao_gravados[VBO_PIRAMIDE];
    vbo = gl::VboPiramideSolida(1.0f, 1.0f);
    vbo.Nomeia("Piramide");
  }

  // Cilindro.
  {
    auto& vbo = vbos_nao_gravados[VBO_CILINDRO];
    vbo = gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12, 6);
    vbo.Nomeia("Cilindro");
  }
  // Cilindro fechado.
  {
    auto& vbo = vbos_nao_gravados[VBO_CILINDRO_FECHADO];
    vbo = gl::VboCilindroSolido(0.5f  /*raio*/, 1.0f  /*altura*/, 12, 6);
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
      vbo.Concatena(vbo_disco);
    }
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Translada(0.0f, 0.0f, 1.0f);
      vbo.Concatena(vbo_disco);
    }
    vbo.Nomeia("CilindroFechado");
  }

  // Disco.
  {
    auto& vbo = vbos_nao_gravados[VBO_DISCO];
    vbo = gl::VboDisco(0.5f  /*raio*/, 12);
    vbo.Nomeia("Disco");
  }

  // Retangulo.
  {
    auto& vbo = vbos_nao_gravados[VBO_RETANGULO];
    vbo = gl::VboRetangulo(1.0f);
    vbo.Nomeia("Retangulo");
  }

  // Triangulo.
  {
    auto& vbo = vbos_nao_gravados[VBO_TRIANGULO];
    vbo = gl::VboTriangulo(1.0f);
    vbo.Nomeia("Triangulo");
  }

  // Cone.
  {
    auto& vbo = vbos_nao_gravados[VBO_CONE];
    vbo = gl::VboConeSolido(0.5f, 1.0f, 12, 6);
    vbo.Nomeia("Cone");
  }

  // Cone fechado.
  {
    auto& vbo = vbos_nao_gravados[VBO_CONE_FECHADO];
    vbo = gl::VboConeSolido(0.5f, 1.0f, 12, 6);
    {
      gl::VboNaoGravado vbo_disco = gl::VboDisco(0.5f  /*raio*/, 12  /*fatias*/);
      vbo_disco.Escala(-1.0f, 1.0f, -1.0f);
      vbo.Concatena(vbo_disco);
    }
    vbo.Nomeia("ConeFechado");
  }
  g_vbos.resize(VBO_NUM);
  for (int i = 0; i < VBO_NUM; ++i) {
    g_vbos[i].Grava(GL_TRIANGLES, vbos_nao_gravados[i]);
  }
}
}  // namespace interno

//----------------
// VbosNaoGravados
//----------------
void VbosNaoGravados::AtribuiMatrizModelagem(const Matrix4& m) {
  for (auto& vbo : vbos_) {
    vbo.AtribuiMatrizModelagem(m);
  }
}

void VbosNaoGravados::Multiplica(const Matrix4& m) {
  for (auto& vbo : vbos_) {
    vbo.Multiplica(m);
  }
}

void VbosNaoGravados::AtribuiCor(float r, float g, float b, float a) {
  for (auto& vbo : vbos_) {
    vbo.AtribuiCor(r, g, b, a);
  }
}

void VbosNaoGravados::MesclaCores(float r, float g, float b, float a) {
  for (auto& vbo : vbos_) {
    vbo.MesclaCores(r, g, b, a);
  }
}

void VbosNaoGravados::Concatena(const VboNaoGravado& rhs) {
  if (vbos_.empty()) {
    vbos_.emplace_back(rhs);
  } else {
    try {
      vbos_.back().Concatena(rhs);
    } catch (...) {
      vbos_.push_back(rhs);
    }
  }
}

void VbosNaoGravados::Concatena(VboNaoGravado* rhs) {
  if (vbos_.empty()) {
    vbos_.resize(1);
    vbos_[0] = std::move(*rhs);
  } else {
    VboNaoGravado dummy = std::move(*rhs);
    try {
      // Dummy apenas para retornar rhs vazio.
      vbos_.back().Concatena(dummy);
    } catch (...) {
      vbos_.resize(vbos_.size() + 1);
      vbos_.back() = std::move(dummy);
    }
  }
}

void VbosNaoGravados::Concatena(VbosNaoGravados* rhs) {
  for (auto& vbo : rhs->vbos_) {
    Concatena(&vbo);
  }
}

void VbosNaoGravados::Desenha(GLenum modo) const {
  AtualizaMatrizes();
  for (const auto& vbo : vbos_) {
    DesenhaVboNaoGravado(vbo, modo, /*atualiza_matrizes=*/false);
  }
}

std::string VbosNaoGravados::ParaString(bool completo) const {
  std::string ret;
  int i = 0;
  for (const auto& vbo : vbos_) {
    ret += StringPrintf("%d) %s\n", i++, vbo.ParaString(completo).c_str());
  }
  return ret;
}

//-------------
// VbosGravados
//-------------
void VbosGravados::Grava(const VbosNaoGravados& vbos_nao_gravados) {
  //LOG(INFO) << "gravando vbos de " << nome_ << " atualmente com " << vbos_.size() << " vbos";
  vbos_.resize(vbos_nao_gravados.vbos_.size());
  for (unsigned int i = 0; i < vbos_nao_gravados.vbos_.size(); ++i) {
    //vbos_[i].Desgrava();
    vbos_[i].Grava(GL_TRIANGLES, vbos_nao_gravados.vbos_[i]);
  }
  if (nome_.empty() && !vbos_nao_gravados.vbos_.empty() && !vbos_nao_gravados.vbos_[0].nome().empty()) {
    Nomeia(vbos_nao_gravados.vbos_[0].nome());
  } else if (!nome_.empty()) {
    Nomeia(nome_); // para pegar os novos vbos.
  }
}

void VbosGravados::AtualizaMatrizes(const Matrix4& matriz_modelagem) {
  for (auto& vbo : vbos_) {
    vbo.AtualizaMatrizes(matriz_modelagem);
  }
}

void VbosGravados::Desgrava() {
  //LOG(INFO) << "desgravando vbos de " << nome_;
  vbos_.clear();
}

void VbosGravados::Nomeia(const std::string& nome) {
  nome_ = nome;
  int i = 0;
  for (auto& vbo : vbos_) {
    vbo.Nomeia(StringPrintf("%s %i", nome.c_str(), i));
    ++i;
  }
}

void VbosGravados::Desenha() const {
  //LOG(INFO) << "----- Desenhando: " << nome_ << " no shader: " << interno::BuscaShader().nome;
  if (!vbos_.empty() && !vbos_[0].tem_matriz_modelagem()) {
    ::gl::AtualizaMatrizes();
  }
  for (const auto& vbo : vbos_) {
    DesenhaVboGravado(vbo, /*atualiza_matrizes=*/false);
  }
  //LOG(INFO) << "----- Fim Desenhando " << nome_ << " no shader: " << interno::BuscaShader().nome;
}

//--------------
// VboNaoGravado
//--------------
bool VboNaoGravado::tem_matriz_modelagem() const {
  return false && matriz_modelagem_.has_value();
}

bool VboNaoGravado::tem_matriz_normal() const {
  return tem_matriz_modelagem() && tem_normais();
}

void VboNaoGravado::Escala(GLfloat x, GLfloat y, GLfloat z) {
  if (num_dimensoes_ == 2) {
    for (unsigned int i = 0; i < coordenadas_.size(); i += 2) {
      coordenadas_[i] *= x;
      coordenadas_[i + 1] *= y;
      if (num_dimensoes_ == 3) {
        coordenadas_[i + 2] *= z;
      }
    }
  } else if (num_dimensoes_ == 3) {
    Matrix4 m;
    m.scale(x, y, z);
    for (unsigned int i = 0; i < coordenadas_.size(); i += 3) {
      Vector4 c(coordenadas_[i], coordenadas_[i+1], coordenadas_[i+2], 1.0f);
      c = m * c;
      coordenadas_[i]   = c[0];
      coordenadas_[i+1] = c[1];
      coordenadas_[i+2] = c[2];
    }
    m.invert().transpose();
    auto TransformaNormalTangente = [&m] (unsigned int tam, float* c) {
      Vector4 vc;
      for (unsigned int i = 0; i < tam; i += 3) {
        vc.set(c[i], c[i+1], c[i+2], 1.0f);
        vc = m * vc;
        vc.normalize();
        c[i]   = vc[0];
        c[i+1] = vc[1];
        c[i+2] = vc[2];
      }
    };
    TransformaNormalTangente(normais_.size(), &(*normais_.begin()));
    TransformaNormalTangente(tangentes_.size(), &(*tangentes_.begin()));
  }
}

void VboNaoGravado::Translada(GLfloat x, GLfloat y, GLfloat z) {
  for (unsigned int i = 0; i < coordenadas_.size(); i += num_dimensoes_) {
    coordenadas_[i] += x;
    coordenadas_[i + 1] += y;
    if (num_dimensoes_ == 3) {
      coordenadas_[i + 2] += z;
    }
  }
}

void VboNaoGravado::Multiplica(const Matrix4& m) {
  if (num_dimensoes_ != 3) {
    LOG(ERROR) << "Operacao invalida, objeto nao tem tres dimensoes: " << num_dimensoes_;
    return;
  }
  // Acesso eficiente.
  float* coordenadas = &(*coordenadas_.begin());
  Vector4 v;
  for (unsigned int i = 0; i < coordenadas_.size(); i += 3) {
    v.set(coordenadas[i], coordenadas[i + 1], coordenadas[i + 2], 1.0f);
    v = m * v;
    coordenadas[i] = v.x;
    coordenadas[i + 1] = v.y;
    coordenadas[i + 2] = v.z;
  }
  Matrix4 mn = m;
  mn.invert().transpose();
  auto TransformaNormalTangente = [&mn](unsigned int tam, float* c) {
    Vector4 vc;
    for (unsigned int i = 0; i < tam; i += 3) {
      vc.set(c[i], c[i+1], c[i+2], 1.0f);
      vc = mn * vc;
      vc.normalize();
      c[i]   = vc[0];
      c[i+1] = vc[1];
      c[i+2] = vc[2];
    }
  };
  TransformaNormalTangente(normais_.size(), &(*normais_.begin()));
  TransformaNormalTangente(tangentes_.size(), &(*tangentes_.begin()));
}

void VboNaoGravado::RodaX(GLfloat angulo_graus) {
  Matrix4 m;
  m.rotateX(angulo_graus);
  float* coordenadas = &(*coordenadas_.begin());
  for (unsigned int i = 0; i < coordenadas_.size(); i += 3) {
    Vector4 c(0.0f, coordenadas[i + 1], coordenadas[i + 2], 1.0f);
    c = m * c;
    coordenadas[i + 1] = c[1];
    coordenadas[i + 2] = c[2];
  }
  auto TransformaNormalTangente = [&m](unsigned int tam, float* c) {
    Vector4 vc;
    for (unsigned int i = 0; i < tam; i += 3) {
      vc.set(0.0f, c[i + 1], c[i + 2], 1.0f);
      vc = m * vc;
      c[i + 1] = vc[1];
      c[i + 2] = vc[2];
    }
  };
  // Mesma transformada.
  TransformaNormalTangente(normais_.size(),  &(*normais_.begin()));
  TransformaNormalTangente(tangentes_.size(),  &(*tangentes_.begin()));
}

void VboNaoGravado::RodaY(GLfloat angulo_graus) {
  Matrix4 m;
  m.rotateY(angulo_graus);
  float* coordenadas = &(*coordenadas_.begin());
  for (unsigned int i = 0; i < coordenadas_.size(); i += 3) {
    Vector4 c(coordenadas[i], 0.0f, coordenadas[i + 2], 1.0f);
    c = m * c;
    coordenadas[i]     = c[0];
    coordenadas[i + 2] = c[2];
  }
  // Mesma transformada.
  auto TransformaNormalTangente = [&m](unsigned int tam, float* c) {
    Vector4 vc;
    for (unsigned int i = 0; i < tam; i += 3) {
      vc.set(c[i], 0.0f, c[i+2], 1.0f);
      vc = m * vc;
      c[i]   = vc[0];
      c[i+2] = vc[2];
    }
  };
  TransformaNormalTangente(normais_.size(), &(*normais_.begin()));
  TransformaNormalTangente(tangentes_.size(), &(*tangentes_.begin()));
}

void VboNaoGravado::RodaZ(GLfloat angulo_graus) {
  Matrix4 m;
  m.rotateZ(angulo_graus);
  float* coordenadas = &(*coordenadas_.begin());
  for (unsigned int i = 0; i < coordenadas_.size(); i += num_dimensoes_) {
    Vector4 c(coordenadas[i], coordenadas[i + 1], 0.0f, 1.0f);
    c = m * c;
    coordenadas[i]     = c[0];
    coordenadas[i + 1] = c[1];
  }
  // Mesma transformada.
  auto TransformaNormalTangente = [&m](unsigned int tam, float* c) {
    Vector4 vc;
    for (unsigned int i = 0; i < tam; i += 3) {
      vc.set(c[i], c[i + 1], c[i + 2], 1.0f);
      vc = m * vc;
      c[i]     = vc[0];
      c[i + 1] = vc[1];
    }
  };
  TransformaNormalTangente(normais_.size(), &(*normais_.begin()));
  TransformaNormalTangente(tangentes_.size(), &(*tangentes_.begin()));
}

void VboNaoGravado::Concatena(const VboNaoGravado& rhs) {
  if (num_dimensoes_ == 0) {
    *this = rhs;
    return;
  }
  if (rhs.num_dimensoes_ == 0) {
    //LOG(WARNING) << "ignorando rhs com 0 dimensoes";
    return;
  }
  if (num_dimensoes_ != rhs.num_dimensoes_) {
    throw std::logic_error(std::string("Nao eh possivel concatenar, objetos incompativeis: ") + nome() + " e " + rhs.nome());
  }
  // Coordenadas do primeiro indice apos o ultimo, onde serao inseridos os novos.
  const unsigned short num_coordenadas_inicial = coordenadas_.size() / num_dimensoes_;
  if (num_coordenadas_inicial + (rhs.coordenadas_.size() / num_dimensoes_) >
          std::numeric_limits<unsigned short>::max()) {
    throw std::logic_error("Nao eh possivel concatenar, limite de tamanho alcancado");
  }
  coordenadas_.reserve(coordenadas_.size() + rhs.coordenadas_.size());
  coordenadas_.insert(coordenadas_.end(), rhs.coordenadas_.begin(), rhs.coordenadas_.end());
  normais_.reserve(normais_.size() + rhs.normais_.size());
  normais_.insert(normais_.end(), rhs.normais_.begin(), rhs.normais_.end());
  tangentes_.reserve(tangentes_.size() + rhs.tangentes_.size());
  tangentes_.insert(tangentes_.end(), rhs.tangentes_.begin(), rhs.tangentes_.end());

  auto indices_size_antes = indices_.size();
  indices_.resize(indices_.size() + rhs.indices_.size());
  unsigned short* indices = &(*indices_.begin());
  const unsigned short* rhs_indices = &(*rhs.indices_.begin());
  for (unsigned int i = 0; i < rhs.indices_.size(); ++i) {
    indices[indices_size_antes + i] = rhs_indices[i] + num_coordenadas_inicial;
  }
  cores_.reserve(cores_.size() + rhs.cores_.size());
  cores_.insert(cores_.end(), rhs.cores_.begin(), rhs.cores_.end());
  if (tem_texturas()) {
    if (rhs.tem_texturas()) {
      texturas_.reserve(texturas_.size() + rhs.texturas_.size());
      texturas_.insert(texturas_.end(), rhs.texturas_.begin(), rhs.texturas_.end());
    } else {
      //LOG(WARNING) << "Limpando texturas ao concatenar porque rhs nao tem.";
      texturas_.clear();
    }
  } else if (rhs.tem_texturas()) {
    //LOG(WARNING) << "Ignorando texturas ao concatenar porque lhs nao tem.";
  }
#if DEBUG
  Nomeia(nome_ + "+" + rhs.nome_);
#endif
}

std::vector<float> VboNaoGravado::GeraBufferUnico(
    unsigned int* deslocamento_normais,
    unsigned int* deslocamento_tangentes,
    unsigned int* deslocamento_cores,
    unsigned int* deslocamento_texturas,
    unsigned int* deslocamento_matriz_modelagem,
    unsigned int* deslocamento_matriz_normal) const {
  std::vector<float> buffer_unico;
  buffer_unico.clear();
  buffer_unico.insert(buffer_unico.end(), coordenadas_.begin(), coordenadas_.end());
  buffer_unico.insert(buffer_unico.end(), normais_.begin(), normais_.end());
  buffer_unico.insert(buffer_unico.end(), tangentes_.begin(), tangentes_.end());
  buffer_unico.insert(buffer_unico.end(), cores_.begin(), cores_.end());
  buffer_unico.insert(buffer_unico.end(), texturas_.begin(), texturas_.end());
  if (tem_matriz_modelagem()) {
    const float* mm = matriz_modelagem_->get();
    buffer_unico.insert(buffer_unico.end(), mm, mm + 16);
    if (tem_normais()) {
      Matrix3 matriz_normal = interno::ExtraiMatrizNormal(*matriz_modelagem_);
      const float* mn = matriz_normal.get();
      buffer_unico.insert(buffer_unico.end(), mn, mn + 9);
    }
  }
  unsigned int pos_final = coordenadas_.size() * sizeof(float);
  if (tem_normais()) {
    *deslocamento_normais = pos_final;
    pos_final += normais_.size() * sizeof(float);
  }
  if (tem_tangentes()) {
    *deslocamento_tangentes = pos_final;
    pos_final += tangentes_.size() * sizeof(float);
  }
  if (tem_cores_) {
    *deslocamento_cores = pos_final;
    pos_final += cores_.size() * sizeof(float);
  }
  if (tem_texturas()) {
    *deslocamento_texturas = pos_final;
    pos_final += texturas_.size() * sizeof(float);
  }
  if (tem_matriz_modelagem()) {
    //LOG(INFO) << "gerando buffer unico com matriz modelagem";
    *deslocamento_matriz_modelagem = pos_final;
    pos_final += (16 * sizeof(float));
    if (tem_normais()) {
      *deslocamento_matriz_normal = pos_final;
      pos_final += (9 * sizeof(float));
    }
  }
  return buffer_unico;
}

void VboNaoGravado::AtribuiIndices(const unsigned short* dados, unsigned int num_indices) {
  indices_.clear();
  indices_.insert(indices_.end(), dados, dados + num_indices);
}

void VboNaoGravado::AtribuiIndices(std::vector<unsigned short>* dados) {
  indices_.swap(*dados);
}

void VboNaoGravado::AtribuiCoordenadas(unsigned short num_dimensoes, const float* dados, unsigned int num_coordenadas) {
  if ((num_coordenadas / num_dimensoes) > std::numeric_limits<unsigned short>::max()) {
    LOG(WARNING) << "Nao eh possivel indexar mais que " <<  std::numeric_limits<unsigned short>::max() << " coordenadas";
  }
  coordenadas_.clear();
  coordenadas_.insert(coordenadas_.end(), dados, dados + num_coordenadas);
  num_dimensoes_ = num_dimensoes;
}

void VboNaoGravado::AtribuiCoordenadas(unsigned short num_dimensoes, std::vector<float>* dados) {
  int num_coordenadas = dados->size() / num_dimensoes;
  if ((num_coordenadas / num_dimensoes) > std::numeric_limits<unsigned short>::max()) {
    LOG(WARNING) << "Nao eh possivel indexar mais que " <<  std::numeric_limits<unsigned short>::max() << " coordenadas";
  }
  coordenadas_.swap(*dados);
  num_dimensoes_ = num_dimensoes;
}

void VboNaoGravado::AtribuiCoordenadas(unsigned short num_dimensoes, const std::vector<short>& dados) {
  int num_coordenadas = dados.size() / num_dimensoes;
  if ((num_coordenadas / num_dimensoes) > std::numeric_limits<unsigned short>::max()) {
    LOG(WARNING) << "Nao eh possivel indexar mais que " <<  std::numeric_limits<unsigned short>::max() << " coordenadas";
  }
  coordenadas_.resize(dados.size());
  float* c = &coordenadas_[0];
  for (unsigned int i = 0; i < dados.size(); ++i) {
    c[i] = static_cast<float>(dados[i]);
  }
  num_dimensoes_ = num_dimensoes;
}


void VboNaoGravado::AtribuiNormais(const float* dados) {
  normais_.clear();
  normais_.insert(normais_.end(), dados, dados + coordenadas_.size());
}

void VboNaoGravado::AtribuiNormais(std::vector<float>* dados) {
  normais_.swap(*dados);
}

void VboNaoGravado::AtribuiTangentes(const float* dados) {
  tangentes_.clear();
  tangentes_.insert(tangentes_.end(), dados, dados + coordenadas_.size());
  if (tangentes_.size() != normais_.size()) {
    tangentes_.clear();
  }
}

void VboNaoGravado::AtribuiTangentes(std::vector<float>* dados) {
  tangentes_.swap(*dados);
  if (tangentes_.size() != normais_.size()) {
    tangentes_.clear();
  }
}

void VboNaoGravado::AtribuiMatrizModelagem(const Matrix4& matriz_modelagem) {
  matriz_modelagem_ = matriz_modelagem;
  if (tem_normais()) {
    matriz_normal_ = interno::ExtraiMatrizNormal(matriz_modelagem);
  }
}

void VboNaoGravado::AtribuiTexturas(const float* dados) {
  texturas_.clear();
  texturas_.insert(texturas_.end(), dados, dados + (coordenadas_.size() * 2) / num_dimensoes_ );
}

void VboNaoGravado::AtribuiTexturas(std::vector<float>* dados) {
  texturas_.swap(*dados);
}

void VboNaoGravado::AtribuiCor(float r, float g, float b, float a) {
  cores_.clear();
  int num_vertices = coordenadas_.size() / num_dimensoes_;
  for (int i = 0; i < num_vertices; ++i) {
    cores_.push_back(r);
    cores_.push_back(g);
    cores_.push_back(b);
    cores_.push_back(a);
  }
  tem_cores_ = true;
}

void VboNaoGravado::AtribuiCores(const float* cores) {
  cores_.clear();
  cores_.insert(cores_.end(), cores, cores + (coordenadas_.size() * 4) / num_dimensoes_);
  tem_cores_ = true;
}

void VboNaoGravado::MesclaCores(float r, float g, float b, float a) {
  if (!tem_cores_) {
    AtribuiCor(r, g, b, a);
  } else {
    for (unsigned int i = 0; i < cores_.size(); i += 4) {
      cores_[i] *= r;
      cores_[i + 1] *= g;
      cores_[i + 2] *= b;
      cores_[i + 3] *= a;
    }
  }
}

VboNaoGravado VboNaoGravado::ExtraiVboNormais() const {
  if (num_dimensoes_ != 3) {
    throw std::logic_error("extracao de normais nao suportado para n != 3");
  }
  if (normais_.empty()) {
    throw std::logic_error("extracao de normais para objeto sem normais");
  }
  VboNaoGravado vbo;
  std::vector<float> cs;
  for (unsigned int i = 0; i < coordenadas_.size(); i += 3) {
    cs.push_back(coordenadas_[i]);
    cs.push_back(coordenadas_[i+1]);
    cs.push_back(coordenadas_[i+2]);
    cs.push_back(coordenadas_[i]   + normais_[i]);
    cs.push_back(coordenadas_[i+1] + normais_[i+1]);
    cs.push_back(coordenadas_[i+2] + normais_[i+2]);
  }
  vbo.AtribuiCoordenadas(3, cs.data(), cs.size());
  std::vector<unsigned short> is;
  for (unsigned int i = 0; i < indices_.size(); ++i) {
    is.push_back(indices_[i] * 2);
    is.push_back(indices_[i] * 2 + 1);
  }
  vbo.AtribuiIndices(is.data(), is.size());
  return vbo;
}

std::string VboNaoGravado::ParaString(bool completo) const {
#if WIN32 || ANDROID
  return std::string("vbo: ") + nome_;
#else
  std::string coords;
  if (completo) {
    for (unsigned int i = 0; i < coordenadas_.size(); ++i) {
      coords += StringPrintf("%f", coordenadas_[i]);
      if ((i > 0) && i % NumDimensoes() == 0) {
        coords += ";";
      }
      coords += " ";
    }
  }
  return std::string("vbo: ") + nome_ + ", dimensoes: " + std::to_string(NumDimensoes()) +
         ", num indices: " + std::to_string(indices_.size()) +
         ", cores_size: " + std::to_string(tem_cores_ ? cores_.size() : 0) +
         ", normais_size: " + std::to_string(normais_.size()) +
         ", tangentes_size: " + std::to_string(tangentes_.size()) +
         ", texturas_size: " + std::to_string(texturas_.size()) +
         ", coordenadas_size: " + std::to_string(coordenadas_.size()) +
         coords;
#endif
}

namespace {
void HabilitaAtributosVertice(
    int num_vertices, int num_dimensoes, const void* dados,
    bool &tem_normais, const void* normais, int d_normais,
    bool &tem_tangentes, const void* tangentes, int d_tangentes,
    bool &tem_texturas, const void* texturas, int d_texturas,
    bool &tem_cores, const void* cores, int d_cores,
    bool &tem_matriz_modelagem, const void* matriz_modelagem, int d_modelagem,
    bool &tem_matriz_normal, const void* matriz_normal, int d_normal,
    bool atualiza_matrizes) {
  const auto& shader = interno::BuscaShader();
  V_ERRO("HabilitaAtributosVertice: antes");
  if (gl::HabilitaVetorAtributosVerticePorTipo(ATR_VERTEX_ARRAY)) {
    gl::PonteiroVertices(num_dimensoes, GL_FLOAT, 0, (void*)dados);
  } else {
    // Aqui a vaca foi pro brejo.
    LOG(ERROR) << "nao ha indice para vertices";
    return;
  }
  if (tem_normais && gl::HabilitaVetorAtributosVerticePorTipo(ATR_NORMAL_ARRAY)) {
    gl::PonteiroNormais(GL_FLOAT, static_cast<const char*>(normais) + d_normais);
  } else {
    if (tem_normais && shader.atr_gltab_normal != -1) {
      LOG(WARNING) << "nao consegui gravar normais. Shader: " << shader.nome;
    }
    tem_normais = false;
  }
  if (tem_tangentes && gl::HabilitaVetorAtributosVerticePorTipo(ATR_TANGENT_ARRAY)) {
    gl::PonteiroTangentes(GL_FLOAT, static_cast<const char*>(tangentes) + d_tangentes);
  } else {
    if (tem_tangentes && shader.atr_gltab_tangente != -1) {
      LOG(WARNING) << "nao consegui gravar tangentes. Shader: " << shader.nome;
    }
    tem_tangentes = false;
  }
  if (tem_texturas && gl::HabilitaVetorAtributosVerticePorTipo(ATR_TEXTURE_COORD_ARRAY)) {
    gl::PonteiroVerticesTexturas(2, GL_FLOAT, 0, static_cast<const char*>(texturas) + d_texturas);
  } else {
    if (tem_texturas && shader.atr_gltab_texel != -1) {
      LOG(WARNING) << "nao consegui gravar texturas.";
    }
    tem_texturas = false;
  }
  V_ERRO("DesenhaVBO: um quarto");
  if (tem_cores && !interno::BuscaContexto()->UsarSelecaoPorCor() && gl::HabilitaVetorAtributosVerticePorTipo(ATR_COLOR_ARRAY)) {
    gl::PonteiroCores(4, 0, static_cast<const char*>(cores) + d_cores);
  } else {
    if (tem_cores && shader.atr_gltab_cor != -1) {
      LOG(WARNING) << "nao consegui gravar cores.";
    }
    tem_cores = false;
  }

  V_ERRO("DesenhaVBO: um quarto e meio");
  if (tem_matriz_modelagem && gl::HabilitaVetorAtributosVerticePorTipo(ATR_MODEL_MATRIX_ARRAY)) {
    gl::PonteiroMatrizModelagem(
        static_cast<const char*>(matriz_modelagem) + d_modelagem);
  } else {
    if (tem_matriz_modelagem && shader.atr_gltab_matriz_modelagem != -1) {
      LOG(WARNING) << "nao consegui gravar matriz modelagem. Shader " << shader.nome;
    }
    tem_matriz_modelagem = false;
  }

  V_ERRO("DesenhaVBO: um quarto e tres quartos");
  if (tem_matriz_normal && gl::HabilitaVetorAtributosVerticePorTipo(ATR_NORMAL_MATRIX_ARRAY)) {
    gl::PonteiroMatrizNormal(
        static_cast<const char*>(matriz_normal) + d_normal);
  } else {
    if (tem_matriz_normal && shader.atr_gltab_matriz_normal != -1) {
      LOG(WARNING) << "nao consegui gravar matriz normal. Shader " << shader.nome;
    }
    tem_matriz_normal = false;
  }

  V_ERRO("HabilitaAtributosVertice: meio");
  V_ERRO(StringPrintf("DesenhaVBO: mei ponteiro vertices num dimensoes: %d", num_dimensoes));

  if (atualiza_matrizes) {
    gl::AtualizaMatrizes();
  }
}

void DesabilitaAtributosVertice(
                bool tem_normais,
                bool tem_tangentes,
                bool tem_texturas,
                bool tem_cores,
                bool tem_matriz_modelagem,
                bool tem_matriz_normal) {
  V_ERRO("DesenhaVBO: mei elementos");
  gl::DesabilitaVetorAtributosVerticePorTipo(ATR_VERTEX_ARRAY);
  if (tem_normais) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_NORMAL_ARRAY);
  }
  if (tem_tangentes) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_TANGENT_ARRAY);
  }
  V_ERRO("DesenhaVBO: tres quartos");
  if (tem_cores) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_COLOR_ARRAY);
  }
  if (tem_texturas) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_TEXTURE_COORD_ARRAY);
  }
  if (tem_matriz_modelagem) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_MODEL_MATRIX_ARRAY);
  }
  if (tem_matriz_normal) {
    gl::DesabilitaVetorAtributosVerticePorTipo(ATR_NORMAL_MATRIX_ARRAY);
  }
  V_ERRO("DesenhaVBO: depois");
}

void DesenhaElementosComAtributos(
    GLenum modo,
    int num_vertices, int num_dimensoes, const void* indices, const void* dados,
    bool tem_normais, const void* normais, int d_normais,
    bool tem_tangentes, const void* tangentes, int d_tangentes,
    bool tem_texturas, const void* texturas, int d_texturas,
    bool tem_cores, const void* cores, int d_cores,
    bool tem_matriz_modelagem, const void* matriz_modelagem, int d_matriz_modelagem,
    bool tem_matriz_normal, const void* matriz_normal, int d_matriz_normal,
    bool atualiza_matrizes) {
  HabilitaAtributosVertice(
      num_vertices, num_dimensoes, dados,
      tem_normais, normais, d_normais,
      tem_tangentes, tangentes, d_tangentes,
      tem_texturas, texturas, d_texturas,
      tem_cores, cores, d_cores,
      tem_matriz_modelagem, matriz_modelagem, d_matriz_modelagem,
      tem_matriz_normal, matriz_normal, d_matriz_normal,
      atualiza_matrizes);
  gl::DesenhaElementos(modo, num_vertices, GL_UNSIGNED_SHORT, (void*)indices);
  DesabilitaAtributosVertice(tem_normais, tem_tangentes, tem_texturas, tem_cores, tem_matriz_modelagem, tem_matriz_normal);
}

void ConfiguraVao(GLenum modo, const VboGravado& vbo, int shader) {
  LigacaoComObjetoVertices(vbo.Vao());
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, vbo.nome_coordenadas());
  bool tem_normais = vbo.tem_normais();
  bool tem_tangentes = vbo.tem_tangentes();
  bool tem_texturas = vbo.tem_texturas();
  bool tem_matriz_modelagem = vbo.tem_matriz_modelagem();
  bool tem_matriz_normal = vbo.tem_matriz_normal();
  // Isso é necessario pq o shader de picking tem cor. Entao, mesmo o objeto tendo cor, temos que força-lo a nao ter.
  bool tem_cores = shader == gl::TSH_PICKING ? false : vbo.tem_cores();
  //LOG(INFO)
  //    << "tem_normais: " << tem_normais << ", desloc: " << vbo.DeslocamentoNormais()
  //    << ", tem_tangentes: " << tem_tangentes << ", desloc: " << vbo.DeslocamentoTangentes()
  //    << ", tem cores: " << tem_cores << ", desloc: " << vbo.DeslocamentoCores()
  //    << ", tem_texturas: " << tem_texturas << ", desloc: " << vbo.DeslocamentoTexturas()
  //    << ", num dimensoes: " << vbo.NumDimensoes()
  //    << ", modo: " << vbo.Modo() << ", tamanho buffer: " << (vbo.BufferUnico().size() * sizeof(float))
  //    << ", num vertices: " << vbo.NumVertices()
  //    << ", nome: " << vbo.nome();
  HabilitaAtributosVertice(
      vbo.NumVertices(), vbo.NumDimensoes(), nullptr,
      tem_normais, nullptr, vbo.DeslocamentoNormais(),
      tem_tangentes, nullptr, vbo.DeslocamentoTangentes(),
      tem_texturas, nullptr, vbo.DeslocamentoTexturas(),
      tem_cores, nullptr, vbo.DeslocamentoCores(),
      tem_matriz_modelagem, nullptr, vbo.DeslocamentoMatrizModelagem(),
      tem_matriz_normal, nullptr, vbo.DeslocamentoMatrizNormal(),
      /*atualiza_matrizes=*/false);
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.nome_indices());
  LigacaoComObjetoVertices(0);
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  DesabilitaAtributosVertice(tem_normais, tem_tangentes, tem_texturas, tem_cores, tem_matriz_modelagem, tem_matriz_normal);
}

}  // namespace

//-----------
// VboGravado
//-----------
GLuint VboGravado::Vao() const {
  return vao_por_shader_[TipoShaderCorrente()];
}

void VboGravado::AtualizaMatrizes(const Matrix4& matriz_modelagem) {
  if (!gravado_) {
    LOG(ERROR) << "tentando atualizar a matriz de VBO nao gravado";
    return;
  }
  if (!tem_matriz_modelagem_) {
    LOG(ERROR) << "tentando atualizar a matriz de VBO sem matriz de modelagem";
    return;
  }
  if ((DeslocamentoMatrizModelagem() + 16 * sizeof(float)) > (buffer_unico_.size() * sizeof(float))) {
    LOG(ERROR) << "matriz de modelagem vai explodir o buffer unico";
    return;
  }
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_coordenadas_);
  // Aqui é so pra manter o buffer unico sincronizado.
  memcpy(
      &reinterpret_cast<char*>(buffer_unico_.data())[DeslocamentoMatrizModelagem()],
      matriz_modelagem.get(), 16 * sizeof(float));
  V_ERRO("na ligacao com buffer");
  gl::BufferizaSubDados(
      GL_ARRAY_BUFFER, DeslocamentoMatrizModelagem(),
      sizeof(GL_FLOAT) * 16,
      matriz_modelagem.get());
  V_ERRO("ao bufferizar");
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);

  if (!tem_matriz_normal()) {
    return;
  }
  if ((DeslocamentoMatrizNormal() + 9 * sizeof(float)) > (buffer_unico_.size() * sizeof(float))) {
    LOG(ERROR) << "matriz de normal vai explodir o buffer unico";
    return;
  }
  Matrix4 matriz_normal = matriz_modelagem.get();
  matriz_normal.invert().transpose();
  const float* mn = matriz_normal.get();
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_coordenadas_);
  // Aqui é so pra manter o buffer unico sincronizado.
  memcpy(
      &reinterpret_cast<char*>(buffer_unico_.data())[DeslocamentoMatrizNormal()],
      mn, 9 * sizeof(float));
  V_ERRO("na ligacao com buffer");
  gl::BufferizaSubDados(
      GL_ARRAY_BUFFER, DeslocamentoMatrizNormal(),
      sizeof(GL_FLOAT) * 9,
      mn);
  V_ERRO("ao bufferizar");
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
}

void VboGravado::Grava(GLuint modo, const VboNaoGravado& vbo_nao_gravado) {
  V_ERRO("antes tudo gravar");
  V_ERRO("depois desgravar");
  if (nome_.empty()) {
    nome_ = vbo_nao_gravado.nome();
  }
  modo_ = modo;

  // Gera o buffer se ja nao for gravado.
  bool usar_buffer_sub_data = gravado_;
  if (!gravado_) {
    nome_coordenadas_ = 0;
    gl::GeraBuffers(1, &nome_coordenadas_);
    V_ERRO("ao gerar buffer coordenadas");
    if (nome_coordenadas_ == 0) {
      LOG(ERROR) << "ERRO GRAFICO SERIO!!!!!!!!: glGenBuffers gerou valor 0. Provavelmente aplicação está sem o contexto grafico.";
      return;
    }
  }
  V_ERRO("ao gerar buffer coordenadas");
  // Associa coordenadas com ARRAY_BUFFER.
  deslocamento_normais_ = -1;
  deslocamento_tangentes_ = -1;
  deslocamento_cores_ = -1;
  deslocamento_texturas_ = -1;
  deslocamento_matriz_modelagem_ = -1;
  deslocamento_matriz_normal_ = -1;
  auto tam_antes = buffer_unico_.size();
  buffer_unico_ = vbo_nao_gravado.GeraBufferUnico(
      &deslocamento_normais_,
      &deslocamento_tangentes_,
      &deslocamento_cores_,
      &deslocamento_texturas_,
      &deslocamento_matriz_modelagem_,
      &deslocamento_matriz_normal_);
  if (tam_antes != buffer_unico_.size()) {
    usar_buffer_sub_data = false;
  }
  num_dimensoes_ = vbo_nao_gravado.NumDimensoes();
  tem_normais_ = (deslocamento_normais_ != static_cast<unsigned int>(-1));
  tem_tangentes_ = (deslocamento_tangentes_ != static_cast<unsigned int>(-1));
  tem_cores_ = (deslocamento_cores_ != static_cast<unsigned int>(-1));
  tem_texturas_ = (deslocamento_texturas_ != static_cast<unsigned int>(-1));
  tem_matriz_modelagem_ = (deslocamento_matriz_modelagem_ != static_cast<unsigned int>(-1));
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, nome_coordenadas_);
  V_ERRO("na ligacao com buffer");
  if (usar_buffer_sub_data) {
    gl::BufferizaSubDados(
        GL_ARRAY_BUFFER, 0,
        sizeof(GL_FLOAT) * buffer_unico_.size(),
        buffer_unico_.data());
  } else {
    gl::BufferizaDados(
        GL_ARRAY_BUFFER,
        sizeof(GL_FLOAT) * buffer_unico_.size(),
        buffer_unico_.data(),
        GL_STATIC_DRAW);
  }
  V_ERRO("ao bufferizar");
  gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);

  // Buffer de indices.
  if (!gravado_) {
    gl::GeraBuffers(1, &nome_indices_);
    if (nome_indices_ == 0) {
      LOG(ERROR) << "ERRO GRAFICO SERIO!!!!!!!!: glGenBuffers gerou valor 0. Provavelmente aplicação está sem o contexto grafico.";
      return;
    }
  }
  V_ERRO("ao gerar buffer indices");
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, nome_indices_);
  V_ERRO("na ligacao com buffer 2");
  indices_ = vbo_nao_gravado.indices();
  if (usar_buffer_sub_data) {
    gl::BufferizaSubDados(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned short) * indices_.size(), indices_.data());
  } else {
    gl::BufferizaDados(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * indices_.size(), indices_.data(), GL_STATIC_DRAW);
  }
  V_ERRO("ao bufferizar elementos");
  gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  if (!gravado_) {
    TipoShader shader_corrente = TipoShaderCorrente();
    vao_por_shader_.resize(TSH_NUM);
    vao_instancia_por_shader_.resize(TSH_NUM);
    for (int shader = TSH_LUZ; shader < TSH_NUM; ++shader) {
      UsaShader(static_cast<TipoShader>(shader));
      {
        GLuint& vao = vao_por_shader_[shader];
        GeraObjetosVertices(1, &vao);
        V_ERRO("ao gerar VAO");
        if (vao == 0) {
          LOG(ERROR) << "ERRO GRAFICO SERIO!!!!!!!!: glGenVErtexArrays gerou valor 0. Provavelmente aplicação está sem o contexto grafico.";
          continue;
        }
        ConfiguraVao(modo_, *this, shader);
      }
    }
    UsaShader(shader_corrente);
  }
  //LOG(INFO) << "gravado nome_coordenadas: " << nome_coordenadas_ << ", nome_indices: " << nome_indices_ << ", vao: " << vao_ << ", sub dados: " << usar_buffer_sub_data;
  gravado_ = true;
}

void VboGravado::Desgrava() {
  if (!gravado_) {
    return;
  }
  //LOG(INFO) << "desgravando vbo: " << nome_ << ", nome_coordenadas: " << nome_coordenadas_ << ", nome_indices: " << nome_indices_ << ", vao: " << vao_;
  gl::ApagaBuffers(1, &nome_coordenadas_);
  V_ERRO("ao deletar VBO coordenadas");
  nome_coordenadas_ = 0;
  gl::ApagaBuffers(1, &nome_indices_);
  V_ERRO("ao deletar VBO indices");
  nome_indices_ = 0;
  for (GLuint vao : vao_por_shader_) {
    ApagaObjetosVertices(1, &vao);
  }
  vao_por_shader_.clear();
  for (GLuint vao : vao_instancia_por_shader_) {
    ApagaObjetosVertices(1, &vao);
  }
  vao_instancia_por_shader_.clear();
  V_ERRO("ao deletar VAO");
  ApagaBufferUnico();
  gravado_ = false;
}

//--------
// Funcoes
//--------

VboNaoGravado VboConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  return VboTroncoConeSolido(base, 0.0f, altura, num_fatias, num_tocos);
}

VboNaoGravado VboTroncoConeSolido(GLfloat raio_base, GLfloat raio_topo_original, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_indices_total = num_indices_por_toco * num_tocos;
  const int num_coordenadas_textura_total = num_vertices_por_toco * num_tocos * 2;

  std::vector<float> coordenadas(num_coordenadas_total);
  std::vector<float> normais(num_coordenadas_total);
  std::vector<float> tangentes(num_coordenadas_total);
  std::vector<unsigned short> indices(num_indices_total);
  std::vector<float> coordenadas_textura(num_coordenadas_textura_total);

  float h_delta = altura / num_tocos;
  float h_topo = 0;
  float delta_raio = (raio_base - raio_topo_original) / num_tocos;
  float raio_topo = raio_base;

  float angulo_fatia_rad = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  float cos_fatia = cosf(angulo_fatia_rad);
  float sen_fatia = sinf(angulo_fatia_rad);

  int i_coordenadas = 0;
  int i_indices = 0;
  int i_coordenadas_textura = 0;
  int coordenada_inicial = 0;
  float inc_textura_h = 1.0f / num_fatias;
  float inc_textura_v = 1.0f / num_tocos;

  // Primeiro ponto é apontando para y = 1.0.
  float v_base[2];
  float v_topo[2] = { 0.0f, raio_base };
  // Inclinacao do tronco.
  float beta_rad = atanf(altura / (raio_base - raio_topo_original));
  float alfa_rad = (M_PI / 2.0f) - beta_rad;  // A normal esta a 90 graus da inclinacao.
  float sen_alfa = sinf(alfa_rad);
  float cos_alfa = cosf(alfa_rad);
  // Desenha-se cada fatia de um toco, e passa-se para o proximo toco na vertical, subindo.
  for (int t = 1; t <= num_tocos; ++t) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    raio_topo -= delta_raio;
    v_topo[0] = 0.0f;
    v_topo[1] = raio_topo;
    float v_normal[3];
    v_normal[0] = 0.0f;
    v_normal[1] = cos_alfa;
    v_normal[2] = sen_alfa;

    // texturas: comeca x do zero e y do inicio do toco.
    float tex_x = 0.0f;
    float tex_y = 1.0f - ((t - 1) * inc_textura_v);

    for (int f = 0; f < num_fatias; ++f) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      // v3 = vtopo.
      coordenadas[i_coordenadas + 9] = v_topo[0];
      coordenadas[i_coordenadas + 10] = v_topo[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      // V2 = vtopo rodado.
      float v_topo_0_rodado = v_topo[0] * cos_fatia - v_topo[1] * sen_fatia;
      float v_topo_1_rodado = v_topo[0] * sen_fatia + v_topo[1] * cos_fatia;
      v_topo[0] = v_topo_0_rodado;
      v_topo[1] = v_topo_1_rodado;
      coordenadas[i_coordenadas + 6] = v_topo[0];
      coordenadas[i_coordenadas + 7] = v_topo[1];
      coordenadas[i_coordenadas + 8] = h_topo;

      // Textura: base, base + 1, topo + 1, topo.
      // Observe que eh a mesma sequencia que acima, mas na ordem das texturas.
      coordenadas_textura[i_coordenadas_textura] = tex_x;
      coordenadas_textura[i_coordenadas_textura + 1] = tex_y;
      coordenadas_textura[i_coordenadas_textura + 2] = tex_x + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 3] = tex_y;
      coordenadas_textura[i_coordenadas_textura + 4] = tex_x + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 5] = tex_y - inc_textura_v;
      coordenadas_textura[i_coordenadas_textura + 6] = tex_x;
      coordenadas_textura[i_coordenadas_textura + 7] = tex_y - inc_textura_v;
      i_coordenadas_textura += 8;
      tex_x += inc_textura_h;

      // As normais e tangentes.
      Vector3 c0(coordenadas[i_coordenadas + 0], coordenadas[i_coordenadas + 1], coordenadas[i_coordenadas + 2]);
      Vector3 c1(coordenadas[i_coordenadas + 3], coordenadas[i_coordenadas + 4], coordenadas[i_coordenadas + 5]);
      Vector3 c0c1 = c1 - c0;
      tangentes[i_coordenadas + 0] = c0c1.x;
      tangentes[i_coordenadas + 1] = c0c1.y;
      tangentes[i_coordenadas + 2] = c0c1.z;
      tangentes[i_coordenadas + 3] = c0c1.x;
      tangentes[i_coordenadas + 4] = c0c1.y;
      tangentes[i_coordenadas + 5] = c0c1.z;
      tangentes[i_coordenadas + 6] = c0c1.x;
      tangentes[i_coordenadas + 7] = c0c1.y;
      tangentes[i_coordenadas + 8] = c0c1.z;
      tangentes[i_coordenadas + 9] = c0c1.x;
      tangentes[i_coordenadas + 10] = c0c1.y;
      tangentes[i_coordenadas + 11] = c0c1.z;

      // Vn0.
      normais[i_coordenadas] = v_normal[0];
      normais[i_coordenadas + 1] = v_normal[1];
      normais[i_coordenadas + 2] = v_normal[2];
      // Vn3 = acima de vn0.
      normais[i_coordenadas + 9] = v_normal[0];
      normais[i_coordenadas + 10] = v_normal[1];
      normais[i_coordenadas + 11] = v_normal[2];
      // Vn1 = vn0 rodado.
      float v_normal_0_rodado = v_normal[0] * cos_fatia - v_normal[1] * sen_fatia;
      float v_normal_1_rodado = v_normal[0] * sen_fatia + v_normal[1] * cos_fatia;
      v_normal[0] = v_normal_0_rodado;
      v_normal[1] = v_normal_1_rodado;
      normais[i_coordenadas + 3] = v_normal[0];
      normais[i_coordenadas + 4] = v_normal[1];
      normais[i_coordenadas + 5] = v_normal[2];
      // Vn2 = acima de vn1.
      normais[i_coordenadas + 6] = v_normal[0];
      normais[i_coordenadas + 7] = v_normal[1];
      normais[i_coordenadas + 8] = v_normal[2];

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;

      i_indices += 6;
      i_coordenadas += 12;
      coordenada_inicial += 4;
    }
  }

#if 0
  LOG(INFO) << "raio_base: " << raio_base;
  LOG(INFO) << "raio_topo: " << raio_topo_original;
  for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
    LOG(INFO) << "indices[" << i << "]: " << indices[i];
  }
  for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
    LOG(INFO) << "coordenadas[" << i / 3 << "]: "
              << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  }
#endif

  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas.data(), num_coordenadas_total);
  for (unsigned int i = 0; i < normais.size(); i += 3) {
    {
      Vector3 n(normais[i], normais[i+1], normais[i+2]);
      n.normalize();
      normais[i] = n.x; normais[i + 1] = n.y; normais[i + 2] = n.z;
    }
    {
      Vector3 t(tangentes[i], tangentes[i+1], tangentes[i+2]);
      t.normalize();
      tangentes[i] = t.x; tangentes[i + 1] = t.y; tangentes[i + 2] = t.z;
    }
  }
  vbo.AtribuiNormais(normais.data());
  vbo.AtribuiTangentes(tangentes.data());
  vbo.AtribuiIndices(indices.data(), num_indices_total);
  vbo.AtribuiTexturas(coordenadas_textura.data());
  vbo.Nomeia("troncocone");
  return vbo;
}

VboNaoGravado VboEsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  // Vertices.
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos * 2;
  const int num_indices_total = num_indices_por_toco * num_tocos * 2;
  const int num_coordenadas_textura_total = num_vertices_por_toco * 2 * num_tocos * 2;

  float angulo_h_rad = (90.0f * GRAUS_PARA_RAD) / num_tocos;
  float angulo_fatia_rad = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  std::vector<float> coordenadas(num_coordenadas_total);
  std::vector<float> tangentes(num_coordenadas_total);
  std::vector<float> coordenadas_textura(num_coordenadas_textura_total);
  std::vector<unsigned short> indices(num_indices_total);
  float cos_fatia = cosf(angulo_fatia_rad);
  float sen_fatia = sinf(angulo_fatia_rad);

  float v_base[2];
  v_base[0] = 0;
  v_base[1] = 0;
  float v_topo[2];
  v_topo[0] = 0;
  v_topo[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;

  int i_coordenadas_textura = 0;

  float raio_topo = raio;
  float inc_textura_h = 1.0f / num_fatias;
  float inc_textura_v = -0.5f / num_tocos;  // vai pros dois lados.

  for (int i = 1; i <= num_tocos; ++i) {
    float h_base = h_topo;
    // Novas alturas e base.
    float angulo_h_rad_vezes_i = angulo_h_rad * i;
    h_topo = sinf(angulo_h_rad_vezes_i) * raio;
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    v_topo[0] = 0.0f;
    raio_topo = raio * cosf(angulo_h_rad_vezes_i);
    v_topo[1] = raio_topo;
    float coordenada_textura_h = 0.0f;
    float coordenada_textura_v = 0.5f + (i - 1) * inc_textura_v;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta da esfera possui 4 vertices (anti horario). Cada vertice sera a propria normal.
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      coordenadas_textura[i_coordenadas_textura]     = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 1] = coordenada_textura_v;
      coordenadas_textura[i_coordenadas_textura + 8] = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 9] = 1.0f - coordenada_textura_v;

      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      coordenadas_textura[i_coordenadas_textura + 2]  = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 3]  = coordenada_textura_v;
      coordenadas_textura[i_coordenadas_textura + 10] = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 11] = 1.0f - coordenada_textura_v;

      // v3 = vtopo.
      coordenadas[i_coordenadas + 9] = v_topo[0];
      coordenadas[i_coordenadas + 10] = v_topo[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 6] = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 7] = coordenada_textura_v + inc_textura_v;
      coordenadas_textura[i_coordenadas_textura + 14] = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 15] = 1.0f - coordenada_textura_v - inc_textura_v;
      // V2 = vtopo rodado.
      float v_topo_0_rodado = v_topo[0] * cos_fatia - v_topo[1] * sen_fatia;
      float v_topo_1_rodado = v_topo[0] * sen_fatia + v_topo[1] * cos_fatia;
      v_topo[0] = v_topo_0_rodado;
      v_topo[1] = v_topo_1_rodado;
      coordenadas[i_coordenadas + 6] = v_topo[0];
      coordenadas[i_coordenadas + 7] = v_topo[1];
      coordenadas[i_coordenadas + 8] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 4]  = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 5]  = coordenada_textura_v + inc_textura_v;
      coordenadas_textura[i_coordenadas_textura + 12] = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 13] = 1.0f - coordenada_textura_v - inc_textura_v;

      // Simetria na parte de baixo.
      memcpy(coordenadas.data() + i_coordenadas + 12, coordenadas.data() + i_coordenadas, sizeof(float) * 12);
      for (int c = 2; c < 12 ; c += 3) {
        coordenadas[i_coordenadas + 12 + c] = -coordenadas[i_coordenadas + c];
      }

      Vector3 c0(coordenadas[i_coordenadas], coordenadas[i_coordenadas + 1], coordenadas[i_coordenadas + 2]);
      Vector3 c1(coordenadas[i_coordenadas + 3], coordenadas[i_coordenadas + 4], coordenadas[i_coordenadas + 5]);
      Vector3 c0c1 = c1 - c0;
      tangentes[i_coordenadas + 0 ] = c0c1.x;
      tangentes[i_coordenadas + 1 ] = c0c1.y;
      tangentes[i_coordenadas + 2 ] = c0c1.z;
      tangentes[i_coordenadas + 3 ] = c0c1.x;
      tangentes[i_coordenadas + 4 ] = c0c1.y;
      tangentes[i_coordenadas + 5 ] = c0c1.z;
      tangentes[i_coordenadas + 6 ] = c0c1.x;
      tangentes[i_coordenadas + 7 ] = c0c1.y;
      tangentes[i_coordenadas + 8 ] = c0c1.z;
      tangentes[i_coordenadas + 9 ] = c0c1.x;
      tangentes[i_coordenadas + 10] = c0c1.y;
      tangentes[i_coordenadas + 11] = c0c1.z;
      tangentes[i_coordenadas + 12] = c0c1.x;
      tangentes[i_coordenadas + 13] = c0c1.y;
      tangentes[i_coordenadas + 14] = c0c1.z;
      tangentes[i_coordenadas + 15] = c0c1.x;
      tangentes[i_coordenadas + 16] = c0c1.y;
      tangentes[i_coordenadas + 17] = c0c1.z;
      tangentes[i_coordenadas + 18] = c0c1.x;
      tangentes[i_coordenadas + 19] = c0c1.y;
      tangentes[i_coordenadas + 20] = c0c1.z;
      tangentes[i_coordenadas + 21] = c0c1.x;
      tangentes[i_coordenadas + 22] = c0c1.y;
      tangentes[i_coordenadas + 23] = c0c1.z;

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;
      indices[i_indices + 6] = coordenada_inicial + 4;
      indices[i_indices + 7] = coordenada_inicial + 7;
      indices[i_indices + 8] = coordenada_inicial + 6;
      indices[i_indices + 9] = coordenada_inicial + 4;
      indices[i_indices + 10] = coordenada_inicial + 6;
      indices[i_indices + 11] = coordenada_inicial + 5;

      i_indices += 12;
      i_coordenadas += 24;
      i_coordenadas_textura += 16;
      coordenada_textura_h += inc_textura_h;
      coordenada_inicial += 8;
    }
  }

  std::vector<float> normais(coordenadas);
  float* na = &normais[0];
  for (unsigned int i = 0; i < normais.size(); i += 3) {
    Vector3 vn(na[i], na[i+1], na[i+2]);
    vn.normalize();
    na[i] = vn.x;
    na[i+1] = vn.y;
    na[i+2] = vn.z;
  }

  //LOG(INFO) << "raio: " << raio;
  //for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
  //  LOG(INFO) << "indices[" << i << "]: " << indices[i];
  //}
  //for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
  //  LOG(INFO) << "coordenadas[" << i / 3 << "]: "
  //            << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  //}

  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas.data(), num_coordenadas_total);
  vbo.AtribuiNormais(normais.data());
  vbo.AtribuiTangentes(tangentes.data());
  vbo.AtribuiIndices(indices.data(), num_indices_total);
  vbo.AtribuiTexturas(coordenadas_textura.data());
  vbo.Nomeia("esfera");
  return vbo;
}

VboNaoGravado VboHemisferioSolido(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  // Vertices.
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_indices_total = num_indices_por_toco * num_tocos;
  const int num_coordenadas_textura_total = num_vertices_por_toco * num_tocos * 2;

  float angulo_h_rad = (90.0f * GRAUS_PARA_RAD) / num_tocos;
  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  std::vector<float> coordenadas(num_coordenadas_total);
  std::vector<float> tangentes(num_coordenadas_total);
  std::vector<float> coordenadas_textura(num_coordenadas_textura_total);
  std::vector<unsigned short> indices(num_indices_total);
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  float v_base[2];
  v_base[0] = 0;
  v_base[1] = 0;
  float v_topo[2];
  v_topo[0] = 0;
  v_topo[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;

  int i_coordenadas_textura = 0;

  float raio_topo = raio;
  float inc_textura_h = 1.0f / num_fatias;
  float inc_textura_v = -0.5f / num_tocos;

  for (int i = 1; i <= num_tocos; ++i) {
    float h_base = h_topo;
    // Novas alturas e base.
    float angulo_h_rad_vezes_i = angulo_h_rad * i;
    h_topo = sinf(angulo_h_rad_vezes_i) * raio;
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    v_topo[0] = 0.0f;
    raio_topo = raio * cosf(angulo_h_rad_vezes_i);
    v_topo[1] = raio_topo;
    float coordenada_textura_h = 0.0f;
    float coordenada_textura_v = 0.5f + (i - 1) * inc_textura_v;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta do hemisferio possui 4 vertices (anti horario). Cada vertice sera a propria normal.
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      coordenadas_textura[i_coordenadas_textura]     = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 1] = coordenada_textura_v;

      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      coordenadas_textura[i_coordenadas_textura + 2]  = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 3]  = coordenada_textura_v;
      // v3 = vtopo.
      coordenadas[i_coordenadas + 9] = v_topo[0];
      coordenadas[i_coordenadas + 10] = v_topo[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 6] = coordenada_textura_h;
      coordenadas_textura[i_coordenadas_textura + 7] = coordenada_textura_v + inc_textura_v;
      // V2 = vtopo rodado.
      float v_topo_0_rodado = v_topo[0] * cos_fatia - v_topo[1] * sen_fatia;
      float v_topo_1_rodado = v_topo[0] * sen_fatia + v_topo[1] * cos_fatia;
      v_topo[0] = v_topo_0_rodado;
      v_topo[1] = v_topo_1_rodado;
      coordenadas[i_coordenadas + 6] = v_topo[0];
      coordenadas[i_coordenadas + 7] = v_topo[1];
      coordenadas[i_coordenadas + 8] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 4]  = coordenada_textura_h + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 5]  = coordenada_textura_v + inc_textura_v;

      Vector3 c0(coordenadas[i_coordenadas], coordenadas[i_coordenadas + 1], coordenadas[i_coordenadas + 2]);
      Vector3 c1(coordenadas[i_coordenadas + 3], coordenadas[i_coordenadas + 4], coordenadas[i_coordenadas + 5]);
      Vector3 c0c1 = c1 - c0;
      tangentes[i_coordenadas + 0 ] = c0c1.x;
      tangentes[i_coordenadas + 1 ] = c0c1.y;
      tangentes[i_coordenadas + 2 ] = c0c1.z;
      tangentes[i_coordenadas + 3 ] = c0c1.x;
      tangentes[i_coordenadas + 4 ] = c0c1.y;
      tangentes[i_coordenadas + 5 ] = c0c1.z;
      tangentes[i_coordenadas + 6 ] = c0c1.x;
      tangentes[i_coordenadas + 7 ] = c0c1.y;
      tangentes[i_coordenadas + 8 ] = c0c1.z;
      tangentes[i_coordenadas + 9 ] = c0c1.x;
      tangentes[i_coordenadas + 10] = c0c1.y;
      tangentes[i_coordenadas + 11] = c0c1.z;

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;

      i_indices += 6;
      i_coordenadas += 12;
      i_coordenadas_textura += 8;
      coordenada_textura_h += inc_textura_h;
      coordenada_inicial += 4;
    }
  }
  //LOG(INFO) << "raio: " << raio;
  //for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
  //  LOG(INFO) << "indices[" << i << "]: " << indices[i];
  //}
  //for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
  //  LOG(INFO) << "coordenadas[" << i / 3 << "]: "
  //            << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  //}

  std::vector<float> normais(coordenadas);
  float* na = &normais[0];
  for (unsigned int i = 0; i < normais.size(); i += 3) {
    Vector3 vn(na[i], na[i+1], na[i+2]);
    vn.normalize();
    na[i] = vn.x;
    na[i+1] = vn.y;
    na[i+2] = vn.z;
  }

  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas.data(), num_coordenadas_total);
  vbo.AtribuiNormais(normais.data());
  vbo.AtribuiTangentes(tangentes.data());
  vbo.AtribuiIndices(indices.data(), num_indices_total);
  vbo.AtribuiTexturas(coordenadas_textura.data());
  vbo.Nomeia("hemisferio");
  return vbo;
}


VboNaoGravado VboCilindroSolido(GLfloat raio, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  // Vertices.
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_coordenadas_textura_total = num_vertices_por_toco * num_tocos * 2;
  const int num_indices_total = num_indices_por_toco * num_tocos;

  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  std::vector<float> coordenadas(num_coordenadas_total);
  std::vector<float> coordenadas_textura(num_coordenadas_textura_total);
  std::vector<float> normais(num_coordenadas_total);
  std::vector<float> tangentes(num_coordenadas_total);
  std::vector<unsigned short> indices(num_indices_total);
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  int i_normais = 0;
  float v_base[2];
  float* na = &normais[0];
  for (int toco = 1; toco <= num_tocos; ++toco) {
    v_base[0] = 0.0f;
    v_base[1] = 1.0;  // normais tem tamanho 1.
    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      na[i_normais] = v_base[0];
      na[i_normais + 1] = v_base[1];
      na[i_normais + 2] = 0;

      // v3 = vbase topo.
      na[i_normais + 9] = v_base[0];
      na[i_normais + 10] = v_base[1];
      na[i_normais + 11] = 0;

      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      na[i_normais + 3] = v_base[0];
      na[i_normais + 4] = v_base[1];
      na[i_normais + 5] = 0;
      // V2 = vtopo rodado.
      na[i_normais + 6] = v_base[0];
      na[i_normais + 7] = v_base[1];
      na[i_normais + 8] = 0;

      // Incrementa.
      i_normais += 12;
    }
  }
  Matrix4 mt;
  mt.rotateZ(90);
  Vector4 nt;
  float* ta = &tangentes[0];
  for (unsigned int i = 0; i < normais.size(); i += 3) {
    nt.set(na[i], na[i + 1], na[i + 2], 1.0);
    nt = mt * nt;
    ta[i] = nt.x;
    ta[i + 1] = nt.y;
  }

  float h_delta = altura / num_tocos;
  v_base[0] = 0;
  v_base[1] = raio;
  float h_topo = 0;

  int i_coordenadas = 0;
  int i_coordenadas_textura = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;
  float inc_textura_h = 1.0f / num_fatias;
  float inc_textura_v = 1.0f / num_tocos;
  float h_textura = 0.0f;
  float v_textura = 1.0f;
  for (int toco = 1; toco <= num_tocos; ++toco) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio;
    h_textura = 0.0f;
    v_textura = 1.0f - (toco - 1) * inc_textura_v;

    for (int i = 0; i < num_fatias; ++i) {
      // Cada faceta possui 4 vertices (anti horario).
      // V0 = vbase.
      coordenadas[i_coordenadas] = v_base[0];
      coordenadas[i_coordenadas + 1] = v_base[1];
      coordenadas[i_coordenadas + 2] = h_base;
      coordenadas_textura[i_coordenadas_textura]     = h_textura;
      coordenadas_textura[i_coordenadas_textura + 1] = v_textura;
      // v3 = vbase topo.
      coordenadas[i_coordenadas + 9] = v_base[0];
      coordenadas[i_coordenadas + 10] = v_base[1];
      coordenadas[i_coordenadas + 11] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 6] = h_textura;
      coordenadas_textura[i_coordenadas_textura + 7] = v_textura - inc_textura_v;

      // V1 = vbase rodado.
      float v_base_0_rodado = v_base[0] * cos_fatia - v_base[1] * sen_fatia;
      float v_base_1_rodado = v_base[0] * sen_fatia + v_base[1] * cos_fatia;
      v_base[0] = v_base_0_rodado;
      v_base[1] = v_base_1_rodado;
      coordenadas[i_coordenadas + 3] = v_base[0];
      coordenadas[i_coordenadas + 4] = v_base[1];
      coordenadas[i_coordenadas + 5] = h_base;
      coordenadas_textura[i_coordenadas_textura + 2] = h_textura + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 3] = v_textura;

      // V2 = vtopo rodado.
      coordenadas[i_coordenadas + 6] = v_base[0];
      coordenadas[i_coordenadas + 7] = v_base[1];
      coordenadas[i_coordenadas + 8] = h_topo;
      coordenadas_textura[i_coordenadas_textura + 4] = h_textura + inc_textura_h;
      coordenadas_textura[i_coordenadas_textura + 5] = v_textura - inc_textura_v;

      // Indices: V0, V1, V2, V0, V2, V3.
      indices[i_indices] = coordenada_inicial;
      indices[i_indices + 1] = coordenada_inicial + 1;
      indices[i_indices + 2] = coordenada_inicial + 2;
      indices[i_indices + 3] = coordenada_inicial;
      indices[i_indices + 4] = coordenada_inicial + 2;
      indices[i_indices + 5] = coordenada_inicial + 3;

      i_indices += 6;
      i_coordenadas += 12;
      i_coordenadas_textura += 8;
      coordenada_inicial += 4;
      h_textura += inc_textura_h;
    }
  }
  //LOG(INFO) << "raio: " << raio;
  //for (int i = 0; i < sizeof(indices) / sizeof(unsigned short); ++i) {
  //  LOG(INFO) << "indices[" << i << "]: " << indices[i];
  //}
  //for (int i = 0; i < sizeof(coordenadas) / sizeof(float); i += 3) {
  //  LOG(INFO) << "coordenadas[" << i / 3 << "]: "
  //            << coordenadas[i] << ", " << coordenadas[i + 1] << ", " << coordenadas[i + 2];
  //}

  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas.data(), num_coordenadas_total);
  vbo.AtribuiTexturas(coordenadas_textura.data());
  vbo.AtribuiNormais(normais.data());
  vbo.AtribuiTangentes(tangentes.data());
  vbo.AtribuiIndices(indices.data(), num_indices_total);
  vbo.Nomeia("cilindro");
  return vbo;
}

VboNaoGravado VboCuboSolido(GLfloat tam_lado) {
  GLfloat meio_lado = tam_lado / 2.0f;
  const unsigned short num_indices = 36;
  unsigned short indices[num_indices] = {
      0, 1, 2, 0, 2, 3,        // sul
      4, 5, 6, 4, 6, 7,        // norte
      8, 9, 10, 8, 10, 11,     // oeste
      12, 13, 14, 12, 14, 15,  // leste
      16, 17, 18, 16, 18, 19,  // cima
      20, 21, 22, 20, 22, 23,  // baixo
  };
  const unsigned short num_coordenadas = 6 * 3 * 4;
  const float normais[num_coordenadas] = {
    // sul.
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    // norte.
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    // oeste.
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    // leste.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // cima.
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    // baixo.
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
    0.0f, 0.0f, -1.0f,
  };
  const float tangentes[num_coordenadas] = {
    // sul.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // norte.
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f, 0.0f,
    // oeste.
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    // leste.
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    // cima.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // baixo.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
  };

  const float coordenadas[num_coordenadas] = {
    // sul: 0-3
    -meio_lado, -meio_lado, -meio_lado,
    meio_lado, -meio_lado, -meio_lado,
    meio_lado, -meio_lado, meio_lado,
    -meio_lado, -meio_lado, meio_lado,
    // norte: 4-7.
    -meio_lado, meio_lado, -meio_lado,
    -meio_lado, meio_lado, meio_lado,
    meio_lado, meio_lado, meio_lado,
    meio_lado, meio_lado, -meio_lado,
    // oeste: 8-11.
    -meio_lado, -meio_lado, -meio_lado,
    -meio_lado, -meio_lado, meio_lado,
    -meio_lado, meio_lado, meio_lado,
    -meio_lado, meio_lado, -meio_lado,
    // leste: 12-15.
    meio_lado, -meio_lado, -meio_lado,
    meio_lado, meio_lado, -meio_lado,
    meio_lado, meio_lado, meio_lado,
    meio_lado, -meio_lado, meio_lado,
    // cima: 16-19.
    -meio_lado, -meio_lado, meio_lado,
    meio_lado, -meio_lado, meio_lado,
    meio_lado, meio_lado, meio_lado,
    -meio_lado, meio_lado, meio_lado,
    // baixo: 20-23.
    -meio_lado, -meio_lado, -meio_lado,
    -meio_lado, meio_lado, -meio_lado,
    meio_lado, meio_lado, -meio_lado,
    meio_lado, -meio_lado, -meio_lado,
  };
  const float coordenadas_texel[] = {
    // Sul.
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    // Norte.
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    // Oeste.
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    // Leste.
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    // Cima.
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    // Baixo.
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
  };


  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, num_coordenadas);
  vbo.AtribuiNormais(normais);
  vbo.AtribuiTangentes(tangentes);
  vbo.AtribuiIndices(indices, num_indices);
  vbo.AtribuiTexturas(coordenadas_texel);
  vbo.Nomeia("cubo");
  return vbo;
}

void CuboUnitario() {
  DesenhaVboGravado(g_vbos[VBO_CUBO]);
}

VboNaoGravado VboPiramideSolida(GLfloat tam_lado, GLfloat altura) {
  const unsigned short indices[] = {
      0, 1, 2,  // sul
      3, 4, 5,  // leste
      6, 7, 8,  // norte
      9, 10, 11, // oeste
  };
  // Todas normais sao compostas pelo mesmo componente em direcoes diferentes por 90 graus.
  // Alfa eh o angulo da base com as paredes.
  // m = tam_lado / 2.0f.
  // t = lateral da piramide, tamanho: sqrt(m^2 + h^2).
  // cos(a) = m / t = componente eixo x-y.
  // sen(a) = h / t = componente z.
  const float m = tam_lado / 2.0f;
  const float h = altura;
  const float t = sqrt(m * m + h * h);
  const float cos_a = m / t;
  const float comp_xy = cos_a;
  const float sen_a = h / t;
  const float comp_z = sen_a;
  float normais[] = {
    // Face sul
    0.0f, -comp_xy, comp_z,
    0.0f, -comp_xy, comp_z,
    0.0f, -comp_xy, comp_z,
    // Face leste.
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
    // Face norte.
    0.0f, comp_xy, comp_z,
    0.0f, comp_xy, comp_z,
    0.0f, comp_xy, comp_z,
    // Face Oeste.
    -comp_xy, 0.0f, comp_z,
    -comp_xy, 0.0f, comp_z,
    -comp_xy, 0.0f, comp_z,
  };
  for (unsigned int i = 0; i < 36; i+=3) {
    Vector3 nv(normais[i], normais[i+1], normais[i+2]);
    nv.normalize();
    normais[i] = nv.x;
    normais[i+1] = nv.y;
    normais[i+2] = nv.z;
  }
  float tangentes[] = {
    // Face sul
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // Face leste.
    comp_xy, 0.0f, -comp_z,
    comp_xy, 0.0f, -comp_z,
    comp_xy, 0.0f, -comp_z,
    // Face norte.
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    // Face Oeste.
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
    comp_xy, 0.0f, comp_z,
  };
  for (unsigned int i = 0; i < 36; i+=3) {
    Vector3 tv(tangentes[i], tangentes[i+1], tangentes[i+2]);
    tv.normalize();
    tangentes[i]   = tv.x;
    tangentes[i+1] = tv.y;
    tangentes[i+2] = tv.z;
  }

  const float coordenadas[] = {
    // Topo.
    // Face sul.
    0.0f, 0.0f, altura,
    -m, -m, 0.0f,
    m, -m, 0.0f,
    // Face leste.
    0.0f, 0.0f, altura,
    m, -m, 0.0f,
    m, m, 0.0f,
    // Face norte.
    0.0f, 0.0f, altura,
    m, m, 0.0f,
    -m, m, 0.0f,
    // Face Oeste.
    0.0f, 0.0f, altura,
    -m, m, 0.0f,
    -m, -m, 0.0f,
  };
  const float coordenadas_texel[] = {
    // sul
    0.5f, 0.5f,  // s meio
    0.0f, 1.0f,   // s oeste
    1.0f, 1.0f,   // s leste
    // leste
    0.5f, 0.5f,  // e meio
    1.0f, 1.0f,   // e sul
    1.0f, 0.0f,   // e norte
    // norte
    0.5f, 0.5f,  // n meio
    1.0f, 0.0f,   // n leste
    0.0f, 0.0f,   // n oeste
    // oeste
    0.5f, 0.5f,  // w meio
    0.0f, 0.0f,   // w norte
    0.0f, 1.0f,   // w sul
  };
  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, sizeof(coordenadas) / sizeof(float));
  vbo.AtribuiTexturas(coordenadas_texel);
  vbo.AtribuiIndices(indices, 12);
  vbo.AtribuiNormais(normais);
  vbo.AtribuiTangentes(tangentes);
  vbo.Nomeia("piramide");
  return vbo;
}

VboNaoGravado VboRetangulo(GLfloat tam_x, GLfloat tam_y) {
  float x = tam_x / 2.0f;
  float y = tam_y / 2.0f;
  return VboRetangulo(-x, -y, x, y);
}

void RetanguloUnitario() {
  DesenhaVboGravado(g_vbos[VBO_RETANGULO]);
}

VboNaoGravado VboRetangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  const unsigned short indices[] = { 0, 1, 2, 3, 4, 5 };
  const float coordenadas[] = {
    x1, y1, 0.0f,
    x2,  y1, 0.0f,
    x2,  y2,  0.0f,
    x1, y1, 0.0f,
    x2, y2, 0.0f,
    x1, y2,  0.0f,
  };
  const float normais[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f,
  };
  const float tangentes[] = {
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
  };
  const float coordenadas_texel[] = {
    0.0f, 1.0f,  // x1, y1
    1.0f, 1.0f,  // x2, y1
    1.0f, 0.0f,  // x2, y2
    0.0f, 1.0f,  // x1, y1
    1.0f, 0.0f,  // x2, y2
    0.0f, 0.0f,  // x1, y2
  };
  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, sizeof(coordenadas) / sizeof(float));
  vbo.AtribuiNormais(normais);
  vbo.AtribuiTangentes(tangentes);
  vbo.AtribuiTexturas(coordenadas_texel);
  vbo.AtribuiIndices(indices, sizeof(indices) / sizeof(unsigned short));
  vbo.Nomeia("retangulo");
  return vbo;
}

VboNaoGravado VboDisco(GLfloat raio, GLfloat num_faces) {
  const unsigned short num_coordenadas = 3 + (num_faces + 1) * 3;
  std::vector<float> coordenadas(num_coordenadas);
  std::vector<float> normais(num_coordenadas);
  std::vector<float> tangentes(num_coordenadas);
  //std::vector<float> cores((num_coordenadas / 3) * 4);
  std::vector<unsigned short> indices(num_faces * 3);
  // Norte.
  coordenadas[0] = 0.0f;
  coordenadas[1] = raio;
  normais[2] = 1.0f;
  tangentes[0] = 1.0f;
  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_faces;
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);
  //cores[0] = 0;
  //cores[1] = 0;
  //cores[2] = 0;
  //cores[3] = 1.0f;
  for (int i = 3; i < num_coordenadas; i += 3) {
    coordenadas[i] = coordenadas[i - 3] * cos_fatia - coordenadas[i - 2] * sen_fatia;
    coordenadas[i + 1] = coordenadas[i - 3] * sen_fatia + coordenadas[i - 2] * cos_fatia;
    normais[i + 2] = 1.0f;
    tangentes[i] = 1.0f;
    //int ic = (i / 3) * 4;
    //cores[ic] = ic * 0.1;
    //cores[ic+1] = ic * 0.1;
    //cores[ic+2] = ic * 0.1;
    //cores[ic+3] = 1.0f;
  }
  const unsigned short num_coordenadas_texel = 2 + (num_faces + 1) * 2;
  std::vector<float> coordenadas_texel(num_coordenadas_texel);
  coordenadas_texel[0] = 0.0f;
  coordenadas_texel[1] = -1.0f;
  for (unsigned int i = 2; i < num_coordenadas_texel; i += 2) {
    float angulo_rad = (180.0f * GRAUS_PARA_RAD) + ((i / 2) * angulo_fatia);
    coordenadas_texel[i] = sin(angulo_rad);
    coordenadas_texel[i + 1] = cos(angulo_rad);
  }
  // Mapeia para [0, 1.0].
  for (unsigned int i = 0; i < coordenadas_texel.size(); ++i) {
    coordenadas_texel[i] = (coordenadas_texel[i] + 1.0f) / 2.0f;
  }
  for (unsigned int i = 0; i < num_faces; ++i) {
    int ind = i * 3;
    indices[ind] = 0;
    indices[ind + 1] = i;
    indices[ind + 2] = i + 1;
  }
  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas.data(), coordenadas.size());
  vbo.AtribuiNormais(normais.data());
  vbo.AtribuiTangentes(tangentes.data());
  //vbo.AtribuiCores(cores.data());
  vbo.AtribuiTexturas(coordenadas_texel.data());
  vbo.AtribuiIndices(indices.data(), indices.size());
  vbo.Nomeia("disco");
  return vbo;
}
void DiscoUnitario() {
  DesenhaVboGravado(g_vbos[VBO_DISCO]);
}

VboNaoGravado VboTriangulo(GLfloat lado) {
  unsigned short indices[] = { 0, 1, 2 };
  GLfloat coordenadas[9] = { 0.0f };
  GLfloat tangentes[9] = { 0.0f };
  float h = 0.86602540378f * lado;  // sen 60.
  coordenadas[0] = 0.0f;
  coordenadas[1] = h;
  coordenadas[3] = -lado / 2.0f;
  coordenadas[4] = 0.0f;
  coordenadas[6] = lado / 2.0f;
  coordenadas[7] = 0.0f;
  tangentes[0] = 1.0f;
  tangentes[3] = 1.0f;
  tangentes[6] = 1.0f;

  const float coordenadas_texel[] = {
    0.5f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
  };
  GLfloat normais[9] = { 0.0f };
  normais[2] = normais[5] = normais[8] = 1.0f;
  VboNaoGravado vbo;
  vbo.AtribuiCoordenadas(3, coordenadas, 9);
  vbo.AtribuiNormais(normais);
  vbo.AtribuiTangentes(tangentes);
  vbo.AtribuiTexturas(coordenadas_texel);
  vbo.AtribuiIndices(indices, 3);
  vbo.Nomeia("triangulo");
  return vbo;
}

void TrianguloUnitario() {
  DesenhaVboGravado(g_vbos[VBO_TRIANGULO]);
}

VboNaoGravado VboLivre(const std::vector<std::pair<float, float>>& pontos, float largura) {
  VboNaoGravado vbo;
  if (pontos.size() == 0) {
    vbo.Nomeia("livre");
    return vbo;
  }
  gl::VboNaoGravado vbo_disco;
  gl::VboNaoGravado vbo_retangulo;
  for (auto it = pontos.begin(); it != pontos.end() - 1;) {
    const auto& ponto = *it;
    // Disco do ponto corrente.
    vbo_disco = gl::VboDisco(largura / 2.0f, 8);
    vbo_disco.Translada(ponto.first, ponto.second, 0.0f);
    vbo.Concatena(vbo_disco);

    // Reta ate proximo ponto.
    const auto& proximo_ponto = *(++it);
    float tam;
    const float graus = VetorParaRotacaoGraus(proximo_ponto.first - ponto.first, proximo_ponto.second - ponto.second, &tam);
    const float largura_2 = largura / 2.0f;
    vbo_retangulo = gl::VboRetangulo(0.0f, -largura_2, tam, largura_2);
    vbo_retangulo.RodaZ(graus);
    vbo_retangulo.Translada(ponto.first, ponto.second, 0.0f);
    vbo.Concatena(vbo_retangulo);
  }
  const auto& ponto = *pontos.rbegin();
  vbo_disco = gl::VboDisco(largura / 2.0f, 8);
  vbo_disco.Translada(ponto.first, ponto.second, 0.0f);
  vbo.Concatena(vbo_disco);
  char nome[50];
  snprintf(nome, 49, "livre:%lup", (unsigned long)pontos.size());
  vbo.Nomeia(nome);
  return vbo;
}

void DesenhaVboGravado(const VboGravado& vbo, bool atualiza_matrizes) {
  if (!vbo.Gravado()) {
    LOG(WARNING) << "ignorando vbo nao gravado: " << vbo.nome();
    return;
  }
  if (!g_hack) {
    if (!vbo.tem_matriz_modelagem() && atualiza_matrizes) {
      gl::AtualizaMatrizes();
    }
    LigacaoComObjetoVertices(vbo.Vao());
    V_ERRO("DesenhaVboGravado: bind");
    //if (!atualiza_matrizes || vbo.nome() == "caixa_ceu") {
    //  LOG(INFO) << "Desenhando: " << vbo.nome() << ", vao: " << vbo.Vao() << ", modo: " << vbo.Modo() << ", num vertices: " << vbo.NumVertices() << ", tem matriz: " << vbo.tem_matriz_modelagem();
    //}
    if (vbo.tem_matriz_modelagem()) {
      gl::DesenhaElementosInstanciado(vbo.Modo(), vbo.NumVertices(), GL_UNSIGNED_SHORT, nullptr, 1);
    } else {
      gl::DesenhaElementos(vbo.Modo(), vbo.NumVertices(), GL_UNSIGNED_SHORT, nullptr);
    }
    LigacaoComObjetoVertices(0);
    V_ERRO("DesenhaVboGravado: desenha");
  } else {
    LigacaoComObjetoVertices(0);
    // Os casts de char* 0 sao para evitar warning de conversao de short pra void*.
    gl::LigacaoComBuffer(GL_ARRAY_BUFFER, vbo.nome_coordenadas());
    gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.nome_indices());
    DesenhaElementosComAtributos(
        vbo.Modo(), vbo.NumVertices(), vbo.NumDimensoes(), nullptr, nullptr,
        vbo.tem_normais(), nullptr, vbo.DeslocamentoNormais(),
        vbo.tem_tangentes(), nullptr, vbo.DeslocamentoTangentes(),
        vbo.tem_texturas(), nullptr, vbo.DeslocamentoTexturas(),
        vbo.tem_cores(), nullptr, vbo.DeslocamentoCores(),
        vbo.tem_matriz_modelagem(), nullptr, vbo.DeslocamentoMatrizModelagem(),
        vbo.tem_matriz_normal(), nullptr, vbo.DeslocamentoMatrizNormal(),
        atualiza_matrizes);
    gl::LigacaoComBuffer(GL_ARRAY_BUFFER, 0);
    gl::LigacaoComBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
}

void DesenhaVboNaoGravado(const VboNaoGravado& vbo, GLenum modo, bool atualiza_matrizes) {
  LigacaoComObjetoVertices(0);
  DesenhaElementosComAtributos(
      modo, vbo.NumVertices(), vbo.NumDimensoes(), vbo.indices().data(), vbo.coordenadas().data(),
      vbo.tem_normais(), vbo.normais().data(), 0,
      vbo.tem_tangentes(), vbo.tangentes().data(), 0,
      vbo.tem_texturas(), vbo.texturas().data(), 0,
      vbo.tem_cores(), vbo.cores().data(), 0,
      false, nullptr, 0,   // matriz modelagem.
      false, nullptr, 0,   // matriz normal.
      atualiza_matrizes);
}

}  // namespace gl
