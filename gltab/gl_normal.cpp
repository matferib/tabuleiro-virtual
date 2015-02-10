#if WIN32
#include <Windows.h>
#include <Wingdi.h>
#endif

#include <cmath>
#include <vector>
#include <string>
#include "gltab/gl.h"
#include "log/log.h"

namespace gl {

struct ContextoInterno {
 public:
#if WIN32
  PROC pglGenBuffers;
  PROC pglBindBuffer;
  PROC pglDeleteBuffers;
  PROC pglBufferData;
#endif
} g_contexto;

bool ImprimeSeErro() {
  auto erro = glGetError();
  if (erro != GL_NO_ERROR) {
    LOG(ERROR) << "OpenGL Erro: " << gluErrorString(erro);
    return true;
  }
  return false;
}

#define V_ERRO() do { if (ImprimeSeErro()) return; } while (0)
void IniciaGl(int* argcp, char** argv) {
  glutInit(argcp, argv);
#if WIN32
  LOG(INFO) << "pegando ponteiros";
  g_contexto.pglGenBuffers = wglGetProcAddress("glGenBuffers");
  std::string erro;
  if (g_contexto.pglGenBuffers == nullptr) {
    erro = "null glGenBuffers";
  }
  g_contexto.pglDeleteBuffers = wglGetProcAddress("glDeleteBuffers");
  if (g_contexto.pglDeleteBuffers == nullptr) {
    erro = "null glDeleteBuffers";
  }
  g_contexto.pglBufferData = wglGetProcAddress("glBufferData");
  if (g_contexto.pglBufferData == nullptr) {
    erro = "null glBufferData";
  }
  g_contexto.pglBindBuffer = wglGetProcAddress("glBindBuffer");
  if (g_contexto.pglBindBuffer == nullptr) {
    erro = "null glBindBuffer";
  }
  if (!erro.empty()) {
    throw std::logic_error(erro);
  }
#endif
  /*
  LOG(INFO) << "OpenGL: " << (char*)glGetString(GL_VERSION);
  GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
  V_ERRO();
  GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
  V_ERRO();
  std::string codigo_v_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, "vert.c", &codigo_v_shader_str);
  const char* codigo_v_shader = codigo_v_shader_str.c_str();
  glShaderSource(v_shader, 1, &codigo_v_shader, nullptr);
  V_ERRO();
  std::string codigo_f_shader_str;
  arq::LeArquivo(arq::TIPO_SHADER, "frag.c", &codigo_f_shader_str);
  const char* codigo_f_shader = codigo_f_shader_str.c_str();
  glShaderSource(f_shader, 1, &codigo_f_shader, nullptr);
  V_ERRO();
  glCompileShader(v_shader);
  V_ERRO();
  glCompileShader(f_shader);
  V_ERRO();
  GLuint p = glCreateProgram();
  V_ERRO();
  glAttachShader(p, v_shader);
  V_ERRO();
  glAttachShader(p, f_shader);
  V_ERRO();
  glLinkProgram(p);
  V_ERRO();
  glUseProgram(p);
  V_ERRO();
  */
}
#undef V_ERRO

void FinalizaGl() {
#if WIN32
  // Apagar o contexto_interno
#endif
}

// OpenGL normal.
void ConeSolido(GLfloat base, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  glutSolidCone(base, altura, num_fatias, num_tocos);
}

void TroncoConeSolido(GLfloat raio_base, GLfloat raio_topo_original, GLfloat altura, GLint num_fatias, GLint num_tocos) {
  if (raio_base == raio_topo_original) {
    CilindroSolido(raio_base, altura, num_fatias, num_tocos);
    return;
  }
  const int num_vertices_por_fatia = 4;
  const int num_vertices_por_toco = num_vertices_por_fatia * num_fatias;
  const int num_coordenadas_por_toco = num_vertices_por_toco * 3;
  const int num_coordenadas_total = num_coordenadas_por_toco * num_tocos;
  const int num_indices_por_fatia = 6;
  const int num_indices_por_toco = num_indices_por_fatia * num_fatias;
  const int num_indices_total = num_indices_por_toco * num_tocos;

  float coordenadas[num_coordenadas_total];
  float normais[num_coordenadas_total];
  unsigned short indices[num_indices_total];

  float h_delta = altura / num_tocos;
  float h_topo = 0;
  float delta_raio = (raio_base - raio_topo_original) / num_tocos;
  float raio_topo = raio_base;

  float angulo_fatia = (360.0f * GRAUS_PARA_RAD) / num_fatias;
  float cos_fatia = cosf(angulo_fatia);
  float sen_fatia = sinf(angulo_fatia);

  int i_coordenadas = 0;
  int i_indices = 0;
  int coordenada_inicial = 0;
  float v_base[2];
  float v_topo[2] = { 0.0f, raio_base };
  float beta_rad = atanf(altura / (raio_base - raio_topo_original));
  float alfa_rad = (M_PI / 2.0f) - beta_rad;
  float sen_alfa = sinf(alfa_rad);
  float cos_alfa = cosf(alfa_rad);
  for (int t = 1; t <= num_tocos; ++t) {
    float h_base = h_topo;
    h_topo += h_delta;
    // Novas alturas e base.
    v_base[0] = 0.0f;
    v_base[1] = raio_topo;
    raio_topo -= delta_raio;
    v_topo[0] = 0.0f;
    v_topo[1] = raio_topo;
    // Normal: TODO fazer direito.
    float v_normal[3];
    v_normal[0] = 0.0f;
    v_normal[1] = cos_alfa;
    v_normal[2] = sen_alfa;

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

      // As normais.
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

  HabilitaEstadoCliente(GL_VERTEX_ARRAY);
  HabilitaEstadoCliente(GL_NORMAL_ARRAY);
  PonteiroNormais(GL_FLOAT, normais);
  PonteiroVertices(3, GL_FLOAT, coordenadas);
  DesenhaElementos(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned short), GL_UNSIGNED_SHORT, indices);
  DesabilitaEstadoCliente(GL_NORMAL_ARRAY);
  DesabilitaEstadoCliente(GL_VERTEX_ARRAY);

}

void EsferaSolida(GLfloat raio, GLint num_fatias, GLint num_tocos) {
  glutSolidSphere(raio, num_fatias, num_tocos);
}

void CuboSolido(GLfloat tam_lado) {
  glutSolidCube(tam_lado);
}

namespace interno {

// Alinhamento pode ser < 0 esquerda, = 0 centralizado, > 0 direita.
void DesenhaStringAlinhado(const std::string& str, int alinhamento, bool inverte_vertical) {
  GLboolean raster_valido;
  glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &raster_valido);
  if (!raster_valido) {
    return;
  }

