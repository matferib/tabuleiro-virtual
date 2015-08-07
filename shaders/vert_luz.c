// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;

void main() {
  v_Color = gl_Color * gl_LightModel.ambient;
  vec4 normal = /*gl_ModelViewMatrix * */normalize(vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 1.0f));
  //vec4 normal = gl_ModelViewMatrix * vec4(0.5f, 0.5f, 0.0, 1.0f);
  //vec4 normal = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  float tam_normal = 1;
  vec4 vetor_direcao_luz = normalize(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
  //vec4 vetor_direcao_luz = gl_LightSource[0].position;  // Luz no infinito, posicao eh inverso da direcao.
  //vec4 vetor_direcao_luz = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  vetor_direcao_luz.w = 1.0f;
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  float cos_com_normal = dot(normal, vetor_direcao_luz);
  if (cos_com_normal > 0.0f) {
    vec4 c = (gl_Color * gl_LightSource[0].diffuse) * cos_com_normal;
    v_Color += c;
  }
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
