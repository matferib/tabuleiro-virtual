#ifndef GLTAB_GL_VBO_H
#define GLTAB_GL_VBO_H

#include <optional>
#include <utility>
#include <vector>
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
  // Pos multiplica o vbo pela matriz.
  void Multiplica(const Matrix4& m);
  // Fim transformacoes vbo.

  void AtribuiIndices(const unsigned short* dados, unsigned int num_indices);
  void AtribuiIndices(std::vector<unsigned short>* dados);

  void AtribuiCoordenadas(unsigned short num_dimensoes, const float* dados, unsigned int num_coordenadas);
  void AtribuiCoordenadas(unsigned short num_dimensoes, std::vector<float>* dados);
  void AtribuiCoordenadas(unsigned short num_dimensoes, const std::vector<short>& dados);

  void AtribuiNormais(const float* dados);
  void AtribuiNormais(std::vector<float>* dados);

  void AtribuiTangentes(const float* dados);
  void AtribuiTangentes(std::vector<float>* dados);

  void AtribuiTexturas(const float* dados);
  void AtribuiTexturas(std::vector<float>* dados);

  void AtribuiMatriz(const Matrix4& matriz);

  // Atribui a mesma cor a todas coordenadas.
  void AtribuiCor(float r, float g, float b, float a);
  // Cores independentes, como array por vertice.
  void AtribuiCores(const float* cores);
  // Multiplica as cores do objeto pelas cores passadas.
  void MesclaCores(float r, float g, float b, float a);

  // Concatena um vbo a outro, ajustando os indices.
  // @throw caso os objetos nao sejam compativeis.
  void Concatena(const VboNaoGravado& rhs);

  VboNaoGravado ExtraiVboNormais() const;

  // TODO: destruir os dados para liberar memoria.
  std::vector<float> GeraBufferUnico(unsigned int* deslocamento_normais,
                                     unsigned int* deslocamento_tangentes,
                                     unsigned int* deslocamento_cores,
                                     unsigned int* deslocamento_texturas) const;

  unsigned int NumVertices() const {
    return indices_.size();
  }

  unsigned short NumDimensoes() const {
    return num_dimensoes_;
  }

  std::string ParaString(bool completo) const;

  bool tem_normais() const { return !normais_.empty(); }
  bool tem_tangentes() const { return !tangentes_.empty(); }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return !texturas_.empty(); }
  bool tem_matriz() const;

  const std::vector<unsigned short>& indices() const { return indices_; }
  std::vector<float>& coordenadas() { return coordenadas_; }
  std::vector<float>& normais() { return normais_; }
  std::vector<float>& tangentes() { return tangentes_; }
  std::vector<float>& texturas() { return texturas_; }
  std::vector<float>& cores() { return cores_; }
  Matrix4& matriz() { return *matriz_; }
  const std::vector<float>& coordenadas() const { return coordenadas_; }
  const std::vector<float>& normais() const { return normais_; }
  const std::vector<float>& tangentes() const { return tangentes_; }
  const std::vector<float>& texturas() const { return texturas_; }
  const std::vector<float>& cores() const { return cores_; }
  const Matrix4& matriz() const { return *matriz_; }

 private:
  void ArrumaMatrizesNormais();

  std::vector<float> coordenadas_;
  std::vector<float> normais_;
  std::vector<float> tangentes_;
  std::vector<float> cores_;
  std::vector<float> texturas_;
  std::optional<Matrix4> matriz_;
  std::vector<unsigned short> indices_;  // Indices tem seu proprio buffer.
  std::string nome_;
  unsigned short num_dimensoes_ = 0;  // numero de dimensoes por vertice (2 para xy, 3 para xyz, 4 xyzw).
  bool tem_cores_ = false;
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
  unsigned int DeslocamentoTangentes() const { return deslocamento_tangentes_; }
  // Deslocamento em bytes para a primeira coordenada de textura.
  unsigned int  DeslocamentoTexturas() const { return deslocamento_texturas_; }
  // Deslocamento em bytes para a primeira coordenada de cores.
  unsigned int DeslocamentoCores() const { return deslocamento_cores_; }
  // Deslocamento em bytes para a primeira coordenada da matriz.
  unsigned int DeslocamentoMatriz() const { return deslocamento_matriz_; }

  const std::vector<unsigned short>& indices() const { return indices_; }

  GLuint nome_coordenadas() const { return nome_coordenadas_; }
  GLuint nome_indices() const { return nome_indices_; }

  bool tem_normais() const { return tem_normais_; }
  bool tem_tangentes() const { return tem_tangentes_; }
  bool tem_cores() const { return tem_cores_; }
  bool tem_texturas() const { return tem_texturas_; }
  void forca_texturas(bool tem) { tem_texturas_ = tem; }
  bool tem_matriz() const;

  std::string ParaString() const {
#if WIN32 || ANDROID
    return std::string("vbo: ") + nome_;
#else
    return std::string("vbo: ") + nome_ + ", num indices: " + std::to_string(indices_.size()) +
           ", tem_cores: " + (tem_cores_ ? "true" : "false") +
           ", tem_normais: " + (tem_normais_ ? "true" : "false") +
           ", tem_tangentes: " + (tem_tangentes_ ? "true" : "false") +
           ", buffer_unico: " + std::to_string(buffer_unico_.size());
#endif
  }

  const std::string& nome() const { return nome_; }

 private:
  friend class VboNaoGravado;

  std::string nome_;

  std::vector<float> buffer_unico_;  // Buffer unico com coordenadas, normais, cores e texturas. Valido apenas apos geracao.
  std::vector<unsigned short> indices_;  // Indices tem seu proprio buffer.

  // Buffers.
  GLuint nome_coordenadas_ = 0;
  GLuint nome_indices_ = 0;

  unsigned int deslocamento_normais_ = 0;
  unsigned int deslocamento_tangentes_ = 0;
  unsigned int deslocamento_cores_ = 0;
  unsigned int deslocamento_texturas_ = 0;
  unsigned int deslocamento_matriz_ = 0;
  unsigned short num_dimensoes_ = 0;

  bool tem_normais_ = false;
  bool tem_tangentes_ = false;
  bool tem_cores_ = false;
  bool tem_texturas_ = false;
  bool tem_matriz_ = false;

  bool gravado_ = false;
};

