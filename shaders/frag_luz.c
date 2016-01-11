//#version 120

#if defined(GL_ES)
//precision highp float;
//#define lowp highp
//#define mediump highp
#else
precision highp float;
#define lowp
#define highp
#define mediump
#endif

// Varying sao interpoladas da saida do vertex.
varying lowp vec4 v_Color;
varying lowp vec3 v_Normal;
varying highp vec4 v_Pos;  // Posicao do pixel do fragmento.
varying highp vec4 v_Pos_sombra;  // Posicao do pixel do fragmento na perspectiva de sombra.
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
  highp vec4 pos;  // posicao.
  lowp vec4 cor;  // alfa usado para indicar se esta ligada.
  mediump vec4 atributos;  // r=raio, g=?, b=?, a=?
};

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform InfoLuzDirecional gltab_luz_direcional;  // Luz direcional.
uniform InfoLuzPontual gltab_luzes[7];     // Luzes pontuais.
uniform lowp float gltab_textura;               // Textura ligada? 1.0 : 0.0
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.
uniform highp sampler2D gltab_unidade_textura_sombra;   // handler da textura do mapa da sombra.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
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
  // luz ambiente.
  if (gltab_luz_ambiente.a > 0.0) {
    lowp vec4 cor_luz = gltab_luz_ambiente;
    if ((v_Pos_sombra.z - 0.001) <= texture2D(gltab_unidade_textura_sombra, v_Pos_sombra.xy).z) {
      cor_luz += CorLuzDirecional(v_Normal, gltab_luz_direcional);
    }
    //cor_luz += CorLuzDirecional(v_Normal, gltab_luz_direcional);

    // Outras luzes. O for eh ineficiente.
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[0]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[1]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[2]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[3]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[4]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[5]);
    cor_luz += CorLuzPontual(v_Normal, gltab_luzes[6]);
    cor_final *= clamp(cor_luz, 0.0, 1.0);
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
