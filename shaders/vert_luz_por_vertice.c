//# version 110
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

#if defined(GL_ES)
precision mediump float;
#else
#define lowp
#define highp
#define mediump
#endif

// Output pro frag shader, interpolado dos vertices.
varying lowp vec4 v_Color;
varying lowp vec3 v_Normal;
varying highp vec4 v_Pos;  // posicao em coordenada de olho.
varying lowp vec2 v_Tex;  // coordenada texel.
// Uniformes nao variam por vertice, vem de fora.
uniform lowp vec4 gltab_luz_ambiente;      // Cor da luz ambiente.
uniform highp mat4 gltab_prm;    // projecao.
uniform highp mat4 gltab_mvm;    // modelview.
uniform mediump mat3 gltab_nm;     // normal matrix
uniform lowp vec4 gltab_dados_raster;  // p = tamanho ponto.
// Atributos variam por vertice.
attribute highp vec4 gltab_vertice;
attribute mediump vec3 gltab_normal;
attribute lowp vec4 gltab_cor;
attribute lowp vec2 gltab_texel;

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
uniform lowp sampler2D gltab_unidade_textura;   // handler da textura.
uniform mediump vec4 gltab_nevoa_dados;            // x = perto, y = longe, z = ?, w = escala.
uniform lowp vec4 gltab_nevoa_cor;              // Cor da nevoa. alfa para presenca.
uniform highp vec4 gltab_nevoa_referencia;       // Ponto de referencia para computar distancia da nevoa.

lowp vec4 CorLuzDirecional(in lowp vec3 normal, in InfoLuzDirecional luz_direcional) {
  // Converte normal para coordenadas de olho.
  // A luz direcional ja vem em coordenadas de olho.
  lowp vec3 direcao_luz = vec3(luz_direcional.pos);
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_com_normal = clamp(dot(normal, direcao_luz), 0.0, 1.0);
  lowp vec4 cor_final = cos_com_normal * luz_direcional.cor;
  return step(0.1, luz_direcional.cor.a) * cor_final;
}

lowp vec4 CorLuzPontual(in highp vec4 pos, in lowp vec3 normal, in InfoLuzPontual luz) {
  // Se eu assumir o pior caso e remover esse if, adicionando step no final, da um ganho de 1% no pior caso,
  // mas fica bem custoso pra casos onde ha menos luzes.
  if (luz.cor.a == 0.0) return vec4(0.0);
  // Vetor objeto luz.
  highp vec3 objeto_luz = vec3(luz.pos - pos);
  highp float distancia = length(objeto_luz);
  // Esse if salva ~6%  de renderizacao com 8 luzes.
  if (distancia > luz.atributos.r * 2.0) {
    return vec4(0.0);
  }
  step(distancia, luz.atributos.r);
  //lowp float atenuacao = (step(distancia, luz.atributos.r) / 2.0) + 0.5;
  lowp float atenuacao = (clamp(step(distancia, luz.atributos.r), 0.0, 0.5)) + 0.5;
  lowp float cos_com_normal = clamp(dot(normal, normalize(objeto_luz)), 0.0, 1.0);
  return cos_com_normal * luz.cor * atenuacao;
}

void main() {
  v_Pos = gltab_mvm * gltab_vertice;
  lowp vec3 normal  = normalize(gltab_nm * gltab_normal);
  lowp vec4 cor_vertice = gltab_cor;
  if (gltab_luz_ambiente.a > 0.0) {
    lowp vec4 cor_luz = gltab_luz_ambiente + CorLuzDirecional(normal, gltab_luz_direcional);
    // Outras luzes. O for eh ineficiente.
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[0]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[1]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[2]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[3]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[4]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[5]);
    cor_luz += CorLuzPontual(v_Pos, normal, gltab_luzes[6]);
    cor_vertice *= clamp(cor_luz, 0.0, 1.0);
  }
  v_Normal = normal;
  v_Color = cor_vertice;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * v_Pos;
  gl_PointSize = gltab_dados_raster.p;
}













