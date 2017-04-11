#version ${VERSAO}

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

uniform bool gltab_oclusao_ligada;          // true se oclusao estiver ligada.
uniform highp float gltab_plano_distante_oclusao;  // distancia do plano de corte distante durante o mapeamento de oclusao.
varying highp vec3 v_Pos_oclusao;  // Posicao do fragmento com relacao a primeira pesssoa.
uniform highp samplerCube gltab_unidade_textura_oclusao;   // handler da textura do mapa da oclusao.

lowp float Visivel(samplerCube sampler, vec3 pos) {
  highp float bias = 0.5;
  highp vec4 texprofcor = textureCube(sampler, pos, 0.0);
  highp float mais_proximo = (texprofcor.r + (texprofcor.g / 256.0) + (texprofcor.b / 65536.0));
  //gl_FragColor = vec4(mais_proximo, 0.0, 0.0, 1.0);
  mais_proximo *= gltab_plano_distante_oclusao;
  // Se mais_proximo menor que valor computado, retorna 0.
  return step((length(pos) - bias), mais_proximo);
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
  lowp float media = (cor_final.r + cor_final.g + cor_final.b) / 3.0;
  cor_final = vec4(media, media, media, cor_final.a);

  // Limite.
  highp float distancia = length(v_Pos - gltab_nevoa_referencia);
  lowp float peso_nevoa = step(0.001, (distancia - gltab_nevoa_dados.y) * gltab_nevoa_cor.a);
  if (peso_nevoa == 1.0) {
    discard;
  }
  gl_FragColor = mix(cor_final, gltab_nevoa_cor, peso_nevoa) * cor_oclusao;
}
