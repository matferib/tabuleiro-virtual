#version ${VERSAO}

#if defined(GL_ES)
precision mediump float;
#else
#define lowp
#define highp
#define mediump
#endif

// Macros ${XXX} deverao ser substituidas pelo codigo fonte.
#define USAR_MAPEAMENTO_SOMBRAS ${USAR_MAPEAMENTO_SOMBRAS}

// Luz ambiente e direcional.
struct InfoLuzDirecional {
  lowp vec4 pos;
  lowp vec4 cor;  // alfa indica se esta ligada.
};

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
#if USAR_MAPEAMENTO_SOMBRAS
varying lowp vec4 v_ColorSemDirecional;
#endif
varying mediump vec3 v_Normal;
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying highp vec4 v_Pos_model;
#if USAR_MAPEAMENTO_SOMBRAS
varying highp vec4 v_Pos_sombra;  // Posicao do pixel do fragmento na perspectiva de sombra.
#endif
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform lowp float gltab_textura;                // Textura ligada?
uniform lowp float gltab_textura_cubo;           // Textura cubo ligada?
uniform lowp sampler2D gltab_unidade_textura;    // handler da textura.
uniform lowp samplerCube gltab_unidade_textura_cubo;   // handler da textura.
#if USAR_MAPEAMENTO_SOMBRAS
uniform highp sampler2D gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
uniform InfoLuzDirecional gltab_luz_direcional;  // Luz direcional.
#endif
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa.

void main() {
  lowp vec4 cor_final = v_Color;
#if USAR_MAPEAMENTO_SOMBRAS
  highp float cos_theta = clamp(dot(v_Normal, gltab_luz_direcional.pos.xyz), 0.0, 1.0);
  highp float bias = 0.005 * tan(acos(cos_theta));
  bias = clamp(bias, 0.00, 0.0035);
  lowp vec4 texprofcor = texture2D(gltab_unidade_textura_sombra, v_Pos_sombra.xy);
  lowp float texz = texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0);
  // Possivelmente da pra otimizar com mix aqui pois nao eh variavel uniforme.
  if ((v_Pos_sombra.z - bias) > texz) {
    cor_final = v_ColorSemDirecional;
  }
#endif

  // O if saiu mais barato que o mix.
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }
  if (gltab_textura_cubo > 0.0) {
    cor_final *= textureCube(gltab_unidade_textura_cubo, v_Pos_model.yzx);
  }

  // Nevoa: em cenario sem nevoa, o if saiu bem mais barato. Com nevoa ficou igual.
  //mediump float distancia = length(v_Pos - gltab_nevoa_referencia);
  //lowp float peso_nevoa = step(0.1, gltab_nevoa_cor.a) * smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia);
  //gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
  if (gltab_nevoa_cor.a > 0.0) {
    mediump float distancia = length(v_Pos - gltab_nevoa_referencia);
    lowp float peso_nevoa = smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia);
    cor_final = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
  }
  gl_FragColor = cor_final;
}
