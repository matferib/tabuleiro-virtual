//# version 110
// uniform: variaveis que nao variam durante primitiva.
// attribute: variaveis que variam por vertice.

// Output pro frag shader, interpolado dos vertices.
varying vec4 v_Color;
varying vec3 v_Normal;
varying vec4 v_Pos;  // posicao em coordenada de olho.
uniform mat4 gltab_mvm;    // modelview.
uniform mat3 gltab_nm;    // normal matrix

void main() {
  v_Normal = normalize(gl_NormalMatrix * gl_Normal);
  //v_Normal = normalize(gltab_nm * gl_Normal);
  v_Color = gl_Color;
  //v_Pos = gl_ModelViewMatrix * gl_Vertex;
  v_Pos = gltab_mvm * gl_Vertex;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
