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
varying lowp vec3 v_Normal;  // normalizado.
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying highp vec4 v_Pos_model;
uniform bool gltab_oclusao_ligada;          // true se oclusao estiver ligada.
uniform highp float gltab_plano_distante_oclusao;  // distancia do plano de corte distante durante o mapeamento de oclusao.
varying highp vec4 v_Pos_sombra;   // Posicao do fragmento na perspectiva de sombra.
varying highp vec3 v_Pos_oclusao;  // Posicao do fragmento com relacao a primeira pesssoa.
varying highp vec3 v_Pos_luz;      // Posicao do fragmento com relacao a luz.
varying lowp vec2 v_Tex;  // coordenada texel.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform lowp vec4 gltab_cor_mistura_pre_nevoa;      // Mistura antes de aplicar nevoa.
uniform bool gltab_especularidade_ligada;

// Luz ambiente e direcional.
struct InfoLuzDirecional {
  highp vec4 pos;
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
uniform highp samplerCube gltab_unidade_textura_oclusao;   // handler da textura do mapa da oclusao.
uniform highp samplerCube gltab_unidade_textura_luz;       // handler da textura do mapa da luz.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = oclusao, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;      // Ponto de referencia para computar distancia da nevoa em coordenadas de olho.
//uniform mat4 gltab_modelview_camera;     // Matriz de modelagem ponto de vista da camera.
//uniform bool gltab_stencil;              // Stencil ligado?
uniform highp mat3 gltab_nm;     // normal matrix

lowp vec4 CorLuzDirecionalEspecular(in lowp vec3 vetor_referencia, in InfoLuzDirecional luz_direcional) {
  highp vec3 direcao_luz = vec3(luz_direcional.pos * (1 / luz_direcional.pos.w));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_referencia, direcao_luz));
  cos_ref = pow(cos_ref, 8);
  lowp vec4 cor_final = clamp(luz_direcional.cor * cos_ref, 0.0, 1.0);
  return cor_final * step(0.1, luz_direcional.cor.a);
}

lowp vec4 CorLuzDirecionalDifusa(in lowp vec3 vetor_referencia, in InfoLuzDirecional luz_direcional) {
  highp vec3 direcao_luz = vec3(luz_direcional.pos * (1 / luz_direcional.pos.w));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_referencia, direcao_luz));
  lowp vec4 cor_final = clamp(luz_direcional.cor * cos_ref, 0.0, 1.0);
  return cor_final * step(0.1, luz_direcional.cor.a);
}


lowp float Visivel(samplerCube sampler, highp vec3 pos) {
  highp float bias = 0.5;
  highp vec4 texprofcor = textureCube(sampler, pos, 0.0);
  highp float mais_proximo = (texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0));
  //gl_FragColor = vec4(mais_proximo, 0.0, 0.0, 1.0);
  mais_proximo *= gltab_plano_distante_oclusao;
  // Se mais_proximo menor que valor computado, retorna 0.
  return step((length(pos) - bias), mais_proximo);
}

lowp vec4 CorLuzPontualEspecular(in bool testa_visibilidade, in lowp vec3 vetor_referencia, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec4 objeto_luz_homogeneo = (luz.pos / luz.pos.w) - v_Pos;
  highp vec3 objeto_luz = objeto_luz_homogeneo.xyz;
  highp float distancia_objeto_luz = length(objeto_luz);
  lowp float atenuacao =
    // Atenuacao zero ate raio, depois acaba quase repentinamente.
    (1.0 - smoothstep(luz.atributos.r * 4, luz.atributos.r * 4 + 0.1, distancia_objeto_luz));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_referencia, normalize(objeto_luz)));
  //cos_ref = smoothstep(0.95, 0.98, cos_ref);
  cos_ref = pow(cos_ref, 8);
  lowp float visibilidade = testa_visibilidade ? Visivel(gltab_unidade_textura_luz, v_Pos_luz) : 1.0;
  return visibilidade * clamp(luz.cor * cos_ref, 0.0, 1.0) * atenuacao;
}

