/********************************************************************************
** Form generated from reading UI file 'entidade.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTIDADE_H
#define ENTIDADE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace ifg {
namespace qt {

class Ui_DialogoEntidade
{
public:
    QVBoxLayout *verticalLayout_3;
    QTabWidget *tabs;
    QWidget *tab_geral;
    QGridLayout *gridLayout_8;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label;
    QLineEdit *campo_id;
    QLabel *label_8;
    QLineEdit *campo_rotulo;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_10;
    QPlainTextEdit *lista_rotulos;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QLabel *label_tamanho;
    QSlider *slider_tamanho;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_5;
    QSpinBox *spin_pontos_vida;
    QLabel *label_6;
    QSpinBox *spin_max_pontos_vida;
    QLabel *label_26;
    QPushButton *botao_bonus_pv_temporario;
    QLabel *label_79;
    QSpinBox *spin_dano_nao_letal;
    QVBoxLayout *verticalLayout_11;
    QHBoxLayout *horizontalLayout_8;
    QVBoxLayout *verticalLayout_27;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_voadora;
    QCheckBox *checkbox_visibilidade;
    QCheckBox *checkbox_caida;
    QHBoxLayout *horizontalLayout_17;
    QCheckBox *checkbox_morta;
    QCheckBox *checkbox_nocauteada;
    QCheckBox *checkbox_incapacitada;
    QCheckBox *checkbox_inconsciente;
    QHBoxLayout *horizontalLayout_9;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_4;
    QDoubleSpinBox *spin_aura_quad;
    QLabel *label_35;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_7;
    QDoubleSpinBox *spin_translacao_quad;
    QLabel *label_34;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_13;
    QComboBox *combo_visao;
    QLabel *label_14;
    QDoubleSpinBox *spin_raio_visao_escuro_quad;
    QLabel *label_33;
    QHBoxLayout *horizontalLayout_16;
    QCheckBox *checkbox_salvacao;
    QComboBox *combo_salvacao;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_16;
    QComboBox *combo_modelos_3d;
    QHBoxLayout *horizontalLayout_35;
    QLabel *label_68;
    QListWidget *lista_formas_alternativas;
    QVBoxLayout *verticalLayout_12;
    QPushButton *botao_adicionar_forma_alternativa;
    QPushButton *botao_remover_forma_alternativa;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QGroupBox *groupBox_9;
    QHBoxLayout *horizontalLayout_49;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_81;
    QSlider *slider_alfa;
    QHBoxLayout *horizontalLayout_14;
    QGroupBox *groupBox_8;
    QHBoxLayout *horizontalLayout_46;
    QLabel *label_12;
    QDoubleSpinBox *spin_raio_quad;
    QLabel *label_31;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *botao_luz;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_24;
    QGroupBox *groupBox_10;
    QGridLayout *gridLayout_21;
    QComboBox *combo_textura;
    QHBoxLayout *horizontalLayout_22;
    QLabel *label_17;
    QDoubleSpinBox *spin_tex_altura;
    QHBoxLayout *horizontalLayout_23;
    QLabel *label_18;
    QDoubleSpinBox *spin_tex_largura;
    QHBoxLayout *horizontalLayout_25;
    QLabel *label_19;
    QDoubleSpinBox *spin_tex_trans_x;
    QHBoxLayout *horizontalLayout_26;
    QLabel *label_20;
    QDoubleSpinBox *spin_tex_trans_y;
    QWidget *tab_estatisticas;
    QGridLayout *gridLayout_17;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_16;
    QGridLayout *gridLayout_4;
    QLabel *label_65;
    QLabel *label_66;
    QPushButton *botao_bonus_salvacao_fortitude;
    QPushButton *botao_bonus_salvacao_vontade;
    QLabel *label_67;
    QPushButton *botao_bonus_salvacao_reflexo;
    QLabel *label_70;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_18;
    QGridLayout *gridLayout_2;
    QPushButton *botao_bonus_constituicao;
    QLabel *label_47;
    QSpinBox *spin_destreza;
    QLabel *label_49;
    QLabel *label_51;
    QPushButton *botao_bonus_destreza;
    QLabel *label_50;
    QLabel *label_mod_destreza;
    QSpinBox *spin_carisma;
    QPushButton *botao_bonus_forca;
    QSpinBox *spin_sabedoria;
    QSpinBox *spin_inteligencia;
    QSpinBox *spin_forca;
    QLabel *label_27;
    QSpinBox *spin_constituicao;
    QLabel *label_mod_forca;
    QLabel *label_mod_sabedoria;
    QPushButton *botao_bonus_sabedoria;
    QLabel *label_48;
    QPushButton *botao_bonus_inteligencia;
    QLabel *label_mod_inteligencia;
    QPushButton *botao_bonus_carisma;
    QLabel *label_mod_constituicao;
    QLabel *label_mod_carisma;
    QLabel *label_28;
    QLabel *label_63;
    QLabel *label_62;
    QGroupBox *groupBox_7;
    QWidget *layoutWidget_3;
    QHBoxLayout *horizontalLayout_28;
    QLabel *label_22;
    QPushButton *botao_bonus_iniciativa;
    QCheckBox *checkbox_iniciativa;
    QSpinBox *spin_iniciativa;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout_15;
    QGridLayout *gridLayout_5;
    QLabel *label_bba_agarrar;
    QLabel *label_69;
    QLabel *label_54;
    QLabel *label_bba_base;
    QLabel *label_56;
    QLabel *label_71;
    QLabel *label_bba_cac;
    QLabel *label_bba_distancia;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_14;
    QGridLayout *gridLayout_3;
    QSpinBox *spin_ca_escudo_melhoria;
    QComboBox *combo_material_escudo;
    QLabel *spin_bonus_escudo;
    QComboBox *combo_escudo;
    QCheckBox *checkbox_escudo_obra_prima;
    QLabel *label_ca_toque;
    QLabel *label_95;
    QLabel *label_52;
    QLabel *label_57;
    QSpinBox *spin_ca_armadura_melhoria;
    QLabel *label_58;
    QLabel *label_59;
    QLabel *label_ca_surpreso;
    QPushButton *botao_bonus_ca;
    QComboBox *combo_armadura;
    QLabel *spin_bonus_escudo_2;
    QComboBox *combo_material_armadura;
    QLabel *label_100;
    QCheckBox *checkbox_armadura_obra_prima;
    QLabel *label_61;
    QLabel *label_53;
    QLabel *label_60;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_85;
    QPushButton *botao_resistencia_magia;
    QCheckBox *checkbox_imune_critico;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_19;
    QLabel *label_37;
    QLabel *label_25;
    QLabel *label_90;
    QLabel *label_32;
    QSpinBox *spin_mov_aereo;
    QSpinBox *spin_mov_nadando;
    QSpinBox *spin_mov_terrestre;
    QSpinBox *spin_mov_escavando;
    QLabel *label_97;
    QPushButton *botao_mov_terrestre;
    QPushButton *botao_mov_nadando;
    QPushButton *botao_mov_aereo;
    QPushButton *botao_mov_escavando;
    QLabel *label_15;
    QSpinBox *spin_mov_escalando;
    QLabel *label_74;
    QPushButton *botao_mov_escalando;
    QWidget *tab_7;
    QHBoxLayout *horizontalLayout_34;
    QHBoxLayout *horizontalLayout_19;
    QLabel *label_11;
    QTableView *tabela_lista_eventos;
    QVBoxLayout *verticalLayout_13;
    QPushButton *botao_adicionar_evento;
    QPushButton *botao_remover_evento;
    QWidget *tab_nivel;
    QGridLayout *gridLayout_9;
    QHBoxLayout *horizontalLayout_31;
    QLabel *label_39;
    QLineEdit *linha_nivel;
    QHBoxLayout *horizontalLayout_37;
    QLabel *label_80;
    QSpinBox *spin_niveis_negativos;
    QHBoxLayout *horizontalLayout_33;
    QLabel *label_46;
    QLineEdit *linha_bba;
    QHBoxLayout *horizontalLayout_38;
    QLabel *label_84;
    QSpinBox *spin_xp;
    QHBoxLayout *horizontalLayout_43;
    QLabel *label_75;
    QSlider *slider_bem_mal;
    QLabel *label_78;
    QHBoxLayout *horizontalLayout_39;
    QLabel *label_76;
    QSlider *slider_ordem_caos;
    QLabel *label_77;
    QHBoxLayout *horizontalLayout_47;
    QLabel *label_101;
    QComboBox *combo_raca;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_20;
    QHBoxLayout *horizontalLayout;
    QLabel *label_40;
    QComboBox *combo_classe;
    QLineEdit *linha_classe;
    QSpacerItem *horizontalSpacer;
    QLabel *label_41;
    QSpinBox *spin_nivel_classe;
    QSpacerItem *horizontalSpacer_4;
    QLabel *label_43;
    QSpinBox *spin_bba;
    QSpacerItem *horizontalSpacer_7;
    QLabel *label_64;
    QComboBox *combo_salvacoes_fortes;
    QHBoxLayout *horizontalLayout_48;
    QLabel *label_42;
    QSpinBox *spin_nivel_conjurador;
    QLabel *label_mod_conjuracao;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_44;
    QComboBox *combo_mod_conjuracao;
    QSpacerItem *horizontalSpacer_6;
    QLabel *label_87;
    QComboBox *combo_especializacao_escola;
    QLabel *label_88;
    QComboBox *combo_escola_proibida_1;
    QComboBox *combo_escola_proibida_2;
    QSpacerItem *horizontalSpacer_8;
    QLabel *label_102;
    QComboBox *combo_dominio_1;
    QComboBox *combo_dominio_2;
    QVBoxLayout *verticalLayout_14;
    QPushButton *botao_adicionar_nivel;
    QPushButton *botao_remover_nivel;
    QListWidget *lista_niveis;
    QWidget *tab_3;
    QGridLayout *gridLayout_12;
    QGridLayout *gridLayout_11;
    QLabel *label_pericias;
    QTableView *tabela_pericias;
    QGridLayout *gridLayout_10;
    QLabel *label_talentos;
    QGridLayout *gridLayout_6;
    QTableView *tabela_talentos;
    QVBoxLayout *verticalLayout_15;
    QPushButton *botao_adicionar_talento;
    QPushButton *botao_remover_talento;
    QWidget *tab_5;
    QVBoxLayout *verticalLayout_18;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_38;
    QLineEdit *linha_furtivo;
    QHBoxLayout *horizontalLayout_50;
    QHBoxLayout *horizontalLayout_52;
    QLabel *label_86;
    QComboBox *combo_esquiva_sobrenatural;
    QHBoxLayout *horizontalLayout_69;
    QLabel *label_103;
    QComboBox *combo_evasao_estatica;
    QComboBox *combo_evasao_dinamica;
    QVBoxLayout *verticalLayout_17;
    QLabel *label_94;
    QHBoxLayout *horizontalLayout_29;
    QTableView *tabela_inimigos_prediletos;
    QVBoxLayout *verticalLayout_2;
    QPushButton *botao_adicionar_inimigo_predileto;
    QPushButton *botao_remover_inimigo_predileto;
    QWidget *tab_6;
    QGridLayout *gridLayout_13;
    QLabel *label_91;
    QLineEdit *linha_grupo_ataque;
    QLabel *label_36;
    QLineEdit *linha_rotulo_ataque;
    QLabel *label_83;
    QSpinBox *spin_ordem_ataque;
    QLabel *label_73;
    QComboBox *combo_empunhadura;
    QLabel *label_21;
    QComboBox *combo_tipo_ataque;
    QLabel *label_93;
    QComboBox *combo_arma;
    QCheckBox *checkbox_op;
    QLabel *label_23;
    QSpinBox *spin_bonus_magico;
    QLabel *label_92;
    QComboBox *combo_material_arma;
    QLabel *label_55;
    QPushButton *botao_bonus_ataque;
    QLabel *label_24;
    QLineEdit *linha_dano;
    QLabel *label_82;
    QSpinBox *spin_municao;
    QLabel *label_29;
    QSpinBox *spin_alcance_quad;
    QLabel *label_30;
    QSpinBox *spin_incrementos;
    QLabel *label_72;
    QPushButton *botao_bonus_dano;
    QLabel *label_104;
    QSpinBox *spin_nivel_conjurador_pergaminho;
    QLabel *label_mod_atributo;
    QSpinBox *spin_modificador_atributo_pergaminho;
    QLabel *label_9;
    QSpinBox *spin_limite_vezes;
    QCheckBox *checkbox_ignora_rm;
    QCheckBox *checkbox_permite_salvacao;
    QCheckBox *checkbox_ataque_agarrar;
    QCheckBox *checkbox_ataque_toque;
    QListWidget *lista_ataques;
    QVBoxLayout *verticalLayout_16;
    QPushButton *botao_ataque_cima;
    QPushButton *botao_ataque_baixo;
    QPushButton *botao_remover_ataque;
    QPushButton *botao_adicionar_ataque;
    QPushButton *botao_clonar_ataque;
    QWidget *tab;
    QGridLayout *gridLayout_7;
    QGroupBox *groupBox_22;
    QHBoxLayout *horizontalLayout_67;
    QListWidget *lista_bracadeiras;
    QVBoxLayout *verticalLayout_7;
    QPushButton *botao_usar_bracadeiras;
    QPushButton *botao_adicionar_bracadeiras;
    QPushButton *botao_remover_bracadeiras;
    QPushButton *botao_doar_bracadeiras;
    QGroupBox *groupBox_24;
    QHBoxLayout *horizontalLayout_73;
    QListWidget *lista_armas;
    QVBoxLayout *verticalLayout_29;
    QPushButton *botao_adicionar_arma;
    QPushButton *botao_duplicar_arma;
    QPushButton *botao_remover_arma;
    QPushButton *botao_ordenar_armas;
    QPushButton *botao_doar_arma;
    QGroupBox *groupBox_13;
    QHBoxLayout *horizontalLayout_58;
    QListWidget *lista_amuletos;
    QVBoxLayout *verticalLayout_19;
    QPushButton *botao_usar_amuleto;
    QPushButton *botao_adicionar_amuleto;
    QPushButton *botao_remover_amuleto;
    QPushButton *botao_doar_amuleto;
    QGroupBox *groupBox_23;
    QHBoxLayout *horizontalLayout_68;
    QListWidget *lista_itens_mundanos;
    QVBoxLayout *verticalLayout_25;
    QPushButton *botao_adicionar_item_mundano;
    QPushButton *botao_duplicar_item_mundano;
    QPushButton *botao_remover_item_mundano;
    QPushButton *botao_ordenar_item_mundano;
    QPushButton *botao_doar_item_mundano;
    QGroupBox *groupBox_19;
    QHBoxLayout *horizontalLayout_64;
    QListWidget *lista_chapeus;
    QVBoxLayout *verticalLayout_20;
    QPushButton *botao_vestir_chapeu;
    QPushButton *botao_adicionar_chapeu;
    QPushButton *botao_remover_chapeu;
    QPushButton *botao_doar_chapeu;
    QGroupBox *groupBox_26;
    QHBoxLayout *horizontalLayout_75;
    QListWidget *lista_escudos;
    QVBoxLayout *verticalLayout_31;
    QPushButton *botao_adicionar_escudo;
    QPushButton *botao_duplicar_escudo;
    QPushButton *botao_remover_escudo;
    QPushButton *botao_ordenar_escudos;
    QPushButton *botao_doar_escudo;
    QGroupBox *groupBox_20;
    QHBoxLayout *horizontalLayout_65;
    QListWidget *lista_mantos;
    QVBoxLayout *verticalLayout_9;
    QPushButton *botao_usar_manto;
    QPushButton *botao_adicionar_manto;
    QPushButton *botao_remover_manto;
    QPushButton *botao_doar_manto;
    QGroupBox *groupBox_16;
    QHBoxLayout *horizontalLayout_61;
    QListWidget *lista_pergaminhos_divinos;
    QVBoxLayout *verticalLayout_24;
    QPushButton *botao_adicionar_pergaminho_divino;
    QPushButton *botao_duplicar_pergaminho_divino;
    QPushButton *botao_remover_pergaminho_divino;
    QPushButton *botao_ordenar_pergaminhos_divinos;
    QPushButton *botao_doar_pergaminho_divino;
    QGroupBox *groupBox_25;
    QHBoxLayout *horizontalLayout_74;
    QListWidget *lista_armaduras;
    QVBoxLayout *verticalLayout_30;
    QPushButton *botao_adicionar_armadura;
    QPushButton *botao_duplicar_armadura;
    QPushButton *botao_remover_armadura;
    QPushButton *botao_ordenar_armaduras;
    QPushButton *botao_doar_armadura;
    QGroupBox *groupBox_14;
    QHBoxLayout *horizontalLayout_59;
    QListWidget *lista_pocoes;
    QVBoxLayout *verticalLayout_6;
    QPushButton *botao_adicionar_pocao;
    QPushButton *botao_duplicar_pocao;
    QPushButton *botao_remover_pocao;
    QPushButton *botao_ordenar_pocoes;
    QPushButton *botao_doar_pocao;
    QGroupBox *groupBox_12;
    QHBoxLayout *horizontalLayout_44;
    QListWidget *lista_botas;
    QVBoxLayout *verticalLayout_22;
    QPushButton *botao_usar_botas;
    QPushButton *botao_adicionar_botas;
    QPushButton *botao_remover_botas;
    QPushButton *botao_doar_botas;
    QGroupBox *groupBox_15;
    QHBoxLayout *horizontalLayout_60;
    QListWidget *lista_pergaminhos_arcanos;
    QVBoxLayout *verticalLayout_23;
    QPushButton *botao_adicionar_pergaminho_arcano;
    QPushButton *botao_duplicar_pergaminho_arcano;
    QPushButton *botao_remover_pergaminho_arcano;
    QPushButton *botao_ordenar_pergaminhos_arcanos;
    QPushButton *botao_doar_pergaminho_arcano;
    QGroupBox *groupBox_17;
    QHBoxLayout *horizontalLayout_62;
    QListWidget *lista_aneis;
    QVBoxLayout *verticalLayout_10;
    QPushButton *botao_usar_anel;
    QPushButton *botao_adicionar_anel;
    QPushButton *botao_remover_anel;
    QPushButton *botao_doar_anel;
    QGroupBox *groupBox_18;
    QHBoxLayout *horizontalLayout_63;
    QListWidget *lista_luvas;
    QVBoxLayout *verticalLayout_8;
    QPushButton *botao_usar_luvas;
    QPushButton *botao_adicionar_luvas;
    QPushButton *botao_remover_luvas;
    QPushButton *botao_doar_luvas;
    QGroupBox *groupBox_27;
    QHBoxLayout *horizontalLayout_70;
    QListWidget *lista_varinhas;
    QVBoxLayout *verticalLayout_26;
    QPushButton *botao_adicionar_varinha;
    QPushButton *botao_duplicar_varinha;
    QPushButton *botao_remover_varinha;
    QPushButton *botao_ordenar_varinhas;
    QPushButton *botao_doar_varinha;
    QGroupBox *groupBox_11;
    QVBoxLayout *verticalLayout_21;
    QHBoxLayout *horizontalLayout_53;
    QLabel *label_107;
    QSpinBox *spin_po;
    QHBoxLayout *horizontalLayout_54;
    QLabel *label_108;
    QSpinBox *spin_pp;
    QHBoxLayout *horizontalLayout_56;
    QLabel *label_109;
    QSpinBox *spin_pc;
    QHBoxLayout *horizontalLayout_57;
    QLabel *label_110;
    QSpinBox *spin_pl;
    QHBoxLayout *horizontalLayout_55;
    QLabel *labelpe;
    QSpinBox *spin_pe;
    QGroupBox *groupBox_21;
    QHBoxLayout *horizontalLayout_66;
    QPlainTextEdit *lista_tesouro;
    QWidget *tab_4;
    QVBoxLayout *verticalLayout_4;
    QPushButton *botao_renovar_feiticos;
    QTreeWidget *arvore_feiticos;
    QWidget *tab_2;
    QVBoxLayout *verticalLayout_5;
    QHBoxLayout *horizontalLayout_32;
    QLabel *label_45;
    QPlainTextEdit *texto_notas;
    QDialogButtonBox *botoes;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QString::fromUtf8("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(1582, 961);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ifg__qt__DialogoEntidade->sizePolicy().hasHeightForWidth());
        ifg__qt__DialogoEntidade->setSizePolicy(sizePolicy);
        ifg__qt__DialogoEntidade->setStyleSheet(QString::fromUtf8("QGroupBox {\n"
"  border: 1px solid gray;\n"
"  border-radius: 9px;\n"
"  margin-top: 0.5em;\n"
"}\n"
"\n"
"QGroupBox::title {\n"
"  subcontrol-origin: margin;\n"
"  left: 10px;\n"
"  padding: 0 3px 0 3px;\n"
"  font-weight: bold;\n"
"}"));
        ifg__qt__DialogoEntidade->setModal(true);
        verticalLayout_3 = new QVBoxLayout(ifg__qt__DialogoEntidade);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        tabs = new QTabWidget(ifg__qt__DialogoEntidade);
        tabs->setObjectName(QString::fromUtf8("tabs"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabs->sizePolicy().hasHeightForWidth());
        tabs->setSizePolicy(sizePolicy1);
        tabs->setStyleSheet(QString::fromUtf8(""));
        tab_geral = new QWidget();
        tab_geral->setObjectName(QString::fromUtf8("tab_geral"));
        gridLayout_8 = new QGridLayout(tab_geral);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label = new QLabel(tab_geral);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_15->addWidget(label);

        campo_id = new QLineEdit(tab_geral);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(campo_id->sizePolicy().hasHeightForWidth());
        campo_id->setSizePolicy(sizePolicy2);
        campo_id->setMinimumSize(QSize(0, 0));
        campo_id->setMaximumSize(QSize(70, 16777215));
        campo_id->setReadOnly(true);

        horizontalLayout_15->addWidget(campo_id);

        label_8 = new QLabel(tab_geral);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy3);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_8);

        campo_rotulo = new QLineEdit(tab_geral);
        campo_rotulo->setObjectName(QString::fromUtf8("campo_rotulo"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(campo_rotulo->sizePolicy().hasHeightForWidth());
        campo_rotulo->setSizePolicy(sizePolicy4);
        campo_rotulo->setReadOnly(false);

        horizontalLayout_15->addWidget(campo_rotulo);


        verticalLayout->addLayout(horizontalLayout_15);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        label_10 = new QLabel(tab_geral);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy5);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_10);

        lista_rotulos = new QPlainTextEdit(tab_geral);
        lista_rotulos->setObjectName(QString::fromUtf8("lista_rotulos"));

        horizontalLayout_18->addWidget(lista_rotulos);


        verticalLayout->addLayout(horizontalLayout_18);


        gridLayout_8->addLayout(verticalLayout, 0, 0, 2, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(tab_geral);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy6(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy6);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_2);

        label_tamanho = new QLabel(tab_geral);
        label_tamanho->setObjectName(QString::fromUtf8("label_tamanho"));
        sizePolicy5.setHeightForWidth(label_tamanho->sizePolicy().hasHeightForWidth());
        label_tamanho->setSizePolicy(sizePolicy5);
        label_tamanho->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_tamanho);


        horizontalLayout_4->addLayout(horizontalLayout_5);

        slider_tamanho = new QSlider(tab_geral);
        slider_tamanho->setObjectName(QString::fromUtf8("slider_tamanho"));
        sizePolicy4.setHeightForWidth(slider_tamanho->sizePolicy().hasHeightForWidth());
        slider_tamanho->setSizePolicy(sizePolicy4);
        slider_tamanho->setMaximum(8);
        slider_tamanho->setPageStep(2);
        slider_tamanho->setSliderPosition(4);
        slider_tamanho->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(slider_tamanho);


        gridLayout_8->addLayout(horizontalLayout_4, 2, 0, 2, 1);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_5 = new QLabel(tab_geral);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        QSizePolicy sizePolicy7(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy7);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_5);

        spin_pontos_vida = new QSpinBox(tab_geral);
        spin_pontos_vida->setObjectName(QString::fromUtf8("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_pontos_vida);

        label_6 = new QLabel(tab_geral);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_6);

        spin_max_pontos_vida = new QSpinBox(tab_geral);
        spin_max_pontos_vida->setObjectName(QString::fromUtf8("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_max_pontos_vida);

        label_26 = new QLabel(tab_geral);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        sizePolicy.setHeightForWidth(label_26->sizePolicy().hasHeightForWidth());
        label_26->setSizePolicy(sizePolicy);
        label_26->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_26);

        botao_bonus_pv_temporario = new QPushButton(tab_geral);
        botao_bonus_pv_temporario->setObjectName(QString::fromUtf8("botao_bonus_pv_temporario"));
        sizePolicy2.setHeightForWidth(botao_bonus_pv_temporario->sizePolicy().hasHeightForWidth());
        botao_bonus_pv_temporario->setSizePolicy(sizePolicy2);
        botao_bonus_pv_temporario->setMinimumSize(QSize(0, 0));
        botao_bonus_pv_temporario->setMaximumSize(QSize(40, 16777215));

        horizontalLayout_12->addWidget(botao_bonus_pv_temporario);

        label_79 = new QLabel(tab_geral);
        label_79->setObjectName(QString::fromUtf8("label_79"));
        sizePolicy.setHeightForWidth(label_79->sizePolicy().hasHeightForWidth());
        label_79->setSizePolicy(sizePolicy);
        label_79->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_79);

        spin_dano_nao_letal = new QSpinBox(tab_geral);
        spin_dano_nao_letal->setObjectName(QString::fromUtf8("spin_dano_nao_letal"));
        sizePolicy2.setHeightForWidth(spin_dano_nao_letal->sizePolicy().hasHeightForWidth());
        spin_dano_nao_letal->setSizePolicy(sizePolicy2);
        spin_dano_nao_letal->setMinimum(0);
        spin_dano_nao_letal->setMaximum(999);

        horizontalLayout_12->addWidget(spin_dano_nao_letal);


        gridLayout_8->addLayout(horizontalLayout_12, 4, 0, 1, 1);

        verticalLayout_11 = new QVBoxLayout();
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        verticalLayout_27 = new QVBoxLayout();
        verticalLayout_27->setObjectName(QString::fromUtf8("verticalLayout_27"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkbox_selecionavel = new QCheckBox(tab_geral);
        checkbox_selecionavel->setObjectName(QString::fromUtf8("checkbox_selecionavel"));

        horizontalLayout_3->addWidget(checkbox_selecionavel);

        checkbox_voadora = new QCheckBox(tab_geral);
        checkbox_voadora->setObjectName(QString::fromUtf8("checkbox_voadora"));

        horizontalLayout_3->addWidget(checkbox_voadora);

        checkbox_visibilidade = new QCheckBox(tab_geral);
        checkbox_visibilidade->setObjectName(QString::fromUtf8("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);

        horizontalLayout_3->addWidget(checkbox_visibilidade);

        checkbox_caida = new QCheckBox(tab_geral);
        checkbox_caida->setObjectName(QString::fromUtf8("checkbox_caida"));

        horizontalLayout_3->addWidget(checkbox_caida);


        verticalLayout_27->addLayout(horizontalLayout_3);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        checkbox_morta = new QCheckBox(tab_geral);
        checkbox_morta->setObjectName(QString::fromUtf8("checkbox_morta"));

        horizontalLayout_17->addWidget(checkbox_morta);

        checkbox_nocauteada = new QCheckBox(tab_geral);
        checkbox_nocauteada->setObjectName(QString::fromUtf8("checkbox_nocauteada"));

        horizontalLayout_17->addWidget(checkbox_nocauteada);

        checkbox_incapacitada = new QCheckBox(tab_geral);
        checkbox_incapacitada->setObjectName(QString::fromUtf8("checkbox_incapacitada"));

        horizontalLayout_17->addWidget(checkbox_incapacitada);

        checkbox_inconsciente = new QCheckBox(tab_geral);
        checkbox_inconsciente->setObjectName(QString::fromUtf8("checkbox_inconsciente"));

        horizontalLayout_17->addWidget(checkbox_inconsciente);


        verticalLayout_27->addLayout(horizontalLayout_17);


        horizontalLayout_8->addLayout(verticalLayout_27);


        verticalLayout_11->addLayout(horizontalLayout_8);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_4 = new QLabel(tab_geral);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy7.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy7);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_4);

        spin_aura_quad = new QDoubleSpinBox(tab_geral);
        spin_aura_quad->setObjectName(QString::fromUtf8("spin_aura_quad"));
        spin_aura_quad->setDecimals(1);
        spin_aura_quad->setSingleStep(1.000000000000000);

        horizontalLayout_10->addWidget(spin_aura_quad);

        label_35 = new QLabel(tab_geral);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        sizePolicy7.setHeightForWidth(label_35->sizePolicy().hasHeightForWidth());
        label_35->setSizePolicy(sizePolicy7);
        label_35->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_35);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        label_7 = new QLabel(tab_geral);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_7);

        spin_translacao_quad = new QDoubleSpinBox(tab_geral);
        spin_translacao_quad->setObjectName(QString::fromUtf8("spin_translacao_quad"));
        sizePolicy2.setHeightForWidth(spin_translacao_quad->sizePolicy().hasHeightForWidth());
        spin_translacao_quad->setSizePolicy(sizePolicy2);
        spin_translacao_quad->setDecimals(1);
        spin_translacao_quad->setMinimum(-100.000000000000000);
        spin_translacao_quad->setMaximum(100.000000000000000);
        spin_translacao_quad->setSingleStep(0.100000000000000);

        horizontalLayout_13->addWidget(spin_translacao_quad);

        label_34 = new QLabel(tab_geral);
        label_34->setObjectName(QString::fromUtf8("label_34"));
        sizePolicy7.setHeightForWidth(label_34->sizePolicy().hasHeightForWidth());
        label_34->setSizePolicy(sizePolicy7);
        label_34->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_34);


        horizontalLayout_9->addLayout(horizontalLayout_13);


        verticalLayout_11->addLayout(horizontalLayout_9);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        label_13 = new QLabel(tab_geral);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        sizePolicy7.setHeightForWidth(label_13->sizePolicy().hasHeightForWidth());
        label_13->setSizePolicy(sizePolicy7);
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_13);

        combo_visao = new QComboBox(tab_geral);
        combo_visao->addItem(QString());
        combo_visao->addItem(QString());
        combo_visao->addItem(QString());
        combo_visao->setObjectName(QString::fromUtf8("combo_visao"));

        horizontalLayout_20->addWidget(combo_visao);

        label_14 = new QLabel(tab_geral);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        sizePolicy7.setHeightForWidth(label_14->sizePolicy().hasHeightForWidth());
        label_14->setSizePolicy(sizePolicy7);
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_14);

        spin_raio_visao_escuro_quad = new QDoubleSpinBox(tab_geral);
        spin_raio_visao_escuro_quad->setObjectName(QString::fromUtf8("spin_raio_visao_escuro_quad"));
        spin_raio_visao_escuro_quad->setDecimals(1);
        spin_raio_visao_escuro_quad->setSingleStep(1.000000000000000);

        horizontalLayout_20->addWidget(spin_raio_visao_escuro_quad);

        label_33 = new QLabel(tab_geral);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        sizePolicy7.setHeightForWidth(label_33->sizePolicy().hasHeightForWidth());
        label_33->setSizePolicy(sizePolicy7);
        label_33->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_33);


        verticalLayout_11->addLayout(horizontalLayout_20);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        checkbox_salvacao = new QCheckBox(tab_geral);
        checkbox_salvacao->setObjectName(QString::fromUtf8("checkbox_salvacao"));

        horizontalLayout_16->addWidget(checkbox_salvacao);

        combo_salvacao = new QComboBox(tab_geral);
        combo_salvacao->addItem(QString());
        combo_salvacao->addItem(QString());
        combo_salvacao->addItem(QString());
        combo_salvacao->addItem(QString());
        combo_salvacao->setObjectName(QString::fromUtf8("combo_salvacao"));

        horizontalLayout_16->addWidget(combo_salvacao);


        verticalLayout_11->addLayout(horizontalLayout_16);


        gridLayout_8->addLayout(verticalLayout_11, 5, 0, 3, 1);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QString::fromUtf8("horizontalLayout_21"));
        label_16 = new QLabel(tab_geral);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_16);

        combo_modelos_3d = new QComboBox(tab_geral);
        combo_modelos_3d->setObjectName(QString::fromUtf8("combo_modelos_3d"));

        horizontalLayout_21->addWidget(combo_modelos_3d);


        gridLayout_8->addLayout(horizontalLayout_21, 6, 1, 1, 1);

        horizontalLayout_35 = new QHBoxLayout();
        horizontalLayout_35->setObjectName(QString::fromUtf8("horizontalLayout_35"));
        label_68 = new QLabel(tab_geral);
        label_68->setObjectName(QString::fromUtf8("label_68"));
        sizePolicy7.setHeightForWidth(label_68->sizePolicy().hasHeightForWidth());
        label_68->setSizePolicy(sizePolicy7);
        label_68->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_35->addWidget(label_68);

        lista_formas_alternativas = new QListWidget(tab_geral);
        lista_formas_alternativas->setObjectName(QString::fromUtf8("lista_formas_alternativas"));

        horizontalLayout_35->addWidget(lista_formas_alternativas);

        verticalLayout_12 = new QVBoxLayout();
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        botao_adicionar_forma_alternativa = new QPushButton(tab_geral);
        botao_adicionar_forma_alternativa->setObjectName(QString::fromUtf8("botao_adicionar_forma_alternativa"));
        QSizePolicy sizePolicy8(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(botao_adicionar_forma_alternativa->sizePolicy().hasHeightForWidth());
        botao_adicionar_forma_alternativa->setSizePolicy(sizePolicy8);

        verticalLayout_12->addWidget(botao_adicionar_forma_alternativa);

        botao_remover_forma_alternativa = new QPushButton(tab_geral);
        botao_remover_forma_alternativa->setObjectName(QString::fromUtf8("botao_remover_forma_alternativa"));
        sizePolicy8.setHeightForWidth(botao_remover_forma_alternativa->sizePolicy().hasHeightForWidth());
        botao_remover_forma_alternativa->setSizePolicy(sizePolicy8);

        verticalLayout_12->addWidget(botao_remover_forma_alternativa);


        horizontalLayout_35->addLayout(verticalLayout_12);


        gridLayout_8->addLayout(horizontalLayout_35, 7, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        groupBox_9 = new QGroupBox(tab_geral);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        horizontalLayout_49 = new QHBoxLayout(groupBox_9);
        horizontalLayout_49->setObjectName(QString::fromUtf8("horizontalLayout_49"));
        label_3 = new QLabel(groupBox_9);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_49->addWidget(label_3);

        checkbox_cor = new QCheckBox(groupBox_9);
        checkbox_cor->setObjectName(QString::fromUtf8("checkbox_cor"));
        sizePolicy2.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy2);

        horizontalLayout_49->addWidget(checkbox_cor);

        botao_cor = new QPushButton(groupBox_9);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_49->addWidget(botao_cor);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_49->addItem(horizontalSpacer_3);

        label_81 = new QLabel(groupBox_9);
        label_81->setObjectName(QString::fromUtf8("label_81"));

        horizontalLayout_49->addWidget(label_81);

        slider_alfa = new QSlider(groupBox_9);
        slider_alfa->setObjectName(QString::fromUtf8("slider_alfa"));
        sizePolicy2.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy2);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_49->addWidget(slider_alfa);


        horizontalLayout_7->addWidget(groupBox_9);


        horizontalLayout_2->addLayout(horizontalLayout_7);


        gridLayout_8->addLayout(horizontalLayout_2, 0, 1, 1, 1);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        groupBox_8 = new QGroupBox(tab_geral);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        horizontalLayout_46 = new QHBoxLayout(groupBox_8);
        horizontalLayout_46->setObjectName(QString::fromUtf8("horizontalLayout_46"));
        label_12 = new QLabel(groupBox_8);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        sizePolicy7.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy7);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_46->addWidget(label_12);

        spin_raio_quad = new QDoubleSpinBox(groupBox_8);
        spin_raio_quad->setObjectName(QString::fromUtf8("spin_raio_quad"));
        sizePolicy2.setHeightForWidth(spin_raio_quad->sizePolicy().hasHeightForWidth());
        spin_raio_quad->setSizePolicy(sizePolicy2);
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1.000000000000000);

        horizontalLayout_46->addWidget(spin_raio_quad);

        label_31 = new QLabel(groupBox_8);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        sizePolicy7.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy7);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_46->addWidget(label_31);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_46->addItem(horizontalSpacer_2);

        botao_luz = new QPushButton(groupBox_8);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        sizePolicy2.setHeightForWidth(botao_luz->sizePolicy().hasHeightForWidth());
        botao_luz->setSizePolicy(sizePolicy2);
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_46->addWidget(botao_luz);


        horizontalLayout_14->addWidget(groupBox_8);


        gridLayout_8->addLayout(horizontalLayout_14, 1, 1, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        horizontalLayout_24 = new QHBoxLayout();
        horizontalLayout_24->setObjectName(QString::fromUtf8("horizontalLayout_24"));
        groupBox_10 = new QGroupBox(tab_geral);
        groupBox_10->setObjectName(QString::fromUtf8("groupBox_10"));
        gridLayout_21 = new QGridLayout(groupBox_10);
        gridLayout_21->setObjectName(QString::fromUtf8("gridLayout_21"));
        combo_textura = new QComboBox(groupBox_10);
        combo_textura->setObjectName(QString::fromUtf8("combo_textura"));

        gridLayout_21->addWidget(combo_textura, 0, 0, 1, 1);

        horizontalLayout_22 = new QHBoxLayout();
        horizontalLayout_22->setObjectName(QString::fromUtf8("horizontalLayout_22"));
        label_17 = new QLabel(groupBox_10);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_22->addWidget(label_17);

        spin_tex_altura = new QDoubleSpinBox(groupBox_10);
        spin_tex_altura->setObjectName(QString::fromUtf8("spin_tex_altura"));
        spin_tex_altura->setDecimals(2);
        spin_tex_altura->setMaximum(1.000000000000000);
        spin_tex_altura->setSingleStep(0.100000000000000);

        horizontalLayout_22->addWidget(spin_tex_altura);


        gridLayout_21->addLayout(horizontalLayout_22, 1, 0, 1, 1);

        horizontalLayout_23 = new QHBoxLayout();
        horizontalLayout_23->setObjectName(QString::fromUtf8("horizontalLayout_23"));
        label_18 = new QLabel(groupBox_10);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_23->addWidget(label_18);

        spin_tex_largura = new QDoubleSpinBox(groupBox_10);
        spin_tex_largura->setObjectName(QString::fromUtf8("spin_tex_largura"));
        spin_tex_largura->setDecimals(2);
        spin_tex_largura->setMaximum(1.000000000000000);
        spin_tex_largura->setSingleStep(0.100000000000000);

        horizontalLayout_23->addWidget(spin_tex_largura);


        gridLayout_21->addLayout(horizontalLayout_23, 3, 0, 1, 1);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName(QString::fromUtf8("horizontalLayout_25"));
        label_19 = new QLabel(groupBox_10);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_25->addWidget(label_19);

        spin_tex_trans_x = new QDoubleSpinBox(groupBox_10);
        spin_tex_trans_x->setObjectName(QString::fromUtf8("spin_tex_trans_x"));
        spin_tex_trans_x->setDecimals(2);
        spin_tex_trans_x->setMinimum(-1.000000000000000);
        spin_tex_trans_x->setMaximum(1.000000000000000);
        spin_tex_trans_x->setSingleStep(0.100000000000000);

        horizontalLayout_25->addWidget(spin_tex_trans_x);


        gridLayout_21->addLayout(horizontalLayout_25, 1, 3, 1, 1);

        horizontalLayout_26 = new QHBoxLayout();
        horizontalLayout_26->setObjectName(QString::fromUtf8("horizontalLayout_26"));
        label_20 = new QLabel(groupBox_10);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_26->addWidget(label_20);

        spin_tex_trans_y = new QDoubleSpinBox(groupBox_10);
        spin_tex_trans_y->setObjectName(QString::fromUtf8("spin_tex_trans_y"));
        spin_tex_trans_y->setDecimals(2);
        spin_tex_trans_y->setMinimum(-1.000000000000000);
        spin_tex_trans_y->setMaximum(1.000000000000000);
        spin_tex_trans_y->setSingleStep(0.100000000000000);

        horizontalLayout_26->addWidget(spin_tex_trans_y);


        gridLayout_21->addLayout(horizontalLayout_26, 3, 3, 1, 1);


        horizontalLayout_24->addWidget(groupBox_10);


        gridLayout->addLayout(horizontalLayout_24, 0, 0, 2, 1);


        gridLayout_8->addLayout(gridLayout, 2, 1, 4, 1);

        tabs->addTab(tab_geral, QString());
        tab_estatisticas = new QWidget();
        tab_estatisticas->setObjectName(QString::fromUtf8("tab_estatisticas"));
        gridLayout_17 = new QGridLayout(tab_estatisticas);
        gridLayout_17->setObjectName(QString::fromUtf8("gridLayout_17"));
        groupBox_5 = new QGroupBox(tab_estatisticas);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout_16 = new QGridLayout(groupBox_5);
        gridLayout_16->setObjectName(QString::fromUtf8("gridLayout_16"));
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        label_65 = new QLabel(groupBox_5);
        label_65->setObjectName(QString::fromUtf8("label_65"));
        label_65->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_65, 2, 0, 1, 1);

        label_66 = new QLabel(groupBox_5);
        label_66->setObjectName(QString::fromUtf8("label_66"));
        label_66->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_66, 3, 0, 1, 1);

        botao_bonus_salvacao_fortitude = new QPushButton(groupBox_5);
        botao_bonus_salvacao_fortitude->setObjectName(QString::fromUtf8("botao_bonus_salvacao_fortitude"));

        gridLayout_4->addWidget(botao_bonus_salvacao_fortitude, 1, 1, 1, 1);

        botao_bonus_salvacao_vontade = new QPushButton(groupBox_5);
        botao_bonus_salvacao_vontade->setObjectName(QString::fromUtf8("botao_bonus_salvacao_vontade"));

        gridLayout_4->addWidget(botao_bonus_salvacao_vontade, 3, 1, 1, 1);

        label_67 = new QLabel(groupBox_5);
        label_67->setObjectName(QString::fromUtf8("label_67"));
        label_67->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_67, 1, 0, 1, 1);

        botao_bonus_salvacao_reflexo = new QPushButton(groupBox_5);
        botao_bonus_salvacao_reflexo->setObjectName(QString::fromUtf8("botao_bonus_salvacao_reflexo"));

        gridLayout_4->addWidget(botao_bonus_salvacao_reflexo, 2, 1, 1, 1);

        label_70 = new QLabel(groupBox_5);
        label_70->setObjectName(QString::fromUtf8("label_70"));
        sizePolicy8.setHeightForWidth(label_70->sizePolicy().hasHeightForWidth());
        label_70->setSizePolicy(sizePolicy8);
        QFont font;
        font.setFamily(QString::fromUtf8("Noto Sans [unknown]"));
        font.setBold(true);
        font.setWeight(75);
        label_70->setFont(font);
        label_70->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_70, 0, 1, 1, 1);


        gridLayout_16->addLayout(gridLayout_4, 0, 0, 1, 1);


        gridLayout_17->addWidget(groupBox_5, 2, 0, 2, 1);

        groupBox_4 = new QGroupBox(tab_estatisticas);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setStyleSheet(QString::fromUtf8(""));
        gridLayout_18 = new QGridLayout(groupBox_4);
        gridLayout_18->setObjectName(QString::fromUtf8("gridLayout_18"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        botao_bonus_constituicao = new QPushButton(groupBox_4);
        botao_bonus_constituicao->setObjectName(QString::fromUtf8("botao_bonus_constituicao"));

        gridLayout_2->addWidget(botao_bonus_constituicao, 3, 2, 1, 1);

        label_47 = new QLabel(groupBox_4);
        label_47->setObjectName(QString::fromUtf8("label_47"));
        label_47->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_47, 4, 0, 1, 1);

        spin_destreza = new QSpinBox(groupBox_4);
        spin_destreza->setObjectName(QString::fromUtf8("spin_destreza"));

        gridLayout_2->addWidget(spin_destreza, 2, 1, 1, 1);

        label_49 = new QLabel(groupBox_4);
        label_49->setObjectName(QString::fromUtf8("label_49"));
        label_49->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_49, 5, 0, 1, 1);

        label_51 = new QLabel(groupBox_4);
        label_51->setObjectName(QString::fromUtf8("label_51"));
        label_51->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_51, 6, 0, 1, 1);

        botao_bonus_destreza = new QPushButton(groupBox_4);
        botao_bonus_destreza->setObjectName(QString::fromUtf8("botao_bonus_destreza"));

        gridLayout_2->addWidget(botao_bonus_destreza, 2, 2, 1, 1);

        label_50 = new QLabel(groupBox_4);
        label_50->setObjectName(QString::fromUtf8("label_50"));
        label_50->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_50, 3, 0, 1, 1);

        label_mod_destreza = new QLabel(groupBox_4);
        label_mod_destreza->setObjectName(QString::fromUtf8("label_mod_destreza"));
        label_mod_destreza->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_destreza, 2, 3, 1, 1);

        spin_carisma = new QSpinBox(groupBox_4);
        spin_carisma->setObjectName(QString::fromUtf8("spin_carisma"));

        gridLayout_2->addWidget(spin_carisma, 6, 1, 1, 1);

        botao_bonus_forca = new QPushButton(groupBox_4);
        botao_bonus_forca->setObjectName(QString::fromUtf8("botao_bonus_forca"));

        gridLayout_2->addWidget(botao_bonus_forca, 1, 2, 1, 1);

        spin_sabedoria = new QSpinBox(groupBox_4);
        spin_sabedoria->setObjectName(QString::fromUtf8("spin_sabedoria"));

        gridLayout_2->addWidget(spin_sabedoria, 5, 1, 1, 1);

        spin_inteligencia = new QSpinBox(groupBox_4);
        spin_inteligencia->setObjectName(QString::fromUtf8("spin_inteligencia"));

        gridLayout_2->addWidget(spin_inteligencia, 4, 1, 1, 1);

        spin_forca = new QSpinBox(groupBox_4);
        spin_forca->setObjectName(QString::fromUtf8("spin_forca"));

        gridLayout_2->addWidget(spin_forca, 1, 1, 1, 1);

        label_27 = new QLabel(groupBox_4);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_27, 1, 0, 1, 1);

        spin_constituicao = new QSpinBox(groupBox_4);
        spin_constituicao->setObjectName(QString::fromUtf8("spin_constituicao"));

        gridLayout_2->addWidget(spin_constituicao, 3, 1, 1, 1);

        label_mod_forca = new QLabel(groupBox_4);
        label_mod_forca->setObjectName(QString::fromUtf8("label_mod_forca"));
        label_mod_forca->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_forca, 1, 3, 1, 1);

        label_mod_sabedoria = new QLabel(groupBox_4);
        label_mod_sabedoria->setObjectName(QString::fromUtf8("label_mod_sabedoria"));
        label_mod_sabedoria->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_sabedoria, 5, 3, 1, 1);

        botao_bonus_sabedoria = new QPushButton(groupBox_4);
        botao_bonus_sabedoria->setObjectName(QString::fromUtf8("botao_bonus_sabedoria"));

        gridLayout_2->addWidget(botao_bonus_sabedoria, 5, 2, 1, 1);

        label_48 = new QLabel(groupBox_4);
        label_48->setObjectName(QString::fromUtf8("label_48"));
        label_48->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_48, 2, 0, 1, 1);

        botao_bonus_inteligencia = new QPushButton(groupBox_4);
        botao_bonus_inteligencia->setObjectName(QString::fromUtf8("botao_bonus_inteligencia"));

        gridLayout_2->addWidget(botao_bonus_inteligencia, 4, 2, 1, 1);

        label_mod_inteligencia = new QLabel(groupBox_4);
        label_mod_inteligencia->setObjectName(QString::fromUtf8("label_mod_inteligencia"));
        label_mod_inteligencia->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_inteligencia, 4, 3, 1, 1);

        botao_bonus_carisma = new QPushButton(groupBox_4);
        botao_bonus_carisma->setObjectName(QString::fromUtf8("botao_bonus_carisma"));

        gridLayout_2->addWidget(botao_bonus_carisma, 6, 2, 1, 1);

        label_mod_constituicao = new QLabel(groupBox_4);
        label_mod_constituicao->setObjectName(QString::fromUtf8("label_mod_constituicao"));
        label_mod_constituicao->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_constituicao, 3, 3, 1, 1);

        label_mod_carisma = new QLabel(groupBox_4);
        label_mod_carisma->setObjectName(QString::fromUtf8("label_mod_carisma"));
        label_mod_carisma->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_carisma, 6, 3, 1, 1);

        label_28 = new QLabel(groupBox_4);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        sizePolicy8.setHeightForWidth(label_28->sizePolicy().hasHeightForWidth());
        label_28->setSizePolicy(sizePolicy8);
        label_28->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_28, 0, 3, 1, 1);

        label_63 = new QLabel(groupBox_4);
        label_63->setObjectName(QString::fromUtf8("label_63"));
        sizePolicy8.setHeightForWidth(label_63->sizePolicy().hasHeightForWidth());
        label_63->setSizePolicy(sizePolicy8);
        label_63->setFont(font);
        label_63->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_63, 0, 2, 1, 1);

        label_62 = new QLabel(groupBox_4);
        label_62->setObjectName(QString::fromUtf8("label_62"));
        sizePolicy8.setHeightForWidth(label_62->sizePolicy().hasHeightForWidth());
        label_62->setSizePolicy(sizePolicy8);
        label_62->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_62, 0, 1, 1, 1);


        gridLayout_18->addLayout(gridLayout_2, 0, 0, 1, 1);


        gridLayout_17->addWidget(groupBox_4, 0, 0, 2, 1);

        groupBox_7 = new QGroupBox(tab_estatisticas);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        groupBox_7->setStyleSheet(QString::fromUtf8(""));
        layoutWidget_3 = new QWidget(groupBox_7);
        layoutWidget_3->setObjectName(QString::fromUtf8("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(0, 10, 341, 33));
        horizontalLayout_28 = new QHBoxLayout(layoutWidget_3);
        horizontalLayout_28->setObjectName(QString::fromUtf8("horizontalLayout_28"));
        horizontalLayout_28->setContentsMargins(0, 0, 0, 0);
        label_22 = new QLabel(layoutWidget_3);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        sizePolicy.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy);
        label_22->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_28->addWidget(label_22);

        botao_bonus_iniciativa = new QPushButton(layoutWidget_3);
        botao_bonus_iniciativa->setObjectName(QString::fromUtf8("botao_bonus_iniciativa"));

        horizontalLayout_28->addWidget(botao_bonus_iniciativa);

        checkbox_iniciativa = new QCheckBox(layoutWidget_3);
        checkbox_iniciativa->setObjectName(QString::fromUtf8("checkbox_iniciativa"));
        sizePolicy2.setHeightForWidth(checkbox_iniciativa->sizePolicy().hasHeightForWidth());
        checkbox_iniciativa->setSizePolicy(sizePolicy2);

        horizontalLayout_28->addWidget(checkbox_iniciativa);

        spin_iniciativa = new QSpinBox(layoutWidget_3);
        spin_iniciativa->setObjectName(QString::fromUtf8("spin_iniciativa"));
        sizePolicy2.setHeightForWidth(spin_iniciativa->sizePolicy().hasHeightForWidth());
        spin_iniciativa->setSizePolicy(sizePolicy2);
        spin_iniciativa->setMinimum(-100);
        spin_iniciativa->setMaximum(999);

        horizontalLayout_28->addWidget(spin_iniciativa);


        gridLayout_17->addWidget(groupBox_7, 0, 1, 1, 1);

        groupBox_6 = new QGroupBox(tab_estatisticas);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        groupBox_6->setStyleSheet(QString::fromUtf8(""));
        gridLayout_15 = new QGridLayout(groupBox_6);
        gridLayout_15->setObjectName(QString::fromUtf8("gridLayout_15"));
        gridLayout_5 = new QGridLayout();
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        label_bba_agarrar = new QLabel(groupBox_6);
        label_bba_agarrar->setObjectName(QString::fromUtf8("label_bba_agarrar"));
        label_bba_agarrar->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_agarrar, 1, 1, 1, 1);

        label_69 = new QLabel(groupBox_6);
        label_69->setObjectName(QString::fromUtf8("label_69"));
        label_69->setFont(font);
        label_69->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_69, 0, 2, 1, 1);

        label_54 = new QLabel(groupBox_6);
        label_54->setObjectName(QString::fromUtf8("label_54"));
        label_54->setFont(font);
        label_54->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_54, 0, 0, 1, 1);

        label_bba_base = new QLabel(groupBox_6);
        label_bba_base->setObjectName(QString::fromUtf8("label_bba_base"));
        label_bba_base->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_base, 1, 0, 1, 1);

        label_56 = new QLabel(groupBox_6);
        label_56->setObjectName(QString::fromUtf8("label_56"));
        label_56->setFont(font);
        label_56->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_56, 0, 1, 1, 1);

        label_71 = new QLabel(groupBox_6);
        label_71->setObjectName(QString::fromUtf8("label_71"));
        label_71->setFont(font);

        gridLayout_5->addWidget(label_71, 0, 3, 1, 1);

        label_bba_cac = new QLabel(groupBox_6);
        label_bba_cac->setObjectName(QString::fromUtf8("label_bba_cac"));
        label_bba_cac->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_cac, 1, 2, 1, 1);

        label_bba_distancia = new QLabel(groupBox_6);
        label_bba_distancia->setObjectName(QString::fromUtf8("label_bba_distancia"));
        label_bba_distancia->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_distancia, 1, 3, 1, 1);


        gridLayout_15->addLayout(gridLayout_5, 0, 0, 1, 1);


        gridLayout_17->addWidget(groupBox_6, 0, 2, 1, 1);

        groupBox_3 = new QGroupBox(tab_estatisticas);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_14 = new QGridLayout(groupBox_3);
        gridLayout_14->setObjectName(QString::fromUtf8("gridLayout_14"));
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        spin_ca_escudo_melhoria = new QSpinBox(groupBox_3);
        spin_ca_escudo_melhoria->setObjectName(QString::fromUtf8("spin_ca_escudo_melhoria"));
        spin_ca_escudo_melhoria->setMinimum(0);

        gridLayout_3->addWidget(spin_ca_escudo_melhoria, 2, 4, 1, 1);

        combo_material_escudo = new QComboBox(groupBox_3);
        combo_material_escudo->addItem(QString());
        combo_material_escudo->addItem(QString());
        combo_material_escudo->addItem(QString());
        combo_material_escudo->addItem(QString());
        combo_material_escudo->addItem(QString());
        combo_material_escudo->setObjectName(QString::fromUtf8("combo_material_escudo"));
        sizePolicy2.setHeightForWidth(combo_material_escudo->sizePolicy().hasHeightForWidth());
        combo_material_escudo->setSizePolicy(sizePolicy2);

        gridLayout_3->addWidget(combo_material_escudo, 2, 2, 1, 1);

        spin_bonus_escudo = new QLabel(groupBox_3);
        spin_bonus_escudo->setObjectName(QString::fromUtf8("spin_bonus_escudo"));
        spin_bonus_escudo->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(spin_bonus_escudo, 2, 0, 1, 1);

        combo_escudo = new QComboBox(groupBox_3);
        combo_escudo->setObjectName(QString::fromUtf8("combo_escudo"));

        gridLayout_3->addWidget(combo_escudo, 2, 1, 1, 1);

        checkbox_escudo_obra_prima = new QCheckBox(groupBox_3);
        checkbox_escudo_obra_prima->setObjectName(QString::fromUtf8("checkbox_escudo_obra_prima"));
        sizePolicy2.setHeightForWidth(checkbox_escudo_obra_prima->sizePolicy().hasHeightForWidth());
        checkbox_escudo_obra_prima->setSizePolicy(sizePolicy2);
        checkbox_escudo_obra_prima->setLayoutDirection(Qt::LeftToRight);

        gridLayout_3->addWidget(checkbox_escudo_obra_prima, 2, 3, 1, 1);

        label_ca_toque = new QLabel(groupBox_3);
        label_ca_toque->setObjectName(QString::fromUtf8("label_ca_toque"));
        label_ca_toque->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_ca_toque, 1, 10, 1, 1);

        label_95 = new QLabel(groupBox_3);
        label_95->setObjectName(QString::fromUtf8("label_95"));
        sizePolicy8.setHeightForWidth(label_95->sizePolicy().hasHeightForWidth());
        label_95->setSizePolicy(sizePolicy8);
        label_95->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_95, 0, 2, 1, 1);

        label_52 = new QLabel(groupBox_3);
        label_52->setObjectName(QString::fromUtf8("label_52"));
        label_52->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_52, 1, 0, 1, 1);

        label_57 = new QLabel(groupBox_3);
        label_57->setObjectName(QString::fromUtf8("label_57"));
        label_57->setFont(font);
        label_57->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_57, 0, 9, 1, 1);

        spin_ca_armadura_melhoria = new QSpinBox(groupBox_3);
        spin_ca_armadura_melhoria->setObjectName(QString::fromUtf8("spin_ca_armadura_melhoria"));
        spin_ca_armadura_melhoria->setMinimum(0);

        gridLayout_3->addWidget(spin_ca_armadura_melhoria, 1, 4, 1, 1);

        label_58 = new QLabel(groupBox_3);
        label_58->setObjectName(QString::fromUtf8("label_58"));
        label_58->setFont(font);
        label_58->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_58, 0, 10, 1, 1);

        label_59 = new QLabel(groupBox_3);
        label_59->setObjectName(QString::fromUtf8("label_59"));
        label_59->setFont(font);
        label_59->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_59, 0, 11, 1, 1);

        label_ca_surpreso = new QLabel(groupBox_3);
        label_ca_surpreso->setObjectName(QString::fromUtf8("label_ca_surpreso"));
        label_ca_surpreso->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_ca_surpreso, 1, 11, 1, 1);

        botao_bonus_ca = new QPushButton(groupBox_3);
        botao_bonus_ca->setObjectName(QString::fromUtf8("botao_bonus_ca"));

        gridLayout_3->addWidget(botao_bonus_ca, 1, 9, 1, 1);

        combo_armadura = new QComboBox(groupBox_3);
        combo_armadura->setObjectName(QString::fromUtf8("combo_armadura"));

        gridLayout_3->addWidget(combo_armadura, 1, 1, 1, 1);

        spin_bonus_escudo_2 = new QLabel(groupBox_3);
        spin_bonus_escudo_2->setObjectName(QString::fromUtf8("spin_bonus_escudo_2"));
        spin_bonus_escudo_2->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(spin_bonus_escudo_2, 0, 6, 1, 1);

        combo_material_armadura = new QComboBox(groupBox_3);
        combo_material_armadura->addItem(QString());
        combo_material_armadura->addItem(QString());
        combo_material_armadura->addItem(QString());
        combo_material_armadura->addItem(QString());
        combo_material_armadura->setObjectName(QString::fromUtf8("combo_material_armadura"));
        sizePolicy2.setHeightForWidth(combo_material_armadura->sizePolicy().hasHeightForWidth());
        combo_material_armadura->setSizePolicy(sizePolicy2);

        gridLayout_3->addWidget(combo_material_armadura, 1, 2, 1, 1);

        label_100 = new QLabel(groupBox_3);
        label_100->setObjectName(QString::fromUtf8("label_100"));
        label_100->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_100, 0, 4, 1, 1);

        checkbox_armadura_obra_prima = new QCheckBox(groupBox_3);
        checkbox_armadura_obra_prima->setObjectName(QString::fromUtf8("checkbox_armadura_obra_prima"));
        sizePolicy2.setHeightForWidth(checkbox_armadura_obra_prima->sizePolicy().hasHeightForWidth());
        checkbox_armadura_obra_prima->setSizePolicy(sizePolicy2);

        gridLayout_3->addWidget(checkbox_armadura_obra_prima, 1, 3, 1, 1);

        label_61 = new QLabel(groupBox_3);
        label_61->setObjectName(QString::fromUtf8("label_61"));
        label_61->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_61, 1, 6, 1, 1);

        label_53 = new QLabel(groupBox_3);
        label_53->setObjectName(QString::fromUtf8("label_53"));
        label_53->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_53, 0, 1, 1, 1);

        label_60 = new QLabel(groupBox_3);
        label_60->setObjectName(QString::fromUtf8("label_60"));
        label_60->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_60, 0, 3, 1, 1);


        gridLayout_14->addLayout(gridLayout_3, 0, 0, 1, 1);


        gridLayout_17->addWidget(groupBox_3, 1, 1, 1, 2);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_85 = new QLabel(tab_estatisticas);
        label_85->setObjectName(QString::fromUtf8("label_85"));
        label_85->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_11->addWidget(label_85);

        botao_resistencia_magia = new QPushButton(tab_estatisticas);
        botao_resistencia_magia->setObjectName(QString::fromUtf8("botao_resistencia_magia"));

        horizontalLayout_11->addWidget(botao_resistencia_magia);


        gridLayout_17->addLayout(horizontalLayout_11, 4, 0, 1, 1);

        checkbox_imune_critico = new QCheckBox(tab_estatisticas);
        checkbox_imune_critico->setObjectName(QString::fromUtf8("checkbox_imune_critico"));

        gridLayout_17->addWidget(checkbox_imune_critico, 4, 1, 1, 1);

        groupBox = new QGroupBox(tab_estatisticas);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_19 = new QGridLayout(groupBox);
        gridLayout_19->setObjectName(QString::fromUtf8("gridLayout_19"));
        label_37 = new QLabel(groupBox);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        label_37->setFont(font1);
        label_37->setAlignment(Qt::AlignCenter);

        gridLayout_19->addWidget(label_37, 0, 2, 1, 1);

        label_25 = new QLabel(groupBox);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_19->addWidget(label_25, 1, 0, 1, 1);

        label_90 = new QLabel(groupBox);
        label_90->setObjectName(QString::fromUtf8("label_90"));
        label_90->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_19->addWidget(label_90, 3, 0, 1, 1);

        label_32 = new QLabel(groupBox);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_19->addWidget(label_32, 2, 0, 1, 1);

        spin_mov_aereo = new QSpinBox(groupBox);
        spin_mov_aereo->setObjectName(QString::fromUtf8("spin_mov_aereo"));

        gridLayout_19->addWidget(spin_mov_aereo, 2, 1, 1, 1);

        spin_mov_nadando = new QSpinBox(groupBox);
        spin_mov_nadando->setObjectName(QString::fromUtf8("spin_mov_nadando"));

        gridLayout_19->addWidget(spin_mov_nadando, 3, 1, 1, 1);

        spin_mov_terrestre = new QSpinBox(groupBox);
        spin_mov_terrestre->setObjectName(QString::fromUtf8("spin_mov_terrestre"));

        gridLayout_19->addWidget(spin_mov_terrestre, 1, 1, 1, 1);

        spin_mov_escavando = new QSpinBox(groupBox);
        spin_mov_escavando->setObjectName(QString::fromUtf8("spin_mov_escavando"));

        gridLayout_19->addWidget(spin_mov_escavando, 4, 1, 1, 1);

        label_97 = new QLabel(groupBox);
        label_97->setObjectName(QString::fromUtf8("label_97"));
        label_97->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_19->addWidget(label_97, 4, 0, 1, 1);

        botao_mov_terrestre = new QPushButton(groupBox);
        botao_mov_terrestre->setObjectName(QString::fromUtf8("botao_mov_terrestre"));

        gridLayout_19->addWidget(botao_mov_terrestre, 1, 2, 1, 1);

        botao_mov_nadando = new QPushButton(groupBox);
        botao_mov_nadando->setObjectName(QString::fromUtf8("botao_mov_nadando"));

        gridLayout_19->addWidget(botao_mov_nadando, 3, 2, 1, 1);

        botao_mov_aereo = new QPushButton(groupBox);
        botao_mov_aereo->setObjectName(QString::fromUtf8("botao_mov_aereo"));

        gridLayout_19->addWidget(botao_mov_aereo, 2, 2, 1, 1);

        botao_mov_escavando = new QPushButton(groupBox);
        botao_mov_escavando->setObjectName(QString::fromUtf8("botao_mov_escavando"));

        gridLayout_19->addWidget(botao_mov_escavando, 4, 2, 1, 1);

        label_15 = new QLabel(groupBox);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_19->addWidget(label_15, 0, 1, 1, 1);

        spin_mov_escalando = new QSpinBox(groupBox);
        spin_mov_escalando->setObjectName(QString::fromUtf8("spin_mov_escalando"));

        gridLayout_19->addWidget(spin_mov_escalando, 5, 1, 1, 1);

        label_74 = new QLabel(groupBox);
        label_74->setObjectName(QString::fromUtf8("label_74"));
        label_74->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_19->addWidget(label_74, 5, 0, 1, 1);

        botao_mov_escalando = new QPushButton(groupBox);
        botao_mov_escalando->setObjectName(QString::fromUtf8("botao_mov_escalando"));

        gridLayout_19->addWidget(botao_mov_escalando, 5, 2, 1, 1);


        gridLayout_17->addWidget(groupBox, 3, 1, 1, 1);

        tabs->addTab(tab_estatisticas, QString());
        tab_7 = new QWidget();
        tab_7->setObjectName(QString::fromUtf8("tab_7"));
        horizontalLayout_34 = new QHBoxLayout(tab_7);
        horizontalLayout_34->setObjectName(QString::fromUtf8("horizontalLayout_34"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        label_11 = new QLabel(tab_7);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        sizePolicy7.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy7);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_19->addWidget(label_11);

        tabela_lista_eventos = new QTableView(tab_7);
        tabela_lista_eventos->setObjectName(QString::fromUtf8("tabela_lista_eventos"));

        horizontalLayout_19->addWidget(tabela_lista_eventos);

        verticalLayout_13 = new QVBoxLayout();
        verticalLayout_13->setObjectName(QString::fromUtf8("verticalLayout_13"));
        botao_adicionar_evento = new QPushButton(tab_7);
        botao_adicionar_evento->setObjectName(QString::fromUtf8("botao_adicionar_evento"));

        verticalLayout_13->addWidget(botao_adicionar_evento);

        botao_remover_evento = new QPushButton(tab_7);
        botao_remover_evento->setObjectName(QString::fromUtf8("botao_remover_evento"));

        verticalLayout_13->addWidget(botao_remover_evento);


        horizontalLayout_19->addLayout(verticalLayout_13);


        horizontalLayout_34->addLayout(horizontalLayout_19);

        tabs->addTab(tab_7, QString());
        tab_nivel = new QWidget();
        tab_nivel->setObjectName(QString::fromUtf8("tab_nivel"));
        gridLayout_9 = new QGridLayout(tab_nivel);
        gridLayout_9->setObjectName(QString::fromUtf8("gridLayout_9"));
        horizontalLayout_31 = new QHBoxLayout();
        horizontalLayout_31->setObjectName(QString::fromUtf8("horizontalLayout_31"));
        label_39 = new QLabel(tab_nivel);
        label_39->setObjectName(QString::fromUtf8("label_39"));
        sizePolicy5.setHeightForWidth(label_39->sizePolicy().hasHeightForWidth());
        label_39->setSizePolicy(sizePolicy5);

        horizontalLayout_31->addWidget(label_39, 0, Qt::AlignRight);

        linha_nivel = new QLineEdit(tab_nivel);
        linha_nivel->setObjectName(QString::fromUtf8("linha_nivel"));
        linha_nivel->setReadOnly(true);

        horizontalLayout_31->addWidget(linha_nivel, 0, Qt::AlignLeft);


        gridLayout_9->addLayout(horizontalLayout_31, 0, 0, 1, 1);

        horizontalLayout_37 = new QHBoxLayout();
        horizontalLayout_37->setObjectName(QString::fromUtf8("horizontalLayout_37"));
        label_80 = new QLabel(tab_nivel);
        label_80->setObjectName(QString::fromUtf8("label_80"));
        sizePolicy5.setHeightForWidth(label_80->sizePolicy().hasHeightForWidth());
        label_80->setSizePolicy(sizePolicy5);
        label_80->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_37->addWidget(label_80);

        spin_niveis_negativos = new QSpinBox(tab_nivel);
        spin_niveis_negativos->setObjectName(QString::fromUtf8("spin_niveis_negativos"));
        sizePolicy2.setHeightForWidth(spin_niveis_negativos->sizePolicy().hasHeightForWidth());
        spin_niveis_negativos->setSizePolicy(sizePolicy2);
        spin_niveis_negativos->setMinimum(0);
        spin_niveis_negativos->setValue(0);

        horizontalLayout_37->addWidget(spin_niveis_negativos);


        gridLayout_9->addLayout(horizontalLayout_37, 0, 1, 1, 1);

        horizontalLayout_33 = new QHBoxLayout();
        horizontalLayout_33->setObjectName(QString::fromUtf8("horizontalLayout_33"));
        label_46 = new QLabel(tab_nivel);
        label_46->setObjectName(QString::fromUtf8("label_46"));
        sizePolicy5.setHeightForWidth(label_46->sizePolicy().hasHeightForWidth());
        label_46->setSizePolicy(sizePolicy5);

        horizontalLayout_33->addWidget(label_46, 0, Qt::AlignRight);

        linha_bba = new QLineEdit(tab_nivel);
        linha_bba->setObjectName(QString::fromUtf8("linha_bba"));
        linha_bba->setReadOnly(true);

        horizontalLayout_33->addWidget(linha_bba, 0, Qt::AlignLeft);


        gridLayout_9->addLayout(horizontalLayout_33, 0, 2, 1, 1);

        horizontalLayout_38 = new QHBoxLayout();
        horizontalLayout_38->setObjectName(QString::fromUtf8("horizontalLayout_38"));
        label_84 = new QLabel(tab_nivel);
        label_84->setObjectName(QString::fromUtf8("label_84"));
        sizePolicy5.setHeightForWidth(label_84->sizePolicy().hasHeightForWidth());
        label_84->setSizePolicy(sizePolicy5);

        horizontalLayout_38->addWidget(label_84, 0, Qt::AlignRight);

        spin_xp = new QSpinBox(tab_nivel);
        spin_xp->setObjectName(QString::fromUtf8("spin_xp"));
        spin_xp->setMinimum(0);
        spin_xp->setMaximum(1000000);

        horizontalLayout_38->addWidget(spin_xp);


        gridLayout_9->addLayout(horizontalLayout_38, 0, 3, 1, 1);

        horizontalLayout_43 = new QHBoxLayout();
        horizontalLayout_43->setObjectName(QString::fromUtf8("horizontalLayout_43"));
        label_75 = new QLabel(tab_nivel);
        label_75->setObjectName(QString::fromUtf8("label_75"));

        horizontalLayout_43->addWidget(label_75);

        slider_bem_mal = new QSlider(tab_nivel);
        slider_bem_mal->setObjectName(QString::fromUtf8("slider_bem_mal"));
        sizePolicy3.setHeightForWidth(slider_bem_mal->sizePolicy().hasHeightForWidth());
        slider_bem_mal->setSizePolicy(sizePolicy3);
        slider_bem_mal->setMaximum(8);
        slider_bem_mal->setOrientation(Qt::Horizontal);

        horizontalLayout_43->addWidget(slider_bem_mal);

        label_78 = new QLabel(tab_nivel);
        label_78->setObjectName(QString::fromUtf8("label_78"));

        horizontalLayout_43->addWidget(label_78);


        gridLayout_9->addLayout(horizontalLayout_43, 1, 0, 1, 1);

        horizontalLayout_39 = new QHBoxLayout();
        horizontalLayout_39->setObjectName(QString::fromUtf8("horizontalLayout_39"));
        label_76 = new QLabel(tab_nivel);
        label_76->setObjectName(QString::fromUtf8("label_76"));

        horizontalLayout_39->addWidget(label_76);

        slider_ordem_caos = new QSlider(tab_nivel);
        slider_ordem_caos->setObjectName(QString::fromUtf8("slider_ordem_caos"));
        sizePolicy3.setHeightForWidth(slider_ordem_caos->sizePolicy().hasHeightForWidth());
        slider_ordem_caos->setSizePolicy(sizePolicy3);
        slider_ordem_caos->setMaximum(8);
        slider_ordem_caos->setOrientation(Qt::Horizontal);

        horizontalLayout_39->addWidget(slider_ordem_caos);

        label_77 = new QLabel(tab_nivel);
        label_77->setObjectName(QString::fromUtf8("label_77"));

        horizontalLayout_39->addWidget(label_77);


        gridLayout_9->addLayout(horizontalLayout_39, 1, 1, 1, 1);

        horizontalLayout_47 = new QHBoxLayout();
        horizontalLayout_47->setObjectName(QString::fromUtf8("horizontalLayout_47"));
        label_101 = new QLabel(tab_nivel);
        label_101->setObjectName(QString::fromUtf8("label_101"));
        sizePolicy7.setHeightForWidth(label_101->sizePolicy().hasHeightForWidth());
        label_101->setSizePolicy(sizePolicy7);

        horizontalLayout_47->addWidget(label_101);

        combo_raca = new QComboBox(tab_nivel);
        combo_raca->setObjectName(QString::fromUtf8("combo_raca"));

        horizontalLayout_47->addWidget(combo_raca);


        gridLayout_9->addLayout(horizontalLayout_47, 1, 2, 1, 1);

        groupBox_2 = new QGroupBox(tab_nivel);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_20 = new QGridLayout(groupBox_2);
        gridLayout_20->setObjectName(QString::fromUtf8("gridLayout_20"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_40 = new QLabel(groupBox_2);
        label_40->setObjectName(QString::fromUtf8("label_40"));
        sizePolicy7.setHeightForWidth(label_40->sizePolicy().hasHeightForWidth());
        label_40->setSizePolicy(sizePolicy7);

        horizontalLayout->addWidget(label_40);

        combo_classe = new QComboBox(groupBox_2);
        combo_classe->setObjectName(QString::fromUtf8("combo_classe"));

        horizontalLayout->addWidget(combo_classe);

        linha_classe = new QLineEdit(groupBox_2);
        linha_classe->setObjectName(QString::fromUtf8("linha_classe"));
        sizePolicy2.setHeightForWidth(linha_classe->sizePolicy().hasHeightForWidth());
        linha_classe->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(linha_classe);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_41 = new QLabel(groupBox_2);
        label_41->setObjectName(QString::fromUtf8("label_41"));
        label_41->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_41);

        spin_nivel_classe = new QSpinBox(groupBox_2);
        spin_nivel_classe->setObjectName(QString::fromUtf8("spin_nivel_classe"));
        spin_nivel_classe->setMinimum(1);

        horizontalLayout->addWidget(spin_nivel_classe);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        label_43 = new QLabel(groupBox_2);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_43);

        spin_bba = new QSpinBox(groupBox_2);
        spin_bba->setObjectName(QString::fromUtf8("spin_bba"));
        spin_bba->setMinimum(-1);

        horizontalLayout->addWidget(spin_bba);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_7);

        label_64 = new QLabel(groupBox_2);
        label_64->setObjectName(QString::fromUtf8("label_64"));
        label_64->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_64);

        combo_salvacoes_fortes = new QComboBox(groupBox_2);
        combo_salvacoes_fortes->setObjectName(QString::fromUtf8("combo_salvacoes_fortes"));

        horizontalLayout->addWidget(combo_salvacoes_fortes);


        gridLayout_20->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_48 = new QHBoxLayout();
        horizontalLayout_48->setObjectName(QString::fromUtf8("horizontalLayout_48"));
        label_42 = new QLabel(groupBox_2);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        label_42->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_48->addWidget(label_42);

        spin_nivel_conjurador = new QSpinBox(groupBox_2);
        spin_nivel_conjurador->setObjectName(QString::fromUtf8("spin_nivel_conjurador"));
        spin_nivel_conjurador->setMinimum(-1);

        horizontalLayout_48->addWidget(spin_nivel_conjurador);

        label_mod_conjuracao = new QLabel(groupBox_2);
        label_mod_conjuracao->setObjectName(QString::fromUtf8("label_mod_conjuracao"));
        sizePolicy7.setHeightForWidth(label_mod_conjuracao->sizePolicy().hasHeightForWidth());
        label_mod_conjuracao->setSizePolicy(sizePolicy7);
        label_mod_conjuracao->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_48->addWidget(label_mod_conjuracao);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_48->addItem(horizontalSpacer_5);

        label_44 = new QLabel(groupBox_2);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        label_44->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_48->addWidget(label_44);

        combo_mod_conjuracao = new QComboBox(groupBox_2);
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->addItem(QString());
        combo_mod_conjuracao->setObjectName(QString::fromUtf8("combo_mod_conjuracao"));

        horizontalLayout_48->addWidget(combo_mod_conjuracao);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_48->addItem(horizontalSpacer_6);

        label_87 = new QLabel(groupBox_2);
        label_87->setObjectName(QString::fromUtf8("label_87"));

        horizontalLayout_48->addWidget(label_87);

        combo_especializacao_escola = new QComboBox(groupBox_2);
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->addItem(QString());
        combo_especializacao_escola->setObjectName(QString::fromUtf8("combo_especializacao_escola"));

        horizontalLayout_48->addWidget(combo_especializacao_escola);

        label_88 = new QLabel(groupBox_2);
        label_88->setObjectName(QString::fromUtf8("label_88"));

        horizontalLayout_48->addWidget(label_88);

        combo_escola_proibida_1 = new QComboBox(groupBox_2);
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->addItem(QString());
        combo_escola_proibida_1->setObjectName(QString::fromUtf8("combo_escola_proibida_1"));

        horizontalLayout_48->addWidget(combo_escola_proibida_1);

        combo_escola_proibida_2 = new QComboBox(groupBox_2);
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->addItem(QString());
        combo_escola_proibida_2->setObjectName(QString::fromUtf8("combo_escola_proibida_2"));

        horizontalLayout_48->addWidget(combo_escola_proibida_2);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_48->addItem(horizontalSpacer_8);

        label_102 = new QLabel(groupBox_2);
        label_102->setObjectName(QString::fromUtf8("label_102"));
        sizePolicy5.setHeightForWidth(label_102->sizePolicy().hasHeightForWidth());
        label_102->setSizePolicy(sizePolicy5);

        horizontalLayout_48->addWidget(label_102);

        combo_dominio_1 = new QComboBox(groupBox_2);
        combo_dominio_1->setObjectName(QString::fromUtf8("combo_dominio_1"));

        horizontalLayout_48->addWidget(combo_dominio_1);

        combo_dominio_2 = new QComboBox(groupBox_2);
        combo_dominio_2->setObjectName(QString::fromUtf8("combo_dominio_2"));

        horizontalLayout_48->addWidget(combo_dominio_2);


        gridLayout_20->addLayout(horizontalLayout_48, 1, 0, 1, 1);


        gridLayout_9->addWidget(groupBox_2, 2, 0, 1, 4);

        verticalLayout_14 = new QVBoxLayout();
        verticalLayout_14->setObjectName(QString::fromUtf8("verticalLayout_14"));
        botao_adicionar_nivel = new QPushButton(tab_nivel);
        botao_adicionar_nivel->setObjectName(QString::fromUtf8("botao_adicionar_nivel"));
        botao_adicionar_nivel->setEnabled(true);
        sizePolicy3.setHeightForWidth(botao_adicionar_nivel->sizePolicy().hasHeightForWidth());
        botao_adicionar_nivel->setSizePolicy(sizePolicy3);

        verticalLayout_14->addWidget(botao_adicionar_nivel);

        botao_remover_nivel = new QPushButton(tab_nivel);
        botao_remover_nivel->setObjectName(QString::fromUtf8("botao_remover_nivel"));
        botao_remover_nivel->setEnabled(false);
        sizePolicy3.setHeightForWidth(botao_remover_nivel->sizePolicy().hasHeightForWidth());
        botao_remover_nivel->setSizePolicy(sizePolicy3);

        verticalLayout_14->addWidget(botao_remover_nivel);


        gridLayout_9->addLayout(verticalLayout_14, 3, 3, 1, 1);

        lista_niveis = new QListWidget(tab_nivel);
        lista_niveis->setObjectName(QString::fromUtf8("lista_niveis"));
        sizePolicy1.setHeightForWidth(lista_niveis->sizePolicy().hasHeightForWidth());
        lista_niveis->setSizePolicy(sizePolicy1);

        gridLayout_9->addWidget(lista_niveis, 3, 0, 1, 3);

        tabs->addTab(tab_nivel, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        gridLayout_12 = new QGridLayout(tab_3);
        gridLayout_12->setObjectName(QString::fromUtf8("gridLayout_12"));
        gridLayout_11 = new QGridLayout();
        gridLayout_11->setObjectName(QString::fromUtf8("gridLayout_11"));
        label_pericias = new QLabel(tab_3);
        label_pericias->setObjectName(QString::fromUtf8("label_pericias"));
        sizePolicy8.setHeightForWidth(label_pericias->sizePolicy().hasHeightForWidth());
        label_pericias->setSizePolicy(sizePolicy8);
        label_pericias->setAlignment(Qt::AlignCenter);

        gridLayout_11->addWidget(label_pericias, 0, 0, 1, 1);

        tabela_pericias = new QTableView(tab_3);
        tabela_pericias->setObjectName(QString::fromUtf8("tabela_pericias"));

        gridLayout_11->addWidget(tabela_pericias, 1, 0, 1, 1);


        gridLayout_12->addLayout(gridLayout_11, 0, 0, 1, 1);

        gridLayout_10 = new QGridLayout();
        gridLayout_10->setObjectName(QString::fromUtf8("gridLayout_10"));
        label_talentos = new QLabel(tab_3);
        label_talentos->setObjectName(QString::fromUtf8("label_talentos"));
        sizePolicy8.setHeightForWidth(label_talentos->sizePolicy().hasHeightForWidth());
        label_talentos->setSizePolicy(sizePolicy8);
        label_talentos->setAlignment(Qt::AlignCenter);

        gridLayout_10->addWidget(label_talentos, 0, 0, 1, 1);

        gridLayout_6 = new QGridLayout();
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        tabela_talentos = new QTableView(tab_3);
        tabela_talentos->setObjectName(QString::fromUtf8("tabela_talentos"));

        gridLayout_6->addWidget(tabela_talentos, 0, 0, 1, 1);

        verticalLayout_15 = new QVBoxLayout();
        verticalLayout_15->setObjectName(QString::fromUtf8("verticalLayout_15"));
        botao_adicionar_talento = new QPushButton(tab_3);
        botao_adicionar_talento->setObjectName(QString::fromUtf8("botao_adicionar_talento"));

        verticalLayout_15->addWidget(botao_adicionar_talento);

        botao_remover_talento = new QPushButton(tab_3);
        botao_remover_talento->setObjectName(QString::fromUtf8("botao_remover_talento"));

        verticalLayout_15->addWidget(botao_remover_talento);


        gridLayout_6->addLayout(verticalLayout_15, 0, 1, 1, 1);


        gridLayout_10->addLayout(gridLayout_6, 1, 0, 1, 1);


        gridLayout_12->addLayout(gridLayout_10, 0, 1, 1, 1);

        tabs->addTab(tab_3, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        verticalLayout_18 = new QVBoxLayout(tab_5);
        verticalLayout_18->setObjectName(QString::fromUtf8("verticalLayout_18"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_38 = new QLabel(tab_5);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        sizePolicy3.setHeightForWidth(label_38->sizePolicy().hasHeightForWidth());
        label_38->setSizePolicy(sizePolicy3);
        label_38->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_38);

        linha_furtivo = new QLineEdit(tab_5);
        linha_furtivo->setObjectName(QString::fromUtf8("linha_furtivo"));
        sizePolicy3.setHeightForWidth(linha_furtivo->sizePolicy().hasHeightForWidth());
        linha_furtivo->setSizePolicy(sizePolicy3);

        horizontalLayout_6->addWidget(linha_furtivo);


        verticalLayout_18->addLayout(horizontalLayout_6);

        horizontalLayout_50 = new QHBoxLayout();
        horizontalLayout_50->setObjectName(QString::fromUtf8("horizontalLayout_50"));
        horizontalLayout_52 = new QHBoxLayout();
        horizontalLayout_52->setObjectName(QString::fromUtf8("horizontalLayout_52"));
        label_86 = new QLabel(tab_5);
        label_86->setObjectName(QString::fromUtf8("label_86"));
        label_86->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_52->addWidget(label_86);

        combo_esquiva_sobrenatural = new QComboBox(tab_5);
        combo_esquiva_sobrenatural->addItem(QString());
        combo_esquiva_sobrenatural->addItem(QString());
        combo_esquiva_sobrenatural->addItem(QString());
        combo_esquiva_sobrenatural->setObjectName(QString::fromUtf8("combo_esquiva_sobrenatural"));

        horizontalLayout_52->addWidget(combo_esquiva_sobrenatural);


        horizontalLayout_50->addLayout(horizontalLayout_52);

        horizontalLayout_69 = new QHBoxLayout();
        horizontalLayout_69->setObjectName(QString::fromUtf8("horizontalLayout_69"));
        label_103 = new QLabel(tab_5);
        label_103->setObjectName(QString::fromUtf8("label_103"));
        label_103->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_69->addWidget(label_103);

        combo_evasao_estatica = new QComboBox(tab_5);
        combo_evasao_estatica->addItem(QString());
        combo_evasao_estatica->addItem(QString());
        combo_evasao_estatica->addItem(QString());
        combo_evasao_estatica->setObjectName(QString::fromUtf8("combo_evasao_estatica"));
        combo_evasao_estatica->setEnabled(true);
        sizePolicy2.setHeightForWidth(combo_evasao_estatica->sizePolicy().hasHeightForWidth());
        combo_evasao_estatica->setSizePolicy(sizePolicy2);
        combo_evasao_estatica->setEditable(false);

        horizontalLayout_69->addWidget(combo_evasao_estatica);

        combo_evasao_dinamica = new QComboBox(tab_5);
        combo_evasao_dinamica->addItem(QString());
        combo_evasao_dinamica->addItem(QString());
        combo_evasao_dinamica->addItem(QString());
        combo_evasao_dinamica->setObjectName(QString::fromUtf8("combo_evasao_dinamica"));
        combo_evasao_dinamica->setEnabled(false);
        sizePolicy2.setHeightForWidth(combo_evasao_dinamica->sizePolicy().hasHeightForWidth());
        combo_evasao_dinamica->setSizePolicy(sizePolicy2);

        horizontalLayout_69->addWidget(combo_evasao_dinamica);


        horizontalLayout_50->addLayout(horizontalLayout_69);


        verticalLayout_18->addLayout(horizontalLayout_50);

        verticalLayout_17 = new QVBoxLayout();
        verticalLayout_17->setObjectName(QString::fromUtf8("verticalLayout_17"));
        label_94 = new QLabel(tab_5);
        label_94->setObjectName(QString::fromUtf8("label_94"));
        QFont font2;
        font2.setPointSize(11);
        font2.setBold(true);
        font2.setWeight(75);
        label_94->setFont(font2);

        verticalLayout_17->addWidget(label_94);

        horizontalLayout_29 = new QHBoxLayout();
        horizontalLayout_29->setObjectName(QString::fromUtf8("horizontalLayout_29"));
        tabela_inimigos_prediletos = new QTableView(tab_5);
        tabela_inimigos_prediletos->setObjectName(QString::fromUtf8("tabela_inimigos_prediletos"));
        sizePolicy1.setHeightForWidth(tabela_inimigos_prediletos->sizePolicy().hasHeightForWidth());
        tabela_inimigos_prediletos->setSizePolicy(sizePolicy1);

        horizontalLayout_29->addWidget(tabela_inimigos_prediletos);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        botao_adicionar_inimigo_predileto = new QPushButton(tab_5);
        botao_adicionar_inimigo_predileto->setObjectName(QString::fromUtf8("botao_adicionar_inimigo_predileto"));

        verticalLayout_2->addWidget(botao_adicionar_inimigo_predileto);

        botao_remover_inimigo_predileto = new QPushButton(tab_5);
        botao_remover_inimigo_predileto->setObjectName(QString::fromUtf8("botao_remover_inimigo_predileto"));

        verticalLayout_2->addWidget(botao_remover_inimigo_predileto);


        horizontalLayout_29->addLayout(verticalLayout_2);


        verticalLayout_17->addLayout(horizontalLayout_29);


        verticalLayout_18->addLayout(verticalLayout_17);

        tabs->addTab(tab_5, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QString::fromUtf8("tab_6"));
        gridLayout_13 = new QGridLayout(tab_6);
        gridLayout_13->setObjectName(QString::fromUtf8("gridLayout_13"));
        label_91 = new QLabel(tab_6);
        label_91->setObjectName(QString::fromUtf8("label_91"));
        sizePolicy8.setHeightForWidth(label_91->sizePolicy().hasHeightForWidth());
        label_91->setSizePolicy(sizePolicy8);
        label_91->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_91, 0, 0, 1, 1);

        linha_grupo_ataque = new QLineEdit(tab_6);
        linha_grupo_ataque->setObjectName(QString::fromUtf8("linha_grupo_ataque"));
        sizePolicy2.setHeightForWidth(linha_grupo_ataque->sizePolicy().hasHeightForWidth());
        linha_grupo_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(linha_grupo_ataque, 0, 1, 1, 1);

        label_36 = new QLabel(tab_6);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        sizePolicy8.setHeightForWidth(label_36->sizePolicy().hasHeightForWidth());
        label_36->setSizePolicy(sizePolicy8);
        QFont font3;
        font3.setKerning(false);
        label_36->setFont(font3);
        label_36->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_36, 0, 2, 1, 1);

        linha_rotulo_ataque = new QLineEdit(tab_6);
        linha_rotulo_ataque->setObjectName(QString::fromUtf8("linha_rotulo_ataque"));
        sizePolicy2.setHeightForWidth(linha_rotulo_ataque->sizePolicy().hasHeightForWidth());
        linha_rotulo_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(linha_rotulo_ataque, 0, 3, 1, 1);

        label_83 = new QLabel(tab_6);
        label_83->setObjectName(QString::fromUtf8("label_83"));
        sizePolicy8.setHeightForWidth(label_83->sizePolicy().hasHeightForWidth());
        label_83->setSizePolicy(sizePolicy8);
        label_83->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_83, 0, 5, 1, 1);

        spin_ordem_ataque = new QSpinBox(tab_6);
        spin_ordem_ataque->setObjectName(QString::fromUtf8("spin_ordem_ataque"));
        sizePolicy2.setHeightForWidth(spin_ordem_ataque->sizePolicy().hasHeightForWidth());
        spin_ordem_ataque->setSizePolicy(sizePolicy2);
        spin_ordem_ataque->setMinimum(1);
        spin_ordem_ataque->setMaximum(9);

        gridLayout_13->addWidget(spin_ordem_ataque, 0, 6, 1, 1);

        label_73 = new QLabel(tab_6);
        label_73->setObjectName(QString::fromUtf8("label_73"));
        sizePolicy8.setHeightForWidth(label_73->sizePolicy().hasHeightForWidth());
        label_73->setSizePolicy(sizePolicy8);
        label_73->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_73, 0, 7, 1, 1);

        combo_empunhadura = new QComboBox(tab_6);
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->addItem(QString());
        combo_empunhadura->setObjectName(QString::fromUtf8("combo_empunhadura"));
        sizePolicy3.setHeightForWidth(combo_empunhadura->sizePolicy().hasHeightForWidth());
        combo_empunhadura->setSizePolicy(sizePolicy3);

        gridLayout_13->addWidget(combo_empunhadura, 0, 8, 1, 4);

        label_21 = new QLabel(tab_6);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        sizePolicy8.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy8);
        label_21->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_21, 1, 0, 1, 1);

        combo_tipo_ataque = new QComboBox(tab_6);
        combo_tipo_ataque->setObjectName(QString::fromUtf8("combo_tipo_ataque"));
        sizePolicy3.setHeightForWidth(combo_tipo_ataque->sizePolicy().hasHeightForWidth());
        combo_tipo_ataque->setSizePolicy(sizePolicy3);

        gridLayout_13->addWidget(combo_tipo_ataque, 1, 1, 1, 1);

        label_93 = new QLabel(tab_6);
        label_93->setObjectName(QString::fromUtf8("label_93"));
        sizePolicy8.setHeightForWidth(label_93->sizePolicy().hasHeightForWidth());
        label_93->setSizePolicy(sizePolicy8);
        label_93->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_93, 1, 2, 1, 1);

        combo_arma = new QComboBox(tab_6);
        combo_arma->setObjectName(QString::fromUtf8("combo_arma"));
        sizePolicy3.setHeightForWidth(combo_arma->sizePolicy().hasHeightForWidth());
        combo_arma->setSizePolicy(sizePolicy3);

        gridLayout_13->addWidget(combo_arma, 1, 3, 1, 1);

        checkbox_op = new QCheckBox(tab_6);
        checkbox_op->setObjectName(QString::fromUtf8("checkbox_op"));
        sizePolicy2.setHeightForWidth(checkbox_op->sizePolicy().hasHeightForWidth());
        checkbox_op->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(checkbox_op, 1, 4, 1, 1);

        label_23 = new QLabel(tab_6);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        sizePolicy8.setHeightForWidth(label_23->sizePolicy().hasHeightForWidth());
        label_23->setSizePolicy(sizePolicy8);
        label_23->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_23, 1, 5, 1, 1);

        spin_bonus_magico = new QSpinBox(tab_6);
        spin_bonus_magico->setObjectName(QString::fromUtf8("spin_bonus_magico"));
        sizePolicy2.setHeightForWidth(spin_bonus_magico->sizePolicy().hasHeightForWidth());
        spin_bonus_magico->setSizePolicy(sizePolicy2);
        spin_bonus_magico->setMinimum(-50);
        spin_bonus_magico->setMaximum(50);

        gridLayout_13->addWidget(spin_bonus_magico, 1, 6, 1, 1);

        label_92 = new QLabel(tab_6);
        label_92->setObjectName(QString::fromUtf8("label_92"));
        sizePolicy8.setHeightForWidth(label_92->sizePolicy().hasHeightForWidth());
        label_92->setSizePolicy(sizePolicy8);
        label_92->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_92, 1, 7, 1, 1);

        combo_material_arma = new QComboBox(tab_6);
        combo_material_arma->addItem(QString());
        combo_material_arma->addItem(QString());
        combo_material_arma->addItem(QString());
        combo_material_arma->addItem(QString());
        combo_material_arma->addItem(QString());
        combo_material_arma->addItem(QString());
        combo_material_arma->setObjectName(QString::fromUtf8("combo_material_arma"));
        sizePolicy2.setHeightForWidth(combo_material_arma->sizePolicy().hasHeightForWidth());
        combo_material_arma->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(combo_material_arma, 1, 8, 1, 2);

        label_55 = new QLabel(tab_6);
        label_55->setObjectName(QString::fromUtf8("label_55"));
        sizePolicy2.setHeightForWidth(label_55->sizePolicy().hasHeightForWidth());
        label_55->setSizePolicy(sizePolicy2);
        label_55->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_55, 1, 12, 1, 2);

        botao_bonus_ataque = new QPushButton(tab_6);
        botao_bonus_ataque->setObjectName(QString::fromUtf8("botao_bonus_ataque"));
        sizePolicy2.setHeightForWidth(botao_bonus_ataque->sizePolicy().hasHeightForWidth());
        botao_bonus_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(botao_bonus_ataque, 1, 14, 1, 1);

        label_24 = new QLabel(tab_6);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        sizePolicy8.setHeightForWidth(label_24->sizePolicy().hasHeightForWidth());
        label_24->setSizePolicy(sizePolicy8);
        label_24->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_24, 2, 0, 1, 1);

        linha_dano = new QLineEdit(tab_6);
        linha_dano->setObjectName(QString::fromUtf8("linha_dano"));
        sizePolicy2.setHeightForWidth(linha_dano->sizePolicy().hasHeightForWidth());
        linha_dano->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(linha_dano, 2, 1, 1, 1);

        label_82 = new QLabel(tab_6);
        label_82->setObjectName(QString::fromUtf8("label_82"));
        sizePolicy8.setHeightForWidth(label_82->sizePolicy().hasHeightForWidth());
        label_82->setSizePolicy(sizePolicy8);
        label_82->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_82, 2, 2, 1, 1);

        spin_municao = new QSpinBox(tab_6);
        spin_municao->setObjectName(QString::fromUtf8("spin_municao"));
        sizePolicy2.setHeightForWidth(spin_municao->sizePolicy().hasHeightForWidth());
        spin_municao->setSizePolicy(sizePolicy2);
        spin_municao->setMaximum(10000);

        gridLayout_13->addWidget(spin_municao, 2, 3, 1, 1);

        label_29 = new QLabel(tab_6);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        sizePolicy2.setHeightForWidth(label_29->sizePolicy().hasHeightForWidth());
        label_29->setSizePolicy(sizePolicy2);
        label_29->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_29, 2, 5, 1, 1);

        spin_alcance_quad = new QSpinBox(tab_6);
        spin_alcance_quad->setObjectName(QString::fromUtf8("spin_alcance_quad"));
        sizePolicy2.setHeightForWidth(spin_alcance_quad->sizePolicy().hasHeightForWidth());
        spin_alcance_quad->setSizePolicy(sizePolicy2);
        spin_alcance_quad->setMinimum(-1);

        gridLayout_13->addWidget(spin_alcance_quad, 2, 6, 1, 1);

        label_30 = new QLabel(tab_6);
        label_30->setObjectName(QString::fromUtf8("label_30"));
        sizePolicy8.setHeightForWidth(label_30->sizePolicy().hasHeightForWidth());
        label_30->setSizePolicy(sizePolicy8);
        label_30->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_30, 2, 7, 1, 1);

        spin_incrementos = new QSpinBox(tab_6);
        spin_incrementos->setObjectName(QString::fromUtf8("spin_incrementos"));
        sizePolicy2.setHeightForWidth(spin_incrementos->sizePolicy().hasHeightForWidth());
        spin_incrementos->setSizePolicy(sizePolicy2);
        spin_incrementos->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_13->addWidget(spin_incrementos, 2, 8, 1, 1);

        label_72 = new QLabel(tab_6);
        label_72->setObjectName(QString::fromUtf8("label_72"));
        sizePolicy8.setHeightForWidth(label_72->sizePolicy().hasHeightForWidth());
        label_72->setSizePolicy(sizePolicy8);
        label_72->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_72, 2, 12, 1, 2);

        botao_bonus_dano = new QPushButton(tab_6);
        botao_bonus_dano->setObjectName(QString::fromUtf8("botao_bonus_dano"));

        gridLayout_13->addWidget(botao_bonus_dano, 2, 14, 1, 1);

        label_104 = new QLabel(tab_6);
        label_104->setObjectName(QString::fromUtf8("label_104"));
        sizePolicy8.setHeightForWidth(label_104->sizePolicy().hasHeightForWidth());
        label_104->setSizePolicy(sizePolicy8);
        label_104->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_104, 3, 0, 1, 1);

        spin_nivel_conjurador_pergaminho = new QSpinBox(tab_6);
        spin_nivel_conjurador_pergaminho->setObjectName(QString::fromUtf8("spin_nivel_conjurador_pergaminho"));

        gridLayout_13->addWidget(spin_nivel_conjurador_pergaminho, 3, 1, 1, 1);

        label_mod_atributo = new QLabel(tab_6);
        label_mod_atributo->setObjectName(QString::fromUtf8("label_mod_atributo"));
        label_mod_atributo->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_13->addWidget(label_mod_atributo, 3, 2, 1, 1);

        spin_modificador_atributo_pergaminho = new QSpinBox(tab_6);
        spin_modificador_atributo_pergaminho->setObjectName(QString::fromUtf8("spin_modificador_atributo_pergaminho"));

        gridLayout_13->addWidget(spin_modificador_atributo_pergaminho, 3, 3, 1, 1);

        label_9 = new QLabel(tab_6);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_13->addWidget(label_9, 3, 5, 1, 1);

        spin_limite_vezes = new QSpinBox(tab_6);
        spin_limite_vezes->setObjectName(QString::fromUtf8("spin_limite_vezes"));

        gridLayout_13->addWidget(spin_limite_vezes, 3, 6, 1, 1);

        checkbox_ignora_rm = new QCheckBox(tab_6);
        checkbox_ignora_rm->setObjectName(QString::fromUtf8("checkbox_ignora_rm"));
        sizePolicy2.setHeightForWidth(checkbox_ignora_rm->sizePolicy().hasHeightForWidth());
        checkbox_ignora_rm->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(checkbox_ignora_rm, 3, 7, 1, 1);

        checkbox_permite_salvacao = new QCheckBox(tab_6);
        checkbox_permite_salvacao->setObjectName(QString::fromUtf8("checkbox_permite_salvacao"));
        sizePolicy2.setHeightForWidth(checkbox_permite_salvacao->sizePolicy().hasHeightForWidth());
        checkbox_permite_salvacao->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(checkbox_permite_salvacao, 3, 9, 1, 1);

        checkbox_ataque_agarrar = new QCheckBox(tab_6);
        checkbox_ataque_agarrar->setObjectName(QString::fromUtf8("checkbox_ataque_agarrar"));

        gridLayout_13->addWidget(checkbox_ataque_agarrar, 3, 10, 1, 2);

        checkbox_ataque_toque = new QCheckBox(tab_6);
        checkbox_ataque_toque->setObjectName(QString::fromUtf8("checkbox_ataque_toque"));

        gridLayout_13->addWidget(checkbox_ataque_toque, 3, 12, 1, 2);

        lista_ataques = new QListWidget(tab_6);
        lista_ataques->setObjectName(QString::fromUtf8("lista_ataques"));
        QSizePolicy sizePolicy9(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy9.setHorizontalStretch(0);
        sizePolicy9.setVerticalStretch(0);
        sizePolicy9.setHeightForWidth(lista_ataques->sizePolicy().hasHeightForWidth());
        lista_ataques->setSizePolicy(sizePolicy9);
        lista_ataques->setSelectionMode(QAbstractItemView::ExtendedSelection);
        lista_ataques->setSelectionBehavior(QAbstractItemView::SelectItems);

        gridLayout_13->addWidget(lista_ataques, 4, 0, 1, 14);

        verticalLayout_16 = new QVBoxLayout();
        verticalLayout_16->setObjectName(QString::fromUtf8("verticalLayout_16"));
        botao_ataque_cima = new QPushButton(tab_6);
        botao_ataque_cima->setObjectName(QString::fromUtf8("botao_ataque_cima"));
        sizePolicy2.setHeightForWidth(botao_ataque_cima->sizePolicy().hasHeightForWidth());
        botao_ataque_cima->setSizePolicy(sizePolicy2);

        verticalLayout_16->addWidget(botao_ataque_cima);

        botao_ataque_baixo = new QPushButton(tab_6);
        botao_ataque_baixo->setObjectName(QString::fromUtf8("botao_ataque_baixo"));
        sizePolicy2.setHeightForWidth(botao_ataque_baixo->sizePolicy().hasHeightForWidth());
        botao_ataque_baixo->setSizePolicy(sizePolicy2);

        verticalLayout_16->addWidget(botao_ataque_baixo);


        gridLayout_13->addLayout(verticalLayout_16, 4, 14, 1, 1);

        botao_remover_ataque = new QPushButton(tab_6);
        botao_remover_ataque->setObjectName(QString::fromUtf8("botao_remover_ataque"));
        botao_remover_ataque->setEnabled(false);
        sizePolicy2.setHeightForWidth(botao_remover_ataque->sizePolicy().hasHeightForWidth());
        botao_remover_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(botao_remover_ataque, 5, 10, 1, 1);

        botao_adicionar_ataque = new QPushButton(tab_6);
        botao_adicionar_ataque->setObjectName(QString::fromUtf8("botao_adicionar_ataque"));
        sizePolicy2.setHeightForWidth(botao_adicionar_ataque->sizePolicy().hasHeightForWidth());
        botao_adicionar_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(botao_adicionar_ataque, 5, 11, 1, 2);

        botao_clonar_ataque = new QPushButton(tab_6);
        botao_clonar_ataque->setObjectName(QString::fromUtf8("botao_clonar_ataque"));
        botao_clonar_ataque->setEnabled(true);
        sizePolicy2.setHeightForWidth(botao_clonar_ataque->sizePolicy().hasHeightForWidth());
        botao_clonar_ataque->setSizePolicy(sizePolicy2);

        gridLayout_13->addWidget(botao_clonar_ataque, 5, 13, 1, 1);

        tabs->addTab(tab_6, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        gridLayout_7 = new QGridLayout(tab);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        groupBox_22 = new QGroupBox(tab);
        groupBox_22->setObjectName(QString::fromUtf8("groupBox_22"));
        horizontalLayout_67 = new QHBoxLayout(groupBox_22);
        horizontalLayout_67->setObjectName(QString::fromUtf8("horizontalLayout_67"));
        lista_bracadeiras = new QListWidget(groupBox_22);
        lista_bracadeiras->setObjectName(QString::fromUtf8("lista_bracadeiras"));

        horizontalLayout_67->addWidget(lista_bracadeiras);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        botao_usar_bracadeiras = new QPushButton(groupBox_22);
        botao_usar_bracadeiras->setObjectName(QString::fromUtf8("botao_usar_bracadeiras"));

        verticalLayout_7->addWidget(botao_usar_bracadeiras);

        botao_adicionar_bracadeiras = new QPushButton(groupBox_22);
        botao_adicionar_bracadeiras->setObjectName(QString::fromUtf8("botao_adicionar_bracadeiras"));

        verticalLayout_7->addWidget(botao_adicionar_bracadeiras);

        botao_remover_bracadeiras = new QPushButton(groupBox_22);
        botao_remover_bracadeiras->setObjectName(QString::fromUtf8("botao_remover_bracadeiras"));

        verticalLayout_7->addWidget(botao_remover_bracadeiras);

        botao_doar_bracadeiras = new QPushButton(groupBox_22);
        botao_doar_bracadeiras->setObjectName(QString::fromUtf8("botao_doar_bracadeiras"));

        verticalLayout_7->addWidget(botao_doar_bracadeiras);


        horizontalLayout_67->addLayout(verticalLayout_7);


        gridLayout_7->addWidget(groupBox_22, 1, 2, 1, 1);

        groupBox_24 = new QGroupBox(tab);
        groupBox_24->setObjectName(QString::fromUtf8("groupBox_24"));
        horizontalLayout_73 = new QHBoxLayout(groupBox_24);
        horizontalLayout_73->setObjectName(QString::fromUtf8("horizontalLayout_73"));
        lista_armas = new QListWidget(groupBox_24);
        lista_armas->setObjectName(QString::fromUtf8("lista_armas"));

        horizontalLayout_73->addWidget(lista_armas);

        verticalLayout_29 = new QVBoxLayout();
        verticalLayout_29->setObjectName(QString::fromUtf8("verticalLayout_29"));
        botao_adicionar_arma = new QPushButton(groupBox_24);
        botao_adicionar_arma->setObjectName(QString::fromUtf8("botao_adicionar_arma"));

        verticalLayout_29->addWidget(botao_adicionar_arma);

        botao_duplicar_arma = new QPushButton(groupBox_24);
        botao_duplicar_arma->setObjectName(QString::fromUtf8("botao_duplicar_arma"));

        verticalLayout_29->addWidget(botao_duplicar_arma);

        botao_remover_arma = new QPushButton(groupBox_24);
        botao_remover_arma->setObjectName(QString::fromUtf8("botao_remover_arma"));

        verticalLayout_29->addWidget(botao_remover_arma);

        botao_ordenar_armas = new QPushButton(groupBox_24);
        botao_ordenar_armas->setObjectName(QString::fromUtf8("botao_ordenar_armas"));

        verticalLayout_29->addWidget(botao_ordenar_armas);

        botao_doar_arma = new QPushButton(groupBox_24);
        botao_doar_arma->setObjectName(QString::fromUtf8("botao_doar_arma"));

        verticalLayout_29->addWidget(botao_doar_arma);


        horizontalLayout_73->addLayout(verticalLayout_29);


        gridLayout_7->addWidget(groupBox_24, 3, 0, 1, 1);

        groupBox_13 = new QGroupBox(tab);
        groupBox_13->setObjectName(QString::fromUtf8("groupBox_13"));
        horizontalLayout_58 = new QHBoxLayout(groupBox_13);
        horizontalLayout_58->setObjectName(QString::fromUtf8("horizontalLayout_58"));
        lista_amuletos = new QListWidget(groupBox_13);
        lista_amuletos->setObjectName(QString::fromUtf8("lista_amuletos"));

        horizontalLayout_58->addWidget(lista_amuletos);

        verticalLayout_19 = new QVBoxLayout();
        verticalLayout_19->setObjectName(QString::fromUtf8("verticalLayout_19"));
        botao_usar_amuleto = new QPushButton(groupBox_13);
        botao_usar_amuleto->setObjectName(QString::fromUtf8("botao_usar_amuleto"));

        verticalLayout_19->addWidget(botao_usar_amuleto);

        botao_adicionar_amuleto = new QPushButton(groupBox_13);
        botao_adicionar_amuleto->setObjectName(QString::fromUtf8("botao_adicionar_amuleto"));

        verticalLayout_19->addWidget(botao_adicionar_amuleto);

        botao_remover_amuleto = new QPushButton(groupBox_13);
        botao_remover_amuleto->setObjectName(QString::fromUtf8("botao_remover_amuleto"));

        verticalLayout_19->addWidget(botao_remover_amuleto);

        botao_doar_amuleto = new QPushButton(groupBox_13);
        botao_doar_amuleto->setObjectName(QString::fromUtf8("botao_doar_amuleto"));

        verticalLayout_19->addWidget(botao_doar_amuleto);


        horizontalLayout_58->addLayout(verticalLayout_19);


        gridLayout_7->addWidget(groupBox_13, 0, 2, 1, 1);

        groupBox_23 = new QGroupBox(tab);
        groupBox_23->setObjectName(QString::fromUtf8("groupBox_23"));
        horizontalLayout_68 = new QHBoxLayout(groupBox_23);
        horizontalLayout_68->setObjectName(QString::fromUtf8("horizontalLayout_68"));
        lista_itens_mundanos = new QListWidget(groupBox_23);
        lista_itens_mundanos->setObjectName(QString::fromUtf8("lista_itens_mundanos"));

        horizontalLayout_68->addWidget(lista_itens_mundanos);

        verticalLayout_25 = new QVBoxLayout();
        verticalLayout_25->setObjectName(QString::fromUtf8("verticalLayout_25"));
        botao_adicionar_item_mundano = new QPushButton(groupBox_23);
        botao_adicionar_item_mundano->setObjectName(QString::fromUtf8("botao_adicionar_item_mundano"));

        verticalLayout_25->addWidget(botao_adicionar_item_mundano);

        botao_duplicar_item_mundano = new QPushButton(groupBox_23);
        botao_duplicar_item_mundano->setObjectName(QString::fromUtf8("botao_duplicar_item_mundano"));

        verticalLayout_25->addWidget(botao_duplicar_item_mundano);

        botao_remover_item_mundano = new QPushButton(groupBox_23);
        botao_remover_item_mundano->setObjectName(QString::fromUtf8("botao_remover_item_mundano"));

        verticalLayout_25->addWidget(botao_remover_item_mundano);

        botao_ordenar_item_mundano = new QPushButton(groupBox_23);
        botao_ordenar_item_mundano->setObjectName(QString::fromUtf8("botao_ordenar_item_mundano"));

        verticalLayout_25->addWidget(botao_ordenar_item_mundano);

        botao_doar_item_mundano = new QPushButton(groupBox_23);
        botao_doar_item_mundano->setObjectName(QString::fromUtf8("botao_doar_item_mundano"));

        verticalLayout_25->addWidget(botao_doar_item_mundano);


        horizontalLayout_68->addLayout(verticalLayout_25);


        gridLayout_7->addWidget(groupBox_23, 2, 1, 1, 1);

        groupBox_19 = new QGroupBox(tab);
        groupBox_19->setObjectName(QString::fromUtf8("groupBox_19"));
        horizontalLayout_64 = new QHBoxLayout(groupBox_19);
        horizontalLayout_64->setObjectName(QString::fromUtf8("horizontalLayout_64"));
        lista_chapeus = new QListWidget(groupBox_19);
        lista_chapeus->setObjectName(QString::fromUtf8("lista_chapeus"));

        horizontalLayout_64->addWidget(lista_chapeus);

        verticalLayout_20 = new QVBoxLayout();
        verticalLayout_20->setObjectName(QString::fromUtf8("verticalLayout_20"));
        botao_vestir_chapeu = new QPushButton(groupBox_19);
        botao_vestir_chapeu->setObjectName(QString::fromUtf8("botao_vestir_chapeu"));

        verticalLayout_20->addWidget(botao_vestir_chapeu);

        botao_adicionar_chapeu = new QPushButton(groupBox_19);
        botao_adicionar_chapeu->setObjectName(QString::fromUtf8("botao_adicionar_chapeu"));

        verticalLayout_20->addWidget(botao_adicionar_chapeu);

        botao_remover_chapeu = new QPushButton(groupBox_19);
        botao_remover_chapeu->setObjectName(QString::fromUtf8("botao_remover_chapeu"));

        verticalLayout_20->addWidget(botao_remover_chapeu);

        botao_doar_chapeu = new QPushButton(groupBox_19);
        botao_doar_chapeu->setObjectName(QString::fromUtf8("botao_doar_chapeu"));

        verticalLayout_20->addWidget(botao_doar_chapeu);


        horizontalLayout_64->addLayout(verticalLayout_20);


        gridLayout_7->addWidget(groupBox_19, 0, 3, 1, 2);

        groupBox_26 = new QGroupBox(tab);
        groupBox_26->setObjectName(QString::fromUtf8("groupBox_26"));
        horizontalLayout_75 = new QHBoxLayout(groupBox_26);
        horizontalLayout_75->setObjectName(QString::fromUtf8("horizontalLayout_75"));
        lista_escudos = new QListWidget(groupBox_26);
        lista_escudos->setObjectName(QString::fromUtf8("lista_escudos"));

        horizontalLayout_75->addWidget(lista_escudos);

        verticalLayout_31 = new QVBoxLayout();
        verticalLayout_31->setObjectName(QString::fromUtf8("verticalLayout_31"));
        botao_adicionar_escudo = new QPushButton(groupBox_26);
        botao_adicionar_escudo->setObjectName(QString::fromUtf8("botao_adicionar_escudo"));

        verticalLayout_31->addWidget(botao_adicionar_escudo);

        botao_duplicar_escudo = new QPushButton(groupBox_26);
        botao_duplicar_escudo->setObjectName(QString::fromUtf8("botao_duplicar_escudo"));

        verticalLayout_31->addWidget(botao_duplicar_escudo);

        botao_remover_escudo = new QPushButton(groupBox_26);
        botao_remover_escudo->setObjectName(QString::fromUtf8("botao_remover_escudo"));

        verticalLayout_31->addWidget(botao_remover_escudo);

        botao_ordenar_escudos = new QPushButton(groupBox_26);
        botao_ordenar_escudos->setObjectName(QString::fromUtf8("botao_ordenar_escudos"));

        verticalLayout_31->addWidget(botao_ordenar_escudos);

        botao_doar_escudo = new QPushButton(groupBox_26);
        botao_doar_escudo->setObjectName(QString::fromUtf8("botao_doar_escudo"));

        verticalLayout_31->addWidget(botao_doar_escudo);


        horizontalLayout_75->addLayout(verticalLayout_31);


        gridLayout_7->addWidget(groupBox_26, 3, 2, 1, 1);

        groupBox_20 = new QGroupBox(tab);
        groupBox_20->setObjectName(QString::fromUtf8("groupBox_20"));
        horizontalLayout_65 = new QHBoxLayout(groupBox_20);
        horizontalLayout_65->setObjectName(QString::fromUtf8("horizontalLayout_65"));
        lista_mantos = new QListWidget(groupBox_20);
        lista_mantos->setObjectName(QString::fromUtf8("lista_mantos"));

        horizontalLayout_65->addWidget(lista_mantos);

        verticalLayout_9 = new QVBoxLayout();
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        botao_usar_manto = new QPushButton(groupBox_20);
        botao_usar_manto->setObjectName(QString::fromUtf8("botao_usar_manto"));
        sizePolicy3.setHeightForWidth(botao_usar_manto->sizePolicy().hasHeightForWidth());
        botao_usar_manto->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(botao_usar_manto);

        botao_adicionar_manto = new QPushButton(groupBox_20);
        botao_adicionar_manto->setObjectName(QString::fromUtf8("botao_adicionar_manto"));
        sizePolicy3.setHeightForWidth(botao_adicionar_manto->sizePolicy().hasHeightForWidth());
        botao_adicionar_manto->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(botao_adicionar_manto);

        botao_remover_manto = new QPushButton(groupBox_20);
        botao_remover_manto->setObjectName(QString::fromUtf8("botao_remover_manto"));
        sizePolicy3.setHeightForWidth(botao_remover_manto->sizePolicy().hasHeightForWidth());
        botao_remover_manto->setSizePolicy(sizePolicy3);

        verticalLayout_9->addWidget(botao_remover_manto);

        botao_doar_manto = new QPushButton(groupBox_20);
        botao_doar_manto->setObjectName(QString::fromUtf8("botao_doar_manto"));

        verticalLayout_9->addWidget(botao_doar_manto);


        horizontalLayout_65->addLayout(verticalLayout_9);


        gridLayout_7->addWidget(groupBox_20, 1, 3, 1, 2);

        groupBox_16 = new QGroupBox(tab);
        groupBox_16->setObjectName(QString::fromUtf8("groupBox_16"));
        horizontalLayout_61 = new QHBoxLayout(groupBox_16);
        horizontalLayout_61->setObjectName(QString::fromUtf8("horizontalLayout_61"));
        lista_pergaminhos_divinos = new QListWidget(groupBox_16);
        lista_pergaminhos_divinos->setObjectName(QString::fromUtf8("lista_pergaminhos_divinos"));

        horizontalLayout_61->addWidget(lista_pergaminhos_divinos);

        verticalLayout_24 = new QVBoxLayout();
        verticalLayout_24->setObjectName(QString::fromUtf8("verticalLayout_24"));
        botao_adicionar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_adicionar_pergaminho_divino->setObjectName(QString::fromUtf8("botao_adicionar_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_adicionar_pergaminho_divino);

        botao_duplicar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_duplicar_pergaminho_divino->setObjectName(QString::fromUtf8("botao_duplicar_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_duplicar_pergaminho_divino);

        botao_remover_pergaminho_divino = new QPushButton(groupBox_16);
        botao_remover_pergaminho_divino->setObjectName(QString::fromUtf8("botao_remover_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_remover_pergaminho_divino);

        botao_ordenar_pergaminhos_divinos = new QPushButton(groupBox_16);
        botao_ordenar_pergaminhos_divinos->setObjectName(QString::fromUtf8("botao_ordenar_pergaminhos_divinos"));

        verticalLayout_24->addWidget(botao_ordenar_pergaminhos_divinos);

        botao_doar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_doar_pergaminho_divino->setObjectName(QString::fromUtf8("botao_doar_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_doar_pergaminho_divino);


        horizontalLayout_61->addLayout(verticalLayout_24);


        gridLayout_7->addWidget(groupBox_16, 2, 0, 1, 1);

        groupBox_25 = new QGroupBox(tab);
        groupBox_25->setObjectName(QString::fromUtf8("groupBox_25"));
        horizontalLayout_74 = new QHBoxLayout(groupBox_25);
        horizontalLayout_74->setObjectName(QString::fromUtf8("horizontalLayout_74"));
        lista_armaduras = new QListWidget(groupBox_25);
        lista_armaduras->setObjectName(QString::fromUtf8("lista_armaduras"));

        horizontalLayout_74->addWidget(lista_armaduras);

        verticalLayout_30 = new QVBoxLayout();
        verticalLayout_30->setObjectName(QString::fromUtf8("verticalLayout_30"));
        botao_adicionar_armadura = new QPushButton(groupBox_25);
        botao_adicionar_armadura->setObjectName(QString::fromUtf8("botao_adicionar_armadura"));

        verticalLayout_30->addWidget(botao_adicionar_armadura);

        botao_duplicar_armadura = new QPushButton(groupBox_25);
        botao_duplicar_armadura->setObjectName(QString::fromUtf8("botao_duplicar_armadura"));

        verticalLayout_30->addWidget(botao_duplicar_armadura);

        botao_remover_armadura = new QPushButton(groupBox_25);
        botao_remover_armadura->setObjectName(QString::fromUtf8("botao_remover_armadura"));

        verticalLayout_30->addWidget(botao_remover_armadura);

        botao_ordenar_armaduras = new QPushButton(groupBox_25);
        botao_ordenar_armaduras->setObjectName(QString::fromUtf8("botao_ordenar_armaduras"));

        verticalLayout_30->addWidget(botao_ordenar_armaduras);

        botao_doar_armadura = new QPushButton(groupBox_25);
        botao_doar_armadura->setObjectName(QString::fromUtf8("botao_doar_armadura"));

        verticalLayout_30->addWidget(botao_doar_armadura);


        horizontalLayout_74->addLayout(verticalLayout_30);


        gridLayout_7->addWidget(groupBox_25, 3, 1, 1, 1);

        groupBox_14 = new QGroupBox(tab);
        groupBox_14->setObjectName(QString::fromUtf8("groupBox_14"));
        horizontalLayout_59 = new QHBoxLayout(groupBox_14);
        horizontalLayout_59->setObjectName(QString::fromUtf8("horizontalLayout_59"));
        lista_pocoes = new QListWidget(groupBox_14);
        lista_pocoes->setObjectName(QString::fromUtf8("lista_pocoes"));

        horizontalLayout_59->addWidget(lista_pocoes);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        botao_adicionar_pocao = new QPushButton(groupBox_14);
        botao_adicionar_pocao->setObjectName(QString::fromUtf8("botao_adicionar_pocao"));
        sizePolicy3.setHeightForWidth(botao_adicionar_pocao->sizePolicy().hasHeightForWidth());
        botao_adicionar_pocao->setSizePolicy(sizePolicy3);

        verticalLayout_6->addWidget(botao_adicionar_pocao);

        botao_duplicar_pocao = new QPushButton(groupBox_14);
        botao_duplicar_pocao->setObjectName(QString::fromUtf8("botao_duplicar_pocao"));

        verticalLayout_6->addWidget(botao_duplicar_pocao);

        botao_remover_pocao = new QPushButton(groupBox_14);
        botao_remover_pocao->setObjectName(QString::fromUtf8("botao_remover_pocao"));
        sizePolicy3.setHeightForWidth(botao_remover_pocao->sizePolicy().hasHeightForWidth());
        botao_remover_pocao->setSizePolicy(sizePolicy3);

        verticalLayout_6->addWidget(botao_remover_pocao);

        botao_ordenar_pocoes = new QPushButton(groupBox_14);
        botao_ordenar_pocoes->setObjectName(QString::fromUtf8("botao_ordenar_pocoes"));

        verticalLayout_6->addWidget(botao_ordenar_pocoes);

        botao_doar_pocao = new QPushButton(groupBox_14);
        botao_doar_pocao->setObjectName(QString::fromUtf8("botao_doar_pocao"));

        verticalLayout_6->addWidget(botao_doar_pocao);


        horizontalLayout_59->addLayout(verticalLayout_6);


        gridLayout_7->addWidget(groupBox_14, 0, 0, 1, 1);

        groupBox_12 = new QGroupBox(tab);
        groupBox_12->setObjectName(QString::fromUtf8("groupBox_12"));
        horizontalLayout_44 = new QHBoxLayout(groupBox_12);
        horizontalLayout_44->setObjectName(QString::fromUtf8("horizontalLayout_44"));
        lista_botas = new QListWidget(groupBox_12);
        lista_botas->setObjectName(QString::fromUtf8("lista_botas"));

        horizontalLayout_44->addWidget(lista_botas);

        verticalLayout_22 = new QVBoxLayout();
        verticalLayout_22->setObjectName(QString::fromUtf8("verticalLayout_22"));
        botao_usar_botas = new QPushButton(groupBox_12);
        botao_usar_botas->setObjectName(QString::fromUtf8("botao_usar_botas"));

        verticalLayout_22->addWidget(botao_usar_botas);

        botao_adicionar_botas = new QPushButton(groupBox_12);
        botao_adicionar_botas->setObjectName(QString::fromUtf8("botao_adicionar_botas"));

        verticalLayout_22->addWidget(botao_adicionar_botas);

        botao_remover_botas = new QPushButton(groupBox_12);
        botao_remover_botas->setObjectName(QString::fromUtf8("botao_remover_botas"));

        verticalLayout_22->addWidget(botao_remover_botas);

        botao_doar_botas = new QPushButton(groupBox_12);
        botao_doar_botas->setObjectName(QString::fromUtf8("botao_doar_botas"));

        verticalLayout_22->addWidget(botao_doar_botas);


        horizontalLayout_44->addLayout(verticalLayout_22);


        gridLayout_7->addWidget(groupBox_12, 0, 1, 1, 1);

        groupBox_15 = new QGroupBox(tab);
        groupBox_15->setObjectName(QString::fromUtf8("groupBox_15"));
        horizontalLayout_60 = new QHBoxLayout(groupBox_15);
        horizontalLayout_60->setObjectName(QString::fromUtf8("horizontalLayout_60"));
        lista_pergaminhos_arcanos = new QListWidget(groupBox_15);
        lista_pergaminhos_arcanos->setObjectName(QString::fromUtf8("lista_pergaminhos_arcanos"));

        horizontalLayout_60->addWidget(lista_pergaminhos_arcanos);

        verticalLayout_23 = new QVBoxLayout();
        verticalLayout_23->setObjectName(QString::fromUtf8("verticalLayout_23"));
        botao_adicionar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_adicionar_pergaminho_arcano->setObjectName(QString::fromUtf8("botao_adicionar_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_adicionar_pergaminho_arcano);

        botao_duplicar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_duplicar_pergaminho_arcano->setObjectName(QString::fromUtf8("botao_duplicar_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_duplicar_pergaminho_arcano);

        botao_remover_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_remover_pergaminho_arcano->setObjectName(QString::fromUtf8("botao_remover_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_remover_pergaminho_arcano);

        botao_ordenar_pergaminhos_arcanos = new QPushButton(groupBox_15);
        botao_ordenar_pergaminhos_arcanos->setObjectName(QString::fromUtf8("botao_ordenar_pergaminhos_arcanos"));

        verticalLayout_23->addWidget(botao_ordenar_pergaminhos_arcanos);

        botao_doar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_doar_pergaminho_arcano->setObjectName(QString::fromUtf8("botao_doar_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_doar_pergaminho_arcano);


        horizontalLayout_60->addLayout(verticalLayout_23);


        gridLayout_7->addWidget(groupBox_15, 1, 0, 1, 1);

        groupBox_17 = new QGroupBox(tab);
        groupBox_17->setObjectName(QString::fromUtf8("groupBox_17"));
        horizontalLayout_62 = new QHBoxLayout(groupBox_17);
        horizontalLayout_62->setObjectName(QString::fromUtf8("horizontalLayout_62"));
        lista_aneis = new QListWidget(groupBox_17);
        lista_aneis->setObjectName(QString::fromUtf8("lista_aneis"));

        horizontalLayout_62->addWidget(lista_aneis);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        botao_usar_anel = new QPushButton(groupBox_17);
        botao_usar_anel->setObjectName(QString::fromUtf8("botao_usar_anel"));

        verticalLayout_10->addWidget(botao_usar_anel);

        botao_adicionar_anel = new QPushButton(groupBox_17);
        botao_adicionar_anel->setObjectName(QString::fromUtf8("botao_adicionar_anel"));

        verticalLayout_10->addWidget(botao_adicionar_anel);

        botao_remover_anel = new QPushButton(groupBox_17);
        botao_remover_anel->setObjectName(QString::fromUtf8("botao_remover_anel"));

        verticalLayout_10->addWidget(botao_remover_anel);

        botao_doar_anel = new QPushButton(groupBox_17);
        botao_doar_anel->setObjectName(QString::fromUtf8("botao_doar_anel"));

        verticalLayout_10->addWidget(botao_doar_anel);


        horizontalLayout_62->addLayout(verticalLayout_10);


        gridLayout_7->addWidget(groupBox_17, 2, 2, 1, 1);

        groupBox_18 = new QGroupBox(tab);
        groupBox_18->setObjectName(QString::fromUtf8("groupBox_18"));
        horizontalLayout_63 = new QHBoxLayout(groupBox_18);
        horizontalLayout_63->setObjectName(QString::fromUtf8("horizontalLayout_63"));
        lista_luvas = new QListWidget(groupBox_18);
        lista_luvas->setObjectName(QString::fromUtf8("lista_luvas"));

        horizontalLayout_63->addWidget(lista_luvas);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        botao_usar_luvas = new QPushButton(groupBox_18);
        botao_usar_luvas->setObjectName(QString::fromUtf8("botao_usar_luvas"));

        verticalLayout_8->addWidget(botao_usar_luvas);

        botao_adicionar_luvas = new QPushButton(groupBox_18);
        botao_adicionar_luvas->setObjectName(QString::fromUtf8("botao_adicionar_luvas"));

        verticalLayout_8->addWidget(botao_adicionar_luvas);

        botao_remover_luvas = new QPushButton(groupBox_18);
        botao_remover_luvas->setObjectName(QString::fromUtf8("botao_remover_luvas"));

        verticalLayout_8->addWidget(botao_remover_luvas);

        botao_doar_luvas = new QPushButton(groupBox_18);
        botao_doar_luvas->setObjectName(QString::fromUtf8("botao_doar_luvas"));

        verticalLayout_8->addWidget(botao_doar_luvas);


        horizontalLayout_63->addLayout(verticalLayout_8);


        gridLayout_7->addWidget(groupBox_18, 1, 1, 1, 1);

        groupBox_27 = new QGroupBox(tab);
        groupBox_27->setObjectName(QString::fromUtf8("groupBox_27"));
        horizontalLayout_70 = new QHBoxLayout(groupBox_27);
        horizontalLayout_70->setObjectName(QString::fromUtf8("horizontalLayout_70"));
        lista_varinhas = new QListWidget(groupBox_27);
        lista_varinhas->setObjectName(QString::fromUtf8("lista_varinhas"));

        horizontalLayout_70->addWidget(lista_varinhas);

        verticalLayout_26 = new QVBoxLayout();
        verticalLayout_26->setObjectName(QString::fromUtf8("verticalLayout_26"));
        botao_adicionar_varinha = new QPushButton(groupBox_27);
        botao_adicionar_varinha->setObjectName(QString::fromUtf8("botao_adicionar_varinha"));
        sizePolicy3.setHeightForWidth(botao_adicionar_varinha->sizePolicy().hasHeightForWidth());
        botao_adicionar_varinha->setSizePolicy(sizePolicy3);

        verticalLayout_26->addWidget(botao_adicionar_varinha);

        botao_duplicar_varinha = new QPushButton(groupBox_27);
        botao_duplicar_varinha->setObjectName(QString::fromUtf8("botao_duplicar_varinha"));

        verticalLayout_26->addWidget(botao_duplicar_varinha);

        botao_remover_varinha = new QPushButton(groupBox_27);
        botao_remover_varinha->setObjectName(QString::fromUtf8("botao_remover_varinha"));
        sizePolicy3.setHeightForWidth(botao_remover_varinha->sizePolicy().hasHeightForWidth());
        botao_remover_varinha->setSizePolicy(sizePolicy3);

        verticalLayout_26->addWidget(botao_remover_varinha);

        botao_ordenar_varinhas = new QPushButton(groupBox_27);
        botao_ordenar_varinhas->setObjectName(QString::fromUtf8("botao_ordenar_varinhas"));

        verticalLayout_26->addWidget(botao_ordenar_varinhas);

        botao_doar_varinha = new QPushButton(groupBox_27);
        botao_doar_varinha->setObjectName(QString::fromUtf8("botao_doar_varinha"));

        verticalLayout_26->addWidget(botao_doar_varinha);


        horizontalLayout_70->addLayout(verticalLayout_26);


        gridLayout_7->addWidget(groupBox_27, 2, 3, 1, 1);

        groupBox_11 = new QGroupBox(tab);
        groupBox_11->setObjectName(QString::fromUtf8("groupBox_11"));
        verticalLayout_21 = new QVBoxLayout(groupBox_11);
        verticalLayout_21->setObjectName(QString::fromUtf8("verticalLayout_21"));
        horizontalLayout_53 = new QHBoxLayout();
        horizontalLayout_53->setObjectName(QString::fromUtf8("horizontalLayout_53"));
        label_107 = new QLabel(groupBox_11);
        label_107->setObjectName(QString::fromUtf8("label_107"));
        label_107->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_53->addWidget(label_107);

        spin_po = new QSpinBox(groupBox_11);
        spin_po->setObjectName(QString::fromUtf8("spin_po"));
        spin_po->setMaximum(10000000);

        horizontalLayout_53->addWidget(spin_po);


        verticalLayout_21->addLayout(horizontalLayout_53);

        horizontalLayout_54 = new QHBoxLayout();
        horizontalLayout_54->setObjectName(QString::fromUtf8("horizontalLayout_54"));
        label_108 = new QLabel(groupBox_11);
        label_108->setObjectName(QString::fromUtf8("label_108"));
        label_108->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_54->addWidget(label_108);

        spin_pp = new QSpinBox(groupBox_11);
        spin_pp->setObjectName(QString::fromUtf8("spin_pp"));
        spin_pp->setMaximum(10000000);

        horizontalLayout_54->addWidget(spin_pp);


        verticalLayout_21->addLayout(horizontalLayout_54);

        horizontalLayout_56 = new QHBoxLayout();
        horizontalLayout_56->setObjectName(QString::fromUtf8("horizontalLayout_56"));
        label_109 = new QLabel(groupBox_11);
        label_109->setObjectName(QString::fromUtf8("label_109"));
        label_109->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_56->addWidget(label_109);

        spin_pc = new QSpinBox(groupBox_11);
        spin_pc->setObjectName(QString::fromUtf8("spin_pc"));
        spin_pc->setMaximum(10000000);

        horizontalLayout_56->addWidget(spin_pc);


        verticalLayout_21->addLayout(horizontalLayout_56);

        horizontalLayout_57 = new QHBoxLayout();
        horizontalLayout_57->setObjectName(QString::fromUtf8("horizontalLayout_57"));
        label_110 = new QLabel(groupBox_11);
        label_110->setObjectName(QString::fromUtf8("label_110"));
        label_110->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_57->addWidget(label_110);

        spin_pl = new QSpinBox(groupBox_11);
        spin_pl->setObjectName(QString::fromUtf8("spin_pl"));
        spin_pl->setMaximum(10000000);

        horizontalLayout_57->addWidget(spin_pl);


        verticalLayout_21->addLayout(horizontalLayout_57);

        horizontalLayout_55 = new QHBoxLayout();
        horizontalLayout_55->setObjectName(QString::fromUtf8("horizontalLayout_55"));
        labelpe = new QLabel(groupBox_11);
        labelpe->setObjectName(QString::fromUtf8("labelpe"));
        labelpe->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_55->addWidget(labelpe);

        spin_pe = new QSpinBox(groupBox_11);
        spin_pe->setObjectName(QString::fromUtf8("spin_pe"));
        spin_pe->setMaximum(10000000);

        horizontalLayout_55->addWidget(spin_pe);


        verticalLayout_21->addLayout(horizontalLayout_55);


        gridLayout_7->addWidget(groupBox_11, 3, 3, 1, 1);

        groupBox_21 = new QGroupBox(tab);
        groupBox_21->setObjectName(QString::fromUtf8("groupBox_21"));
        horizontalLayout_66 = new QHBoxLayout(groupBox_21);
        horizontalLayout_66->setObjectName(QString::fromUtf8("horizontalLayout_66"));
        lista_tesouro = new QPlainTextEdit(groupBox_21);
        lista_tesouro->setObjectName(QString::fromUtf8("lista_tesouro"));

        horizontalLayout_66->addWidget(lista_tesouro);


        gridLayout_7->addWidget(groupBox_21, 4, 0, 1, 4);

        tabs->addTab(tab, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        verticalLayout_4 = new QVBoxLayout(tab_4);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        botao_renovar_feiticos = new QPushButton(tab_4);
        botao_renovar_feiticos->setObjectName(QString::fromUtf8("botao_renovar_feiticos"));

        verticalLayout_4->addWidget(botao_renovar_feiticos);

        arvore_feiticos = new QTreeWidget(tab_4);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        arvore_feiticos->setHeaderItem(__qtreewidgetitem);
        arvore_feiticos->setObjectName(QString::fromUtf8("arvore_feiticos"));
        sizePolicy1.setHeightForWidth(arvore_feiticos->sizePolicy().hasHeightForWidth());
        arvore_feiticos->setSizePolicy(sizePolicy1);
        arvore_feiticos->setDragEnabled(false);
        arvore_feiticos->setDragDropMode(QAbstractItemView::NoDragDrop);
        arvore_feiticos->setHeaderHidden(true);
        arvore_feiticos->setColumnCount(1);
        arvore_feiticos->header()->setDefaultSectionSize(100);

        verticalLayout_4->addWidget(arvore_feiticos);

        tabs->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        verticalLayout_5 = new QVBoxLayout(tab_2);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        horizontalLayout_32 = new QHBoxLayout();
        horizontalLayout_32->setObjectName(QString::fromUtf8("horizontalLayout_32"));
        label_45 = new QLabel(tab_2);
        label_45->setObjectName(QString::fromUtf8("label_45"));
        sizePolicy7.setHeightForWidth(label_45->sizePolicy().hasHeightForWidth());
        label_45->setSizePolicy(sizePolicy7);
        label_45->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_32->addWidget(label_45);

        texto_notas = new QPlainTextEdit(tab_2);
        texto_notas->setObjectName(QString::fromUtf8("texto_notas"));

        horizontalLayout_32->addWidget(texto_notas);


        verticalLayout_5->addLayout(horizontalLayout_32);

        tabs->addTab(tab_2, QString());

        verticalLayout_3->addWidget(tabs);

        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        sizePolicy4.setHeightForWidth(botoes->sizePolicy().hasHeightForWidth());
        botoes->setSizePolicy(sizePolicy4);
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_3->addWidget(botoes);

        QWidget::setTabOrder(campo_id, campo_rotulo);
        QWidget::setTabOrder(campo_rotulo, lista_rotulos);
        QWidget::setTabOrder(lista_rotulos, checkbox_cor);
        QWidget::setTabOrder(checkbox_cor, botao_cor);
        QWidget::setTabOrder(botao_cor, slider_alfa);
        QWidget::setTabOrder(slider_alfa, spin_raio_quad);
        QWidget::setTabOrder(spin_raio_quad, botao_luz);
        QWidget::setTabOrder(botao_luz, slider_tamanho);
        QWidget::setTabOrder(slider_tamanho, spin_pontos_vida);
        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, botao_bonus_pv_temporario);
        QWidget::setTabOrder(botao_bonus_pv_temporario, spin_dano_nao_letal);
        QWidget::setTabOrder(spin_dano_nao_letal, combo_textura);
        QWidget::setTabOrder(combo_textura, spin_tex_altura);
        QWidget::setTabOrder(spin_tex_altura, spin_tex_largura);
        QWidget::setTabOrder(spin_tex_largura, spin_tex_trans_x);
        QWidget::setTabOrder(spin_tex_trans_x, spin_tex_trans_y);
        QWidget::setTabOrder(spin_tex_trans_y, checkbox_selecionavel);
        QWidget::setTabOrder(checkbox_selecionavel, checkbox_voadora);
        QWidget::setTabOrder(checkbox_voadora, checkbox_visibilidade);
        QWidget::setTabOrder(checkbox_visibilidade, checkbox_caida);
        QWidget::setTabOrder(checkbox_caida, checkbox_morta);
        QWidget::setTabOrder(checkbox_morta, checkbox_nocauteada);
        QWidget::setTabOrder(checkbox_nocauteada, checkbox_incapacitada);
        QWidget::setTabOrder(checkbox_incapacitada, checkbox_inconsciente);
        QWidget::setTabOrder(checkbox_inconsciente, spin_aura_quad);
        QWidget::setTabOrder(spin_aura_quad, spin_translacao_quad);
        QWidget::setTabOrder(spin_translacao_quad, combo_visao);
        QWidget::setTabOrder(combo_visao, spin_raio_visao_escuro_quad);
        QWidget::setTabOrder(spin_raio_visao_escuro_quad, checkbox_salvacao);
        QWidget::setTabOrder(checkbox_salvacao, combo_salvacao);
        QWidget::setTabOrder(combo_salvacao, combo_modelos_3d);
        QWidget::setTabOrder(combo_modelos_3d, lista_formas_alternativas);
        QWidget::setTabOrder(lista_formas_alternativas, botao_adicionar_forma_alternativa);
        QWidget::setTabOrder(botao_adicionar_forma_alternativa, botao_remover_forma_alternativa);
        QWidget::setTabOrder(botao_remover_forma_alternativa, spin_forca);
        QWidget::setTabOrder(spin_forca, botao_bonus_forca);
        QWidget::setTabOrder(botao_bonus_forca, spin_destreza);
        QWidget::setTabOrder(spin_destreza, botao_bonus_destreza);
        QWidget::setTabOrder(botao_bonus_destreza, spin_constituicao);
        QWidget::setTabOrder(spin_constituicao, botao_bonus_constituicao);
        QWidget::setTabOrder(botao_bonus_constituicao, spin_inteligencia);
        QWidget::setTabOrder(spin_inteligencia, botao_bonus_inteligencia);
        QWidget::setTabOrder(botao_bonus_inteligencia, spin_sabedoria);
        QWidget::setTabOrder(spin_sabedoria, botao_bonus_sabedoria);
        QWidget::setTabOrder(botao_bonus_sabedoria, spin_carisma);
        QWidget::setTabOrder(spin_carisma, botao_bonus_carisma);
        QWidget::setTabOrder(botao_bonus_carisma, botao_bonus_iniciativa);
        QWidget::setTabOrder(botao_bonus_iniciativa, checkbox_iniciativa);
        QWidget::setTabOrder(checkbox_iniciativa, spin_iniciativa);
        QWidget::setTabOrder(spin_iniciativa, combo_armadura);
        QWidget::setTabOrder(combo_armadura, combo_material_armadura);
        QWidget::setTabOrder(combo_material_armadura, checkbox_armadura_obra_prima);
        QWidget::setTabOrder(checkbox_armadura_obra_prima, spin_ca_armadura_melhoria);
        QWidget::setTabOrder(spin_ca_armadura_melhoria, botao_bonus_ca);
        QWidget::setTabOrder(botao_bonus_ca, combo_escudo);
        QWidget::setTabOrder(combo_escudo, combo_material_escudo);
        QWidget::setTabOrder(combo_material_escudo, checkbox_escudo_obra_prima);
        QWidget::setTabOrder(checkbox_escudo_obra_prima, spin_ca_escudo_melhoria);
        QWidget::setTabOrder(spin_ca_escudo_melhoria, botao_bonus_salvacao_fortitude);
        QWidget::setTabOrder(botao_bonus_salvacao_fortitude, botao_bonus_salvacao_reflexo);
        QWidget::setTabOrder(botao_bonus_salvacao_reflexo, botao_bonus_salvacao_vontade);
        QWidget::setTabOrder(botao_bonus_salvacao_vontade, botao_resistencia_magia);
        QWidget::setTabOrder(botao_resistencia_magia, checkbox_imune_critico);
        QWidget::setTabOrder(checkbox_imune_critico, spin_mov_terrestre);
        QWidget::setTabOrder(spin_mov_terrestre, botao_mov_terrestre);
        QWidget::setTabOrder(botao_mov_terrestre, spin_mov_aereo);
        QWidget::setTabOrder(spin_mov_aereo, botao_mov_aereo);
        QWidget::setTabOrder(botao_mov_aereo, spin_mov_nadando);
        QWidget::setTabOrder(spin_mov_nadando, botao_mov_nadando);
        QWidget::setTabOrder(botao_mov_nadando, spin_mov_escavando);
        QWidget::setTabOrder(spin_mov_escavando, botao_mov_escavando);
        QWidget::setTabOrder(botao_mov_escavando, spin_mov_escalando);
        QWidget::setTabOrder(spin_mov_escalando, botao_mov_escalando);
        QWidget::setTabOrder(botao_mov_escalando, tabela_lista_eventos);
        QWidget::setTabOrder(tabela_lista_eventos, botao_adicionar_evento);
        QWidget::setTabOrder(botao_adicionar_evento, botao_remover_evento);
        QWidget::setTabOrder(botao_remover_evento, linha_nivel);
        QWidget::setTabOrder(linha_nivel, spin_niveis_negativos);
        QWidget::setTabOrder(spin_niveis_negativos, linha_bba);
        QWidget::setTabOrder(linha_bba, spin_xp);
        QWidget::setTabOrder(spin_xp, slider_bem_mal);
        QWidget::setTabOrder(slider_bem_mal, slider_ordem_caos);
        QWidget::setTabOrder(slider_ordem_caos, combo_raca);
        QWidget::setTabOrder(combo_raca, combo_classe);
        QWidget::setTabOrder(combo_classe, linha_classe);
        QWidget::setTabOrder(linha_classe, spin_nivel_classe);
        QWidget::setTabOrder(spin_nivel_classe, spin_bba);
        QWidget::setTabOrder(spin_bba, combo_salvacoes_fortes);
        QWidget::setTabOrder(combo_salvacoes_fortes, spin_nivel_conjurador);
        QWidget::setTabOrder(spin_nivel_conjurador, combo_mod_conjuracao);
        QWidget::setTabOrder(combo_mod_conjuracao, combo_especializacao_escola);
        QWidget::setTabOrder(combo_especializacao_escola, combo_escola_proibida_1);
        QWidget::setTabOrder(combo_escola_proibida_1, combo_escola_proibida_2);
        QWidget::setTabOrder(combo_escola_proibida_2, combo_dominio_1);
        QWidget::setTabOrder(combo_dominio_1, combo_dominio_2);
        QWidget::setTabOrder(combo_dominio_2, lista_niveis);
        QWidget::setTabOrder(lista_niveis, botao_adicionar_nivel);
        QWidget::setTabOrder(botao_adicionar_nivel, botao_remover_nivel);
        QWidget::setTabOrder(botao_remover_nivel, tabela_pericias);
        QWidget::setTabOrder(tabela_pericias, tabela_talentos);
        QWidget::setTabOrder(tabela_talentos, botao_adicionar_talento);
        QWidget::setTabOrder(botao_adicionar_talento, botao_remover_talento);
        QWidget::setTabOrder(botao_remover_talento, linha_furtivo);
        QWidget::setTabOrder(linha_furtivo, combo_esquiva_sobrenatural);
        QWidget::setTabOrder(combo_esquiva_sobrenatural, combo_evasao_estatica);
        QWidget::setTabOrder(combo_evasao_estatica, combo_evasao_dinamica);
        QWidget::setTabOrder(combo_evasao_dinamica, tabela_inimigos_prediletos);
        QWidget::setTabOrder(tabela_inimigos_prediletos, botao_adicionar_inimigo_predileto);
        QWidget::setTabOrder(botao_adicionar_inimigo_predileto, botao_remover_inimigo_predileto);
        QWidget::setTabOrder(botao_remover_inimigo_predileto, linha_grupo_ataque);
        QWidget::setTabOrder(linha_grupo_ataque, linha_rotulo_ataque);
        QWidget::setTabOrder(linha_rotulo_ataque, spin_ordem_ataque);
        QWidget::setTabOrder(spin_ordem_ataque, combo_empunhadura);
        QWidget::setTabOrder(combo_empunhadura, combo_tipo_ataque);
        QWidget::setTabOrder(combo_tipo_ataque, combo_arma);
        QWidget::setTabOrder(combo_arma, checkbox_op);
        QWidget::setTabOrder(checkbox_op, spin_bonus_magico);
        QWidget::setTabOrder(spin_bonus_magico, combo_material_arma);
        QWidget::setTabOrder(combo_material_arma, botao_bonus_ataque);
        QWidget::setTabOrder(botao_bonus_ataque, linha_dano);
        QWidget::setTabOrder(linha_dano, spin_municao);
        QWidget::setTabOrder(spin_municao, spin_alcance_quad);
        QWidget::setTabOrder(spin_alcance_quad, spin_incrementos);
        QWidget::setTabOrder(spin_incrementos, botao_bonus_dano);
        QWidget::setTabOrder(botao_bonus_dano, spin_nivel_conjurador_pergaminho);
        QWidget::setTabOrder(spin_nivel_conjurador_pergaminho, spin_modificador_atributo_pergaminho);
        QWidget::setTabOrder(spin_modificador_atributo_pergaminho, spin_limite_vezes);
        QWidget::setTabOrder(spin_limite_vezes, checkbox_ignora_rm);
        QWidget::setTabOrder(checkbox_ignora_rm, checkbox_permite_salvacao);
        QWidget::setTabOrder(checkbox_permite_salvacao, checkbox_ataque_agarrar);
        QWidget::setTabOrder(checkbox_ataque_agarrar, checkbox_ataque_toque);
        QWidget::setTabOrder(checkbox_ataque_toque, lista_ataques);
        QWidget::setTabOrder(lista_ataques, botao_ataque_cima);
        QWidget::setTabOrder(botao_ataque_cima, botao_ataque_baixo);
        QWidget::setTabOrder(botao_ataque_baixo, botao_remover_ataque);
        QWidget::setTabOrder(botao_remover_ataque, botao_adicionar_ataque);
        QWidget::setTabOrder(botao_adicionar_ataque, botao_clonar_ataque);
        QWidget::setTabOrder(botao_clonar_ataque, lista_pocoes);
        QWidget::setTabOrder(lista_pocoes, botao_adicionar_pocao);
        QWidget::setTabOrder(botao_adicionar_pocao, botao_duplicar_pocao);
        QWidget::setTabOrder(botao_duplicar_pocao, botao_remover_pocao);
        QWidget::setTabOrder(botao_remover_pocao, botao_ordenar_pocoes);
        QWidget::setTabOrder(botao_ordenar_pocoes, botao_doar_pocao);
        QWidget::setTabOrder(botao_doar_pocao, lista_botas);
        QWidget::setTabOrder(lista_botas, botao_usar_botas);
        QWidget::setTabOrder(botao_usar_botas, botao_adicionar_botas);
        QWidget::setTabOrder(botao_adicionar_botas, botao_remover_botas);
        QWidget::setTabOrder(botao_remover_botas, botao_doar_botas);
        QWidget::setTabOrder(botao_doar_botas, lista_amuletos);
        QWidget::setTabOrder(lista_amuletos, botao_usar_amuleto);
        QWidget::setTabOrder(botao_usar_amuleto, botao_adicionar_amuleto);
        QWidget::setTabOrder(botao_adicionar_amuleto, botao_remover_amuleto);
        QWidget::setTabOrder(botao_remover_amuleto, botao_doar_amuleto);
        QWidget::setTabOrder(botao_doar_amuleto, lista_chapeus);
        QWidget::setTabOrder(lista_chapeus, botao_vestir_chapeu);
        QWidget::setTabOrder(botao_vestir_chapeu, botao_adicionar_chapeu);
        QWidget::setTabOrder(botao_adicionar_chapeu, botao_remover_chapeu);
        QWidget::setTabOrder(botao_remover_chapeu, botao_doar_chapeu);
        QWidget::setTabOrder(botao_doar_chapeu, lista_pergaminhos_arcanos);
        QWidget::setTabOrder(lista_pergaminhos_arcanos, botao_adicionar_pergaminho_arcano);
        QWidget::setTabOrder(botao_adicionar_pergaminho_arcano, botao_duplicar_pergaminho_arcano);
        QWidget::setTabOrder(botao_duplicar_pergaminho_arcano, botao_remover_pergaminho_arcano);
        QWidget::setTabOrder(botao_remover_pergaminho_arcano, botao_ordenar_pergaminhos_arcanos);
        QWidget::setTabOrder(botao_ordenar_pergaminhos_arcanos, botao_doar_pergaminho_arcano);
        QWidget::setTabOrder(botao_doar_pergaminho_arcano, lista_luvas);
        QWidget::setTabOrder(lista_luvas, botao_usar_luvas);
        QWidget::setTabOrder(botao_usar_luvas, botao_adicionar_luvas);
        QWidget::setTabOrder(botao_adicionar_luvas, botao_remover_luvas);
        QWidget::setTabOrder(botao_remover_luvas, botao_doar_luvas);
        QWidget::setTabOrder(botao_doar_luvas, lista_bracadeiras);
        QWidget::setTabOrder(lista_bracadeiras, botao_usar_bracadeiras);
        QWidget::setTabOrder(botao_usar_bracadeiras, botao_adicionar_bracadeiras);
        QWidget::setTabOrder(botao_adicionar_bracadeiras, botao_remover_bracadeiras);
        QWidget::setTabOrder(botao_remover_bracadeiras, botao_doar_bracadeiras);
        QWidget::setTabOrder(botao_doar_bracadeiras, lista_mantos);
        QWidget::setTabOrder(lista_mantos, botao_usar_manto);
        QWidget::setTabOrder(botao_usar_manto, botao_adicionar_manto);
        QWidget::setTabOrder(botao_adicionar_manto, botao_remover_manto);
        QWidget::setTabOrder(botao_remover_manto, botao_doar_manto);
        QWidget::setTabOrder(botao_doar_manto, lista_pergaminhos_divinos);
        QWidget::setTabOrder(lista_pergaminhos_divinos, botao_adicionar_pergaminho_divino);
        QWidget::setTabOrder(botao_adicionar_pergaminho_divino, botao_duplicar_pergaminho_divino);
        QWidget::setTabOrder(botao_duplicar_pergaminho_divino, botao_remover_pergaminho_divino);
        QWidget::setTabOrder(botao_remover_pergaminho_divino, botao_ordenar_pergaminhos_divinos);
        QWidget::setTabOrder(botao_ordenar_pergaminhos_divinos, botao_doar_pergaminho_divino);
        QWidget::setTabOrder(botao_doar_pergaminho_divino, lista_itens_mundanos);
        QWidget::setTabOrder(lista_itens_mundanos, botao_adicionar_item_mundano);
        QWidget::setTabOrder(botao_adicionar_item_mundano, botao_duplicar_item_mundano);
        QWidget::setTabOrder(botao_duplicar_item_mundano, botao_remover_item_mundano);
        QWidget::setTabOrder(botao_remover_item_mundano, botao_ordenar_item_mundano);
        QWidget::setTabOrder(botao_ordenar_item_mundano, botao_doar_item_mundano);
        QWidget::setTabOrder(botao_doar_item_mundano, lista_aneis);
        QWidget::setTabOrder(lista_aneis, botao_usar_anel);
        QWidget::setTabOrder(botao_usar_anel, botao_adicionar_anel);
        QWidget::setTabOrder(botao_adicionar_anel, botao_remover_anel);
        QWidget::setTabOrder(botao_remover_anel, botao_doar_anel);
        QWidget::setTabOrder(botao_doar_anel, lista_varinhas);
        QWidget::setTabOrder(lista_varinhas, botao_adicionar_varinha);
        QWidget::setTabOrder(botao_adicionar_varinha, botao_duplicar_varinha);
        QWidget::setTabOrder(botao_duplicar_varinha, botao_remover_varinha);
        QWidget::setTabOrder(botao_remover_varinha, botao_ordenar_varinhas);
        QWidget::setTabOrder(botao_ordenar_varinhas, botao_doar_varinha);
        QWidget::setTabOrder(botao_doar_varinha, lista_armas);
        QWidget::setTabOrder(lista_armas, botao_adicionar_arma);
        QWidget::setTabOrder(botao_adicionar_arma, botao_duplicar_arma);
        QWidget::setTabOrder(botao_duplicar_arma, botao_remover_arma);
        QWidget::setTabOrder(botao_remover_arma, botao_ordenar_armas);
        QWidget::setTabOrder(botao_ordenar_armas, botao_doar_arma);
        QWidget::setTabOrder(botao_doar_arma, lista_armaduras);
        QWidget::setTabOrder(lista_armaduras, botao_adicionar_armadura);
        QWidget::setTabOrder(botao_adicionar_armadura, botao_duplicar_armadura);
        QWidget::setTabOrder(botao_duplicar_armadura, botao_remover_armadura);
        QWidget::setTabOrder(botao_remover_armadura, botao_ordenar_armaduras);
        QWidget::setTabOrder(botao_ordenar_armaduras, botao_doar_armadura);
        QWidget::setTabOrder(botao_doar_armadura, lista_escudos);
        QWidget::setTabOrder(lista_escudos, botao_adicionar_escudo);
        QWidget::setTabOrder(botao_adicionar_escudo, botao_duplicar_escudo);
        QWidget::setTabOrder(botao_duplicar_escudo, botao_remover_escudo);
        QWidget::setTabOrder(botao_remover_escudo, botao_ordenar_escudos);
        QWidget::setTabOrder(botao_ordenar_escudos, botao_doar_escudo);
        QWidget::setTabOrder(botao_doar_escudo, spin_po);
        QWidget::setTabOrder(spin_po, spin_pp);
        QWidget::setTabOrder(spin_pp, spin_pc);
        QWidget::setTabOrder(spin_pc, spin_pl);
        QWidget::setTabOrder(spin_pl, spin_pe);
        QWidget::setTabOrder(spin_pe, lista_tesouro);
        QWidget::setTabOrder(lista_tesouro, botao_renovar_feiticos);
        QWidget::setTabOrder(botao_renovar_feiticos, arvore_feiticos);
        QWidget::setTabOrder(arvore_feiticos, texto_notas);
        QWidget::setTabOrder(texto_notas, tabs);

        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));

        tabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", nullptr));
#if QT_CONFIG(tooltip)
        ifg__qt__DialogoEntidade->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Id", nullptr));
        label_8->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulo", nullptr));
        label_10->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos Especial", nullptr));
#if QT_CONFIG(tooltip)
        lista_rotulos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", nullptr));
        label_tamanho->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "(m\303\251dio)", nullptr));
#if QT_CONFIG(tooltip)
        slider_tamanho->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tamanho da entidade", nullptr));
#endif // QT_CONFIG(tooltip)
        label_5->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PV", nullptr));
#if QT_CONFIG(tooltip)
        spin_pontos_vida->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Pontos de vida para entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_6->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Max", nullptr));
#if QT_CONFIG(tooltip)
        spin_max_pontos_vida->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de pontos de vida para entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_26->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Temp", nullptr));
#if QT_CONFIG(tooltip)
        botao_bonus_pv_temporario->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Pontos de Vida tempor\303\241rios.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_bonus_pv_temporario->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        label_79->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\243o letal", nullptr));
#if QT_CONFIG(tooltip)
        spin_dano_nao_letal->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Quantidade de dano n\303\243o letal.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_selecionavel->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, jogadores poder\303\243o ver as propriedades e controlar a entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_selecionavel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Selecion\303\241vel", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_voadora->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade flutuar\303\241.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_voadora->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Voadora", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_visibilidade->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade ser\303\241 vista para jogadores. Caso seja selecion\303\241vel, a entidade ficar\303\241 transl\303\272cida.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_visibilidade->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\255vel", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_caida->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade cair\303\241.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_caida->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ca\303\255da", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_morta->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade estar\303\241 morta.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_morta->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Morta", nullptr));
        checkbox_nocauteada->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nocauteada", nullptr));
        checkbox_incapacitada->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Incapacitada", nullptr));
        checkbox_inconsciente->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Inconsciente", nullptr));
        label_4->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Aura:", nullptr));
#if QT_CONFIG(tooltip)
        spin_aura_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Aura da entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_35->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "quadrados", nullptr));
        label_7->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Deslocamento Vertical", nullptr));
#if QT_CONFIG(tooltip)
        spin_translacao_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Para colocar a entidade acima do plano do tabuleiro.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_34->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "quadrados", nullptr));
        label_13->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o", nullptr));
        combo_visao->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Normal", nullptr));
        combo_visao->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o na Penumbra", nullptr));
        combo_visao->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o no Escuro", nullptr));

#if QT_CONFIG(tooltip)
        combo_visao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo de vis\303\243o da entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_14->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Raio", nullptr));
#if QT_CONFIG(tooltip)
        spin_raio_visao_escuro_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_33->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "quadrados", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_salvacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Marcar, se a entidade tiver rolado a pr\303\263xima salva\303\247\303\243o.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_salvacao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Pr\303\263xima Salva\303\247\303\243o", nullptr));
        combo_salvacao->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Falha", nullptr));
        combo_salvacao->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Meio Dano", nullptr));
        combo_salvacao->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Um Quarto de Dano", nullptr));
        combo_salvacao->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dano Anulado", nullptr));

#if QT_CONFIG(tooltip)
        combo_salvacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dano que a entidade receber\303\241 na pr\303\263xima a\303\247\303\243o de \303\241rea.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_16->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modelo 3D", nullptr));
        label_68->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Formas Alternativas", nullptr));
#if QT_CONFIG(tooltip)
        lista_formas_alternativas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Lista de formas alternativas, clique duplo para editar.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        botao_adicionar_forma_alternativa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adiciona uma forma alternativa", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_adicionar_forma_alternativa->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
#if QT_CONFIG(tooltip)
        botao_remover_forma_alternativa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Remove a forma alternativa selecionada", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_remover_forma_alternativa->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        groupBox_9->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Cor", nullptr));
        label_3->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Cor:", nullptr));
        checkbox_cor->setText(QString());
#if QT_CONFIG(tooltip)
        botao_cor->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Cor da entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_cor->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", nullptr));
        label_81->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transpar\303\252ncia", nullptr));
        groupBox_8->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Luz", nullptr));
        label_12->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Raio", nullptr));
#if QT_CONFIG(tooltip)
        spin_raio_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Raio da luz, em metros.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_31->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "quadrados", nullptr));
        botao_luz->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor da Luz", nullptr));
        groupBox_10->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Textura", nullptr));
        label_17->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "altura", nullptr));
#if QT_CONFIG(tooltip)
        spin_tex_altura->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Altura da textura, de 0 a 1", nullptr));
#endif // QT_CONFIG(tooltip)
        label_18->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "largura", nullptr));
#if QT_CONFIG(tooltip)
        spin_tex_largura->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Largura da textura, de 0 a 1", nullptr));
#endif // QT_CONFIG(tooltip)
        label_19->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "trans x", nullptr));
#if QT_CONFIG(tooltip)
        spin_tex_trans_x->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", nullptr));
#endif // QT_CONFIG(tooltip)
        label_20->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "trans y", nullptr));
#if QT_CONFIG(tooltip)
        spin_tex_trans_y->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", nullptr));
#endif // QT_CONFIG(tooltip)
        tabs->setTabText(tabs->indexOf(tab_geral), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Geral", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Salva\303\247\303\265es", nullptr));
        label_65->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Reflexos", nullptr));
        label_66->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Vontade", nullptr));
        botao_bonus_salvacao_fortitude->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        botao_bonus_salvacao_vontade->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_67->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Fortitude", nullptr));
        botao_bonus_salvacao_reflexo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_70->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Total", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Atributos", nullptr));
        botao_bonus_constituicao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_47->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Int", nullptr));
#if QT_CONFIG(tooltip)
        spin_destreza->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Destreza", nullptr));
#endif // QT_CONFIG(tooltip)
        label_49->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Sab", nullptr));
        label_51->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Car", nullptr));
        botao_bonus_destreza->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_50->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Con", nullptr));
#if QT_CONFIG(tooltip)
        label_mod_destreza->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_destreza->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
#if QT_CONFIG(tooltip)
        spin_carisma->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Carisma", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_bonus_forca->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
#if QT_CONFIG(tooltip)
        spin_sabedoria->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Sabedoria", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spin_inteligencia->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Intelig\303\252ncia", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        spin_forca->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "For\303\247a", nullptr));
#endif // QT_CONFIG(tooltip)
        label_27->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "For", nullptr));
#if QT_CONFIG(tooltip)
        spin_constituicao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Constitui\303\247\303\243o", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_mod_forca->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_forca->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
#if QT_CONFIG(tooltip)
        label_mod_sabedoria->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_sabedoria->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
        botao_bonus_sabedoria->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_48->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Des", nullptr));
        botao_bonus_inteligencia->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
#if QT_CONFIG(tooltip)
        label_mod_inteligencia->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_inteligencia->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
        botao_bonus_carisma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
#if QT_CONFIG(tooltip)
        label_mod_constituicao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_constituicao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
#if QT_CONFIG(tooltip)
        label_mod_carisma->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_carisma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+0", nullptr));
        label_28->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mod", nullptr));
        label_63->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Total", nullptr));
        label_62->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Base", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Iniciativa", nullptr));
        label_22->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador", nullptr));
#if QT_CONFIG(tooltip)
        botao_bonus_iniciativa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificadores de iniciativa.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_bonus_iniciativa->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_iniciativa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade ter\303\241 iniciativa", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_iniciativa->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ligado?   Valor", nullptr));
#if QT_CONFIG(tooltip)
        spin_iniciativa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Valor da iniciativa para o combate corrente", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_6->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque", nullptr));
        label_bba_agarrar->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        label_69->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Corpo a Corpo", nullptr));
        label_54->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Base", nullptr));
        label_bba_base->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        label_56->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Agarrar", nullptr));
        label_71->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dist\303\242ncia", nullptr));
        label_bba_cac->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        label_bba_distancia->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dados de CA", nullptr));
#if QT_CONFIG(tooltip)
        spin_ca_escudo_melhoria->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Melhoria Escudo", nullptr));
#endif // QT_CONFIG(tooltip)
        combo_material_escudo->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhum", nullptr));
        combo_material_escudo->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adamante", nullptr));
        combo_material_escudo->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Couro de Drag\303\243o", nullptr));
        combo_material_escudo->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Madeira Negra", nullptr));
        combo_material_escudo->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mitral", nullptr));

#if QT_CONFIG(tooltip)
        combo_material_escudo->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo do material da arma", nullptr));
#endif // QT_CONFIG(tooltip)
        spin_bonus_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escudo", nullptr));
        checkbox_escudo_obra_prima->setText(QString());
        label_ca_toque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        label_95->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Material", nullptr));
        label_52->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Armadura", nullptr));
        label_57->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Total", nullptr));
#if QT_CONFIG(tooltip)
        spin_ca_armadura_melhoria->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Melhoria armadura", nullptr));
#endif // QT_CONFIG(tooltip)
        label_58->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Toque", nullptr));
        label_59->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Surp", nullptr));
        label_ca_surpreso->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "0", nullptr));
        botao_bonus_ca->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        spin_bonus_escudo_2->setText(QString());
        combo_material_armadura->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhum", nullptr));
        combo_material_armadura->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adamante", nullptr));
        combo_material_armadura->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Couro de Drag\303\243o", nullptr));
        combo_material_armadura->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mitral", nullptr));

#if QT_CONFIG(tooltip)
        combo_material_armadura->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo do material da arma", nullptr));
#endif // QT_CONFIG(tooltip)
        label_100->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus M\303\241gico", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_armadura_obra_prima->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "armadura obra prima", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_armadura_obra_prima->setText(QString());
        label_61->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "=", nullptr));
        label_53->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nome", nullptr));
        label_60->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Obra Prima", nullptr));
        label_85->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Resist\303\252ncia a Magia", nullptr));
        botao_resistencia_magia->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        checkbox_imune_critico->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Imune a Cr\303\255tico?", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Movimento", nullptr));
        label_37->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Total", nullptr));
        label_25->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "terrestre", nullptr));
        label_90->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "nadando", nullptr));
        label_32->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "a\303\251reo", nullptr));
        label_97->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escavando", nullptr));
        botao_mov_terrestre->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PushButton", nullptr));
        botao_mov_nadando->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PushButton", nullptr));
        botao_mov_aereo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PushButton", nullptr));
        botao_mov_escavando->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PushButton", nullptr));
        label_15->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\241sico Quadrados", nullptr));
        label_74->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escalando", nullptr));
        botao_mov_escalando->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "PushButton", nullptr));
        tabs->setTabText(tabs->indexOf(tab_estatisticas), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Estat\303\255sticas", nullptr));
        label_11->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Lista de Eventos", nullptr));
        botao_adicionar_evento->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_evento->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        tabs->setTabText(tabs->indexOf(tab_7), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Efeitos", nullptr));
        label_39->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel PC", nullptr));
#if QT_CONFIG(tooltip)
        linha_nivel->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel do Personagem", nullptr));
#endif // QT_CONFIG(tooltip)
        label_80->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255veis Negativos", nullptr));
#if QT_CONFIG(tooltip)
        spin_niveis_negativos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255veis negativos.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_46->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "BBA", nullptr));
#if QT_CONFIG(tooltip)
        linha_bba->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque Total", nullptr));
#endif // QT_CONFIG(tooltip)
        label_84->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "XP", nullptr));
#if QT_CONFIG(tooltip)
        spin_xp->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Experi\303\252ncia do personagem.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_75->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mal", nullptr));
        label_78->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bem", nullptr));
        label_76->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Caos", nullptr));
        label_77->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordem", nullptr));
        label_101->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ra\303\247a", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dados N\303\255vel", nullptr));
        label_40->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Classe", nullptr));
#if QT_CONFIG(tooltip)
        linha_classe->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Identificador da Classe", nullptr));
#endif // QT_CONFIG(tooltip)
        label_41->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel", nullptr));
#if QT_CONFIG(tooltip)
        spin_nivel_classe->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel da Classe", nullptr));
#endif // QT_CONFIG(tooltip)
        label_43->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "BBA", nullptr));
#if QT_CONFIG(tooltip)
        spin_bba->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque ", nullptr));
#endif // QT_CONFIG(tooltip)
        label_64->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Salva\303\247\303\265es Fortes", nullptr));
        label_42->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel de Conjurador", nullptr));
#if QT_CONFIG(tooltip)
        spin_nivel_conjurador->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel de Conjurador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_conjuracao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "00", nullptr));
        label_44->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Atributo de Conjura\303\247\303\243o", nullptr));
        combo_mod_conjuracao->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "For\303\247a", nullptr));
        combo_mod_conjuracao->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Destreza", nullptr));
        combo_mod_conjuracao->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Constitui\303\247\303\243o", nullptr));
        combo_mod_conjuracao->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Intelig\303\252ncia", nullptr));
        combo_mod_conjuracao->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Sabedoria", nullptr));
        combo_mod_conjuracao->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Carisma", nullptr));

        label_87->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Especializa\303\247ao Escola", nullptr));
        combo_especializacao_escola->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhuma", nullptr));
        combo_especializacao_escola->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Abjura\303\247\303\243o", nullptr));
        combo_especializacao_escola->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adivinha\303\247\303\243o", nullptr));
        combo_especializacao_escola->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Conjura\303\247\303\243o", nullptr));
        combo_especializacao_escola->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evoca\303\247\303\243o", nullptr));
        combo_especializacao_escola->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Encantamento", nullptr));
        combo_especializacao_escola->setItemText(6, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ilus\303\243o", nullptr));
        combo_especializacao_escola->setItemText(7, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Necromancia", nullptr));
        combo_especializacao_escola->setItemText(8, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transmuta\303\247\303\243o", nullptr));

#if QT_CONFIG(tooltip)
        combo_especializacao_escola->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Caso mago seja especialista, escolha uma esocla de magia", nullptr));
#endif // QT_CONFIG(tooltip)
        label_88->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Proibidas", nullptr));
        combo_escola_proibida_1->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhuma", nullptr));
        combo_escola_proibida_1->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Abjura\303\247\303\243o", nullptr));
        combo_escola_proibida_1->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Conjura\303\247\303\243o", nullptr));
        combo_escola_proibida_1->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evoca\303\247\303\243o", nullptr));
        combo_escola_proibida_1->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Encantamento", nullptr));
        combo_escola_proibida_1->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ilus\303\243o", nullptr));
        combo_escola_proibida_1->setItemText(6, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Necromancia", nullptr));
        combo_escola_proibida_1->setItemText(7, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transmuta\303\247\303\243o", nullptr));

#if QT_CONFIG(tooltip)
        combo_escola_proibida_1->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Caso mago seja especialista, escolha uma escola proibida (exceto adivinha\303\247\303\243o)", nullptr));
#endif // QT_CONFIG(tooltip)
        combo_escola_proibida_2->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhuma", nullptr));
        combo_escola_proibida_2->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Abjura\303\247\303\243o", nullptr));
        combo_escola_proibida_2->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Conjura\303\247\303\243o", nullptr));
        combo_escola_proibida_2->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evoca\303\247\303\243o", nullptr));
        combo_escola_proibida_2->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Encantamento", nullptr));
        combo_escola_proibida_2->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ilus\303\243o", nullptr));
        combo_escola_proibida_2->setItemText(6, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Necromancia", nullptr));
        combo_escola_proibida_2->setItemText(7, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Transmuta\303\247\303\243o", nullptr));

#if QT_CONFIG(tooltip)
        combo_escola_proibida_2->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Caso mago seja especialista, escolha uma escola proibida (exceto adivinha\303\247\303\243o)", nullptr));
#endif // QT_CONFIG(tooltip)
        label_102->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dom\303\255nios", nullptr));
        botao_adicionar_nivel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adicionar N\303\255vel", nullptr));
        botao_remover_nivel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Remover", nullptr));
        tabs->setTabText(tabs->indexOf(tab_nivel), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ra\303\247a, Classes e Tend\303\252ncia", nullptr));
        label_pericias->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Per\303\255cias", nullptr));
        label_talentos->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Talentos", nullptr));
        botao_adicionar_talento->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_talento->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        tabs->setTabText(tabs->indexOf(tab_3), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Per\303\255cias e Talentos", nullptr));
        label_38->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ataque Furtivo", nullptr));
#if QT_CONFIG(tooltip)
        linha_furtivo->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Exemplo: 3d6", nullptr));
#endif // QT_CONFIG(tooltip)
        linha_furtivo->setText(QString());
        label_86->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Esquiva Sobrenatural", nullptr));
        combo_esquiva_sobrenatural->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\243o Possui", nullptr));
        combo_esquiva_sobrenatural->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Normal", nullptr));
        combo_esquiva_sobrenatural->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Aprimorada", nullptr));

        label_103->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evas\303\243o", nullptr));
        combo_evasao_estatica->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Sem Evas\303\243o", nullptr));
        combo_evasao_estatica->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evas\303\243o", nullptr));
        combo_evasao_estatica->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evas\303\243o Aprimorada", nullptr));

#if QT_CONFIG(tooltip)
        combo_evasao_estatica->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo de evas\303\243o que n\303\243o vem de classe, por exemplo, familiares.", nullptr));
#endif // QT_CONFIG(tooltip)
        combo_evasao_dinamica->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Sem Evas\303\243o", nullptr));
        combo_evasao_dinamica->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evas\303\243o", nullptr));
        combo_evasao_dinamica->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Evas\303\243o Aprimorada", nullptr));

#if QT_CONFIG(tooltip)
        combo_evasao_dinamica->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo de evas\303\243o do personagem, computado dinamicamente", nullptr));
#endif // QT_CONFIG(tooltip)
        label_94->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Inimigos Prediletos", nullptr));
        botao_adicionar_inimigo_predileto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adicionar", nullptr));
        botao_remover_inimigo_predileto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Remover", nullptr));
        tabs->setTabText(tabs->indexOf(tab_5), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Habilidades Especiais", nullptr));
        label_91->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Grupo", nullptr));
#if QT_CONFIG(tooltip)
        linha_grupo_ataque->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Identificador do ataque", nullptr));
#endif // QT_CONFIG(tooltip)
        label_36->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nome", nullptr));
#if QT_CONFIG(tooltip)
        linha_rotulo_ataque->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Identificador do ataque", nullptr));
#endif // QT_CONFIG(tooltip)
        label_83->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordem Ataque", nullptr));
#if QT_CONFIG(tooltip)
        spin_ordem_ataque->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordem do ataque (1 para primeiro, 2 para segundo etc)", nullptr));
#endif // QT_CONFIG(tooltip)
        label_73->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Empunhadura", nullptr));
        combo_empunhadura->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma apenas", nullptr));
        combo_empunhadura->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duas m\303\243os", nullptr));
        combo_empunhadura->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "2 Armas, m\303\243o boa ou Arma Dupla, principal", nullptr));
        combo_empunhadura->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "2 Armas, m\303\243o ruim ou Arma Dupla, secund\303\241rio", nullptr));
        combo_empunhadura->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma e Escudo", nullptr));
        combo_empunhadura->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Monstro: Ataque Principal", nullptr));
        combo_empunhadura->setItemText(6, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Monstro: Ataque Secund\303\241rio", nullptr));
        combo_empunhadura->setItemText(7, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Rajada de Monge", nullptr));

        label_21->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo", nullptr));
        label_93->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma", nullptr));
#if QT_CONFIG(tooltip)
        combo_arma->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escolha uma arma, ou nenhuma para preencher manualmente.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_op->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma obra prima?", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_op->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "OP", nullptr));
#if QT_CONFIG(tooltip)
        label_23->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus M\303\241gico", nullptr));
#endif // QT_CONFIG(tooltip)
        label_23->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus M\303\241gico", nullptr));
#if QT_CONFIG(tooltip)
        spin_bonus_magico->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus m\303\241gico da arma", nullptr));
#endif // QT_CONFIG(tooltip)
        label_92->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Material", nullptr));
        combo_material_arma->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Nenhum", nullptr));
        combo_material_arma->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adamante", nullptr));
        combo_material_arma->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ferro Frio", nullptr));
        combo_material_arma->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Madeira Negra", nullptr));
        combo_material_arma->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mitral", nullptr));
        combo_material_arma->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoEntidade", "Prata Alqu\303\255mica", nullptr));

#if QT_CONFIG(tooltip)
        combo_material_arma->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tipo do material da arma", nullptr));
#endif // QT_CONFIG(tooltip)
        label_55->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Ataque", nullptr));
#if QT_CONFIG(tooltip)
        botao_bonus_ataque->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Outros b\303\264nus de ataque.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_bonus_ataque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_24->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dano", nullptr));
#if QT_CONFIG(tooltip)
        linha_dano->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Dano base da arma. Exemplo: 1d8 (19-20/x2)", nullptr));
#endif // QT_CONFIG(tooltip)
        label_82->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Muni\303\247\303\243o", nullptr));
#if QT_CONFIG(tooltip)
        spin_municao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Quantidade de muni\303\247\303\243o da arma.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_29->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Alcance (quads)", nullptr));
#if QT_CONFIG(tooltip)
        spin_alcance_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Alcance em quadrados", nullptr));
#endif // QT_CONFIG(tooltip)
        label_30->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Max incrementos", nullptr));
#if QT_CONFIG(tooltip)
        spin_incrementos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de incrementos permitido pelo ataque", nullptr));
#endif // QT_CONFIG(tooltip)
        label_72->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Outros (dano)", nullptr));
#if QT_CONFIG(tooltip)
        botao_bonus_dano->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Outros b\303\264nus de dano.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_bonus_dano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bonus", nullptr));
        label_104->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel de Conjurador", nullptr));
#if QT_CONFIG(tooltip)
        spin_nivel_conjurador_pergaminho->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Para pergaminhos, o n\303\255vel de conjurador", nullptr));
#endif // QT_CONFIG(tooltip)
        label_mod_atributo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Modificador de Atributo", nullptr));
#if QT_CONFIG(tooltip)
        spin_modificador_atributo_pergaminho->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Para pergaminhos, o modificador de atributo", nullptr));
#endif // QT_CONFIG(tooltip)
        label_9->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Limite de Vezes", nullptr));
#if QT_CONFIG(tooltip)
        spin_limite_vezes->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Se > 0, o ataque poder\303\241 ser usado este n\303\272mero de vezes e depois ser\303\241 consumido.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_ignora_rm->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma obra prima?", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_ignora_rm->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ignora RM", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_permite_salvacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Arma obra prima?", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_permite_salvacao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Permite Salva\303\247\303\243o", nullptr));
        checkbox_ataque_agarrar->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ataque de Agarrar", nullptr));
        checkbox_ataque_toque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ataque de Toque", nullptr));
        botao_ataque_cima->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "\342\206\221", nullptr));
        botao_ataque_baixo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "\342\206\223", nullptr));
        botao_remover_ataque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Remover ataque", nullptr));
        botao_adicionar_ataque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Adicionar Ataque", nullptr));
        botao_clonar_ataque->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Clonar ataque", nullptr));
        tabs->setTabText(tabs->indexOf(tab_6), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ataques", nullptr));
        groupBox_22->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Bra\303\247adeiras", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setText(QString());
        botao_adicionar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_24->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Armas", nullptr));
        botao_adicionar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_arma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_armas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_13->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Amuletos", nullptr));
        botao_usar_amuleto->setText(QString());
        botao_adicionar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_23->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Items Mundanos", nullptr));
        botao_adicionar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_19->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Chap\303\251us", nullptr));
        botao_vestir_chapeu->setText(QString());
        botao_adicionar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_26->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Escudos", nullptr));
        botao_adicionar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_escudos->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_20->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Mantos", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_manto->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_manto->setText(QString());
        botao_adicionar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_manto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_16->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Pergaminhos Divinos", nullptr));
        botao_adicionar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_pergaminhos_divinos->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_25->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Armaduras", nullptr));
        botao_adicionar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_armaduras->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_14->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Po\303\247\303\265es", nullptr));
        botao_adicionar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_pocoes->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_12->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Botas", nullptr));
        botao_usar_botas->setText(QString());
        botao_adicionar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_botas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_15->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Pergaminhos Arcanos", nullptr));
        botao_adicionar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_pergaminhos_arcanos->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_17->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "An\303\251is", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_anel->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_anel->setText(QString());
        botao_adicionar_anel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_anel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_anel->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_18->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Luvas e Manoplas", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_luvas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_luvas->setText(QString());
        botao_adicionar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_remover_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_doar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_27->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Varinhas", nullptr));
        botao_adicionar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "+", nullptr));
        botao_duplicar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Duplicar", nullptr));
        botao_remover_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "-", nullptr));
        botao_ordenar_varinhas->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ordenar", nullptr));
        botao_doar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Doar", nullptr));
        groupBox_11->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Moedas", nullptr));
        label_107->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Ouro", nullptr));
        label_108->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Prata", nullptr));
        label_109->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Cobre", nullptr));
        label_110->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Platina", nullptr));
        labelpe->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Electrum", nullptr));
        groupBox_21->setTitle(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Outros", nullptr));
#if QT_CONFIG(tooltip)
        lista_tesouro->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_CONFIG(tooltip)
        tabs->setTabText(tabs->indexOf(tab), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Tesouro", nullptr));
#if QT_CONFIG(tooltip)
        botao_renovar_feiticos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Deixa todos os feiti\303\247os dispon\303\255veis para uso.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_renovar_feiticos->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Renovar Feiti\303\247os", nullptr));
        tabs->setTabText(tabs->indexOf(tab_4), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Feiti\303\247os", nullptr));
        label_45->setText(QCoreApplication::translate("ifg::qt::DialogoEntidade", "Notas", nullptr));
#if QT_CONFIG(tooltip)
        texto_notas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_CONFIG(tooltip)
        tabs->setTabText(tabs->indexOf(tab_2), QCoreApplication::translate("ifg::qt::DialogoEntidade", "Notas", nullptr));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoEntidade: public Ui_DialogoEntidade {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // ENTIDADE_H
