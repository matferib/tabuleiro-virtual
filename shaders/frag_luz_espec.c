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
varying lowp vec3 v_Tangent;  // normalizado.
varying lowp vec3 v_Bitangent;  // normalizado.
varying lowp mat4 v_Matriz_Normal;
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
uniform lowp float gltab_textura_bump;           // Textura ligada? 1.0 : 0.0
uniform lowp float gltab_textura_cubo;           // Textura cubo ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;    // handler da textura.
//uniform lowp sampler2D gltab_unidade_textura_bump;    // handler da textura de bump.
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

//-------------------------------
// Luz distante direcional (sol).
//-------------------------------
lowp vec4 CorLuzDirecionalDifusa(in lowp vec3 normal) {
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(normal, gltab_luz_direcional.pos.xyz));
  lowp vec4 cor_final = clamp(gltab_luz_direcional.cor * cos_ref, 0.0, 1.0);
  return cor_final * step(0.1, gltab_luz_direcional.cor.a);
}

lowp vec4 CorLuzDirecionalEspecular(
    in lowp vec3 vetor_olho_objeto_refletido) {
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_olho_objeto_refletido, gltab_luz_direcional.pos.xyz));
  cos_ref = pow(cos_ref, 8.0);
  lowp vec4 cor_final = clamp(gltab_luz_direcional.cor * cos_ref, 0.0, 1.0);
  return cor_final * step(0.1, gltab_luz_direcional.cor.a);
}

//-------------
// Demais luzes
//--------------
lowp vec4 CorLuzPontualDifusa(
    in lowp float atenuacao, in lowp vec3 normal, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec4 objeto_luz_homogeneo = (luz.pos / luz.pos.w) - v_Pos;
  highp vec3 objeto_luz = objeto_luz_homogeneo.xyz;
  highp float distancia_objeto_luz = length(objeto_luz);
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(normal, normalize(objeto_luz)));
  return luz.cor * cos_ref * atenuacao;
}

lowp vec4 CorLuzPontualEspecular(
    in lowp float atenuacao, in lowp vec3 vetor_olho_objeto_refletido, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec4 objeto_luz_homogeneo = (luz.pos / luz.pos.w) - v_Pos;
  highp vec3 objeto_luz = objeto_luz_homogeneo.xyz;
  highp float distancia_objeto_luz = length(objeto_luz);
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_ref = max(0.0, dot(vetor_olho_objeto_refletido, normalize(objeto_luz)));
  //cos_ref = smoothstep(0.95, 0.98, cos_ref);
  cos_ref = pow(cos_ref, 8.0);
  return atenuacao * cos_ref * luz.cor * cos_ref;
}

//---------------------------
// Funcoes auxiliares de luz.
//---------------------------
lowp float AtenuacaoLuz(in mediump float distancia_objeto_luz, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return 0.0;
  lowp float distancia_pos_raio = max(0.0, distancia_objeto_luz - luz.atributos.r);
  return 1.0 - smoothstep(0.0, luz.atributos.r, distancia_pos_raio);
  // Algoritmo real: atenuacao quadratuca apos raio.
    //1.0 / (1.0 + pow(distancia_pos_raio, 2));
  // Melhor para D&D.
  // Sem atenuacao ate raio, depois meia luz ate raio final.
    //1.0 / (1 + pow(clamp(distancia_objeto_luz - luz.atributos.r, 0, distancia_objeto_luz), 2));
    //(0.5 * step(distancia_objeto_luz, luz.atributos.r) + 0.5 * step(distancia_objeto_luz, luz.atributos.r * 2.0));
}

mediump vec4 VetorLuzObjeto(in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  return (luz.pos / luz.pos.w) - v_Pos;
}

lowp float VisibilidadeLuzDirecional(in lowp vec3 normal) {
  //lowp vec4 cor_luz = gltab_luz_ambiente;
  highp float cos_theta = clamp(dot(normal, gltab_luz_direcional.pos.xyz), 0.0, 1.0);
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
  return aplicar_luz_direcional;
}

