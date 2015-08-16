# version 120
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;
varying vec3 v_Normal;

void main() {
  v_Normal = normalize(gl_NormalMatrix * gl_Normal);
  v_Color = gl_Color;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
