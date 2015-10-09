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

#define BITS_PROFUNDIDADE 16

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp float gltab_textura;               // Textura ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.

void main() {
  lowp vec4 cor_final = v_Color;
  cor_final *= mix(vec4(1.0), texture2D(gltab_unidade_textura, v_Tex.st), gltab_textura);
#if BITS_PROFUNDIDADE == 8
  gl_FragColor = vec4(cor_final.rgb, gl_FragCoord.z);
#else
  highp vec4 v4 = vec4(cor_final.rg, gl_FragCoord.z, fract(gl_FragCoord.z * 256.0));
  //v4.b -= v4.a / 256.0;  // Tira a ultima parte.
  gl_FragColor = v4;
#endif
}
