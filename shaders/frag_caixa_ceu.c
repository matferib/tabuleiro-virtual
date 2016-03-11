#version ${VERSAO}

#if defined(GL_ES)
//precision highp float;
//#define lowp highp
//#define mediump highp
#else
//precision highp float;
#define lowp
#define highp
#define mediump
#if __VERSION__ == 130
#define varying in
#endif
#endif

// Macros ${XXX} deverao ser substituidas pelo codigo fonte.

// Varying sao interpoladas da saida do vertex.
varying highp vec4 v_Pos_model;
varying lowp vec2 v_Tex;  // coordenada texel.
varying lowp vec4 v_Color;

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp float gltab_textura;               // Textura ligada? 1.0 : 0.0
uniform lowp float gltab_textura_cubo;          // Textura cubo ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.
uniform highp samplerCube gltab_unidade_textura_cubo;   // handler da textura de cubos.

void main() {
  lowp vec4 cor_final = v_Color;
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  } else if (gltab_textura_cubo > 0.0) {
    cor_final *= textureCube(gltab_unidade_textura_cubo, v_Pos_model.yzx);
  }
  gl_FragColor = cor_final;
}
