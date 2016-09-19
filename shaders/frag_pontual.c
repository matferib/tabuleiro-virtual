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

varying highp vec4 v_PosView;
uniform highp float gltab_plano_distante;

void main() {
  gl_FragDepth = length(v_PosView.xyz / v_PosView.w) / 160.0f;
}
