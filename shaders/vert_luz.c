//# version 110
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader, interpolado dos vertices.
varying vec4 v_Color;
varying vec3 v_Normal;
varying vec4 v_Pos;  // posicao em coordenada de olho.
varying vec2 v_Tex;  // coordenada texel.
uniform mat4 gltab_prm;    // projecao.
uniform mat4 gltab_mvm;    // modelview.
uniform mat3 gltab_nm;     // normal matrix

void main() {
  v_Normal = gltab_nm * normalize(gl_Normal);
  v_Color = gl_Color;
  v_Pos = gltab_mvm * gl_Vertex;
  v_Tex.st = gl_MultiTexCoord0.st;
  gl_Position = gltab_prm * gltab_mvm * gl_Vertex;
}
