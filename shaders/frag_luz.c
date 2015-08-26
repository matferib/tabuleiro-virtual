//#version 120

#if defined(GL_ES)
precision mediump float;
#else
#endif

// Varying sao interpoladas da saida do vertex.
varying vec4 v_Color;
varying vec3 v_Normal;
varying vec4 v_Pos;  // Posicao do pixel do fragmento.
varying vec2 v_Tex;  // coordenada texel.

// Luz ambiente e direcional.
struct InfoLuzDirecional {
  vec4 pos;
  vec4 cor;  // alfa indica se esta ligada.
};

// Informacao sobre luzes pontuais. Os atributos sao colocados em vec4 para melhor aproveitamento
// dos uniformes.
struct InfoLuzPontual {
  vec4 pos;  // posicao.
  vec4 cor;  // alfa usado para indicar se esta ligada.
  vec4 atributos;  // r=raio, g=?, b=?, a=?
};

// Uniforms sao constantes durante desenho, setadas no codigo nativo.
uniform bool gltab_luz;                  // Iluminacao ligada?
uniform vec4 gltab_luz_ambiente;         // Cor da luz ambiente.
uniform InfoLuzDirecional gltab_luz_direcional;  // Luz direcional.
//uniform InfoLuzPontual gltab_luzes[7];   // Luzes pontuais.
uniform bool gltab_luzes[7];   // Luzes pontuais.
uniform bool gltab_textura;              // Textura ligada?
uniform sampler2D gltab_unidade_textura; // handler da textura.
uniform bool gltab_nevoa;                // Nevoa ligada?
uniform vec4 gltab_referencia_nevoa;     // Ponto de referencia para computar distancia da nevoa.
//uniform mat4 gltab_modelview_camera;     // Matriz de modelagem ponto de vista da camera.
//uniform bool gltab_stencil;              // Stencil ligado?

void main() {
  //gl_FragColor = vec4(clamp(v_Normal, 0.0, 1.0), 1.0);
  vec4 cor_final = vec4(0, 0, 0, 0);
  float peso_nevoa = 0.0;
  if (gltab_luz) {
    // luz ambiente.
    //cor_final += v_Color * gl_LightModel.ambient;
    cor_final += v_Color * gltab_luz_ambiente;
    //if (gltab_luzes[0]) {
    if (gltab_luz_direcional.cor.a > 0.0) {
      // Converte normal para coordenadas de olho.
      // A luz direcional ja vem em coordenadas de olho.
      //vec3 direcao_luz = vec3(normalize(gl_LightSource[0].position));
      vec3 direcao_luz = vec3(normalize(gltab_luz_direcional.pos));
      // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
      float cos_com_normal = dot(v_Normal, direcao_luz);
      if (cos_com_normal > 0.0) {
        //cor_final += (v_Color * gl_LightSource[0].diffuse) * cos_com_normal;
        cor_final += (v_Color * gltab_luz_direcional.cor) * cos_com_normal;
      }
    }

    // Outras luzes.
    for (int i = 1; i < 7; ++i) {
      if (!gltab_luzes[i]) continue;
      //if (gltab_luzes[i].cor.a == 0.0) continue;
      float atenuacao = 1.0;
      // Vetor objeto luz.
      vec3 objeto_luz = vec3(gl_LightSource[i].position - v_Pos);
      //vec3 objeto_luz = vec3(gltab_luzes[i].pos - v_Pos);
      float tam = length(objeto_luz);
      if (tam > 12.0) {
      //if (tam > (gltab_luzes[i].atributos.r * 2.0f)) {
        continue;
      }
      if (tam > 6.0) {
      //if (tam > gltab_luzes[i].atributos.r) {
        atenuacao = 0.5;
      }

      float cos_com_normal = dot(v_Normal, normalize(objeto_luz));
      if (cos_com_normal > 0.0) {
        vec4 c = (v_Color * gl_LightSource[i].diffuse) * cos_com_normal;
        //vec4 c = (v_Color * gltab_luzes[i].cor) * cos_com_normal;
        cor_final += c * atenuacao;
      }
    }
    cor_final.a = v_Color.a;
    gl_FragColor = clamp(cor_final, 0.0, 1.0);   // Pass the color directly through the pipeline.
  } else {
    cor_final = v_Color;
  }
  if (gltab_textura) {
    cor_final *= texture2D(gltab_unidade_textura, v_Tex.st);
  }
  if (gltab_nevoa) {
    float distancia = length(v_Pos - gltab_referencia_nevoa);
    if (distancia > gl_Fog.end) {
      gl_FragColor = gl_Fog.color;
      return;
    } else if (distancia > gl_Fog.start) {
      float s = (distancia - gl_Fog.start) * gl_Fog.scale;
      gl_FragColor = (cor_final * (1.0 - s)) + (gl_Fog.color * s);
      return;
    }
  }
  gl_FragColor = cor_final;
}
