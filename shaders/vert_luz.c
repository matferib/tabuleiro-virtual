#version ${VERSAO}
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

#if defined(GL_ES)
//precision highp float;
//#define lowp highp
//#define mediump highp
#else
#define lowp
#define highp
#define mediump
#if __VERSION__ == 130
#define varying out
#endif
#endif

// Macros ${XXX} deverao ser substituidas pelo codigo fonte.

// Output pro frag shader, interpolado dos vertices.
varying lowp vec4 v_Color;
varying lowp vec3 v_Normal;
varying lowp vec3 v_Tangent;
varying lowp vec3 v_Bitangent;
varying highp vec4 v_Pos;  // posicao em coordenada de olho.
varying highp vec4 v_Pos_model;
varying highp vec4 v_Pos_sombra;
varying highp vec3 v_Pos_oclusao;
varying highp vec3 v_Pos_luz;
varying lowp vec2 v_Tex;  // coordenada texel.
// Uniformes nao variam por vertice, vem de fora.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform highp mat4 gltab_prm;           // projecao.
uniform highp mat4 gltab_prm_sombra;    // projecao sombra.
uniform highp mat4 gltab_mvm_sombra;    // modelagem sombra.
uniform highp mat4 gltab_mvm_oclusao;   // modelagem oclusao.
uniform highp mat4 gltab_mvm_luz;       // modelagem luz.
uniform highp mat4 gltab_mvm_ajuste_textura;    // modelagem ajuste textura.
uniform highp mat4 gltab_view;          // Matriz de view.
uniform mediump vec4 gltab_dados_raster;  // p = tamanho ponto.
uniform bool gltab_especularidade_ligada;
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute mediump vec3 gltab_normal;
attribute mediump vec3 gltab_tangent;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;
attribute highp mat4 gltab_model_i;
attribute highp mat3 gltab_nm_i;

void main() {
  highp mat3 nm = mat3(gltab_view) * gltab_nm_i;
  v_Normal = normalize(nm * gltab_normal);
  v_Tangent = normalize(nm * gltab_tangent);
  v_Bitangent = cross(v_Normal, v_Tangent);
  v_Color = gltab_cor;
  highp vec4 vertice_mundo = gltab_model_i * gltab_vertice;
  v_Pos = gltab_view * vertice_mundo;
  v_Pos = v_Pos / v_Pos.w;
  v_Pos_model = gltab_vertice;
  v_Tex.st = (gltab_mvm_ajuste_textura * vec4(gltab_texel.st, 1.0, 1.0)).st;
  gl_Position = gltab_prm * v_Pos;
  v_Pos_sombra = gltab_prm_sombra * gltab_mvm_sombra * vertice_mundo;
  // Oclusao.
  highp vec4 pos_oclusao = gltab_mvm_oclusao * vertice_mundo;
  v_Pos_oclusao = pos_oclusao.xyz / pos_oclusao.w;
  // luz pontual.
  highp vec4 pos_luz = gltab_mvm_luz * vertice_mundo;
  v_Pos_luz = pos_luz.xyz / pos_luz.w;

  gl_PointSize = gltab_dados_raster.p;
}