lowp vec4 CorLuzPontualDifusa(in bool testa_visibilidade, in lowp vec3 vetor_referencia, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec4 objeto_luz_homogeneo = (luz.pos / luz.pos.w) - v_Pos;
  highp vec3 objeto_luz = objeto_luz_homogeneo.xyz;
  highp float distancia_objeto_luz = length(objeto_luz);
  lowp float atenuacao =
    // Sem atenuacao ate raio, depois meia luz ate raio final.
    (0.5 * step(distancia_objeto_luz, luz.atributos.r) + 0.5 * step(distancia_objeto_luz, luz.atributos.r * 2.0));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_referencia, normalize(objeto_luz)));
  lowp float visibilidade = testa_visibilidade ? Visivel(gltab_unidade_textura_luz, v_Pos_luz) : 1.0;
  return visibilidade * clamp(luz.cor * cos_ref, 0.0, 1.0) * atenuacao;
}

void main() {
  lowp vec4 cor_final = v_Color;
  lowp vec4 cor_oclusao = vec4(1.0, 1.0, 1.0, 1.0);
  if (gltab_oclusao_ligada) {
    lowp float visivel = Visivel(gltab_unidade_textura_oclusao, v_Pos_oclusao);
    cor_oclusao = vec4(visivel, visivel, visivel, 1.0);
  }

  if (gltab_textura > 0.0) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
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
    // Se texz menor que valor computado, retorna 0.
    lowp float aplicar_luz_direcional = step((v_Pos_sombra.z - bias), texz);
#endif
    // Outras luzes.
    lowp vec4 uns = vec4(1.0, 1.0, 1.0, 1.0);
    lowp vec3 vetor_referencia;
    if (gltab_especularidade_ligada) {
      // A camera esta em 0, entao o vetor olho objeto e simplesmente o objeto.
      vetor_referencia = normalize(reflect(vec3(v_Pos), v_Normal));
    } else {
      vetor_referencia = v_Normal;
    }
    lowp mat4 cor_luzes_difusas_1 = mat4(aplicar_luz_direcional * CorLuzDirecionalDifusa(vetor_referencia, gltab_luz_direcional),
                                         CorLuzPontualDifusa(true, vetor_referencia, gltab_luzes[0]),
                                         CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[1]),
                                         CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[2]));
    lowp mat4 cor_luzes_difusas_2 = mat4(CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[3]),
                                         CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[4]),
                                         CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[5]),
                                         CorLuzPontualDifusa(false, vetor_referencia, gltab_luzes[6]));
    cor_final *= clamp(gltab_luz_ambiente + cor_luzes_difusas_1 * uns + cor_luzes_difusas_2 * uns, 0.0, 1.0);
    if (gltab_especularidade_ligada) {
      lowp mat4 cor_luzes_especulares_1 = mat4(aplicar_luz_direcional * CorLuzDirecionalEspecular(vetor_referencia, gltab_luz_direcional),
                                               CorLuzPontualEspecular(true, vetor_referencia, gltab_luzes[0]),
                                               CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[1]),
                                               CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[2]));
      lowp mat4 cor_luzes_especulares_2 = mat4(CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[3]),
                                               CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[4]),
                                               CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[5]),
                                               CorLuzPontualEspecular(false, vetor_referencia, gltab_luzes[6]));

      cor_final = clamp(cor_final + cor_luzes_especulares_1 * uns + cor_luzes_especulares_2 * uns, 0.0, 1.0);
    }
  }
  //mediump float gamma_correction = 1.0 / 2.2;
  //mediump vec4 gamma = vec4(gamma_correction, gamma_correction, gamma_correction, 1.0);
  //cor_final = pow(cor_final, gamma);

  // Nevoa.
  cor_final *= gltab_cor_mistura_pre_nevoa * cor_oclusao;
  if (gltab_nevoa_cor.a > 0.0) {
    mediump float distancia_nevoa = length(v_Pos - gltab_nevoa_referencia);
    lowp float peso_nevoa = smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia_nevoa);
    cor_final.rgb = mix(cor_final.rgb, gltab_nevoa_cor.rgb, peso_nevoa);
  }
  gl_FragColor = cor_final;
}
