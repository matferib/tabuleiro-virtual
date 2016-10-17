/** Implementacao dos varios tipos de desenho da classe Entidade. */

#include <algorithm>
#include <cmath>
#include <memory>
#include "ent/constantes.h"
#include "ent/entidade.h"
#include "ent/util.h"
#include "gltab/gl.h"
#include "log/log.h"
#include "m3d/m3d.h"

namespace gl {
bool ImprimeSeErro(const char*);
}  // namespace gl

namespace ent {

namespace {
// Tamanho da barra de vida.
const float TAMANHO_BARRA_VIDA = TAMANHO_LADO_QUADRADO_2;
const float TAMANHO_BARRA_VIDA_2 = TAMANHO_BARRA_VIDA / 2.0f;

}  // namespace

// Eh usada em outros arquivos tambem.
void AjustaCor(const EntidadeProto& proto, const ParametrosDesenho* pd) {
  const auto& cp = proto.cor();
  float cor[4] = { cp.r(), cp.g(), cp.b(), 1.0f };
  if (pd->has_alfa_translucidos()) {
    cor[3] = cp.a() * pd->alfa_translucidos();
  }
  if (pd->entidade_selecionada()) {
    RealcaCor(cor);
  }
  if (proto.morta()) {
    EscureceCor(cor);
  }
  MudaCorAlfa(cor);
}

void Entidade::Desenha(ParametrosDesenho* pd) {
  if (vd_.nao_desenhar && !pd->has_picking_x()) {
    return;
  }
  if (!proto_.visivel() || proto_.cor().a() < 1.0f) {
    // Sera desenhado translucido.
    return;
  }
  DesenhaObjetoComDecoracoes(pd);
  DesenhaEfeitos(pd);
}

void Entidade::DesenhaTranslucido(ParametrosDesenho* pd) {
  if (vd_.nao_desenhar && !pd->has_picking_x()) {
    return;
  }
  bool desenhar_objeto = true;
  if (proto_.visivel()) {
    // Visivel so eh desenhado aqui se a cor for transparente e mesmo assim,
    // nos casos de picking para os jogadores, so se a unidade for selecionavel.
    if (proto_.cor().a() == 1.0f ||
        (pd->has_picking_x() && !pd->modo_mestre() && !proto_.selecionavel_para_jogador())) {
      desenhar_objeto = false;
    }
  } else {
    // Invisivel, so desenha para o mestre independente da cor (sera translucido).
    // Para jogador desenha se for selecionavel.
    if (!pd->modo_mestre() && !proto_.selecionavel_para_jogador()) {
      desenhar_objeto = false;
    }
  }
  // Os efeitos translucidos devem ser desenhados independente do objeto ter cor solida.
  if (desenhar_objeto || (proto_.visivel() && proto_.cor().a() == 1.0f)) {
    DesenhaEfeitos(pd);
  }
  if (desenhar_objeto) {
    DesenhaObjetoComDecoracoes(pd);
  }
}

void Entidade::DesenhaObjeto(ParametrosDesenho* pd) {
  DesenhaObjetoProto(proto_, vd_, pd);
}

void Entidade::DesenhaObjetoComDecoracoes(ParametrosDesenho* pd) {
  try {
    gl::CarregaNome(Id());
  } catch (...) {
    return;
  }
  // Tem que normalizar por causa das operacoes de escala, que afetam as normais.
  gl::Habilita(GL_NORMALIZE);
  DesenhaObjeto(pd);
  DesenhaDecoracoes(pd);
  gl::Desabilita(GL_NORMALIZE);
}

void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, ParametrosDesenho* pd) {
  DesenhaObjetoProto(proto, VariaveisDerivadas(), pd);
}

