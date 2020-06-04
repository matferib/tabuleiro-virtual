/********************************************************************************
** Form generated from reading UI file 'forma.ui'
**
** Created by: Qt User Interface Compiler version 5.14.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef FORMA_H
#define FORMA_H

#include <QtCore/QVariant>
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
    QVBoxLayout *verticalLayout_4;
    QLabel *label_11;
    QSpinBox *spin_tex_periodo;
    QSpacerItem *verticalSpacer;
    QLabel *label_19;
    QDial *dial_tex_direcao;
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
    QGroupBox *groupBox_12;
    QHBoxLayout *horizontalLayout_44;
    QListWidget *lista_botas;
    QVBoxLayout *verticalLayout_22;
    QPushButton *botao_usar_botas;
    QPushButton *botao_adicionar_botas;
    QPushButton *botao_remover_botas;
    QPushButton *botao_doar_botas;
    QGroupBox *groupBox_19;
    QHBoxLayout *horizontalLayout_64;
    QListWidget *lista_chapeus;
    QVBoxLayout *verticalLayout_20;
    QPushButton *botao_vestir_chapeu;
    QPushButton *botao_adicionar_chapeu;
    QPushButton *botao_remover_chapeu;
    QPushButton *botao_doar_chapeu;
    QGroupBox *groupBox_15;
    QHBoxLayout *horizontalLayout_60;
    QListWidget *lista_pergaminhos_arcanos;
    QVBoxLayout *verticalLayout_23;
    QPushButton *botao_adicionar_pergaminho_arcano;
    QPushButton *botao_duplicar_pergaminho_arcano;
    QPushButton *botao_remover_pergaminho_arcano;
    QPushButton *botao_ordenar_pergaminhos_arcanos;
    QPushButton *botao_doar_pergaminho_arcano;
    QGroupBox *groupBox_13;
    QHBoxLayout *horizontalLayout_58;
    QListWidget *lista_amuletos;
    QVBoxLayout *verticalLayout_19;
    QPushButton *botao_usar_amuleto;
    QPushButton *botao_adicionar_amuleto;
    QPushButton *botao_remover_amuleto;
    QPushButton *botao_doar_amuleto;
    QGroupBox *groupBox_22;
    QHBoxLayout *horizontalLayout_67;
    QListWidget *lista_bracadeiras;
    QVBoxLayout *verticalLayout_7;
    QPushButton *botao_usar_bracadeiras;
    QPushButton *botao_adicionar_bracadeiras;
    QPushButton *botao_remover_bracadeiras;
    QPushButton *botao_doar_bracadeiras;
    QGroupBox *groupBox_20;
    QHBoxLayout *horizontalLayout_65;
    QListWidget *lista_mantos;
    QVBoxLayout *verticalLayout_9;
    QPushButton *botao_usar_manto;
    QPushButton *botao_adicionar_manto;
    QPushButton *botao_remover_manto;
    QPushButton *botao_doar_manto;
    QGroupBox *groupBox_18;
    QHBoxLayout *horizontalLayout_63;
    QListWidget *lista_luvas;
    QVBoxLayout *verticalLayout_8;
    QPushButton *botao_usar_luvas;
    QPushButton *botao_adicionar_luvas;
    QPushButton *botao_remover_luvas;
    QPushButton *botao_doar_luvas;
    QGroupBox *groupBox_16;
    QHBoxLayout *horizontalLayout_61;
    QListWidget *lista_pergaminhos_divinos;
    QVBoxLayout *verticalLayout_24;
    QPushButton *botao_adicionar_pergaminho_divino;
    QPushButton *botao_duplicar_pergaminho_divino;
    QPushButton *botao_remover_pergaminho_divino;
    QPushButton *botao_ordenar_pergaminhos_divinos;
    QPushButton *botao_doar_pergaminho_divino;
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
            ifg__qt__DialogoForma->setObjectName(QString::fromUtf8("ifg__qt__DialogoForma"));
        ifg__qt__DialogoForma->resize(1403, 809);
        gridLayout_2 = new QGridLayout(ifg__qt__DialogoForma);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(botoes, 7, 2, 1, 1);

        tabWidget = new QTabWidget(ifg__qt__DialogoForma);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        gridLayout_4 = new QGridLayout(tab);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(tab);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(tab);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);


        gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QString::fromUtf8("horizontalLayout_21"));
        label_17 = new QLabel(tab);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_17);

        combo_tipo_forma = new QComboBox(tab);
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->setObjectName(QString::fromUtf8("combo_tipo_forma"));

        horizontalLayout_21->addWidget(combo_tipo_forma);


        gridLayout_4->addLayout(horizontalLayout_21, 0, 1, 1, 1);

        groupBoxDim = new QGroupBox(tab);
        groupBoxDim->setObjectName(QString::fromUtf8("groupBoxDim"));
        horizontalLayout_23 = new QHBoxLayout(groupBoxDim);
        horizontalLayout_23->setObjectName(QString::fromUtf8("horizontalLayout_23"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_7 = new QLabel(groupBoxDim);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_x_quad->setObjectName(QString::fromUtf8("spin_escala_x_quad"));
        spin_escala_x_quad->setDecimals(2);
        spin_escala_x_quad->setMinimum(-1000.000000000000000);
        spin_escala_x_quad->setMaximum(1000.000000000000000);
        spin_escala_x_quad->setSingleStep(0.100000000000000);

        horizontalLayout_4->addWidget(spin_escala_x_quad);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_5 = new QLabel(groupBoxDim);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_y_quad->setObjectName(QString::fromUtf8("spin_escala_y_quad"));
        spin_escala_y_quad->setDecimals(2);
        spin_escala_y_quad->setMinimum(-1000.000000000000000);
        spin_escala_y_quad->setMaximum(1000.000000000000000);
        spin_escala_y_quad->setSingleStep(0.100000000000000);

        horizontalLayout_5->addWidget(spin_escala_y_quad);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(groupBoxDim);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_z_quad->setObjectName(QString::fromUtf8("spin_escala_z_quad"));
        spin_escala_z_quad->setDecimals(2);
        spin_escala_z_quad->setMinimum(-50.000000000000000);
        spin_escala_z_quad->setMaximum(50.000000000000000);
        spin_escala_z_quad->setSingleStep(0.100000000000000);

        horizontalLayout_6->addWidget(spin_escala_z_quad);


        verticalLayout->addLayout(horizontalLayout_6);


        horizontalLayout_23->addLayout(verticalLayout);


        gridLayout_4->addWidget(groupBoxDim, 1, 1, 1, 1);

        groupBox_7 = new QGroupBox(tab);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        gridLayout_3 = new QGridLayout(groupBox_7);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        checkbox_fixa = new QCheckBox(groupBox_7);
        checkbox_fixa->setObjectName(QString::fromUtf8("checkbox_fixa"));

        gridLayout_3->addWidget(checkbox_fixa, 1, 1, 1, 1);

        checkbox_respeita_solo = new QCheckBox(groupBox_7);
        checkbox_respeita_solo->setObjectName(QString::fromUtf8("checkbox_respeita_solo"));

        gridLayout_3->addWidget(checkbox_respeita_solo, 0, 0, 1, 1);

        checkbox_colisao = new QCheckBox(groupBox_7);
        checkbox_colisao->setObjectName(QString::fromUtf8("checkbox_colisao"));

        gridLayout_3->addWidget(checkbox_colisao, 2, 1, 1, 1);

        checkbox_visibilidade = new QCheckBox(groupBox_7);
        checkbox_visibilidade->setObjectName(QString::fromUtf8("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(checkbox_visibilidade, 2, 2, 1, 1);

        checkbox_selecionavel = new QCheckBox(groupBox_7);
        checkbox_selecionavel->setObjectName(QString::fromUtf8("checkbox_selecionavel"));

        gridLayout_3->addWidget(checkbox_selecionavel, 1, 0, 1, 1);

        checkbox_afetado_por_efeitos = new QCheckBox(groupBox_7);
        checkbox_afetado_por_efeitos->setObjectName(QString::fromUtf8("checkbox_afetado_por_efeitos"));

        gridLayout_3->addWidget(checkbox_afetado_por_efeitos, 2, 0, 1, 1);

        checkbox_ignora_luz = new QCheckBox(groupBox_7);
        checkbox_ignora_luz->setObjectName(QString::fromUtf8("checkbox_ignora_luz"));

        gridLayout_3->addWidget(checkbox_ignora_luz, 0, 2, 1, 1);

        checkbox_dois_lados = new QCheckBox(groupBox_7);
        checkbox_dois_lados->setObjectName(QString::fromUtf8("checkbox_dois_lados"));

        gridLayout_3->addWidget(checkbox_dois_lados, 0, 1, 1, 1);

        checkbox_faz_sombra = new QCheckBox(groupBox_7);
        checkbox_faz_sombra->setObjectName(QString::fromUtf8("checkbox_faz_sombra"));

        gridLayout_3->addWidget(checkbox_faz_sombra, 1, 2, 1, 1);

        checkbox_especular = new QCheckBox(groupBox_7);
        checkbox_especular->setObjectName(QString::fromUtf8("checkbox_especular"));

        gridLayout_3->addWidget(checkbox_especular, 0, 3, 1, 1);

        checkbox_fumegando = new QCheckBox(groupBox_7);
        checkbox_fumegando->setObjectName(QString::fromUtf8("checkbox_fumegando"));

        gridLayout_3->addWidget(checkbox_fumegando, 1, 3, 1, 1);


        gridLayout_4->addWidget(groupBox_7, 1, 0, 1, 1);

        groupBox_10 = new QGroupBox(tab);
        groupBox_10->setObjectName(QString::fromUtf8("groupBox_10"));
        horizontalLayout_11 = new QHBoxLayout(groupBox_10);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        lista_rotulos = new QPlainTextEdit(groupBox_10);
        lista_rotulos->setObjectName(QString::fromUtf8("lista_rotulos"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lista_rotulos->sizePolicy().hasHeightForWidth());
        lista_rotulos->setSizePolicy(sizePolicy1);

        horizontalLayout_11->addWidget(lista_rotulos);


        gridLayout_4->addWidget(groupBox_10, 6, 0, 1, 1);

        groupBox_8 = new QGroupBox(tab);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        horizontalLayout_26 = new QHBoxLayout(groupBox_8);
        horizontalLayout_26->setObjectName(QString::fromUtf8("horizontalLayout_26"));
        horizontalLayout_24 = new QHBoxLayout();
        horizontalLayout_24->setObjectName(QString::fromUtf8("horizontalLayout_24"));
        groupBox = new QGroupBox(groupBox_8);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_8 = new QGridLayout(groupBox);
        gridLayout_8->setObjectName(QString::fromUtf8("gridLayout_8"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label_9);

        slider_alfa = new QSlider(groupBox);
        slider_alfa->setObjectName(QString::fromUtf8("slider_alfa"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy2);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(slider_alfa);


        gridLayout_8->addLayout(horizontalLayout_2, 1, 0, 1, 1);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy3);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(groupBox);
        checkbox_cor->setObjectName(QString::fromUtf8("checkbox_cor"));
        sizePolicy2.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy2);

        horizontalLayout_7->addWidget(checkbox_cor);

        botao_cor = new QPushButton(groupBox);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_7->addWidget(botao_cor);


        gridLayout_8->addLayout(horizontalLayout_7, 0, 0, 1, 1);


        horizontalLayout_24->addWidget(groupBox);


        horizontalLayout_26->addLayout(horizontalLayout_24);

        groupBox1 = new QGroupBox(groupBox_8);
        groupBox1->setObjectName(QString::fromUtf8("groupBox1"));
        gridLayout_6 = new QGridLayout(groupBox1);
        gridLayout_6->setObjectName(QString::fromUtf8("gridLayout_6"));
        combo_textura = new QComboBox(groupBox1);
        combo_textura->setObjectName(QString::fromUtf8("combo_textura"));

        gridLayout_6->addWidget(combo_textura, 0, 0, 1, 1);

        horizontalLayout_25 = new QHBoxLayout();
        horizontalLayout_25->setObjectName(QString::fromUtf8("horizontalLayout_25"));
        checkbox_ladrilho = new QCheckBox(groupBox1);
        checkbox_ladrilho->setObjectName(QString::fromUtf8("checkbox_ladrilho"));
        sizePolicy2.setHeightForWidth(checkbox_ladrilho->sizePolicy().hasHeightForWidth());
        checkbox_ladrilho->setSizePolicy(sizePolicy2);

        horizontalLayout_25->addWidget(checkbox_ladrilho);

        checkbox_bump = new QCheckBox(groupBox1);
        checkbox_bump->setObjectName(QString::fromUtf8("checkbox_bump"));

        horizontalLayout_25->addWidget(checkbox_bump);


        gridLayout_6->addLayout(horizontalLayout_25, 0, 1, 1, 1);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        label_4 = new QLabel(groupBox1);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_20->addWidget(label_4);

        spin_tex_escala_x = new QDoubleSpinBox(groupBox1);
        spin_tex_escala_x->setObjectName(QString::fromUtf8("spin_tex_escala_x"));
        spin_tex_escala_x->setMaximum(1000.000000000000000);

        horizontalLayout_20->addWidget(spin_tex_escala_x);


        gridLayout_6->addLayout(horizontalLayout_20, 1, 0, 1, 1);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label_16 = new QLabel(groupBox1);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        horizontalLayout_8->addWidget(label_16);

        spin_tex_escala_y = new QDoubleSpinBox(groupBox1);
        spin_tex_escala_y->setObjectName(QString::fromUtf8("spin_tex_escala_y"));
        spin_tex_escala_y->setMaximum(1000.000000000000000);

        horizontalLayout_8->addWidget(spin_tex_escala_y);


        gridLayout_6->addLayout(horizontalLayout_8, 1, 1, 1, 1);

        groupBox2 = new QGroupBox(groupBox1);
        groupBox2->setObjectName(QString::fromUtf8("groupBox2"));
        gridLayout_7 = new QGridLayout(groupBox2);
        gridLayout_7->setObjectName(QString::fromUtf8("gridLayout_7"));
        spin_tex_direcao = new QSpinBox(groupBox2);
        spin_tex_direcao->setObjectName(QString::fromUtf8("spin_tex_direcao"));
        spin_tex_direcao->setMinimum(-180);
        spin_tex_direcao->setMaximum(180);

        gridLayout_7->addWidget(spin_tex_direcao, 2, 0, 1, 1);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label_11 = new QLabel(groupBox2);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setAlignment(Qt::AlignCenter);

        verticalLayout_4->addWidget(label_11);

        spin_tex_periodo = new QSpinBox(groupBox2);
        spin_tex_periodo->setObjectName(QString::fromUtf8("spin_tex_periodo"));
        spin_tex_periodo->setMaximum(1000);

        verticalLayout_4->addWidget(spin_tex_periodo);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);


        gridLayout_7->addLayout(verticalLayout_4, 0, 2, 3, 1);

        label_19 = new QLabel(groupBox2);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setAlignment(Qt::AlignCenter);

        gridLayout_7->addWidget(label_19, 0, 0, 1, 1);

        dial_tex_direcao = new QDial(groupBox2);
        dial_tex_direcao->setObjectName(QString::fromUtf8("dial_tex_direcao"));
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

        gridLayout_7->addWidget(dial_tex_direcao, 1, 0, 1, 1);


        gridLayout_6->addWidget(groupBox2, 0, 2, 2, 1);


        horizontalLayout_26->addWidget(groupBox1);


        gridLayout_4->addWidget(groupBox_8, 3, 1, 1, 1);

        groupBox_6 = new QGroupBox(tab);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        horizontalLayout_22 = new QHBoxLayout(groupBox_6);
        horizontalLayout_22->setObjectName(QString::fromUtf8("horizontalLayout_22"));
        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        label_2 = new QLabel(groupBox_6);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_18->addWidget(label_2);

        spin_translacao_quad = new QDoubleSpinBox(groupBox_6);
        spin_translacao_quad->setObjectName(QString::fromUtf8("spin_translacao_quad"));
        spin_translacao_quad->setDecimals(2);
        spin_translacao_quad->setMinimum(-100.000000000000000);
        spin_translacao_quad->setMaximum(100.000000000000000);
        spin_translacao_quad->setSingleStep(0.100000000000000);

        horizontalLayout_18->addWidget(spin_translacao_quad);


        horizontalLayout_22->addLayout(horizontalLayout_18);

        groupBox_2 = new QGroupBox(groupBox_6);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        dial_rotacao = new QDial(groupBox_2);
        dial_rotacao->setObjectName(QString::fromUtf8("dial_rotacao"));
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
        spin_rotacao->setObjectName(QString::fromUtf8("spin_rotacao"));
        spin_rotacao->setGeometry(QRect(20, 100, 51, 24));
        spin_rotacao->setMinimum(-180);
        spin_rotacao->setMaximum(180);
        label_15 = new QLabel(groupBox_2);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(170, 100, 41, 16));

        horizontalLayout_22->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(groupBox_6);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        dial_rotacao_y = new QDial(groupBox_3);
        dial_rotacao_y->setObjectName(QString::fromUtf8("dial_rotacao_y"));
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
        spin_rotacao_y->setObjectName(QString::fromUtf8("spin_rotacao_y"));
        spin_rotacao_y->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_y->setMinimum(-180);
        spin_rotacao_y->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(groupBox_6);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        dial_rotacao_x = new QDial(groupBox_4);
        dial_rotacao_x->setObjectName(QString::fromUtf8("dial_rotacao_x"));
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
        spin_rotacao_x->setObjectName(QString::fromUtf8("spin_rotacao_x"));
        spin_rotacao_x->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_x->setMinimum(-180);
        spin_rotacao_x->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_4);


        gridLayout_4->addWidget(groupBox_6, 6, 1, 1, 1);

        groupBox_9 = new QGroupBox(tab);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        horizontalLayout_27 = new QHBoxLayout(groupBox_9);
        horizontalLayout_27->setObjectName(QString::fromUtf8("horizontalLayout_27"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkbox_luz = new QCheckBox(groupBox_9);
        checkbox_luz->setObjectName(QString::fromUtf8("checkbox_luz"));
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        label_18 = new QLabel(groupBox_9);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_18);

        spin_raio_quad = new QDoubleSpinBox(groupBox_9);
        spin_raio_quad->setObjectName(QString::fromUtf8("spin_raio_quad"));
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1.000000000000000);

        horizontalLayout_3->addWidget(spin_raio_quad);

        label_31 = new QLabel(groupBox_9);
        label_31->setObjectName(QString::fromUtf8("label_31"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy4);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_31);

        botao_luz = new QPushButton(groupBox_9);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(botao_luz);


        horizontalLayout_27->addLayout(horizontalLayout_3);


        gridLayout_4->addWidget(groupBox_9, 4, 1, 1, 1);

        groupBoxPv = new QGroupBox(tab);
        groupBoxPv->setObjectName(QString::fromUtf8("groupBoxPv"));
        horizontalLayout_13 = new QHBoxLayout(groupBoxPv);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        label_8 = new QLabel(groupBoxPv);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(groupBoxPv);
        spin_pontos_vida->setObjectName(QString::fromUtf8("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(groupBoxPv);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy3.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy3);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(groupBoxPv);
        spin_max_pontos_vida->setObjectName(QString::fromUtf8("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);


        gridLayout_4->addWidget(groupBoxPv, 4, 0, 1, 1);

        groupBox_5 = new QGroupBox(tab);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout = new QGridLayout(groupBox_5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        combo_transicao = new QComboBox(groupBox_5);
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->setObjectName(QString::fromUtf8("combo_transicao"));

        horizontalLayout_19->addWidget(combo_transicao);

        combo_id_cenario = new QComboBox(groupBox_5);
        combo_id_cenario->setObjectName(QString::fromUtf8("combo_id_cenario"));

        horizontalLayout_19->addWidget(combo_id_cenario);

        checkbox_transicao_posicao = new QCheckBox(groupBox_5);
        checkbox_transicao_posicao->setObjectName(QString::fromUtf8("checkbox_transicao_posicao"));

        horizontalLayout_19->addWidget(checkbox_transicao_posicao);

        botao_transicao_mapa = new QPushButton(groupBox_5);
        botao_transicao_mapa->setObjectName(QString::fromUtf8("botao_transicao_mapa"));
        sizePolicy2.setHeightForWidth(botao_transicao_mapa->sizePolicy().hasHeightForWidth());
        botao_transicao_mapa->setSizePolicy(sizePolicy2);

        horizontalLayout_19->addWidget(botao_transicao_mapa);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy5);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_12);

        spin_trans_x = new QDoubleSpinBox(groupBox_5);
        spin_trans_x->setObjectName(QString::fromUtf8("spin_trans_x"));
        spin_trans_x->setDecimals(1);
        spin_trans_x->setMinimum(-1000.000000000000000);
        spin_trans_x->setMaximum(1000.000000000000000);
        spin_trans_x->setSingleStep(0.500000000000000);

        horizontalLayout_15->addWidget(spin_trans_x);


        horizontalLayout_14->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_16->addWidget(label_13);

        spin_trans_y = new QDoubleSpinBox(groupBox_5);
        spin_trans_y->setObjectName(QString::fromUtf8("spin_trans_y"));
        spin_trans_y->setDecimals(1);
        spin_trans_y->setMinimum(-1000.000000000000000);
        spin_trans_y->setMaximum(1000.000000000000000);
        spin_trans_y->setSingleStep(0.500000000000000);

        horizontalLayout_16->addWidget(spin_trans_y);


        horizontalLayout_14->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_17->addWidget(label_14);

        spin_trans_z = new QDoubleSpinBox(groupBox_5);
        spin_trans_z->setObjectName(QString::fromUtf8("spin_trans_z"));
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
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        gridLayout_5 = new QGridLayout(tab_2);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        groupBox_14 = new QGroupBox(tab_2);
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
        sizePolicy.setHeightForWidth(botao_adicionar_pocao->sizePolicy().hasHeightForWidth());
        botao_adicionar_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_adicionar_pocao);

        botao_duplicar_pocao = new QPushButton(groupBox_14);
        botao_duplicar_pocao->setObjectName(QString::fromUtf8("botao_duplicar_pocao"));

        verticalLayout_6->addWidget(botao_duplicar_pocao);

        botao_remover_pocao = new QPushButton(groupBox_14);
        botao_remover_pocao->setObjectName(QString::fromUtf8("botao_remover_pocao"));
        sizePolicy.setHeightForWidth(botao_remover_pocao->sizePolicy().hasHeightForWidth());
        botao_remover_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_remover_pocao);

        botao_ordenar_pocoes = new QPushButton(groupBox_14);
        botao_ordenar_pocoes->setObjectName(QString::fromUtf8("botao_ordenar_pocoes"));

        verticalLayout_6->addWidget(botao_ordenar_pocoes);

        botao_doar_pocao = new QPushButton(groupBox_14);
        botao_doar_pocao->setObjectName(QString::fromUtf8("botao_doar_pocao"));

        verticalLayout_6->addWidget(botao_doar_pocao);


        horizontalLayout_59->addLayout(verticalLayout_6);


        gridLayout_5->addWidget(groupBox_14, 0, 1, 1, 1);

        groupBox_23 = new QGroupBox(tab_2);
        groupBox_23->setObjectName(QString::fromUtf8("groupBox_23"));
        horizontalLayout_68 = new QHBoxLayout(groupBox_23);
        horizontalLayout_68->setObjectName(QString::fromUtf8("horizontalLayout_68"));
        lista_itens_mundanos = new QListWidget(groupBox_23);
        lista_itens_mundanos->setObjectName(QString::fromUtf8("lista_itens_mundanos"));

        horizontalLayout_68->addWidget(lista_itens_mundanos);

        verticalLayout_11 = new QVBoxLayout();
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        botao_adicionar_item_mundano = new QPushButton(groupBox_23);
        botao_adicionar_item_mundano->setObjectName(QString::fromUtf8("botao_adicionar_item_mundano"));
        sizePolicy.setHeightForWidth(botao_adicionar_item_mundano->sizePolicy().hasHeightForWidth());
        botao_adicionar_item_mundano->setSizePolicy(sizePolicy);

        verticalLayout_11->addWidget(botao_adicionar_item_mundano);

        botao_duplicar_item_mundano = new QPushButton(groupBox_23);
        botao_duplicar_item_mundano->setObjectName(QString::fromUtf8("botao_duplicar_item_mundano"));

        verticalLayout_11->addWidget(botao_duplicar_item_mundano);

        botao_remover_item_mundano = new QPushButton(groupBox_23);
        botao_remover_item_mundano->setObjectName(QString::fromUtf8("botao_remover_item_mundano"));
        sizePolicy.setHeightForWidth(botao_remover_item_mundano->sizePolicy().hasHeightForWidth());
        botao_remover_item_mundano->setSizePolicy(sizePolicy);

        verticalLayout_11->addWidget(botao_remover_item_mundano);

        botao_ordenar_item_mundano = new QPushButton(groupBox_23);
        botao_ordenar_item_mundano->setObjectName(QString::fromUtf8("botao_ordenar_item_mundano"));

        verticalLayout_11->addWidget(botao_ordenar_item_mundano);

        botao_doar_item_mundano = new QPushButton(groupBox_23);
        botao_doar_item_mundano->setObjectName(QString::fromUtf8("botao_doar_item_mundano"));

        verticalLayout_11->addWidget(botao_doar_item_mundano);


        horizontalLayout_68->addLayout(verticalLayout_11);


        gridLayout_5->addWidget(groupBox_23, 2, 0, 1, 1);

        groupBox_17 = new QGroupBox(tab_2);
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
        botao_usar_anel->setEnabled(false);

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


        gridLayout_5->addWidget(groupBox_17, 0, 2, 1, 1);

        groupBox_12 = new QGroupBox(tab_2);
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
        botao_usar_botas->setEnabled(false);

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


        gridLayout_5->addWidget(groupBox_12, 2, 1, 1, 1);

        groupBox_19 = new QGroupBox(tab_2);
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
        botao_vestir_chapeu->setEnabled(false);

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


        gridLayout_5->addWidget(groupBox_19, 1, 3, 1, 1);

        groupBox_15 = new QGroupBox(tab_2);
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


        gridLayout_5->addWidget(groupBox_15, 0, 0, 1, 1);

        groupBox_13 = new QGroupBox(tab_2);
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
        botao_usar_amuleto->setEnabled(false);

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


        gridLayout_5->addWidget(groupBox_13, 1, 1, 1, 1);

        groupBox_22 = new QGroupBox(tab_2);
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


        gridLayout_5->addWidget(groupBox_22, 2, 2, 1, 1);

        groupBox_20 = new QGroupBox(tab_2);
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
        botao_usar_manto->setEnabled(false);
        sizePolicy.setHeightForWidth(botao_usar_manto->sizePolicy().hasHeightForWidth());
        botao_usar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_usar_manto);

        botao_adicionar_manto = new QPushButton(groupBox_20);
        botao_adicionar_manto->setObjectName(QString::fromUtf8("botao_adicionar_manto"));
        sizePolicy.setHeightForWidth(botao_adicionar_manto->sizePolicy().hasHeightForWidth());
        botao_adicionar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_adicionar_manto);

        botao_remover_manto = new QPushButton(groupBox_20);
        botao_remover_manto->setObjectName(QString::fromUtf8("botao_remover_manto"));
        sizePolicy.setHeightForWidth(botao_remover_manto->sizePolicy().hasHeightForWidth());
        botao_remover_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_remover_manto);

        botao_doar_manto = new QPushButton(groupBox_20);
        botao_doar_manto->setObjectName(QString::fromUtf8("botao_doar_manto"));

        verticalLayout_9->addWidget(botao_doar_manto);


        horizontalLayout_65->addLayout(verticalLayout_9);


        gridLayout_5->addWidget(groupBox_20, 0, 3, 1, 1);

        groupBox_18 = new QGroupBox(tab_2);
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
        botao_usar_luvas->setEnabled(false);

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


        gridLayout_5->addWidget(groupBox_18, 1, 2, 1, 1);

        groupBox_16 = new QGroupBox(tab_2);
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


        gridLayout_5->addWidget(groupBox_16, 1, 0, 1, 1);

        groupBox_11 = new QGroupBox(tab_2);
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


        gridLayout_5->addWidget(groupBox_11, 2, 3, 1, 1);

        groupBox_21 = new QGroupBox(tab_2);
        groupBox_21->setObjectName(QString::fromUtf8("groupBox_21"));
        horizontalLayout_66 = new QHBoxLayout(groupBox_21);
        horizontalLayout_66->setObjectName(QString::fromUtf8("horizontalLayout_66"));
        lista_tesouro = new QPlainTextEdit(groupBox_21);
        lista_tesouro->setObjectName(QString::fromUtf8("lista_tesouro"));

        horizontalLayout_66->addWidget(lista_tesouro);


        gridLayout_5->addWidget(groupBox_21, 3, 0, 1, 4);

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
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoForma, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoForma, SLOT(accept()));

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
        label_11->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Per\303\255odo (s)", nullptr));
        label_19->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Dire\303\247\303\243o", nullptr));
#if QT_CONFIG(tooltip)
        dial_tex_direcao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
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
        groupBox_12->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Botas", nullptr));
        botao_usar_botas->setText(QString());
        botao_adicionar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_botas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_19->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Chap\303\251us", nullptr));
        botao_vestir_chapeu->setText(QString());
        botao_adicionar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_chapeu->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_15->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Arcanos", nullptr));
        botao_adicionar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_arcanos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_pergaminho_arcano->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_13->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Amuletos", nullptr));
        botao_usar_amuleto->setText(QString());
        botao_adicionar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_amuleto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_22->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Bra\303\247adeiras", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_bracadeiras->setText(QString());
        botao_adicionar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_bracadeiras->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_20->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Mantos", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_manto->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_manto->setText(QString());
        botao_adicionar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_manto->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_18->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Luvas e Manoplas", nullptr));
#if QT_CONFIG(tooltip)
        botao_usar_luvas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_usar_luvas->setText(QString());
        botao_adicionar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_doar_luvas->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
        groupBox_16->setTitle(QCoreApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Divinos", nullptr));
        botao_adicionar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_divinos->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        botao_doar_pergaminho_divino->setText(QCoreApplication::translate("ifg::qt::DialogoForma", "Doar", nullptr));
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
