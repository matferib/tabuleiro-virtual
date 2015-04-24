#ifndef GLTAB_GL_VBO_H
#define GLTAB_GL_VBO_H

#include "gltab/gl.h"

namespace gl {

/** Vertex Buffer Objects. */
class VboNaoGravado {
 public:
  explicit VboNaoGravado(const std::string& nome = "") : nome_(nome) {}
  VboNaoGravado(VboNaoGravado&& rhs) = default;
  ~VboNaoGravado() {}
  VboNaoGravado& operator=(VboNaoGravado&& vbo) = default;

  void Nomeia(const std::string& nome) {
    nome_ = nome;
  }

  const std::string& nome() const { return nome_; }

  // Transformacao do vbo.
  void Escala(GLfloat x, GLfloat y, GLfloat z);
  void Translada(GLfloat x, GLfloat y, GLfloat z);
  // TODO rodar as normais.
  void RodaX(GLfloat angulo_graus);
  void RodaY(GLfloat angulo_graus);
  void RodaZ(GLfloat angulo_graus);
  // Fim transformacoes vbo.

  void AtribuiIndices(const unsigned short* dados, unsigned short num_indices) {
    indices_.clear();
    indices_.insert(indices_.end(), dados, dados + num_indices);
  }

  void AtribuiCoordenadas(unsigned short num_dimensoes, const float* dados, unsigned short num_coordenadas) {
    coordenadas_.clear();
    coordenadas_.insert(coordenadas_.end(), dados, dados + num_coordenadas);
    num_dimensoes_ = num_dimensoes;
  }

  void AtribuiNormais(const float* dados) {
    normais_.clear();
    normais_.insert(normais_.end(), dados, dados + coordenadas_.size());
    tem_normais_ = true;
  }

  void AtribuiTexturas(const float* dados) {
    texturas_.clear();
    texturas_.insert(texturas_.end(), dados, dados + (coordenadas_.size() * 2) / num_dimensoes_ );
    tem_texturas_ = true;
  }

  // Concatena um vbo a outro, ajustando os indices.
  void Concatena(const VboNaoGravado& rhs) {
    // Coordenadas do primeiro indice apos o ultimo, onde serao inseridos os novos.
    const unsigned short num_coordenadas_inicial = coordenadas_.size() / num_dimensoes_;
    coordenadas_.insert(coordenadas_.end(), rhs.coordenadas_.begin(), rhs.coordenadas_.end());
    normais_.insert(normais_.end(), rhs.normais_.begin(), rhs.normais_.end());
    for (const auto indice : rhs.indices_) {
      indices_.push_back(indice + num_coordenadas_inicial);
    }
  }

  // TODO: destruir os dados para liberar memoria.
  std::vector<float> GeraBufferUnico(unsigned int* deslocamento_normais,
                                     unsigned int* deslocamento_cores,
                                     unsigned int* deslocamento_texturas) const {
    std::vector<float> buffer_unico;
    buffer_unico.clear();
    buffer_unico.insert(buffer_unico.end(), coordenadas_.begin(), coordenadas_.end());
    buffer_unico.insert(buffer_unico.end(), normais_.begin(), normais_.end());
    buffer_unico.insert(buffer_unico.end(), cores_.begin(), cores_.end());
    buffer_unico.insert(buffer_unico.end(), texturas_.begin(), texturas_.end());
    unsigned int pos_final = coordenadas_.size() * sizeof(float);
    if (tem_normais_) {
      *deslocamento_normais = pos_final;
      pos_final += normais_.size() * sizeof(float);
    }
    if (tem_cores_) {
      *deslocamento_cores = pos_final;
      pos_final += cores_.size() * sizeof(float);
    }
    if (tem_texturas_) {
      *deslocamento_texturas = pos_final;
      pos_final += texturas_.size() * sizeof(float);
    }
    return buffer_unico;
  }

  unsigned short NumVertices() const {
    return indices_.size();
  }

  unsigned short num_dimensoes() const {
    return num_dimensoes_;
  }

  bool tem_normais() const { return tem_normais_; }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return tem_texturas_; }

  const std::vector<unsigned short> indices() const { return indices_; }
  std::vector<float>& coordenadas() { return coordenadas_; }
  std::vector<float>& normais() { return normais_; }
  std::vector<float>& texturas() { return texturas_; }
  const std::vector<float>& coordenadas() const { return coordenadas_; }
  const std::vector<float>& normais() const { return normais_; }
  const std::vector<float>& texturas() const { return texturas_; }