void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd) {
  switch (proto.tipo()) {
    case TE_ENTIDADE:
      DesenhaObjetoEntidadeProto(proto, vd, pd);
      return;
    case TE_FORMA:
      DesenhaObjetoFormaProto(proto, vd, pd);
      return;
    case TE_COMPOSTA: {
      DesenhaObjetoCompostoProto(proto, vd, pd);
      return;
    }
    return;
  }
}

void Entidade::DesenhaObjetoEntidadeProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd) {
  bool achatar = !proto.has_modelo_3d() && !proto.info_textura().id().empty() &&
                 (pd->desenha_texturas_para_cima() || proto.achatado()) && !proto.caida();
  std::unique_ptr<AlteraBlendEscopo> blend_escopo;
  if (proto.has_modelo_3d()) {
    blend_escopo.reset(new AlteraBlendEscopo(pd, proto.cor()));
  } else {
    AjustaCor(proto, pd);
  }
  if (!achatar || VBO_COM_MODELAGEM) {
#if VBO_COM_MODELAGEM
    vd.vbos_gravados.Desenha();
#else
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);
    Matrix4 m;
    if (!proto.has_modelo_3d() && !proto.info_textura().id().empty()) {
      m.scale(proto.info_textura().largura(), 0.1f, proto.info_textura().altura());
      m.translate(0, 0, (TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO_10) - (1.0f - proto.info_textura().altura()));
    }
    m.rotateZ(vd.angulo_rotacao_textura_graus);
    m = MontaMatrizModelagem(true  /*queda*/, true  /*z*/, proto, vd, pd) * m;
    gl::MultiplicaMatriz(m.get());
    if (proto.has_modelo_3d()) {
      const auto* modelo = vd.m3d->Modelo(proto.modelo_3d().id());
      if (modelo != nullptr) {
        modelo->vbos_nao_gravados.Desenha();
      }
    } else if (!proto.info_textura().id().empty()) {
      gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
    } else {
      gl::DesenhaVbo(g_vbos[VBO_PEAO]);
    }
#endif
  }

  if (proto.has_modelo_3d() || proto.info_textura().id().empty()) {
    return;
  }

#if !VBO_COM_MODELAGEM
  // No caso de VBO com modelagem, o tijolo da base ja esta no modelo.
  // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
  if (!proto.morta()) {
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);
    Matrix4 m;
    if (pd->entidade_selecionada()) {
      m.rotateZ(vd.angulo_disco_selecao_graus);
    }
    m.scale(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
    m.translate(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 4);
    m = MontaMatrizModelagem(true,  // queda.
                             (vd.altura_voo == 0.0f)  /*z*/,  // so desloca tijolo se nao estiver voando.
                              proto, vd, pd) * m;
    gl::MultiplicaMatriz(m.get());
    gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
  }
#endif

  // Tela da textura.
  gl::MatrizEscopo salva_matriz_textura(gl::MATRIZ_AJUSTE_TEXTURA, false);
  if (!achatar) {
    Matrix4 m;
    m.scale(proto.info_textura().largura(), proto.info_textura().altura(), 1.0f);
    m.translate(proto.info_textura().translacao_x(), proto.info_textura().translacao_y(), 0.0f);
    gl::MultiplicaMatriz(m.get());
  }

  gl::MatrizEscopo salva_matriz(GL_MODELVIEW, false);

  MontaMatriz(true  /*queda*/, true  /*z*/, proto, vd, pd);
  if (achatar) {
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10, false);
    gl::Roda(90.0f, -1.0f, 0.0f, 0.0f, false);
    gl::Escala(0.8f, 1.0f, 0.8f, true);
  } else {
    gl::Translada(0, 0, (TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO_10) - (1.0f - proto.info_textura().altura()), false);
    gl::Roda(vd.angulo_rotacao_textura_graus, 0.0f, 0.0f, 1.0f, false);
    gl::Escala(proto.info_textura().largura(), 1.0f, proto.info_textura().altura());
  }
  GLuint id_textura = pd->desenha_texturas() && !proto.info_textura().id().empty() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    gl::Normal(0.0f, -1.0f, 0.0f);
    Cor c;
    c.set_r(1.0f);
    c.set_g(1.0f);
    c.set_b(1.0f);
    c.set_a(pd->has_alfa_translucidos() ? pd->alfa_translucidos() : 1.0f);
    MudaCor(proto.morta() ? EscureceCor(c) : c);
    gl::DesenhaVbo(g_vbos[VBO_TELA_TEXTURA], GL_TRIANGLE_FAN);
    gl::Desabilita(GL_TEXTURE_2D);
  }
}

