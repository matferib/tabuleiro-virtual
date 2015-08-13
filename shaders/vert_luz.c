// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;

void main() {
  // luz ambiente.
  v_Color = gl_Color * gl_LightModel.ambient;
  // Converte normal para mundo.
  mat3 normal_para_mundo = transpose(inverse(mat3(gl_ModelViewMatrix)));
  vec3 normal = normalize(normal_para_mundo * gl_Normal);
  vec4 direcao_luz = normalize(gl_LightSource[0].position);
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  float cos_com_normal = dot(normal, direcao_luz);
  if (cos_com_normal > 0.0f) {
    vec4 c = (gl_Color * gl_LightSource[0].diffuse) * cos_com_normal;
    v_Color += c;
  }

  // Posicao do vertice.
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
