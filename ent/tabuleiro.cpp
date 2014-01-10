#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <map>
#include <stdexcept>
#include <vector>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "ent/entidade.h"
#include "ent/tabuleiro.h"
#include "ent/tabuleiro.pb.h"
#include "log/log.h"
#include "ntf/notificacao.pb.h"

using namespace ent;

namespace {
/** campo de visao vertical em graus. */
const double CAMPO_VERTICAL = 60.0;

/** altura inicial do olho. */
const double OLHO_ALTURA_INICIAL = 10.0;
/** altura maxima do olho. */
const double OLHO_ALTURA_MAXIMA = 15;
/** altura minima do olho. */
const double OLHO_ALTURA_MINIMA = 1.5;

/** raio (distancia) inicial do olho. */
const double OLHO_RAIO_INICIAL = 20.0;
/** raio maximo do olho. */
const double OLHO_RAIO_MAXIMO = 40.0;
/** raio minimo do olho. */
const double OLHO_RAIO_MINIMO = 1.5;

/** sensibilidade da rodela do mouse. */
const double SENSIBILIDADE_RODA = 0.01;
/** sensibilidade da rotacao lateral do olho. */
const double SENSIBILIDADE_ROTACAO_X = 0.01;
/** sensibilidade da altura do olho. */
const double SENSIBILIDADE_ROTACAO_Y = 0.08;

/** tamanho do lado do quadrado no 3D. */
const double TAMANHO_GL = 1.5;
/** expessura da linha do tabuleiro. */
const double EXPESSURA_LINHA = 0.1;
/** tamanho do lado do quadrado / 2. */
const double TAMANHO_GL_2 = (TAMANHO_GL / 2.0);

/** Altera a cor correnta para cor. */
void MudaCor(GLfloat* cor) {
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cor);
  glColor3fv(cor);
}

/** desenha o quadrado, embaixo preto, acima cinza com pequeno offset para evitar z fight. */
void DesenhaQuadrado(GLuint id, bool selecionado) {
  // desenha o quadrado negro embaixo.
  glLoadName(id);
  GLfloat preto[] = { 0, 0, 0, 1.0 };
  MudaCor(preto);
  glRectf(0, 0, TAMANHO_GL, TAMANHO_GL);

  // Habilita a função pra acabar com zfight.
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(-0.1f, -0.1f);
  if (selecionado) {
    GLfloat cinza[] = { 0.5, 0.5, 0.5, 1.0 };
    MudaCor(cinza);
  } else {
    GLfloat cinza_claro[] = { 0.8, 0.8, 0.8, 1.0 };
    MudaCor(cinza_claro);
  }
  glRectf(0, 0, TAMANHO_GL - EXPESSURA_LINHA, TAMANHO_GL - EXPESSURA_LINHA);
  // Restaura os offset de zfight.
  glDisable(GL_POLYGON_OFFSET_FILL);
}

// TODO: mover para entidade.
// Gera um EntidadeProto com os valores passados.
const EntidadeProto GeraEntidadeProto(int id_cliente, int id_entidade, double x, double y, double z) {
  EntidadeProto ep;
  ep.set_id((id_cliente << 28) | id_entidade);
  auto* pos = ep.mutable_pos();
  pos->set_x(x);
  pos->set_y(y);
  pos->set_z(z);
  // Verde.
  auto* cor = ep.mutable_cor();
  cor->set_r(0);
  cor->set_g(1.0);
  cor->set_b(0);
  return ep;
}

// Busca o hit mais próximo em buffer_hits. Cada posicao do buffer são 3 inteiros:
// - 0: pos_pilha de nomes;
// - 1: componente Z do hits.
// - 2: id do hit.
void BuscaHitMaisProximo(
    unsigned int numero_hits, GLuint* buffer_hits, GLuint* id, GLuint* pos_pilha) {
  VLOG(1) << "numero de hits: " << (unsigned int)numero_hits;
  GLuint* ptr_hits = buffer_hits;
  *id = 0;
  *pos_pilha = 0;
  GLuint menor_z = 0xFFFFFFFF;
  // Busca o hit mais proximo.
  for (GLuint i = 0; i < numero_hits; ++i) {
    if (*(ptr_hits + 1) < menor_z) {
      *pos_pilha = *ptr_hits;
      VLOG(1) << "posicao pilha: " << (unsigned int)(*pos_pilha);
      menor_z = *(ptr_hits+1); 
      // pula ele mesmo, profundidade e ids anteriores na pilha
      ptr_hits += (*pos_pilha + 2);
      *id = *ptr_hits;
      VLOG(1) << "id: " << (unsigned int)(*id);
      ++ptr_hits;
    } else {
      VLOG(1) << "pulando objeto mais longe...";
    }
  }
}


}  // namespace.