void Entidade::DesenhaDecoracoes(ParametrosDesenho* pd) {
  if (proto_.tipo() != TE_ENTIDADE) {
    // Apenas entidades tem decoracoes.
    return;
  }
  // Disco da entidade.
  if (proto_.modelo_3d().id().empty() && proto_.info_textura().id().empty() && pd->entidade_selecionada()) {
    // Volta pro chao.
    gl::MatrizEscopo salva_matriz(false);
    MontaMatriz(true  /*queda*/,
                (vd_.altura_voo == 0.0f)  /*z*/,  // so desloca disco se nao estiver voando mais.
                proto_, vd_, pd);
    MudaCor(proto_.cor());
    gl::Roda(vd_.angulo_disco_selecao_graus, 0, 0, 1.0f, false);
    gl::Disco(TAMANHO_LADO_QUADRADO_2, 6);
  }

  // Desenha a barra de vida.
  if (pd->desenha_barra_vida()) {
    gl::MatrizEscopo salva_matriz(false);
    MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * (proto_.achatado() ? 0.5f : 1.5f), false);
    {
      gl::MatrizEscopo salva_matriz(false);
      gl::Escala(0.2f, 0.2f, 1.0f, false);
      MudaCor(COR_VERMELHA);
      gl::CuboSolido(TAMANHO_BARRA_VIDA);
    }
    if (proto_.max_pontos_vida() > 0 && proto_.pontos_vida() > 0) {
      float porcentagem = static_cast<float>(proto_.pontos_vida()) / proto_.max_pontos_vida();
      float tamanho_barra = TAMANHO_BARRA_VIDA * porcentagem;
      float delta = -TAMANHO_BARRA_VIDA_2 + (tamanho_barra / 2.0f);
      gl::Translada(0, 0, delta, false);
      gl::Escala(0.3f, 0.3f, porcentagem, false);
      gl::HabilitaEscopo habilita_offset(GL_POLYGON_OFFSET_FILL);
      gl::DesvioProfundidade(0, -25.0);
      MudaCor(COR_VERDE);
      gl::CuboSolido(TAMANHO_BARRA_VIDA);
    }
  }

  // Eventos.
  if (pd->desenha_eventos_entidades()) {
    bool ha_evento = false;
    std::string descricao;
    int num_descricoes = 0;
    for (auto& e : *proto_.mutable_evento()) {
      if (e.rodadas() == 0) {
        ha_evento = true;
        if (!e.descricao().empty()) {
          descricao += e.descricao() + "\n";
          ++num_descricoes;
        }
      }
    }
    if (ha_evento) {
      // Eventos na quinta posicao da pilha (ja tem tabuleiro e entidades aqui).
      gl::TipoEscopo nomes_eventos(OBJ_EVENTO_ENTIDADE, OBJ_ENTIDADE);
      gl::CarregaNome(Id());
      gl::DesabilitaEscopo de(GL_LIGHTING);
      MudaCor(COR_AMARELA);
      gl::MatrizEscopo salva_matriz(false);
      MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
      gl::Translada(pd->desenha_barra_vida() ? 0.5f : 0.0f, 0.0f, ALTURA * 1.5f, false);
      gl::EsferaSolida(0.2f, 4, 2);
      gl::Translada(0.0f, 0.0f, 0.3f, false);
      gl::TroncoConeSolido(0, 0.2f, TAMANHO_BARRA_VIDA, 4, 1);
      gl::Translada(0.0f, 0.0f, TAMANHO_BARRA_VIDA, false);
      gl::EsferaSolida(0.2f, 4, 2);
      // Descricao (so quando nao for picking).
      if (!pd->has_picking_x() && !descricao.empty()) {
        gl::DesabilitaEscopo salva_nevoa(GL_FOG);
        gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
        gl::Translada(0.0f, 0.0f, 0.4f);
        if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
          MudaCorAplicandoNevoa(COR_AMARELA, pd);
          gl::DesenhaString(StringSemUtf8(descricao), true  /*inverte vertical*/);
        }
      }
    }
  }

  if (pd->desenha_rotulo() || pd->desenha_rotulo_especial()) {
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * 1.5f + TAMANHO_BARRA_VIDA);
    bool desenhou_rotulo = false;
    if (pd->desenha_rotulo()) {
      gl::DesabilitaEscopo salva_nevoa(GL_FOG);
      gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        MudaCorAplicandoNevoa(COR_AMARELA, pd);
        gl::DesenhaString(StringSemUtf8(proto_.rotulo()), false);
        desenhou_rotulo = true;
      }
    }
    if (pd->desenha_rotulo_especial()) {
      gl::DesabilitaEscopo salva_nevoa(GL_FOG);
      gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
      MudaCorAplicandoNevoa(COR_AMARELA, pd);
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        std::string rotulo;
        for (const std::string& rotulo_especial : proto_.rotulo_especial()) {
          rotulo += (desenhou_rotulo ? std::string("\n") : std::string("")) + rotulo_especial;
        }
        if (proto_.proxima_salvacao() != RS_FALHOU) {
          rotulo += "\nprox. salv.: ";
          switch (proto_.proxima_salvacao()) {
            case RS_MEIO:
              rotulo += "1/2";
              break;
            case RS_QUARTO:
              rotulo += "1/4";
              break;
            case RS_ANULOU:
              rotulo += "ANULA";
              break;
            default:
              rotulo += "VALOR INVALIDO";
          }
        }
        gl::DesenhaString(StringSemUtf8(rotulo));
      }
    }
  }
}

