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
#endif

// Output pro frag shader, interpolado dos vertices.
varying lowp vec4 v_Color;
varying lowp vec2 v_Tex;  // coordenada texel.
varying highp vec4 v_Pos;  // posicao em coordenada de olho.
varying highp vec3 v_Pos_oclusao;
// Uniformes nao variam por vertice, vem de fora.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_mvm;    // modelview.
uniform highp mat4 gltab_mvm_oclusao;   // modelagem oclusao.
uniform highp mat4 gltab_mvm_ajuste_textura;    // modelagem ajuste textura.
uniform mediump vec4 gltab_dados_raster;    // p = tamanho ponto.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;

void main() {
  v_Color = gltab_cor;
  v_Tex.st = (gltab_mvm_ajuste_textura * vec4(gltab_texel.st, 1.0, 1.0)).st;
  v_Pos = gltab_mvm * gltab_vertice;
  gl_Position = gltab_prm * v_Pos;
  gl_PointSize = gltab_dados_raster.p;
  // Oclusao.
  highp vec4 pos_oclusao = gltab_mvm_oclusao * gltab_vertice;
  v_Pos_oclusao = pos_oclusao.xyz / pos_oclusao.w;
}
