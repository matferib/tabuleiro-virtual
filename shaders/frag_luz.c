#version ${VERSAO}

// Macros ${XXX} deverao ser substituidas pelo codigo fonte.

#if defined(GL_EXT_shadow_samplers)
#extension GL_EXT_shadow_samplers : enable
#endif
#if defined(GL_EXT_gpu_shader4)
#extension GL_EXT_gpu_shader4 : enable
#endif

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

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying lowp vec3 v_Normal;
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying highp vec4 v_Pos_model;
uniform highp float gltab_plano_distante_oclusao;  // distancia do plano de corte distante durante o mapeamento de oclusao.
varying highp vec4 v_Pos_sombra;  // Posicao do pixel do fragmento na perspectiva de sombra.
varying highp vec3 v_Pos_oclusao;  // Posicao do pixel do fragmento com relacao a primeira pesssoa.
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.

// Luz ambiente e direcional.
struct InfoLuzDirecional {
  lowp vec4 pos;
  lowp vec4 cor;  // alfa indica se esta ligada.
};

// Informacao sobre luzes pontuais. Os atributos sao colocados em vec4 para melhor aproveitamento
// dos uniformes.
struct InfoLuzPontual {
  highp vec4 pos;  // posicao em coordenadas de camera.
  lowp vec4 cor;  // alfa usado para indicar se esta ligada.
  mediump vec4 atributos;  // r=raio, g=?, b=?, a=?
};

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform InfoLuzDirecional gltab_luz_direcional;  // Luz direcional.
uniform InfoLuzPontual gltab_luzes[7];           // Luzes pontuais.
uniform lowp float gltab_textura;                // Textura ligada? 1.0 : 0.0
uniform lowp float gltab_textura_cubo;           // Textura cubo ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;    // handler da textura.
#if __VERSION__ == 130 || __VERSION__ == 120 || defined(GL_EXT_shadow_samplers)
uniform highp sampler2DShadow gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
#else
uniform highp sampler2D gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
#endif
#if __VERSION__ == 130 || __VERSION__ == 120 || defined(GL_EXT_gpu_shader4)
uniform highp samplerCube gltab_unidade_textura_oclusao;   // handler da textura do mapa da oclusao.
#else
uniform highp samplerCube gltab_unidade_textura_oclusao;   // handler da textura do mapa da oclusao.
#endif
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = oclusao, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa em coordenadas de olho.
//uniform mat4 gltab_modelview_camera;     // Matriz de modelagem ponto de vista da camera.
//uniform bool gltab_stencil;              // Stencil ligado?

lowp vec4 CorLuzDirecional(in lowp vec3 normal, in InfoLuzDirecional luz_direcional) {
  // Converte normal para coordenadas de olho.
  // A luz direcional ja vem em coordenadas de olho.
  lowp vec3 direcao_luz = vec3(normalize(luz_direcional.pos));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_com_normal = dot(normal, direcao_luz);
  lowp vec4 cor_final = clamp(luz_direcional.cor * cos_com_normal, 0.0, 1.0);
  return cor_final * step(0.1, luz_direcional.cor.a);
}

lowp vec4 CorLuzPontual(in lowp vec3 normal, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec3 objeto_luz = vec3(luz.pos - v_Pos);
  highp float tam = length(objeto_luz);
  lowp float atenuacao = 0.5 * step(tam, luz.atributos.r);
  atenuacao += 0.5 * step(tam, luz.atributos.r * 2.0);
  lowp float cos_com_normal = dot(normal, normalize(objeto_luz));
  return clamp(luz.cor * cos_com_normal, 0.0, 1.0) * atenuacao;
}

