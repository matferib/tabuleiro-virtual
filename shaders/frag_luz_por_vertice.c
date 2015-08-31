//#version 120

#if defined(GL_ES)
precision mediump float;
#else
#define lowp
#define highp
#define mediump
#endif

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying mediump vec3 v_Normal;
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform bool gltab_textura;                // Textura ligada?
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa.

void main() {
  lowp vec4 cor_final = v_Color;

  if (gltab_textura) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }

  // Nevoa.
  mediump float distancia = length(v_Pos - gltab_nevoa_referencia);
  lowp float peso_nevoa = step(0.1, gltab_nevoa_cor.a) * smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia);
  gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
}