Tabuleiro::Tabuleiro(int tamanho, ntf::CentralNotificacoes* central) : 
    tamanho_(tamanho), 
    id_cliente_(0),
    entidade_selecionada_(NULL), 
    quadrado_selecionado_(-1), 
    estado_(ETAB_OCIOSO), proximo_id_entidade_(0), proximo_id_cliente_(1),
    olho_x_(0), olho_y_(0), olho_z_(0), olho_delta_rotacao_(0),
    olho_altura_(OLHO_ALTURA_INICIAL), olho_raio_(OLHO_RAIO_INICIAL),
    central_(central) {
  central_->RegistraReceptor(this);
}

Tabuleiro::~Tabuleiro() {
}

int Tabuleiro::TamanhoX() const { 
  return tamanho_; 
}

int Tabuleiro::TamanhoY() const { 
  return tamanho_; 
}

void Tabuleiro::Desenha() {
  parametros_desenho_.Clear();
  DesenhaCena();
}

void Tabuleiro::AdicionaEntidade(const ntf::Notificacao& notificacao) {
  if (!notificacao.has_entidade()) {
    // Mensagem local.
    if (estado_ != ETAB_QUAD_SELECIONADO) {
      return;
    }
    if (proximo_id_entidade_ >= (1 << 29)) {
      throw std::logic_error("Limite de entidades alcançado.");
    }
    double x, y, z;
    CoordenadaQuadrado(quadrado_selecionado_, &x, &y, &z);
    auto* entidade = NovaEntidade(TE_ENTIDADE);
    entidade->Inicializa(GeraEntidadeProto(id_cliente_, proximo_id_entidade_++, x, y, z));
    entidades_.insert(std::make_pair(entidade->Id(), entidade));
    SelecionaEntidade(entidade->Id());
    // Envia a entidade para os outros.
    auto* n = new ntf::Notificacao;
    n->set_tipo(notificacao.tipo());
    n->mutable_entidade()->CopyFrom(entidade->Proto());
    central_->AdicionaNotificacaoRemota(n);
  } else {
    // Mensagem veio de fora.
    auto* entidade = NovaEntidade(notificacao.entidade().tipo());
    entidade->Inicializa(notificacao.entidade());
    entidades_.insert(std::make_pair(entidade->Id(), entidade));
  }
}

void Tabuleiro::RemoveEntidade(const ntf::Notificacao& notificacao) {
  unsigned int id_remocao = 0;
  if (notificacao.entidade().has_id()) {
    // Comando vindo de fora.
    id_remocao = notificacao.entidade().id();
  } else if (estado_ == ETAB_ENT_SELECIONADA) {
    // Remover entidade selecionada local.
    id_remocao = entidade_selecionada_->Id();
    // Envia para os clientes.
    auto* n = new ntf::Notificacao;
    n->set_tipo(ntf::TN_REMOVER_ENTIDADE);
    n->mutable_entidade()->set_id(id_remocao);
    central_->AdicionaNotificacaoRemota(n);
  } else {
    return;
  }
  if (RemoveEntidade(id_remocao)) {
    DeselecionaEntidade();
  }
}