void Entidade::DesenhaEfeitos(ParametrosDesenho* pd) {
  if (!pd->desenha_efeitos_entidades()) {
    return;
  }
  for (const auto& efeito : proto_.evento()) {
    if (!efeito.has_id_efeito() || efeito.id_efeito() == EFEITO_INVALIDO) {
      continue;
    }
    DesenhaEfeito(pd, efeito, vd_.complementos_efeitos[efeito.id_efeito()]);
  }
}

void Entidade::DesenhaEfeito(ParametrosDesenho* pd, const EntidadeProto::Evento& efeito_proto, const ComplementoEfeito& complemento) {
  efeitos_e efeito = static_cast<efeitos_e>(efeito_proto.id_efeito());
  if (efeito == EFEITO_INVALIDO) {
    return;
  }
  switch (efeito) {
    case EFEITO_BORRAR: {
      if (!pd->has_alfa_translucidos()) {
        return;
      }
      // Desenha a entidade maior e translucida.
      gl::MatrizEscopo salva_matriz(false);
      auto* escala_efeito = pd->mutable_escala_efeito();
      escala_efeito->set_x(1.2);
      escala_efeito->set_y(1.2);
      escala_efeito->set_z(1.2);
      DesenhaObjetoProto(proto_, vd_, pd);
      pd->clear_escala_efeito();
    }
    break;
    case EFEITO_REFLEXOS: {
      if (!pd->has_alfa_translucidos()) {
        // So desenha translucido.
        return;
      }
      // Desenha a entidade maior e translucida.
      gl::MatrizEscopo salva_matriz(false);
      // TODO colocar o numero certo por complemento.
      const int num_imagens = efeito_proto.has_complemento() ? efeito_proto.complemento() : 3;
      const float inc_angulo_graus = 360.0 / num_imagens;
      for (int i = 0; i < num_imagens; ++i) {
        pd->mutable_rotacao_efeito()->set_z(i * inc_angulo_graus);
        pd->mutable_translacao_efeito()->set_x(1.0f);
        DesenhaObjetoProto(proto_, vd_, pd);
      }
      pd->clear_rotacao_efeito();
      pd->clear_translacao_efeito();
    }
    break;
    default:
      ;
  }
}

