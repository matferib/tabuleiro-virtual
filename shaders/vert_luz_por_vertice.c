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
  lowp vec3 direcao_luz = vec3(normalize(luz_direcional.pos));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  lowp float cos_com_normal = dot(normal, direcao_luz);
  lowp vec4 cor_final = clamp(luz_direcional.cor * cos_com_normal, 0.0, 1.0);
  return cor_final * step(0.1, luz_direcional.cor.a);
}

lowp vec4 CorLuzPontual(in highp vec4 pos, in lowp vec3 normal, in InfoLuzPontual luz) {
  if (luz.cor.a == 0.0) return vec4(0.0, 0.0, 0.0, 0.0);
  // Vetor objeto luz.
  highp vec3 objeto_luz = vec3(luz.pos - pos);
  highp float tam = length(objeto_luz);
  lowp float atenuacao = 0.5 * step(tam, luz.atributos.r);
  atenuacao += 0.5 * step(tam, luz.atributos.r * 2.0);
  lowp float cos_com_normal = dot(normal, normalize(objeto_luz));
  return clamp(luz.cor * cos_com_normal, 0.0, 1.0) * atenuacao;
}

void main() {
  v_Pos = gltab_mvm * gltab_vertice;
  lowp vec3 normal  = normalize(gltab_nm * gltab_normal);
  lowp vec4 cor_final = gltab_cor;
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
    cor_final *= clamp(cor_luz, 0.0, 1.0);
  }
  v_Normal = normal;
  v_Color = cor_final;
  v_Tex.st = gltab_texel;
  gl_Position = gltab_prm * v_Pos;
}













