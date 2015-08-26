#ifndef GLTAB_GL_VBO_H
#define GLTAB_GL_VBO_H

#include "gltab/gl.h"

namespace gl {

/** Vertex Buffer Objects. */
class VboNaoGravado {
 public:
  explicit VboNaoGravado(const std::string& nome = "") : nome_(nome) {}

  // Um nome para identificacao do VBO.
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

  void AtribuiIndices(const unsigned short* dados, unsigned int num_indices);

  void AtribuiCoordenadas(unsigned short num_dimensoes, const float* dados, unsigned int num_coordenadas);

  void AtribuiNormais(const float* dados);

  void AtribuiTexturas(const float* dados);

  // Atribui a mesma cor a todas coordenadas.
  void AtribuiCor(float r, float g, float b, float a);

  // Concatena um vbo a outro, ajustando os indices.
  // @throw caso os objetos nao sejam compativeis.
  void Concatena(const VboNaoGravado& rhs);

  // TODO: destruir os dados para liberar memoria.
  std::vector<float> GeraBufferUnico(unsigned int* deslocamento_normais,
                                     unsigned int* deslocamento_cores,
                                     unsigned int* deslocamento_texturas) const;

  unsigned int NumVertices() const {
    return indices_.size();
  }

  unsigned short NumDimensoes() const {
    return num_dimensoes_;
  }

  std::string ParaString() const;

  bool tem_normais() const { return tem_normais_; }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return tem_texturas_; }

  const std::vector<unsigned short> indices() const { return indices_; }
  std::vector<float>& coordenadas() { return coordenadas_; }
  std::vector<float>& normais() { return normais_; }
  std::vector<float>& texturas() { return texturas_; }
  std::vector<float>& cores() { return cores_; }
  const std::vector<float>& coordenadas() const { return coordenadas_; }
  const std::vector<float>& normais() const { return normais_; }
  const std::vector<float>& texturas() const { return texturas_; }
  const std::vector<float>& cores() const { return cores_; }

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
  bool Gravado() const { return gravado_; }

  unsigned int NumVertices() const { return indices_.size(); }

  unsigned short NumDimensoes() const { return num_dimensoes_; }

  void ApagaBufferUnico() {
    buffer_unico_.clear();
  }

  // Deslocamento em bytes para a primeira coordenada de normal.
  unsigned int DeslocamentoNormais() const { return deslocamento_normais_; }
  // Deslocamento em bytes para a primeira coordenada de textura.
  unsigned int  DeslocamentoTexturas() const { return deslocamento_texturas_; }
  // Deslocamento em bytes para a primeira coordenada de cores.
  unsigned int DeslocamentoCores() const { return deslocamento_cores_; }

  const std::vector<unsigned short> indices() const { return indices_; }

  GLuint nome_coordenadas() const { return nome_coordenadas_; }
  GLuint nome_indices() const { return nome_indices_; }

  bool tem_normais() const { return tem_normais_; }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return tem_texturas_; }

  std::string ParaString() const {
#if WIN32 || ANDROID
    return std::string("vbo: ") + nome_;
#else
    return std::string("vbo: ") + nome_ + ", num indices: " + std::to_string(indices_.size()) +
           ", tem_cores: " + (tem_cores_ ? "true" : "false") +
           ", tem_normais: " + (tem_normais_ ? "true" : "false") +
           ", buffer_unico: " + std::to_string(buffer_unico_.size());
#endif
  }

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
  unsigned short num_dimensoes_ = 0;

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
inline void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  gl::MatrizEscopo salva_matriz;
  gl::Translada((x1 + x2) / 2.0f, (y1 + y2) / 2.0f, 0.0f);
  gl::Escala(fabs(x1 - x2), fabs(y1 - y2), 1.0f);
  Retangulo(1.0f);
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
