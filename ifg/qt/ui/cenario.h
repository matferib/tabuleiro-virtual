/********************************************************************************
** Form generated from reading UI file 'cenario.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CENARIO_H
#define CENARIO_H

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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

namespace ifg {
namespace qt {

class Ui_DialogoIluminacao
{
public:
    QGridLayout *gridLayout_2;
    QLabel *label_15;
    QLineEdit *campo_id;
    QLabel *label_16;
    QLineEdit *campo_descricao;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_17;
    QComboBox *combo_id_cenario;
    QPushButton *botao_clonar;
    QHBoxLayout *horizontalLayout_9;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_21;
    QComboBox *combo_herdar_iluminacao;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_10;
    QPushButton *botao_cor_ambiente;
    QLabel *label_18;
    QPushButton *botao_cor_direcional;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_3;
    QDial *dial_posicao;
    QLabel *label_9;
    QLabel *label_2;
    QDial *dial_inclinacao;
    QLabel *label_8;
    QLabel *label_5;
    QSpacerItem *horizontalSpacer;
    QLabel *label_4;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_22;
    QComboBox *combo_herdar_nevoa;
    QSpacerItem *horizontalSpacer_5;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_nevoa;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_12;
    QLineEdit *linha_nevoa_min;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_13;
    QLineEdit *linha_nevoa_max;
    QCheckBox *checkbox_cor_nevoa;
    QPushButton *botao_cor_nevoa;
    QSpacerItem *horizontalSpacer_6;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_20;
    QComboBox *combo_herdar_ceu;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_14;
    QComboBox *combo_ceu;
    QCheckBox *checkbox_luz_ceu;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_11;
    QLabel *label_19;
    QComboBox *combo_herdar_piso;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label;
    QComboBox *combo_fundo;
    QHBoxLayout *horizontalLayout_7;
    QCheckBox *checkbox_cor_piso;
    QPushButton *botao_cor_piso;
    QCheckBox *checkbox_ladrilho;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_11;
    QDoubleSpinBox *spin_escala_piso_x;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_23;
    QDoubleSpinBox *spin_escala_piso_y;
    QCheckBox *checkbox_mestre_apenas;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_13;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *linha_largura;
    QLabel *label_10;
    QLineEdit *linha_altura;
    QCheckBox *checkbox_tamanho_automatico;
    QCheckBox *checkbox_grade;
    QDialogButtonBox *botoes;

    void setupUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        if (ifg__qt__DialogoIluminacao->objectName().isEmpty())
            ifg__qt__DialogoIluminacao->setObjectName(QString::fromUtf8("ifg__qt__DialogoIluminacao"));
        ifg__qt__DialogoIluminacao->resize(929, 948);
        gridLayout_2 = new QGridLayout(ifg__qt__DialogoIluminacao);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_15 = new QLabel(ifg__qt__DialogoIluminacao);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout_2->addWidget(label_15, 0, 0, 1, 1);

        campo_id = new QLineEdit(ifg__qt__DialogoIluminacao);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        campo_id->setMaximumSize(QSize(50, 16777215));
        campo_id->setReadOnly(true);

        gridLayout_2->addWidget(campo_id, 0, 1, 1, 1);

        label_16 = new QLabel(ifg__qt__DialogoIluminacao);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        gridLayout_2->addWidget(label_16, 0, 2, 1, 1);

        campo_descricao = new QLineEdit(ifg__qt__DialogoIluminacao);
        campo_descricao->setObjectName(QString::fromUtf8("campo_descricao"));
        campo_descricao->setReadOnly(false);

        gridLayout_2->addWidget(campo_descricao, 0, 3, 1, 1);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_17 = new QLabel(ifg__qt__DialogoIluminacao);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_17->sizePolicy().hasHeightForWidth());
        label_17->setSizePolicy(sizePolicy);

        horizontalLayout_6->addWidget(label_17);

        combo_id_cenario = new QComboBox(ifg__qt__DialogoIluminacao);
        combo_id_cenario->setObjectName(QString::fromUtf8("combo_id_cenario"));

        horizontalLayout_6->addWidget(combo_id_cenario);

        botao_clonar = new QPushButton(ifg__qt__DialogoIluminacao);
        botao_clonar->setObjectName(QString::fromUtf8("botao_clonar"));

        horizontalLayout_6->addWidget(botao_clonar);


        gridLayout_2->addLayout(horizontalLayout_6, 1, 0, 1, 4);

        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        groupBox = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_21 = new QLabel(groupBox);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        sizePolicy.setHeightForWidth(label_21->sizePolicy().hasHeightForWidth());
        label_21->setSizePolicy(sizePolicy);

        horizontalLayout_14->addWidget(label_21);

        combo_herdar_iluminacao = new QComboBox(groupBox);
        combo_herdar_iluminacao->setObjectName(QString::fromUtf8("combo_herdar_iluminacao"));

        horizontalLayout_14->addWidget(combo_herdar_iluminacao);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_14->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_14);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        botao_cor_ambiente = new QPushButton(groupBox);
        botao_cor_ambiente->setObjectName(QString::fromUtf8("botao_cor_ambiente"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(botao_cor_ambiente->sizePolicy().hasHeightForWidth());
        botao_cor_ambiente->setSizePolicy(sizePolicy1);

        horizontalLayout_10->addWidget(botao_cor_ambiente);

        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_18->sizePolicy().hasHeightForWidth());
        label_18->setSizePolicy(sizePolicy2);
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_18);

        botao_cor_direcional = new QPushButton(groupBox);
        botao_cor_direcional->setObjectName(QString::fromUtf8("botao_cor_direcional"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(botao_cor_direcional->sizePolicy().hasHeightForWidth());
        botao_cor_direcional->setSizePolicy(sizePolicy3);

        horizontalLayout_10->addWidget(botao_cor_direcional);


        verticalLayout->addLayout(horizontalLayout_10);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy3.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy3);
        label_6->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_6, 1, 6, 1, 1);

        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        QSizePolicy sizePolicy4(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy4);

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy3);
        label_3->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_3, 2, 5, 1, 1);

        dial_posicao = new QDial(groupBox);
        dial_posicao->setObjectName(QString::fromUtf8("dial_posicao"));
        dial_posicao->setMinimum(0);
        dial_posicao->setMaximum(360);
        dial_posicao->setValue(0);
        dial_posicao->setSliderPosition(0);
        dial_posicao->setOrientation(Qt::Horizontal);
        dial_posicao->setInvertedAppearance(true);
        dial_posicao->setInvertedControls(true);
        dial_posicao->setWrapping(true);
        dial_posicao->setNotchTarget(45.000000000000000);
        dial_posicao->setNotchesVisible(true);

        gridLayout->addWidget(dial_posicao, 1, 5, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        sizePolicy4.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy4);
        label_9->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_9, 0, 1, 1, 1);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy4.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy4);

        gridLayout->addWidget(label_2, 1, 2, 1, 1);

        dial_inclinacao = new QDial(groupBox);
        dial_inclinacao->setObjectName(QString::fromUtf8("dial_inclinacao"));
        dial_inclinacao->setMinimum(0);
        dial_inclinacao->setMaximum(360);
        dial_inclinacao->setValue(135);
        dial_inclinacao->setSliderPosition(135);
        dial_inclinacao->setOrientation(Qt::Horizontal);
        dial_inclinacao->setInvertedAppearance(true);
        dial_inclinacao->setInvertedControls(true);
        dial_inclinacao->setWrapping(true);
        dial_inclinacao->setNotchTarget(30.000000000000000);
        dial_inclinacao->setNotchesVisible(true);

        gridLayout->addWidget(dial_inclinacao, 1, 1, 1, 1);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        sizePolicy4.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy4);
        label_8->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_8, 2, 1, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy3.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy3);
        label_5->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_5, 1, 4, 1, 1);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 1, 3, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy3.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy3);
        label_4->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_4, 0, 5, 1, 1);


        verticalLayout->addLayout(gridLayout);


        horizontalLayout_9->addWidget(groupBox);


        gridLayout_2->addLayout(horizontalLayout_9, 2, 0, 1, 4);

        groupBox_5 = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_4 = new QVBoxLayout(groupBox_5);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_22 = new QLabel(groupBox_5);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        sizePolicy.setHeightForWidth(label_22->sizePolicy().hasHeightForWidth());
        label_22->setSizePolicy(sizePolicy);

        horizontalLayout_15->addWidget(label_22);

        combo_herdar_nevoa = new QComboBox(groupBox_5);
        combo_herdar_nevoa->setObjectName(QString::fromUtf8("combo_herdar_nevoa"));

        horizontalLayout_15->addWidget(combo_herdar_nevoa);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer_5);


        verticalLayout_4->addLayout(horizontalLayout_15);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkbox_nevoa = new QCheckBox(groupBox_5);
        checkbox_nevoa->setObjectName(QString::fromUtf8("checkbox_nevoa"));
        checkbox_nevoa->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_3->addWidget(checkbox_nevoa);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        sizePolicy3.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy3);
        label_12->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_5->addWidget(label_12);

        linha_nevoa_min = new QLineEdit(groupBox_5);
        linha_nevoa_min->setObjectName(QString::fromUtf8("linha_nevoa_min"));
        linha_nevoa_min->setEnabled(false);
        QSizePolicy sizePolicy5(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(linha_nevoa_min->sizePolicy().hasHeightForWidth());
        linha_nevoa_min->setSizePolicy(sizePolicy5);
        linha_nevoa_min->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_5->addWidget(linha_nevoa_min);


        horizontalLayout_3->addLayout(horizontalLayout_5);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        sizePolicy3.setHeightForWidth(label_13->sizePolicy().hasHeightForWidth());
        label_13->setSizePolicy(sizePolicy3);
        label_13->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_17->addWidget(label_13);

        linha_nevoa_max = new QLineEdit(groupBox_5);
        linha_nevoa_max->setObjectName(QString::fromUtf8("linha_nevoa_max"));
        linha_nevoa_max->setEnabled(false);
        sizePolicy5.setHeightForWidth(linha_nevoa_max->sizePolicy().hasHeightForWidth());
        linha_nevoa_max->setSizePolicy(sizePolicy5);
        linha_nevoa_max->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_17->addWidget(linha_nevoa_max);


        horizontalLayout_3->addLayout(horizontalLayout_17);

        checkbox_cor_nevoa = new QCheckBox(groupBox_5);
        checkbox_cor_nevoa->setObjectName(QString::fromUtf8("checkbox_cor_nevoa"));

        horizontalLayout_3->addWidget(checkbox_cor_nevoa);

        botao_cor_nevoa = new QPushButton(groupBox_5);
        botao_cor_nevoa->setObjectName(QString::fromUtf8("botao_cor_nevoa"));
        sizePolicy5.setHeightForWidth(botao_cor_nevoa->sizePolicy().hasHeightForWidth());
        botao_cor_nevoa->setSizePolicy(sizePolicy5);

        horizontalLayout_3->addWidget(botao_cor_nevoa);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_6);


        verticalLayout_4->addLayout(horizontalLayout_3);


        gridLayout_2->addWidget(groupBox_5, 3, 0, 1, 4);

        groupBox_3 = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_20 = new QLabel(groupBox_3);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        sizePolicy.setHeightForWidth(label_20->sizePolicy().hasHeightForWidth());
        label_20->setSizePolicy(sizePolicy);

        horizontalLayout_12->addWidget(label_20);

        combo_herdar_ceu = new QComboBox(groupBox_3);
        combo_herdar_ceu->setObjectName(QString::fromUtf8("combo_herdar_ceu"));

        horizontalLayout_12->addWidget(combo_herdar_ceu);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_12->addItem(horizontalSpacer_2);


        verticalLayout_3->addLayout(horizontalLayout_12);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_14 = new QLabel(groupBox_3);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_14);

        combo_ceu = new QComboBox(groupBox_3);
        combo_ceu->setObjectName(QString::fromUtf8("combo_ceu"));

        horizontalLayout_4->addWidget(combo_ceu);

        checkbox_luz_ceu = new QCheckBox(groupBox_3);
        checkbox_luz_ceu->setObjectName(QString::fromUtf8("checkbox_luz_ceu"));

        horizontalLayout_4->addWidget(checkbox_luz_ceu);


        verticalLayout_3->addLayout(horizontalLayout_4);


        gridLayout_2->addWidget(groupBox_3, 4, 0, 1, 4);

        groupBox_2 = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        label_19 = new QLabel(groupBox_2);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        sizePolicy.setHeightForWidth(label_19->sizePolicy().hasHeightForWidth());
        label_19->setSizePolicy(sizePolicy);

        horizontalLayout_11->addWidget(label_19);

        combo_herdar_piso = new QComboBox(groupBox_2);
        combo_herdar_piso->setObjectName(QString::fromUtf8("combo_herdar_piso"));

        horizontalLayout_11->addWidget(combo_herdar_piso);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_4);


        verticalLayout_2->addLayout(horizontalLayout_11);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_8->addWidget(label);

        combo_fundo = new QComboBox(groupBox_2);
        combo_fundo->setObjectName(QString::fromUtf8("combo_fundo"));

        horizontalLayout_8->addWidget(combo_fundo);


        horizontalLayout->addLayout(horizontalLayout_8);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        checkbox_cor_piso = new QCheckBox(groupBox_2);
        checkbox_cor_piso->setObjectName(QString::fromUtf8("checkbox_cor_piso"));
        sizePolicy1.setHeightForWidth(checkbox_cor_piso->sizePolicy().hasHeightForWidth());
        checkbox_cor_piso->setSizePolicy(sizePolicy1);

        horizontalLayout_7->addWidget(checkbox_cor_piso);

        botao_cor_piso = new QPushButton(groupBox_2);
        botao_cor_piso->setObjectName(QString::fromUtf8("botao_cor_piso"));

        horizontalLayout_7->addWidget(botao_cor_piso);


        horizontalLayout->addLayout(horizontalLayout_7);

        checkbox_ladrilho = new QCheckBox(groupBox_2);
        checkbox_ladrilho->setObjectName(QString::fromUtf8("checkbox_ladrilho"));

        horizontalLayout->addWidget(checkbox_ladrilho);

        horizontalLayout_20 = new QHBoxLayout();
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        label_11 = new QLabel(groupBox_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_20->addWidget(label_11);

        spin_escala_piso_x = new QDoubleSpinBox(groupBox_2);
        spin_escala_piso_x->setObjectName(QString::fromUtf8("spin_escala_piso_x"));
        spin_escala_piso_x->setMaximum(1000.000000000000000);

        horizontalLayout_20->addWidget(spin_escala_piso_x);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_23 = new QLabel(groupBox_2);
        label_23->setObjectName(QString::fromUtf8("label_23"));

        horizontalLayout_16->addWidget(label_23);

        spin_escala_piso_y = new QDoubleSpinBox(groupBox_2);
        spin_escala_piso_y->setObjectName(QString::fromUtf8("spin_escala_piso_y"));
        spin_escala_piso_y->setMaximum(1000.000000000000000);

        horizontalLayout_16->addWidget(spin_escala_piso_y);


        horizontalLayout_20->addLayout(horizontalLayout_16);


        horizontalLayout->addLayout(horizontalLayout_20);

        checkbox_mestre_apenas = new QCheckBox(groupBox_2);
        checkbox_mestre_apenas->setObjectName(QString::fromUtf8("checkbox_mestre_apenas"));

        horizontalLayout->addWidget(checkbox_mestre_apenas);


        verticalLayout_2->addLayout(horizontalLayout);


        gridLayout_2->addWidget(groupBox_2, 5, 0, 1, 4);

        groupBox_4 = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        horizontalLayout_13 = new QHBoxLayout(groupBox_4);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        linha_largura = new QLineEdit(groupBox_4);
        linha_largura->setObjectName(QString::fromUtf8("linha_largura"));
        sizePolicy1.setHeightForWidth(linha_largura->sizePolicy().hasHeightForWidth());
        linha_largura->setSizePolicy(sizePolicy1);
        linha_largura->setMaximumSize(QSize(50, 16777215));
        linha_largura->setInputMask(QString::fromUtf8("099"));

        horizontalLayout_2->addWidget(linha_largura);

        label_10 = new QLabel(groupBox_4);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy1.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(label_10);

        linha_altura = new QLineEdit(groupBox_4);
        linha_altura->setObjectName(QString::fromUtf8("linha_altura"));
        sizePolicy1.setHeightForWidth(linha_altura->sizePolicy().hasHeightForWidth());
        linha_altura->setSizePolicy(sizePolicy1);
        linha_altura->setMaximumSize(QSize(50, 16777215));
        linha_altura->setInputMask(QString::fromUtf8("099"));

        horizontalLayout_2->addWidget(linha_altura);

        checkbox_tamanho_automatico = new QCheckBox(groupBox_4);
        checkbox_tamanho_automatico->setObjectName(QString::fromUtf8("checkbox_tamanho_automatico"));

        horizontalLayout_2->addWidget(checkbox_tamanho_automatico);

        checkbox_grade = new QCheckBox(groupBox_4);
        checkbox_grade->setObjectName(QString::fromUtf8("checkbox_grade"));

        horizontalLayout_2->addWidget(checkbox_grade);


        horizontalLayout_13->addLayout(horizontalLayout_2);


        gridLayout_2->addWidget(groupBox_4, 6, 0, 1, 4);

        botoes = new QDialogButtonBox(ifg__qt__DialogoIluminacao);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout_2->addWidget(botoes, 7, 0, 1, 4);


        retranslateUi(ifg__qt__DialogoIluminacao);
        QObject::connect(botoes, &QDialogButtonBox::rejected, ifg__qt__DialogoIluminacao, qOverload<>(&QDialog::reject));
        QObject::connect(checkbox_nevoa, &QCheckBox::clicked, linha_nevoa_min, &QLineEdit::setEnabled);
        QObject::connect(checkbox_nevoa, &QCheckBox::clicked, linha_nevoa_max, &QLineEdit::setEnabled);

        QMetaObject::connectSlotsByName(ifg__qt__DialogoIluminacao);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        ifg__qt__DialogoIluminacao->setWindowTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Propriedades Tabuleiro", nullptr));
        label_15->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Id", nullptr));
        label_16->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Descri\303\247\303\243o", nullptr));
        campo_descricao->setPlaceholderText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Descri\303\247\303\243o do cen\303\241rio", nullptr));
        label_17->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Clonar", nullptr));
        botao_clonar->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "OK", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Ilumina\303\247\303\243o", nullptr));
        label_21->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Herdar de", nullptr));
#if QT_CONFIG(tooltip)
        combo_herdar_iluminacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se v\303\241ilido, configura\303\247\303\265es de luz ser\303\243o as mesmas do cen\303\241rio passado.", nullptr));
#endif // QT_CONFIG(tooltip)
        botao_cor_ambiente->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz  Ambiente", nullptr));
        label_18->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Luz Direcional", nullptr));
        botao_cor_direcional->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz Direcional", nullptr));
        label_6->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "E", nullptr));
        label_7->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Poente", nullptr));
        label_3->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "S", nullptr));
#if QT_CONFIG(tooltip)
        dial_posicao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Posi\303\247\303\243o da fonte de luz", nullptr));
#endif // QT_CONFIG(tooltip)
        label_9->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "A pino", nullptr));
        label_2->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Nascente", nullptr));
#if QT_CONFIG(tooltip)
        dial_inclinacao->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Inclina\303\247\303\243o da Luz", nullptr));
#endif // QT_CONFIG(tooltip)
        label_8->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Abaixo solo", nullptr));
        label_5->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "W", nullptr));
        label_4->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "N", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "N\303\251voa", nullptr));
        label_22->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Herdar de", nullptr));
#if QT_CONFIG(tooltip)
        combo_herdar_nevoa->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se v\303\241ilido, configura\303\247\303\265es de n\303\251voa ser\303\243o as mesmas do cen\303\241rio passado.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_nevoa->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Habilitar", nullptr));
        label_12->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Min", nullptr));
#if QT_CONFIG(tooltip)
        linha_nevoa_min->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Dist\303\242ncia m\303\255nma da n\303\251voa, em m.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_13->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Max", nullptr));
#if QT_CONFIG(tooltip)
        linha_nevoa_max->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Dist\303\242ncia onde o efeito m\303\241ximo da n\303\251voa \303\251 aplicado.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_cor_nevoa->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor", nullptr));
        botao_cor_nevoa->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor da N\303\251voa", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "C\303\251u", nullptr));
        label_20->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Herdar de", nullptr));
#if QT_CONFIG(tooltip)
        combo_herdar_ceu->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se v\303\241ilido, configura\303\247\303\265es de c\303\251u ser\303\243o as mesmas do cen\303\241rio passado.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_14->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "C\303\251u", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_luz_ceu->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_luz_ceu->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Aplicar luz ambiente", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Piso", nullptr));
        label_19->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Herdar de", nullptr));
#if QT_CONFIG(tooltip)
        combo_herdar_piso->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se v\303\241ilido, configura\303\247\303\265es de piso ser\303\243o as mesmas do cen\303\241rio passado.", nullptr));
#endif // QT_CONFIG(tooltip)
        label->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Piso", nullptr));
#if QT_CONFIG(tooltip)
        combo_fundo->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Escolha o tipo de piso", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_cor_piso->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Usar cor de piso?", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_cor_piso->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor", nullptr));
        botao_cor_piso->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Cor do Piso", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_ladrilho->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, aplicar\303\241 a textura como um ladrilho (por quadrado)", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_ladrilho->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Ladrilho", nullptr));
        label_11->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Escala X", nullptr));
        label_23->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Escala Y", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_mestre_apenas->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, apenas mestre ver\303\241 o piso", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_mestre_apenas->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Mestre Apenas", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Tamanho", nullptr));
#if QT_CONFIG(tooltip)
        linha_largura->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "largura", nullptr));
#endif // QT_CONFIG(tooltip)
        label_10->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "x", nullptr));
#if QT_CONFIG(tooltip)
        linha_altura->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "altura", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        checkbox_tamanho_automatico->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_tamanho_automatico->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Autom\303\241tico", nullptr));
#if QT_CONFIG(tooltip)
        checkbox_grade->setToolTip(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_grade->setText(QCoreApplication::translate("ifg::qt::DialogoIluminacao", "Grade", nullptr));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoIluminacao: public Ui_DialogoIluminacao {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // CENARIO_H