bool Tabuleiro::TrataNotificacao(const ntf::Notificacao& notificacao) {
  switch (notificacao.tipo()) {
    case ntf::TN_ADICIONAR_ENTIDADE:
      try {
        AdicionaEntidade(notificacao);
      } catch (const std::logic_error& e) {
        LOG(ERROR) << "Limite de entidades alcançado.";
      }
      return true;
    case ntf::TN_REMOVER_ENTIDADE: {
      RemoveEntidade(notificacao);
      return true;
    }
    case ntf::TN_TEMPORIZADOR: {
      for (auto& id_ent : entidades_) {
        id_ent.second->Atualiza();
      }
      return true;
    }
    case ntf::TN_SERIALIZAR_TABULEIRO: {
      auto* nt_tabuleiro = SerializaTabuleiro();
      if (notificacao.has_endereco()) {
        std::string nt_tabuleiro_str = nt_tabuleiro->SerializeAsString();
        // Salvar no endereco.
        std::ofstream arquivo(notificacao.endereco());
        arquivo.write(nt_tabuleiro_str.c_str(), nt_tabuleiro_str.size());
        if (!arquivo) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro escrevendo arquivo";
        }
        arquivo.close();
        delete nt_tabuleiro;
      } else {
        // Enviar remotamente.
        central_->AdicionaNotificacaoRemota(nt_tabuleiro);
      }
      return true;
    }
    case ntf::TN_DESERIALIZAR_TABULEIRO: {
      if (notificacao.has_endereco()) {
        // Deserializar de arquivo.
        std::ifstream arquivo(notificacao.endereco());
        if (!arquivo) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro lendo arquivo";
          return true;
        }
        arquivo.seekg(0, std::ifstream::end);
        std::ifstream::pos_type tamanho = arquivo.tellg();
        arquivo.seekg(0, std::ifstream::beg);
        std::vector<char> buffer(tamanho);
        arquivo.read(&buffer[0], tamanho);
        arquivo.close();
        ntf::Notificacao nt_tabuleiro;
        if (!nt_tabuleiro.ParseFromString(std::string(buffer.begin(), buffer.end()))) {
          // TODO enviar uma mensagem de erro direto aqui na UI.
          LOG(ERROR) << "Erro restaurando notificacao do arquivo";
          return true;
        }
        DeserializaTabuleiro(nt_tabuleiro);
      } else {
        // Deserializar da rede.
        DeserializaTabuleiro(notificacao);
      }
      return true;
    }
    case ntf::TN_MOVER_ENTIDADE: {
      const auto& proto = notificacao.entidade();
      auto* entidade = BuscaEntidade(proto.id());
      if (entidade == nullptr) {
        LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
        return true;
      }
      entidade->Destino(proto);
      return true;
    }
    case ntf::TN_ATUALIZAR_ENTIDADE: {
      const auto& proto = notificacao.entidade();
      auto* entidade = BuscaEntidade(proto.id());
      if (entidade == nullptr) {
        LOG(ERROR) << "Entidade invalida: " << proto.ShortDebugString();
        return true;
      }
      entidade->Atualiza(proto);
      if (notificacao.has_endereco()) {
        auto* n_remota = new ntf::Notificacao(notificacao);
        n_remota->clear_endereco();
        central_->AdicionaNotificacaoRemota(n_remota);
      }
      return true;
    }
    case ntf::TN_ABRIR_DIALOGO_ILUMINACAO: {
      if (notificacao.has_tabuleiro()) {
        // Notificacao ja foi criada, deixa pra ifg fazer o resto.
        return false;
      }
      central_->AdicionaNotificacao(SerializaIluminacaoTabuleiro());
      return true;
    }
    case ntf::TN_ATUALIZAR_ILUMINACAO: {
      luz_.CopyFrom(notificacao.tabuleiro().luz());
      if (notificacao.has_endereco()) {
        auto* n_remota = new ntf::Notificacao(notificacao);
        n_remota->clear_endereco();
        central_->AdicionaNotificacaoRemota(n_remota);
      }
      return true;
    }
    default:
      return false;
  }
}

void Tabuleiro::TrataRodela(int delta) {
  // move o olho no eixo Z de acordo com o eixo Y do movimento
  olho_raio_ -= (delta * SENSIBILIDADE_RODA); 
  if (olho_raio_ < OLHO_RAIO_MINIMO) {
    olho_raio_ = OLHO_RAIO_MINIMO;
  }
  else if (olho_raio_ > OLHO_RAIO_MAXIMO) {
    olho_raio_ = OLHO_RAIO_MAXIMO;
  }
}

