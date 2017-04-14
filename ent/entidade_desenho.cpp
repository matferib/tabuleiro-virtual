/** Implementacao dos varios tipos de desenho da classe Entidade. */

#include <algorithm>
#include <cmath>
#include <memory>
#include "ent/constantes.h"
#include "ent/controle_virtual.pb.h"
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

void Entidade::DesenhaObjetoEntidadeProtoComMatrizes(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd,
    const Matrix4& modelagem, const Matrix4& tijolo_base, const Matrix4& tijolo_tela, const Matrix4& tela_textura, const Matrix4& deslocamento_textura) {
  bool achatar = Achatar(proto, pd);
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
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
    if (proto.has_modelo_3d()) {
      const auto* modelo = vd.m3d->Modelo(proto.modelo_3d().id());
      if (modelo != nullptr) {
        GLuint id_textura = pd->desenha_texturas() && !proto.info_textura().id().empty() ?
          vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
        if (id_textura != GL_INVALID_VALUE) {
          gl::Habilita(GL_TEXTURE_2D);
          gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
        }
        gl::MultiplicaMatriz(modelagem.get());
        modelo->vbos_gravados.Desenha();
        gl::Desabilita(GL_TEXTURE_2D);
      }
    } else if (!proto.info_textura().id().empty()) {
      gl::MultiplicaMatriz(tijolo_tela.get());
      gl::DesenhaVbo(g_vbos[VBO_MOLDURA_PECA]);
    } else {
      gl::MultiplicaMatriz(modelagem.get());
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
    gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
    gl::MultiplicaMatriz(tijolo_base.get());
    gl::DesenhaVbo(g_vbos[VBO_BASE_PECA]);
  }
#endif

  // Tela da textura.
  gl::MatrizEscopo salva_matriz(GL_MODELVIEW);
  gl::MultiplicaMatriz(tela_textura.get());

  GLuint id_textura = pd->desenha_texturas() && !proto.info_textura().id().empty() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (id_textura != GL_INVALID_VALUE) {
    // Deslocamento da textura.
    bool ajustar_textura = (deslocamento_textura != Matrix4());
    if (ajustar_textura) {
      gl::MatrizEscopo salva_matriz_textura(gl::MATRIZ_AJUSTE_TEXTURA);
      gl::MultiplicaMatriz(deslocamento_textura.get());
      gl::AtualizaMatrizes();
    }

    gl::Habilita(GL_TEXTURE_2D);
    gl::LigacaoComTextura(GL_TEXTURE_2D, id_textura);
    Cor c;
    c.set_r(1.0f);
    c.set_g(1.0f);
    c.set_b(1.0f);
    c.set_a(pd->has_alfa_translucidos() ? pd->alfa_translucidos() : 1.0f);
    MudaCor(proto.morta() ? EscureceCor(c) : c);
    gl::DesenhaVbo(g_vbos[VBO_TELA_TEXTURA], GL_TRIANGLE_FAN);
    gl::Desabilita(GL_TEXTURE_2D);

    // restaura.
    if (ajustar_textura) {
      gl::MatrizEscopo salva_matriz_textura(gl::MATRIZ_AJUSTE_TEXTURA);
      gl::AtualizaMatrizes();
    }
  }
}


void Entidade::DesenhaObjetoEntidadeProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd) {
  DesenhaObjetoEntidadeProtoComMatrizes(
      proto, vd, pd,
      vd.matriz_modelagem, vd.matriz_modelagem_tijolo_base, vd.matriz_modelagem_tijolo_tela, vd.matriz_modelagem_tela_textura, vd.matriz_deslocamento_textura);
}

