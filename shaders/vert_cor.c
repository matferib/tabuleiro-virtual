// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader.
varying vec4 v_Color;

void main() {
  v_Color = gl_Color;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
