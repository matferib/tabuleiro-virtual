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
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp float gltab_textura;               // Textura ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.

void main() {
  lowp vec4 cor_final = v_Color;
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }
  gl_FragColor = cor_final;
}
