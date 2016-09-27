#version ${VERSAO}

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
// Uniformes nao variam por vertice, vem de fora.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_mvm;    // modelview.
uniform mediump vec4 gltab_dados_raster;    // p = tamanho ponto.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;

void main() {
  //v_Color = gltab_cor * vec4(1.0, 0.2, 0.2, 1.0);
  v_Color = gltab_cor;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * gltab_mvm * gltab_vertice;
  gl_PointSize = gltab_dados_raster.p;
}