// Coordenadas em OpenGL (origem canto inferior esquerdo).
void Tabuleiro::TrataMovimento(int x, int y) {
  if (estado_ == ETAB_ROTACAO) {
    // Realiza a rotacao da tela.
    olho_delta_rotacao_ -= (x - rotacao_ultimo_x_) * SENSIBILIDADE_ROTACAO_X;
    if (olho_delta_rotacao_ >= 2*M_PI) {
      olho_delta_rotacao_ -= 2*M_PI;
    }
    else if (olho_delta_rotacao_ <= -2*M_PI) {
      olho_delta_rotacao_ += 2*M_PI;
    }
    // move o olho no eixo Z de acordo com o eixo Y do movimento
    olho_altura_ -= (y - rotacao_ultimo_y_) * SENSIBILIDADE_ROTACAO_Y; 
    if (olho_altura_ < OLHO_ALTURA_MINIMA) {
      olho_altura_ = OLHO_ALTURA_MINIMA;
    }
    else if (olho_altura_ > OLHO_ALTURA_MAXIMA) {
      olho_altura_ = OLHO_ALTURA_MAXIMA;
    }

    rotacao_ultimo_x_ = x;
    rotacao_ultimo_y_ = y;
  } else if (estado_ == ETAB_ENT_PRESSIONADA) {
    // Realiza o movimento da entidade.
    // Transforma x e y em 3D, baseado no nivel do solo.
    parametros_desenho_.set_desenha_entidades(false);
    parametros_desenho_.set_iluminacao(false);
    DesenhaCena();  // Sem as entidades pra pegar nivel solo.
    GLdouble modelview[16], projection[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);
    GLdouble nx, ny, nz;
    GLfloat win_z;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);
    //LOG(INFO) << "WX: " << x << ", WY: " << y << ", WZ: " << win_z 
    //          << ", viewport: " << viewport[0] << " " << viewport[1] << " " 
    //          << viewport[2] << " " << viewport[3] << endl;
    if (!gluUnProject(x, y, win_z, modelview, projection, viewport, &nx, &ny, &nz)) {
      return;
    }
    //cout << "3x: " << nx << ", 3y: " << ny << ", 3z: " << nz << endl; 
    entidade_selecionada_->MovePara(nx, ny, 0);
  }
}

void Tabuleiro::TrataBotaoPressionado(botao_e botao, int x, int y, double aspecto) {
  if (botao == BOTAO_MEIO) {
    rotacao_ultimo_x_ = x;
    rotacao_ultimo_y_ = y;
    estado_anterior_rotacao_ = estado_;
    estado_ = ETAB_ROTACAO;
  } else if (botao == BOTAO_ESQUERDO) {
    // informacao dos hits. TODO ver esse limite aqui.
    GLuint buffer_hits[100] = {0};
    GLuint numero_hits = 0;
    EncontraHits(x, y, aspecto, &numero_hits, buffer_hits);
    TrataClique(numero_hits, buffer_hits);
  }
}

void Tabuleiro::TrataDuploClick(botao_e botao, int x, int y, double aspecto) {
  if (botao == BOTAO_ESQUERDO) {
    // informacao dos hits. TODO ver esse limite aqui.
    GLuint buffer_hits[100] = {0};
    GLuint numero_hits = 0;
    EncontraHits(x, y, aspecto, &numero_hits, buffer_hits);
    TrataDuploClique(numero_hits, buffer_hits);
  }
}

void Tabuleiro::TrataBotaoLiberado() {
  switch (estado_) {
    case ETAB_ROTACAO:
      estado_ = estado_anterior_rotacao_;
      return;
    case ETAB_ENT_PRESSIONADA: {
      auto* n = new ntf::Notificacao;
      n->set_tipo(ntf::TN_MOVER_ENTIDADE);
      auto* e = n->mutable_entidade();
      e->set_id(entidade_selecionada_->Id());
      auto* p = e->mutable_destino();
      p->set_x(entidade_selecionada_->X());
      p->set_y(entidade_selecionada_->Y());
      p->set_z(entidade_selecionada_->Z());
      central_->AdicionaNotificacaoRemota(n);
      return;
    }
    case ETAB_QUAD_PRESSIONADO:
      estado_ = ETAB_QUAD_SELECIONADO;
      return;
    default:
      ;
  }
}


void Tabuleiro::TrataRedimensionaJanela(int largura, int altura) {
  glViewport(0, 0, (GLint)largura, (GLint)altura);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(CAMPO_VERTICAL, (double)largura / altura, 0.5, 500.0);
}

void Tabuleiro::InicializaGL() {
  glClearColor(1.0, 1.0, 1.0, 1.0);

  // back face
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // zbuffer
  glEnable(GL_DEPTH_TEST);
}