void Entidade::DesenhaLuz(ParametrosDesenho* pd) {
  if (!pd->iluminacao() || !proto_.has_luz()) {
    return;
  }
  if (!proto_.visivel() && !proto_.selecionavel_para_jogador() && !pd->modo_mestre()) {
    return;
  }

  bool achatado = (pd != nullptr && pd->desenha_texturas_para_cima()) || proto_.achatado();
  gl::MatrizEscopo salva_matriz(false);
  if (achatado) {
    // So translada para a posicao do objeto.
    gl::Translada(X(), Y(), Z(), false);
  } else {
    MontaMatriz(true  /*queda*/, true  /*z*/, proto_, vd_, pd);
  }
  // Obtem vetor da camera para o objeto e roda para o objeto ficar de frente para camera.
  Posicao vetor_camera_objeto;
  ComputaDiferencaVetor(Pos(), pd->pos_olho(), &vetor_camera_objeto);
  gl::Roda(VetorParaRotacaoGraus(vetor_camera_objeto), 0.0f, 0.0f, 1.0f, false);

  // Um quadrado para direcao da camera para luz iluminar o proprio objeto.
  gl::Translada(-TAMANHO_LADO_QUADRADO_2, 0.0f, ALTURA + TAMANHO_LADO_QUADRADO_2, false);

  int id_luz = pd->luz_corrente();
  if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
    LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
  } else {
    // Objeto de luz. O quarto componente indica que a luz é posicional.
    // Se for 0, a luz é direcional e os componentes indicam sua direção.
    GLfloat pos_luz[] = { 0, 0, 0, 1.0f };
    ent::Cor cor = proto_.luz().cor();
    if (!proto_.luz().has_cor()) {
      cor.set_r(1.0f);
      cor.set_g(1.0f);
      cor.set_b(1.0f);
      cor.set_a(1.0f);
    }
    float raio = (proto_.luz().has_raio_m() ? proto_.luz().raio_m() : 6.0f) + sinf(vd_.angulo_disco_luz_rad) * 0.02;
    float multiplicador_cor = 1.0f;
    if (pd->tipo_visao() == VISAO_BAIXA_LUMINOSIDADE) {
      raio *= 2.0;
      multiplicador_cor = pd->multiplicador_visao_penumbra();
    }
    gl::LuzPontual(id_luz, pos_luz, cor.r() * multiplicador_cor, cor.g() * multiplicador_cor, cor.b() * multiplicador_cor, raio);
    gl::Habilita(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
}

void Entidade::DesenhaAura(ParametrosDesenho* pd) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  if (!pd->desenha_aura() || !proto_.has_aura_m() || proto_.aura_m() == 0) {
    return;
  }
  gl::MatrizEscopo salva_matriz(false);
  gl::Translada(X(), Y(), Z() + DeltaVoo(vd_), false);
  const auto& cor = proto_.cor();
  gl::MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * 0.2f);
  float ent_quadrados = MultiplicadorTamanho();
  if (ent_quadrados < 1.0f) {
    ent_quadrados = 1.0f;
  }
  // A aura estende alem do tamanho da entidade.
  gl::EsferaSolida(proto_.aura_m(), NUM_FACES, NUM_FACES);
}

float Entidade::MultiplicadorTamanho() const {
  return CalculaMultiplicador(proto_.tamanho());
}


}  // namespace ent