// Retorna quao visivel o ponto eh para a luz (mapeamento de sombras).
lowp float Visivel(samplerCube sampler, highp vec3 pos) {
  highp float bias = 0.5;
  highp vec4 texprofcor = textureCube(sampler, pos, 0.0);
  highp float mais_proximo = (texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0));
  //gl_FragColor = vec4(mais_proximo, 0.0, 0.0, 1.0);
  mais_proximo *= gltab_plano_distante_oclusao;
  // Se mais_proximo menor que valor computado, retorna 0.
  //return step((length(pos) - bias), mais_proximo);
  return 1.0 - smoothstep(mais_proximo, mais_proximo + bias, length(pos));
}

void main() {
  lowp vec4 cor_final = v_Color;
  lowp vec3 normal = v_Normal;

  // Se houver oclusao, verifica se o ponto é visivel. Se nao for, corrige a oclusao.
  lowp vec4 cor_oclusao = vec4(1.0, 1.0, 1.0, 1.0);
  if (gltab_oclusao_ligada) {
    lowp float visivel = Visivel(gltab_unidade_textura_oclusao, v_Pos_oclusao);
    cor_oclusao = vec4(visivel, visivel, visivel, 1.0);
  }

  // Aplica textura.
  if (gltab_textura > 0.0) {
    if (gltab_textura_bump > 0.0) {
      highp vec3 desvio = ((vec3(2.0, 2.0, 2.0) * texture2D(gltab_unidade_textura, v_Tex.st).xyz) - vec3(1.0, 1.0, 1.0));
      mediump mat3 tbn = mat3(v_Tangent, v_Bitangent, v_Normal);
      normal = normalize(tbn * desvio);
    } else {
      cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
    }
  }

  // luzes.
  if (gltab_luz_ambiente.a > 0.0) {
    // Auxilia algumas contas.
    lowp vec4 uns = vec4(1.0, 1.0, 1.0, 1.0);

    // Vetores de objeto-luz para cada luz.
    mediump mat4 vetores_luz_objeto_1 = mat4(
        vec4(0.0),  // nao usado.
        VetorLuzObjeto(gltab_luzes[0]),
        VetorLuzObjeto(gltab_luzes[1]),
        VetorLuzObjeto(gltab_luzes[2]));
    mediump mat4 vetores_luz_objeto_2 = mat4(
        VetorLuzObjeto(gltab_luzes[3]),
        VetorLuzObjeto(gltab_luzes[4]),
        VetorLuzObjeto(gltab_luzes[5]),
        VetorLuzObjeto(gltab_luzes[6]));

    // Modulos dos vetores usado para atenuacao.
    mediump vec4 modulos_1 = vec4(
        0.0  /*nao usado*/,
        length(vetores_luz_objeto_1[1]),
        length(vetores_luz_objeto_1[2]),
        length(vetores_luz_objeto_1[3]));
    mediump vec4 modulos_2 = vec4(
        length(vetores_luz_objeto_2[0]),
        length(vetores_luz_objeto_2[1]),
        length(vetores_luz_objeto_2[2]),
        length(vetores_luz_objeto_2[3]));

    // Atenuacoes.
    lowp vec4 atenuacoes_1 = vec4(
        0.0,  // nao usado.
        AtenuacaoLuz(modulos_1[1], gltab_luzes[0]),
        AtenuacaoLuz(modulos_1[2], gltab_luzes[1]),
        AtenuacaoLuz(modulos_1[3], gltab_luzes[2]));
    lowp vec4 atenuacoes_2 = vec4(
        AtenuacaoLuz(modulos_2[0], gltab_luzes[3]),
        AtenuacaoLuz(modulos_2[1], gltab_luzes[4]),
        AtenuacaoLuz(modulos_2[2], gltab_luzes[5]),
        AtenuacaoLuz(modulos_2[3], gltab_luzes[6]));

    // Visibilidade do ponto (mapeamento de sombras).
    // Obs: deixando pronto para quando tiver mais.
    lowp vec4 visibilidades_1 = vec4(
        VisibilidadeLuzDirecional(normal),
        Visivel(gltab_unidade_textura_luz, v_Pos_luz),
        1.0,
        1.0);
    lowp vec4 visibilidades_2 = vec4(
        1.0,
        1.0,
        1.0,
        1.0);

    // As luzes.
    lowp mat4 cor_luzes_difusas_1 = mat4(
        visibilidades_1[0] * CorLuzDirecionalDifusa(normal),
        visibilidades_1[1] * CorLuzPontualDifusa(atenuacoes_1[1], normal, gltab_luzes[0]),
        visibilidades_1[2] * CorLuzPontualDifusa(atenuacoes_1[2], normal, gltab_luzes[1]),
        visibilidades_1[3] * CorLuzPontualDifusa(atenuacoes_1[3], normal, gltab_luzes[2]));
    lowp mat4 cor_luzes_difusas_2 = mat4(
        visibilidades_2[0] * CorLuzPontualDifusa(atenuacoes_2[0], normal, gltab_luzes[3]),
        visibilidades_2[1] * CorLuzPontualDifusa(atenuacoes_2[1], normal, gltab_luzes[4]),
        visibilidades_2[2] * CorLuzPontualDifusa(atenuacoes_2[2], normal, gltab_luzes[5]),
        visibilidades_2[3] * CorLuzPontualDifusa(atenuacoes_2[3], normal, gltab_luzes[6]));

    // Cor difusa e ambiente aplicada.
    cor_final.rgb *= (gltab_luz_ambiente + cor_luzes_difusas_1 * uns + cor_luzes_difusas_2 * uns).rgb;

    // Aplica especularidade.
    if (gltab_especularidade_ligada) {
      lowp vec3 vetor_olho_objeto_refletido = normalize(reflect(vec3(v_Pos), normal));
      lowp mat4 cor_luzes_especulares_1 = mat4(
          visibilidades_1[0] * CorLuzDirecionalEspecular(vetor_olho_objeto_refletido),
          visibilidades_1[1] * CorLuzPontualEspecular(atenuacoes_1[1], vetor_olho_objeto_refletido, gltab_luzes[0]),
          visibilidades_1[2] * CorLuzPontualEspecular(atenuacoes_1[2], vetor_olho_objeto_refletido, gltab_luzes[1]),
          visibilidades_1[3] * CorLuzPontualEspecular(atenuacoes_1[3], vetor_olho_objeto_refletido, gltab_luzes[2]));
      lowp mat4 cor_luzes_especulares_2 = mat4(
          visibilidades_2[0] * CorLuzPontualEspecular(atenuacoes_2[0], vetor_olho_objeto_refletido, gltab_luzes[3]),
          visibilidades_2[1] * CorLuzPontualEspecular(atenuacoes_2[1], vetor_olho_objeto_refletido, gltab_luzes[4]),
          visibilidades_2[2] * CorLuzPontualEspecular(atenuacoes_2[2], vetor_olho_objeto_refletido, gltab_luzes[5]),
          visibilidades_2[3] * CorLuzPontualEspecular(atenuacoes_2[3], vetor_olho_objeto_refletido, gltab_luzes[6]));

      // Usa clamp porque especularidade pode aumentar para acima de 1.
      cor_final.rgb = clamp(cor_final + cor_luzes_especulares_1 * uns + cor_luzes_especulares_2 * uns, 0.0, 1.0).rgb;
    }
  }
  //mediump float gamma_correction = 1.0 / 2.2;
  //mediump vec4 gamma = vec4(gamma_correction, gamma_correction, gamma_correction, 1.0);
  //cor_final = pow(cor_final, gamma);

  // Nevoa.
  cor_final *= (gltab_cor_mistura_pre_nevoa * cor_oclusao);
  if (gltab_nevoa_cor.a > 0.0) {
    mediump float distancia_nevoa = length(v_Pos - gltab_nevoa_referencia);
    lowp float peso_nevoa = smoothstep(gltab_nevoa_dados.x, gltab_nevoa_dados.y, distancia_nevoa);
    cor_final.rgb = mix(cor_final.rgb, gltab_nevoa_cor.rgb, peso_nevoa);
  }
  gl_FragColor = cor_final;
}