// privadas 
void Tabuleiro::DesenhaCena() {
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);
  for (int i = 1; i < 8; ++i) {
    glDisable(GL_LIGHT0 + i);
  }
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
    // from
    olho_x_ + cos(olho_delta_rotacao_) * olho_raio_, 
    olho_y_ + sin(olho_delta_rotacao_) * olho_raio_, 
    olho_altura_,
    // to
    olho_x_, olho_y_, olho_z_,
    // up
    0, 0, 1.0);

  if (parametros_desenho_.iluminacao()) {
    glEnable(GL_LIGHTING);
    // Iluminação ambiente.
    //GLfloat pos_luz[] = { 0, 0, 0, 0.0f };
    //glLightfv(GL_LIGHT0, GL_POSITION, pos_luz);
    GLfloat cor_luz[] = {luz_.cor().r(), luz_.cor().g(), luz_.cor().b(), luz_.cor().a()};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, cor_luz);
    glEnable(GL_LIGHT0);

    // Posiciona as luzes dinâmicas.
    for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
      it->second->DesenhaLuz(&parametros_desenho_);
    }
  } else {
    glDisable(GL_LIGHTING);
  }

  //ceu_.desenha(parametros_desenho_);
  // desenha tabuleiro de baixo pra cima
  glPushMatrix();
  double deltaX = -TamanhoX() * TAMANHO_GL;
  double deltaY = -TamanhoY() * TAMANHO_GL;
  glNormal3f(0, 0, 1.0f);
  glTranslated(deltaX / 2.0, deltaY / 2.0, 0);
  int id = 0;
  for (int y = 0; y < TamanhoY(); ++y) {
    for (int x = 0; x < TamanhoX(); ++x) {
      // desenha quadrado
      DesenhaQuadrado(id, id == quadrado_selecionado_);
      // anda 1 quadrado direita
      glTranslated(TAMANHO_GL, 0, 0);
      ++id;
    }
    // volta tudo esquerda e sobe 1 quadrado
    glTranslated(deltaX, TAMANHO_GL, 0);
  }
  glPopMatrix();
  if (!parametros_desenho_.desenha_entidades()) {
    return;
  }

  //GLfloat ambient[] = { 1.0, 1.0, 1.0, 1.0 };
  //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  // desenha as entidades no segundo lugar da pilha, importante para diferenciar entidades do tabuleiro
  // na hora do picking.
  glPushName(0);
  for (MapaEntidades::iterator it = entidades_.begin(); it != entidades_.end(); ++it) {
    Entidade* entidade = it->second;
    parametros_desenho_.set_entidade_selecionada(entidade == entidade_selecionada_);
    entidade->Desenha(&parametros_desenho_);
  }
  glPopName();
}

// Esta operacao se chama PICKING. Mais informacoes podem ser encontradas no capitulo 11-6 do livro verde
// ou entao aqui http://gpwiki.org/index.php/OpenGL:Tutorials:Picking
// basicamente, entra-se em um modo de desenho onde o buffer apenas recebe o identificador e a 
// profundidade de quem o acertou.
void Tabuleiro::EncontraHits(
    int x, int y, double aspecto, unsigned int* numero_hits, unsigned int* buffer_hits) {
  // inicia o buffer de picking (selecao)
  glSelectBuffer(100, buffer_hits);
  // entra no modo de selecao e limpa a pilha de nomes e inicia com 0
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0); // inicia a pilha de nomes com 0 para sempre haver um nome

  // a matriz de pick afeta a projecao, entao vamos salva-la antes de modifica-la
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  gluPickMatrix(x, y, 1.0, 1.0, viewport);
  gluPerspective(CAMPO_VERTICAL, aspecto, 0.5, 500.0);

  // desenha a cena
  parametros_desenho_.set_iluminacao(false);
  DesenhaCena();

  // volta a projecao
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  // volta pro modo de desenho e processa os hits
  *numero_hits = glRenderMode(GL_RENDER);
  glMatrixMode(GL_MODELVIEW);
}

void Tabuleiro::TrataClique(unsigned int numero_hits, unsigned int* buffer_hits) {
  GLuint id = 0, pos_pilha = 0;
  BuscaHitMaisProximo(numero_hits, buffer_hits, &id, &pos_pilha);
  if (pos_pilha == 1) {
    // Tabuleiro.
    SelecionaQuadrado(id);
  } else if (pos_pilha > 1) {
    // Entidade.
    SelecionaEntidade(id);
    estado_ = ETAB_ENT_PRESSIONADA;
  } else {
    DeselecionaEntidade();
  }
}

