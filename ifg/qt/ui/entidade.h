/********************************************************************************
** Form generated from reading UI file 'entidade.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTIDADE_H
#define ENTIDADE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
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
    QDialogButtonBox *botoes;
    QTabWidget *tab_tesouro;
    QWidget *tab_geral;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QSlider *slider_alfa;
    QWidget *horizontalLayoutWidget_7;
    QHBoxLayout *horizontalLayout_8;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_voadora;
    QCheckBox *checkbox_visibilidade;
    QCheckBox *checkbox_caida;
    QCheckBox *checkbox_morta;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_24;
    QLabel *label_15;
    QComboBox *combo_textura;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QLabel *label_tamanho;
    QSlider *slider_tamanho;
    QWidget *horizontalLayoutWidget_10;
    QHBoxLayout *horizontalLayout_16;
    QCheckBox *checkbox_salvacao;
    QComboBox *combo_salvacao;
    QWidget *horizontalLayoutWidget_11;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_13;
    QComboBox *combo_visao;
    QLabel *label_14;
    QDoubleSpinBox *spin_raio_visao_escuro_quad;
    QLabel *label_33;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_22;
    QLabel *label_17;
    QDoubleSpinBox *spin_tex_altura;
    QHBoxLayout *horizontalLayout_26;
    QLabel *label_19;
    QDoubleSpinBox *spin_tex_trans_x;
    QHBoxLayout *horizontalLayout_25;
    QHBoxLayout *horizontalLayout_27;
    QLabel *label_20;
    QDoubleSpinBox *spin_tex_trans_y;
    QHBoxLayout *horizontalLayout_23;
    QLabel *label_18;
    QDoubleSpinBox *spin_tex_largura;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_19;
    QLabel *label_11;
    QTableView *tabela_lista_eventos;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_12;
    QDoubleSpinBox *spin_raio_quad;
    QLabel *label_31;
    QPushButton *botao_luz;
    QWidget *horizontalLayoutWidget_9;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_16;
    QComboBox *combo_modelos_3d;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label;
    QLineEdit *campo_id;
    QLabel *label_8;
    QLineEdit *campo_rotulo;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_10;
    QPlainTextEdit *lista_rotulos;
    QPushButton *botao_adicionar_evento;
    QPushButton *botao_remover_evento;
    QWidget *layoutWidget_6;
    QHBoxLayout *horizontalLayout_35;
    QLabel *label_68;
    QListWidget *lista_formas_alternativas;
    QPushButton *botao_adicionar_forma_alternativa;
    QPushButton *botao_remover_forma_alternativa;
    QWidget *horizontalLayoutWidget_15;
    QHBoxLayout *horizontalLayout_9;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_4;
    QDoubleSpinBox *spin_aura_quad;
    QLabel *label_35;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_7;
    QDoubleSpinBox *spin_translacao_quad;
    QLabel *label_34;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_5;
    QSpinBox *spin_pontos_vida;
    QLabel *label_6;
    QSpinBox *spin_max_pontos_vida;
    QLabel *label_26;
    QPushButton *botao_bonus_pv_temporario;
    QLabel *label_79;
    QSpinBox *spin_dano_nao_letal;
    QWidget *tab_nivel;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *horizontalLayout_31;
    QLabel *label_39;
    QLineEdit *linha_nivel;
    QPushButton *botao_adicionar_nivel;
    QListWidget *lista_niveis;
    QPushButton *botao_remover_nivel;
    QWidget *horizontalLayoutWidget_12;
    QHBoxLayout *horizontalLayout_33;
    QLabel *label_46;
    QLineEdit *linha_bba;
    QGroupBox *groupBox_2;
    QWidget *horizontalLayoutWidget;
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
    QLabel *label_42;
    QSpinBox *spin_nivel_conjurador;
    QSpacerItem *horizontalSpacer_8;
    QLabel *label_44;
    QComboBox *combo_mod_conjuracao;
    QLabel *label_mod_conjuracao;
    QSpacerItem *horizontalSpacer_9;
    QLabel *label_64;
    QComboBox *combo_salvacoes_fortes;
    QWidget *gridLayoutWidget_6;
    QGridLayout *gridLayout_6;
    QLabel *label_75;
    QSlider *slider_ordem_caos;
    QSlider *slider_bem_mal;
    QLabel *label_76;
    QLabel *label_77;
    QLabel *label_78;
    QWidget *horizontalLayoutWidget_14;
    QHBoxLayout *horizontalLayout_37;
    QLabel *label_80;
    QSpinBox *spin_niveis_negativos;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_38;
    QLabel *label_84;
    QSpinBox *spin_xp;
    QWidget *tab_3;
    QLabel *label_9;
    QPushButton *botao_remover_talento;
    QPushButton *botao_adicionar_talento;
    QTableView *tabela_talentos;
    QLabel *label_81;
    QTableView *tabela_pericias;
    QWidget *tab_estatisticas;
    QPushButton *botao_remover_ataque;
    QListWidget *lista_ataques;
    QCheckBox *checkbox_imune_critico;
    QPushButton *botao_ataque_cima;
    QPushButton *botao_ataque_baixo;
    QPushButton *botao_clonar_ataque;
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_29;
    QLabel *label_36;
    QLineEdit *linha_rotulo_ataque;
    QLabel *label_21;
    QComboBox *combo_tipo_ataque;
    QComboBox *combo_arma;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *checkbox_op;
    QLabel *label_23;
    QSpinBox *spin_bonus_magico;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_29;
    QSpinBox *spin_alcance_quad;
    QSpacerItem *horizontalSpacer_10;
    QLabel *label_30;
    QLabel *label_32;
    QSpinBox *spin_incrementos;
    QSpacerItem *horizontalSpacer_6;
    QLabel *label_82;
    QSpinBox *spin_municao;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_83;
    QSpinBox *spin_ordem_ataque;
    QSpacerItem *horizontalSpacer_11;
    QLabel *label_55;
    QPushButton *botao_bonus_ataque;
    QHBoxLayout *horizontalLayout_34;
    QLabel *label_24;
    QLineEdit *linha_dano;
    QLabel *label_73;
    QComboBox *combo_empunhadura;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_72;
    QPushButton *botao_bonus_dano;
    QLabel *label_38;
    QLineEdit *linha_furtivo;
    QGroupBox *groupBox_3;
    QLabel *label_25;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QLabel *label_52;
    QLabel *label_57;
    QSpinBox *spin_ca_escudo_melhoria;
    QSpinBox *spin_ca_armadura_melhoria;
    QLabel *label_58;
    QLabel *label_59;
    QLabel *label_53;
    QLabel *label_60;
    QLabel *label_61;
    QLabel *label_ca_surpreso;
    QPushButton *botao_bonus_ca;
    QLabel *label_ca_toque;
    QComboBox *combo_armadura;
    QComboBox *combo_escudo;
    QLabel *spin_bonus_escudo;
    QLabel *spin_bonus_escudo_2;
    QGroupBox *groupBox_4;
    QWidget *gridLayoutWidget_2;
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
    QGroupBox *groupBox_5;
    QWidget *gridLayoutWidget_4;
    QGridLayout *gridLayout_4;
    QLabel *label_65;
    QLabel *label_66;
    QPushButton *botao_bonus_salvacao_fortitude;
    QPushButton *botao_bonus_salvacao_vontade;
    QLabel *label_67;
    QPushButton *botao_bonus_salvacao_reflexo;
    QLabel *label_70;
    QGroupBox *groupBox_6;
    QWidget *gridLayoutWidget_5;
    QGridLayout *gridLayout_5;
    QLabel *label_bba_agarrar;
    QLabel *label_69;
    QLabel *label_54;
    QLabel *label_bba_base;
    QLabel *label_56;
    QLabel *label_71;
    QLabel *label_bba_cac;
    QLabel *label_bba_distancia;
    QGroupBox *groupBox_7;
    QWidget *layoutWidget_3;
    QHBoxLayout *horizontalLayout_28;
    QLabel *label_22;
    QPushButton *botao_bonus_iniciativa;
    QCheckBox *checkbox_iniciativa;
    QSpinBox *spin_iniciativa;
    QWidget *horizontalLayoutWidget_8;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_85;
    QSpinBox *spin_rm;
    QWidget *tab;
    QWidget *layoutWidget_4;
    QHBoxLayout *horizontalLayout_30;
    QLabel *label_37;
    QPlainTextEdit *lista_tesouro;
    QWidget *layoutWidget_7;
    QHBoxLayout *horizontalLayout_36;
    QLabel *label_74;
    QListWidget *lista_pocoes;
    QPushButton *botao_adicionar_pocao;
    QPushButton *botao_remover_pocao;
    QWidget *tab_4;
    QTreeWidget *arvore_feiticos;
    QWidget *tab_2;
    QWidget *layoutWidget_5;
    QHBoxLayout *horizontalLayout_32;
    QLabel *label_45;
    QPlainTextEdit *texto_notas;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QStringLiteral("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(1070, 632);
        ifg__qt__DialogoEntidade->setStyleSheet(QLatin1String("QGroupBox {\n"
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
        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setGeometry(QRect(720, 570, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tab_tesouro = new QTabWidget(ifg__qt__DialogoEntidade);
        tab_tesouro->setObjectName(QStringLiteral("tab_tesouro"));
        tab_tesouro->setGeometry(QRect(10, 20, 1051, 541));
        tab_tesouro->setStyleSheet(QStringLiteral(""));
        tab_geral = new QWidget();
        tab_geral->setObjectName(QStringLiteral("tab_geral"));
        horizontalLayoutWidget_2 = new QWidget(tab_geral);
        horizontalLayoutWidget_2->setObjectName(QStringLiteral("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(570, 160, 421, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_3 = new QLabel(horizontalLayoutWidget_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_cor->setObjectName(QStringLiteral("checkbox_cor"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy1);

        horizontalLayout_7->addWidget(checkbox_cor);


        horizontalLayout_2->addLayout(horizontalLayout_7);

        botao_cor = new QPushButton(horizontalLayoutWidget_2);
        botao_cor->setObjectName(QStringLiteral("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        slider_alfa = new QSlider(horizontalLayoutWidget_2);
        slider_alfa->setObjectName(QStringLiteral("slider_alfa"));
        sizePolicy1.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy1);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(slider_alfa);

        horizontalLayoutWidget_7 = new QWidget(tab_geral);
        horizontalLayoutWidget_7->setObjectName(QStringLiteral("horizontalLayoutWidget_7"));
        horizontalLayoutWidget_7->setGeometry(QRect(10, 250, 471, 41));
        horizontalLayout_8 = new QHBoxLayout(horizontalLayoutWidget_7);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        checkbox_selecionavel = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_selecionavel->setObjectName(QStringLiteral("checkbox_selecionavel"));

        horizontalLayout_8->addWidget(checkbox_selecionavel);

        checkbox_voadora = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_voadora->setObjectName(QStringLiteral("checkbox_voadora"));

        horizontalLayout_8->addWidget(checkbox_voadora);

        checkbox_visibilidade = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_visibilidade->setObjectName(QStringLiteral("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);

        horizontalLayout_8->addWidget(checkbox_visibilidade);

        checkbox_caida = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_caida->setObjectName(QStringLiteral("checkbox_caida"));

        horizontalLayout_8->addWidget(checkbox_caida);

        checkbox_morta = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_morta->setObjectName(QStringLiteral("checkbox_morta"));

        horizontalLayout_8->addWidget(checkbox_morta);

        layoutWidget = new QWidget(tab_geral);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(570, 240, 153, 78));
        horizontalLayout_24 = new QHBoxLayout(layoutWidget);
        horizontalLayout_24->setObjectName(QStringLiteral("horizontalLayout_24"));
        horizontalLayout_24->setContentsMargins(0, 0, 0, 0);
        label_15 = new QLabel(layoutWidget);
        label_15->setObjectName(QStringLiteral("label_15"));
        sizePolicy1.setHeightForWidth(label_15->sizePolicy().hasHeightForWidth());
        label_15->setSizePolicy(sizePolicy1);
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_24->addWidget(label_15);

        combo_textura = new QComboBox(layoutWidget);
        combo_textura->setObjectName(QStringLiteral("combo_textura"));

        horizontalLayout_24->addWidget(combo_textura);

        horizontalLayoutWidget_4 = new QWidget(tab_geral);
        horizontalLayoutWidget_4->setObjectName(QStringLiteral("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(10, 170, 471, 41));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_2 = new QLabel(horizontalLayoutWidget_4);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_2);

        label_tamanho = new QLabel(horizontalLayoutWidget_4);
        label_tamanho->setObjectName(QStringLiteral("label_tamanho"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_tamanho->sizePolicy().hasHeightForWidth());
        label_tamanho->setSizePolicy(sizePolicy3);
        label_tamanho->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_tamanho);


        horizontalLayout_4->addLayout(horizontalLayout_5);

        slider_tamanho = new QSlider(horizontalLayoutWidget_4);
        slider_tamanho->setObjectName(QStringLiteral("slider_tamanho"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(slider_tamanho->sizePolicy().hasHeightForWidth());
        slider_tamanho->setSizePolicy(sizePolicy4);
        slider_tamanho->setMaximum(8);
        slider_tamanho->setPageStep(2);
        slider_tamanho->setSliderPosition(4);
        slider_tamanho->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(slider_tamanho);

        horizontalLayoutWidget_10 = new QWidget(tab_geral);
        horizontalLayoutWidget_10->setObjectName(QStringLiteral("horizontalLayoutWidget_10"));
        horizontalLayoutWidget_10->setGeometry(QRect(10, 440, 361, 35));
        horizontalLayout_16 = new QHBoxLayout(horizontalLayoutWidget_10);
        horizontalLayout_16->setObjectName(QStringLiteral("horizontalLayout_16"));
        horizontalLayout_16->setContentsMargins(0, 0, 0, 0);
        checkbox_salvacao = new QCheckBox(horizontalLayoutWidget_10);
        checkbox_salvacao->setObjectName(QStringLiteral("checkbox_salvacao"));

        horizontalLayout_16->addWidget(checkbox_salvacao);

        combo_salvacao = new QComboBox(horizontalLayoutWidget_10);
        combo_salvacao->setObjectName(QStringLiteral("combo_salvacao"));

        horizontalLayout_16->addWidget(combo_salvacao);

        horizontalLayoutWidget_11 = new QWidget(tab_geral);
        horizontalLayoutWidget_11->setObjectName(QStringLiteral("horizontalLayoutWidget_11"));
        horizontalLayoutWidget_11->setGeometry(QRect(10, 330, 471, 35));
        horizontalLayout_20 = new QHBoxLayout(horizontalLayoutWidget_11);
        horizontalLayout_20->setObjectName(QStringLiteral("horizontalLayout_20"));
        horizontalLayout_20->setContentsMargins(0, 0, 0, 0);
        label_13 = new QLabel(horizontalLayoutWidget_11);
        label_13->setObjectName(QStringLiteral("label_13"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_13->sizePolicy().hasHeightForWidth());
        label_13->setSizePolicy(sizePolicy5);
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_13);

        combo_visao = new QComboBox(horizontalLayoutWidget_11);
        combo_visao->setObjectName(QStringLiteral("combo_visao"));

        horizontalLayout_20->addWidget(combo_visao);

        label_14 = new QLabel(horizontalLayoutWidget_11);
        label_14->setObjectName(QStringLiteral("label_14"));
        sizePolicy5.setHeightForWidth(label_14->sizePolicy().hasHeightForWidth());
        label_14->setSizePolicy(sizePolicy5);
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_14);

        spin_raio_visao_escuro_quad = new QDoubleSpinBox(horizontalLayoutWidget_11);
        spin_raio_visao_escuro_quad->setObjectName(QStringLiteral("spin_raio_visao_escuro_quad"));
        spin_raio_visao_escuro_quad->setDecimals(1);
        spin_raio_visao_escuro_quad->setSingleStep(1);

        horizontalLayout_20->addWidget(spin_raio_visao_escuro_quad);

        label_33 = new QLabel(horizontalLayoutWidget_11);
        label_33->setObjectName(QStringLiteral("label_33"));
        sizePolicy5.setHeightForWidth(label_33->sizePolicy().hasHeightForWidth());
        label_33->setSizePolicy(sizePolicy5);
        label_33->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_33);

        gridLayoutWidget = new QWidget(tab_geral);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(710, 240, 281, 81));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_22 = new QHBoxLayout();
        horizontalLayout_22->setObjectName(QStringLiteral("horizontalLayout_22"));
        label_17 = new QLabel(gridLayoutWidget);
        label_17->setObjectName(QStringLiteral("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_22->addWidget(label_17);

        spin_tex_altura = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_altura->setObjectName(QStringLiteral("spin_tex_altura"));
        spin_tex_altura->setDecimals(2);
        spin_tex_altura->setMaximum(1);
        spin_tex_altura->setSingleStep(0.1);

        horizontalLayout_22->addWidget(spin_tex_altura);


        gridLayout->addLayout(horizontalLayout_22, 1, 1, 1, 1);

        horizontalLayout_26 = new QHBoxLayout();
        horizontalLayout_26->setObjectName(QStringLiteral("horizontalLayout_26"));
        label_19 = new QLabel(gridLayoutWidget);
        label_19->setObjectName(QStringLiteral("label_19"));
        label_19->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_26->addWidget(label_19);

        spin_tex_trans_x = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_trans_x->setObjectName(QStringLiteral("spin_tex_trans_x"));
        spin_tex_trans_x->setDecimals(2);
        spin_tex_trans_x->setMinimum(-1);
        spin_tex_trans_x->setMaximum(1);
        spin_tex_trans_x->setSingleStep(0.1);

        horizontalLayout_26->addWidget(spin_tex_trans_x);


        gridLayout->addLayout(horizontalLayout_26, 0, 0, 1, 1);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName(QStringLiteral("horizontalLayout_25"));
        horizontalLayout_27 = new QHBoxLayout();
        horizontalLayout_27->setObjectName(QStringLiteral("horizontalLayout_27"));
        label_20 = new QLabel(gridLayoutWidget);
        label_20->setObjectName(QStringLiteral("label_20"));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_27->addWidget(label_20);

        spin_tex_trans_y = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_trans_y->setObjectName(QStringLiteral("spin_tex_trans_y"));
        spin_tex_trans_y->setDecimals(2);
        spin_tex_trans_y->setMinimum(-1);
        spin_tex_trans_y->setMaximum(1);
        spin_tex_trans_y->setSingleStep(0.1);

        horizontalLayout_27->addWidget(spin_tex_trans_y);


        horizontalLayout_25->addLayout(horizontalLayout_27);


        gridLayout->addLayout(horizontalLayout_25, 1, 0, 1, 1);

        horizontalLayout_23 = new QHBoxLayout();
        horizontalLayout_23->setObjectName(QStringLiteral("horizontalLayout_23"));
        label_18 = new QLabel(gridLayoutWidget);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_23->addWidget(label_18);

        spin_tex_largura = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_largura->setObjectName(QStringLiteral("spin_tex_largura"));
        spin_tex_largura->setDecimals(2);
        spin_tex_largura->setMaximum(1);
        spin_tex_largura->setSingleStep(0.1);

        horizontalLayout_23->addWidget(spin_tex_largura);


        gridLayout->addLayout(horizontalLayout_23, 0, 1, 1, 1);

        layoutWidget_2 = new QWidget(tab_geral);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(570, 10, 421, 141));
        horizontalLayout_19 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_19->setObjectName(QStringLiteral("horizontalLayout_19"));
        horizontalLayout_19->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(layoutWidget_2);
        label_11->setObjectName(QStringLiteral("label_11"));
        sizePolicy5.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy5);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_19->addWidget(label_11);

        tabela_lista_eventos = new QTableView(layoutWidget_2);
        tabela_lista_eventos->setObjectName(QStringLiteral("tabela_lista_eventos"));

        horizontalLayout_19->addWidget(tabela_lista_eventos);

        horizontalLayoutWidget_3 = new QWidget(tab_geral);
        horizontalLayoutWidget_3->setObjectName(QStringLiteral("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(570, 200, 421, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        label_12 = new QLabel(horizontalLayoutWidget_3);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(label_12);

        spin_raio_quad = new QDoubleSpinBox(horizontalLayoutWidget_3);
        spin_raio_quad->setObjectName(QStringLiteral("spin_raio_quad"));
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1);

        horizontalLayout_14->addWidget(spin_raio_quad);

        label_31 = new QLabel(horizontalLayoutWidget_3);
        label_31->setObjectName(QStringLiteral("label_31"));
        sizePolicy5.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy5);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(label_31);

        botao_luz = new QPushButton(horizontalLayoutWidget_3);
        botao_luz->setObjectName(QStringLiteral("botao_luz"));
        botao_luz->setStyleSheet(QStringLiteral(""));

        horizontalLayout_14->addWidget(botao_luz);


        horizontalLayout_3->addLayout(horizontalLayout_14);

        horizontalLayoutWidget_9 = new QWidget(tab_geral);
        horizontalLayoutWidget_9->setObjectName(QStringLiteral("horizontalLayoutWidget_9"));
        horizontalLayoutWidget_9->setGeometry(QRect(570, 320, 421, 41));
        horizontalLayout_21 = new QHBoxLayout(horizontalLayoutWidget_9);
        horizontalLayout_21->setObjectName(QStringLiteral("horizontalLayout_21"));
        horizontalLayout_21->setContentsMargins(0, 0, 0, 0);
        label_16 = new QLabel(horizontalLayoutWidget_9);
        label_16->setObjectName(QStringLiteral("label_16"));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_16);

        combo_modelos_3d = new QComboBox(horizontalLayoutWidget_9);
        combo_modelos_3d->setObjectName(QStringLiteral("combo_modelos_3d"));

        horizontalLayout_21->addWidget(combo_modelos_3d);

        verticalLayoutWidget = new QWidget(tab_geral);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 471, 161));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout_15->addWidget(label);

        campo_id = new QLineEdit(verticalLayoutWidget);
        campo_id->setObjectName(QStringLiteral("campo_id"));
        sizePolicy1.setHeightForWidth(campo_id->sizePolicy().hasHeightForWidth());
        campo_id->setSizePolicy(sizePolicy1);
        campo_id->setMinimumSize(QSize(0, 0));
        campo_id->setMaximumSize(QSize(70, 16777215));
        campo_id->setReadOnly(true);

        horizontalLayout_15->addWidget(campo_id);

        label_8 = new QLabel(verticalLayoutWidget);
        label_8->setObjectName(QStringLiteral("label_8"));
        QSizePolicy sizePolicy6(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy6);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_8);

        campo_rotulo = new QLineEdit(verticalLayoutWidget);
        campo_rotulo->setObjectName(QStringLiteral("campo_rotulo"));
        sizePolicy4.setHeightForWidth(campo_rotulo->sizePolicy().hasHeightForWidth());
        campo_rotulo->setSizePolicy(sizePolicy4);
        campo_rotulo->setReadOnly(false);

        horizontalLayout_15->addWidget(campo_rotulo);


        verticalLayout->addLayout(horizontalLayout_15);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QStringLiteral("horizontalLayout_18"));
        label_10 = new QLabel(verticalLayoutWidget);
        label_10->setObjectName(QStringLiteral("label_10"));
        sizePolicy3.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy3);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_10);

        lista_rotulos = new QPlainTextEdit(verticalLayoutWidget);
        lista_rotulos->setObjectName(QStringLiteral("lista_rotulos"));

        horizontalLayout_18->addWidget(lista_rotulos);


        verticalLayout->addLayout(horizontalLayout_18);

        botao_adicionar_evento = new QPushButton(tab_geral);
        botao_adicionar_evento->setObjectName(QStringLiteral("botao_adicionar_evento"));
        botao_adicionar_evento->setGeometry(QRect(1000, 40, 31, 27));
        botao_remover_evento = new QPushButton(tab_geral);
        botao_remover_evento->setObjectName(QStringLiteral("botao_remover_evento"));
        botao_remover_evento->setGeometry(QRect(1000, 70, 31, 27));
        layoutWidget_6 = new QWidget(tab_geral);
        layoutWidget_6->setObjectName(QStringLiteral("layoutWidget_6"));
        layoutWidget_6->setGeometry(QRect(570, 360, 421, 121));
        horizontalLayout_35 = new QHBoxLayout(layoutWidget_6);
        horizontalLayout_35->setObjectName(QStringLiteral("horizontalLayout_35"));
        horizontalLayout_35->setContentsMargins(0, 0, 0, 0);
        label_68 = new QLabel(layoutWidget_6);
        label_68->setObjectName(QStringLiteral("label_68"));
        sizePolicy5.setHeightForWidth(label_68->sizePolicy().hasHeightForWidth());
        label_68->setSizePolicy(sizePolicy5);
        label_68->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_35->addWidget(label_68);

        lista_formas_alternativas = new QListWidget(layoutWidget_6);
        lista_formas_alternativas->setObjectName(QStringLiteral("lista_formas_alternativas"));

        horizontalLayout_35->addWidget(lista_formas_alternativas);

        botao_adicionar_forma_alternativa = new QPushButton(tab_geral);
        botao_adicionar_forma_alternativa->setObjectName(QStringLiteral("botao_adicionar_forma_alternativa"));
        botao_adicionar_forma_alternativa->setGeometry(QRect(1000, 390, 31, 27));
        botao_remover_forma_alternativa = new QPushButton(tab_geral);
        botao_remover_forma_alternativa->setObjectName(QStringLiteral("botao_remover_forma_alternativa"));
        botao_remover_forma_alternativa->setGeometry(QRect(1000, 420, 31, 27));
        horizontalLayoutWidget_15 = new QWidget(tab_geral);
        horizontalLayoutWidget_15->setObjectName(QStringLiteral("horizontalLayoutWidget_15"));
        horizontalLayoutWidget_15->setGeometry(QRect(10, 290, 471, 41));
        horizontalLayout_9 = new QHBoxLayout(horizontalLayoutWidget_15);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        label_4 = new QLabel(horizontalLayoutWidget_15);
        label_4->setObjectName(QStringLiteral("label_4"));
        sizePolicy5.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy5);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_4);

        spin_aura_quad = new QDoubleSpinBox(horizontalLayoutWidget_15);
        spin_aura_quad->setObjectName(QStringLiteral("spin_aura_quad"));
        spin_aura_quad->setDecimals(1);
        spin_aura_quad->setSingleStep(1);

        horizontalLayout_10->addWidget(spin_aura_quad);

        label_35 = new QLabel(horizontalLayoutWidget_15);
        label_35->setObjectName(QStringLiteral("label_35"));
        sizePolicy5.setHeightForWidth(label_35->sizePolicy().hasHeightForWidth());
        label_35->setSizePolicy(sizePolicy5);
        label_35->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_35);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        label_7 = new QLabel(horizontalLayoutWidget_15);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_7);

        spin_translacao_quad = new QDoubleSpinBox(horizontalLayoutWidget_15);
        spin_translacao_quad->setObjectName(QStringLiteral("spin_translacao_quad"));
        sizePolicy1.setHeightForWidth(spin_translacao_quad->sizePolicy().hasHeightForWidth());
        spin_translacao_quad->setSizePolicy(sizePolicy1);
        spin_translacao_quad->setDecimals(1);
        spin_translacao_quad->setMinimum(-100);
        spin_translacao_quad->setMaximum(100);
        spin_translacao_quad->setSingleStep(0.1);

        horizontalLayout_13->addWidget(spin_translacao_quad);

        label_34 = new QLabel(horizontalLayoutWidget_15);
        label_34->setObjectName(QStringLiteral("label_34"));
        sizePolicy5.setHeightForWidth(label_34->sizePolicy().hasHeightForWidth());
        label_34->setSizePolicy(sizePolicy5);
        label_34->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_34);


        horizontalLayout_9->addLayout(horizontalLayout_13);

        layoutWidget1 = new QWidget(tab_geral);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 210, 471, 29));
        horizontalLayout_12 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
        horizontalLayout_12->setContentsMargins(0, 0, 0, 0);
        label_5 = new QLabel(layoutWidget1);
        label_5->setObjectName(QStringLiteral("label_5"));
        sizePolicy5.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy5);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_5);

        spin_pontos_vida = new QSpinBox(layoutWidget1);
        spin_pontos_vida->setObjectName(QStringLiteral("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_pontos_vida);

        label_6 = new QLabel(layoutWidget1);
        label_6->setObjectName(QStringLiteral("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_6);

        spin_max_pontos_vida = new QSpinBox(layoutWidget1);
        spin_max_pontos_vida->setObjectName(QStringLiteral("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_max_pontos_vida);

        label_26 = new QLabel(layoutWidget1);
        label_26->setObjectName(QStringLiteral("label_26"));
        sizePolicy.setHeightForWidth(label_26->sizePolicy().hasHeightForWidth());
        label_26->setSizePolicy(sizePolicy);
        label_26->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_26);

        botao_bonus_pv_temporario = new QPushButton(layoutWidget1);
        botao_bonus_pv_temporario->setObjectName(QStringLiteral("botao_bonus_pv_temporario"));
        sizePolicy1.setHeightForWidth(botao_bonus_pv_temporario->sizePolicy().hasHeightForWidth());
        botao_bonus_pv_temporario->setSizePolicy(sizePolicy1);
        botao_bonus_pv_temporario->setMinimumSize(QSize(0, 0));
        botao_bonus_pv_temporario->setMaximumSize(QSize(40, 16777215));

        horizontalLayout_12->addWidget(botao_bonus_pv_temporario);

        label_79 = new QLabel(layoutWidget1);
        label_79->setObjectName(QStringLiteral("label_79"));
        sizePolicy.setHeightForWidth(label_79->sizePolicy().hasHeightForWidth());
        label_79->setSizePolicy(sizePolicy);
        label_79->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_79);

        spin_dano_nao_letal = new QSpinBox(layoutWidget1);
        spin_dano_nao_letal->setObjectName(QStringLiteral("spin_dano_nao_letal"));
        sizePolicy1.setHeightForWidth(spin_dano_nao_letal->sizePolicy().hasHeightForWidth());
        spin_dano_nao_letal->setSizePolicy(sizePolicy1);
        spin_dano_nao_letal->setMinimum(0);
        spin_dano_nao_letal->setMaximum(999);

        horizontalLayout_12->addWidget(spin_dano_nao_letal);

        tab_tesouro->addTab(tab_geral, QString());
        tab_nivel = new QWidget();
        tab_nivel->setObjectName(QStringLiteral("tab_nivel"));
        horizontalLayoutWidget_5 = new QWidget(tab_nivel);
        horizontalLayoutWidget_5->setObjectName(QStringLiteral("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(10, 10, 181, 41));
        horizontalLayout_31 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_31->setObjectName(QStringLiteral("horizontalLayout_31"));
        horizontalLayout_31->setContentsMargins(0, 0, 0, 0);
        label_39 = new QLabel(horizontalLayoutWidget_5);
        label_39->setObjectName(QStringLiteral("label_39"));

        horizontalLayout_31->addWidget(label_39, 0, Qt::AlignRight);

        linha_nivel = new QLineEdit(horizontalLayoutWidget_5);
        linha_nivel->setObjectName(QStringLiteral("linha_nivel"));
        linha_nivel->setReadOnly(true);

        horizontalLayout_31->addWidget(linha_nivel, 0, Qt::AlignLeft);

        botao_adicionar_nivel = new QPushButton(tab_nivel);
        botao_adicionar_nivel->setObjectName(QStringLiteral("botao_adicionar_nivel"));
        botao_adicionar_nivel->setEnabled(true);
        botao_adicionar_nivel->setGeometry(QRect(190, 350, 121, 27));
        lista_niveis = new QListWidget(tab_nivel);
        lista_niveis->setObjectName(QStringLiteral("lista_niveis"));
        lista_niveis->setGeometry(QRect(10, 160, 451, 181));
        botao_remover_nivel = new QPushButton(tab_nivel);
        botao_remover_nivel->setObjectName(QStringLiteral("botao_remover_nivel"));
        botao_remover_nivel->setEnabled(false);
        botao_remover_nivel->setGeometry(QRect(340, 350, 121, 27));
        horizontalLayoutWidget_12 = new QWidget(tab_nivel);
        horizontalLayoutWidget_12->setObjectName(QStringLiteral("horizontalLayoutWidget_12"));
        horizontalLayoutWidget_12->setGeometry(QRect(210, 10, 181, 41));
        horizontalLayout_33 = new QHBoxLayout(horizontalLayoutWidget_12);
        horizontalLayout_33->setObjectName(QStringLiteral("horizontalLayout_33"));
        horizontalLayout_33->setContentsMargins(0, 0, 0, 0);
        label_46 = new QLabel(horizontalLayoutWidget_12);
        label_46->setObjectName(QStringLiteral("label_46"));

        horizontalLayout_33->addWidget(label_46, 0, Qt::AlignRight);

        linha_bba = new QLineEdit(horizontalLayoutWidget_12);
        linha_bba->setObjectName(QStringLiteral("linha_bba"));
        linha_bba->setReadOnly(true);

        horizontalLayout_33->addWidget(linha_bba, 0, Qt::AlignLeft);

        groupBox_2 = new QGroupBox(tab_nivel);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 90, 951, 61));
        horizontalLayoutWidget = new QWidget(groupBox_2);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(9, 20, 941, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_40 = new QLabel(horizontalLayoutWidget);
        label_40->setObjectName(QStringLiteral("label_40"));
        sizePolicy5.setHeightForWidth(label_40->sizePolicy().hasHeightForWidth());
        label_40->setSizePolicy(sizePolicy5);

        horizontalLayout->addWidget(label_40);

        combo_classe = new QComboBox(horizontalLayoutWidget);
        combo_classe->setObjectName(QStringLiteral("combo_classe"));

        horizontalLayout->addWidget(combo_classe);

        linha_classe = new QLineEdit(horizontalLayoutWidget);
        linha_classe->setObjectName(QStringLiteral("linha_classe"));
        sizePolicy1.setHeightForWidth(linha_classe->sizePolicy().hasHeightForWidth());
        linha_classe->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(linha_classe);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_41 = new QLabel(horizontalLayoutWidget);
        label_41->setObjectName(QStringLiteral("label_41"));
        label_41->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_41);

        spin_nivel_classe = new QSpinBox(horizontalLayoutWidget);
        spin_nivel_classe->setObjectName(QStringLiteral("spin_nivel_classe"));
        spin_nivel_classe->setMinimum(1);

        horizontalLayout->addWidget(spin_nivel_classe);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        label_43 = new QLabel(horizontalLayoutWidget);
        label_43->setObjectName(QStringLiteral("label_43"));
        label_43->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_43);

        spin_bba = new QSpinBox(horizontalLayoutWidget);
        spin_bba->setObjectName(QStringLiteral("spin_bba"));
        spin_bba->setMinimum(-1);

        horizontalLayout->addWidget(spin_bba);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_7);

        label_42 = new QLabel(horizontalLayoutWidget);
        label_42->setObjectName(QStringLiteral("label_42"));
        label_42->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_42);

        spin_nivel_conjurador = new QSpinBox(horizontalLayoutWidget);
        spin_nivel_conjurador->setObjectName(QStringLiteral("spin_nivel_conjurador"));
        spin_nivel_conjurador->setMinimum(-1);

        horizontalLayout->addWidget(spin_nivel_conjurador);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_8);

        label_44 = new QLabel(horizontalLayoutWidget);
        label_44->setObjectName(QStringLiteral("label_44"));
        label_44->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_44);

        combo_mod_conjuracao = new QComboBox(horizontalLayoutWidget);
        combo_mod_conjuracao->setObjectName(QStringLiteral("combo_mod_conjuracao"));

        horizontalLayout->addWidget(combo_mod_conjuracao);

        label_mod_conjuracao = new QLabel(horizontalLayoutWidget);
        label_mod_conjuracao->setObjectName(QStringLiteral("label_mod_conjuracao"));
        sizePolicy5.setHeightForWidth(label_mod_conjuracao->sizePolicy().hasHeightForWidth());
        label_mod_conjuracao->setSizePolicy(sizePolicy5);
        label_mod_conjuracao->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_mod_conjuracao);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_9);

        label_64 = new QLabel(horizontalLayoutWidget);
        label_64->setObjectName(QStringLiteral("label_64"));
        label_64->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_64);

        combo_salvacoes_fortes = new QComboBox(horizontalLayoutWidget);
        combo_salvacoes_fortes->setObjectName(QStringLiteral("combo_salvacoes_fortes"));

        horizontalLayout->addWidget(combo_salvacoes_fortes);

        gridLayoutWidget_6 = new QWidget(tab_nivel);
        gridLayoutWidget_6->setObjectName(QStringLiteral("gridLayoutWidget_6"));
        gridLayoutWidget_6->setGeometry(QRect(420, 10, 211, 71));
        gridLayout_6 = new QGridLayout(gridLayoutWidget_6);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        gridLayout_6->setContentsMargins(0, 0, 0, 0);
        label_75 = new QLabel(gridLayoutWidget_6);
        label_75->setObjectName(QStringLiteral("label_75"));

        gridLayout_6->addWidget(label_75, 0, 0, 1, 1);

        slider_ordem_caos = new QSlider(gridLayoutWidget_6);
        slider_ordem_caos->setObjectName(QStringLiteral("slider_ordem_caos"));
        slider_ordem_caos->setMaximum(8);
        slider_ordem_caos->setOrientation(Qt::Horizontal);

        gridLayout_6->addWidget(slider_ordem_caos, 1, 1, 1, 1);

        slider_bem_mal = new QSlider(gridLayoutWidget_6);
        slider_bem_mal->setObjectName(QStringLiteral("slider_bem_mal"));
        slider_bem_mal->setMaximum(8);
        slider_bem_mal->setOrientation(Qt::Horizontal);

        gridLayout_6->addWidget(slider_bem_mal, 0, 1, 1, 1);

        label_76 = new QLabel(gridLayoutWidget_6);
        label_76->setObjectName(QStringLiteral("label_76"));

        gridLayout_6->addWidget(label_76, 1, 0, 1, 1);

        label_77 = new QLabel(gridLayoutWidget_6);
        label_77->setObjectName(QStringLiteral("label_77"));

        gridLayout_6->addWidget(label_77, 1, 2, 1, 1);

        label_78 = new QLabel(gridLayoutWidget_6);
        label_78->setObjectName(QStringLiteral("label_78"));

        gridLayout_6->addWidget(label_78, 0, 2, 1, 1);

        horizontalLayoutWidget_14 = new QWidget(tab_nivel);
        horizontalLayoutWidget_14->setObjectName(QStringLiteral("horizontalLayoutWidget_14"));
        horizontalLayoutWidget_14->setGeometry(QRect(650, 10, 261, 31));
        horizontalLayout_37 = new QHBoxLayout(horizontalLayoutWidget_14);
        horizontalLayout_37->setObjectName(QStringLiteral("horizontalLayout_37"));
        horizontalLayout_37->setContentsMargins(0, 0, 0, 0);
        label_80 = new QLabel(horizontalLayoutWidget_14);
        label_80->setObjectName(QStringLiteral("label_80"));
        label_80->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_37->addWidget(label_80);

        spin_niveis_negativos = new QSpinBox(horizontalLayoutWidget_14);
        spin_niveis_negativos->setObjectName(QStringLiteral("spin_niveis_negativos"));
        sizePolicy1.setHeightForWidth(spin_niveis_negativos->sizePolicy().hasHeightForWidth());
        spin_niveis_negativos->setSizePolicy(sizePolicy1);
        spin_niveis_negativos->setMinimum(0);
        spin_niveis_negativos->setValue(0);

        horizontalLayout_37->addWidget(spin_niveis_negativos);

        horizontalLayoutWidget_6 = new QWidget(tab_nivel);
        horizontalLayoutWidget_6->setObjectName(QStringLiteral("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(10, 50, 181, 41));
        horizontalLayout_38 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_38->setObjectName(QStringLiteral("horizontalLayout_38"));
        horizontalLayout_38->setContentsMargins(0, 0, 0, 0);
        label_84 = new QLabel(horizontalLayoutWidget_6);
        label_84->setObjectName(QStringLiteral("label_84"));

        horizontalLayout_38->addWidget(label_84, 0, Qt::AlignRight);

        spin_xp = new QSpinBox(horizontalLayoutWidget_6);
        spin_xp->setObjectName(QStringLiteral("spin_xp"));
        spin_xp->setMinimum(0);
        spin_xp->setMaximum(1000000);

        horizontalLayout_38->addWidget(spin_xp);

        tab_tesouro->addTab(tab_nivel, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        label_9 = new QLabel(tab_3);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(750, 20, 57, 17));
        botao_remover_talento = new QPushButton(tab_3);
        botao_remover_talento->setObjectName(QStringLiteral("botao_remover_talento"));
        botao_remover_talento->setGeometry(QRect(990, 80, 31, 27));
        botao_adicionar_talento = new QPushButton(tab_3);
        botao_adicionar_talento->setObjectName(QStringLiteral("botao_adicionar_talento"));
        botao_adicionar_talento->setGeometry(QRect(990, 50, 31, 27));
        tabela_talentos = new QTableView(tab_3);
        tabela_talentos->setObjectName(QStringLiteral("tabela_talentos"));
        tabela_talentos->setGeometry(QRect(550, 50, 431, 431));
        label_81 = new QLabel(tab_3);
        label_81->setObjectName(QStringLiteral("label_81"));
        label_81->setGeometry(QRect(225, 20, 57, 17));
        tabela_pericias = new QTableView(tab_3);
        tabela_pericias->setObjectName(QStringLiteral("tabela_pericias"));
        tabela_pericias->setGeometry(QRect(30, 50, 501, 431));
        tab_tesouro->addTab(tab_3, QString());
        tab_estatisticas = new QWidget();
        tab_estatisticas->setObjectName(QStringLiteral("tab_estatisticas"));
        botao_remover_ataque = new QPushButton(tab_estatisticas);
        botao_remover_ataque->setObjectName(QStringLiteral("botao_remover_ataque"));
        botao_remover_ataque->setEnabled(false);
        botao_remover_ataque->setGeometry(QRect(870, 460, 121, 27));
        lista_ataques = new QListWidget(tab_estatisticas);
        lista_ataques->setObjectName(QStringLiteral("lista_ataques"));
        lista_ataques->setGeometry(QRect(279, 320, 711, 131));
        checkbox_imune_critico = new QCheckBox(tab_estatisticas);
        checkbox_imune_critico->setObjectName(QStringLiteral("checkbox_imune_critico"));
        checkbox_imune_critico->setGeometry(QRect(20, 460, 121, 22));
        botao_ataque_cima = new QPushButton(tab_estatisticas);
        botao_ataque_cima->setObjectName(QStringLiteral("botao_ataque_cima"));
        botao_ataque_cima->setGeometry(QRect(1000, 320, 31, 27));
        botao_ataque_baixo = new QPushButton(tab_estatisticas);
        botao_ataque_baixo->setObjectName(QStringLiteral("botao_ataque_baixo"));
        botao_ataque_baixo->setGeometry(QRect(1000, 360, 31, 27));
        botao_clonar_ataque = new QPushButton(tab_estatisticas);
        botao_clonar_ataque->setObjectName(QStringLiteral("botao_clonar_ataque"));
        botao_clonar_ataque->setEnabled(true);
        botao_clonar_ataque->setGeometry(QRect(740, 460, 121, 27));
        groupBox = new QGroupBox(tab_estatisticas);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(280, 160, 761, 151));
        verticalLayoutWidget_2 = new QWidget(groupBox);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(0, 10, 761, 134));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_29 = new QHBoxLayout();
        horizontalLayout_29->setObjectName(QStringLiteral("horizontalLayout_29"));
        label_36 = new QLabel(verticalLayoutWidget_2);
        label_36->setObjectName(QStringLiteral("label_36"));
        sizePolicy5.setHeightForWidth(label_36->sizePolicy().hasHeightForWidth());
        label_36->setSizePolicy(sizePolicy5);

        horizontalLayout_29->addWidget(label_36);

        linha_rotulo_ataque = new QLineEdit(verticalLayoutWidget_2);
        linha_rotulo_ataque->setObjectName(QStringLiteral("linha_rotulo_ataque"));
        sizePolicy1.setHeightForWidth(linha_rotulo_ataque->sizePolicy().hasHeightForWidth());
        linha_rotulo_ataque->setSizePolicy(sizePolicy1);

        horizontalLayout_29->addWidget(linha_rotulo_ataque);

        label_21 = new QLabel(verticalLayoutWidget_2);
        label_21->setObjectName(QStringLiteral("label_21"));
        sizePolicy5.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy5);

        horizontalLayout_29->addWidget(label_21);

        combo_tipo_ataque = new QComboBox(verticalLayoutWidget_2);
        combo_tipo_ataque->setObjectName(QStringLiteral("combo_tipo_ataque"));
        sizePolicy4.setHeightForWidth(combo_tipo_ataque->sizePolicy().hasHeightForWidth());
        combo_tipo_ataque->setSizePolicy(sizePolicy4);

        horizontalLayout_29->addWidget(combo_tipo_ataque);

        combo_arma = new QComboBox(verticalLayoutWidget_2);
        combo_arma->setObjectName(QStringLiteral("combo_arma"));
        sizePolicy4.setHeightForWidth(combo_arma->sizePolicy().hasHeightForWidth());
        combo_arma->setSizePolicy(sizePolicy4);

        horizontalLayout_29->addWidget(combo_arma);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_29->addItem(horizontalSpacer_5);

        checkbox_op = new QCheckBox(verticalLayoutWidget_2);
        checkbox_op->setObjectName(QStringLiteral("checkbox_op"));
        sizePolicy1.setHeightForWidth(checkbox_op->sizePolicy().hasHeightForWidth());
        checkbox_op->setSizePolicy(sizePolicy1);

        horizontalLayout_29->addWidget(checkbox_op);

        label_23 = new QLabel(verticalLayoutWidget_2);
        label_23->setObjectName(QStringLiteral("label_23"));
        sizePolicy5.setHeightForWidth(label_23->sizePolicy().hasHeightForWidth());
        label_23->setSizePolicy(sizePolicy5);
        label_23->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_29->addWidget(label_23);

        spin_bonus_magico = new QSpinBox(verticalLayoutWidget_2);
        spin_bonus_magico->setObjectName(QStringLiteral("spin_bonus_magico"));
        sizePolicy1.setHeightForWidth(spin_bonus_magico->sizePolicy().hasHeightForWidth());
        spin_bonus_magico->setSizePolicy(sizePolicy1);
        spin_bonus_magico->setMinimum(-50);
        spin_bonus_magico->setMaximum(50);

        horizontalLayout_29->addWidget(spin_bonus_magico);


        verticalLayout_2->addLayout(horizontalLayout_29);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_29 = new QLabel(verticalLayoutWidget_2);
        label_29->setObjectName(QStringLiteral("label_29"));
        sizePolicy5.setHeightForWidth(label_29->sizePolicy().hasHeightForWidth());
        label_29->setSizePolicy(sizePolicy5);
        label_29->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_29);

        spin_alcance_quad = new QSpinBox(verticalLayoutWidget_2);
        spin_alcance_quad->setObjectName(QStringLiteral("spin_alcance_quad"));
        sizePolicy1.setHeightForWidth(spin_alcance_quad->sizePolicy().hasHeightForWidth());
        spin_alcance_quad->setSizePolicy(sizePolicy1);
        spin_alcance_quad->setMinimum(-1);

        horizontalLayout_6->addWidget(spin_alcance_quad);

        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_10);

        label_30 = new QLabel(verticalLayoutWidget_2);
        label_30->setObjectName(QStringLiteral("label_30"));
        sizePolicy5.setHeightForWidth(label_30->sizePolicy().hasHeightForWidth());
        label_30->setSizePolicy(sizePolicy5);
        label_30->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_30);

        label_32 = new QLabel(verticalLayoutWidget_2);
        label_32->setObjectName(QStringLiteral("label_32"));
        sizePolicy5.setHeightForWidth(label_32->sizePolicy().hasHeightForWidth());
        label_32->setSizePolicy(sizePolicy5);
        label_32->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_32);

        spin_incrementos = new QSpinBox(verticalLayoutWidget_2);
        spin_incrementos->setObjectName(QStringLiteral("spin_incrementos"));
        sizePolicy1.setHeightForWidth(spin_incrementos->sizePolicy().hasHeightForWidth());
        spin_incrementos->setSizePolicy(sizePolicy1);
        spin_incrementos->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(spin_incrementos);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_6);

        label_82 = new QLabel(verticalLayoutWidget_2);
        label_82->setObjectName(QStringLiteral("label_82"));
        sizePolicy5.setHeightForWidth(label_82->sizePolicy().hasHeightForWidth());
        label_82->setSizePolicy(sizePolicy5);
        label_82->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_82);

        spin_municao = new QSpinBox(verticalLayoutWidget_2);
        spin_municao->setObjectName(QStringLiteral("spin_municao"));
        sizePolicy1.setHeightForWidth(spin_municao->sizePolicy().hasHeightForWidth());
        spin_municao->setSizePolicy(sizePolicy1);
        spin_municao->setMaximum(10000);

        horizontalLayout_6->addWidget(spin_municao);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        label_83 = new QLabel(verticalLayoutWidget_2);
        label_83->setObjectName(QStringLiteral("label_83"));
        sizePolicy5.setHeightForWidth(label_83->sizePolicy().hasHeightForWidth());
        label_83->setSizePolicy(sizePolicy5);
        label_83->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_83);

        spin_ordem_ataque = new QSpinBox(verticalLayoutWidget_2);
        spin_ordem_ataque->setObjectName(QStringLiteral("spin_ordem_ataque"));
        sizePolicy1.setHeightForWidth(spin_ordem_ataque->sizePolicy().hasHeightForWidth());
        spin_ordem_ataque->setSizePolicy(sizePolicy1);
        spin_ordem_ataque->setMinimum(1);
        spin_ordem_ataque->setMaximum(9);

        horizontalLayout_6->addWidget(spin_ordem_ataque);

        horizontalSpacer_11 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_11);

        label_55 = new QLabel(verticalLayoutWidget_2);
        label_55->setObjectName(QStringLiteral("label_55"));
        sizePolicy5.setHeightForWidth(label_55->sizePolicy().hasHeightForWidth());
        label_55->setSizePolicy(sizePolicy5);
        label_55->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_55);

        botao_bonus_ataque = new QPushButton(verticalLayoutWidget_2);
        botao_bonus_ataque->setObjectName(QStringLiteral("botao_bonus_ataque"));
        sizePolicy1.setHeightForWidth(botao_bonus_ataque->sizePolicy().hasHeightForWidth());
        botao_bonus_ataque->setSizePolicy(sizePolicy1);

        horizontalLayout_6->addWidget(botao_bonus_ataque);


        verticalLayout_2->addLayout(horizontalLayout_6);

        horizontalLayout_34 = new QHBoxLayout();
        horizontalLayout_34->setObjectName(QStringLiteral("horizontalLayout_34"));
        label_24 = new QLabel(verticalLayoutWidget_2);
        label_24->setObjectName(QStringLiteral("label_24"));
        sizePolicy5.setHeightForWidth(label_24->sizePolicy().hasHeightForWidth());
        label_24->setSizePolicy(sizePolicy5);
        label_24->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_34->addWidget(label_24);

        linha_dano = new QLineEdit(verticalLayoutWidget_2);
        linha_dano->setObjectName(QStringLiteral("linha_dano"));
        sizePolicy1.setHeightForWidth(linha_dano->sizePolicy().hasHeightForWidth());
        linha_dano->setSizePolicy(sizePolicy1);

        horizontalLayout_34->addWidget(linha_dano);

        label_73 = new QLabel(verticalLayoutWidget_2);
        label_73->setObjectName(QStringLiteral("label_73"));
        sizePolicy5.setHeightForWidth(label_73->sizePolicy().hasHeightForWidth());
        label_73->setSizePolicy(sizePolicy5);
        label_73->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_34->addWidget(label_73);

        combo_empunhadura = new QComboBox(verticalLayoutWidget_2);
        combo_empunhadura->setObjectName(QStringLiteral("combo_empunhadura"));
        sizePolicy1.setHeightForWidth(combo_empunhadura->sizePolicy().hasHeightForWidth());
        combo_empunhadura->setSizePolicy(sizePolicy1);

        horizontalLayout_34->addWidget(combo_empunhadura);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_34->addItem(horizontalSpacer_3);

        label_72 = new QLabel(verticalLayoutWidget_2);
        label_72->setObjectName(QStringLiteral("label_72"));
        sizePolicy5.setHeightForWidth(label_72->sizePolicy().hasHeightForWidth());
        label_72->setSizePolicy(sizePolicy5);
        label_72->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_34->addWidget(label_72);

        botao_bonus_dano = new QPushButton(verticalLayoutWidget_2);
        botao_bonus_dano->setObjectName(QStringLiteral("botao_bonus_dano"));

        horizontalLayout_34->addWidget(botao_bonus_dano);


        verticalLayout_2->addLayout(horizontalLayout_34);

        label_38 = new QLabel(tab_estatisticas);
        label_38->setObjectName(QStringLiteral("label_38"));
        label_38->setGeometry(QRect(280, 460, 41, 34));
        label_38->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        linha_furtivo = new QLineEdit(tab_estatisticas);
        linha_furtivo->setObjectName(QStringLiteral("linha_furtivo"));
        linha_furtivo->setGeometry(QRect(330, 460, 129, 27));
        groupBox_3 = new QGroupBox(tab_estatisticas);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(280, 90, 511, 71));
        label_25 = new QLabel(groupBox_3);
        label_25->setObjectName(QStringLiteral("label_25"));
        label_25->setGeometry(QRect(10, 10, 31, 34));
        label_25->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        gridLayoutWidget_3 = new QWidget(groupBox_3);
        gridLayoutWidget_3->setObjectName(QStringLiteral("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(0, 10, 497, 61));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        label_52 = new QLabel(gridLayoutWidget_3);
        label_52->setObjectName(QStringLiteral("label_52"));
        label_52->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_52, 0, 1, 1, 1);

        label_57 = new QLabel(gridLayoutWidget_3);
        label_57->setObjectName(QStringLiteral("label_57"));
        QFont font;
        font.setFamily(QStringLiteral("Noto Sans [unknown]"));
        font.setBold(true);
        font.setWeight(75);
        label_57->setFont(font);
        label_57->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_57, 0, 7, 1, 1);

        spin_ca_escudo_melhoria = new QSpinBox(gridLayoutWidget_3);
        spin_ca_escudo_melhoria->setObjectName(QStringLiteral("spin_ca_escudo_melhoria"));
        spin_ca_escudo_melhoria->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_escudo_melhoria, 1, 4, 1, 1);

        spin_ca_armadura_melhoria = new QSpinBox(gridLayoutWidget_3);
        spin_ca_armadura_melhoria->setObjectName(QStringLiteral("spin_ca_armadura_melhoria"));
        spin_ca_armadura_melhoria->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_armadura_melhoria, 1, 2, 1, 1);

        label_58 = new QLabel(gridLayoutWidget_3);
        label_58->setObjectName(QStringLiteral("label_58"));
        label_58->setFont(font);
        label_58->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_58, 0, 8, 1, 1);

        label_59 = new QLabel(gridLayoutWidget_3);
        label_59->setObjectName(QStringLiteral("label_59"));
        label_59->setFont(font);
        label_59->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_59, 0, 9, 1, 1);

        label_53 = new QLabel(gridLayoutWidget_3);
        label_53->setObjectName(QStringLiteral("label_53"));
        label_53->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_53, 0, 0, 1, 1);

        label_60 = new QLabel(gridLayoutWidget_3);
        label_60->setObjectName(QStringLiteral("label_60"));
        label_60->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_60, 1, 0, 1, 1);

        label_61 = new QLabel(gridLayoutWidget_3);
        label_61->setObjectName(QStringLiteral("label_61"));
        label_61->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_61, 1, 6, 1, 1);

        label_ca_surpreso = new QLabel(gridLayoutWidget_3);
        label_ca_surpreso->setObjectName(QStringLiteral("label_ca_surpreso"));
        label_ca_surpreso->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_ca_surpreso, 1, 9, 1, 1);

        botao_bonus_ca = new QPushButton(gridLayoutWidget_3);
        botao_bonus_ca->setObjectName(QStringLiteral("botao_bonus_ca"));

        gridLayout_3->addWidget(botao_bonus_ca, 1, 7, 1, 1);

        label_ca_toque = new QLabel(gridLayoutWidget_3);
        label_ca_toque->setObjectName(QStringLiteral("label_ca_toque"));
        label_ca_toque->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_ca_toque, 1, 8, 1, 1);

        combo_armadura = new QComboBox(gridLayoutWidget_3);
        combo_armadura->setObjectName(QStringLiteral("combo_armadura"));

        gridLayout_3->addWidget(combo_armadura, 1, 1, 1, 1);

        combo_escudo = new QComboBox(gridLayoutWidget_3);
        combo_escudo->setObjectName(QStringLiteral("combo_escudo"));

        gridLayout_3->addWidget(combo_escudo, 1, 3, 1, 1);

        spin_bonus_escudo = new QLabel(gridLayoutWidget_3);
        spin_bonus_escudo->setObjectName(QStringLiteral("spin_bonus_escudo"));
        spin_bonus_escudo->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(spin_bonus_escudo, 0, 3, 1, 1);

        spin_bonus_escudo_2 = new QLabel(gridLayoutWidget_3);
        spin_bonus_escudo_2->setObjectName(QStringLiteral("spin_bonus_escudo_2"));
        spin_bonus_escudo_2->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(spin_bonus_escudo_2, 0, 4, 1, 1);

        groupBox_4 = new QGroupBox(tab_estatisticas);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(0, 0, 231, 261));
        groupBox_4->setStyleSheet(QStringLiteral(""));
        gridLayoutWidget_2 = new QWidget(groupBox_4);
        gridLayoutWidget_2->setObjectName(QStringLiteral("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(9, 10, 211, 241));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        botao_bonus_constituicao = new QPushButton(gridLayoutWidget_2);
        botao_bonus_constituicao->setObjectName(QStringLiteral("botao_bonus_constituicao"));

        gridLayout_2->addWidget(botao_bonus_constituicao, 3, 2, 1, 1);

        label_47 = new QLabel(gridLayoutWidget_2);
        label_47->setObjectName(QStringLiteral("label_47"));
        label_47->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_47, 4, 0, 1, 1);

        spin_destreza = new QSpinBox(gridLayoutWidget_2);
        spin_destreza->setObjectName(QStringLiteral("spin_destreza"));

        gridLayout_2->addWidget(spin_destreza, 2, 1, 1, 1);

        label_49 = new QLabel(gridLayoutWidget_2);
        label_49->setObjectName(QStringLiteral("label_49"));
        label_49->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_49, 5, 0, 1, 1);

        label_51 = new QLabel(gridLayoutWidget_2);
        label_51->setObjectName(QStringLiteral("label_51"));
        label_51->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_51, 6, 0, 1, 1);

        botao_bonus_destreza = new QPushButton(gridLayoutWidget_2);
        botao_bonus_destreza->setObjectName(QStringLiteral("botao_bonus_destreza"));

        gridLayout_2->addWidget(botao_bonus_destreza, 2, 2, 1, 1);

        label_50 = new QLabel(gridLayoutWidget_2);
        label_50->setObjectName(QStringLiteral("label_50"));
        label_50->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_50, 3, 0, 1, 1);

        label_mod_destreza = new QLabel(gridLayoutWidget_2);
        label_mod_destreza->setObjectName(QStringLiteral("label_mod_destreza"));
        label_mod_destreza->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_destreza, 2, 3, 1, 1);

        spin_carisma = new QSpinBox(gridLayoutWidget_2);
        spin_carisma->setObjectName(QStringLiteral("spin_carisma"));

        gridLayout_2->addWidget(spin_carisma, 6, 1, 1, 1);

        botao_bonus_forca = new QPushButton(gridLayoutWidget_2);
        botao_bonus_forca->setObjectName(QStringLiteral("botao_bonus_forca"));

        gridLayout_2->addWidget(botao_bonus_forca, 1, 2, 1, 1);

        spin_sabedoria = new QSpinBox(gridLayoutWidget_2);
        spin_sabedoria->setObjectName(QStringLiteral("spin_sabedoria"));

        gridLayout_2->addWidget(spin_sabedoria, 5, 1, 1, 1);

        spin_inteligencia = new QSpinBox(gridLayoutWidget_2);
        spin_inteligencia->setObjectName(QStringLiteral("spin_inteligencia"));

        gridLayout_2->addWidget(spin_inteligencia, 4, 1, 1, 1);

        spin_forca = new QSpinBox(gridLayoutWidget_2);
        spin_forca->setObjectName(QStringLiteral("spin_forca"));

        gridLayout_2->addWidget(spin_forca, 1, 1, 1, 1);

        label_27 = new QLabel(gridLayoutWidget_2);
        label_27->setObjectName(QStringLiteral("label_27"));
        label_27->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_27, 1, 0, 1, 1);

        spin_constituicao = new QSpinBox(gridLayoutWidget_2);
        spin_constituicao->setObjectName(QStringLiteral("spin_constituicao"));

        gridLayout_2->addWidget(spin_constituicao, 3, 1, 1, 1);

        label_mod_forca = new QLabel(gridLayoutWidget_2);
        label_mod_forca->setObjectName(QStringLiteral("label_mod_forca"));
        label_mod_forca->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_forca, 1, 3, 1, 1);

        label_mod_sabedoria = new QLabel(gridLayoutWidget_2);
        label_mod_sabedoria->setObjectName(QStringLiteral("label_mod_sabedoria"));
        label_mod_sabedoria->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_sabedoria, 5, 3, 1, 1);

        botao_bonus_sabedoria = new QPushButton(gridLayoutWidget_2);
        botao_bonus_sabedoria->setObjectName(QStringLiteral("botao_bonus_sabedoria"));

        gridLayout_2->addWidget(botao_bonus_sabedoria, 5, 2, 1, 1);

        label_48 = new QLabel(gridLayoutWidget_2);
        label_48->setObjectName(QStringLiteral("label_48"));
        label_48->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_48, 2, 0, 1, 1);

        botao_bonus_inteligencia = new QPushButton(gridLayoutWidget_2);
        botao_bonus_inteligencia->setObjectName(QStringLiteral("botao_bonus_inteligencia"));

        gridLayout_2->addWidget(botao_bonus_inteligencia, 4, 2, 1, 1);

        label_mod_inteligencia = new QLabel(gridLayoutWidget_2);
        label_mod_inteligencia->setObjectName(QStringLiteral("label_mod_inteligencia"));
        label_mod_inteligencia->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_inteligencia, 4, 3, 1, 1);

        botao_bonus_carisma = new QPushButton(gridLayoutWidget_2);
        botao_bonus_carisma->setObjectName(QStringLiteral("botao_bonus_carisma"));

        gridLayout_2->addWidget(botao_bonus_carisma, 6, 2, 1, 1);

        label_mod_constituicao = new QLabel(gridLayoutWidget_2);
        label_mod_constituicao->setObjectName(QStringLiteral("label_mod_constituicao"));
        label_mod_constituicao->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_constituicao, 3, 3, 1, 1);

        label_mod_carisma = new QLabel(gridLayoutWidget_2);
        label_mod_carisma->setObjectName(QStringLiteral("label_mod_carisma"));
        label_mod_carisma->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_mod_carisma, 6, 3, 1, 1);

        label_28 = new QLabel(gridLayoutWidget_2);
        label_28->setObjectName(QStringLiteral("label_28"));
        QSizePolicy sizePolicy7(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(label_28->sizePolicy().hasHeightForWidth());
        label_28->setSizePolicy(sizePolicy7);
        label_28->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_28, 0, 3, 1, 1);

        label_63 = new QLabel(gridLayoutWidget_2);
        label_63->setObjectName(QStringLiteral("label_63"));
        sizePolicy7.setHeightForWidth(label_63->sizePolicy().hasHeightForWidth());
        label_63->setSizePolicy(sizePolicy7);
        label_63->setFont(font);
        label_63->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_63, 0, 2, 1, 1);

        label_62 = new QLabel(gridLayoutWidget_2);
        label_62->setObjectName(QStringLiteral("label_62"));
        sizePolicy7.setHeightForWidth(label_62->sizePolicy().hasHeightForWidth());
        label_62->setSizePolicy(sizePolicy7);
        label_62->setAlignment(Qt::AlignCenter);

        gridLayout_2->addWidget(label_62, 0, 1, 1, 1);

        groupBox_5 = new QGroupBox(tab_estatisticas);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(0, 270, 171, 141));
        gridLayoutWidget_4 = new QWidget(groupBox_5);
        gridLayoutWidget_4->setObjectName(QStringLiteral("gridLayoutWidget_4"));
        gridLayoutWidget_4->setGeometry(QRect(-1, 19, 161, 118));
        gridLayout_4 = new QGridLayout(gridLayoutWidget_4);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        gridLayout_4->setContentsMargins(0, 0, 0, 0);
        label_65 = new QLabel(gridLayoutWidget_4);
        label_65->setObjectName(QStringLiteral("label_65"));
        label_65->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_65, 2, 0, 1, 1);

        label_66 = new QLabel(gridLayoutWidget_4);
        label_66->setObjectName(QStringLiteral("label_66"));
        label_66->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_66, 3, 0, 1, 1);

        botao_bonus_salvacao_fortitude = new QPushButton(gridLayoutWidget_4);
        botao_bonus_salvacao_fortitude->setObjectName(QStringLiteral("botao_bonus_salvacao_fortitude"));

        gridLayout_4->addWidget(botao_bonus_salvacao_fortitude, 1, 1, 1, 1);

        botao_bonus_salvacao_vontade = new QPushButton(gridLayoutWidget_4);
        botao_bonus_salvacao_vontade->setObjectName(QStringLiteral("botao_bonus_salvacao_vontade"));

        gridLayout_4->addWidget(botao_bonus_salvacao_vontade, 3, 1, 1, 1);

        label_67 = new QLabel(gridLayoutWidget_4);
        label_67->setObjectName(QStringLiteral("label_67"));
        label_67->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_4->addWidget(label_67, 1, 0, 1, 1);

        botao_bonus_salvacao_reflexo = new QPushButton(gridLayoutWidget_4);
        botao_bonus_salvacao_reflexo->setObjectName(QStringLiteral("botao_bonus_salvacao_reflexo"));

        gridLayout_4->addWidget(botao_bonus_salvacao_reflexo, 2, 1, 1, 1);

        label_70 = new QLabel(gridLayoutWidget_4);
        label_70->setObjectName(QStringLiteral("label_70"));
        sizePolicy7.setHeightForWidth(label_70->sizePolicy().hasHeightForWidth());
        label_70->setSizePolicy(sizePolicy7);
        label_70->setFont(font);
        label_70->setAlignment(Qt::AlignCenter);

        gridLayout_4->addWidget(label_70, 0, 1, 1, 1);

        groupBox_6 = new QGroupBox(tab_estatisticas);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        groupBox_6->setGeometry(QRect(670, 10, 281, 71));
        groupBox_6->setStyleSheet(QStringLiteral(""));
        gridLayoutWidget_5 = new QWidget(groupBox_6);
        gridLayoutWidget_5->setObjectName(QStringLiteral("gridLayoutWidget_5"));
        gridLayoutWidget_5->setGeometry(QRect(0, 20, 281, 42));
        gridLayout_5 = new QGridLayout(gridLayoutWidget_5);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_5->setContentsMargins(0, 0, 0, 0);
        label_bba_agarrar = new QLabel(gridLayoutWidget_5);
        label_bba_agarrar->setObjectName(QStringLiteral("label_bba_agarrar"));
        label_bba_agarrar->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_agarrar, 1, 1, 1, 1);

        label_69 = new QLabel(gridLayoutWidget_5);
        label_69->setObjectName(QStringLiteral("label_69"));
        label_69->setFont(font);
        label_69->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_69, 0, 2, 1, 1);

        label_54 = new QLabel(gridLayoutWidget_5);
        label_54->setObjectName(QStringLiteral("label_54"));
        label_54->setFont(font);
        label_54->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_54, 0, 0, 1, 1);

        label_bba_base = new QLabel(gridLayoutWidget_5);
        label_bba_base->setObjectName(QStringLiteral("label_bba_base"));
        label_bba_base->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_base, 1, 0, 1, 1);

        label_56 = new QLabel(gridLayoutWidget_5);
        label_56->setObjectName(QStringLiteral("label_56"));
        label_56->setFont(font);
        label_56->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_56, 0, 1, 1, 1);

        label_71 = new QLabel(gridLayoutWidget_5);
        label_71->setObjectName(QStringLiteral("label_71"));
        label_71->setFont(font);

        gridLayout_5->addWidget(label_71, 0, 3, 1, 1);

        label_bba_cac = new QLabel(gridLayoutWidget_5);
        label_bba_cac->setObjectName(QStringLiteral("label_bba_cac"));
        label_bba_cac->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_cac, 1, 2, 1, 1);

        label_bba_distancia = new QLabel(gridLayoutWidget_5);
        label_bba_distancia->setObjectName(QStringLiteral("label_bba_distancia"));
        label_bba_distancia->setAlignment(Qt::AlignCenter);

        gridLayout_5->addWidget(label_bba_distancia, 1, 3, 1, 1);

        groupBox_7 = new QGroupBox(tab_estatisticas);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        groupBox_7->setGeometry(QRect(290, 10, 341, 41));
        groupBox_7->setStyleSheet(QStringLiteral(""));
        layoutWidget_3 = new QWidget(groupBox_7);
        layoutWidget_3->setObjectName(QStringLiteral("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(0, 10, 341, 33));
        horizontalLayout_28 = new QHBoxLayout(layoutWidget_3);
        horizontalLayout_28->setObjectName(QStringLiteral("horizontalLayout_28"));
        horizontalLayout_28->setContentsMargins(0, 0, 0, 0);
        label_22 = new QLabel(layoutWidget_3);
        label_22->setObjectName(QStringLiteral("label_22"));
        sizePolicy.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy);
        label_22->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_28->addWidget(label_22);

        botao_bonus_iniciativa = new QPushButton(layoutWidget_3);
        botao_bonus_iniciativa->setObjectName(QStringLiteral("botao_bonus_iniciativa"));

        horizontalLayout_28->addWidget(botao_bonus_iniciativa);

        checkbox_iniciativa = new QCheckBox(layoutWidget_3);
        checkbox_iniciativa->setObjectName(QStringLiteral("checkbox_iniciativa"));
        sizePolicy1.setHeightForWidth(checkbox_iniciativa->sizePolicy().hasHeightForWidth());
        checkbox_iniciativa->setSizePolicy(sizePolicy1);

        horizontalLayout_28->addWidget(checkbox_iniciativa);

        spin_iniciativa = new QSpinBox(layoutWidget_3);
        spin_iniciativa->setObjectName(QStringLiteral("spin_iniciativa"));
        sizePolicy1.setHeightForWidth(spin_iniciativa->sizePolicy().hasHeightForWidth());
        spin_iniciativa->setSizePolicy(sizePolicy1);
        spin_iniciativa->setMinimum(-100);
        spin_iniciativa->setMaximum(999);

        horizontalLayout_28->addWidget(spin_iniciativa);

        horizontalLayoutWidget_8 = new QWidget(tab_estatisticas);
        horizontalLayoutWidget_8->setObjectName(QStringLiteral("horizontalLayoutWidget_8"));
        horizontalLayoutWidget_8->setGeometry(QRect(10, 420, 161, 31));
        horizontalLayout_11 = new QHBoxLayout(horizontalLayoutWidget_8);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        label_85 = new QLabel(horizontalLayoutWidget_8);
        label_85->setObjectName(QStringLiteral("label_85"));
        label_85->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_11->addWidget(label_85);

        spin_rm = new QSpinBox(horizontalLayoutWidget_8);
        spin_rm->setObjectName(QStringLiteral("spin_rm"));

        horizontalLayout_11->addWidget(spin_rm);

        tab_tesouro->addTab(tab_estatisticas, QString());
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        layoutWidget_4 = new QWidget(tab);
        layoutWidget_4->setObjectName(QStringLiteral("layoutWidget_4"));
        layoutWidget_4->setGeometry(QRect(0, 10, 481, 221));
        horizontalLayout_30 = new QHBoxLayout(layoutWidget_4);
        horizontalLayout_30->setObjectName(QStringLiteral("horizontalLayout_30"));
        horizontalLayout_30->setContentsMargins(0, 0, 0, 0);
        label_37 = new QLabel(layoutWidget_4);
        label_37->setObjectName(QStringLiteral("label_37"));
        sizePolicy5.setHeightForWidth(label_37->sizePolicy().hasHeightForWidth());
        label_37->setSizePolicy(sizePolicy5);
        label_37->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_30->addWidget(label_37);

        lista_tesouro = new QPlainTextEdit(layoutWidget_4);
        lista_tesouro->setObjectName(QStringLiteral("lista_tesouro"));

        horizontalLayout_30->addWidget(lista_tesouro);

        layoutWidget_7 = new QWidget(tab);
        layoutWidget_7->setObjectName(QStringLiteral("layoutWidget_7"));
        layoutWidget_7->setGeometry(QRect(10, 250, 481, 221));
        horizontalLayout_36 = new QHBoxLayout(layoutWidget_7);
        horizontalLayout_36->setObjectName(QStringLiteral("horizontalLayout_36"));
        horizontalLayout_36->setContentsMargins(0, 0, 0, 0);
        label_74 = new QLabel(layoutWidget_7);
        label_74->setObjectName(QStringLiteral("label_74"));
        sizePolicy5.setHeightForWidth(label_74->sizePolicy().hasHeightForWidth());
        label_74->setSizePolicy(sizePolicy5);
        label_74->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_36->addWidget(label_74);

        lista_pocoes = new QListWidget(layoutWidget_7);
        lista_pocoes->setObjectName(QStringLiteral("lista_pocoes"));

        horizontalLayout_36->addWidget(lista_pocoes);

        botao_adicionar_pocao = new QPushButton(tab);
        botao_adicionar_pocao->setObjectName(QStringLiteral("botao_adicionar_pocao"));
        botao_adicionar_pocao->setGeometry(QRect(500, 320, 31, 27));
        botao_remover_pocao = new QPushButton(tab);
        botao_remover_pocao->setObjectName(QStringLiteral("botao_remover_pocao"));
        botao_remover_pocao->setGeometry(QRect(500, 350, 31, 27));
        tab_tesouro->addTab(tab, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        arvore_feiticos = new QTreeWidget(tab_4);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        arvore_feiticos->setHeaderItem(__qtreewidgetitem);
        arvore_feiticos->setObjectName(QStringLiteral("arvore_feiticos"));
        arvore_feiticos->setGeometry(QRect(10, 20, 1021, 461));
        QSizePolicy sizePolicy8(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(arvore_feiticos->sizePolicy().hasHeightForWidth());
        arvore_feiticos->setSizePolicy(sizePolicy8);
        arvore_feiticos->setDragEnabled(false);
        arvore_feiticos->setDragDropMode(QAbstractItemView::NoDragDrop);
        arvore_feiticos->setHeaderHidden(true);
        arvore_feiticos->setColumnCount(1);
        arvore_feiticos->header()->setDefaultSectionSize(100);
        tab_tesouro->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        layoutWidget_5 = new QWidget(tab_2);
        layoutWidget_5->setObjectName(QStringLiteral("layoutWidget_5"));
        layoutWidget_5->setGeometry(QRect(20, 20, 751, 481));
        horizontalLayout_32 = new QHBoxLayout(layoutWidget_5);
        horizontalLayout_32->setObjectName(QStringLiteral("horizontalLayout_32"));
        horizontalLayout_32->setContentsMargins(0, 0, 0, 0);
        label_45 = new QLabel(layoutWidget_5);
        label_45->setObjectName(QStringLiteral("label_45"));
        sizePolicy5.setHeightForWidth(label_45->sizePolicy().hasHeightForWidth());
        label_45->setSizePolicy(sizePolicy5);
        label_45->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_32->addWidget(label_45);

        texto_notas = new QPlainTextEdit(layoutWidget_5);
        texto_notas->setObjectName(QStringLiteral("texto_notas"));

        horizontalLayout_32->addWidget(texto_notas);

        tab_tesouro->addTab(tab_2, QString());
        QWidget::setTabOrder(campo_rotulo, lista_rotulos);
        QWidget::setTabOrder(lista_rotulos, slider_tamanho);
        QWidget::setTabOrder(slider_tamanho, spin_pontos_vida);
        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, checkbox_selecionavel);
        QWidget::setTabOrder(checkbox_selecionavel, checkbox_voadora);
        QWidget::setTabOrder(checkbox_voadora, checkbox_visibilidade);
        QWidget::setTabOrder(checkbox_visibilidade, spin_aura_quad);
        QWidget::setTabOrder(spin_aura_quad, combo_visao);
        QWidget::setTabOrder(combo_visao, spin_raio_visao_escuro_quad);
        QWidget::setTabOrder(spin_raio_visao_escuro_quad, spin_translacao_quad);
        QWidget::setTabOrder(spin_translacao_quad, combo_salvacao);
        QWidget::setTabOrder(combo_salvacao, checkbox_cor);
        QWidget::setTabOrder(checkbox_cor, botao_cor);
        QWidget::setTabOrder(botao_cor, slider_alfa);
        QWidget::setTabOrder(slider_alfa, spin_raio_quad);
        QWidget::setTabOrder(spin_raio_quad, botao_luz);
        QWidget::setTabOrder(botao_luz, combo_textura);
        QWidget::setTabOrder(combo_textura, spin_tex_trans_x);
        QWidget::setTabOrder(spin_tex_trans_x, spin_tex_largura);
        QWidget::setTabOrder(spin_tex_largura, spin_tex_trans_y);
        QWidget::setTabOrder(spin_tex_trans_y, spin_tex_altura);
        QWidget::setTabOrder(spin_tex_altura, combo_modelos_3d);
        QWidget::setTabOrder(combo_modelos_3d, linha_classe);
        QWidget::setTabOrder(linha_classe, spin_nivel_classe);
        QWidget::setTabOrder(spin_nivel_classe, spin_bba);
        QWidget::setTabOrder(spin_bba, spin_nivel_conjurador);
        QWidget::setTabOrder(spin_nivel_conjurador, lista_niveis);
        QWidget::setTabOrder(lista_niveis, botao_adicionar_nivel);
        QWidget::setTabOrder(botao_adicionar_nivel, botao_remover_nivel);
        QWidget::setTabOrder(botao_remover_nivel, checkbox_iniciativa);
        QWidget::setTabOrder(checkbox_iniciativa, spin_iniciativa);
        QWidget::setTabOrder(spin_iniciativa, spin_forca);
        QWidget::setTabOrder(spin_forca, spin_destreza);
        QWidget::setTabOrder(spin_destreza, spin_constituicao);
        QWidget::setTabOrder(spin_constituicao, spin_inteligencia);
        QWidget::setTabOrder(spin_inteligencia, spin_sabedoria);
        QWidget::setTabOrder(spin_sabedoria, spin_carisma);
        QWidget::setTabOrder(spin_carisma, spin_ca_armadura_melhoria);
        QWidget::setTabOrder(spin_ca_armadura_melhoria, spin_ca_escudo_melhoria);
        QWidget::setTabOrder(spin_ca_escudo_melhoria, linha_rotulo_ataque);
        QWidget::setTabOrder(linha_rotulo_ataque, combo_tipo_ataque);
        QWidget::setTabOrder(combo_tipo_ataque, spin_alcance_quad);
        QWidget::setTabOrder(spin_alcance_quad, spin_incrementos);
        QWidget::setTabOrder(spin_incrementos, campo_id);
        QWidget::setTabOrder(campo_id, checkbox_imune_critico);
        QWidget::setTabOrder(checkbox_imune_critico, linha_furtivo);
        QWidget::setTabOrder(linha_furtivo, botao_ataque_cima);
        QWidget::setTabOrder(botao_ataque_cima, botao_ataque_baixo);
        QWidget::setTabOrder(botao_ataque_baixo, botao_clonar_ataque);
        QWidget::setTabOrder(botao_clonar_ataque, botao_remover_ataque);
        QWidget::setTabOrder(botao_remover_ataque, lista_tesouro);
        QWidget::setTabOrder(lista_tesouro, texto_notas);
        QWidget::setTabOrder(texto_notas, lista_ataques);
        QWidget::setTabOrder(lista_ataques, botoes);
        QWidget::setTabOrder(botoes, tab_tesouro);
        QWidget::setTabOrder(tab_tesouro, linha_bba);
        QWidget::setTabOrder(linha_bba, linha_nivel);
        QWidget::setTabOrder(linha_nivel, checkbox_caida);
        QWidget::setTabOrder(checkbox_caida, checkbox_morta);
        QWidget::setTabOrder(checkbox_morta, checkbox_salvacao);
        QWidget::setTabOrder(checkbox_salvacao, tabela_lista_eventos);
        QWidget::setTabOrder(tabela_lista_eventos, botao_adicionar_evento);
        QWidget::setTabOrder(botao_adicionar_evento, botao_remover_evento);
        QWidget::setTabOrder(botao_remover_evento, lista_formas_alternativas);
        QWidget::setTabOrder(lista_formas_alternativas, botao_adicionar_forma_alternativa);
        QWidget::setTabOrder(botao_adicionar_forma_alternativa, botao_remover_forma_alternativa);
        QWidget::setTabOrder(botao_remover_forma_alternativa, botao_bonus_pv_temporario);
        QWidget::setTabOrder(botao_bonus_pv_temporario, spin_dano_nao_letal);
        QWidget::setTabOrder(spin_dano_nao_letal, combo_classe);
        QWidget::setTabOrder(combo_classe, combo_mod_conjuracao);
        QWidget::setTabOrder(combo_mod_conjuracao, combo_salvacoes_fortes);
        QWidget::setTabOrder(combo_salvacoes_fortes, slider_ordem_caos);
        QWidget::setTabOrder(slider_ordem_caos, slider_bem_mal);
        QWidget::setTabOrder(slider_bem_mal, spin_niveis_negativos);
        QWidget::setTabOrder(spin_niveis_negativos, spin_xp);
        QWidget::setTabOrder(spin_xp, botao_remover_talento);
        QWidget::setTabOrder(botao_remover_talento, botao_adicionar_talento);
        QWidget::setTabOrder(botao_adicionar_talento, tabela_talentos);
        QWidget::setTabOrder(tabela_talentos, tabela_pericias);
        QWidget::setTabOrder(tabela_pericias, combo_arma);
        QWidget::setTabOrder(combo_arma, checkbox_op);
        QWidget::setTabOrder(checkbox_op, spin_bonus_magico);
        QWidget::setTabOrder(spin_bonus_magico, spin_municao);
        QWidget::setTabOrder(spin_municao, spin_ordem_ataque);
        QWidget::setTabOrder(spin_ordem_ataque, botao_bonus_ataque);
        QWidget::setTabOrder(botao_bonus_ataque, linha_dano);
        QWidget::setTabOrder(linha_dano, combo_empunhadura);
        QWidget::setTabOrder(combo_empunhadura, botao_bonus_dano);
        QWidget::setTabOrder(botao_bonus_dano, botao_bonus_ca);
        QWidget::setTabOrder(botao_bonus_ca, combo_armadura);
        QWidget::setTabOrder(combo_armadura, combo_escudo);
        QWidget::setTabOrder(combo_escudo, botao_bonus_constituicao);
        QWidget::setTabOrder(botao_bonus_constituicao, botao_bonus_destreza);
        QWidget::setTabOrder(botao_bonus_destreza, botao_bonus_forca);
        QWidget::setTabOrder(botao_bonus_forca, botao_bonus_sabedoria);
        QWidget::setTabOrder(botao_bonus_sabedoria, botao_bonus_inteligencia);
        QWidget::setTabOrder(botao_bonus_inteligencia, botao_bonus_carisma);
        QWidget::setTabOrder(botao_bonus_carisma, botao_bonus_salvacao_fortitude);
        QWidget::setTabOrder(botao_bonus_salvacao_fortitude, botao_bonus_salvacao_vontade);
        QWidget::setTabOrder(botao_bonus_salvacao_vontade, botao_bonus_salvacao_reflexo);
        QWidget::setTabOrder(botao_bonus_salvacao_reflexo, botao_bonus_iniciativa);
        QWidget::setTabOrder(botao_bonus_iniciativa, spin_rm);
        QWidget::setTabOrder(spin_rm, lista_pocoes);
        QWidget::setTabOrder(lista_pocoes, botao_adicionar_pocao);
        QWidget::setTabOrder(botao_adicionar_pocao, botao_remover_pocao);

        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));

        tab_tesouro->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", 0));
#ifndef QT_NO_TOOLTIP
        ifg__qt__DialogoEntidade->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Cor:", 0));
        checkbox_cor->setText(QString());
#ifndef QT_NO_TOOLTIP
        botao_cor->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Cor da entidade.", 0));
#endif // QT_NO_TOOLTIP
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_selecionavel->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, jogadores poder\303\243o ver as propriedades e controlar a entidade.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Selecion\303\241vel", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_voadora->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade flutuar\303\241.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_voadora->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Voadora", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_visibilidade->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade ser\303\241 vista para jogadores. Caso seja selecion\303\241vel, a entidade ficar\303\241 transl\303\272cida.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\255vel", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_caida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade cair\303\241.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_caida->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Ca\303\255da", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_morta->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade estar\303\241 morta.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_morta->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Morta", 0));
        label_15->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Textura", 0));
        label_2->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", 0));
        label_tamanho->setText(QApplication::translate("ifg::qt::DialogoEntidade", "(m\303\251dio)", 0));
#ifndef QT_NO_TOOLTIP
        slider_tamanho->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho da entidade", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_salvacao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Marcar, se a entidade tiver rolado a pr\303\263xima salva\303\247\303\243o.", 0));
#endif // QT_NO_TOOLTIP
        checkbox_salvacao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Pr\303\263xima Salva\303\247\303\243o", 0));
        combo_salvacao->clear();
        combo_salvacao->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "Falha", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Meio Dano", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Um Quarto de Dano", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Dano Anulado", 0)
        );
#ifndef QT_NO_TOOLTIP
        combo_salvacao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Dano que a entidade receber\303\241 na pr\303\263xima a\303\247\303\243o de \303\241rea.", 0));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o", 0));
        combo_visao->clear();
        combo_visao->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "Normal", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o na Penumbra", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o no Escuro", 0)
        );
#ifndef QT_NO_TOOLTIP
        combo_visao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Tipo de vis\303\243o da entidade.", 0));
#endif // QT_NO_TOOLTIP
        label_14->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0));
#ifndef QT_NO_TOOLTIP
        spin_raio_visao_escuro_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0));
#endif // QT_NO_TOOLTIP
        label_33->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0));
        label_17->setText(QApplication::translate("ifg::qt::DialogoEntidade", "altura", 0));
#ifndef QT_NO_TOOLTIP
        spin_tex_altura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Altura da textura, de 0 a 1", 0));
#endif // QT_NO_TOOLTIP
        label_19->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans x", 0));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_x->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", 0));
#endif // QT_NO_TOOLTIP
        label_20->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans y", 0));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_y->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", 0));
#endif // QT_NO_TOOLTIP
        label_18->setText(QApplication::translate("ifg::qt::DialogoEntidade", "largura", 0));
#ifndef QT_NO_TOOLTIP
        spin_tex_largura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Largura da textura, de 0 a 1", 0));
#endif // QT_NO_TOOLTIP
        label_11->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Lista de Eventos", 0));
        label_12->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0));
#ifndef QT_NO_TOOLTIP
        spin_raio_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da luz, em metros.", 0));
#endif // QT_NO_TOOLTIP
        label_31->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor da Luz", 0));
        label_16->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Modelo 3D", 0));
        label->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Id", 0));
        label_8->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulo", 0));
        label_10->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos Especial", 0));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0));
#endif // QT_NO_TOOLTIP
        botao_adicionar_evento->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+", 0));
        botao_remover_evento->setText(QApplication::translate("ifg::qt::DialogoEntidade", "-", 0));
        label_68->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Formas Alternativas", 0));
#ifndef QT_NO_TOOLTIP
        lista_formas_alternativas->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Lista de formas alternativas, clique duplo para editar.", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        botao_adicionar_forma_alternativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Adiciona uma forma alternativa", 0));
#endif // QT_NO_TOOLTIP
        botao_adicionar_forma_alternativa->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+", 0));
#ifndef QT_NO_TOOLTIP
        botao_remover_forma_alternativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Remove a forma alternativa selecionada", 0));
#endif // QT_NO_TOOLTIP
        botao_remover_forma_alternativa->setText(QApplication::translate("ifg::qt::DialogoEntidade", "-", 0));
        label_4->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Aura:", 0));
#ifndef QT_NO_TOOLTIP
        spin_aura_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Aura da entidade.", 0));
#endif // QT_NO_TOOLTIP
        label_35->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0));
        label_7->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Deslocamento Vertical", 0));
#ifndef QT_NO_TOOLTIP
        spin_translacao_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Para colocar a entidade acima do plano do tabuleiro.", 0));
#endif // QT_NO_TOOLTIP
        label_34->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0));
        label_5->setText(QApplication::translate("ifg::qt::DialogoEntidade", "PV", 0));
#ifndef QT_NO_TOOLTIP
        spin_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de vida para entidade.", 0));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Max", 0));
#ifndef QT_NO_TOOLTIP
        spin_max_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de pontos de vida para entidade.", 0));
#endif // QT_NO_TOOLTIP
        label_26->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Temp", 0));
#ifndef QT_NO_TOOLTIP
        botao_bonus_pv_temporario->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de Vida tempor\303\241rios.", 0));
#endif // QT_NO_TOOLTIP
        botao_bonus_pv_temporario->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        label_79->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\243o letal", 0));
#ifndef QT_NO_TOOLTIP
        spin_dano_nao_letal->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Quantidade de dano n\303\243o letal.", 0));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_geral), QApplication::translate("ifg::qt::DialogoEntidade", "Geral", 0));
        label_39->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel PC", 0));
#ifndef QT_NO_TOOLTIP
        linha_nivel->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel do Personagem", 0));
#endif // QT_NO_TOOLTIP
        botao_adicionar_nivel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Adicionar N\303\255vel", 0));
        botao_remover_nivel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Remover", 0));
        label_46->setText(QApplication::translate("ifg::qt::DialogoEntidade", "BBA", 0));
#ifndef QT_NO_TOOLTIP
        linha_bba->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque Total", 0));
#endif // QT_NO_TOOLTIP
        groupBox_2->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados N\303\255vel", 0));
        label_40->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Classe", 0));
#ifndef QT_NO_TOOLTIP
        linha_classe->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Identificador da Classe", 0));
#endif // QT_NO_TOOLTIP
        label_41->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel", 0));
#ifndef QT_NO_TOOLTIP
        spin_nivel_classe->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel da Classe", 0));
#endif // QT_NO_TOOLTIP
        label_43->setText(QApplication::translate("ifg::qt::DialogoEntidade", "BBA", 0));
#ifndef QT_NO_TOOLTIP
        spin_bba->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque ", 0));
#endif // QT_NO_TOOLTIP
        label_42->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Conjurador", 0));
#ifndef QT_NO_TOOLTIP
        spin_nivel_conjurador->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel de Conjurador", 0));
#endif // QT_NO_TOOLTIP
        label_44->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Mod", 0));
        combo_mod_conjuracao->clear();
        combo_mod_conjuracao->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "For\303\247a", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Destreza", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Constitui\303\247\303\243o", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Intelig\303\252ncia", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Sabedoria", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Carisma", 0)
        );
        label_mod_conjuracao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "00", 0));
        label_64->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Salva\303\247\303\265es Fortes", 0));
        label_75->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Mal", 0));
        label_76->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Caos", 0));
        label_77->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Ordem", 0));
        label_78->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bem", 0));
        label_80->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255veis Negativos", 0));
#ifndef QT_NO_TOOLTIP
        spin_niveis_negativos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255veis negativos.", 0));
#endif // QT_NO_TOOLTIP
        label_84->setText(QApplication::translate("ifg::qt::DialogoEntidade", "XP", 0));
#ifndef QT_NO_TOOLTIP
        spin_xp->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Experi\303\252ncia do personagem.", 0));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_nivel), QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel e Tend\303\252ncia", 0));
        label_9->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Talentos", 0));
        botao_remover_talento->setText(QApplication::translate("ifg::qt::DialogoEntidade", "-", 0));
        botao_adicionar_talento->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+", 0));
        label_81->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Per\303\255cias", 0));
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_3), QApplication::translate("ifg::qt::DialogoEntidade", "Per\303\255cias e Talentos", 0));
        botao_remover_ataque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Remover ataque", 0));
        checkbox_imune_critico->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Imune a Cr\303\255tico?", 0));
        botao_ataque_cima->setText(QApplication::translate("ifg::qt::DialogoEntidade", "\342\206\221", 0));
        botao_ataque_baixo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "\342\206\223", 0));
        botao_clonar_ataque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Clonar ataque", 0));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados Ataque", 0));
        label_36->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Nome", 0));
#ifndef QT_NO_TOOLTIP
        linha_rotulo_ataque->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Identificador do ataque", 0));
#endif // QT_NO_TOOLTIP
        label_21->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tipo", 0));
#ifndef QT_NO_TOOLTIP
        combo_arma->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Escolha uma arma, ou nenhuma para preencher manualmente.", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_op->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Arma obra prima?", 0));
#endif // QT_NO_TOOLTIP
        checkbox_op->setText(QApplication::translate("ifg::qt::DialogoEntidade", "OP", 0));
#ifndef QT_NO_TOOLTIP
        label_23->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus M\303\241gico", 0));
#endif // QT_NO_TOOLTIP
        label_23->setText(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus M\303\241gico", 0));
#ifndef QT_NO_TOOLTIP
        spin_bonus_magico->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus m\303\241gico da arma", 0));
#endif // QT_NO_TOOLTIP
        label_29->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Alcance (quads)", 0));
#ifndef QT_NO_TOOLTIP
        spin_alcance_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Alcance em quadrados", 0));
#endif // QT_NO_TOOLTIP
        label_30->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Max incrementos", 0));
        label_32->setText(QString());
#ifndef QT_NO_TOOLTIP
        spin_incrementos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de incrementos permitido pelo ataque", 0));
#endif // QT_NO_TOOLTIP
        label_82->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Muni\303\247\303\243o", 0));
#ifndef QT_NO_TOOLTIP
        spin_municao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de incrementos permitido pelo ataque", 0));
#endif // QT_NO_TOOLTIP
        label_83->setText(QApplication::translate("ifg::qt::DialogoEntidade", "#", 0));
#ifndef QT_NO_TOOLTIP
        spin_ordem_ataque->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Ordem do ataque (1 para primeiro, 2 para segundo etc)", 0));
#endif // QT_NO_TOOLTIP
        label_55->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Outros (ataque)", 0));
#ifndef QT_NO_TOOLTIP
        botao_bonus_ataque->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Outros b\303\264nus de ataque.", 0));
#endif // QT_NO_TOOLTIP
        botao_bonus_ataque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_24->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Dano", 0));
#ifndef QT_NO_TOOLTIP
        linha_dano->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Dano base da arma. Exemplo: 1d8 (19-20/x2)", 0));
#endif // QT_NO_TOOLTIP
        label_73->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Empunhadura", 0));
        combo_empunhadura->clear();
        combo_empunhadura->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "Arma apenas", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Duas m\303\243os", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "2 Armas, m\303\243o boa ou Arma Dupla, principal", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "2 Armas, m\303\243o ruim ou Arma Dupla, secund\303\241rio", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Arma e Escudo", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Monstro: Ataque Principal", 0)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Monstro: Ataque Secund\303\241rio", 0)
        );
        label_72->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Outros (dano)", 0));
#ifndef QT_NO_TOOLTIP
        botao_bonus_dano->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Outros b\303\264nus de dano.", 0));
#endif // QT_NO_TOOLTIP
        botao_bonus_dano->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_38->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Furtivo", 0));
#ifndef QT_NO_TOOLTIP
        linha_furtivo->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Exemplo: 3d6", 0));
#endif // QT_NO_TOOLTIP
        linha_furtivo->setText(QString());
        groupBox_3->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados de CA", 0));
        label_25->setText(QString());
        label_52->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Armadura", 0));
        label_57->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Total", 0));
#ifndef QT_NO_TOOLTIP
        spin_ca_escudo_melhoria->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Melhoria Escudo", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_ca_armadura_melhoria->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Melhoria armadura", 0));
#endif // QT_NO_TOOLTIP
        label_58->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Toque", 0));
        label_59->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Surp", 0));
        label_53->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Base", 0));
        label_60->setText(QApplication::translate("ifg::qt::DialogoEntidade", "10+", 0));
        label_61->setText(QApplication::translate("ifg::qt::DialogoEntidade", "=", 0));
        label_ca_surpreso->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        botao_bonus_ca->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_ca_toque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        spin_bonus_escudo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escudo", 0));
        spin_bonus_escudo_2->setText(QString());
        groupBox_4->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Atributos", 0));
        botao_bonus_constituicao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_47->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Int", 0));
#ifndef QT_NO_TOOLTIP
        spin_destreza->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Destreza", 0));
#endif // QT_NO_TOOLTIP
        label_49->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Sab", 0));
        label_51->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Car", 0));
        botao_bonus_destreza->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_50->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Con", 0));
#ifndef QT_NO_TOOLTIP
        label_mod_destreza->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_destreza->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
#ifndef QT_NO_TOOLTIP
        spin_carisma->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Carisma", 0));
#endif // QT_NO_TOOLTIP
        botao_bonus_forca->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
#ifndef QT_NO_TOOLTIP
        spin_sabedoria->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Sabedoria", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_inteligencia->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Intelig\303\252ncia", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_forca->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "For\303\247a", 0));
#endif // QT_NO_TOOLTIP
        label_27->setText(QApplication::translate("ifg::qt::DialogoEntidade", "For", 0));
#ifndef QT_NO_TOOLTIP
        spin_constituicao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Constitui\303\247\303\243o", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        label_mod_forca->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_forca->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
#ifndef QT_NO_TOOLTIP
        label_mod_sabedoria->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_sabedoria->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
        botao_bonus_sabedoria->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_48->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Des", 0));
        botao_bonus_inteligencia->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
#ifndef QT_NO_TOOLTIP
        label_mod_inteligencia->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_inteligencia->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
        botao_bonus_carisma->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
#ifndef QT_NO_TOOLTIP
        label_mod_constituicao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_constituicao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
#ifndef QT_NO_TOOLTIP
        label_mod_carisma->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#endif // QT_NO_TOOLTIP
        label_mod_carisma->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0));
        label_28->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Mod", 0));
        label_63->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Total", 0));
        label_62->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Base", 0));
        groupBox_5->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Salva\303\247\303\265es", 0));
        label_65->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Reflexos", 0));
        label_66->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vontade", 0));
        botao_bonus_salvacao_fortitude->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        botao_bonus_salvacao_vontade->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_67->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Fortitude", 0));
        botao_bonus_salvacao_reflexo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
        label_70->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Total", 0));
        groupBox_6->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque", 0));
        label_bba_agarrar->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        label_69->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Corpo a Corpo", 0));
        label_54->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Base", 0));
        label_bba_base->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        label_56->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Agarrar", 0));
        label_71->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Dist\303\242ncia", 0));
        label_bba_cac->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        label_bba_distancia->setText(QApplication::translate("ifg::qt::DialogoEntidade", "0", 0));
        groupBox_7->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Iniciativa", 0));
        label_22->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0));
#ifndef QT_NO_TOOLTIP
        botao_bonus_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificadores de iniciativa.", 0));
#endif // QT_NO_TOOLTIP
        botao_bonus_iniciativa->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0));
#ifndef QT_NO_TOOLTIP
        checkbox_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade ter\303\241 iniciativa", 0));
#endif // QT_NO_TOOLTIP
        checkbox_iniciativa->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Ligado?   Valor", 0));
#ifndef QT_NO_TOOLTIP
        spin_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Valor da iniciativa para o combate corrente", 0));
#endif // QT_NO_TOOLTIP
        label_85->setText(QApplication::translate("ifg::qt::DialogoEntidade", "RM", 0));
#ifndef QT_NO_TOOLTIP
        spin_rm->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "spin_rm", 0));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_estatisticas), QApplication::translate("ifg::qt::DialogoEntidade", "Estat\303\255sticas", 0));
        label_37->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tesouro", 0));
#ifndef QT_NO_TOOLTIP
        lista_tesouro->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0));
#endif // QT_NO_TOOLTIP
        label_74->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Po\303\247\303\265es", 0));
        botao_adicionar_pocao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+", 0));
        botao_remover_pocao->setText(QApplication::translate("ifg::qt::DialogoEntidade", "-", 0));
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab), QApplication::translate("ifg::qt::DialogoEntidade", "Tesouro", 0));
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_4), QApplication::translate("ifg::qt::DialogoEntidade", "Feiti\303\247os", 0));
        label_45->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Notas", 0));
#ifndef QT_NO_TOOLTIP
        texto_notas->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_2), QApplication::translate("ifg::qt::DialogoEntidade", "Notas", 0));
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
