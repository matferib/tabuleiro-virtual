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
bool ImprimeSeErro();
}  // namespace gl

#define V_ERRO() do { gl::ImprimeSeErro(); } while (0)

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

void Entidade::DesenhaObjeto(ParametrosDesenho* pd, const float* matriz_shear) {
  DesenhaObjetoProto(proto_, vd_, pd, matriz_shear);
}

void Entidade::DesenhaObjetoComDecoracoes(ParametrosDesenho* pd) {
  gl::CarregaNome(Id());
  // Tem que normalizar por causa das operacoes de escala, que afetam as normais.
  gl::Habilita(GL_NORMALIZE);
  DesenhaObjeto(pd);
  DesenhaDecoracoes(pd);
  gl::Desabilita(GL_NORMALIZE);
}


void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, ParametrosDesenho* pd, const float* matriz_shear) {
  DesenhaObjetoProto(proto, VariaveisDerivadas(), pd, matriz_shear);
}

void Entidade::DesenhaObjetoProto(const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  switch (proto.tipo()) {
    case TE_ENTIDADE:
      DesenhaObjetoEntidadeProto(proto, vd, pd, matriz_shear);
      return;
    case TE_FORMA:
      DesenhaObjetoFormaProto(proto, vd, pd, matriz_shear);
      return;
    case TE_COMPOSTA: {
      DesenhaObjetoCompostoProto(proto, vd, pd, matriz_shear);
      return;
    }
    return;
  }
}

