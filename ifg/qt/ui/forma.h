/********************************************************************************
** Form generated from reading UI file 'forma.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef FORMA_H
#define FORMA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace ifg {
namespace qt {

class Ui_DialogoForma
{
public:
    QGridLayout *gridLayout_2;
    QDialogButtonBox *botoes;
    QTabWidget *tabWidget;
    QWidget *tab;
    QGridLayout *gridLayout_4;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *campo_id;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_17;
    QComboBox *combo_tipo_forma;
    QGroupBox *groupBoxDim;
    QHBoxLayout *horizontalLayout_23;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_7;
    QDoubleSpinBox *spin_escala_x_quad;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_5;
    QDoubleSpinBox *spin_escala_y_quad;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QDoubleSpinBox *spin_escala_z_quad;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout_3;
    QCheckBox *checkbox_fixa;
    QCheckBox *checkbox_respeita_solo;
    QCheckBox *checkbox_colisao;
    QCheckBox *checkbox_visibilidade;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_afetado_por_efeitos;
    QCheckBox *checkbox_ignora_luz;
    QCheckBox *checkbox_dois_lados;
    QCheckBox *checkbox_faz_sombra;
    QCheckBox *checkbox_especular;
    QCheckBox *checkbox_fumegando;
    QGroupBox *groupBox_10;
    QHBoxLayout *horizontalLayout_11;
    QPlainTextEdit *lista_rotulos;
    QGroupBox *groupBox_8;
    QHBoxLayout *horizontalLayout_26;
    QHBoxLayout *horizontalLayout_24;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_8;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_9;
    QSlider *slider_alfa;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QGroupBox *groupBox1;
    QGridLayout *gridLayout_6;
    QComboBox *combo_textura;
    QHBoxLayout *horizontalLayout_25;
    QCheckBox *checkbox_ladrilho;
    QCheckBox *checkbox_bump;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_4;
    QDoubleSpinBox *spin_tex_escala_x;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_16;
    QDoubleSpinBox *spin_tex_escala_y;
    QGroupBox *groupBox2;
    QGridLayout *gridLayout_7;
    QSpinBox *spin_tex_direcao;
    QLabel *label_19;
    QDial *dial_tex_direcao;
    QCheckBox *checkbox_textura_circular;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_11;
    QSpinBox *spin_tex_periodo;
    QSpacerItem *verticalSpacer;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_22;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_2;
    QDoubleSpinBox *spin_translacao_quad;
    QGroupBox *groupBox_2;
    QDial *dial_rotacao;
    QSpinBox *spin_rotacao;
    QLabel *label_15;
    QGroupBox *groupBox_3;
    QDial *dial_rotacao_y;
    QSpinBox *spin_rotacao_y;
    QGroupBox *groupBox_4;
    QDial *dial_rotacao_x;
    QSpinBox *spin_rotacao_x;
    QGroupBox *groupBox_9;
    QHBoxLayout *horizontalLayout_27;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_luz;
    QLabel *label_18;
    QDoubleSpinBox *spin_raio_quad;
    QLabel *label_31;
    QPushButton *botao_luz;
    QGroupBox *groupBoxPv;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_8;
    QSpinBox *spin_pontos_vida;
    QLabel *label_10;
    QSpinBox *spin_max_pontos_vida;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_19;
    QComboBox *combo_transicao;
    QComboBox *combo_id_cenario;
    QCheckBox *checkbox_transicao_posicao;
    QPushButton *botao_transicao_mapa;
    QHBoxLayout *horizontalLayout_14;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_12;
    QDoubleSpinBox *spin_trans_x;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_13;
    QDoubleSpinBox *spin_trans_y;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_14;
    QDoubleSpinBox *spin_trans_z;
    QWidget *tab_2;
    QGridLayout *gridLayout_5;
    QGroupBox *groupBox_14;
    QHBoxLayout *horizontalLayout_59;
    QListWidget *lista_pocoes;
    QVBoxLayout *verticalLayout_6;
    QPushButton *botao_adicionar_pocao;
    QPushButton *botao_duplicar_pocao;
    QPushButton *botao_remover_pocao;
    QPushButton *botao_ordenar_pocoes;
    QPushButton *botao_doar_pocao;
    QGroupBox *groupBox_22;
    QHBoxLayout *horizontalLayout_67;
    QListWidget *lista_bracadeiras;
    QVBoxLayout *verticalLayout_7;
    QPushButton *botao_usar_bracadeiras;
    QPushButton *botao_adicionar_bracadeiras;
    QPushButton *botao_remover_bracadeiras;
    QPushButton *botao_doar_bracadeiras;
    QGroupBox *groupBox_16;
    QHBoxLayout *horizontalLayout_61;
    QListWidget *lista_pergaminhos_divinos;
    QVBoxLayout *verticalLayout_24;
    QPushButton *botao_adicionar_pergaminho_divino;
    QPushButton *botao_duplicar_pergaminho_divino;
    QPushButton *botao_remover_pergaminho_divino;
    QPushButton *botao_ordenar_pergaminhos_divinos;
    QPushButton *botao_doar_pergaminho_divino;
    QGroupBox *groupBox_15;
    QHBoxLayout *horizontalLayout_60;
    QListWidget *lista_pergaminhos_arcanos;
    QVBoxLayout *verticalLayout_23;
    QPushButton *botao_adicionar_pergaminho_arcano;
    QPushButton *botao_duplicar_pergaminho_arcano;
    QPushButton *botao_remover_pergaminho_arcano;
    QPushButton *botao_ordenar_pergaminhos_arcanos;
    QPushButton *botao_doar_pergaminho_arcano;
    QGroupBox *groupBox_26;
    QHBoxLayout *horizontalLayout_75;
    QListWidget *lista_escudos;
    QVBoxLayout *verticalLayout_31;
    QPushButton *botao_adicionar_escudo;
    QPushButton *botao_duplicar_escudo;
    QPushButton *botao_remover_escudo;
    QPushButton *botao_ordenar_escudos;
    QPushButton *botao_doar_escudo;
    QGroupBox *groupBox_25;
    QHBoxLayout *horizontalLayout_74;
    QListWidget *lista_armaduras;
    QVBoxLayout *verticalLayout_30;
    QPushButton *botao_adicionar_armadura;
    QPushButton *botao_duplicar_armadura;
    QPushButton *botao_remover_armadura;
    QPushButton *botao_ordenar_armaduras;
    QPushButton *botao_doar_armadura;
    QGroupBox *groupBox_12;
    QHBoxLayout *horizontalLayout_44;
    QListWidget *lista_botas;
    QVBoxLayout *verticalLayout_22;
    QPushButton *botao_usar_botas;
    QPushButton *botao_adicionar_botas;
    QPushButton *botao_remover_botas;
    QPushButton *botao_doar_botas;
    QGroupBox *groupBox_20;
    QHBoxLayout *horizontalLayout_65;
    QListWidget *lista_mantos;
    QVBoxLayout *verticalLayout_9;
    QPushButton *botao_usar_manto;
    QPushButton *botao_adicionar_manto;
    QPushButton *botao_remover_manto;
    QPushButton *botao_doar_manto;
    QGroupBox *groupBox_13;
    QHBoxLayout *horizontalLayout_58;
    QListWidget *lista_amuletos;
    QVBoxLayout *verticalLayout_19;
    QPushButton *botao_usar_amuleto;
    QPushButton *botao_adicionar_amuleto;
    QPushButton *botao_remover_amuleto;
    QPushButton *botao_doar_amuleto;
    QGroupBox *groupBox_18;
    QHBoxLayout *horizontalLayout_63;
    QListWidget *lista_luvas;
    QVBoxLayout *verticalLayout_8;
    QPushButton *botao_usar_luvas;
    QPushButton *botao_adicionar_luvas;
    QPushButton *botao_remover_luvas;
    QPushButton *botao_doar_luvas;
    QGroupBox *groupBox_24;
    QHBoxLayout *horizontalLayout_73;
    QListWidget *lista_armas;
    QVBoxLayout *verticalLayout_29;
    QPushButton *botao_adicionar_arma;
    QPushButton *botao_duplicar_arma;
    QPushButton *botao_remover_arma;
    QPushButton *botao_ordenar_armas;
    QPushButton *botao_doar_arma;
    QGroupBox *groupBox_19;
    QHBoxLayout *horizontalLayout_64;
    QListWidget *lista_chapeus;
    QVBoxLayout *verticalLayout_20;
    QPushButton *botao_vestir_chapeu;
    QPushButton *botao_adicionar_chapeu;
    QPushButton *botao_remover_chapeu;
    QPushButton *botao_doar_chapeu;
    QGroupBox *groupBox_23;
    QHBoxLayout *horizontalLayout_68;
    QListWidget *lista_itens_mundanos;
    QVBoxLayout *verticalLayout_11;
    QPushButton *botao_adicionar_item_mundano;
    QPushButton *botao_duplicar_item_mundano;
    QPushButton *botao_remover_item_mundano;
    QPushButton *botao_ordenar_item_mundano;
    QPushButton *botao_doar_item_mundano;
    QGroupBox *groupBox_17;
    QHBoxLayout *horizontalLayout_62;
    QListWidget *lista_aneis;
    QVBoxLayout *verticalLayout_10;
    QPushButton *botao_usar_anel;
    QPushButton *botao_adicionar_anel;
    QPushButton *botao_remover_anel;
    QPushButton *botao_doar_anel;
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

    void setupUi(QDialog *ifg__qt__DialogoForma)
    {
        if (ifg__qt__DialogoForma->objectName().isEmpty())
            ifg__qt__DialogoForma->setObjectName("ifg__qt__DialogoForma");
        ifg__qt__DialogoForma->resize(1403, 978);
        gridLayout_2 = new QGridLayout(ifg__qt__DialogoForma);
        gridLayout_2->setObjectName("gridLayout_2");
        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName("botoes");
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(botoes, 7, 2, 1, 1);

        tabWidget = new QTabWidget(ifg__qt__DialogoForma);
        tabWidget->setObjectName("tabWidget");
        tab = new QWidget();
        tab->setObjectName("tab");
        gridLayout_4 = new QGridLayout(tab);
        gridLayout_4->setObjectName("gridLayout_4");
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        label = new QLabel(tab);
        label->setObjectName("label");

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(tab);
        campo_id->setObjectName("campo_id");
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);


        gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName("horizontalLayout_21");
        label_17 = new QLabel(tab);
        label_17->setObjectName("label_17");
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_17);

        combo_tipo_forma = new QComboBox(tab);
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->setObjectName("combo_tipo_forma");

        horizontalLayout_21->addWidget(combo_tipo_forma);


        gridLayout_4->addLayout(horizontalLayout_21, 0, 1, 1, 1);

        groupBoxDim = new QGroupBox(tab);
        groupBoxDim->setObjectName("groupBoxDim");
        horizontalLayout_23 = new QHBoxLayout(groupBoxDim);
        horizontalLayout_23->setObjectName("horizontalLayout_23");
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label_7 = new QLabel(groupBoxDim);
        label_7->setObjectName("label_7");

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_x_quad->setObjectName("spin_escala_x_quad");
        spin_escala_x_quad->setDecimals(2);
        spin_escala_x_quad->setMinimum(-1000.000000000000000);
        spin_escala_x_quad->setMaximum(1000.000000000000000);
        spin_escala_x_quad->setSingleStep(0.100000000000000);

        horizontalLayout_4->addWidget(spin_escala_x_quad);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        label_5 = new QLabel(groupBoxDim);
        label_5->setObjectName("label_5");

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_y_quad->setObjectName("spin_escala_y_quad");
        spin_escala_y_quad->setDecimals(2);
        spin_escala_y_quad->setMinimum(-1000.000000000000000);
        spin_escala_y_quad->setMaximum(1000.000000000000000);
        spin_escala_y_quad->setSingleStep(0.100000000000000);

        horizontalLayout_5->addWidget(spin_escala_y_quad);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName("horizontalLayout_6");
        label_6 = new QLabel(groupBoxDim);
        label_6->setObjectName("label_6");

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_z_quad->setObjectName("spin_escala_z_quad");
        spin_escala_z_quad->setDecimals(2);
        spin_escala_z_quad->setMinimum(-50.000000000000000);
        spin_escala_z_quad->setMaximum(50.000000000000000);
        spin_escala_z_quad->setSingleStep(0.100000000000000);

        horizontalLayout_6->addWidget(spin_escala_z_quad);


        verticalLayout->addLayout(horizontalLayout_6);


        horizontalLayout_23->addLayout(verticalLayout);


        gridLayout_4->addWidget(groupBoxDim, 1, 1, 1, 1);

        groupBox_7 = new QGroupBox(tab);
        groupBox_7->setObjectName("groupBox_7");
        gridLayout_3 = new QGridLayout(groupBox_7);
        gridLayout_3->setObjectName("gridLayout_3");
        checkbox_fixa = new QCheckBox(groupBox_7);
        checkbox_fixa->setObjectName("checkbox_fixa");

        gridLayout_3->addWidget(checkbox_fixa, 1, 1, 1, 1);

        checkbox_respeita_solo = new QCheckBox(groupBox_7);
        checkbox_respeita_solo->setObjectName("checkbox_respeita_solo");

        gridLayout_3->addWidget(checkbox_respeita_solo, 0, 0, 1, 1);

        checkbox_colisao = new QCheckBox(groupBox_7);
        checkbox_colisao->setObjectName("checkbox_colisao");

        gridLayout_3->addWidget(checkbox_colisao, 2, 1, 1, 1);

        checkbox_visibilidade = new QCheckBox(groupBox_7);
        checkbox_visibilidade->setObjectName("checkbox_visibilidade");
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(checkbox_visibilidade, 2, 2, 1, 1);

        checkbox_selecionavel = new QCheckBox(groupBox_7);
        checkbox_selecionavel->setObjectName("checkbox_selecionavel");

        gridLayout_3->addWidget(checkbox_selecionavel, 1, 0, 1, 1);

        checkbox_afetado_por_efeitos = new QCheckBox(groupBox_7);
        checkbox_afetado_por_efeitos->setObjectName("checkbox_afetado_por_efeitos");

        gridLayout_3->addWidget(checkbox_afetado_por_efeitos, 2, 0, 1, 1);

        checkbox_ignora_luz = new QCheckBox(groupBox_7);
        checkbox_ignora_luz->setObjectName("checkbox_ignora_luz");

        gridLayout_3->addWidget(checkbox_ignora_luz, 0, 2, 1, 1);

        checkbox_dois_lados = new QCheckBox(groupBox_7);
        checkbox_dois_lados->setObjectName("checkbox_dois_lados");

        gridLayout_3->addWidget(checkbox_dois_lados, 0, 1, 1, 1);

        checkbox_faz_sombra = new QCheckBox(groupBox_7);
        checkbox_faz_sombra->setObjectName("checkbox_faz_sombra");

        gridLayout_3->addWidget(checkbox_faz_sombra, 1, 2, 1, 1);

        checkbox_especular = new QCheckBox(groupBox_7);
        checkbox_especular->setObjectName("checkbox_especular");

        gridLayout_3->addWidget(checkbox_especular, 0, 3, 1, 1);

        checkbox_fumegando = new QCheckBox(groupBox_7);
        checkbox_fumegando->setObjectName("checkbox_fumegando");

        gridLayout_3->addWidget(checkbox_fumegando, 1, 3, 1, 1);


        gridLayout_4->addWidget(groupBox_7, 1, 0, 1, 1);

        groupBox_10 = new QGroupBox(tab);
        groupBox_10->setObjectName("groupBox_10");
        horizontalLayout_11 = new QHBoxLayout(groupBox_10);
        horizontalLayout_11->setObjectName("horizontalLayout_11");
        lista_rotulos = new QPlainTextEdit(groupBox_10);
        lista_rotulos->setObjectName("lista_rotulos");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lista_rotulos->sizePolicy().hasHeightForWidth());
        lista_rotulos->setSizePolicy(sizePolicy1);

        horizontalLayout_11->addWidget(lista_rotulos);


        gridLayout_4->addWidget(groupBox_10, 6, 0, 1, 1);

        groupBox_8 = new QGroupBox(tab);
        groupBox_8->setObjectName("groupBox_8");
        horizontalLayout_26 = new QHBoxLayout(groupBox_8);
        horizontalLayout_26->setObjectName("horizontalLayout_26");
        horizontalLayout_24 = new QHBoxLayout();
        horizontalLayout_24->setObjectName("horizontalLayout_24");
        groupBox = new QGroupBox(groupBox_8);
        groupBox->setObjectName("groupBox");
        gridLayout_8 = new QGridLayout(groupBox);
        gridLayout_8->setObjectName("gridLayout_8");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_9 = new QLabel(groupBox);
        label_9->setObjectName("label_9");
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label_9);

        slider_alfa = new QSlider(groupBox);
        slider_alfa->setObjectName("slider_alfa");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy2);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(slider_alfa);


        gridLayout_8->addLayout(horizontalLayout_2, 1, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName("horizontalLayout_7");
        label_3 = new QLabel(groupBox);
        label_3->setObjectName("label_3");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy3);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(groupBox);
        checkbox_cor->setObjectName("checkbox_cor");
        sizePolicy2.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy2);

        horizontalLayout_7->addWidget(checkbox_cor);

        botao_cor = new QPushButton(groupBox);
        botao_cor->setObjectName("botao_cor");

        horizontalLayout_7->addWidget(botao_cor);


        gridLayout_8->addLayout(horizontalLayout_7, 0, 0, 1, 1);


        horizontalLayout_24->addWidget(groupBox);


        horizontalLayout_26->addLayout(horizontalLayout_24);

        groupBox1 = new QGroupBox(groupBox_8);
        groupBox1->setObjectName("groupBox1");
        gridLayout_6 = new QGridLayout(groupBox1);
        gridLayout_6->setObjectName("gridLayout_6");
        combo_textura = new QComboBox(groupBox1);
        combo_textura->setObjectName("combo_textura");

        gridLayout_6->addWidget(combo_textura, 0, 0, 1, 1);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName("horizontalLayout_25");
        checkbox_ladrilho = new QCheckBox(groupBox1);
        checkbox_ladrilho->setObjectName("checkbox_ladrilho");
        sizePolicy2.setHeightForWidth(checkbox_ladrilho->sizePolicy().hasHeightForWidth());
        checkbox_ladrilho->setSizePolicy(sizePolicy2);

        horizontalLayout_25->addWidget(checkbox_ladrilho);

        checkbox_bump = new QCheckBox(groupBox1);
        checkbox_bump->setObjectName("checkbox_bump");

        horizontalLayout_25->addWidget(checkbox_bump);


        gridLayout_6->addLayout(horizontalLayout_25, 0, 1, 1, 1);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName("horizontalLayout_20");
        label_4 = new QLabel(groupBox1);
        label_4->setObjectName("label_4");

        horizontalLayout_20->addWidget(label_4);

        spin_tex_escala_x = new QDoubleSpinBox(groupBox1);
        spin_tex_escala_x->setObjectName("spin_tex_escala_x");
        spin_tex_escala_x->setMaximum(1000.000000000000000);

        horizontalLayout_20->addWidget(spin_tex_escala_x);


        gridLayout_6->addLayout(horizontalLayout_20, 1, 0, 1, 1);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName("horizontalLayout_8");
        label_16 = new QLabel(groupBox1);
        label_16->setObjectName("label_16");

        horizontalLayout_8->addWidget(label_16);

        spin_tex_escala_y = new QDoubleSpinBox(groupBox1);
        spin_tex_escala_y->setObjectName("spin_tex_escala_y");
        spin_tex_escala_y->setMaximum(1000.000000000000000);

        horizontalLayout_8->addWidget(spin_tex_escala_y);


        gridLayout_6->addLayout(horizontalLayout_8, 1, 1, 1, 1);

        groupBox2 = new QGroupBox(groupBox1);
        groupBox2->setObjectName("groupBox2");
        gridLayout_7 = new QGridLayout(groupBox2);
        gridLayout_7->setObjectName("gridLayout_7");
        spin_tex_direcao = new QSpinBox(groupBox2);
        spin_tex_direcao->setObjectName("spin_tex_direcao");
        spin_tex_direcao->setMinimum(-180);
        spin_tex_direcao->setMaximum(180);

        gridLayout_7->addWidget(spin_tex_direcao, 3, 0, 1, 1);

        label_19 = new QLabel(groupBox2);
        label_19->setObjectName("label_19");
        label_19->setAlignment(Qt::AlignCenter);

        gridLayout_7->addWidget(label_19, 1, 0, 1, 1);

        dial_tex_direcao = new QDial(groupBox2);
        dial_tex_direcao->setObjectName("dial_tex_direcao");
        dial_tex_direcao->setMinimum(0);
        dial_tex_direcao->setMaximum(360);
        dial_tex_direcao->setValue(0);
        dial_tex_direcao->setSliderPosition(0);
        dial_tex_direcao->setOrientation(Qt::Horizontal);
        dial_tex_direcao->setInvertedAppearance(true);
        dial_tex_direcao->setInvertedControls(true);
        dial_tex_direcao->setWrapping(true);
        dial_tex_direcao->setNotchTarget(45.000000000000000);
        dial_tex_direcao->setNotchesVisible(true);

        gridLayout_7->addWidget(dial_tex_direcao, 2, 0, 1, 1);

        checkbox_textura_circular = new QCheckBox(groupBox2);
        checkbox_textura_circular->setObjectName("checkbox_textura_circular");

        gridLayout_7->addWidget(checkbox_textura_circular, 0, 0, 1, 1);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName("verticalLayout_4");
        label_11 = new QLabel(groupBox2);
        label_11->setObjectName("label_11");
        label_11->setAlignment(Qt::AlignCenter);

        verticalLayout_4->addWidget(label_11);

        spin_tex_periodo = new QSpinBox(groupBox2);
        spin_tex_periodo->setObjectName("spin_tex_periodo");
        spin_tex_periodo->setMaximum(1000);

        verticalLayout_4->addWidget(spin_tex_periodo);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);


        gridLayout_7->addLayout(verticalLayout_4, 0, 2, 4, 1);


        gridLayout_6->addWidget(groupBox2, 0, 2, 2, 1);


        horizontalLayout_26->addWidget(groupBox1);


        gridLayout_4->addWidget(groupBox_8, 3, 1, 1, 1);

        groupBox_6 = new QGroupBox(tab);
        groupBox_6->setObjectName("groupBox_6");
        horizontalLayout_22 = new QHBoxLayout(groupBox_6);
        horizontalLayout_22->setObjectName("horizontalLayout_22");
        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName("horizontalLayout_18");
        label_2 = new QLabel(groupBox_6);
        label_2->setObjectName("label_2");

        horizontalLayout_18->addWidget(label_2);

        spin_translacao_quad = new QDoubleSpinBox(groupBox_6);
        spin_translacao_quad->setObjectName("spin_translacao_quad");
        spin_translacao_quad->setDecimals(2);
        spin_translacao_quad->setMinimum(-100.000000000000000);
        spin_translacao_quad->setMaximum(100.000000000000000);
        spin_translacao_quad->setSingleStep(0.100000000000000);

        horizontalLayout_18->addWidget(spin_translacao_quad);


        horizontalLayout_22->addLayout(horizontalLayout_18);

        groupBox_2 = new QGroupBox(groupBox_6);
        groupBox_2->setObjectName("groupBox_2");
        dial_rotacao = new QDial(groupBox_2);
        dial_rotacao->setObjectName("dial_rotacao");
        dial_rotacao->setGeometry(QRect(10, 20, 71, 71));
        dial_rotacao->setMinimum(0);
        dial_rotacao->setMaximum(360);
        dial_rotacao->setValue(0);
        dial_rotacao->setSliderPosition(0);
        dial_rotacao->setOrientation(Qt::Horizontal);
        dial_rotacao->setInvertedAppearance(true);
        dial_rotacao->setInvertedControls(true);
        dial_rotacao->setWrapping(true);
        dial_rotacao->setNotchTarget(45.000000000000000);
        dial_rotacao->setNotchesVisible(true);
        spin_rotacao = new QSpinBox(groupBox_2);
        spin_rotacao->setObjectName("spin_rotacao");
        spin_rotacao->setGeometry(QRect(20, 100, 51, 24));
        spin_rotacao->setMinimum(-180);
        spin_rotacao->setMaximum(180);
        label_15 = new QLabel(groupBox_2);
        label_15->setObjectName("label_15");
        label_15->setGeometry(QRect(170, 100, 41, 16));

        horizontalLayout_22->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(groupBox_6);
        groupBox_3->setObjectName("groupBox_3");
        dial_rotacao_y = new QDial(groupBox_3);
        dial_rotacao_y->setObjectName("dial_rotacao_y");
        dial_rotacao_y->setGeometry(QRect(20, 20, 71, 71));
        dial_rotacao_y->setMinimum(0);
        dial_rotacao_y->setMaximum(360);
        dial_rotacao_y->setValue(0);
        dial_rotacao_y->setSliderPosition(0);
        dial_rotacao_y->setOrientation(Qt::Horizontal);
        dial_rotacao_y->setInvertedAppearance(true);
        dial_rotacao_y->setInvertedControls(true);
        dial_rotacao_y->setWrapping(true);
        dial_rotacao_y->setNotchTarget(45.000000000000000);
        dial_rotacao_y->setNotchesVisible(true);
        spin_rotacao_y = new QSpinBox(groupBox_3);
        spin_rotacao_y->setObjectName("spin_rotacao_y");
        spin_rotacao_y->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_y->setMinimum(-180);
        spin_rotacao_y->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(groupBox_6);
        groupBox_4->setObjectName("groupBox_4");
        dial_rotacao_x = new QDial(groupBox_4);
        dial_rotacao_x->setObjectName("dial_rotacao_x");
        dial_rotacao_x->setGeometry(QRect(20, 20, 71, 71));
        dial_rotacao_x->setMinimum(0);
        dial_rotacao_x->setMaximum(360);
        dial_rotacao_x->setValue(0);
        dial_rotacao_x->setSliderPosition(0);
        dial_rotacao_x->setOrientation(Qt::Horizontal);
        dial_rotacao_x->setInvertedAppearance(true);
        dial_rotacao_x->setInvertedControls(true);
        dial_rotacao_x->setWrapping(true);
        dial_rotacao_x->setNotchTarget(45.000000000000000);
        dial_rotacao_x->setNotchesVisible(true);
        spin_rotacao_x = new QSpinBox(groupBox_4);
        spin_rotacao_x->setObjectName("spin_rotacao_x");
        spin_rotacao_x->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_x->setMinimum(-180);
        spin_rotacao_x->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_4);


        gridLayout_4->addWidget(groupBox_6, 6, 1, 1, 1);

        groupBox_9 = new QGroupBox(tab);
        groupBox_9->setObjectName("groupBox_9");
        horizontalLayout_27 = new QHBoxLayout(groupBox_9);
        horizontalLayout_27->setObjectName("horizontalLayout_27");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        checkbox_luz = new QCheckBox(groupBox_9);
        checkbox_luz->setObjectName("checkbox_luz");
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        label_18 = new QLabel(groupBox_9);
        label_18->setObjectName("label_18");
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_18);

        spin_raio_quad = new QDoubleSpinBox(groupBox_9);
        spin_raio_quad->setObjectName("spin_raio_quad");
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1.000000000000000);

        horizontalLayout_3->addWidget(spin_raio_quad);

        label_31 = new QLabel(groupBox_9);
        label_31->setObjectName("label_31");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy4);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_31);

        botao_luz = new QPushButton(groupBox_9);
        botao_luz->setObjectName("botao_luz");
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(botao_luz);


        horizontalLayout_27->addLayout(horizontalLayout_3);


        gridLayout_4->addWidget(groupBox_9, 4, 1, 1, 1);

        groupBoxPv = new QGroupBox(tab);
        groupBoxPv->setObjectName("groupBoxPv");
        horizontalLayout_13 = new QHBoxLayout(groupBoxPv);
        horizontalLayout_13->setObjectName("horizontalLayout_13");
        label_8 = new QLabel(groupBoxPv);
        label_8->setObjectName("label_8");
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(groupBoxPv);
        spin_pontos_vida->setObjectName("spin_pontos_vida");
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(groupBoxPv);
        label_10->setObjectName("label_10");
        sizePolicy3.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy3);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(groupBoxPv);
        spin_max_pontos_vida->setObjectName("spin_max_pontos_vida");
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);


        gridLayout_4->addWidget(groupBoxPv, 4, 0, 1, 1);

        groupBox_5 = new QGroupBox(tab);
        groupBox_5->setObjectName("groupBox_5");
        gridLayout = new QGridLayout(groupBox_5);
        gridLayout->setObjectName("gridLayout");
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName("horizontalLayout_19");
        combo_transicao = new QComboBox(groupBox_5);
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->setObjectName("combo_transicao");

        horizontalLayout_19->addWidget(combo_transicao);

        combo_id_cenario = new QComboBox(groupBox_5);
        combo_id_cenario->setObjectName("combo_id_cenario");

        horizontalLayout_19->addWidget(combo_id_cenario);

        checkbox_transicao_posicao = new QCheckBox(groupBox_5);
        checkbox_transicao_posicao->setObjectName("checkbox_transicao_posicao");

        horizontalLayout_19->addWidget(checkbox_transicao_posicao);

        botao_transicao_mapa = new QPushButton(groupBox_5);
        botao_transicao_mapa->setObjectName("botao_transicao_mapa");
        sizePolicy2.setHeightForWidth(botao_transicao_mapa->sizePolicy().hasHeightForWidth());
        botao_transicao_mapa->setSizePolicy(sizePolicy2);

        horizontalLayout_19->addWidget(botao_transicao_mapa);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName("horizontalLayout_14");
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName("horizontalLayout_15");
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName("label_12");
        QSizePolicy sizePolicy5(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy5);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_12);

        spin_trans_x = new QDoubleSpinBox(groupBox_5);
        spin_trans_x->setObjectName("spin_trans_x");
        spin_trans_x->setDecimals(1);
        spin_trans_x->setMinimum(-1000.000000000000000);
        spin_trans_x->setMaximum(1000.000000000000000);
        spin_trans_x->setSingleStep(0.500000000000000);

        horizontalLayout_15->addWidget(spin_trans_x);


        horizontalLayout_14->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName("horizontalLayout_16");
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName("label_13");
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_16->addWidget(label_13);

        spin_trans_y = new QDoubleSpinBox(groupBox_5);
        spin_trans_y->setObjectName("spin_trans_y");
        spin_trans_y->setDecimals(1);
        spin_trans_y->setMinimum(-1000.000000000000000);
        spin_trans_y->setMaximum(1000.000000000000000);
        spin_trans_y->setSingleStep(0.500000000000000);

        horizontalLayout_16->addWidget(spin_trans_y);


        horizontalLayout_14->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName("horizontalLayout_17");
        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName("label_14");
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_17->addWidget(label_14);

        spin_trans_z = new QDoubleSpinBox(groupBox_5);
        spin_trans_z->setObjectName("spin_trans_z");
        spin_trans_z->setDecimals(1);
        spin_trans_z->setMinimum(-1000.000000000000000);
        spin_trans_z->setMaximum(1000.000000000000000);
        spin_trans_z->setSingleStep(0.500000000000000);

        horizontalLayout_17->addWidget(spin_trans_z);


        horizontalLayout_14->addLayout(horizontalLayout_17);


        verticalLayout_2->addLayout(horizontalLayout_14);


        gridLayout->addLayout(verticalLayout_2, 0, 0, 1, 1);


        gridLayout_4->addWidget(groupBox_5, 3, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        gridLayout_5 = new QGridLayout(tab_2);
        gridLayout_5->setObjectName("gridLayout_5");
        groupBox_14 = new QGroupBox(tab_2);
        groupBox_14->setObjectName("groupBox_14");
        horizontalLayout_59 = new QHBoxLayout(groupBox_14);
        horizontalLayout_59->setObjectName("horizontalLayout_59");
        lista_pocoes = new QListWidget(groupBox_14);
        lista_pocoes->setObjectName("lista_pocoes");

        horizontalLayout_59->addWidget(lista_pocoes);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName("verticalLayout_6");
        botao_adicionar_pocao = new QPushButton(groupBox_14);
        botao_adicionar_pocao->setObjectName("botao_adicionar_pocao");
        sizePolicy.setHeightForWidth(botao_adicionar_pocao->sizePolicy().hasHeightForWidth());
        botao_adicionar_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_adicionar_pocao);

        botao_duplicar_pocao = new QPushButton(groupBox_14);
        botao_duplicar_pocao->setObjectName("botao_duplicar_pocao");

        verticalLayout_6->addWidget(botao_duplicar_pocao);

        botao_remover_pocao = new QPushButton(groupBox_14);
        botao_remover_pocao->setObjectName("botao_remover_pocao");
        sizePolicy.setHeightForWidth(botao_remover_pocao->sizePolicy().hasHeightForWidth());
        botao_remover_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_remover_pocao);

        botao_ordenar_pocoes = new QPushButton(groupBox_14);
        botao_ordenar_pocoes->setObjectName("botao_ordenar_pocoes");

        verticalLayout_6->addWidget(botao_ordenar_pocoes);

        botao_doar_pocao = new QPushButton(groupBox_14);
        botao_doar_pocao->setObjectName("botao_doar_pocao");

        verticalLayout_6->addWidget(botao_doar_pocao);


        horizontalLayout_59->addLayout(verticalLayout_6);


        gridLayout_5->addWidget(groupBox_14, 0, 1, 1, 1);

        groupBox_22 = new QGroupBox(tab_2);
        groupBox_22->setObjectName("groupBox_22");
        horizontalLayout_67 = new QHBoxLayout(groupBox_22);
        horizontalLayout_67->setObjectName("horizontalLayout_67");
        lista_bracadeiras = new QListWidget(groupBox_22);
        lista_bracadeiras->setObjectName("lista_bracadeiras");

        horizontalLayout_67->addWidget(lista_bracadeiras);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName("verticalLayout_7");
        botao_usar_bracadeiras = new QPushButton(groupBox_22);
        botao_usar_bracadeiras->setObjectName("botao_usar_bracadeiras");

        verticalLayout_7->addWidget(botao_usar_bracadeiras);

        botao_adicionar_bracadeiras = new QPushButton(groupBox_22);
        botao_adicionar_bracadeiras->setObjectName("botao_adicionar_bracadeiras");

        verticalLayout_7->addWidget(botao_adicionar_bracadeiras);

        botao_remover_bracadeiras = new QPushButton(groupBox_22);
        botao_remover_bracadeiras->setObjectName("botao_remover_bracadeiras");

        verticalLayout_7->addWidget(botao_remover_bracadeiras);

        botao_doar_bracadeiras = new QPushButton(groupBox_22);
        botao_doar_bracadeiras->setObjectName("botao_doar_bracadeiras");

        verticalLayout_7->addWidget(botao_doar_bracadeiras);


        horizontalLayout_67->addLayout(verticalLayout_7);


        gridLayout_5->addWidget(groupBox_22, 2, 2, 1, 1);

        groupBox_16 = new QGroupBox(tab_2);
        groupBox_16->setObjectName("groupBox_16");
        horizontalLayout_61 = new QHBoxLayout(groupBox_16);
        horizontalLayout_61->setObjectName("horizontalLayout_61");
        lista_pergaminhos_divinos = new QListWidget(groupBox_16);
        lista_pergaminhos_divinos->setObjectName("lista_pergaminhos_divinos");

        horizontalLayout_61->addWidget(lista_pergaminhos_divinos);

        verticalLayout_24 = new QVBoxLayout();
        verticalLayout_24->setObjectName("verticalLayout_24");
        botao_adicionar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_adicionar_pergaminho_divino->setObjectName("botao_adicionar_pergaminho_divino");

        verticalLayout_24->addWidget(botao_adicionar_pergaminho_divino);

        botao_duplicar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_duplicar_pergaminho_divino->setObjectName("botao_duplicar_pergaminho_divino");

        verticalLayout_24->addWidget(botao_duplicar_pergaminho_divino);

        botao_remover_pergaminho_divino = new QPushButton(groupBox_16);
        botao_remover_pergaminho_divino->setObjectName("botao_remover_pergaminho_divino");

        verticalLayout_24->addWidget(botao_remover_pergaminho_divino);

        botao_ordenar_pergaminhos_divinos = new QPushButton(groupBox_16);
        botao_ordenar_pergaminhos_divinos->setObjectName("botao_ordenar_pergaminhos_divinos");

        verticalLayout_24->addWidget(botao_ordenar_pergaminhos_divinos);

        botao_doar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_doar_pergaminho_divino->setObjectName("botao_doar_pergaminho_divino");

        verticalLayout_24->addWidget(botao_doar_pergaminho_divino);


        horizontalLayout_61->addLayout(verticalLayout_24);


        gridLayout_5->addWidget(groupBox_16, 1, 0, 1, 1);

        groupBox_15 = new QGroupBox(tab_2);
        groupBox_15->setObjectName("groupBox_15");
        horizontalLayout_60 = new QHBoxLayout(groupBox_15);
        horizontalLayout_60->setObjectName("horizontalLayout_60");
        lista_pergaminhos_arcanos = new QListWidget(groupBox_15);
        lista_pergaminhos_arcanos->setObjectName("lista_pergaminhos_arcanos");

        horizontalLayout_60->addWidget(lista_pergaminhos_arcanos);

        verticalLayout_23 = new QVBoxLayout();
        verticalLayout_23->setObjectName("verticalLayout_23");
        botao_adicionar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_adicionar_pergaminho_arcano->setObjectName("botao_adicionar_pergaminho_arcano");

        verticalLayout_23->addWidget(botao_adicionar_pergaminho_arcano);

        botao_duplicar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_duplicar_pergaminho_arcano->setObjectName("botao_duplicar_pergaminho_arcano");

        verticalLayout_23->addWidget(botao_duplicar_pergaminho_arcano);

        botao_remover_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_remover_pergaminho_arcano->setObjectName("botao_remover_pergaminho_arcano");

        verticalLayout_23->addWidget(botao_remover_pergaminho_arcano);

        botao_ordenar_pergaminhos_arcanos = new QPushButton(groupBox_15);
        botao_ordenar_pergaminhos_arcanos->setObjectName("botao_ordenar_pergaminhos_arcanos");

        verticalLayout_23->addWidget(botao_ordenar_pergaminhos_arcanos);

        botao_doar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_doar_pergaminho_arcano->setObjectName("botao_doar_pergaminho_arcano");

        verticalLayout_23->addWidget(botao_doar_pergaminho_arcano);


        horizontalLayout_60->addLayout(verticalLayout_23);


        gridLayout_5->addWidget(groupBox_15, 0, 0, 1, 1);

        groupBox_26 = new QGroupBox(tab_2);
        groupBox_26->setObjectName("groupBox_26");
        horizontalLayout_75 = new QHBoxLayout(groupBox_26);
        horizontalLayout_75->setObjectName("horizontalLayout_75");
        lista_escudos = new QListWidget(groupBox_26);
        lista_escudos->setObjectName("lista_escudos");

        horizontalLayout_75->addWidget(lista_escudos);

        verticalLayout_31 = new QVBoxLayout();
        verticalLayout_31->setObjectName("verticalLayout_31");
        botao_adicionar_escudo = new QPushButton(groupBox_26);
        botao_adicionar_escudo->setObjectName("botao_adicionar_escudo");

        verticalLayout_31->addWidget(botao_adicionar_escudo);

        botao_duplicar_escudo = new QPushButton(groupBox_26);
        botao_duplicar_escudo->setObjectName("botao_duplicar_escudo");

        verticalLayout_31->addWidget(botao_duplicar_escudo);

        botao_remover_escudo = new QPushButton(groupBox_26);
        botao_remover_escudo->setObjectName("botao_remover_escudo");

        verticalLayout_31->addWidget(botao_remover_escudo);

        botao_ordenar_escudos = new QPushButton(groupBox_26);
        botao_ordenar_escudos->setObjectName("botao_ordenar_escudos");

        verticalLayout_31->addWidget(botao_ordenar_escudos);

        botao_doar_escudo = new QPushButton(groupBox_26);
        botao_doar_escudo->setObjectName("botao_doar_escudo");

        verticalLayout_31->addWidget(botao_doar_escudo);


        horizontalLayout_75->addLayout(verticalLayout_31);


        gridLayout_5->addWidget(groupBox_26, 3, 2, 1, 1);

        groupBox_25 = new QGroupBox(tab_2);
        groupBox_25->setObjectName("groupBox_25");
        horizontalLayout_74 = new QHBoxLayout(groupBox_25);
        horizontalLayout_74->setObjectName("horizontalLayout_74");
        lista_armaduras = new QListWidget(groupBox_25);
        lista_armaduras->setObjectName("lista_armaduras");

        horizontalLayout_74->addWidget(lista_armaduras);

        verticalLayout_30 = new QVBoxLayout();
        verticalLayout_30->setObjectName("verticalLayout_30");
        botao_adicionar_armadura = new QPushButton(groupBox_25);
        botao_adicionar_armadura->setObjectName("botao_adicionar_armadura");

        verticalLayout_30->addWidget(botao_adicionar_armadura);

        botao_duplicar_armadura = new QPushButton(groupBox_25);
        botao_duplicar_armadura->setObjectName("botao_duplicar_armadura");

        verticalLayout_30->addWidget(botao_duplicar_armadura);

        botao_remover_armadura = new QPushButton(groupBox_25);
        botao_remover_armadura->setObjectName("botao_remover_armadura");

        verticalLayout_30->addWidget(botao_remover_armadura);

        botao_ordenar_armaduras = new QPushButton(groupBox_25);
        botao_ordenar_armaduras->setObjectName("botao_ordenar_armaduras");

        verticalLayout_30->addWidget(botao_ordenar_armaduras);

        botao_doar_armadura = new QPushButton(groupBox_25);
        botao_doar_armadura->setObjectName("botao_doar_armadura");

        verticalLayout_30->addWidget(botao_doar_armadura);


        horizontalLayout_74->addLayout(verticalLayout_30);


        gridLayout_5->addWidget(groupBox_25, 3, 1, 1, 1);

        groupBox_12 = new QGroupBox(tab_2);
        groupBox_12->setObjectName("groupBox_12");
        horizontalLayout_44 = new QHBoxLayout(groupBox_12);
        horizontalLayout_44->setObjectName("horizontalLayout_44");
        lista_botas = new QListWidget(groupBox_12);
        lista_botas->setObjectName("lista_botas");

        horizontalLayout_44->addWidget(lista_botas);

        verticalLayout_22 = new QVBoxLayout();
        verticalLayout_22->setObjectName("verticalLayout_22");
        botao_usar_botas = new QPushButton(groupBox_12);
        botao_usar_botas->setObjectName("botao_usar_botas");
        botao_usar_botas->setEnabled(false);

        verticalLayout_22->addWidget(botao_usar_botas);

        botao_adicionar_botas = new QPushButton(groupBox_12);
        botao_adicionar_botas->setObjectName("botao_adicionar_botas");

        verticalLayout_22->addWidget(botao_adicionar_botas);

        botao_remover_botas = new QPushButton(groupBox_12);
        botao_remover_botas->setObjectName("botao_remover_botas");

        verticalLayout_22->addWidget(botao_remover_botas);

        botao_doar_botas = new QPushButton(groupBox_12);
        botao_doar_botas->setObjectName("botao_doar_botas");

        verticalLayout_22->addWidget(botao_doar_botas);


        horizontalLayout_44->addLayout(verticalLayout_22);


        gridLayout_5->addWidget(groupBox_12, 2, 1, 1, 1);

        groupBox_20 = new QGroupBox(tab_2);
        groupBox_20->setObjectName("groupBox_20");
        horizontalLayout_65 = new QHBoxLayout(groupBox_20);
        horizontalLayout_65->setObjectName("horizontalLayout_65");
        lista_mantos = new QListWidget(groupBox_20);
        lista_mantos->setObjectName("lista_mantos");

        horizontalLayout_65->addWidget(lista_mantos);

        verticalLayout_9 = new QVBoxLayout();
        verticalLayout_9->setObjectName("verticalLayout_9");
        botao_usar_manto = new QPushButton(groupBox_20);
        botao_usar_manto->setObjectName("botao_usar_manto");
        botao_usar_manto->setEnabled(false);
        sizePolicy.setHeightForWidth(botao_usar_manto->sizePolicy().hasHeightForWidth());
        botao_usar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_usar_manto);

        botao_adicionar_manto = new QPushButton(groupBox_20);
        botao_adicionar_manto->setObjectName("botao_adicionar_manto");
        sizePolicy.setHeightForWidth(botao_adicionar_manto->sizePolicy().hasHeightForWidth());
        botao_adicionar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_adicionar_manto);

        botao_remover_manto = new QPushButton(groupBox_20);
        botao_remover_manto->setObjectName("botao_remover_manto");
        sizePolicy.setHeightForWidth(botao_remover_manto->sizePolicy().hasHeightForWidth());
        botao_remover_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_remover_manto);

        botao_doar_manto = new QPushButton(groupBox_20);
        botao_doar_manto->setObjectName("botao_doar_manto");

        verticalLayout_9->addWidget(botao_doar_manto);


        horizontalLayout_65->addLayout(verticalLayout_9);


        gridLayout_5->addWidget(groupBox_20, 0, 3, 1, 2);

        groupBox_13 = new QGroupBox(tab_2);
        groupBox_13->setObjectName("groupBox_13");
        horizontalLayout_58 = new QHBoxLayout(groupBox_13);
        horizontalLayout_58->setObjectName("horizontalLayout_58");
        lista_amuletos = new QListWidget(groupBox_13);
        lista_amuletos->setObjectName("lista_amuletos");

        horizontalLayout_58->addWidget(lista_amuletos);

        verticalLayout_19 = new QVBoxLayout();
        verticalLayout_19->setObjectName("verticalLayout_19");
        botao_usar_amuleto = new QPushButton(groupBox_13);
        botao_usar_amuleto->setObjectName("botao_usar_amuleto");
        botao_usar_amuleto->setEnabled(false);

        verticalLayout_19->addWidget(botao_usar_amuleto);

        botao_adicionar_amuleto = new QPushButton(groupBox_13);
        botao_adicionar_amuleto->setObjectName("botao_adicionar_amuleto");

        verticalLayout_19->addWidget(botao_adicionar_amuleto);

        botao_remover_amuleto = new QPushButton(groupBox_13);
        botao_remover_amuleto->setObjectName("botao_remover_amuleto");

        verticalLayout_19->addWidget(botao_remover_amuleto);

        botao_doar_amuleto = new QPushButton(groupBox_13);
        botao_doar_amuleto->setObjectName("botao_doar_amuleto");

        verticalLayout_19->addWidget(botao_doar_amuleto);


        horizontalLayout_58->addLayout(verticalLayout_19);


        gridLayout_5->addWidget(groupBox_13, 1, 1, 1, 1);

        groupBox_18 = new QGroupBox(tab_2);
        groupBox_18->setObjectName("groupBox_18");
        horizontalLayout_63 = new QHBoxLayout(groupBox_18);
        horizontalLayout_63->setObjectName("horizontalLayout_63");
        lista_luvas = new QListWidget(groupBox_18);
        lista_luvas->setObjectName("lista_luvas");

        horizontalLayout_63->addWidget(lista_luvas);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setObjectName("verticalLayout_8");
        botao_usar_luvas = new QPushButton(groupBox_18);
        botao_usar_luvas->setObjectName("botao_usar_luvas");
        botao_usar_luvas->setEnabled(false);

        verticalLayout_8->addWidget(botao_usar_luvas);

        botao_adicionar_luvas = new QPushButton(groupBox_18);
        botao_adicionar_luvas->setObjectName("botao_adicionar_luvas");

        verticalLayout_8->addWidget(botao_adicionar_luvas);

        botao_remover_luvas = new QPushButton(groupBox_18);
        botao_remover_luvas->setObjectName("botao_remover_luvas");

        verticalLayout_8->addWidget(botao_remover_luvas);

        botao_doar_luvas = new QPushButton(groupBox_18);
        botao_doar_luvas->setObjectName("botao_doar_luvas");

        verticalLayout_8->addWidget(botao_doar_luvas);


        horizontalLayout_63->addLayout(verticalLayout_8);


        gridLayout_5->addWidget(groupBox_18, 1, 2, 1, 1);

        groupBox_24 = new QGroupBox(tab_2);
        groupBox_24->setObjectName("groupBox_24");
        horizontalLayout_73 = new QHBoxLayout(groupBox_24);
        horizontalLayout_73->setObjectName("horizontalLayout_73");
        lista_armas = new QListWidget(groupBox_24);
        lista_armas->setObjectName("lista_armas");

        horizontalLayout_73->addWidget(lista_armas);

        verticalLayout_29 = new QVBoxLayout();
        verticalLayout_29->setObjectName("verticalLayout_29");
        botao_adicionar_arma = new QPushButton(groupBox_24);
        botao_adicionar_arma->setObjectName("botao_adicionar_arma");

        verticalLayout_29->addWidget(botao_adicionar_arma);

        botao_duplicar_arma = new QPushButton(groupBox_24);
        botao_duplicar_arma->setObjectName("botao_duplicar_arma");

        verticalLayout_29->addWidget(botao_duplicar_arma);

        botao_remover_arma = new QPushButton(groupBox_24);
        botao_remover_arma->setObjectName("botao_remover_arma");

        verticalLayout_29->addWidget(botao_remover_arma);

        botao_ordenar_armas = new QPushButton(groupBox_24);
        botao_ordenar_armas->setObjectName("botao_ordenar_armas");

        verticalLayout_29->addWidget(botao_ordenar_armas);

        botao_doar_arma = new QPushButton(groupBox_24);
        botao_doar_arma->setObjectName("botao_doar_arma");

        verticalLayout_29->addWidget(botao_doar_arma);


        horizontalLayout_73->addLayout(verticalLayout_29);


        gridLayout_5->addWidget(groupBox_24, 3, 0, 1, 1);

        groupBox_19 = new QGroupBox(tab_2);
        groupBox_19->setObjectName("groupBox_19");
        horizontalLayout_64 = new QHBoxLayout(groupBox_19);
        horizontalLayout_64->setObjectName("horizontalLayout_64");
        lista_chapeus = new QListWidget(groupBox_19);
        lista_chapeus->setObjectName("lista_chapeus");

        horizontalLayout_64->addWidget(lista_chapeus);

        verticalLayout_20 = new QVBoxLayout();
        verticalLayout_20->setObjectName("verticalLayout_20");
        botao_vestir_chapeu = new QPushButton(groupBox_19);
        botao_vestir_chapeu->setObjectName("botao_vestir_chapeu");
        botao_vestir_chapeu->setEnabled(false);

        verticalLayout_20->addWidget(botao_vestir_chapeu);

        botao_adicionar_chapeu = new QPushButton(groupBox_19);
        botao_adicionar_chapeu->setObjectName("botao_adicionar_chapeu");

        verticalLayout_20->addWidget(botao_adicionar_chapeu);

        botao_remover_chapeu = new QPushButton(groupBox_19);
        botao_remover_chapeu->setObjectName("botao_remover_chapeu");

        verticalLayout_20->addWidget(botao_remover_chapeu);

        botao_doar_chapeu = new QPushButton(groupBox_19);
        botao_doar_chapeu->setObjectName("botao_doar_chapeu");

        verticalLayout_20->addWidget(botao_doar_chapeu);


        horizontalLayout_64->addLayout(verticalLayout_20);


        gridLayout_5->addWidget(groupBox_19, 1, 3, 1, 2);

        groupBox_23 = new QGroupBox(tab_2);
        groupBox_23->setObjectName("groupBox_23");
        horizontalLayout_68 = new QHBoxLayout(groupBox_23);
        horizontalLayout_68->setObjectName("horizontalLayout_68");
        lista_itens_mundanos = new QListWidget(groupBox_23);
        lista_itens_mundanos->setObjectName("lista_itens_mundanos");

        horizontalLayout_68->addWidget(lista_itens_mundanos);

        verticalLayout_11 = new QVBoxLayout();
        verticalLayout_11->setObjectName("verticalLayout_11");
        botao_adicionar_item_mundano = new QPushButton(groupBox_23);
        botao_adicionar_item_mundano->setObjectName("botao_adicionar_item_mundano");
        sizePolicy.setHeightForWidth(botao_adicionar_item_mundano->sizePolicy().hasHeightForWidth());
        botao_adicionar_item_mundano->setSizePolicy(sizePolicy);

        verticalLayout_11->addWidget(botao_adicionar_item_mundano);

        botao_duplicar_item_mundano = new QPushButton(groupBox_23);
        botao_duplicar_item_mundano->setObjectName("botao_duplicar_item_mundano");

        verticalLayout_11->addWidget(botao_duplicar_item_mundano);

        botao_remover_item_mundano = new QPushButton(groupBox_23);
        botao_remover_item_mundano->setObjectName("botao_remover_item_mundano");
        sizePolicy.setHeightForWidth(botao_remover_item_mundano->sizePolicy().hasHeightForWidth());
        botao_remover_item_mundano->setSizePolicy(sizePolicy);

        verticalLayout_11->addWidget(botao_remover_item_mundano);

        botao_ordenar_item_mundano = new QPushButton(groupBox_23);
        botao_ordenar_item_mundano->setObjectName("botao_ordenar_item_mundano");

        verticalLayout_11->addWidget(botao_ordenar_item_mundano);

        botao_doar_item_mundano = new QPushButton(groupBox_23);
        botao_doar_item_mundano->setObjectName("botao_doar_item_mundano");

        verticalLayout_11->addWidget(botao_doar_item_mundano);


        horizontalLayout_68->addLayout(verticalLayout_11);


        gridLayout_5->addWidget(groupBox_23, 2, 0, 1, 1);

        groupBox_17 = new QGroupBox(tab_2);
        groupBox_17->setObjectName("groupBox_17");
        horizontalLayout_62 = new QHBoxLayout(groupBox_17);
        horizontalLayout_62->setObjectName("horizontalLayout_62");
        lista_aneis = new QListWidget(groupBox_17);
        lista_aneis->setObjectName("lista_aneis");

        horizontalLayout_62->addWidget(lista_aneis);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName("verticalLayout_10");
        botao_usar_anel = new QPushButton(groupBox_17);
        botao_usar_anel->setObjectName("botao_usar_anel");
        botao_usar_anel->setEnabled(false);

        verticalLayout_10->addWidget(botao_usar_anel);

        botao_adicionar_anel = new QPushButton(groupBox_17);
        botao_adicionar_anel->setObjectName("botao_adicionar_anel");

        verticalLayout_10->addWidget(botao_adicionar_anel);

        botao_remover_anel = new QPushButton(groupBox_17);
        botao_remover_anel->setObjectName("botao_remover_anel");

        verticalLayout_10->addWidget(botao_remover_anel);

        botao_doar_anel = new QPushButton(groupBox_17);
        botao_doar_anel->setObjectName("botao_doar_anel");

        verticalLayout_10->addWidget(botao_doar_anel);


        horizontalLayout_62->addLayout(verticalLayout_10);


        gridLayout_5->addWidget(groupBox_17, 0, 2, 1, 1);

        groupBox_27 = new QGroupBox(tab_2);
        groupBox_27->setObjectName("groupBox_27");
        horizontalLayout_70 = new QHBoxLayout(groupBox_27);
        horizontalLayout_70->setObjectName("horizontalLayout_70");
        lista_varinhas = new QListWidget(groupBox_27);
        lista_varinhas->setObjectName("lista_varinhas");

        horizontalLayout_70->addWidget(lista_varinhas);

        verticalLayout_26 = new QVBoxLayout();
        verticalLayout_26->setObjectName("verticalLayout_26");
        botao_adicionar_varinha = new QPushButton(groupBox_27);
        botao_adicionar_varinha->setObjectName("botao_adicionar_varinha");
        sizePolicy.setHeightForWidth(botao_adicionar_varinha->sizePolicy().hasHeightForWidth());
        botao_adicionar_varinha->setSizePolicy(sizePolicy);

        verticalLayout_26->addWidget(botao_adicionar_varinha);

        botao_duplicar_varinha = new QPushButton(groupBox_27);
        botao_duplicar_varinha->setObjectName("botao_duplicar_varinha");

        verticalLayout_26->addWidget(botao_duplicar_varinha);

        botao_remover_varinha = new QPushButton(groupBox_27);
        botao_remover_varinha->setObjectName("botao_remover_varinha");
        sizePolicy.setHeightForWidth(botao_remover_varinha->sizePolicy().hasHeightForWidth());
        botao_remover_varinha->setSizePolicy(sizePolicy);

        verticalLayout_26->addWidget(botao_remover_varinha);

        botao_ordenar_varinhas = new QPushButton(groupBox_27);
        botao_ordenar_varinhas->setObjectName("botao_ordenar_varinhas");

        verticalLayout_26->addWidget(botao_ordenar_varinhas);

        botao_doar_varinha = new QPushButton(groupBox_27);
        botao_doar_varinha->setObjectName("botao_doar_varinha");

        verticalLayout_26->addWidget(botao_doar_varinha);


        horizontalLayout_70->addLayout(verticalLayout_26);


        gridLayout_5->addWidget(groupBox_27, 2, 3, 1, 1);

        groupBox_11 = new QGroupBox(tab_2);
        groupBox_11->setObjectName("groupBox_11");
        verticalLayout_21 = new QVBoxLayout(groupBox_11);
        verticalLayout_21->setObjectName("verticalLayout_21");
        horizontalLayout_53 = new QHBoxLayout();
        horizontalLayout_53->setObjectName("horizontalLayout_53");
        label_107 = new QLabel(groupBox_11);
        label_107->setObjectName("label_107");
        label_107->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_53->addWidget(label_107);

        spin_po = new QSpinBox(groupBox_11);
        spin_po->setObjectName("spin_po");
        spin_po->setMaximum(10000000);

        horizontalLayout_53->addWidget(spin_po);


        verticalLayout_21->addLayout(horizontalLayout_53);

        horizontalLayout_54 = new QHBoxLayout();
        horizontalLayout_54->setObjectName("horizontalLayout_54");
        label_108 = new QLabel(groupBox_11);
        label_108->setObjectName("label_108");
        label_108->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_54->addWidget(label_108);

        spin_pp = new QSpinBox(groupBox_11);
        spin_pp->setObjectName("spin_pp");
        spin_pp->setMaximum(10000000);

        horizontalLayout_54->addWidget(spin_pp);


        verticalLayout_21->addLayout(horizontalLayout_54);

        horizontalLayout_56 = new QHBoxLayout();
        horizontalLayout_56->setObjectName("horizontalLayout_56");
        label_109 = new QLabel(groupBox_11);
        label_109->setObjectName("label_109");
        label_109->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_56->addWidget(label_109);

        spin_pc = new QSpinBox(groupBox_11);
        spin_pc->setObjectName("spin_pc");
        spin_pc->setMaximum(10000000);

        horizontalLayout_56->addWidget(spin_pc);


        verticalLayout_21->addLayout(horizontalLayout_56);

        horizontalLayout_57 = new QHBoxLayout();
        horizontalLayout_57->setObjectName("horizontalLayout_57");
        label_110 = new QLabel(groupBox_11);
        label_110->setObjectName("label_110");
        label_110->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_57->addWidget(label_110);

        spin_pl = new QSpinBox(groupBox_11);
        spin_pl->setObjectName("spin_pl");
        spin_pl->setMaximum(10000000);

        horizontalLayout_57->addWidget(spin_pl);


        verticalLayout_21->addLayout(horizontalLayout_57);

        horizontalLayout_55 = new QHBoxLayout();
        horizontalLayout_55->setObjectName("horizontalLayout_55");
        labelpe = new QLabel(groupBox_11);
        labelpe->setObjectName("labelpe");
        labelpe->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_55->addWidget(labelpe);

        spin_pe = new QSpinBox(groupBox_11);
        spin_pe->setObjectName("spin_pe");
        spin_pe->setMaximum(10000000);

        horizontalLayout_55->addWidget(spin_pe);


        verticalLayout_21->addLayout(horizontalLayout_55);


        gridLayout_5->addWidget(groupBox_11, 3, 3, 1, 1);

        groupBox_21 = new QGroupBox(tab_2);
        groupBox_21->setObjectName("groupBox_21");
        horizontalLayout_66 = new QHBoxLayout(groupBox_21);
        horizontalLayout_66->setObjectName("horizontalLayout_66");
        lista_tesouro = new QPlainTextEdit(groupBox_21);
        lista_tesouro->setObjectName("lista_tesouro");

        horizontalLayout_66->addWidget(lista_tesouro);


        gridLayout_5->addWidget(groupBox_21, 4, 0, 1, 4);

        tabWidget->addTab(tab_2, QString());

        gridLayout_2->addWidget(tabWidget, 0, 0, 1, 3);

        QWidget::setTabOrder(campo_id, combo_tipo_forma);
        QWidget::setTabOrder(combo_tipo_forma, checkbox_respeita_solo);
        QWidget::setTabOrder(checkbox_respeita_solo, checkbox_dois_lados);
        QWidget::setTabOrder(checkbox_dois_lados, checkbox_ignora_luz);
        QWidget::setTabOrder(checkbox_ignora_luz, checkbox_especular);
        QWidget::setTabOrder(checkbox_especular, checkbox_selecionavel);
        QWidget::setTabOrder(checkbox_selecionavel, checkbox_fixa);
        QWidget::setTabOrder(checkbox_fixa, checkbox_faz_sombra);
        QWidget::setTabOrder(checkbox_faz_sombra, checkbox_afetado_por_efeitos);
        QWidget::setTabOrder(checkbox_afetado_por_efeitos, checkbox_colisao);
        QWidget::setTabOrder(checkbox_colisao, checkbox_visibilidade);
        QWidget::setTabOrder(checkbox_visibilidade, spin_escala_x_quad);
        QWidget::setTabOrder(spin_escala_x_quad, spin_escala_y_quad);
        QWidget::setTabOrder(spin_escala_y_quad, spin_escala_z_quad);
        QWidget::setTabOrder(spin_escala_z_quad, combo_transicao);
        QWidget::setTabOrder(combo_transicao, combo_id_cenario);
        QWidget::setTabOrder(combo_id_cenario, checkbox_transicao_posicao);
        QWidget::setTabOrder(checkbox_transicao_posicao, botao_transicao_mapa);
        QWidget::setTabOrder(botao_transicao_mapa, spin_trans_x);
        QWidget::setTabOrder(spin_trans_x, spin_trans_y);
        QWidget::setTabOrder(spin_trans_y, spin_trans_z);
        QWidget::setTabOrder(spin_trans_z, checkbox_cor);
        QWidget::setTabOrder(checkbox_cor, botao_cor);
        QWidget::setTabOrder(botao_cor, slider_alfa);
        QWidget::setTabOrder(slider_alfa, combo_textura);
        QWidget::setTabOrder(combo_textura, checkbox_ladrilho);
        QWidget::setTabOrder(checkbox_ladrilho, checkbox_bump);
        QWidget::setTabOrder(checkbox_bump, spin_tex_escala_x);
        QWidget::setTabOrder(spin_tex_escala_x, spin_tex_escala_y);
        QWidget::setTabOrder(spin_tex_escala_y, dial_tex_direcao);
        QWidget::setTabOrder(dial_tex_direcao, spin_tex_direcao);
        QWidget::setTabOrder(spin_tex_direcao, spin_tex_periodo);
        QWidget::setTabOrder(spin_tex_periodo, spin_pontos_vida);
        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, checkbox_luz);
        QWidget::setTabOrder(checkbox_luz, spin_raio_quad);
        QWidget::setTabOrder(spin_raio_quad, botao_luz);
        QWidget::setTabOrder(botao_luz, lista_rotulos);
        QWidget::setTabOrder(lista_rotulos, spin_translacao_quad);
        QWidget::setTabOrder(spin_translacao_quad, dial_rotacao);
        QWidget::setTabOrder(dial_rotacao, spin_rotacao);
        QWidget::setTabOrder(spin_rotacao, dial_rotacao_y);
        QWidget::setTabOrder(dial_rotacao_y, spin_rotacao_y);
        QWidget::setTabOrder(spin_rotacao_y, dial_rotacao_x);
        QWidget::setTabOrder(dial_rotacao_x, spin_rotacao_x);
        QWidget::setTabOrder(spin_rotacao_x, lista_pergaminhos_arcanos);
        QWidget::setTabOrder(lista_pergaminhos_arcanos, botao_adicionar_pergaminho_arcano);
        QWidget::setTabOrder(botao_adicionar_pergaminho_arcano, botao_duplicar_pergaminho_arcano);
        QWidget::setTabOrder(botao_duplicar_pergaminho_arcano, botao_remover_pergaminho_arcano);
        QWidget::setTabOrder(botao_remover_pergaminho_arcano, botao_ordenar_pergaminhos_arcanos);
        QWidget::setTabOrder(botao_ordenar_pergaminhos_arcanos, lista_pocoes);
        QWidget::setTabOrder(lista_pocoes, botao_adicionar_pocao);
        QWidget::setTabOrder(botao_adicionar_pocao, botao_duplicar_pocao);
        QWidget::setTabOrder(botao_duplicar_pocao, botao_remover_pocao);
        QWidget::setTabOrder(botao_remover_pocao, botao_ordenar_pocoes);
        QWidget::setTabOrder(botao_ordenar_pocoes, lista_aneis);
        QWidget::setTabOrder(lista_aneis, botao_usar_anel);
        QWidget::setTabOrder(botao_usar_anel, botao_adicionar_anel);
        QWidget::setTabOrder(botao_adicionar_anel, botao_remover_anel);
        QWidget::setTabOrder(botao_remover_anel, lista_mantos);
        QWidget::setTabOrder(lista_mantos, botao_usar_manto);
        QWidget::setTabOrder(botao_usar_manto, botao_adicionar_manto);
        QWidget::setTabOrder(botao_adicionar_manto, botao_remover_manto);
        QWidget::setTabOrder(botao_remover_manto, lista_pergaminhos_divinos);
        QWidget::setTabOrder(lista_pergaminhos_divinos, botao_adicionar_pergaminho_divino);
        QWidget::setTabOrder(botao_adicionar_pergaminho_divino, botao_duplicar_pergaminho_divino);
        QWidget::setTabOrder(botao_duplicar_pergaminho_divino, botao_remover_pergaminho_divino);
        QWidget::setTabOrder(botao_remover_pergaminho_divino, botao_ordenar_pergaminhos_divinos);
        QWidget::setTabOrder(botao_ordenar_pergaminhos_divinos, lista_amuletos);
        QWidget::setTabOrder(lista_amuletos, botao_usar_amuleto);
        QWidget::setTabOrder(botao_usar_amuleto, botao_adicionar_amuleto);
        QWidget::setTabOrder(botao_adicionar_amuleto, botao_remover_amuleto);
        QWidget::setTabOrder(botao_remover_amuleto, lista_luvas);
        QWidget::setTabOrder(lista_luvas, botao_usar_luvas);
        QWidget::setTabOrder(botao_usar_luvas, botao_adicionar_luvas);
        QWidget::setTabOrder(botao_adicionar_luvas, botao_remover_luvas);
        QWidget::setTabOrder(botao_remover_luvas, lista_chapeus);
        QWidget::setTabOrder(lista_chapeus, botao_vestir_chapeu);
        QWidget::setTabOrder(botao_vestir_chapeu, botao_adicionar_chapeu);
        QWidget::setTabOrder(botao_adicionar_chapeu, botao_remover_chapeu);
        QWidget::setTabOrder(botao_remover_chapeu, spin_po);
        QWidget::setTabOrder(spin_po, spin_pp);
        QWidget::setTabOrder(spin_pp, spin_pc);
        QWidget::setTabOrder(spin_pc, spin_pl);
        QWidget::setTabOrder(spin_pl, spin_pe);
        QWidget::setTabOrder(spin_pe, lista_botas);
        QWidget::setTabOrder(lista_botas, botao_usar_botas);
        QWidget::setTabOrder(botao_usar_botas, botao_adicionar_botas);
        QWidget::setTabOrder(botao_adicionar_botas, botao_remover_botas);
        QWidget::setTabOrder(botao_remover_botas, lista_bracadeiras);
        QWidget::setTabOrder(lista_bracadeiras, botao_usar_bracadeiras);
        QWidget::setTabOrder(botao_usar_bracadeiras, botao_adicionar_bracadeiras);
        QWidget::setTabOrder(botao_adicionar_bracadeiras, botao_remover_bracadeiras);
        QWidget::setTabOrder(botao_remover_bracadeiras, lista_tesouro);
        QWidget::setTabOrder(lista_tesouro, tabWidget);

        retranslateUi(ifg__qt__DialogoForma);
        QObject::connect(botoes, &QDialogButtonBox::rejected, ifg__qt__DialogoForma, qOverload<>(&QDialog::reject));
        QObject::connect(botoes, &QDialogButtonBox::accepted, ifg__qt__DialogoForma, qOverload<>(&QDialog::accept));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ifg__qt__DialogoForma);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoForma)
    {
        ifg__qt__DialogoForma->setWindowTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Id", nullptr));
        label_17->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Tipo", nullptr));
        combo_tipo_forma->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoForma", "Cilindro", nullptr));
        combo_tipo_forma->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoForma", "Cone", nullptr));
        combo_tipo_forma->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoForma", "Cubo", nullptr));
        combo_tipo_forma->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoForma", "Esfera", nullptr));
        combo_tipo_forma->setItemText(4, QCoreApplication::translate("ifg::qt::DialogoForma", "Pir\303\242mide", nullptr));
        combo_tipo_forma->setItemText(5, QCoreApplication::translate("ifg::qt::DialogoForma", "Hemisf\303\251rio", nullptr));

        groupBoxDim->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Dimens\303\265es (quadrados)", nullptr));
        label_7->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Tam X", nullptr));
        label_5->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Tam Y", nullptr));
        label_6->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Altura", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Atributos", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_fixa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade ser\303\241 fixa (implicando que n\303\243o ser\303\241 selecionada com um clique nem sele\303\247\303\243o de \303\241rea)", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_fixa->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Fixa", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_respeita_solo->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade n\303\243o afundar\303\241 no solo.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_respeita_solo->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Respeita Solo", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_colisao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, n\303\243o ser\303\241 poss\303\255vel colidor com a entidade.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_colisao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Colis\303\243o", nullptr));
        checkbox_visibilidade->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Vis\303\255vel", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_selecionavel->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade poder\303\241 ser selecionada por jogadores.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_selecionavel->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Selecion\303\241vel para jogadores", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_afetado_por_efeitos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Marque se objeto pode ser afetado por efeitos.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_afetado_por_efeitos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Afetado por Efeitos", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_ignora_luz->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade ser\303\241 desenhada com ilumina\303\247\303\243o total.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_ignora_luz->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ignora Luz", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_dois_lados->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nos dois lados da primitiva ser\303\243o desenhados.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_dois_lados->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Dois Lados", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_faz_sombra->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade n\303\243o far\303\241 sombra.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_faz_sombra->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Faz Sombra", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_especular->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, objeto brilhar\303\241 como metal", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_especular->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Especular", nullptr));
        checkbox_fumegando->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Fumegando", nullptr));
        groupBox_10->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos Especiais", nullptr));
#if QT_CONFIG(tooltip)
        lista_rotulos->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_8->setTitle(QString());
        groupBox->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Cor", nullptr));
        label_9->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Alfa", nullptr));
        label_3->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Habilitar", nullptr));
        checkbox_cor->setText(QString());
        botao_cor->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Escolher Cor", nullptr));
        groupBox1->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Textura", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_ladrilho->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_ladrilho->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "ladrilho", nullptr));
        checkbox_bump->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Bump", nullptr));
        label_4->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Escala X", nullptr));
        label_16->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Escala Y", nullptr));
        groupBox2->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Dire\303\247\303\243o e Per\303\255odo de Repeti\303\247\303\243o", nullptr));
        label_19->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Dire\303\247\303\243o", nullptr));
#if QT_CONFIG(tooltip)
        dial_tex_direcao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_textura_circular->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Se marcado, textura fara movimento circular de acordo com per\303\255odo", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_textura_circular->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Circular", nullptr));
        label_11->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Per\303\255odo (s)", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Transforma\303\247\303\265es", nullptr));
        label_2->setText(QCoreApplication::translate("ifg::qt::DialogoForma", " Transla\303\247\303\243o em Z", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em Z", nullptr));
#if QT_CONFIG(tooltip)
        dial_rotacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o do objeto ao redor do eixo Z.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_15->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "quad", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em Y", nullptr));
#if QT_CONFIG(tooltip)
        dial_rotacao_y->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_4->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em X", nullptr));
#if QT_CONFIG(tooltip)
        dial_rotacao_x->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        groupBox_9->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Ilumina\303\247\303\243o", nullptr));
        checkbox_luz->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Luz", nullptr));
        label_18->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Raio", nullptr));
#if QT_CONFIG(tooltip)
        spin_raio_quad->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Raio da luz, em metros.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_31->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "quadrados", nullptr));
        botao_luz->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Escolher Cor da Luz", nullptr));
        groupBoxPv->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida", nullptr));
        label_8->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Corrente", nullptr));
        label_10->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Max", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Transi\303\247\303\243o de Cen\303\241rio", nullptr));
        combo_transicao->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoForma", "Sem Transi\303\247\303\243o", nullptr));
        combo_transicao->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoForma", "Cen\303\241rio", nullptr));
        combo_transicao->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));

        checkbox_transicao_posicao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Posi\303\247\303\243o?", nullptr));
        botao_transicao_mapa->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Clicar", nullptr));
        label_12->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "X", nullptr));
        label_13->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Y", nullptr));
        label_14->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Z", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("ifg::qt::DialogoForma", "Geral", nullptr));
        groupBox_14->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Po\303\247\303\265es", nullptr));
        botao_adicionar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pocoes->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_pocao->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_22->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Bra\303\247adeiras", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setText(QString());
        botao_adicionar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_16->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Divinos", nullptr));
        botao_adicionar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_divinos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_15->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Arcanos", nullptr));
        botao_adicionar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_arcanos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_26->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Escudos", nullptr));
        botao_adicionar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_escudos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_escudo->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_25->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Armaduras", nullptr));
        botao_adicionar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_armaduras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_armadura->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_12->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Botas", nullptr));
        botao_usar_botas->setText(QString());
        botao_adicionar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_20->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Mantos", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_manto->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_manto->setText(QString());
        botao_adicionar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_13->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Amuletos", nullptr));
        botao_usar_amuleto->setText(QString());
        botao_adicionar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_18->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Luvas e Manoplas", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_luvas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_luvas->setText(QString());
        botao_adicionar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_24->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Armas", nullptr));
        botao_adicionar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_arma->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_armas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_arma->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_19->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Chap\303\251us", nullptr));
        botao_vestir_chapeu->setText(QString());
        botao_adicionar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_23->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Itens Mundanos", nullptr));
        botao_adicionar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_item_mundano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_17->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "An\303\251is", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_anel->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_anel->setText(QString());
        botao_adicionar_anel->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_anel->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_anel->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_27->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Varinhas", nullptr));
        botao_adicionar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_varinhas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_varinha->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_11->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Moedas", nullptr));
        label_107->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ouro", nullptr));
        label_108->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Prata", nullptr));
        label_109->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Cobre", nullptr));
        label_110->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Platina", nullptr));
        labelpe->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Electrum", nullptr));
        groupBox_21->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Outros", nullptr));
#if QT_CONFIG(tooltip)
        lista_tesouro->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_CONFIG(tooltip)
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoForma: public Ui_DialogoForma {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // FORMA_H
