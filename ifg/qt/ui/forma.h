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
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

namespace ifg {
namespace qt {

class Ui_DialogoForma
{
public:
    QGridLayout *gridLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *campo_id;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_17;
    QComboBox *combo_tipo_forma;
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
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_11;
    QPlainTextEdit *lista_rotulos;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_16;
    QPlainTextEdit *lista_tesouro;
    QGroupBox *groupBox;
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
    QGroupBox *groupBox_7;
    QHBoxLayout *horizontalLayout_25;
    QHBoxLayout *horizontalLayout_11;
    QCheckBox *checkbox_colisao;
    QCheckBox *checkbox_fixa;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_dois_lados;
    QCheckBox *checkbox_visibilidade;
    QCheckBox *checkbox_faz_sombra;
    QGroupBox *groupBox_9;
    QHBoxLayout *horizontalLayout_27;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_luz;
    QLabel *label_18;
    QDoubleSpinBox *spin_raio_quad;
    QLabel *label_31;
    QPushButton *botao_luz;
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
    QDialogButtonBox *botoes;
    QGroupBox *groupBox1;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_8;
    QSpinBox *spin_pontos_vida;
    QLabel *label_10;
    QSpinBox *spin_max_pontos_vida;

    void setupUi(QDialog *ifg__qt__DialogoForma)
    {
        if (ifg__qt__DialogoForma->objectName().isEmpty())
            ifg__qt__DialogoForma->setObjectName(QStringLiteral("ifg__qt__DialogoForma"));
        ifg__qt__DialogoForma->resize(1139, 739);
        gridLayout_2 = new QGridLayout(ifg__qt__DialogoForma);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(ifg__qt__DialogoForma);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(ifg__qt__DialogoForma);
        campo_id->setObjectName(QStringLiteral("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);


        gridLayout_2->addLayout(horizontalLayout, 0, 0, 1, 1);

        horizontalLayout_21 = new QHBoxLayout();
        horizontalLayout_21->setObjectName(QStringLiteral("horizontalLayout_21"));
        label_17 = new QLabel(ifg__qt__DialogoForma);
        label_17->setObjectName(QStringLiteral("label_17"));

        horizontalLayout_21->addWidget(label_17);

        combo_tipo_forma = new QComboBox(ifg__qt__DialogoForma);
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->setObjectName(QStringLiteral("combo_tipo_forma"));

        horizontalLayout_21->addWidget(combo_tipo_forma);


        gridLayout_2->addLayout(horizontalLayout_21, 0, 1, 1, 1);

        groupBox_8 = new QGroupBox(ifg__qt__DialogoForma);
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
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);

        checkbox_cor = new QCheckBox(groupBox_8);
        checkbox_cor->setObjectName(QStringLiteral("checkbox_cor"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(checkbox_cor->sizePolicy().hasHeightForWidth());
        checkbox_cor->setSizePolicy(sizePolicy1);

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
        sizePolicy1.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy1);
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
        sizePolicy1.setHeightForWidth(checkbox_ladrilho->sizePolicy().hasHeightForWidth());
        checkbox_ladrilho->setSizePolicy(sizePolicy1);

        horizontalLayout_8->addWidget(checkbox_ladrilho);


        horizontalLayout_24->addLayout(horizontalLayout_8);


        horizontalLayout_26->addLayout(horizontalLayout_24);


        gridLayout_2->addWidget(groupBox_8, 6, 0, 1, 2);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QStringLiteral("horizontalLayout_18"));
        label_11 = new QLabel(ifg__qt__DialogoForma);
        label_11->setObjectName(QStringLiteral("label_11"));
        sizePolicy1.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy1);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_11);

        lista_rotulos = new QPlainTextEdit(ifg__qt__DialogoForma);
        lista_rotulos->setObjectName(QStringLiteral("lista_rotulos"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lista_rotulos->sizePolicy().hasHeightForWidth());
        lista_rotulos->setSizePolicy(sizePolicy2);

        horizontalLayout_18->addWidget(lista_rotulos);


        gridLayout_2->addLayout(horizontalLayout_18, 2, 0, 1, 1);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QStringLiteral("horizontalLayout_20"));
        label_16 = new QLabel(ifg__qt__DialogoForma);
        label_16->setObjectName(QStringLiteral("label_16"));
        sizePolicy1.setHeightForWidth(label_16->sizePolicy().hasHeightForWidth());
        label_16->setSizePolicy(sizePolicy1);
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_16);

        lista_tesouro = new QPlainTextEdit(ifg__qt__DialogoForma);
        lista_tesouro->setObjectName(QStringLiteral("lista_tesouro"));
        sizePolicy2.setHeightForWidth(lista_tesouro->sizePolicy().hasHeightForWidth());
        lista_tesouro->setSizePolicy(sizePolicy2);

