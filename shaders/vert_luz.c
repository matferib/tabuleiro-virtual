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
varying lowp vec3 v_Normal;
varying highp vec4 v_Pos;  // posicao em coordenada de olho.
varying lowp vec2 v_Tex;  // coordenada texel.
// Uniformes nao variam por vertice, vem de fora.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_mvm;    // modelview.
uniform highp mat3 gltab_nm;     // normal matrix
uniform lowp vec4 gltab_dados_raster;  // p = tamanho ponto.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute mediump vec3 gltab_normal;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;

void main() {
  v_Normal = normalize(gltab_nm * gltab_normal);
  v_Color = gltab_cor;
  v_Pos = gltab_mvm * gltab_vertice;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * v_Pos;
  gl_PointSize = gltab_dados_raster.p;
}
