#version ${VERSAO}

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
// Uniformes nao variam por vertice, vem de fora.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_model;    // model.
uniform highp mat4 gltab_view;    // view.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;

varying highp vec4 v_PosView;

void main() {
  v_PosView = gltab_view * gltab_model * gltab_vertice;
  gl_Position = gltab_prm * v_PosView;
}