/** Conjunto de VBOs nao gravados.
* Esta classe é feita para ser eficiente, portanto todos os operadores e construtores que recebem algo recebem rvalue.
*/
class VbosNaoGravados {
 public:
  VbosNaoGravados() {}
  VbosNaoGravados(VboNaoGravado&& vbo) {
    vbos_.resize(1);
    vbos_[0] = std::move(vbo);
  }
  VbosNaoGravados(std::vector<VboNaoGravado>&& vbos) : vbos_(vbos) {}
  VbosNaoGravados(VbosNaoGravados&& vbos_nao_gravados) : vbos_(vbos_nao_gravados.vbos_) {}
  VbosNaoGravados& operator=(std::vector<VboNaoGravado>&& vbos) {
    vbos_ = std::move(vbos);
    return *this;
  }
  VbosNaoGravados& operator=(VbosNaoGravados&& rhs) {
    vbos_ = std::move(rhs.vbos_);
    return *this;
  }
  VbosNaoGravados& operator=(VboNaoGravado&& vbo_nao_gravado) {
    vbos_.resize(1);
    vbos_[0] = std::move(vbo_nao_gravado);
    return *this;
  }

  // O motivo da funcao ser assim e nao um operator eh evitar que os operadores de copia sejam caros de forma silenciosa.
  void CopiaDe(const VbosNaoGravados& rhs) {
    vbos_ = rhs.vbos_;
  }