void Tabuleiro::TrataDuploClique(unsigned int numero_hits, unsigned int* buffer_hits) {
  GLuint id = 0, pos_pilha = 0;
  BuscaHitMaisProximo(numero_hits, buffer_hits, &id, &pos_pilha);
  if (pos_pilha == 1) {
    // Tabuleiro: cria uma entidade nova.
    ntf::Notificacao notificacao;
    notificacao.set_tipo(ntf::TN_ADICIONAR_ENTIDADE);
    AdicionaEntidade(notificacao);
  } else if (pos_pilha > 1) {
    // Entidade.
    SelecionaEntidade(id);
    auto* n = new ntf::Notificacao;
    n->set_tipo(ntf::TN_ABRIR_DIALOGO_ENTIDADE);
    n->mutable_entidade()->CopyFrom(entidade_selecionada_->Proto());
    central_->AdicionaNotificacao(n);
  } else {
    ;
  }
}

void Tabuleiro::SelecionaEntidade(unsigned int id) {
  VLOG(1) << "selecionando entidade: ";
  auto* entidade = BuscaEntidade(id);
  if (entidade == nullptr) {
    throw std::logic_error("Entidade inválida");
  }
  entidade_selecionada_ = entidade; 
  quadrado_selecionado_ = -1;
  estado_ = ETAB_ENT_SELECIONADA;
}

void Tabuleiro::DeselecionaEntidade() {
  entidade_selecionada_ = nullptr; 
  quadrado_selecionado_ = -1;
  estado_ = ETAB_OCIOSO;
}

void Tabuleiro::SelecionaQuadrado(int id_quadrado) {
  quadrado_selecionado_ = id_quadrado; 
  entidade_selecionada_ = nullptr;
  estado_ = ETAB_QUAD_PRESSIONADO; 
}

void Tabuleiro::CoordenadaQuadrado(int id_quadrado, double* x, double* y, double* z) {
  int quad_x = id_quadrado % TamanhoX();
  int quad_y = id_quadrado / TamanhoY();

  // centro do quadrado
  *x = ((quad_x * TAMANHO_GL) + TAMANHO_GL_2) - (TamanhoX() * TAMANHO_GL_2);
  *y = ((quad_y * TAMANHO_GL) + TAMANHO_GL_2) - (TamanhoY() * TAMANHO_GL_2); 
  *z = 0;
}

ntf::Notificacao* Tabuleiro::SerializaIluminacaoTabuleiro() {
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_ABRIR_DIALOGO_ILUMINACAO);
  notificacao->mutable_tabuleiro()->mutable_luz()->CopyFrom(luz_);
  return notificacao;
}

ntf::Notificacao* Tabuleiro::SerializaTabuleiro() {
  auto* notificacao = new ntf::Notificacao;
  notificacao->set_tipo(ntf::TN_DESERIALIZAR_TABULEIRO);
  if (proximo_id_cliente_ >= 16) {
    notificacao->set_erro("Limite de clientes alcançado.");
    return notificacao;
  }
  auto* t = notificacao->mutable_tabuleiro();
  t->set_id_cliente(proximo_id_cliente_++);
  for (const auto& id_ent : entidades_) {
    t->add_entidade()->CopyFrom(id_ent.second->Proto());
  }
  t->mutable_luz()->CopyFrom(luz_);
  return notificacao;
}

void Tabuleiro::DeserializaTabuleiro(const ntf::Notificacao& notificacao) {
  if (!entidades_.empty()) {
    LOG(ERROR) << "Tabuleiro não está vazio!";
    return;
  }
  if (notificacao.has_erro()) {
    LOG(ERROR) << "Erro ao deserializar tabuleiro: " << notificacao.erro();
    auto* n = new ntf::Notificacao;
    n->set_tipo(ntf::TN_DESCONECTAR);
    central_->AdicionaNotificacao(n);
    return;
  }
  const auto& tabuleiro = notificacao.tabuleiro();
  id_cliente_ = tabuleiro.id_cliente();
  for (const auto& ep : tabuleiro.entidade()) {
    auto* e = NovaEntidade(ep.tipo());
    e->Inicializa(ep);
    entidades_.insert({ e->Id(), e });
  }
}

Entidade* Tabuleiro::BuscaEntidade(unsigned int id) {
  auto it = entidades_.find(id);
  return (it != entidades_.end()) ? it->second : nullptr;
}

bool Tabuleiro::RemoveEntidade(unsigned int id) {
  MapaEntidades::iterator res_find = entidades_.find(id);
  if (res_find == entidades_.end()) {
    return false;
  }
  Entidade* entidade = res_find->second;
  entidades_.erase(res_find);
  delete entidade;
  // Retorna so o endereco, apenas para verificacao.
  return entidade == entidade_selecionada_;
}