void Entidade::DesenhaDecoracoes(ParametrosDesenho* pd) {
  if (proto_.tipo() != TE_ENTIDADE) {
    // Apenas entidades tem decoracoes.
    return;
  }
  // Disco da entidade.
  if (proto_.modelo_3d().id().empty() && proto_.info_textura().id().empty() && pd->entidade_selecionada()) {
    // Volta pro chao.
    gl::MatrizEscopo salva_matriz;
    MudaCor(proto_.cor());
    Matrix4 matriz_disco;
    matriz_disco.rotateZ(vd_.angulo_disco_selecao_graus);
    matriz_disco = vd_.matriz_modelagem_tijolo_base * matriz_disco;
    gl::MultiplicaMatriz(matriz_disco.get());
    gl::Disco(TAMANHO_LADO_QUADRADO_2, 6);
  }

  // Desenha a barra de vida.
  if (pd->desenha_barra_vida()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * (proto_.achatado() ? 0.5f : 1.5f));
    {
      gl::MatrizEscopo salva_matriz;
      gl::Escala(0.2f, 0.2f, 1.0f);
      MudaCor(COR_VERMELHA);
      gl::CuboSolido(TAMANHO_BARRA_VIDA);
    }
    if (proto_.max_pontos_vida() > 0 && proto_.pontos_vida() > 0) {
      float porcentagem = static_cast<float>(proto_.pontos_vida()) / proto_.max_pontos_vida();
      float tamanho_barra = TAMANHO_BARRA_VIDA * porcentagem;
      float delta = -TAMANHO_BARRA_VIDA_2 + (tamanho_barra / 2.0f);
      gl::Translada(0, 0, delta);
      gl::Escala(0.3f, 0.3f, porcentagem);
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
      gl::MatrizEscopo salva_matriz;
      MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
      gl::Translada(pd->desenha_barra_vida() ? 0.5f : 0.0f, 0.0f, ALTURA * 1.5f);
      gl::EsferaSolida(0.2f, 4, 2);
      gl::Translada(0.0f, 0.0f, 0.3f);
      gl::TroncoConeSolido(0, 0.2f, TAMANHO_BARRA_VIDA, 4, 1);
      gl::Translada(0.0f, 0.0f, TAMANHO_BARRA_VIDA);
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
    if (pd->iniciativa_corrente()) {
      gl::TipoEscopo nomes_eventos(OBJ_CONTROLE_VIRTUAL, OBJ_ENTIDADE);
      // Iniciativa.
      gl::CarregaNome(CONTROLE_PROXIMA_INICIATIVA);
      gl::DesabilitaEscopo de(GL_LIGHTING);
      MudaCor(COR_BRANCA);
      gl::MatrizEscopo salva_matriz;
      MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
      float escala = 1.0f + sinf(vd_.angulo_disco_iniciativa_rad) * 0.05;
      gl::Escala(escala, escala, escala);
      gl::Translada(0.0f, 0.0f, pd->desenha_barra_vida() ? ALTURA * 1.5f + TAMANHO_BARRA_VIDA * 1.2f: ALTURA * 1.5f);
      gl::TroncoConeSolido(0, 0.2f, TAMANHO_BARRA_VIDA, 4, 1);
      gl::Translada(0.0f, 0.0f, TAMANHO_BARRA_VIDA / 1.5f);
      gl::TroncoConeSolido(0, 0.2f, TAMANHO_BARRA_VIDA, 4, 1);
    }
  }

  if ((pd->desenha_rotulo() && !proto_.rotulo().empty()) ||
      (pd->desenha_rotulo_especial() && !proto_.rotulo_especial().empty())) {
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * 1.5f + TAMANHO_BARRA_VIDA);
    bool desenhou_rotulo = false;
    if (pd->desenha_rotulo() && !proto_.rotulo().empty()) {
      gl::DesabilitaEscopo salva_nevoa(GL_FOG);
      gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        MudaCorAplicandoNevoa(COR_AMARELA, pd);
        gl::DesenhaString(StringSemUtf8(proto_.rotulo()));
        desenhou_rotulo = true;
      }
    }
    if (pd->desenha_rotulo_especial() && !proto_.rotulo_especial().empty()) {
      gl::DesabilitaEscopo salva_nevoa(GL_FOG);
      gl::DesabilitaEscopo salva_oclusao(gl::OclusaoLigada, gl::Oclusao);
      MudaCorAplicandoNevoa(COR_AMARELA, pd);
      if (gl::PosicaoRaster(0.0f, 0.0f, 0.0f)) {
        std::string rotulo(desenhou_rotulo ? std::string("\n") : std::string(""));
        for (const std::string& rotulo_especial : proto_.rotulo_especial()) {
          rotulo += rotulo_especial + "\n";
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
      gl::MatrizEscopo salva_matriz;
      auto* escala_efeito = pd->mutable_escala_efeito();
      escala_efeito->set_x(1.2);
      escala_efeito->set_y(1.2);
      escala_efeito->set_z(1.2);
      MatrizesDesenho md = GeraMatrizesDesenho(proto_, vd_, pd);
      DesenhaObjetoEntidadeProtoComMatrizes(proto_, vd_, pd, md.modelagem, md.tijolo_base, md.tijolo_tela, md.tela_textura, md.deslocamento_textura);
      pd->clear_escala_efeito();
    }
    break;
    case EFEITO_REFLEXOS: {
      if (!pd->has_alfa_translucidos()) {
        // So desenha translucido.
        return;
      }
      // Desenha a entidade maior e translucida.
      gl::MatrizEscopo salva_matriz;
      // TODO colocar o numero certo por complemento.
      const int num_imagens = efeito_proto.has_complemento() ? efeito_proto.complemento() : 3;
      const float inc_angulo_graus = 360.0 / num_imagens;
      for (int i = 0; i < num_imagens; ++i) {
        pd->mutable_rotacao_efeito()->set_z(i * inc_angulo_graus);
        pd->mutable_translacao_efeito()->set_x(1.0f);
        MatrizesDesenho md = GeraMatrizesDesenho(proto_, vd_, pd);
        DesenhaObjetoEntidadeProtoComMatrizes(proto_, vd_, pd, md.modelagem, md.tijolo_base, md.tijolo_tela, md.tela_textura, md.deslocamento_textura);
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
  gl::MatrizEscopo salva_matriz;
  if (achatado) {
    // So translada para a posicao do objeto.
    gl::Translada(X(), Y(), Z());
  } else {
    MontaMatriz(true  /*queda*/, true  /*z*/, proto_, vd_, pd);
  }
  // Obtem vetor da camera para o objeto e roda para o objeto ficar de frente para camera.
  Posicao vetor_camera_objeto;
  ComputaDiferencaVetor(Pos(), pd->pos_olho(), &vetor_camera_objeto);
  gl::Roda(VetorParaRotacaoGraus(vetor_camera_objeto), 0.0f, 0.0f, 1.0f);

  // Um quadrado para direcao da camera para luz iluminar o proprio objeto.
  gl::Translada(-TAMANHO_LADO_QUADRADO_2, 0.0f, ALTURA + TAMANHO_LADO_QUADRADO_2);

  int id_luz = pd->luz_corrente();
  if (id_luz == 0 || id_luz >= pd->max_num_luzes()) {
    LOG_EVERY_N(WARNING, 1000) << "Limite de luzes alcançado: " << id_luz;
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
  if (!pd->desenha_aura() || proto_.aura_m() == 0) {
    return;
  }
  gl::MatrizEscopo salva_matriz;
  gl::Translada(X(), Y(), Z() + DeltaVoo(vd_));
  const auto& cor = proto_.cor();
  gl::MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * 0.2f);
  float raio = std::max(1.0f, MultiplicadorTamanho()) * TAMANHO_LADO_QUADRADO_2;
  // A aura estende alem do tamanho da entidade.
  gl::EsferaSolida(proto_.aura_m() + raio, NUM_FACES, NUM_FACES);
}

}  // namespace ent