void Entidade::DesenhaObjetoEntidadeProto(
    const EntidadeProto& proto, const VariaveisDerivadas& vd, ParametrosDesenho* pd, const float* matriz_shear) {
  AjustaCor(proto, pd);
  // desenha o cone com NUM_FACES faces com raio de RAIO e altura ALTURA
  const auto& pos = proto.pos();
  if (!proto.has_info_textura() && !proto.has_modelo_3d()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
    gl::DesenhaVbo(g_vbos[VBO_PEAO]);
    return;
  }

  // tijolo da base (altura TAMANHO_LADO_QUADRADO_10).
  if (!proto.morta()) {
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*queda*/,
                (vd.altura_voo == 0.0f)  /*z*/,  // so desloca tijolo se nao estiver voando.
                proto, vd, pd, matriz_shear);
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10 / 2);
    gl::Escala(0.8f, 0.8f, TAMANHO_LADO_QUADRADO_10 / 2);
    if (pd->entidade_selecionada()) {
      gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    }
    gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
  }

  if (proto.has_modelo_3d()) {
    const auto* vbo = vd.m3d->Modelo(proto.modelo_3d().id());
    if (vbo != nullptr) {
      // TODO vbo gravado
      gl::MatrizEscopo salva_matriz;
      MontaMatriz(true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
      gl::DesenhaVbo(*vbo);
      return;
    } else {
      LOG(INFO) << "Modelo3d invalido: " << proto.modelo_3d().id();
    }
  }

  // Moldura da textura.
  bool achatar = (pd->desenha_texturas_para_cima() || proto.achatado()) && !proto.caida();
  gl::MatrizEscopo salva_matriz;
  MontaMatriz(true  /*queda*/, true  /*z*/, proto, vd, pd, matriz_shear);
  // Tijolo da moldura: nao roda selecionado (comentado).
  if (achatar) {
    gl::Translada(0.0, 0.0, TAMANHO_LADO_QUADRADO_10);
    //if (pd->entidade_selecionada()) {
    //  gl::Roda(vd.angulo_disco_selecao_graus, 0, 0, 1.0f);
    //}
    gl::Roda(90.0f, -1.0f, 0.0f, 0.0f);
    gl::Escala(0.8f, 1.0f, 0.8f);
  } else {
    // Moldura da textura: acima do tijolo de base e achatado em Y (longe da camera).
    gl::Translada(0, 0, TAMANHO_LADO_QUADRADO_2 + TAMANHO_LADO_QUADRADO_10);
    float angulo = 0;
    // So desenha a textura de frente pra entidades nao caidas.
    if (pd->texturas_sempre_de_frente() && !proto.caida()) {
      double dx = pos.x() - pd->pos_olho().x();
      double dy = pos.y() - pd->pos_olho().y();
      double r = sqrt(pow(dx, 2) + pow(dy, 2));
      angulo = (acosf(dx / r) * RAD_PARA_GRAUS);
      if (dy < 0) {
        // A funcao asin tem dois resultados mas sempre retorna o positivo [0, PI].
        // Se o vetor estiver nos quadrantes de baixo, inverte o angulo.
        angulo = -angulo;
      }
      gl::Roda(angulo - 90.0f, 0, 0, 1.0);
    }
    gl::MatrizEscopo salva_matriz;
    gl::Escala(1.0f, 0.1f, 1.0f);
    gl::DesenhaVbo(g_vbos[VBO_TIJOLO_BASE]);
  }

  // Tela onde a textura será desenhada face para o sul (nao desenha para sombra).
  GLuint id_textura = pd->desenha_texturas() && proto.has_info_textura() ?
    vd.texturas->Textura(proto.info_textura().id()) : GL_INVALID_VALUE;
  if (matriz_shear == nullptr && id_textura != GL_INVALID_VALUE) {
    gl::Habilita(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, id_textura);
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
  if (!proto_.has_info_textura() && pd->entidade_selecionada()) {
    // Volta pro chao.
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(true  /*queda*/,
                (vd_.altura_voo == 0.0f)  /*z*/,  // so desloca disco se nao estiver voando mais.
                proto_, vd_, pd);
    MudaCor(proto_.cor());
    gl::Roda(vd_.angulo_disco_selecao_graus, 0, 0, 1.0f);
    DesenhaDisco(TAMANHO_LADO_QUADRADO_2, 6);
  }

  // Desenha a barra de vida.
  if (pd->desenha_barra_vida()) {
#if 0
    // Codigo para iluminar barra de vida.
    gl::AtributosEscopo salva_attributos(GL_LIGHTING_BIT | GL_ENABLE_BIT);
    // Luz no olho apontando para a barra.
    const Posicao& pos_olho = pd->pos_olho();
    gl::Luz(GL_LIGHT0, GL_DIFFUSE, COR_BRANCA);
    const auto& pos = proto_.pos();
    GLfloat pos_luz[] = { pos_olho.x() - pos.x(), pos_olho.y() - pos.y(), pos_olho.z() - pos.z(), 0.0f };
    gl::Luz(GL_LIGHT0, GL_POSITION, pos_luz);
#endif

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
        int l, a;
        gl::TamanhoFonte(&l, &a);
        gl::Translada(0.0f, 0.0f, 0.4f);
        gl::PosicaoRaster(0.0f, 0.0f, 0.0f);
        gl::DesenhaString(descricao, true  /*inverte vertical*/);
      }
    }
  }

  // Efeitos.
  if (pd->desenha_efeitos_entidades()) {
    DesenhaEfeitos(pd);
  }

  if (pd->desenha_rotulo() || pd->desenha_rotulo_especial()) {
    gl::DesabilitaEscopo salva_luz(GL_LIGHTING);
    gl::MatrizEscopo salva_matriz;
    MontaMatriz(false  /*queda*/, true  /*z*/, proto_, vd_, pd);
    gl::Translada(0.0f, 0.0f, ALTURA * 1.5f + TAMANHO_BARRA_VIDA);
    MudaCor(COR_AMARELA);
    if (pd->desenha_rotulo()) {
      gl::PosicaoRaster(0.0f, 0.0f, 0.0f);
      gl::DesenhaString(proto_.rotulo());
    }
    if (pd->desenha_rotulo_especial()) {
      gl::PosicaoRaster(0.0f, 0.0f, 0.0f);
      std::string rotulo;
      for (const std::string& rotulo_especial : proto_.rotulo_especial()) {
        rotulo += std::string("\n") + rotulo_especial;
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
      gl::DesenhaString(rotulo);
    }
  }
}