        horizontalLayout_20->addWidget(lista_tesouro);


        gridLayout_2->addLayout(horizontalLayout_20, 3, 0, 2, 1);

        groupBox = new QGroupBox(ifg__qt__DialogoForma);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout_23 = new QHBoxLayout(groupBox);
        horizontalLayout_23->setObjectName(QStringLiteral("horizontalLayout_23"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x_quad = new QDoubleSpinBox(groupBox);
        spin_escala_x_quad->setObjectName(QStringLiteral("spin_escala_x_quad"));
        spin_escala_x_quad->setDecimals(2);
        spin_escala_x_quad->setMinimum(-1000);
        spin_escala_x_quad->setMaximum(1000);
        spin_escala_x_quad->setSingleStep(0.1);

        horizontalLayout_4->addWidget(spin_escala_x_quad);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y_quad = new QDoubleSpinBox(groupBox);
        spin_escala_y_quad->setObjectName(QStringLiteral("spin_escala_y_quad"));
        spin_escala_y_quad->setDecimals(2);
        spin_escala_y_quad->setMinimum(-1000);
        spin_escala_y_quad->setMaximum(1000);
        spin_escala_y_quad->setSingleStep(0.1);

        horizontalLayout_5->addWidget(spin_escala_y_quad);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z_quad = new QDoubleSpinBox(groupBox);
        spin_escala_z_quad->setObjectName(QStringLiteral("spin_escala_z_quad"));
        spin_escala_z_quad->setDecimals(2);
        spin_escala_z_quad->setMinimum(-50);
        spin_escala_z_quad->setMaximum(50);
        spin_escala_z_quad->setSingleStep(0.1);

        horizontalLayout_6->addWidget(spin_escala_z_quad);


        verticalLayout->addLayout(horizontalLayout_6);


        horizontalLayout_23->addLayout(verticalLayout);


        gridLayout_2->addWidget(groupBox, 3, 1, 1, 1);

        groupBox_5 = new QGroupBox(ifg__qt__DialogoForma);
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
        sizePolicy1.setHeightForWidth(botao_transicao_mapa->sizePolicy().hasHeightForWidth());
        botao_transicao_mapa->setSizePolicy(sizePolicy1);

        horizontalLayout_19->addWidget(botao_transicao_mapa);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QStringLiteral("label_12"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy3);
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


        gridLayout_2->addWidget(groupBox_5, 3, 2, 1, 1);

        groupBox_7 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_7->setObjectName(QStringLiteral("groupBox_7"));
        horizontalLayout_25 = new QHBoxLayout(groupBox_7);
        horizontalLayout_25->setObjectName(QStringLiteral("horizontalLayout_25"));
        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        checkbox_colisao = new QCheckBox(groupBox_7);
        checkbox_colisao->setObjectName(QStringLiteral("checkbox_colisao"));

        horizontalLayout_11->addWidget(checkbox_colisao);

        checkbox_fixa = new QCheckBox(groupBox_7);
        checkbox_fixa->setObjectName(QStringLiteral("checkbox_fixa"));

        horizontalLayout_11->addWidget(checkbox_fixa);

        checkbox_selecionavel = new QCheckBox(groupBox_7);
        checkbox_selecionavel->setObjectName(QStringLiteral("checkbox_selecionavel"));

        horizontalLayout_11->addWidget(checkbox_selecionavel);

        checkbox_dois_lados = new QCheckBox(groupBox_7);
        checkbox_dois_lados->setObjectName(QStringLiteral("checkbox_dois_lados"));

        horizontalLayout_11->addWidget(checkbox_dois_lados);

        checkbox_visibilidade = new QCheckBox(groupBox_7);
        checkbox_visibilidade->setObjectName(QStringLiteral("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy4);

        horizontalLayout_11->addWidget(checkbox_visibilidade);

        checkbox_faz_sombra = new QCheckBox(groupBox_7);
        checkbox_faz_sombra->setObjectName(QStringLiteral("checkbox_faz_sombra"));

        horizontalLayout_11->addWidget(checkbox_faz_sombra);


        horizontalLayout_25->addLayout(horizontalLayout_11);


        gridLayout_2->addWidget(groupBox_7, 4, 1, 1, 2);

        groupBox_9 = new QGroupBox(ifg__qt__DialogoForma);
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
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_31->sizePolicy().hasHeightForWidth());
        label_31->setSizePolicy(sizePolicy5);
        label_31->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_31);

        botao_luz = new QPushButton(groupBox_9);
        botao_luz->setObjectName(QStringLiteral("botao_luz"));
        botao_luz->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(botao_luz);


        horizontalLayout_27->addLayout(horizontalLayout_3);


        gridLayout_2->addWidget(groupBox_9, 6, 2, 1, 1);

        groupBox_6 = new QGroupBox(ifg__qt__DialogoForma);
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


        gridLayout_2->addWidget(groupBox_6, 1, 1, 2, 2);

        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(botoes, 7, 2, 1, 1);

        groupBox1 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox1->setObjectName(QStringLiteral("groupBox1"));
        horizontalLayout_13 = new QHBoxLayout(groupBox1);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        label_8 = new QLabel(groupBox1);
        label_8->setObjectName(QStringLiteral("label_8"));
        sizePolicy.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(groupBox1);
        spin_pontos_vida->setObjectName(QStringLiteral("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(groupBox1);
        label_10->setObjectName(QStringLiteral("label_10"));
        sizePolicy.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(groupBox1);
        spin_max_pontos_vida->setObjectName(QStringLiteral("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);


        gridLayout_2->addWidget(groupBox1, 1, 0, 1, 1);

        QWidget::setTabOrder(spin_pontos_vida, spin_max_pontos_vida);
        QWidget::setTabOrder(spin_max_pontos_vida, lista_rotulos);
        QWidget::setTabOrder(lista_rotulos, dial_rotacao);
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
        QWidget::setTabOrder(spin_trans_z, checkbox_fixa);
        QWidget::setTabOrder(checkbox_fixa, checkbox_selecionavel);
        QWidget::setTabOrder(checkbox_selecionavel, checkbox_visibilidade);
        QWidget::setTabOrder(checkbox_visibilidade, checkbox_faz_sombra);
        QWidget::setTabOrder(checkbox_faz_sombra, checkbox_colisao);
        QWidget::setTabOrder(checkbox_colisao, checkbox_cor);
        QWidget::setTabOrder(checkbox_cor, botao_cor);
        QWidget::setTabOrder(botao_cor, slider_alfa);
        QWidget::setTabOrder(slider_alfa, checkbox_luz);
        QWidget::setTabOrder(checkbox_luz, botao_luz);
        QWidget::setTabOrder(botao_luz, campo_id);

        retranslateUi(ifg__qt__DialogoForma);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoForma, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoForma, SLOT(accept()));

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
        label_11->setText(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos Especial", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
        label_16->setText(QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_tesouro->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Dimens\303\265es (quadrados)", nullptr));
        label_7->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam X", nullptr));
        label_5->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam Y", nullptr));
        label_6->setText(QApplication::translate("ifg::qt::DialogoForma", "Altura", nullptr));
        groupBox_5->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Transi\303\247\303\243o de Cen\303\241rio", nullptr));
        combo_transicao->setItemText(0, QApplication::translate("ifg::qt::DialogoForma", "Sem Transi\303\247\303\243o", nullptr));
        combo_transicao->setItemText(1, QApplication::translate("ifg::qt::DialogoForma", "Cen\303\241rio", nullptr));
        combo_transicao->setItemText(2, QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));

        checkbox_transicao_posicao->setText(QApplication::translate("ifg::qt::DialogoForma", "Posi\303\247\303\243o?", nullptr));
        botao_transicao_mapa->setText(QApplication::translate("ifg::qt::DialogoForma", "Clicar", nullptr));
        label_12->setText(QApplication::translate("ifg::qt::DialogoForma", "X", nullptr));
        label_13->setText(QApplication::translate("ifg::qt::DialogoForma", "Y", nullptr));
        label_14->setText(QApplication::translate("ifg::qt::DialogoForma", "Z", nullptr));
        groupBox_7->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Atributos", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_colisao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_colisao->setText(QApplication::translate("ifg::qt::DialogoForma", "Colis\303\243o", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_fixa->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_fixa->setText(QApplication::translate("ifg::qt::DialogoForma", "Fixa", nullptr));
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoForma", "Selecion\303\241vel", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_dois_lados->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nos dois lados da primitiva ser\303\243o desenhados.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_dois_lados->setText(QApplication::translate("ifg::qt::DialogoForma", "Dois Lados", nullptr));
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoForma", "Vis\303\255vel", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_faz_sombra->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_faz_sombra->setText(QApplication::translate("ifg::qt::DialogoForma", "Faz Sombra", nullptr));
        groupBox_9->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Ilumina\303\247\303\243o", nullptr));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Luz", nullptr));
        label_18->setText(QApplication::translate("ifg::qt::DialogoForma", "Raio", nullptr));
#ifndef QT_NO_TOOLTIP
        spin_raio_quad->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Raio da luz, em metros.", nullptr));
#endif // QT_NO_TOOLTIP
        label_31->setText(QApplication::translate("ifg::qt::DialogoForma", "quadrados", nullptr));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor da Luz", nullptr));
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
        groupBox1->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida", nullptr));
        label_8->setText(QApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida:", nullptr));
        label_10->setText(QApplication::translate("ifg::qt::DialogoForma", "Max", nullptr));
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
