/********************************************************************************
** Form generated from reading UI file 'forma.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
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
    QGroupBox *groupBox_10;
    QHBoxLayout *horizontalLayout_11;
    QPlainTextEdit *lista_rotulos;
    QGroupBox *groupBox_8;
    QHBoxLayout *horizontalLayout_26;
    QHBoxLayout *horizontalLayout_24;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QHBoxLayout *horizontalLayout_9;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_12;
    QSlider *slider_alfa;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_4;
    QComboBox *combo_textura;
    QCheckBox *checkbox_ladrilho;
    QCheckBox *checkbox_bump;
    QGroupBox *groupBox_6;
    QHBoxLayout *horizontalLayout_22;
    QGroupBox *groupBox_2;
    QDial *dial_rotacao;
    QDoubleSpinBox *spin_translacao_quad;
    QLabel *label_2;
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
    QGroupBox *groupBox_13;
    QHBoxLayout *horizontalLayout_58;
    QListWidget *lista_amuletos;
    QVBoxLayout *verticalLayout_19;
    QPushButton *botao_usar_amuleto;
    QPushButton *botao_adicionar_amuleto;
    QPushButton *botao_remover_amuleto;
    QGroupBox *groupBox_20;
    QHBoxLayout *horizontalLayout_65;
    QListWidget *lista_mantos;
    QVBoxLayout *verticalLayout_9;
    QPushButton *botao_usar_manto;
    QPushButton *botao_adicionar_manto;
    QPushButton *botao_remover_manto;
    QGroupBox *groupBox_15;
    QHBoxLayout *horizontalLayout_60;
    QListWidget *lista_pergaminhos_arcanos;
    QVBoxLayout *verticalLayout_23;
    QPushButton *botao_adicionar_pergaminho_arcano;
    QPushButton *botao_duplicar_pergaminho_arcano;
    QPushButton *botao_remover_pergaminho_arcano;
    QPushButton *botao_ordenar_pergaminhos_arcanos;
    QGroupBox *groupBox_18;
    QHBoxLayout *horizontalLayout_63;
    QListWidget *lista_luvas;
    QVBoxLayout *verticalLayout_8;
    QPushButton *botao_usar_luvas;
    QPushButton *botao_adicionar_luvas;
    QPushButton *botao_remover_luvas;
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
    QGroupBox *groupBox_12;
    QHBoxLayout *horizontalLayout_44;
    QListWidget *lista_botas;
    QVBoxLayout *verticalLayout_22;
    QPushButton *botao_usar_botas;
    QPushButton *botao_adicionar_botas;
    QPushButton *botao_remover_botas;
    QGroupBox *groupBox_22;
    QHBoxLayout *horizontalLayout_67;
    QListWidget *lista_bracadeiras;
    QVBoxLayout *verticalLayout_7;
    QPushButton *botao_usar_bracadeiras;
    QPushButton *botao_adicionar_bracadeiras;
    QPushButton *botao_remover_bracadeiras;
    QGroupBox *groupBox_17;
    QHBoxLayout *horizontalLayout_62;
    QListWidget *lista_aneis;
    QVBoxLayout *verticalLayout_10;
    QPushButton *botao_usar_anel;
    QPushButton *botao_adicionar_anel;
    QPushButton *botao_remover_anel;
    QGroupBox *groupBox_14;
    QHBoxLayout *horizontalLayout_59;
    QListWidget *lista_pocoes;
    QVBoxLayout *verticalLayout_6;
    QPushButton *botao_adicionar_pocao;
    QPushButton *botao_duplicar_pocao;
    QPushButton *botao_remover_pocao;
    QPushButton *botao_ordenar_pocoes;
    QGroupBox *groupBox_16;
    QHBoxLayout *horizontalLayout_61;
    QListWidget *lista_pergaminhos_divinos;
    QVBoxLayout *verticalLayout_24;
    QPushButton *botao_adicionar_pergaminho_divino;
    QPushButton *botao_duplicar_pergaminho_divino;
    QPushButton *botao_remover_pergaminho_divino;
    QPushButton *botao_ordenar_pergaminhos_divinos;
    QGroupBox *groupBox_21;
    QHBoxLayout *horizontalLayout_66;
    QPlainTextEdit *lista_tesouro;
    QGroupBox *groupBox_19;
    QHBoxLayout *horizontalLayout_64;
    QListWidget *lista_chapeus;
    QVBoxLayout *verticalLayout_20;
    QPushButton *botao_vestir_chapeu;
    QPushButton *botao_adicionar_chapeu;
    QPushButton *botao_remover_chapeu;

    void setupUi(QDialog *ifg__qt__DialogoForma)
    {
        if (ifg__qt__DialogoForma->objectName().isEmpty())
            ifg__qt__DialogoForma->setObjectName(QStringLiteral("ifg__qt__DialogoForma"));
        ifg__qt__DialogoForma->resize(1403, 771);
        gridLayout_2 = new QGridLayout(ifg__qt__DialogoForma);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(botoes, 7, 2, 1, 1);

        tabWidget = new QTabWidget(ifg__qt__DialogoForma);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        gridLayout_4 = new QGridLayout(tab);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(tab);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(tab);
        campo_id->setObjectName(QStringLiteral("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);


        gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QStringLiteral("horizontalLayout_21"));
        label_17 = new QLabel(tab);
        label_17->setObjectName(QStringLiteral("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_21->addWidget(label_17);

        combo_tipo_forma = new QComboBox(tab);
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->setObjectName(QStringLiteral("combo_tipo_forma"));

        horizontalLayout_21->addWidget(combo_tipo_forma);


        gridLayout_4->addLayout(horizontalLayout_21, 0, 1, 1, 1);

        groupBoxDim = new QGroupBox(tab);
        groupBoxDim->setObjectName(QStringLiteral("groupBoxDim"));
        horizontalLayout_23 = new QHBoxLayout(groupBoxDim);
        horizontalLayout_23->setObjectName(QStringLiteral("horizontalLayout_23"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_7 = new QLabel(groupBoxDim);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_x_quad->setObjectName(QStringLiteral("spin_escala_x_quad"));
        spin_escala_x_quad->setDecimals(2);
        spin_escala_x_quad->setMinimum(-1000);
        spin_escala_x_quad->setMaximum(1000);
        spin_escala_x_quad->setSingleStep(0.1);

        horizontalLayout_4->addWidget(spin_escala_x_quad);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_5 = new QLabel(groupBoxDim);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_y_quad->setObjectName(QStringLiteral("spin_escala_y_quad"));
        spin_escala_y_quad->setDecimals(2);
        spin_escala_y_quad->setMinimum(-1000);
        spin_escala_y_quad->setMaximum(1000);
        spin_escala_y_quad->setSingleStep(0.1);

        horizontalLayout_5->addWidget(spin_escala_y_quad);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_6 = new QLabel(groupBoxDim);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z_quad = new QDoubleSpinBox(groupBoxDim);
        spin_escala_z_quad->setObjectName(QStringLiteral("spin_escala_z_quad"));
        spin_escala_z_quad->setDecimals(2);
        spin_escala_z_quad->setMinimum(-50);
        spin_escala_z_quad->setMaximum(50);
        spin_escala_z_quad->setSingleStep(0.1);

        horizontalLayout_6->addWidget(spin_escala_z_quad);


        verticalLayout->addLayout(horizontalLayout_6);


        horizontalLayout_23->addLayout(verticalLayout);


        gridLayout_4->addWidget(groupBoxDim, 1, 1, 1, 1);

        groupBox_7 = new QGroupBox(tab);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        gridLayout_3 = new QGridLayout(groupBox_7);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        checkbox_fixa = new QCheckBox(groupBox_7);
        checkbox_fixa->setObjectName(QStringLiteral("checkbox_fixa"));

        gridLayout_3->addWidget(checkbox_fixa, 1, 1, 1, 1);

        checkbox_respeita_solo = new QCheckBox(groupBox_7);
        checkbox_respeita_solo->setObjectName(QStringLiteral("checkbox_respeita_solo"));

        gridLayout_3->addWidget(checkbox_respeita_solo, 0, 0, 1, 1);

        checkbox_colisao = new QCheckBox(groupBox_7);
        checkbox_colisao->setObjectName(QStringLiteral("checkbox_colisao"));

        gridLayout_3->addWidget(checkbox_colisao, 2, 1, 1, 1);

        checkbox_visibilidade = new QCheckBox(groupBox_7);
        checkbox_visibilidade->setObjectName(QStringLiteral("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy);

        gridLayout_3->addWidget(checkbox_visibilidade, 2, 2, 1, 1);

        checkbox_selecionavel = new QCheckBox(groupBox_7);
        checkbox_selecionavel->setObjectName(QStringLiteral("checkbox_selecionavel"));

        gridLayout_3->addWidget(checkbox_selecionavel, 1, 0, 1, 1);

        checkbox_afetado_por_efeitos = new QCheckBox(groupBox_7);
        checkbox_afetado_por_efeitos->setObjectName(QStringLiteral("checkbox_afetado_por_efeitos"));

        gridLayout_3->addWidget(checkbox_afetado_por_efeitos, 2, 0, 1, 1);

        checkbox_ignora_luz = new QCheckBox(groupBox_7);
        checkbox_ignora_luz->setObjectName(QStringLiteral("checkbox_ignora_luz"));

        gridLayout_3->addWidget(checkbox_ignora_luz, 0, 2, 1, 1);

        checkbox_dois_lados = new QCheckBox(groupBox_7);
        checkbox_dois_lados->setObjectName(QStringLiteral("checkbox_dois_lados"));

        gridLayout_3->addWidget(checkbox_dois_lados, 0, 1, 1, 1);

        checkbox_faz_sombra = new QCheckBox(groupBox_7);
        checkbox_faz_sombra->setObjectName(QStringLiteral("checkbox_faz_sombra"));

        gridLayout_3->addWidget(checkbox_faz_sombra, 1, 2, 1, 1);

        checkbox_especular = new QCheckBox(groupBox_7);
        checkbox_especular->setObjectName(QStringLiteral("checkbox_especular"));

        gridLayout_3->addWidget(checkbox_especular, 0, 3, 1, 1);


        gridLayout_4->addWidget(groupBox_7, 1, 0, 1, 1);

        groupBox_10 = new QGroupBox(tab);
        groupBox_10->setObjectName(QStringLiteral("groupBox_10"));
        horizontalLayout_11 = new QHBoxLayout(groupBox_10);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        lista_rotulos = new QPlainTextEdit(groupBox_10);
        lista_rotulos->setObjectName(QStringLiteral("lista_rotulos"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lista_rotulos->sizePolicy().hasHeightForWidth());
        lista_rotulos->setSizePolicy(sizePolicy1);

        horizontalLayout_11->addWidget(lista_rotulos);


        gridLayout_4->addWidget(groupBox_10, 5, 0, 1, 1);

        groupBox_8 = new QGroupBox(tab);
        groupBox_8->setObjectName(QStringLiteral("groupBox_8"));
        horizontalLayout_26 = new QHBoxLayout(groupBox_8);
        horizontalLayout_26->setObjectName(QStringLiteral("horizontalLayout_26"));
        horizontalLayout_24 = new QHBoxLayout();
        horizontalLayout_24->setObjectName(QStringLiteral("horizontalLayout_24"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        label_3 = new QLabel(groupBox_8);
        label_3->setObjectName(QStringLiteral("label_3"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy2);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(groupBox_8);
        checkbox_cor->setObjectName(QStringLiteral("checkbox_cor"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy3);

        horizontalLayout_7->addWidget(checkbox_cor);


        horizontalLayout_2->addLayout(horizontalLayout_7);

        botao_cor = new QPushButton(groupBox_8);
        botao_cor->setObjectName(QStringLiteral("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        label_9 = new QLabel(groupBox_8);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_9);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
        slider_alfa = new QSlider(groupBox_8);
        slider_alfa->setObjectName(QStringLiteral("slider_alfa"));
        sizePolicy3.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy3);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_12->addWidget(slider_alfa);


        horizontalLayout_9->addLayout(horizontalLayout_12);


        horizontalLayout_2->addLayout(horizontalLayout_9);


        horizontalLayout_24->addLayout(horizontalLayout_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_24->addItem(horizontalSpacer);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        label_4 = new QLabel(groupBox_8);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_8->addWidget(label_4);

        combo_textura = new QComboBox(groupBox_8);
        combo_textura->setObjectName(QStringLiteral("combo_textura"));

        horizontalLayout_8->addWidget(combo_textura);

        checkbox_ladrilho = new QCheckBox(groupBox_8);
        checkbox_ladrilho->setObjectName(QStringLiteral("checkbox_ladrilho"));
        sizePolicy3.setHeightForWidth(checkbox_ladrilho->sizePolicy().hasHeightForWidth());
        checkbox_ladrilho->setSizePolicy(sizePolicy3);

        horizontalLayout_8->addWidget(checkbox_ladrilho);

        checkbox_bump = new QCheckBox(groupBox_8);
        checkbox_bump->setObjectName(QStringLiteral("checkbox_bump"));

        horizontalLayout_8->addWidget(checkbox_bump);


        horizontalLayout_24->addLayout(horizontalLayout_8);


        horizontalLayout_26->addLayout(horizontalLayout_24);


        gridLayout_4->addWidget(groupBox_8, 2, 1, 1, 1);

        groupBox_6 = new QGroupBox(tab);
        groupBox_6->setObjectName(QStringLiteral("groupBox_6"));
        horizontalLayout_22 = new QHBoxLayout(groupBox_6);
        horizontalLayout_22->setObjectName(QStringLiteral("horizontalLayout_22"));
        groupBox_2 = new QGroupBox(groupBox_6);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        dial_rotacao = new QDial(groupBox_2);
        dial_rotacao->setObjectName(QStringLiteral("dial_rotacao"));
        dial_rotacao->setGeometry(QRect(10, 20, 71, 71));
        dial_rotacao->setMinimum(0);
        dial_rotacao->setMaximum(360);
        dial_rotacao->setValue(0);
        dial_rotacao->setSliderPosition(0);
        dial_rotacao->setOrientation(Qt::Horizontal);
        dial_rotacao->setInvertedAppearance(true);
        dial_rotacao->setInvertedControls(true);
        dial_rotacao->setWrapping(true);
        dial_rotacao->setNotchTarget(45);
        dial_rotacao->setNotchesVisible(true);
        spin_translacao_quad = new QDoubleSpinBox(groupBox_2);
        spin_translacao_quad->setObjectName(QStringLiteral("spin_translacao_quad"));
        spin_translacao_quad->setGeometry(QRect(80, 60, 51, 24));
        spin_translacao_quad->setDecimals(2);
        spin_translacao_quad->setMinimum(-100);
        spin_translacao_quad->setMaximum(100);
        spin_translacao_quad->setSingleStep(0.1);
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(80, 40, 111, 16));
        spin_rotacao = new QSpinBox(groupBox_2);
        spin_rotacao->setObjectName(QStringLiteral("spin_rotacao"));
        spin_rotacao->setGeometry(QRect(20, 100, 51, 24));
        spin_rotacao->setMinimum(-180);
        spin_rotacao->setMaximum(180);
        label_15 = new QLabel(groupBox_2);
        label_15->setObjectName(QStringLiteral("label_15"));
        label_15->setGeometry(QRect(140, 60, 41, 16));

        horizontalLayout_22->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(groupBox_6);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        dial_rotacao_y = new QDial(groupBox_3);
        dial_rotacao_y->setObjectName(QStringLiteral("dial_rotacao_y"));
        dial_rotacao_y->setGeometry(QRect(20, 20, 71, 71));
        dial_rotacao_y->setMinimum(0);
        dial_rotacao_y->setMaximum(360);
        dial_rotacao_y->setValue(0);
        dial_rotacao_y->setSliderPosition(0);
        dial_rotacao_y->setOrientation(Qt::Horizontal);
        dial_rotacao_y->setInvertedAppearance(true);
        dial_rotacao_y->setInvertedControls(true);
        dial_rotacao_y->setWrapping(true);
        dial_rotacao_y->setNotchTarget(45);
        dial_rotacao_y->setNotchesVisible(true);
        spin_rotacao_y = new QSpinBox(groupBox_3);
        spin_rotacao_y->setObjectName(QStringLiteral("spin_rotacao_y"));
        spin_rotacao_y->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_y->setMinimum(-180);
        spin_rotacao_y->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(groupBox_6);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        dial_rotacao_x = new QDial(groupBox_4);
        dial_rotacao_x->setObjectName(QStringLiteral("dial_rotacao_x"));
        dial_rotacao_x->setGeometry(QRect(20, 20, 71, 71));
        dial_rotacao_x->setMinimum(0);
        dial_rotacao_x->setMaximum(360);
        dial_rotacao_x->setValue(0);
        dial_rotacao_x->setSliderPosition(0);
        dial_rotacao_x->setOrientation(Qt::Horizontal);
        dial_rotacao_x->setInvertedAppearance(true);
        dial_rotacao_x->setInvertedControls(true);
        dial_rotacao_x->setWrapping(true);
        dial_rotacao_x->setNotchTarget(45);
        dial_rotacao_x->setNotchesVisible(true);
        spin_rotacao_x = new QSpinBox(groupBox_4);
        spin_rotacao_x->setObjectName(QStringLiteral("spin_rotacao_x"));
        spin_rotacao_x->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_x->setMinimum(-180);
        spin_rotacao_x->setMaximum(180);

        horizontalLayout_22->addWidget(groupBox_4);


        gridLayout_4->addWidget(groupBox_6, 5, 1, 1, 1);

        groupBox_9 = new QGroupBox(tab);
        groupBox_9->setObjectName(QStringLiteral("groupBox_9"));
        horizontalLayout_27 = new QHBoxLayout(groupBox_9);
        horizontalLayout_27->setObjectName(QStringLiteral("horizontalLayout_27"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        checkbox_luz = new QCheckBox(groupBox_9);
        checkbox_luz->setObjectName(QStringLiteral("checkbox_luz"));
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        label_18 = new QLabel(groupBox_9);
        label_18->setObjectName(QStringLiteral("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_18);

        spin_raio_quad = new QDoubleSpinBox(groupBox_9);
        spin_raio_quad->setObjectName(QStringLiteral("spin_raio_quad"));
        spin_raio_quad->setDecimals(1);
        spin_raio_quad->setSingleStep(1);

        horizontalLayout_3->addWidget(spin_raio_quad);

        label_31 = new QLabel(groupBox_9);
        label_31->setObjectName(QStringLiteral("label_31"));
        QSizePolicy sizePolicy4(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy4);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_31);

        botao_luz = new QPushButton(groupBox_9);
        botao_luz->setObjectName(QStringLiteral("botao_luz"));
        botao_luz->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(botao_luz);


        horizontalLayout_27->addLayout(horizontalLayout_3);


        gridLayout_4->addWidget(groupBox_9, 3, 1, 1, 1);

        groupBoxPv = new QGroupBox(tab);
        groupBoxPv->setObjectName(QStringLiteral("groupBoxPv"));
        horizontalLayout_13 = new QHBoxLayout(groupBoxPv);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        label_8 = new QLabel(groupBoxPv);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(groupBoxPv);
        spin_pontos_vida->setObjectName(QStringLiteral("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(groupBoxPv);
        label_10->setObjectName(QStringLiteral("label_10"));
        sizePolicy2.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy2);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(groupBoxPv);
        spin_max_pontos_vida->setObjectName(QStringLiteral("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);


        gridLayout_4->addWidget(groupBoxPv, 2, 0, 1, 1);

        groupBox_5 = new QGroupBox(tab);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        gridLayout = new QGridLayout(groupBox_5);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QStringLiteral("horizontalLayout_19"));
        combo_transicao = new QComboBox(groupBox_5);
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->setObjectName(QStringLiteral("combo_transicao"));

        horizontalLayout_19->addWidget(combo_transicao);

        combo_id_cenario = new QComboBox(groupBox_5);
        combo_id_cenario->setObjectName(QStringLiteral("combo_id_cenario"));

        horizontalLayout_19->addWidget(combo_id_cenario);

        checkbox_transicao_posicao = new QCheckBox(groupBox_5);
        checkbox_transicao_posicao->setObjectName(QStringLiteral("checkbox_transicao_posicao"));

        horizontalLayout_19->addWidget(checkbox_transicao_posicao);

        botao_transicao_mapa = new QPushButton(groupBox_5);
        botao_transicao_mapa->setObjectName(QStringLiteral("botao_transicao_mapa"));
        sizePolicy3.setHeightForWidth(botao_transicao_mapa->sizePolicy().hasHeightForWidth());
        botao_transicao_mapa->setSizePolicy(sizePolicy3);

        horizontalLayout_19->addWidget(botao_transicao_mapa);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QStringLiteral("label_12"));
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy5);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_12);

        spin_trans_x = new QDoubleSpinBox(groupBox_5);
        spin_trans_x->setObjectName(QStringLiteral("spin_trans_x"));
        spin_trans_x->setDecimals(1);
        spin_trans_x->setMinimum(-1000);
        spin_trans_x->setMaximum(1000);
        spin_trans_x->setSingleStep(0.5);

        horizontalLayout_15->addWidget(spin_trans_x);


        horizontalLayout_14->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QStringLiteral("horizontalLayout_16"));
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_16->addWidget(label_13);

        spin_trans_y = new QDoubleSpinBox(groupBox_5);
        spin_trans_y->setObjectName(QStringLiteral("spin_trans_y"));
        spin_trans_y->setDecimals(1);
        spin_trans_y->setMinimum(-1000);
        spin_trans_y->setMaximum(1000);
        spin_trans_y->setSingleStep(0.5);

        horizontalLayout_16->addWidget(spin_trans_y);


        horizontalLayout_14->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QStringLiteral("horizontalLayout_17"));
        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_17->addWidget(label_14);

        spin_trans_z = new QDoubleSpinBox(groupBox_5);
        spin_trans_z->setObjectName(QStringLiteral("spin_trans_z"));
        spin_trans_z->setDecimals(1);
        spin_trans_z->setMinimum(-1000);
        spin_trans_z->setMaximum(1000);
        spin_trans_z->setSingleStep(0.5);

        horizontalLayout_17->addWidget(spin_trans_z);


        horizontalLayout_14->addLayout(horizontalLayout_17);


        verticalLayout_2->addLayout(horizontalLayout_14);


        gridLayout->addLayout(verticalLayout_2, 0, 0, 1, 1);


        gridLayout_4->addWidget(groupBox_5, 3, 0, 1, 1);

        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        gridLayout_5 = new QGridLayout(tab_2);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        groupBox_13 = new QGroupBox(tab_2);
        groupBox_13->setObjectName(QStringLiteral("groupBox_13"));
        horizontalLayout_58 = new QHBoxLayout(groupBox_13);
        horizontalLayout_58->setObjectName(QStringLiteral("horizontalLayout_58"));
        lista_amuletos = new QListWidget(groupBox_13);
        lista_amuletos->setObjectName(QStringLiteral("lista_amuletos"));

        horizontalLayout_58->addWidget(lista_amuletos);

        verticalLayout_19 = new QVBoxLayout();
        verticalLayout_19->setObjectName(QStringLiteral("verticalLayout_19"));
        botao_usar_amuleto = new QPushButton(groupBox_13);
        botao_usar_amuleto->setObjectName(QStringLiteral("botao_usar_amuleto"));
        botao_usar_amuleto->setEnabled(false);

        verticalLayout_19->addWidget(botao_usar_amuleto);

        botao_adicionar_amuleto = new QPushButton(groupBox_13);
        botao_adicionar_amuleto->setObjectName(QStringLiteral("botao_adicionar_amuleto"));

        verticalLayout_19->addWidget(botao_adicionar_amuleto);

        botao_remover_amuleto = new QPushButton(groupBox_13);
        botao_remover_amuleto->setObjectName(QStringLiteral("botao_remover_amuleto"));

        verticalLayout_19->addWidget(botao_remover_amuleto);


        horizontalLayout_58->addLayout(verticalLayout_19);


        gridLayout_5->addWidget(groupBox_13, 3, 1, 1, 1);

        groupBox_20 = new QGroupBox(tab_2);
        groupBox_20->setObjectName(QStringLiteral("groupBox_20"));
        horizontalLayout_65 = new QHBoxLayout(groupBox_20);
        horizontalLayout_65->setObjectName(QStringLiteral("horizontalLayout_65"));
        lista_mantos = new QListWidget(groupBox_20);
        lista_mantos->setObjectName(QStringLiteral("lista_mantos"));

        horizontalLayout_65->addWidget(lista_mantos);

        verticalLayout_9 = new QVBoxLayout();
        verticalLayout_9->setObjectName(QStringLiteral("verticalLayout_9"));
        botao_usar_manto = new QPushButton(groupBox_20);
        botao_usar_manto->setObjectName(QStringLiteral("botao_usar_manto"));
        botao_usar_manto->setEnabled(false);
        sizePolicy.setHeightForWidth(botao_usar_manto->sizePolicy().hasHeightForWidth());
        botao_usar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_usar_manto);

        botao_adicionar_manto = new QPushButton(groupBox_20);
        botao_adicionar_manto->setObjectName(QStringLiteral("botao_adicionar_manto"));
        sizePolicy.setHeightForWidth(botao_adicionar_manto->sizePolicy().hasHeightForWidth());
        botao_adicionar_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_adicionar_manto);

        botao_remover_manto = new QPushButton(groupBox_20);
        botao_remover_manto->setObjectName(QStringLiteral("botao_remover_manto"));
        sizePolicy.setHeightForWidth(botao_remover_manto->sizePolicy().hasHeightForWidth());
        botao_remover_manto->setSizePolicy(sizePolicy);

        verticalLayout_9->addWidget(botao_remover_manto);


        horizontalLayout_65->addLayout(verticalLayout_9);


        gridLayout_5->addWidget(groupBox_20, 1, 3, 1, 1);

        groupBox_15 = new QGroupBox(tab_2);
        groupBox_15->setObjectName(QStringLiteral("groupBox_15"));
        horizontalLayout_60 = new QHBoxLayout(groupBox_15);
        horizontalLayout_60->setObjectName(QStringLiteral("horizontalLayout_60"));
        lista_pergaminhos_arcanos = new QListWidget(groupBox_15);
        lista_pergaminhos_arcanos->setObjectName(QStringLiteral("lista_pergaminhos_arcanos"));

        horizontalLayout_60->addWidget(lista_pergaminhos_arcanos);

        verticalLayout_23 = new QVBoxLayout();
        verticalLayout_23->setObjectName(QStringLiteral("verticalLayout_23"));
        botao_adicionar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_adicionar_pergaminho_arcano->setObjectName(QStringLiteral("botao_adicionar_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_adicionar_pergaminho_arcano);

        botao_duplicar_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_duplicar_pergaminho_arcano->setObjectName(QStringLiteral("botao_duplicar_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_duplicar_pergaminho_arcano);

        botao_remover_pergaminho_arcano = new QPushButton(groupBox_15);
        botao_remover_pergaminho_arcano->setObjectName(QStringLiteral("botao_remover_pergaminho_arcano"));

        verticalLayout_23->addWidget(botao_remover_pergaminho_arcano);

        botao_ordenar_pergaminhos_arcanos = new QPushButton(groupBox_15);
        botao_ordenar_pergaminhos_arcanos->setObjectName(QStringLiteral("botao_ordenar_pergaminhos_arcanos"));

        verticalLayout_23->addWidget(botao_ordenar_pergaminhos_arcanos);


        horizontalLayout_60->addLayout(verticalLayout_23);


        gridLayout_5->addWidget(groupBox_15, 1, 0, 1, 1);

        groupBox_18 = new QGroupBox(tab_2);
        groupBox_18->setObjectName(QStringLiteral("groupBox_18"));
        horizontalLayout_63 = new QHBoxLayout(groupBox_18);
        horizontalLayout_63->setObjectName(QStringLiteral("horizontalLayout_63"));
        lista_luvas = new QListWidget(groupBox_18);
        lista_luvas->setObjectName(QStringLiteral("lista_luvas"));

        horizontalLayout_63->addWidget(lista_luvas);

        verticalLayout_8 = new QVBoxLayout();
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        botao_usar_luvas = new QPushButton(groupBox_18);
        botao_usar_luvas->setObjectName(QStringLiteral("botao_usar_luvas"));
        botao_usar_luvas->setEnabled(false);

        verticalLayout_8->addWidget(botao_usar_luvas);

        botao_adicionar_luvas = new QPushButton(groupBox_18);
        botao_adicionar_luvas->setObjectName(QStringLiteral("botao_adicionar_luvas"));

        verticalLayout_8->addWidget(botao_adicionar_luvas);

        botao_remover_luvas = new QPushButton(groupBox_18);
        botao_remover_luvas->setObjectName(QStringLiteral("botao_remover_luvas"));

        verticalLayout_8->addWidget(botao_remover_luvas);


        horizontalLayout_63->addLayout(verticalLayout_8);


        gridLayout_5->addWidget(groupBox_18, 3, 2, 1, 1);

        groupBox_11 = new QGroupBox(tab_2);
        groupBox_11->setObjectName(QStringLiteral("groupBox_11"));
        verticalLayout_21 = new QVBoxLayout(groupBox_11);
        verticalLayout_21->setObjectName(QStringLiteral("verticalLayout_21"));
        horizontalLayout_53 = new QHBoxLayout();
        horizontalLayout_53->setObjectName(QStringLiteral("horizontalLayout_53"));
        label_107 = new QLabel(groupBox_11);
        label_107->setObjectName(QStringLiteral("label_107"));
        label_107->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_53->addWidget(label_107);

        spin_po = new QSpinBox(groupBox_11);
        spin_po->setObjectName(QStringLiteral("spin_po"));

        horizontalLayout_53->addWidget(spin_po);


        verticalLayout_21->addLayout(horizontalLayout_53);

        horizontalLayout_54 = new QHBoxLayout();
        horizontalLayout_54->setObjectName(QStringLiteral("horizontalLayout_54"));
        label_108 = new QLabel(groupBox_11);
        label_108->setObjectName(QStringLiteral("label_108"));
        label_108->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_54->addWidget(label_108);

        spin_pp = new QSpinBox(groupBox_11);
        spin_pp->setObjectName(QStringLiteral("spin_pp"));

        horizontalLayout_54->addWidget(spin_pp);


        verticalLayout_21->addLayout(horizontalLayout_54);

        horizontalLayout_56 = new QHBoxLayout();
        horizontalLayout_56->setObjectName(QStringLiteral("horizontalLayout_56"));
        label_109 = new QLabel(groupBox_11);
        label_109->setObjectName(QStringLiteral("label_109"));
        label_109->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_56->addWidget(label_109);

        spin_pc = new QSpinBox(groupBox_11);
        spin_pc->setObjectName(QStringLiteral("spin_pc"));

        horizontalLayout_56->addWidget(spin_pc);


        verticalLayout_21->addLayout(horizontalLayout_56);

        horizontalLayout_57 = new QHBoxLayout();
        horizontalLayout_57->setObjectName(QStringLiteral("horizontalLayout_57"));
        label_110 = new QLabel(groupBox_11);
        label_110->setObjectName(QStringLiteral("label_110"));
        label_110->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_57->addWidget(label_110);

        spin_pl = new QSpinBox(groupBox_11);
        spin_pl->setObjectName(QStringLiteral("spin_pl"));

        horizontalLayout_57->addWidget(spin_pl);


        verticalLayout_21->addLayout(horizontalLayout_57);

        horizontalLayout_55 = new QHBoxLayout();
        horizontalLayout_55->setObjectName(QStringLiteral("horizontalLayout_55"));
        labelpe = new QLabel(groupBox_11);
        labelpe->setObjectName(QStringLiteral("labelpe"));
        labelpe->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_55->addWidget(labelpe);

        spin_pe = new QSpinBox(groupBox_11);
        spin_pe->setObjectName(QStringLiteral("spin_pe"));

        horizontalLayout_55->addWidget(spin_pe);


        verticalLayout_21->addLayout(horizontalLayout_55);


        gridLayout_5->addWidget(groupBox_11, 4, 0, 1, 1);

        groupBox_12 = new QGroupBox(tab_2);
        groupBox_12->setObjectName(QStringLiteral("groupBox_12"));
        horizontalLayout_44 = new QHBoxLayout(groupBox_12);
        horizontalLayout_44->setObjectName(QStringLiteral("horizontalLayout_44"));
        lista_botas = new QListWidget(groupBox_12);
        lista_botas->setObjectName(QStringLiteral("lista_botas"));

        horizontalLayout_44->addWidget(lista_botas);

        verticalLayout_22 = new QVBoxLayout();
        verticalLayout_22->setObjectName(QStringLiteral("verticalLayout_22"));
        botao_usar_botas = new QPushButton(groupBox_12);
        botao_usar_botas->setObjectName(QStringLiteral("botao_usar_botas"));
        botao_usar_botas->setEnabled(false);

        verticalLayout_22->addWidget(botao_usar_botas);

        botao_adicionar_botas = new QPushButton(groupBox_12);
        botao_adicionar_botas->setObjectName(QStringLiteral("botao_adicionar_botas"));

        verticalLayout_22->addWidget(botao_adicionar_botas);

        botao_remover_botas = new QPushButton(groupBox_12);
        botao_remover_botas->setObjectName(QStringLiteral("botao_remover_botas"));

        verticalLayout_22->addWidget(botao_remover_botas);


        horizontalLayout_44->addLayout(verticalLayout_22);


        gridLayout_5->addWidget(groupBox_12, 4, 1, 1, 1);

        groupBox_22 = new QGroupBox(tab_2);
        groupBox_22->setObjectName(QStringLiteral("groupBox_22"));
        horizontalLayout_67 = new QHBoxLayout(groupBox_22);
        horizontalLayout_67->setObjectName(QStringLiteral("horizontalLayout_67"));
        lista_bracadeiras = new QListWidget(groupBox_22);
        lista_bracadeiras->setObjectName(QStringLiteral("lista_bracadeiras"));

        horizontalLayout_67->addWidget(lista_bracadeiras);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        botao_usar_bracadeiras = new QPushButton(groupBox_22);
        botao_usar_bracadeiras->setObjectName(QStringLiteral("botao_usar_bracadeiras"));

        verticalLayout_7->addWidget(botao_usar_bracadeiras);

        botao_adicionar_bracadeiras = new QPushButton(groupBox_22);
        botao_adicionar_bracadeiras->setObjectName(QStringLiteral("botao_adicionar_bracadeiras"));

        verticalLayout_7->addWidget(botao_adicionar_bracadeiras);

        botao_remover_bracadeiras = new QPushButton(groupBox_22);
        botao_remover_bracadeiras->setObjectName(QStringLiteral("botao_remover_bracadeiras"));

        verticalLayout_7->addWidget(botao_remover_bracadeiras);


        horizontalLayout_67->addLayout(verticalLayout_7);


        gridLayout_5->addWidget(groupBox_22, 4, 2, 1, 1);

        groupBox_17 = new QGroupBox(tab_2);
        groupBox_17->setObjectName(QStringLiteral("groupBox_17"));
        horizontalLayout_62 = new QHBoxLayout(groupBox_17);
        horizontalLayout_62->setObjectName(QStringLiteral("horizontalLayout_62"));
        lista_aneis = new QListWidget(groupBox_17);
        lista_aneis->setObjectName(QStringLiteral("lista_aneis"));

        horizontalLayout_62->addWidget(lista_aneis);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        botao_usar_anel = new QPushButton(groupBox_17);
        botao_usar_anel->setObjectName(QStringLiteral("botao_usar_anel"));
        botao_usar_anel->setEnabled(false);

        verticalLayout_10->addWidget(botao_usar_anel);

        botao_adicionar_anel = new QPushButton(groupBox_17);
        botao_adicionar_anel->setObjectName(QStringLiteral("botao_adicionar_anel"));

        verticalLayout_10->addWidget(botao_adicionar_anel);

        botao_remover_anel = new QPushButton(groupBox_17);
        botao_remover_anel->setObjectName(QStringLiteral("botao_remover_anel"));

        verticalLayout_10->addWidget(botao_remover_anel);


        horizontalLayout_62->addLayout(verticalLayout_10);


        gridLayout_5->addWidget(groupBox_17, 1, 2, 1, 1);

        groupBox_14 = new QGroupBox(tab_2);
        groupBox_14->setObjectName(QStringLiteral("groupBox_14"));
        horizontalLayout_59 = new QHBoxLayout(groupBox_14);
        horizontalLayout_59->setObjectName(QStringLiteral("horizontalLayout_59"));
        lista_pocoes = new QListWidget(groupBox_14);
        lista_pocoes->setObjectName(QStringLiteral("lista_pocoes"));

        horizontalLayout_59->addWidget(lista_pocoes);

        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        botao_adicionar_pocao = new QPushButton(groupBox_14);
        botao_adicionar_pocao->setObjectName(QStringLiteral("botao_adicionar_pocao"));
        sizePolicy.setHeightForWidth(botao_adicionar_pocao->sizePolicy().hasHeightForWidth());
        botao_adicionar_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_adicionar_pocao);

        botao_duplicar_pocao = new QPushButton(groupBox_14);
        botao_duplicar_pocao->setObjectName(QStringLiteral("botao_duplicar_pocao"));

        verticalLayout_6->addWidget(botao_duplicar_pocao);

        botao_remover_pocao = new QPushButton(groupBox_14);
        botao_remover_pocao->setObjectName(QStringLiteral("botao_remover_pocao"));
        sizePolicy.setHeightForWidth(botao_remover_pocao->sizePolicy().hasHeightForWidth());
        botao_remover_pocao->setSizePolicy(sizePolicy);

        verticalLayout_6->addWidget(botao_remover_pocao);

        botao_ordenar_pocoes = new QPushButton(groupBox_14);
        botao_ordenar_pocoes->setObjectName(QStringLiteral("botao_ordenar_pocoes"));

        verticalLayout_6->addWidget(botao_ordenar_pocoes);


        horizontalLayout_59->addLayout(verticalLayout_6);


        gridLayout_5->addWidget(groupBox_14, 1, 1, 1, 1);

        groupBox_16 = new QGroupBox(tab_2);
        groupBox_16->setObjectName(QStringLiteral("groupBox_16"));
        horizontalLayout_61 = new QHBoxLayout(groupBox_16);
        horizontalLayout_61->setObjectName(QStringLiteral("horizontalLayout_61"));
        lista_pergaminhos_divinos = new QListWidget(groupBox_16);
        lista_pergaminhos_divinos->setObjectName(QStringLiteral("lista_pergaminhos_divinos"));

        horizontalLayout_61->addWidget(lista_pergaminhos_divinos);

        verticalLayout_24 = new QVBoxLayout();
        verticalLayout_24->setObjectName(QStringLiteral("verticalLayout_24"));
        botao_adicionar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_adicionar_pergaminho_divino->setObjectName(QStringLiteral("botao_adicionar_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_adicionar_pergaminho_divino);

        botao_duplicar_pergaminho_divino = new QPushButton(groupBox_16);
        botao_duplicar_pergaminho_divino->setObjectName(QStringLiteral("botao_duplicar_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_duplicar_pergaminho_divino);

        botao_remover_pergaminho_divino = new QPushButton(groupBox_16);
        botao_remover_pergaminho_divino->setObjectName(QStringLiteral("botao_remover_pergaminho_divino"));

        verticalLayout_24->addWidget(botao_remover_pergaminho_divino);

        botao_ordenar_pergaminhos_divinos = new QPushButton(groupBox_16);
        botao_ordenar_pergaminhos_divinos->setObjectName(QStringLiteral("botao_ordenar_pergaminhos_divinos"));

        verticalLayout_24->addWidget(botao_ordenar_pergaminhos_divinos);


        horizontalLayout_61->addLayout(verticalLayout_24);


        gridLayout_5->addWidget(groupBox_16, 3, 0, 1, 1);

        groupBox_21 = new QGroupBox(tab_2);
        groupBox_21->setObjectName(QStringLiteral("groupBox_21"));
        horizontalLayout_66 = new QHBoxLayout(groupBox_21);
        horizontalLayout_66->setObjectName(QStringLiteral("horizontalLayout_66"));
        lista_tesouro = new QPlainTextEdit(groupBox_21);
        lista_tesouro->setObjectName(QStringLiteral("lista_tesouro"));

        horizontalLayout_66->addWidget(lista_tesouro);


        gridLayout_5->addWidget(groupBox_21, 4, 3, 1, 1);

        groupBox_19 = new QGroupBox(tab_2);
        groupBox_19->setObjectName(QStringLiteral("groupBox_19"));
        horizontalLayout_64 = new QHBoxLayout(groupBox_19);
        horizontalLayout_64->setObjectName(QStringLiteral("horizontalLayout_64"));
        lista_chapeus = new QListWidget(groupBox_19);
        lista_chapeus->setObjectName(QStringLiteral("lista_chapeus"));

        horizontalLayout_64->addWidget(lista_chapeus);

        verticalLayout_20 = new QVBoxLayout();
        verticalLayout_20->setObjectName(QStringLiteral("verticalLayout_20"));
        botao_vestir_chapeu = new QPushButton(groupBox_19);
        botao_vestir_chapeu->setObjectName(QStringLiteral("botao_vestir_chapeu"));
        botao_vestir_chapeu->setEnabled(false);

        verticalLayout_20->addWidget(botao_vestir_chapeu);

        botao_adicionar_chapeu = new QPushButton(groupBox_19);
        botao_adicionar_chapeu->setObjectName(QStringLiteral("botao_adicionar_chapeu"));

        verticalLayout_20->addWidget(botao_adicionar_chapeu);

        botao_remover_chapeu = new QPushButton(groupBox_19);
        botao_remover_chapeu->setObjectName(QStringLiteral("botao_remover_chapeu"));

        verticalLayout_20->addWidget(botao_remover_chapeu);


        horizontalLayout_64->addLayout(verticalLayout_20);


        gridLayout_5->addWidget(groupBox_19, 3, 3, 1, 1);

        tabWidget->addTab(tab_2, QString());

        gridLayout_2->addWidget(tabWidget, 0, 0, 1, 3);

        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, dial_rotacao);
        QWidget::setTabOrder(dial_rotacao, spin_rotacao);
        QWidget::setTabOrder(spin_rotacao, spin_translacao_quad);
        QWidget::setTabOrder(spin_translacao_quad, dial_rotacao_y);
        QWidget::setTabOrder(dial_rotacao_y, spin_rotacao_y);
        QWidget::setTabOrder(spin_rotacao_y, dial_rotacao_x);
        QWidget::setTabOrder(dial_rotacao_x, spin_rotacao_x);
        QWidget::setTabOrder(spin_rotacao_x, spin_escala_x_quad);
        QWidget::setTabOrder(spin_escala_x_quad, spin_escala_y_quad);
        QWidget::setTabOrder(spin_escala_y_quad, spin_escala_z_quad);
        QWidget::setTabOrder(spin_escala_z_quad, combo_textura);
        QWidget::setTabOrder(combo_textura, checkbox_ladrilho);
        QWidget::setTabOrder(checkbox_ladrilho, checkbox_transicao_posicao);
        QWidget::setTabOrder(checkbox_transicao_posicao, botao_transicao_mapa);
        QWidget::setTabOrder(botao_transicao_mapa, spin_trans_x);
        QWidget::setTabOrder(spin_trans_x, spin_trans_y);
        QWidget::setTabOrder(spin_trans_y, spin_trans_z);
        QWidget::setTabOrder(spin_trans_z, checkbox_colisao);
        QWidget::setTabOrder(checkbox_colisao, checkbox_cor);
        QWidget::setTabOrder(checkbox_cor, botao_cor);
        QWidget::setTabOrder(botao_cor, slider_alfa);
        QWidget::setTabOrder(slider_alfa, checkbox_luz);
        QWidget::setTabOrder(checkbox_luz, botao_luz);
        QWidget::setTabOrder(botao_luz, campo_id);

        retranslateUi(ifg__qt__DialogoForma);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoForma, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoForma, SLOT(accept()));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ifg__qt__DialogoForma);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoForma)
    {
        ifg__qt__DialogoForma->setWindowTitle(QApplication::translate("ifg::qt::DialogoForma", "Dialog", nullptr));
        label->setText(QApplication::translate("ifg::qt::DialogoForma", "Id", nullptr));
        label_17->setText(QApplication::translate("ifg::qt::DialogoForma", "Tipo", nullptr));
        combo_tipo_forma->setItemText(0, QApplication::translate("ifg::qt::DialogoForma", "Cilindro", nullptr));
        combo_tipo_forma->setItemText(1, QApplication::translate("ifg::qt::DialogoForma", "Cone", nullptr));
        combo_tipo_forma->setItemText(2, QApplication::translate("ifg::qt::DialogoForma", "Cubo", nullptr));
        combo_tipo_forma->setItemText(3, QApplication::translate("ifg::qt::DialogoForma", "Esfera", nullptr));
        combo_tipo_forma->setItemText(4, QApplication::translate("ifg::qt::DialogoForma", "Pir\303\242mide", nullptr));
        combo_tipo_forma->setItemText(5, QApplication::translate("ifg::qt::DialogoForma", "Hemisf\303\251rio", nullptr));

        groupBoxDim->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Dimens\303\265es (quadrados)", nullptr));
        label_7->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam X", nullptr));
        label_5->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam Y", nullptr));
        label_6->setText(QApplication::translate("ifg::qt::DialogoForma", "Altura", nullptr));
        groupBox_7->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Atributos", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_fixa->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade ser\303\241 fixa (implicando que n\303\243o ser\303\241 selecionada com um clique nem sele\303\247\303\243o de \303\241rea)", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_fixa->setText(QApplication::translate("ifg::qt::DialogoForma", "Fixa", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_respeita_solo->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade n\303\243o afundar\303\241 no solo.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_respeita_solo->setText(QApplication::translate("ifg::qt::DialogoForma", "Respeita Solo", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_colisao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, n\303\243o ser\303\241 poss\303\255vel colidor com a entidade.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_colisao->setText(QApplication::translate("ifg::qt::DialogoForma", "Colis\303\243o", nullptr));
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoForma", "Vis\303\255vel", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_selecionavel->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade poder\303\241 ser selecionada por jogadores.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoForma", "Selecion\303\241vel para jogadores", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_afetado_por_efeitos->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Marque se objeto pode ser afetado por efeitos.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_afetado_por_efeitos->setText(QApplication::translate("ifg::qt::DialogoForma", "Afetado por Efeitos", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_ignora_luz->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade ser\303\241 desenhada com ilumina\303\247\303\243o total.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_ignora_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Ignora Luz", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_dois_lados->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nos dois lados da primitiva ser\303\243o desenhados.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_dois_lados->setText(QApplication::translate("ifg::qt::DialogoForma", "Dois Lados", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_faz_sombra->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, a entidade n\303\243o far\303\241 sombra.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_faz_sombra->setText(QApplication::translate("ifg::qt::DialogoForma", "Faz Sombra", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_especular->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, objeto brilhar\303\241 como metal", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_especular->setText(QApplication::translate("ifg::qt::DialogoForma", "Especular", nullptr));
        groupBox_10->setTitle(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos Especiais", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_8->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Cor e Textura", nullptr));
        label_3->setText(QApplication::translate("ifg::qt::DialogoForma", "Cor:", nullptr));
        checkbox_cor->setText(QString());
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor", nullptr));
        label_9->setText(QApplication::translate("ifg::qt::DialogoForma", "Alfa", nullptr));
        label_4->setText(QApplication::translate("ifg::qt::DialogoForma", "Textura", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_ladrilho->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_ladrilho->setText(QApplication::translate("ifg::qt::DialogoForma", "ladrilho", nullptr));
        checkbox_bump->setText(QApplication::translate("ifg::qt::DialogoForma", "Bump", nullptr));
        groupBox_6->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Transforma\303\247\303\265es", nullptr));
        groupBox_2->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o e Transla\303\247\303\243o em Z", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o do objeto ao redor do eixo Z.", nullptr));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ifg::qt::DialogoForma", " Transla\303\247\303\243o", nullptr));
        label_15->setText(QApplication::translate("ifg::qt::DialogoForma", "quad", nullptr));
        groupBox_3->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em Y", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_y->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_4->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em X", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_x->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_9->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Ilumina\303\247\303\243o", nullptr));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Luz", nullptr));
        label_18->setText(QApplication::translate("ifg::qt::DialogoForma", "Raio", nullptr));
#ifndef QT_NO_TOOLTIP
        spin_raio_quad->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Raio da luz, em metros.", nullptr));
#endif // QT_NO_TOOLTIP
        label_31->setText(QApplication::translate("ifg::qt::DialogoForma", "quadrados", nullptr));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor da Luz", nullptr));
        groupBoxPv->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida", nullptr));
        label_8->setText(QApplication::translate("ifg::qt::DialogoForma", "Corrente", nullptr));
        label_10->setText(QApplication::translate("ifg::qt::DialogoForma", "Max", nullptr));
        groupBox_5->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Transi\303\247\303\243o de Cen\303\241rio", nullptr));
        combo_transicao->setItemText(0, QApplication::translate("ifg::qt::DialogoForma", "Sem Transi\303\247\303\243o", nullptr));
        combo_transicao->setItemText(1, QApplication::translate("ifg::qt::DialogoForma", "Cen\303\241rio", nullptr));
        combo_transicao->setItemText(2, QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));

        checkbox_transicao_posicao->setText(QApplication::translate("ifg::qt::DialogoForma", "Posi\303\247\303\243o?", nullptr));
        botao_transicao_mapa->setText(QApplication::translate("ifg::qt::DialogoForma", "Clicar", nullptr));
        label_12->setText(QApplication::translate("ifg::qt::DialogoForma", "X", nullptr));
        label_13->setText(QApplication::translate("ifg::qt::DialogoForma", "Y", nullptr));
        label_14->setText(QApplication::translate("ifg::qt::DialogoForma", "Z", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("ifg::qt::DialogoForma", "Geral", nullptr));
        groupBox_13->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Amuletos", nullptr));
        botao_usar_amuleto->setText(QString());
        botao_adicionar_amuleto->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_amuleto->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_20->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Mantos", nullptr));
#ifndef QT_NO_TOOLTIP
        botao_usar_manto->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_NO_TOOLTIP
        botao_usar_manto->setText(QString());
        botao_adicionar_manto->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_manto->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_15->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Arcanos", nullptr));
        botao_adicionar_pergaminho_arcano->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_arcano->setText(QApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_arcano->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_arcanos->setText(QApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        groupBox_18->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Luvas e Manoplas", nullptr));
#ifndef QT_NO_TOOLTIP
        botao_usar_luvas->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_NO_TOOLTIP
        botao_usar_luvas->setText(QString());
        botao_adicionar_luvas->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_luvas->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_11->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Moedas", nullptr));
        label_107->setText(QApplication::translate("ifg::qt::DialogoForma", "Ouro", nullptr));
        label_108->setText(QApplication::translate("ifg::qt::DialogoForma", "Prata", nullptr));
        label_109->setText(QApplication::translate("ifg::qt::DialogoForma", "Cobre", nullptr));
        label_110->setText(QApplication::translate("ifg::qt::DialogoForma", "Platina", nullptr));
        labelpe->setText(QApplication::translate("ifg::qt::DialogoForma", "Electrum", nullptr));
        groupBox_12->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Botas", nullptr));
        botao_usar_botas->setText(QString());
        botao_adicionar_botas->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_botas->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_22->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Bra\303\247adeiras", nullptr));
#ifndef QT_NO_TOOLTIP
        botao_usar_bracadeiras->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_NO_TOOLTIP
        botao_usar_bracadeiras->setText(QString());
        botao_adicionar_bracadeiras->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_bracadeiras->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_17->setTitle(QApplication::translate("ifg::qt::DialogoForma", "An\303\251is", nullptr));
#ifndef QT_NO_TOOLTIP
        botao_usar_anel->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Usar/retirar anel", nullptr));
#endif // QT_NO_TOOLTIP
        botao_usar_anel->setText(QString());
        botao_adicionar_anel->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_anel->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        groupBox_14->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Po\303\247\303\265es", nullptr));
        botao_adicionar_pocao->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pocao->setText(QApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pocao->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pocoes->setText(QApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        groupBox_16->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Pergaminhos Divinos", nullptr));
        botao_adicionar_pergaminho_divino->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_duplicar_pergaminho_divino->setText(QApplication::translate("ifg::qt::DialogoForma", "Duplicar", nullptr));
        botao_remover_pergaminho_divino->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        botao_ordenar_pergaminhos_divinos->setText(QApplication::translate("ifg::qt::DialogoForma", "Ordenar", nullptr));
        groupBox_21->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Outros", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_tesouro->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_19->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Chap\303\251us", nullptr));
        botao_vestir_chapeu->setText(QString());
        botao_adicionar_chapeu->setText(QApplication::translate("ifg::qt::DialogoForma", "+", nullptr));
        botao_remover_chapeu->setText(QApplication::translate("ifg::qt::DialogoForma", "-", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));
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