  // Le o raster em coordenadas de janela.
  GLint raster_pos[4];
  glGetIntegerv(GL_CURRENT_RASTER_POSITION, raster_pos);
  // Le viewport.
  GLint viewport[4];
  gl::Le(GL_VIEWPORT, viewport);
  int largura = viewport[2], altura = viewport[3];
  int altura_fonte, largura_fonte;
  TamanhoFonte(&largura_fonte, &altura_fonte);

  // Muda para projecao 2D.
  gl::MatrizEscopo salva_matriz_2(GL_PROJECTION);
  gl::CarregaIdentidade();
  gl::Ortogonal(0, largura, 0, altura, 0, 1);
  gl::MatrizEscopo salva_matriz_3(GL_MODELVIEW);
  gl::CarregaIdentidade();
  int x_original = raster_pos[0];
  int y = raster_pos[1];
  std::vector<std::string> str_linhas(interno::QuebraString(str, '\n'));
  for (const std::string& str_linha : str_linhas) {
    if (alinhamento < 0) {
      glRasterPos2i(x_original, y);
    } else if (alinhamento == 0) {
      glRasterPos2i(x_original - (str_linha.size() / 2.0f) * largura_fonte, y);
    } else {
      glRasterPos2i(x_original - (str_linha.size() * largura_fonte), y);
    }
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &raster_valido);
    if (!raster_valido) {
      break;
    }
    for (const char c : str_linha) {
     gl::DesenhaCaractere(c);
    }
    y -= (altura_fonte + 1) * (inverte_vertical ? -1 : 1);
  }
}

}  // namespace

void CilindroSolido(GLfloat raio, GLfloat altura, GLint fatias, GLint tocos) {
  GLUquadric* cilindro = gluNewQuadric();
  gluQuadricOrientation(cilindro, GLU_OUTSIDE);
  gluQuadricNormals(cilindro, GLU_SMOOTH);
  gluQuadricDrawStyle(cilindro, GLU_FILL);
  gluCylinder(cilindro, raio, raio, altura, fatias, tocos);
  gluDeleteQuadric(cilindro);
}

void CilindroSolido(GLfloat raio_base, GLfloat raio_topo, GLfloat altura, GLint fatias, GLint tocos) {
  GLUquadric* cilindro = gluNewQuadric();
  gluQuadricOrientation(cilindro, GLU_OUTSIDE);
  gluQuadricNormals(cilindro, GLU_SMOOTH);
  gluQuadricDrawStyle(cilindro, GLU_FILL);
  gluCylinder(cilindro, raio_base, raio_topo, altura, fatias, tocos);
  gluDeleteQuadric(cilindro);
}

#if WIN32
void GeraBuffers(GLsizei n, GLuint* buffers) {
  ((PFNGLGENBUFFERSPROC)g_contexto.pglGenBuffers)(n, buffers);
}

void LigacaoComBuffer(GLenum target, GLuint buffer) {
  ((PFNGLBINDBUFFERPROC)g_contexto.pglBindBuffer)(target, buffer);
}

void ApagaBuffers(GLsizei n, const GLuint* buffers) {
  ((PFNGLDELETEBUFFERSPROC)g_contexto.pglDeleteBuffers)(n, buffers);
}

void BufferizaDados(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) {
  ((PFNGLBUFFERDATAPROC)g_contexto.pglBufferData)(target, size, data, usage);
}
#endif

}  // namespace gl.
