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
#define varying in
#endif
#endif

varying highp vec4 v_PosView;
uniform highp float gltab_plano_distante;

void main() {
#if defined(GL_ES)
  //highp float prof = length(v_PosView.xyz / v_PosView.w) / gltab_plano_distante;
  highp float prof = 1.0;
  // codifica como cor.
  highp vec4 v4 = vec4(prof, fract(prof * 256.0), fract(prof * 65536.0), 1.0);
  highp mat4 ajuste = mat4(vec4(1.0, -1.0 / 256.0, -1.0 / 65536.0, 0.0),
                           vec4(0.0, 1.0, -1.0 / 256.0, 0.0),
                           vec4(0.0, 0.0, 1.0, 0.0),
                           vec4(0.0, 0.0, 0.0, 1.0));
  gl_FragColor = ajuste * v4;
  gl_FragDepth = prof;
#else
  gl_FragDepth = length(v_PosView.xyz / v_PosView.w) / gltab_plano_distante;
#endif
}