void main() {
  lowp vec4 cor_final = v_Color;
  if (gltab_nevoa_dados.z > 0.0) {
    highp float bias = 0.5;
#if __VERSION__ == 130
    //lowp float visivel = texture(gltab_unidade_textura_oclusao, vec4(pos_oclusao.x, pos_oclusao.y, pos_oclusao.z, valor_comparacao - bias), 0.0f);
    highp float mais_proximo = texture(gltab_unidade_textura_oclusao, v_Pos_oclusao).r * gltab_plano_distante_oclusao;
    lowp float visivel = length(v_Pos_oclusao) - bias < mais_proximo ? 1.0f : 0.0f;
#elif __VERSION__ == 120
    lowp float visivel = shadowCube(gltab_unidade_textura_oclusao, vec3(v_Pos_oclusao.xy / v_Pos_oclusao.w, (v_Pos_oclusao.z / v_Pos_oclusao.w) - bias)).r;
//#elif defined(GL_EXT_gpu_shader4)
//    lowp float visivel = shadowCube(
//        gltab_unidade_textura_oclusao, vec4(v_Pos_oclusao.x, v_Pos_oclusao.y, v_Pos_oclusao.z, distancia_projetada - bias));
#else
    // OpenGL ES 2.0.
    highp vec4 texprofcor = textureCube(gltab_unidade_textura_oclusao, v_Pos_oclusao, 0.0);
    highp float mais_proximo = (texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0));
    //gl_FragColor = vec4(mais_proximo, 0.0, 0.0, 1.0);
    mais_proximo *= gltab_plano_distante_oclusao;
    lowp float visivel = length(v_Pos_oclusao) - bias < mais_proximo ? 1.0 : 0.0;
#endif

    if (visivel == 0.0) {
      gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
      return;
    }
  }

  // luz ambiente.
  if (gltab_luz_ambiente.a > 0.0) {
    //lowp vec4 cor_luz = gltab_luz_ambiente;
    highp float cos_theta = clamp(dot(v_Normal, gltab_luz_direcional.pos.xyz), 0.0, 1.0);
    highp float bias = 0.002 * tan(acos(cos_theta));
    bias = clamp(bias, 0.00, 0.0035);
#if __VERSION__ == 130
    lowp float aplicar_luz_direcional = texture(gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - bias));
#elif __VERSION__ == 120
    lowp float aplicar_luz_direcional = shadow2D(gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - bias)).r;
#elif defined(GL_EXT_shadow_samplers)
    lowp float aplicar_luz_direcional = shadow2DEXT(
        gltab_unidade_textura_sombra, vec3(v_Pos_sombra.xy, v_Pos_sombra.z - bias));
#else
    // OpenGL ES 2.0.
    lowp vec4 texprofcor = texture2D(gltab_unidade_textura_sombra, v_Pos_sombra.xy);
    lowp float texz = texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0);
    lowp float aplicar_luz_direcional = (v_Pos_sombra.z - bias) > texz ? 0.0 : 1.0;
#endif
    // Outras luzes.
    lowp vec4 uns = vec4(1.0, 1.0, 1.0, 1.0);
    lowp mat4 cor_luz = mat4(aplicar_luz_direcional * CorLuzDirecional(v_Normal, gltab_luz_direcional),
                             CorLuzPontual(v_Normal, gltab_luzes[0]),
                             CorLuzPontual(v_Normal, gltab_luzes[1]),
                             CorLuzPontual(v_Normal, gltab_luzes[2]));
    lowp mat4 cor_luz_2 = mat4(CorLuzPontual(v_Normal, gltab_luzes[3]),
                               CorLuzPontual(v_Normal, gltab_luzes[4]),
                               CorLuzPontual(v_Normal, gltab_luzes[5]),
                               CorLuzPontual(v_Normal, gltab_luzes[6]));
    cor_final *= clamp(gltab_luz_ambiente + cor_luz * uns + cor_luz_2 * uns, 0.0, 1.0);
  }

  // Ipad da pau usando o mix.
  //cor_final *= mix(vec4(1.0), texture2D(gltab_unidade_textura, v_Tex.st), gltab_textura);
  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }

  //lowp float cor = (cor_final.r + cor_final.g + cor_final.b) / 3.0;
  //cor_final = vec4(cor, cor, cor, cor_final.a);

  // Nevoa.
  highp float distancia = length(v_Pos - gltab_nevoa_referencia);
  lowp float peso_nevoa = step(0.1, gltab_nevoa_cor.a) * smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia);
#if 0
  if (gltab_textura == 0.0) {
    cor_final.r = 0.0;  //v_Pos_sombra.x; //v_Pos.x;
    cor_final.g = v_Pos_sombra.y;
    cor_final.b = 0.0;
    cor_final.a = 1.0;
    //vec2 xy = vec2(v_Pos_sombra.y, 1.0 - v_Pos_sombra.x);
    //cor_final.r = clamp(texture2D(gltab_unidade_textura_sombra, xy).z, 0.0, 1.0);
    //cor_final.g = 0.0; //clamp(texture2D(gltab_unidade_textura_sombra, v_Pos_sombra.xy).z, 0.0, 1.0);
    //cor_final.b = 0.0;
    //cor_final.a = 1.0;
    //if (cor_final.r < cor_final.g){
    //  cor_final = vec4(1.0, 0.0, 0.0, 1.0);
    //} else {
    //  cor_final = vec4(0.0, 1.0, 0.0, 1.0);
    //}
  }
#endif
  gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa);
}
