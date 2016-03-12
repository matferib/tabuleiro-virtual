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

// Output pro frag shader, interpolado dos vertices.
varying highp vec4 v_Pos_model;
varying lowp vec2 v_Tex;  // coordenada texel.
varying lowp vec4 v_Color;
// Uniformes nao variam por vertice, vem de fora.
uniform highp mat4 gltab_prm;           // projecao.
uniform highp mat4 gltab_mvm;    // modelview.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute lowp vec2 gltab_texel;
attribute lowp vec4 gltab_cor;

void main() {
  v_Color = gltab_cor;
  v_Pos_model = gltab_vertice;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * gltab_mvm * gltab_vertice;
  gl_Position.zw = vec2(1.0, 1.0);
}
