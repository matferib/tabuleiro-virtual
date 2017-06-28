/********************************************************************************
** Form generated from reading UI file 'entidade.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTIDADE_H
#define ENTIDADE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

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
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_24;
    QLabel *label_15;
    QComboBox *combo_textura;
    QWidget *horizontalLayoutWidget_8;
    QHBoxLayout *horizontalLayout_11;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_5;
    QSpinBox *spin_pontos_vida;
    QLabel *label_6;
    QSpinBox *spin_max_pontos_vida;
    QLabel *label_26;
    QSpinBox *spin_pontos_vida_temporarios;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QLabel *label_tamanho;
    QSlider *slider_tamanho;
    QWidget *horizontalLayoutWidget_10;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_9;
    QComboBox *combo_salvacao;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_7;
    QDoubleSpinBox *spin_translacao_quad;
    QLabel *label_34;
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
    QPlainTextEdit *lista_eventos;
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
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_9;
    QCheckBox *checkbox_caida;
    QCheckBox *checkbox_morta;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_4;
    QDoubleSpinBox *spin_aura_quad;
    QLabel *label_35;
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
    QSpinBox *spin_mod_conjuracao;
    QWidget *tab_estatisticas;
    QWidget *layoutWidget_3;
    QHBoxLayout *horizontalLayout_28;
    QCheckBox *checkbox_iniciativa;
    QSpinBox *spin_iniciativa;
    QLabel *label_22;
    QSpinBox *spin_modificador_iniciativa;
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
    QCheckBox *checkbox_permite_escudo;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_29;
    QSpinBox *spin_alcance_quad;
    QLabel *label_30;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_32;
    QSpinBox *spin_incrementos;
    QSpacerItem *horizontalSpacer_2;
    QLabel *label_23;
    QSpinBox *spin_ataque;
    QLabel *label_24;
    QLineEdit *linha_dano;
    QLabel *label_38;
    QLineEdit *linha_furtivo;
    QGroupBox *groupBox_3;
    QLabel *label_25;
    QWidget *gridLayoutWidget_3;
    QGridLayout *gridLayout_3;
    QLabel *spin_bonus_escudo;
    QLabel *label_57;
    QSpinBox *spin_ca_escudo;
    QSpinBox *spin_ca_armadura;
    QSpinBox *spin_ca_toque;
    QSpinBox *spin_ca_destreza;
    QLabel *label_52;
    QLabel *label_58;
    QSpinBox *spin_ca_total;
    QLabel *label_56;
    QLabel *label_55;
    QSpinBox *spin_ca_tamanho;
    QLabel *label_54;
    QSpinBox *spin_ca_surpreso;
    QLabel *label_59;
    QLabel *label_53;
    QLabel *label_60;
    QLabel *label_61;
    QPushButton *botao_bonus_ca;
    QGroupBox *groupBox_4;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QLabel *label_47;
    QSpinBox *spin_des;
    QLabel *label_49;
    QLabel *label_51;
    QPushButton *botao_bonus_con;
    QLabel *label_50;
    QLabel *label_mod_des;
    QPushButton *botao_bonus_des;
    QSpinBox *spin_car;
    QSpinBox *spin_sab;
    QSpinBox *spin_int;
    QPushButton *botao_bonus_for;
    QLabel *label_27;
    QSpinBox *spin_for;
    QLabel *label_mod_for;
    QSpinBox *spin_con;
    QLabel *label_mod_sab;
    QLabel *label_48;
    QLabel *label_mod_int;
    QPushButton *botao_bonus_sab;
    QPushButton *botao_bonus_int;
    QLabel *label_mod_con;
    QLabel *label_mod_car;
    QPushButton *botao_bonus_car;
    QWidget *tab;
    QWidget *layoutWidget_4;
    QHBoxLayout *horizontalLayout_30;
    QLabel *label_37;
    QPlainTextEdit *lista_tesouro;
    QWidget *tab_2;
    QWidget *layoutWidget_5;
    QHBoxLayout *horizontalLayout_32;
    QLabel *label_45;
    QPlainTextEdit *texto_notas;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QString::fromUtf8("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(962, 632);
        ifg__qt__DialogoEntidade->setStyleSheet(QString::fromUtf8(""));
        ifg__qt__DialogoEntidade->setModal(true);
        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(560, 570, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tab_tesouro = new QTabWidget(ifg__qt__DialogoEntidade);
        tab_tesouro->setObjectName(QString::fromUtf8("tab_tesouro"));
        tab_tesouro->setGeometry(QRect(10, 20, 931, 541));
        tab_geral = new QWidget();
        tab_geral->setObjectName(QString::fromUtf8("tab_geral"));
        horizontalLayoutWidget_2 = new QWidget(tab_geral);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(420, 160, 421, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_3 = new QLabel(horizontalLayoutWidget_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_cor->setObjectName(QString::fromUtf8("checkbox_cor"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy1);

        horizontalLayout_7->addWidget(checkbox_cor);


        horizontalLayout_2->addLayout(horizontalLayout_7);

        botao_cor = new QPushButton(horizontalLayoutWidget_2);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        slider_alfa = new QSlider(horizontalLayoutWidget_2);
        slider_alfa->setObjectName(QString::fromUtf8("slider_alfa"));
        sizePolicy1.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy1);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(slider_alfa);

        horizontalLayoutWidget_7 = new QWidget(tab_geral);
        horizontalLayoutWidget_7->setObjectName(QString::fromUtf8("horizontalLayoutWidget_7"));
        horizontalLayoutWidget_7->setGeometry(QRect(10, 250, 361, 41));
        horizontalLayout_8 = new QHBoxLayout(horizontalLayoutWidget_7);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        checkbox_selecionavel = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_selecionavel->setObjectName(QString::fromUtf8("checkbox_selecionavel"));

        horizontalLayout_8->addWidget(checkbox_selecionavel);

        checkbox_voadora = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_voadora->setObjectName(QString::fromUtf8("checkbox_voadora"));

        horizontalLayout_8->addWidget(checkbox_voadora);

        checkbox_visibilidade = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_visibilidade->setObjectName(QString::fromUtf8("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);

        horizontalLayout_8->addWidget(checkbox_visibilidade);

        layoutWidget = new QWidget(tab_geral);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(420, 240, 153, 78));
        horizontalLayout_24 = new QHBoxLayout(layoutWidget);
        horizontalLayout_24->setObjectName(QString::fromUtf8("horizontalLayout_24"));
        horizontalLayout_24->setContentsMargins(0, 0, 0, 0);
        label_15 = new QLabel(layoutWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_24->addWidget(label_15);

        combo_textura = new QComboBox(layoutWidget);
        combo_textura->setObjectName(QString::fromUtf8("combo_textura"));

        horizontalLayout_24->addWidget(combo_textura);

        horizontalLayoutWidget_8 = new QWidget(tab_geral);
        horizontalLayoutWidget_8->setObjectName(QString::fromUtf8("horizontalLayoutWidget_8"));
        horizontalLayoutWidget_8->setGeometry(QRect(10, 210, 365, 41));
        horizontalLayout_11 = new QHBoxLayout(horizontalLayoutWidget_8);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_5 = new QLabel(horizontalLayoutWidget_8);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_5);

        spin_pontos_vida = new QSpinBox(horizontalLayoutWidget_8);
        spin_pontos_vida->setObjectName(QString::fromUtf8("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_pontos_vida);

        label_6 = new QLabel(horizontalLayoutWidget_8);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_6);

        spin_max_pontos_vida = new QSpinBox(horizontalLayoutWidget_8);
        spin_max_pontos_vida->setObjectName(QString::fromUtf8("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_max_pontos_vida);

        label_26 = new QLabel(horizontalLayoutWidget_8);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        sizePolicy.setHeightForWidth(label_26->sizePolicy().hasHeightForWidth());
        label_26->setSizePolicy(sizePolicy);
        label_26->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_26);

        spin_pontos_vida_temporarios = new QSpinBox(horizontalLayoutWidget_8);
        spin_pontos_vida_temporarios->setObjectName(QString::fromUtf8("spin_pontos_vida_temporarios"));
        spin_pontos_vida_temporarios->setMinimum(-100);
        spin_pontos_vida_temporarios->setMaximum(999);

        horizontalLayout_12->addWidget(spin_pontos_vida_temporarios);


        horizontalLayout_11->addLayout(horizontalLayout_12);

        horizontalLayoutWidget_4 = new QWidget(tab_geral);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(10, 170, 361, 41));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(horizontalLayoutWidget_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_2);

        label_tamanho = new QLabel(horizontalLayoutWidget_4);
        label_tamanho->setObjectName(QString::fromUtf8("label_tamanho"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_tamanho->sizePolicy().hasHeightForWidth());
        label_tamanho->setSizePolicy(sizePolicy3);
        label_tamanho->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_tamanho);


        horizontalLayout_4->addLayout(horizontalLayout_5);

        slider_tamanho = new QSlider(horizontalLayoutWidget_4);
        slider_tamanho->setObjectName(QString::fromUtf8("slider_tamanho"));
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
        horizontalLayoutWidget_10->setObjectName(QString::fromUtf8("horizontalLayoutWidget_10"));
        horizontalLayoutWidget_10->setGeometry(QRect(10, 410, 361, 35));
        horizontalLayout_16 = new QHBoxLayout(horizontalLayoutWidget_10);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        horizontalLayout_16->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(horizontalLayoutWidget_10);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_16->addWidget(label_9);

        combo_salvacao = new QComboBox(horizontalLayoutWidget_10);
        combo_salvacao->setObjectName(QString::fromUtf8("combo_salvacao"));

        horizontalLayout_16->addWidget(combo_salvacao);

        horizontalLayoutWidget_6 = new QWidget(tab_geral);
        horizontalLayoutWidget_6->setObjectName(QString::fromUtf8("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(10, 380, 361, 31));
        horizontalLayout_13 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(horizontalLayoutWidget_6);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_7);

        spin_translacao_quad = new QDoubleSpinBox(horizontalLayoutWidget_6);
        spin_translacao_quad->setObjectName(QString::fromUtf8("spin_translacao_quad"));
        spin_translacao_quad->setDecimals(1);
        spin_translacao_quad->setMinimum(-100);
        spin_translacao_quad->setMaximum(100);
        spin_translacao_quad->setSingleStep(0.1);

        horizontalLayout_13->addWidget(spin_translacao_quad);

        label_34 = new QLabel(horizontalLayoutWidget_6);
        label_34->setObjectName(QString::fromUtf8("label_34"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_34->sizePolicy().hasHeightForWidth());
        label_34->setSizePolicy(sizePolicy5);
        label_34->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_34);

        horizontalLayoutWidget_11 = new QWidget(tab_geral);
        horizontalLayoutWidget_11->setObjectName(QString::fromUtf8("horizontalLayoutWidget_11"));
        horizontalLayoutWidget_11->setGeometry(QRect(10, 340, 387, 35));
        horizontalLayout_20 = new QHBoxLayout(horizontalLayoutWidget_11);
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        horizontalLayout_20->setContentsMargins(0, 0, 0, 0);
        label_13 = new QLabel(horizontalLayoutWidget_11);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        sizePolicy3.setHeightForWidth(label_13->sizePolicy().hasHeightForWidth());
        label_13->setSizePolicy(sizePolicy3);
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_13);

        combo_visao = new QComboBox(horizontalLayoutWidget_11);
        combo_visao->setObjectName(QString::fromUtf8("combo_visao"));

        horizontalLayout_20->addWidget(combo_visao);

        label_14 = new QLabel(horizontalLayoutWidget_11);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        sizePolicy5.setHeightForWidth(label_14->sizePolicy().hasHeightForWidth());
        label_14->setSizePolicy(sizePolicy5);
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_14);

        spin_raio_visao_escuro_quad = new QDoubleSpinBox(horizontalLayoutWidget_11);
        spin_raio_visao_escuro_quad->setObjectName(QString::fromUtf8("spin_raio_visao_escuro_quad"));
        spin_raio_visao_escuro_quad->setDecimals(1);
        spin_raio_visao_escuro_quad->setSingleStep(1);

        horizontalLayout_20->addWidget(spin_raio_visao_escuro_quad);

        label_33 = new QLabel(horizontalLayoutWidget_11);
        label_33->setObjectName(QString::fromUtf8("label_33"));
        sizePolicy5.setHeightForWidth(label_33->sizePolicy().hasHeightForWidth());
        label_33->setSizePolicy(sizePolicy5);
        label_33->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_33);

        gridLayoutWidget = new QWidget(tab_geral);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(560, 240, 281, 81));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_22 = new QHBoxLayout();
        horizontalLayout_22->setObjectName(QString::fromUtf8("horizontalLayout_22"));
        label_17 = new QLabel(gridLayoutWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_22->addWidget(label_17);

        spin_tex_altura = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_altura->setObjectName(QString::fromUtf8("spin_tex_altura"));
        spin_tex_altura->setDecimals(2);
        spin_tex_altura->setMaximum(1);
        spin_tex_altura->setSingleStep(0.1);

        horizontalLayout_22->addWidget(spin_tex_altura);


        gridLayout->addLayout(horizontalLayout_22, 1, 1, 1, 1);

        horizontalLayout_26 = new QHBoxLayout();
        horizontalLayout_26->setObjectName(QString::fromUtf8("horizontalLayout_26"));
        label_19 = new QLabel(gridLayoutWidget);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_26->addWidget(label_19);

        spin_tex_trans_x = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_trans_x->setObjectName(QString::fromUtf8("spin_tex_trans_x"));
        spin_tex_trans_x->setDecimals(2);
        spin_tex_trans_x->setMinimum(-1);
        spin_tex_trans_x->setMaximum(1);
        spin_tex_trans_x->setSingleStep(0.1);

        horizontalLayout_26->addWidget(spin_tex_trans_x);


        gridLayout->addLayout(horizontalLayout_26, 0, 0, 1, 1);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName(QString::fromUtf8("horizontalLayout_25"));
        horizontalLayout_27 = new QHBoxLayout();
        horizontalLayout_27->setObjectName(QString::fromUtf8("horizontalLayout_27"));
        label_20 = new QLabel(gridLayoutWidget);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_27->addWidget(label_20);

        spin_tex_trans_y = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_trans_y->setObjectName(QString::fromUtf8("spin_tex_trans_y"));
        spin_tex_trans_y->setDecimals(2);
        spin_tex_trans_y->setMinimum(-1);
        spin_tex_trans_y->setMaximum(1);
        spin_tex_trans_y->setSingleStep(0.1);

        horizontalLayout_27->addWidget(spin_tex_trans_y);


        horizontalLayout_25->addLayout(horizontalLayout_27);


        gridLayout->addLayout(horizontalLayout_25, 1, 0, 1, 1);

        horizontalLayout_23 = new QHBoxLayout();
        horizontalLayout_23->setObjectName(QString::fromUtf8("horizontalLayout_23"));
        label_18 = new QLabel(gridLayoutWidget);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_23->addWidget(label_18);

        spin_tex_largura = new QDoubleSpinBox(gridLayoutWidget);
        spin_tex_largura->setObjectName(QString::fromUtf8("spin_tex_largura"));
        spin_tex_largura->setDecimals(2);
        spin_tex_largura->setMaximum(1);
        spin_tex_largura->setSingleStep(0.1);

        horizontalLayout_23->addWidget(spin_tex_largura);


        gridLayout->addLayout(horizontalLayout_23, 0, 1, 1, 1);

        layoutWidget_2 = new QWidget(tab_geral);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(420, 10, 421, 141));
        horizontalLayout_19 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        horizontalLayout_19->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(layoutWidget_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        sizePolicy2.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy2);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_19->addWidget(label_11);

        lista_eventos = new QPlainTextEdit(layoutWidget_2);
        lista_eventos->setObjectName(QString::fromUtf8("lista_eventos"));

        horizontalLayout_19->addWidget(lista_eventos);

        horizontalLayoutWidget_3 = new QWidget(tab_geral);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(420, 200, 421, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_12 = new QLabel(horizontalLayoutWidget_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(label_12);

        spin_raio_quad = new QDoubleSpinBox(horizontalLayoutWidget_3);
        spin_raio_quad->setObjectName(QString::fromUtf8("spin_raio_quad"));
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1);

        horizontalLayout_14->addWidget(spin_raio_quad);

        label_31 = new QLabel(horizontalLayoutWidget_3);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        sizePolicy5.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy5);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(label_31);

        botao_luz = new QPushButton(horizontalLayoutWidget_3);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_14->addWidget(botao_luz);


        horizontalLayout_3->addLayout(horizontalLayout_14);

        horizontalLayoutWidget_9 = new QWidget(tab_geral);
        horizontalLayoutWidget_9->setObjectName(QString::fromUtf8("horizontalLayoutWidget_9"));
        horizontalLayoutWidget_9->setGeometry(QRect(420, 320, 421, 41));
        horizontalLayout_21 = new QHBoxLayout(horizontalLayoutWidget_9);
        horizontalLayout_21->setObjectName(QString::fromUtf8("horizontalLayout_21"));
        horizontalLayout_21->setContentsMargins(0, 0, 0, 0);
        label_16 = new QLabel(horizontalLayoutWidget_9);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_16);

        combo_modelos_3d = new QComboBox(horizontalLayoutWidget_9);
        combo_modelos_3d->setObjectName(QString::fromUtf8("combo_modelos_3d"));

        horizontalLayout_21->addWidget(combo_modelos_3d);

        layoutWidget1 = new QWidget(tab_geral);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 290, 361, 39));
        horizontalLayout_9 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
        checkbox_caida = new QCheckBox(layoutWidget1);
        checkbox_caida->setObjectName(QString::fromUtf8("checkbox_caida"));

        horizontalLayout_9->addWidget(checkbox_caida);

        checkbox_morta = new QCheckBox(layoutWidget1);
        checkbox_morta->setObjectName(QString::fromUtf8("checkbox_morta"));

        horizontalLayout_9->addWidget(checkbox_morta);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_4 = new QLabel(layoutWidget1);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_4);

        spin_aura_quad = new QDoubleSpinBox(layoutWidget1);
        spin_aura_quad->setObjectName(QString::fromUtf8("spin_aura_quad"));
        spin_aura_quad->setDecimals(1);
        spin_aura_quad->setSingleStep(1);

        horizontalLayout_10->addWidget(spin_aura_quad);

        label_35 = new QLabel(layoutWidget1);
        label_35->setObjectName(QString::fromUtf8("label_35"));
        sizePolicy5.setHeightForWidth(label_35->sizePolicy().hasHeightForWidth());
        label_35->setSizePolicy(sizePolicy5);
        label_35->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_35);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        verticalLayoutWidget = new QWidget(tab_geral);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 361, 161));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_15->addWidget(label);

        campo_id = new QLineEdit(verticalLayoutWidget);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        sizePolicy1.setHeightForWidth(campo_id->sizePolicy().hasHeightForWidth());
        campo_id->setSizePolicy(sizePolicy1);
        campo_id->setMinimumSize(QSize(0, 0));
        campo_id->setMaximumSize(QSize(70, 16777215));
        campo_id->setReadOnly(true);

        horizontalLayout_15->addWidget(campo_id);

        label_8 = new QLabel(verticalLayoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        QSizePolicy sizePolicy6(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(0);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy6);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_8);

        campo_rotulo = new QLineEdit(verticalLayoutWidget);
        campo_rotulo->setObjectName(QString::fromUtf8("campo_rotulo"));
        sizePolicy4.setHeightForWidth(campo_rotulo->sizePolicy().hasHeightForWidth());
        campo_rotulo->setSizePolicy(sizePolicy4);
        campo_rotulo->setReadOnly(false);

        horizontalLayout_15->addWidget(campo_rotulo);


        verticalLayout->addLayout(horizontalLayout_15);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        label_10 = new QLabel(verticalLayoutWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy2.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy2);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_10);

        lista_rotulos = new QPlainTextEdit(verticalLayoutWidget);
        lista_rotulos->setObjectName(QString::fromUtf8("lista_rotulos"));

        horizontalLayout_18->addWidget(lista_rotulos);


        verticalLayout->addLayout(horizontalLayout_18);

        tab_tesouro->addTab(tab_geral, QString());
        tab_nivel = new QWidget();
        tab_nivel->setObjectName(QString::fromUtf8("tab_nivel"));
        horizontalLayoutWidget_5 = new QWidget(tab_nivel);
        horizontalLayoutWidget_5->setObjectName(QString::fromUtf8("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(10, 10, 181, 41));
        horizontalLayout_31 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_31->setObjectName(QString::fromUtf8("horizontalLayout_31"));
        horizontalLayout_31->setContentsMargins(0, 0, 0, 0);
        label_39 = new QLabel(horizontalLayoutWidget_5);
        label_39->setObjectName(QString::fromUtf8("label_39"));

        horizontalLayout_31->addWidget(label_39, 0, Qt::AlignRight);

        linha_nivel = new QLineEdit(horizontalLayoutWidget_5);
        linha_nivel->setObjectName(QString::fromUtf8("linha_nivel"));
        linha_nivel->setReadOnly(true);

        horizontalLayout_31->addWidget(linha_nivel, 0, Qt::AlignLeft);

        botao_adicionar_nivel = new QPushButton(tab_nivel);
        botao_adicionar_nivel->setObjectName(QString::fromUtf8("botao_adicionar_nivel"));
        botao_adicionar_nivel->setEnabled(true);
        botao_adicionar_nivel->setGeometry(QRect(190, 350, 121, 27));
        lista_niveis = new QListWidget(tab_nivel);
        lista_niveis->setObjectName(QString::fromUtf8("lista_niveis"));
        lista_niveis->setGeometry(QRect(10, 160, 451, 181));
        botao_remover_nivel = new QPushButton(tab_nivel);
        botao_remover_nivel->setObjectName(QString::fromUtf8("botao_remover_nivel"));
        botao_remover_nivel->setEnabled(false);
        botao_remover_nivel->setGeometry(QRect(340, 350, 121, 27));
        horizontalLayoutWidget_12 = new QWidget(tab_nivel);
        horizontalLayoutWidget_12->setObjectName(QString::fromUtf8("horizontalLayoutWidget_12"));
        horizontalLayoutWidget_12->setGeometry(QRect(210, 10, 181, 41));
        horizontalLayout_33 = new QHBoxLayout(horizontalLayoutWidget_12);
        horizontalLayout_33->setObjectName(QString::fromUtf8("horizontalLayout_33"));
        horizontalLayout_33->setContentsMargins(0, 0, 0, 0);
        label_46 = new QLabel(horizontalLayoutWidget_12);
        label_46->setObjectName(QString::fromUtf8("label_46"));

        horizontalLayout_33->addWidget(label_46, 0, Qt::AlignRight);

        linha_bba = new QLineEdit(horizontalLayoutWidget_12);
        linha_bba->setObjectName(QString::fromUtf8("linha_bba"));
        linha_bba->setReadOnly(true);

        horizontalLayout_33->addWidget(linha_bba, 0, Qt::AlignLeft);

        groupBox_2 = new QGroupBox(tab_nivel);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 70, 661, 61));
        horizontalLayoutWidget = new QWidget(groupBox_2);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(9, 20, 650, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_40 = new QLabel(horizontalLayoutWidget);
        label_40->setObjectName(QString::fromUtf8("label_40"));
        sizePolicy5.setHeightForWidth(label_40->sizePolicy().hasHeightForWidth());
        label_40->setSizePolicy(sizePolicy5);

        horizontalLayout->addWidget(label_40);

        linha_classe = new QLineEdit(horizontalLayoutWidget);
        linha_classe->setObjectName(QString::fromUtf8("linha_classe"));
        sizePolicy1.setHeightForWidth(linha_classe->sizePolicy().hasHeightForWidth());
        linha_classe->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(linha_classe);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label_41 = new QLabel(horizontalLayoutWidget);
        label_41->setObjectName(QString::fromUtf8("label_41"));
        label_41->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_41);

        spin_nivel_classe = new QSpinBox(horizontalLayoutWidget);
        spin_nivel_classe->setObjectName(QString::fromUtf8("spin_nivel_classe"));
        spin_nivel_classe->setMinimum(-1);

        horizontalLayout->addWidget(spin_nivel_classe);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        label_43 = new QLabel(horizontalLayoutWidget);
        label_43->setObjectName(QString::fromUtf8("label_43"));
        label_43->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_43);

        spin_bba = new QSpinBox(horizontalLayoutWidget);
        spin_bba->setObjectName(QString::fromUtf8("spin_bba"));
        spin_bba->setMinimum(-1);

        horizontalLayout->addWidget(spin_bba);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_7);

        label_42 = new QLabel(horizontalLayoutWidget);
        label_42->setObjectName(QString::fromUtf8("label_42"));
        label_42->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_42);

        spin_nivel_conjurador = new QSpinBox(horizontalLayoutWidget);
        spin_nivel_conjurador->setObjectName(QString::fromUtf8("spin_nivel_conjurador"));
        spin_nivel_conjurador->setMinimum(-1);

        horizontalLayout->addWidget(spin_nivel_conjurador);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_8);

        label_44 = new QLabel(horizontalLayoutWidget);
        label_44->setObjectName(QString::fromUtf8("label_44"));
        label_44->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_44);

        spin_mod_conjuracao = new QSpinBox(horizontalLayoutWidget);
        spin_mod_conjuracao->setObjectName(QString::fromUtf8("spin_mod_conjuracao"));
        spin_mod_conjuracao->setMinimum(-1);

        horizontalLayout->addWidget(spin_mod_conjuracao);

        tab_tesouro->addTab(tab_nivel, QString());
        tab_estatisticas = new QWidget();
        tab_estatisticas->setObjectName(QString::fromUtf8("tab_estatisticas"));
        layoutWidget_3 = new QWidget(tab_estatisticas);
        layoutWidget_3->setObjectName(QString::fromUtf8("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(10, 10, 421, 31));
        horizontalLayout_28 = new QHBoxLayout(layoutWidget_3);
        horizontalLayout_28->setObjectName(QString::fromUtf8("horizontalLayout_28"));
        horizontalLayout_28->setContentsMargins(0, 0, 0, 0);
        checkbox_iniciativa = new QCheckBox(layoutWidget_3);
        checkbox_iniciativa->setObjectName(QString::fromUtf8("checkbox_iniciativa"));
        sizePolicy1.setHeightForWidth(checkbox_iniciativa->sizePolicy().hasHeightForWidth());
        checkbox_iniciativa->setSizePolicy(sizePolicy1);

        horizontalLayout_28->addWidget(checkbox_iniciativa);

        spin_iniciativa = new QSpinBox(layoutWidget_3);
        spin_iniciativa->setObjectName(QString::fromUtf8("spin_iniciativa"));
        spin_iniciativa->setMinimum(-100);
        spin_iniciativa->setMaximum(999);

        horizontalLayout_28->addWidget(spin_iniciativa);

        label_22 = new QLabel(layoutWidget_3);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        sizePolicy.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy);
        label_22->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_28->addWidget(label_22);

        spin_modificador_iniciativa = new QSpinBox(layoutWidget_3);
        spin_modificador_iniciativa->setObjectName(QString::fromUtf8("spin_modificador_iniciativa"));
        spin_modificador_iniciativa->setMinimum(-100);
        spin_modificador_iniciativa->setMaximum(999);

        horizontalLayout_28->addWidget(spin_modificador_iniciativa);

        botao_remover_ataque = new QPushButton(tab_estatisticas);
        botao_remover_ataque->setObjectName(QString::fromUtf8("botao_remover_ataque"));
        botao_remover_ataque->setEnabled(false);
        botao_remover_ataque->setGeometry(QRect(749, 460, 121, 27));
        lista_ataques = new QListWidget(tab_estatisticas);
        lista_ataques->setObjectName(QString::fromUtf8("lista_ataques"));
        lista_ataques->setGeometry(QRect(349, 270, 511, 181));
        checkbox_imune_critico = new QCheckBox(tab_estatisticas);
        checkbox_imune_critico->setObjectName(QString::fromUtf8("checkbox_imune_critico"));
        checkbox_imune_critico->setGeometry(QRect(20, 460, 121, 22));
        botao_ataque_cima = new QPushButton(tab_estatisticas);
        botao_ataque_cima->setObjectName(QString::fromUtf8("botao_ataque_cima"));
        botao_ataque_cima->setGeometry(QRect(870, 320, 31, 27));
        botao_ataque_baixo = new QPushButton(tab_estatisticas);
        botao_ataque_baixo->setObjectName(QString::fromUtf8("botao_ataque_baixo"));
        botao_ataque_baixo->setGeometry(QRect(870, 360, 31, 27));
        botao_clonar_ataque = new QPushButton(tab_estatisticas);
        botao_clonar_ataque->setObjectName(QString::fromUtf8("botao_clonar_ataque"));
        botao_clonar_ataque->setEnabled(true);
        botao_clonar_ataque->setGeometry(QRect(619, 460, 121, 27));
        groupBox = new QGroupBox(tab_estatisticas);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(350, 160, 541, 101));
        verticalLayoutWidget_2 = new QWidget(groupBox);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(0, 20, 509, 80));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_29 = new QHBoxLayout();
        horizontalLayout_29->setObjectName(QString::fromUtf8("horizontalLayout_29"));
        label_36 = new QLabel(verticalLayoutWidget_2);
        label_36->setObjectName(QString::fromUtf8("label_36"));
        sizePolicy5.setHeightForWidth(label_36->sizePolicy().hasHeightForWidth());
        label_36->setSizePolicy(sizePolicy5);

        horizontalLayout_29->addWidget(label_36);

        linha_rotulo_ataque = new QLineEdit(verticalLayoutWidget_2);
        linha_rotulo_ataque->setObjectName(QString::fromUtf8("linha_rotulo_ataque"));
        sizePolicy1.setHeightForWidth(linha_rotulo_ataque->sizePolicy().hasHeightForWidth());
        linha_rotulo_ataque->setSizePolicy(sizePolicy1);

        horizontalLayout_29->addWidget(linha_rotulo_ataque);

        label_21 = new QLabel(verticalLayoutWidget_2);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        sizePolicy5.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy5);

        horizontalLayout_29->addWidget(label_21);

        combo_tipo_ataque = new QComboBox(verticalLayoutWidget_2);
        combo_tipo_ataque->setObjectName(QString::fromUtf8("combo_tipo_ataque"));
        sizePolicy1.setHeightForWidth(combo_tipo_ataque->sizePolicy().hasHeightForWidth());
        combo_tipo_ataque->setSizePolicy(sizePolicy1);

        horizontalLayout_29->addWidget(combo_tipo_ataque);

        checkbox_permite_escudo = new QCheckBox(verticalLayoutWidget_2);
        checkbox_permite_escudo->setObjectName(QString::fromUtf8("checkbox_permite_escudo"));

        horizontalLayout_29->addWidget(checkbox_permite_escudo);


        verticalLayout_2->addLayout(horizontalLayout_29);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_29 = new QLabel(verticalLayoutWidget_2);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_29);

        spin_alcance_quad = new QSpinBox(verticalLayoutWidget_2);
        spin_alcance_quad->setObjectName(QString::fromUtf8("spin_alcance_quad"));
        spin_alcance_quad->setMinimum(-1);

        horizontalLayout_6->addWidget(spin_alcance_quad);

        label_30 = new QLabel(verticalLayoutWidget_2);
        label_30->setObjectName(QString::fromUtf8("label_30"));

        horizontalLayout_6->addWidget(label_30);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);

        label_32 = new QLabel(verticalLayoutWidget_2);
        label_32->setObjectName(QString::fromUtf8("label_32"));
        label_32->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_32);

        spin_incrementos = new QSpinBox(verticalLayoutWidget_2);
        spin_incrementos->setObjectName(QString::fromUtf8("spin_incrementos"));

        horizontalLayout_6->addWidget(spin_incrementos);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);

        label_23 = new QLabel(verticalLayoutWidget_2);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_23);

        spin_ataque = new QSpinBox(verticalLayoutWidget_2);
        spin_ataque->setObjectName(QString::fromUtf8("spin_ataque"));
        spin_ataque->setMinimum(-50);
        spin_ataque->setMaximum(50);

        horizontalLayout_6->addWidget(spin_ataque);

        label_24 = new QLabel(verticalLayoutWidget_2);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_6->addWidget(label_24);

        linha_dano = new QLineEdit(verticalLayoutWidget_2);
        linha_dano->setObjectName(QString::fromUtf8("linha_dano"));

        horizontalLayout_6->addWidget(linha_dano);


        verticalLayout_2->addLayout(horizontalLayout_6);

        label_38 = new QLabel(tab_estatisticas);
        label_38->setObjectName(QString::fromUtf8("label_38"));
        label_38->setGeometry(QRect(353, 457, 41, 34));
        label_38->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        linha_furtivo = new QLineEdit(tab_estatisticas);
        linha_furtivo->setObjectName(QString::fromUtf8("linha_furtivo"));
        linha_furtivo->setGeometry(QRect(400, 460, 129, 27));
        groupBox_3 = new QGroupBox(tab_estatisticas);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(340, 50, 551, 71));
        label_25 = new QLabel(groupBox_3);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setGeometry(QRect(10, 10, 31, 34));
        label_25->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        gridLayoutWidget_3 = new QWidget(groupBox_3);
        gridLayoutWidget_3->setObjectName(QString::fromUtf8("gridLayoutWidget_3"));
        gridLayoutWidget_3->setGeometry(QRect(10, 10, 533, 61));
        gridLayout_3 = new QGridLayout(gridLayoutWidget_3);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        spin_bonus_escudo = new QLabel(gridLayoutWidget_3);
        spin_bonus_escudo->setObjectName(QString::fromUtf8("spin_bonus_escudo"));
        spin_bonus_escudo->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(spin_bonus_escudo, 0, 2, 1, 1);

        label_57 = new QLabel(gridLayoutWidget_3);
        label_57->setObjectName(QString::fromUtf8("label_57"));
        QFont font;
        font.setFamily(QString::fromUtf8("Noto Sans [unknown]"));
        font.setBold(true);
        font.setWeight(75);
        label_57->setFont(font);
        label_57->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_57, 0, 7, 1, 1);

        spin_ca_escudo = new QSpinBox(gridLayoutWidget_3);
        spin_ca_escudo->setObjectName(QString::fromUtf8("spin_ca_escudo"));
        spin_ca_escudo->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_escudo, 1, 2, 1, 1);

        spin_ca_armadura = new QSpinBox(gridLayoutWidget_3);
        spin_ca_armadura->setObjectName(QString::fromUtf8("spin_ca_armadura"));
        spin_ca_armadura->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_armadura, 1, 1, 1, 1);

        spin_ca_toque = new QSpinBox(gridLayoutWidget_3);
        spin_ca_toque->setObjectName(QString::fromUtf8("spin_ca_toque"));
        spin_ca_toque->setReadOnly(true);
        spin_ca_toque->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_toque, 1, 8, 1, 1);

        spin_ca_destreza = new QSpinBox(gridLayoutWidget_3);
        spin_ca_destreza->setObjectName(QString::fromUtf8("spin_ca_destreza"));
        spin_ca_destreza->setEnabled(true);
        spin_ca_destreza->setReadOnly(true);
        spin_ca_destreza->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_destreza, 1, 3, 1, 1);

        label_52 = new QLabel(gridLayoutWidget_3);
        label_52->setObjectName(QString::fromUtf8("label_52"));
        label_52->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_52, 0, 1, 1, 1);

        label_58 = new QLabel(gridLayoutWidget_3);
        label_58->setObjectName(QString::fromUtf8("label_58"));
        label_58->setFont(font);
        label_58->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_58, 0, 8, 1, 1);

        spin_ca_total = new QSpinBox(gridLayoutWidget_3);
        spin_ca_total->setObjectName(QString::fromUtf8("spin_ca_total"));
        spin_ca_total->setReadOnly(true);
        spin_ca_total->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_total, 1, 7, 1, 1);

        label_56 = new QLabel(gridLayoutWidget_3);
        label_56->setObjectName(QString::fromUtf8("label_56"));
        label_56->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_56, 0, 5, 1, 1);

        label_55 = new QLabel(gridLayoutWidget_3);
        label_55->setObjectName(QString::fromUtf8("label_55"));
        label_55->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_55, 0, 4, 1, 1);

        spin_ca_tamanho = new QSpinBox(gridLayoutWidget_3);
        spin_ca_tamanho->setObjectName(QString::fromUtf8("spin_ca_tamanho"));
        spin_ca_tamanho->setEnabled(true);
        spin_ca_tamanho->setReadOnly(true);
        spin_ca_tamanho->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_tamanho, 1, 4, 1, 1);

        label_54 = new QLabel(gridLayoutWidget_3);
        label_54->setObjectName(QString::fromUtf8("label_54"));
        label_54->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_54, 0, 3, 1, 1);

        spin_ca_surpreso = new QSpinBox(gridLayoutWidget_3);
        spin_ca_surpreso->setObjectName(QString::fromUtf8("spin_ca_surpreso"));
        spin_ca_surpreso->setReadOnly(true);
        spin_ca_surpreso->setMinimum(-99);

        gridLayout_3->addWidget(spin_ca_surpreso, 1, 9, 1, 1);

        label_59 = new QLabel(gridLayoutWidget_3);
        label_59->setObjectName(QString::fromUtf8("label_59"));
        label_59->setFont(font);
        label_59->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_59, 0, 9, 1, 1);

        label_53 = new QLabel(gridLayoutWidget_3);
        label_53->setObjectName(QString::fromUtf8("label_53"));
        label_53->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_53, 0, 0, 1, 1);

        label_60 = new QLabel(gridLayoutWidget_3);
        label_60->setObjectName(QString::fromUtf8("label_60"));
        label_60->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_60, 1, 0, 1, 1);

        label_61 = new QLabel(gridLayoutWidget_3);
        label_61->setObjectName(QString::fromUtf8("label_61"));
        label_61->setAlignment(Qt::AlignCenter);

        gridLayout_3->addWidget(label_61, 1, 6, 1, 1);

        botao_bonus_ca = new QPushButton(gridLayoutWidget_3);
        botao_bonus_ca->setObjectName(QString::fromUtf8("botao_bonus_ca"));

        gridLayout_3->addWidget(botao_bonus_ca, 1, 5, 1, 1);

        groupBox_4 = new QGroupBox(tab_estatisticas);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setGeometry(QRect(10, 50, 221, 261));
        gridLayoutWidget_2 = new QWidget(groupBox_4);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(9, 10, 211, 241));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        label_47 = new QLabel(gridLayoutWidget_2);
        label_47->setObjectName(QString::fromUtf8("label_47"));
        label_47->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_47, 3, 0, 1, 1);

        spin_des = new QSpinBox(gridLayoutWidget_2);
        spin_des->setObjectName(QString::fromUtf8("spin_des"));

        gridLayout_2->addWidget(spin_des, 1, 1, 1, 1);

        label_49 = new QLabel(gridLayoutWidget_2);
        label_49->setObjectName(QString::fromUtf8("label_49"));
        label_49->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_49, 4, 0, 1, 1);

        label_51 = new QLabel(gridLayoutWidget_2);
        label_51->setObjectName(QString::fromUtf8("label_51"));
        label_51->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_51, 5, 0, 1, 1);

        botao_bonus_con = new QPushButton(gridLayoutWidget_2);
        botao_bonus_con->setObjectName(QString::fromUtf8("botao_bonus_con"));

        gridLayout_2->addWidget(botao_bonus_con, 2, 2, 1, 1);

        label_50 = new QLabel(gridLayoutWidget_2);
        label_50->setObjectName(QString::fromUtf8("label_50"));
        label_50->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_50, 2, 0, 1, 1);

        label_mod_des = new QLabel(gridLayoutWidget_2);
        label_mod_des->setObjectName(QString::fromUtf8("label_mod_des"));
        label_mod_des->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_des, 1, 3, 1, 1);

        botao_bonus_des = new QPushButton(gridLayoutWidget_2);
        botao_bonus_des->setObjectName(QString::fromUtf8("botao_bonus_des"));

        gridLayout_2->addWidget(botao_bonus_des, 1, 2, 1, 1);

        spin_car = new QSpinBox(gridLayoutWidget_2);
        spin_car->setObjectName(QString::fromUtf8("spin_car"));

        gridLayout_2->addWidget(spin_car, 5, 1, 1, 1);

        spin_sab = new QSpinBox(gridLayoutWidget_2);
        spin_sab->setObjectName(QString::fromUtf8("spin_sab"));

        gridLayout_2->addWidget(spin_sab, 4, 1, 1, 1);

        spin_int = new QSpinBox(gridLayoutWidget_2);
        spin_int->setObjectName(QString::fromUtf8("spin_int"));

        gridLayout_2->addWidget(spin_int, 3, 1, 1, 1);

        botao_bonus_for = new QPushButton(gridLayoutWidget_2);
        botao_bonus_for->setObjectName(QString::fromUtf8("botao_bonus_for"));

        gridLayout_2->addWidget(botao_bonus_for, 0, 2, 1, 1);

        label_27 = new QLabel(gridLayoutWidget_2);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_27, 0, 0, 1, 1);

        spin_for = new QSpinBox(gridLayoutWidget_2);
        spin_for->setObjectName(QString::fromUtf8("spin_for"));

        gridLayout_2->addWidget(spin_for, 0, 1, 1, 1);

        label_mod_for = new QLabel(gridLayoutWidget_2);
        label_mod_for->setObjectName(QString::fromUtf8("label_mod_for"));
        label_mod_for->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_for, 0, 3, 1, 1);

        spin_con = new QSpinBox(gridLayoutWidget_2);
        spin_con->setObjectName(QString::fromUtf8("spin_con"));

        gridLayout_2->addWidget(spin_con, 2, 1, 1, 1);

        label_mod_sab = new QLabel(gridLayoutWidget_2);
        label_mod_sab->setObjectName(QString::fromUtf8("label_mod_sab"));
        label_mod_sab->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_sab, 4, 3, 1, 1);

        label_48 = new QLabel(gridLayoutWidget_2);
        label_48->setObjectName(QString::fromUtf8("label_48"));
        label_48->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_48, 1, 0, 1, 1);

        label_mod_int = new QLabel(gridLayoutWidget_2);
        label_mod_int->setObjectName(QString::fromUtf8("label_mod_int"));
        label_mod_int->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_int, 3, 3, 1, 1);

        botao_bonus_sab = new QPushButton(gridLayoutWidget_2);
        botao_bonus_sab->setObjectName(QString::fromUtf8("botao_bonus_sab"));

        gridLayout_2->addWidget(botao_bonus_sab, 4, 2, 1, 1);

        botao_bonus_int = new QPushButton(gridLayoutWidget_2);
        botao_bonus_int->setObjectName(QString::fromUtf8("botao_bonus_int"));

        gridLayout_2->addWidget(botao_bonus_int, 3, 2, 1, 1);

        label_mod_con = new QLabel(gridLayoutWidget_2);
        label_mod_con->setObjectName(QString::fromUtf8("label_mod_con"));
        label_mod_con->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_con, 2, 3, 1, 1);

        label_mod_car = new QLabel(gridLayoutWidget_2);
        label_mod_car->setObjectName(QString::fromUtf8("label_mod_car"));
        label_mod_car->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_mod_car, 5, 3, 1, 1);

        botao_bonus_car = new QPushButton(gridLayoutWidget_2);
        botao_bonus_car->setObjectName(QString::fromUtf8("botao_bonus_car"));

        gridLayout_2->addWidget(botao_bonus_car, 5, 2, 1, 1);

        tab_tesouro->addTab(tab_estatisticas, QString());
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        layoutWidget_4 = new QWidget(tab);
        layoutWidget_4->setObjectName(QString::fromUtf8("layoutWidget_4"));
        layoutWidget_4->setGeometry(QRect(0, 10, 481, 221));
        horizontalLayout_30 = new QHBoxLayout(layoutWidget_4);
        horizontalLayout_30->setObjectName(QString::fromUtf8("horizontalLayout_30"));
        horizontalLayout_30->setContentsMargins(0, 0, 0, 0);
        label_37 = new QLabel(layoutWidget_4);
        label_37->setObjectName(QString::fromUtf8("label_37"));
        sizePolicy5.setHeightForWidth(label_37->sizePolicy().hasHeightForWidth());
        label_37->setSizePolicy(sizePolicy5);
        label_37->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_30->addWidget(label_37);

        lista_tesouro = new QPlainTextEdit(layoutWidget_4);
        lista_tesouro->setObjectName(QString::fromUtf8("lista_tesouro"));

        horizontalLayout_30->addWidget(lista_tesouro);

        tab_tesouro->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        layoutWidget_5 = new QWidget(tab_2);
        layoutWidget_5->setObjectName(QString::fromUtf8("layoutWidget_5"));
        layoutWidget_5->setGeometry(QRect(20, 20, 751, 481));
        horizontalLayout_32 = new QHBoxLayout(layoutWidget_5);
        horizontalLayout_32->setObjectName(QString::fromUtf8("horizontalLayout_32"));
        horizontalLayout_32->setContentsMargins(0, 0, 0, 0);
        label_45 = new QLabel(layoutWidget_5);
        label_45->setObjectName(QString::fromUtf8("label_45"));
        sizePolicy5.setHeightForWidth(label_45->sizePolicy().hasHeightForWidth());
        label_45->setSizePolicy(sizePolicy5);
        label_45->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_32->addWidget(label_45);

        texto_notas = new QPlainTextEdit(layoutWidget_5);
        texto_notas->setObjectName(QString::fromUtf8("texto_notas"));

        horizontalLayout_32->addWidget(texto_notas);

        tab_tesouro->addTab(tab_2, QString());
        QWidget::setTabOrder(campo_rotulo, lista_rotulos);
        QWidget::setTabOrder(lista_rotulos, slider_tamanho);
        QWidget::setTabOrder(slider_tamanho, spin_pontos_vida);
        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, spin_pontos_vida_temporarios);
        QWidget::setTabOrder(spin_pontos_vida_temporarios, checkbox_selecionavel);
        QWidget::setTabOrder(checkbox_selecionavel, checkbox_voadora);
        QWidget::setTabOrder(checkbox_voadora, checkbox_visibilidade);
        QWidget::setTabOrder(checkbox_visibilidade, checkbox_caida);
        QWidget::setTabOrder(checkbox_caida, checkbox_morta);
        QWidget::setTabOrder(checkbox_morta, spin_aura_quad);
        QWidget::setTabOrder(spin_aura_quad, combo_visao);
        QWidget::setTabOrder(combo_visao, spin_raio_visao_escuro_quad);
        QWidget::setTabOrder(spin_raio_visao_escuro_quad, spin_translacao_quad);
        QWidget::setTabOrder(spin_translacao_quad, combo_salvacao);
        QWidget::setTabOrder(combo_salvacao, lista_eventos);
        QWidget::setTabOrder(lista_eventos, checkbox_cor);
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
        QWidget::setTabOrder(spin_nivel_conjurador, spin_mod_conjuracao);
        QWidget::setTabOrder(spin_mod_conjuracao, lista_niveis);
        QWidget::setTabOrder(lista_niveis, botao_adicionar_nivel);
        QWidget::setTabOrder(botao_adicionar_nivel, botao_remover_nivel);
        QWidget::setTabOrder(botao_remover_nivel, checkbox_iniciativa);
        QWidget::setTabOrder(checkbox_iniciativa, spin_iniciativa);
        QWidget::setTabOrder(spin_iniciativa, spin_modificador_iniciativa);
        QWidget::setTabOrder(spin_modificador_iniciativa, spin_for);
        QWidget::setTabOrder(spin_for, spin_ca_armadura);
        QWidget::setTabOrder(spin_ca_armadura, spin_ca_escudo);
        QWidget::setTabOrder(spin_ca_escudo, linha_rotulo_ataque);
        QWidget::setTabOrder(linha_rotulo_ataque, combo_tipo_ataque);
        QWidget::setTabOrder(combo_tipo_ataque, checkbox_permite_escudo);
        QWidget::setTabOrder(checkbox_permite_escudo, spin_alcance_quad);
        QWidget::setTabOrder(spin_alcance_quad, spin_incrementos);
        QWidget::setTabOrder(spin_incrementos, spin_ataque);
        QWidget::setTabOrder(spin_ataque, linha_dano);
        QWidget::setTabOrder(linha_dano, campo_id);
        QWidget::setTabOrder(campo_id, checkbox_imune_critico);
        QWidget::setTabOrder(checkbox_imune_critico, linha_furtivo);
        QWidget::setTabOrder(linha_furtivo, botao_ataque_cima);
        QWidget::setTabOrder(botao_ataque_cima, botao_ataque_baixo);
        QWidget::setTabOrder(botao_ataque_baixo, botao_clonar_ataque);
        QWidget::setTabOrder(botao_clonar_ataque, botao_remover_ataque);
        QWidget::setTabOrder(botao_remover_ataque, lista_tesouro);
        QWidget::setTabOrder(lista_tesouro, texto_notas);
        QWidget::setTabOrder(texto_notas, spin_ca_destreza);
        QWidget::setTabOrder(spin_ca_destreza, spin_ca_tamanho);
        QWidget::setTabOrder(spin_ca_tamanho, spin_ca_total);
        QWidget::setTabOrder(spin_ca_total, spin_ca_toque);
        QWidget::setTabOrder(spin_ca_toque, spin_ca_surpreso);
        QWidget::setTabOrder(spin_ca_surpreso, lista_ataques);
        QWidget::setTabOrder(lista_ataques, botoes);
        QWidget::setTabOrder(botoes, tab_tesouro);
        QWidget::setTabOrder(tab_tesouro, linha_bba);
        QWidget::setTabOrder(linha_bba, linha_nivel);

        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));

        tab_tesouro->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        ifg__qt__DialogoEntidade->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Cor:", 0, QApplication::UnicodeUTF8));
        checkbox_cor->setText(QString());
#ifndef QT_NO_TOOLTIP
        botao_cor->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Cor da entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_selecionavel->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, jogadores poder\303\243o ver as propriedades e controlar a entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Selecion\303\241vel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_voadora->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade flutuar\303\241.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_voadora->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Voadora", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_visibilidade->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade ser\303\241 vista para jogadores. Caso seja selecion\303\241vel, a entidade ficar\303\241 transl\303\272cida.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\255vel", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Textura", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de Vida:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de vida para entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Max", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_max_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de pontos de vida para entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_26->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Temp", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_pontos_vida_temporarios->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de vida para entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", 0, QApplication::UnicodeUTF8));
        label_tamanho->setText(QApplication::translate("ifg::qt::DialogoEntidade", "(m\303\251dio)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        slider_tamanho->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho da entidade", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_9->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Pr\303\263xima Salva\303\247\303\243o", 0, QApplication::UnicodeUTF8));
        combo_salvacao->clear();
        combo_salvacao->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "Falha", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Meio Dano", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Um Quarto de Dano", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Dano Anulado", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        combo_salvacao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Dano que a entidade receber\303\241 na pr\303\263xima a\303\247\303\243o de \303\241rea.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_7->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Deslocamento Vertical", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_translacao_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Para colocar a entidade acima do plano do tabuleiro.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_34->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o", 0, QApplication::UnicodeUTF8));
        combo_visao->clear();
        combo_visao->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "Normal", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o na Penumbra", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\243o no Escuro", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        combo_visao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Tipo de vis\303\243o da entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_14->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_raio_visao_escuro_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_33->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0, QApplication::UnicodeUTF8));
        label_17->setText(QApplication::translate("ifg::qt::DialogoEntidade", "altura", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_altura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Altura da textura, de 0 a 1", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_19->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans x", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_x->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_20->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans y", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_y->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Transla\303\247\303\243o da textura", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_18->setText(QApplication::translate("ifg::qt::DialogoEntidade", "largura", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_largura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Largura da textura, de 0 a 1", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_11->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Lista de Eventos", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_eventos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Cada linha deve conter um evento com formato <descricao:rodadas> sem as <>.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_12->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_raio_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da luz, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_31->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0, QApplication::UnicodeUTF8));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor da Luz", 0, QApplication::UnicodeUTF8));
        label_16->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Modelo 3D", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_caida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade cair\303\241.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_caida->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Ca\303\255da", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_morta->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade estar\303\241 morta.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_morta->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Morta", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Aura:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_aura_quad->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Aura da entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_35->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quadrados", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Id", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulo", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos Especial", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_geral), QApplication::translate("ifg::qt::DialogoEntidade", "Geral", 0, QApplication::UnicodeUTF8));
        label_39->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel PC", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_nivel->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel do Personagem", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_adicionar_nivel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Adicionar N\303\255vel", 0, QApplication::UnicodeUTF8));
        botao_remover_nivel->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Remover", 0, QApplication::UnicodeUTF8));
        label_46->setText(QApplication::translate("ifg::qt::DialogoEntidade", "BBA", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_bba->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque Total", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        groupBox_2->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados N\303\255vel", 0, QApplication::UnicodeUTF8));
        label_40->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Classe", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_classe->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Identificador da Classe", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_41->setText(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_nivel_classe->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel da Classe", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_43->setText(QApplication::translate("ifg::qt::DialogoEntidade", "BBA", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_bba->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus Base de Ataque ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_42->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Conjurador", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_nivel_conjurador->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel de Conjurador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_44->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Mod", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_mod_conjuracao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador do atributo de conjura\303\247\303\243o (exemplo: sabedoria para cl\303\251rigos)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_nivel), QApplication::translate("ifg::qt::DialogoEntidade", "N\303\255vel", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, entidade ter\303\241 iniciativa", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_iniciativa->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Iniciativa", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Valor da iniciativa", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_22->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_modificador_iniciativa->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador de iniciativa", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_remover_ataque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Remover ataque", 0, QApplication::UnicodeUTF8));
        checkbox_imune_critico->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Imune a Cr\303\255tico", 0, QApplication::UnicodeUTF8));
        botao_ataque_cima->setText(QApplication::translate("ifg::qt::DialogoEntidade", "^", 0, QApplication::UnicodeUTF8));
        botao_ataque_baixo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "v", 0, QApplication::UnicodeUTF8));
        botao_clonar_ataque->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Clonar ataque", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados Ataque", 0, QApplication::UnicodeUTF8));
        label_36->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Nome", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_rotulo_ataque->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Identificador do ataque", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_21->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tipo", 0, QApplication::UnicodeUTF8));
        combo_tipo_ataque->clear();
        combo_tipo_ataque->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::DialogoEntidade", "\303\201cido", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Ataque Corpo a Corpo", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Ataque a Dist\303\242ncia", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Bola de Fogo", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Coluna de Chamas", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Cone de Gelo", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Feiti\303\247o de Toque", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Fogo Alqu\303\255mico", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "M\303\243os Flamejantes", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "M\303\255ssil M\303\241gico", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Pedrada", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Rel\303\242mpago", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Tempestade Glacial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::DialogoEntidade", "Outro", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        checkbox_permite_escudo->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Ataque permite escudo?", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_permite_escudo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Permite Escudo?", 0, QApplication::UnicodeUTF8));
        label_29->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Alcance", 0, QApplication::UnicodeUTF8));
        label_30->setText(QApplication::translate("ifg::qt::DialogoEntidade", "quads", 0, QApplication::UnicodeUTF8));
        label_32->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Inc", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_incrementos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de incrementos permitido pelo ataque", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_23->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus ", 0, QApplication::UnicodeUTF8));
        label_24->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Dano", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_dano->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Exemplo: 1d8+2 (19-20/x2)", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_38->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Furtivo", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_furtivo->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Exemplo: 3d6", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        linha_furtivo->setText(QString());
        groupBox_3->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Dados de CA", 0, QApplication::UnicodeUTF8));
        label_25->setText(QString());
        spin_bonus_escudo->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escudo", 0, QApplication::UnicodeUTF8));
        label_57->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Total", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_ca_escudo->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus de Escudo", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_ca_armadura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "B\303\264nus de Armadura", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_ca_toque->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "CA Toque", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_ca_destreza->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador de Destreza", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_52->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Armadura", 0, QApplication::UnicodeUTF8));
        label_58->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Toque", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_ca_total->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "CA Total", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_56->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Outros", 0, QApplication::UnicodeUTF8));
        label_55->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_ca_tamanho->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador de Tamanho", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_54->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Des", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_ca_surpreso->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "CA Surpreso", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_59->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Surp", 0, QApplication::UnicodeUTF8));
        label_53->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Base", 0, QApplication::UnicodeUTF8));
        label_60->setText(QApplication::translate("ifg::qt::DialogoEntidade", "10+", 0, QApplication::UnicodeUTF8));
        label_61->setText(QApplication::translate("ifg::qt::DialogoEntidade", "=", 0, QApplication::UnicodeUTF8));
        botao_bonus_ca->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
        groupBox_4->setTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Atributos", 0, QApplication::UnicodeUTF8));
        label_47->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Int", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_des->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Destreza", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_49->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Sab", 0, QApplication::UnicodeUTF8));
        label_51->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Car", 0, QApplication::UnicodeUTF8));
        botao_bonus_con->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
        label_50->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Con", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_mod_des->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_des->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
        botao_bonus_des->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_car->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Carisma", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_sab->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Sabedoria", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        spin_int->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Intelig\303\252ncia", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_bonus_for->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
        label_27->setText(QApplication::translate("ifg::qt::DialogoEntidade", "For", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_for->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "For\303\247a", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        label_mod_for->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_for->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_con->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Constitui\303\247\303\243o", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        label_mod_sab->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_sab->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
        label_48->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Des", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_mod_int->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_int->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
        botao_bonus_sab->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
        botao_bonus_int->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_mod_con->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_con->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        label_mod_car->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Modificador", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_mod_car->setText(QApplication::translate("ifg::qt::DialogoEntidade", "+0", 0, QApplication::UnicodeUTF8));
        botao_bonus_car->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Bonus", 0, QApplication::UnicodeUTF8));
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_estatisticas), QApplication::translate("ifg::qt::DialogoEntidade", "Estat\303\255sticas", 0, QApplication::UnicodeUTF8));
        label_37->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tesouro", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_tesouro->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab), QApplication::translate("ifg::qt::DialogoEntidade", "Tesouro", 0, QApplication::UnicodeUTF8));
        label_45->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Notas", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        texto_notas->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        tab_tesouro->setTabText(tab_tesouro->indexOf(tab_2), QApplication::translate("ifg::qt::DialogoEntidade", "Notas", 0, QApplication::UnicodeUTF8));
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