void Entidade::DesenhaEfeitos(ParametrosDesenho* pd) {
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
      bool tem_alfa = pd->has_alfa_translucidos();
      if (!tem_alfa) {
        pd->set_alfa_translucidos(0.5f);
      }
      auto* escala_efeito = pd->mutable_escala_efeito();
      escala_efeito->set_x(1.2);
      escala_efeito->set_y(1.2);
      escala_efeito->set_z(1.2);
      DesenhaObjetoProto(proto_, vd_, pd, nullptr);
      if (!tem_alfa) {
        pd->clear_alfa_translucidos();
      }
      pd->clear_escala_efeito();
    }
    break;
    case EFEITO_REFLEXOS: {
      if (!pd->has_alfa_translucidos()) {
        return;
      }
      // Desenha a entidade maior e translucida.
      gl::MatrizEscopo salva_matriz;
      bool tem_alfa = pd->has_alfa_translucidos();
      if (!tem_alfa) {
        pd->set_alfa_translucidos(0.5f);
      }
      // TODO colocar o numero certo por complemento.
      const int num_imagens = efeito_proto.has_complemento() ? efeito_proto.complemento() : 3;
      const float inc_angulo_graus = 360.0 / num_imagens;
      for (int i = 0; i < num_imagens; ++i) {
        pd->mutable_rotacao_efeito()->set_z(i * inc_angulo_graus);
        pd->mutable_translacao_efeito()->set_x(1.0f);
        DesenhaObjetoProto(proto_, vd_, pd, nullptr);
      }
      if (!tem_alfa) {
        pd->clear_alfa_translucidos();
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
    LOG(ERROR) << "Limite de luzes alcançado: " << id_luz;
  } else {
    // Objeto de luz. O quarto componente indica que a luz é posicional.
    // Se for 0, a luz é direcional e os componentes indicam sua direção.
    GLfloat pos_luz[] = { 0, 0, 0, 1.0f };
    gl::Luz(GL_LIGHT0 + id_luz, GL_POSITION, pos_luz);
    const ent::Cor& cor = proto_.luz().cor();
    GLfloat cor_luz[] = { cor.r(), cor.g(), cor.b(), cor.a() };
    gl::Luz(GL_LIGHT0 + id_luz, GL_DIFFUSE, cor_luz);
    gl::Luz(GL_LIGHT0 + id_luz, GL_CONSTANT_ATTENUATION, 0.5f + sinf(vd_.angulo_disco_luz_rad) * 0.1);
    gl::Luz(GL_LIGHT0 + id_luz, GL_QUADRATIC_ATTENUATION, 0.02f);
    gl::Habilita(GL_LIGHT0 + id_luz);
    pd->set_luz_corrente(id_luz + 1);
  }
}

void Entidade::DesenhaAura(ParametrosDesenho* pd) {
  if (!proto_.visivel() && !pd->modo_mestre()) {
    return;
  }
  if (!pd->desenha_aura() || !proto_.has_aura() || proto_.aura() == 0) {
    return;
  }
  gl::MatrizEscopo salva_matriz;
  gl::Translada(X(), Y(), Z() + DeltaVoo(vd_));
  const auto& cor = proto_.cor();
  gl::MudaCor(cor.r(), cor.g(), cor.b(), cor.a() * 0.2f);
  float ent_quadrados = MultiplicadorTamanho();
  if (ent_quadrados < 1.0f) {
    ent_quadrados = 1.0f;
  }
  // A aura estende alem do tamanho da entidade.
  gl::EsferaSolida(
      TAMANHO_LADO_QUADRADO_2 * ent_quadrados + TAMANHO_LADO_QUADRADO * proto_.aura(),
      NUM_FACES, NUM_FACES);
}

void Entidade::DesenhaSombra(ParametrosDesenho* pd, const float* matriz_shear) {
  if (vd_.nao_desenhar && !pd->has_picking_x()) {
    return;
  }
  if (!proto_.visivel() && !pd->modo_mestre() && !proto_.selecionavel_para_jogador()) {
    return;
  }
  gl::Habilita(GL_POLYGON_OFFSET_FILL);
  gl::DesvioProfundidade(-1.0f, -60.0f);
  DesenhaObjeto(pd, matriz_shear);
  gl::Desabilita(GL_POLYGON_OFFSET_FILL);
}

float Entidade::MultiplicadorTamanho() const {
  return CalculaMultiplicador(proto_.tamanho());
}


}  // namespace ent
