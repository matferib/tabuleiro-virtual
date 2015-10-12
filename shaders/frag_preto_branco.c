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

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying highp vec4 v_Pos;  // posicao em coordenada de olho.
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa em coordenadas de olho.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp float gltab_textura;               // Textura ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.

void main() {
  lowp vec4 cor_final = v_Color;
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }
  lowp float media = (cor_final.r + cor_final.g + cor_final.b) / 3.0;
  cor_final = vec4(media, media, media, cor_final.a);

  // Limite.
  highp float distancia = length(v_Pos - gltab_nevoa_referencia);
  lowp float peso_nevoa = step(0.0, distancia - gltab_nevoa_dados.y);
  if (peso_nevoa == 1.0) {
    discard;
  }
  gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
}
