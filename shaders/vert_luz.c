# version 120
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader, interpolado dos vertices.
varying vec4 v_Color;
varying vec3 v_Normal;
varying vec4 v_Pos;  // posicao em coordenada de olho.
varying vec2 v_Tex;  // posicao da textura.

void main() {
  v_Normal = normalize(gl_NormalMatrix * gl_Normal);
  v_Color = gl_Color;
  v_Pos = gl_ModelViewMatrix * gl_Vertex;
  //v_Tex = gl_MultiTexCoord2;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
