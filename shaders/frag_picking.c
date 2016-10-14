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

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;

// Codifica a cor final tendo dois bytes de id e dois bytes de profundidade.
void main() {
  lowp vec4 cor_final = v_Color;
  highp vec4 v4 = vec4(cor_final.rg, gl_FragCoord.z, fract(gl_FragCoord.z * 256.0));
  v4.b -= v4.a / 256.0;  // Tira a ultima parte, melhora e muito a precisao.
  gl_FragColor = v4;
}
