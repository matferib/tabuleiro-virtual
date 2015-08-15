# version 120
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;
uniform int gltab_luzes[gl_MaxLights];

void main() {
  // luz ambiente.
  v_Color = gl_Color * gl_LightModel.ambient;
  // Converte normal para coordenadas de olho.
  vec3 normal = normalize(gl_NormalMatrix * gl_Normal);
  // A luz direcional ja vem em coordenadas de olho.
  vec3 direcao_luz = vec3(normalize(gl_LightSource[0].position));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  float cos_com_normal = dot(normal, direcao_luz);
  if (cos_com_normal > 0.0) {
    vec4 c = (gl_Color * gl_LightSource[0].diffuse) * cos_com_normal;
    v_Color += c;
  }

  // TODO Outras luzes.
  vec4 posicao_pixel = gl_ModelViewMatrix * gl_Vertex;
  for (int i = 1; i < gl_MaxLights; ++i) {
    if (gltab_luzes[i] == 0) continue;
    float atenuacao = 1.0f;
    // Vetor objeto luz.
    vec3 objeto_luz = vec3(gl_LightSource[i].position - posicao_pixel);
    float tam = length(objeto_luz);
    if (tam > 6.0) {
      continue;
    }
    if (tam > 3.0) {
      atenuacao = 0.5;
    }

    float cos_com_normal = 1.0;// * dot(normal, normalize(objeto_luz));
    if (cos_com_normal > 0.0) {
      vec4 c = (gl_Color * gl_LightSource[i].diffuse) * cos_com_normal;
      v_Color += c * atenuacao;
    }
  }

  // Posicao do vertice.
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
