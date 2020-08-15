//# version 110
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
// Uniformes nao variam por vertice, vem de fora.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_view;    // view.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;
attribute highp mat4 gltab_model_i;

void main() {
  v_Color = gltab_cor;
  gl_Position = gltab_prm * gltab_view * gltab_model_ * gltab_vertice;
}