 private:
  std::vector<float> coordenadas_;
  std::vector<float> normais_;
  std::vector<float> cores_;
  std::vector<float> texturas_;
  std::vector<unsigned short> indices_;  // Indices tem seu proprio buffer.
  std::string nome_;
  unsigned short num_dimensoes_ = 0;  // numero de dimensoes por vertice (2 para xy, 3 para xyz, 4 xyzw).
  bool tem_normais_ = false;
  bool tem_cores_ = false;
  bool tem_texturas_ = false;
};

// O vbo so esta realmente gravado quando grava for chamado.
class VboGravado {
 public:
  VboGravado() {}
  ~VboGravado() { Desgrava(); }

  // Grava o vbo a partir de um VboNaoGravado.
  void Grava(const VboNaoGravado& vbo);
  // Desgrava se gravado.
  void Desgrava();

  unsigned short NumVertices() const {
    return indices_.size();
  }

  void ApagaBufferUnico() {
    buffer_unico_.clear();
  }

  // Deslocamento em bytes para a primeira coordenada de normal.
  unsigned short DeslocamentoNormais() const {
    if (!tem_normais_) {
      throw std::logic_error(std::string("VBO '") + nome_ + "' sem normal");
    }
    return deslocamento_normais_;
  }
  // Deslocamento em bytes para a primeira coordenada de textura.
  unsigned short DeslocamentoTexturas() const {
    if (!tem_texturas_) {
      throw std::logic_error(std::string("VBO '") + nome_ + "' sem textura");
    }
    return deslocamento_texturas_;
  }
  
  const std::vector<unsigned short> indices() const { return indices_; }

  GLuint nome_coordenadas() const { return nome_coordenadas_; }
  GLuint nome_indices() const { return nome_indices_; }

  bool tem_normais() const { return tem_normais_; }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return tem_texturas_; }

 private:
  friend class VboNaoGravado;

  std::string nome_;

  std::vector<float> buffer_unico_;  // Buffer unico com coordenadas, normais, cores e texturas. Valido apenas apos geracao.
  std::vector<unsigned short> indices_;  // Indices tem seu proprio buffer.

  // Buffers.
  GLuint nome_coordenadas_ = 0;
  GLuint nome_indices_ = 0;

  unsigned int deslocamento_normais_ = 0;
  unsigned int deslocamento_cores_ = 0;
  unsigned int deslocamento_texturas_ = 0;

  bool tem_normais_ = false;
  bool tem_cores_ = false;
  bool tem_texturas_ = false;

  bool gravado_ = false;
};

// Desenha o vbo, assumindo que ele ja tenha sido gravado.
void DesenhaVbo(const VboGravado& vbo, GLenum modo = GL_TRIANGLES);
// Desenha o vbo, assumindo que ele nao tenha sido gravado.
void DesenhaVbo(const VboNaoGravado& vbo, GLenum modo = GL_TRIANGLES);

VboNaoGravado VboCilindroSolido(GLfloat raio, GLfloat altura, GLint fatias, GLint tocos);
inline void CilindroSolido(GLfloat raio, GLfloat altura, GLint fatias, GLint tocos) {
  DesenhaVbo(VboCilindroSolido(raio, altura, fatias, tocos));
}

VboNaoGravado VboTroncoConeSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos);
inline void TroncoConeSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
  DesenhaVbo(VboTroncoConeSolido(raio_base, raio_topo, altura, fatias, tocos));
}

VboNaoGravado VboConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos);
inline void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  DesenhaVbo(VboConeSolido(base, altura, num_fatias, num_tocos));
}

VboNaoGravado VboEsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos);
inline void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  DesenhaVbo(VboEsferaSolida(raio, num_fatias, num_tocos));
}

VboNaoGravado VboCuboSolido(GLfloat tam_lado);
inline void CuboSolido(GLfloat tam_lado) {
  DesenhaVbo(VboCuboSolido(tam_lado));
}

VboNaoGravado VboPiramideSolida(GLfloat tam_lado, GLfloat altura);
inline void PiramideSolida(GLfloat tam_lado, GLfloat altura) {
  DesenhaVbo(VboPiramideSolida(tam_lado, altura));
}

// Retangulo cercando a origem.
VboNaoGravado VboRetangulo(GLfloat tam_lado);
inline void Retangulo(GLfloat tam_lado) {
  DesenhaVbo(VboRetangulo(tam_lado), GL_TRIANGLE_FAN);
}

VboNaoGravado VboDisco(GLfloat raio, GLfloat num_faces);
inline void Disco(GLfloat raio, GLfloat num_faces) {
  DesenhaVbo(VboDisco(raio, num_faces), GL_TRIANGLE_FAN);
}

// Triangulo equilatero, pico para eixo y.
VboNaoGravado VboTriangulo(float lado);
inline void Triangulo(GLfloat lado) {
  DesenhaVbo(VboTriangulo(lado));
}

}  // namespace gl

#endif
