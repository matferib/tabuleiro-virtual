// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;

void main() {
  // luz ambiente.
  v_Color = gl_Color * gl_LightModel.ambient;
  // luz direcional.
  //vec4 normal = normalize(vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 1.0f));
  //float tam_normal = 1.0f;
  //vec4 vetor_direcao_luz = normalize(gl_LightSource[0].position);
  //vetor_direcao_luz.w = 1.0f;
  // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
  //float cos_com_normal = dot(normal, vetor_direcao_luz);
  //if (cos_com_normal > 0.0f) {
  //  vec4 c = (gl_Color * gl_LightSource[0].diffuse) * cos_com_normal;
  //  v_Color += c;
  //}

  // Posicao do vertice.
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
