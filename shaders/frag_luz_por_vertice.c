#version ${VERSAO}

// Macros ${XXX} deverao ser substituidas pelo codigo fonte.

#if defined(GL_EXT_shadow_samplers)
#extension GL_EXT_shadow_samplers : enable
#endif

#if defined(GL_ES)
precision mediump float;
#else
#define lowp
#define highp
#define mediump
#endif

// Luz ambiente e direcional.
struct InfoLuzDirecional {
  lowp vec4 pos;
  lowp vec4 cor;  // alfa indica se esta ligada.
};

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying lowp vec4 v_ColorSemDirecional;
varying mediump vec3 v_Normal;
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying highp vec4 v_Pos_model;
varying highp vec4 v_Pos_sombra;  // Posicao do pixel do fragmento na perspectiva de sombra.
varying highp float v_Bias;
varying lowp vec2 v_Tex;  // coordenada texel.
varying highp vec3 v_Pos_oclusao;  // Posicao do pixel do fragmento com relacao a primeira pesssoa.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform lowp float gltab_textura;                // Textura ligada?
uniform lowp float gltab_textura_cubo;           // Textura cubo ligada?
uniform lowp sampler2D gltab_unidade_textura;    // handler da textura.
#if __VERSION__ == 130 || __VERSION__ == 120 || defined(GL_EXT_shadow_samplers)
uniform highp sampler2DShadow gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
#else
uniform highp sampler2D gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
#endif
uniform highp samplerCube gltab_unidade_textura_oclusao;   // handler da textura do mapa da oclusao.
uniform InfoLuzDirecional gltab_luz_direcional;  // Luz direcional.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa.
uniform bool gltab_oclusao_ligada;          // true se oclusao estiver ligada.
uniform highp float gltab_plano_distante_oclusao;  // distancia do plano de corte distante durante o mapeamento de oclusao.

void main() {
  lowp vec4 cor_oclusao = vec4(1.0, 1.0, 1.0, 1.0);
  if (gltab_oclusao_ligada) {
    highp float bias = 0.5;
    // OpenGL ES 2.0.
    highp vec4 texprofcor = textureCube(gltab_unidade_textura_oclusao, v_Pos_oclusao, 0.0);
    highp float mais_proximo = (texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0));
    mais_proximo *= gltab_plano_distante_oclusao;
    lowp float visivel = sign(mais_proximo - (length(v_Pos_oclusao) - bias));
    cor_oclusao = vec4(visivel, visivel, visivel, 1.0);
  }

#if __VERSION__ == 130
  lowp float aplicar_luz_direcional =
      texture(gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - v_Bias));
#elif __VERSION__ == 120
  lowp float aplicar_luz_direcional =
      shadow2D(gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - v_Bias)).r;
#elif defined(GL_EXT_shadow_samplers)
  lowp float aplicar_luz_direcional =
      shadow2DEXT(gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - v_Bias));
#else
  // OpenGL ES 2.0.
  lowp vec4 texprofcor = texture2D(gltab_unidade_textura_sombra, v_Pos_sombra.xy);
  lowp float texz = texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0);
  lowp float aplicar_luz_direcional = (v_Pos_sombra.z - v_Bias) > texz ? 0.0 : 1.0;
#endif
  lowp vec4 cor_final = mix(v_ColorSemDirecional, v_Color, aplicar_luz_direcional);
  // O if saiu mais barato que o mix.
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }
  cor_final *= cor_oclusao;

  // Nevoa: em cenario sem nevoa, o if saiu bem mais barato. Com nevoa ficou igual.
  //mediump float distancia = length(v_Pos - gltab_nevoa_referencia);
  //lowp float peso_nevoa = step(0.1, gltab_nevoa_cor.a) * smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia);
  //gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
  if (gltab_nevoa_cor.a > 0.0) {
    mediump float distancia_nevoa = length(v_Pos - gltab_nevoa_referencia);
    lowp float peso_nevoa = smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia_nevoa);
    cor_final = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
  }
  gl_FragColor = cor_final;
}
