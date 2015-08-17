#version 120

varying vec4 v_Color;
varying vec3 v_Normal;
varying vec4 v_Pos;
uniform bool gltab_luz;
uniform int[gl_MaxLights] gltab_luzes;
uniform ivec4 gltab_viewport;

void main() {
  vec4 cor_final = vec4(0);
  if (gltab_luz) {
    // luz ambiente.
    cor_final += v_Color * gl_LightModel.ambient;
    // Converte normal para coordenadas de olho.
    // A luz direcional ja vem em coordenadas de olho.
    vec3 direcao_luz = vec3(normalize(gl_LightSource[0].position));
    // dot(v1 v2) = cos(angulo) * |v1| * |v2|.
    float cos_com_normal = dot(v_Normal, direcao_luz);
    if (cos_com_normal > 0.0) {
      cor_final += (v_Color * gl_LightSource[0].diffuse) * cos_com_normal;
    }

    // Outras luzes.
    // Coordenadas do fragmento.
    vec4 frag_xyz = v_Pos;

    for (int i = 1; i < gl_MaxLights; ++i) {
      if (gltab_luzes[i] == 0) continue;
      float atenuacao = 1.0f;
      // Vetor objeto luz.
      vec3 objeto_luz = vec3(gl_LightSource[i].position - frag_xyz);
      float tam = length(objeto_luz);
      if (tam > 12.0) {
        continue;
      }
      if (tam > 6.0) {
        atenuacao = 0.5;
      }

      float cos_com_normal = dot(v_Normal, normalize(objeto_luz));
      if (cos_com_normal > 0.0) {
        vec4 c = (v_Color * gl_LightSource[i].diffuse) * cos_com_normal;
        cor_final += c * atenuacao;
      }
    }
    cor_final.a = v_Color.a;
    gl_FragColor = clamp(cor_final, 0, 1);   // Pass the color directly through the pipeline.
    //gl_FragColor = vec4(gltab_luzes[0], gltab_luzes[1], gltab_luzes[2], 1);
  } else {
    gl_FragColor = v_Color;
  }
}
