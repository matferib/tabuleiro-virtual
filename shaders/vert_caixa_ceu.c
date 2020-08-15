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
uniform highp mat4 gltab_view;    // view.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute lowp vec2 gltab_texel;
attribute lowp vec4 gltab_cor;
attribute highp mat4 gltab_model_i;

void main() {
  v_Color = gltab_cor;
  // Esta parte eh complicada. O OpenGL monta o cubo de uma maneira bem especifica, mas o sistema de coordenadas difere
  // do meu (ele usa x->direita, y->cima, z->atras, o meu eh x->direita, y->frente, z->cima). Aqui eu converto apenas a coordenada
  // para o mapeamento de cubo, mantendo a posicao original em gl_Position.
  v_Pos_model = mat4(vec4(1, 0, 0, 0), vec4(0, 0, -1, 0), vec4(0, 1, 0, 0), vec4(0, 0, 0, 1)) * gltab_vertice;
  //v_Pos_model = gltab_vertice;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * gltab_view * gltab_model_i * gltab_vertice;
  // Esta linha joga o skybox la pro fundo da cena. Fazendo o z == w, o valor
  // de profundidade sera z / w = 1.0. Assim, o skybox pode ser desenhado por ultimo
  // melhorando a performance, pois o fragmento so sera desenhado onde necessario.
  gl_Position.zw = vec2(gl_Position.w, gl_Position.w);
}