  // A concatenacao eh uma operacao cara. Ela tentara colocar objetos no mesmo VBO e caso nao consiga, criara um novo.
  void Concatena(VbosNaoGravados* rhs);
  void Concatena(const VboNaoGravado& rhs);
  void Concatena(VboNaoGravado* rhs);
  void Desenha(GLenum modo = GL_TRIANGLES) const;
  bool Vazio() const { return vbos_.empty(); }
  void Multiplica(const Matrix4& m);
  void AtribuiCor(float r, float g, float b, float a);
  void MesclaCores(float r, float g, float b, float a);
  std::string ParaString(bool completo) const;

 private:
  std::vector<VboNaoGravado> vbos_;
  friend class VbosGravados;
};

/** Conjunto de Vbos gravados. */
class VbosGravados {
 public:
  void Grava(const VbosNaoGravados& vbos_nao_gravados);
  void Desgrava();
  void Desenha() const;
  bool Vazio() const { return vbos_.empty(); }

 private:
  std::vector<VboGravado> vbos_;
};



// Desenha o vbo, assumindo que ele ja tenha sido gravado.
void DesenhaVbo(const VboGravado& vbo, GLenum modo = GL_TRIANGLES, bool atualiza_matrizes = true);
// Desenha o vbo, assumindo que ele nao tenha sido gravado.
void DesenhaVbo(const VboNaoGravado& vbo, GLenum modo = GL_TRIANGLES, bool atualiza_matrizes = true);

//---------------------------------------------------------------------------
// Todos VBOs retornados serao em modo triangulo, para permitir concatenacao.
//---------------------------------------------------------------------------
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

VboNaoGravado VboHemisferioSolido(GLfloat raio, GLint num_fatias, GLint num_tocos);
inline void HemisferioSolido(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  DesenhaVbo(VboHemisferioSolido(raio, num_fatias, num_tocos));
}

VboNaoGravado VboCuboSolido(GLfloat tam_lado);
inline void CuboSolido(GLfloat tam_lado) {
  DesenhaVbo(VboCuboSolido(tam_lado));
}
// Desenha cubo de lado 1.0f, centrado na posicao. Usa VBO.
void CuboUnitario();

VboNaoGravado VboPiramideSolida(GLfloat tam_lado, GLfloat altura);
inline void PiramideSolida(GLfloat tam_lado, GLfloat altura) {
  DesenhaVbo(VboPiramideSolida(tam_lado, altura));
}

// Retangulo cercando a origem.
VboNaoGravado VboRetangulo(GLfloat tam_x, GLfloat tam_y);
inline VboNaoGravado VboRetangulo(GLfloat tam_lado) { return VboRetangulo(tam_lado, tam_lado); }
inline void Retangulo(GLfloat tam_x, GLfloat tam_y) {
  DesenhaVbo(VboRetangulo(tam_x, tam_y));
}
inline void Retangulo(GLfloat tam) {
  DesenhaVbo(VboRetangulo(tam, tam));
}
// Eficiente, usa VBO gravado.
void RetanguloUnitario();

VboNaoGravado VboRetangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
inline void Retangulo(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
  DesenhaVbo(VboRetangulo(x1, y1, x2, y2));
}

VboNaoGravado VboDisco(GLfloat raio, GLfloat num_faces);
inline void Disco(GLfloat raio, GLfloat num_faces) {
  DesenhaVbo(VboDisco(raio, num_faces));
}
// Disco de raio 0,5 (1 diametro) com 12 lados. Eficiente, usa VBO gravado.
void DiscoUnitario();

// Triangulo equilatero, pico para eixo y com a base no y=0.
VboNaoGravado VboTriangulo(float lado);
inline void Triangulo(GLfloat lado) {
  DesenhaVbo(VboTriangulo(lado));
}
// Triangulo equilatero unitario.
void TrianguloUnitario();

VboNaoGravado VboLivre(const std::vector<std::pair<float, float>>& pontos, float largura);
inline void Livre(const std::vector<std::pair<float, float>>& pontos, float largura) {
  DesenhaVbo(VboLivre(pontos, largura));
}

// Retorna o VBO do caractere.
VboNaoGravado VboCaractere(int c);

}  // namespace gl

#endif
