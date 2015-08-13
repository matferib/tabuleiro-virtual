# version 120
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;

void main() {
  // luz ambiente.
  v_Color = gl_Color * gl_LightModel.ambient;
  // Converte normal para mundo. A gl_NormalMatrix eh a transposta da inversa dos 3x3 superiores da MV (apenas oerientacao, sem translacao).
  vec3 normal_mundo = normalize(gl_NormalMatrix * gl_Normal);
  // A luz direcional ja vem em coordenadas de mundo.
  vec3 direcao_luz = vec3(normalize(gl_LightSource[0].position));
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  float cos_com_normal = dot(normal_mundo, direcao_luz);
  if (cos_com_normal > 0.0) {
    vec4 c = (gl_Color * gl_LightSource[0].diffuse) * cos_com_normal;
    v_Color += c;
  }

  // TODO Outras luzes.

  // Posicao do vertice.
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
