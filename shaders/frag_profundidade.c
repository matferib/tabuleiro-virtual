//#version 120

#if defined(GL_ES)
//precision highp float;
//#define lowp highp
//#define mediump highp
#else
#define lowp
#define highp
#define mediump
#endif

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
void main() {
  highp vec4 v4 = vec4(gl_FragCoord.z, fract(gl_FragCoord.z * 256.0), 0.0, 1.0);
  v4.r -= v4.g / 256.0;  // Tira a ultima parte, melhora e muito a precisao.
  gl_FragColor = v4;
  gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
