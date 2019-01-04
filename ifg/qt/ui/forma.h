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
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace ifg {
namespace qt {

class Ui_DialogoForma
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *campo_id;
    QDialogButtonBox *botoes;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget;
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
    QGroupBox *groupBox_2;
    QDial *dial_rotacao;
    QDoubleSpinBox *spin_translacao_quad;
    QLabel *label_2;
    QSpinBox *spin_rotacao;
    QLabel *label_15;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_9;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_12;
    QSlider *slider_alfa;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_8;
    QSpinBox *spin_pontos_vida;
    QLabel *label_10;
    QSpinBox *spin_max_pontos_vida;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_luz;
    QPushButton *botao_luz;
    QGroupBox *groupBox_3;
    QDial *dial_rotacao_y;
    QSpinBox *spin_rotacao_y;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *horizontalLayout_8;
    QLabel *label_4;
    QComboBox *combo_textura;
    QCheckBox *checkbox_ladrilho;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_11;
    QCheckBox *checkbox_fixa;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_visibilidade;
    QCheckBox *checkbox_faz_sombra;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_11;
    QPlainTextEdit *lista_rotulos;
    QGroupBox *groupBox_4;
    QDial *dial_rotacao_x;
    QSpinBox *spin_rotacao_x;
    QGroupBox *groupBox_5;
    QWidget *verticalLayoutWidget_2;
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
    QCheckBox *checkbox_colisao;
    QWidget *layoutWidget_3;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_16;
    QPlainTextEdit *lista_tesouro;
    QCheckBox *checkbox_dois_lados;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_17;
    QComboBox *combo_tipo_forma;

    void setupUi(QDialog *ifg__qt__DialogoForma)
    {
        if (ifg__qt__DialogoForma->objectName().isEmpty())
            ifg__qt__DialogoForma->setObjectName(QStringLiteral("ifg__qt__DialogoForma"));
        ifg__qt__DialogoForma->resize(902, 561);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(320, 10, 181, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(horizontalLayoutWidget);
        campo_id->setObjectName(QStringLiteral("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);

        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setGeometry(QRect(490, 490, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_2->setObjectName(QStringLiteral("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(610, 350, 221, 41));
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

        groupBox = new QGroupBox(ifg__qt__DialogoForma);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(60, 370, 161, 141));
        verticalLayoutWidget = new QWidget(groupBox);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 20, 160, 121));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        label_7 = new QLabel(verticalLayoutWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x_quad = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_x_quad->setObjectName(QStringLiteral("spin_escala_x_quad"));
        spin_escala_x_quad->setDecimals(2);
        spin_escala_x_quad->setMinimum(-1000);
        spin_escala_x_quad->setMaximum(1000);
        spin_escala_x_quad->setSingleStep(0.1);

        horizontalLayout_4->addWidget(spin_escala_x_quad);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        label_5 = new QLabel(verticalLayoutWidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y_quad = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_y_quad->setObjectName(QStringLiteral("spin_escala_y_quad"));
        spin_escala_y_quad->setDecimals(2);
        spin_escala_y_quad->setMinimum(-1000);
        spin_escala_y_quad->setMaximum(1000);
        spin_escala_y_quad->setSingleStep(0.1);

        horizontalLayout_5->addWidget(spin_escala_y_quad);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        label_6 = new QLabel(verticalLayoutWidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z_quad = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_z_quad->setObjectName(QStringLiteral("spin_escala_z_quad"));
        spin_escala_z_quad->setDecimals(2);
        spin_escala_z_quad->setMinimum(-50);
        spin_escala_z_quad->setMaximum(50);
        spin_escala_z_quad->setSingleStep(0.1);

        horizontalLayout_6->addWidget(spin_escala_z_quad);


        verticalLayout->addLayout(horizontalLayout_6);

        groupBox_2 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(50, 240, 191, 131));
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
        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_3->setObjectName(QStringLiteral("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(610, 390, 221, 41));
        horizontalLayout_9 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_9->setObjectName(QStringLiteral("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QStringLiteral("horizontalLayout_10"));
        label_9 = new QLabel(horizontalLayoutWidget_3);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_9);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QStringLiteral("horizontalLayout_12"));
        slider_alfa = new QSlider(horizontalLayoutWidget_3);
        slider_alfa->setObjectName(QStringLiteral("slider_alfa"));
        sizePolicy1.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy1);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_12->addWidget(slider_alfa);


        horizontalLayout_9->addLayout(horizontalLayout_12);

        layoutWidget = new QWidget(ifg__qt__DialogoForma);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(140, 70, 277, 29));
        horizontalLayout_13 = new QHBoxLayout(layoutWidget);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 0, 0, 0);
        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QStringLiteral("label_8"));
        sizePolicy.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(layoutWidget);
        spin_pontos_vida->setObjectName(QStringLiteral("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(layoutWidget);
        label_10->setObjectName(QStringLiteral("label_10"));
        sizePolicy.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(layoutWidget);
        spin_max_pontos_vida->setObjectName(QStringLiteral("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);

        horizontalLayoutWidget_4 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_4->setObjectName(QStringLiteral("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(610, 430, 221, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkbox_luz = new QCheckBox(horizontalLayoutWidget_4);
        checkbox_luz->setObjectName(QStringLiteral("checkbox_luz"));
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        botao_luz = new QPushButton(horizontalLayoutWidget_4);
        botao_luz->setObjectName(QStringLiteral("botao_luz"));
        botao_luz->setStyleSheet(QStringLiteral(""));

        horizontalLayout_3->addWidget(botao_luz);

        groupBox_3 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(250, 230, 111, 141));
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
        horizontalLayoutWidget_5 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_5->setObjectName(QStringLiteral("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(450, 70, 361, 41));
        horizontalLayout_8 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        label_4 = new QLabel(horizontalLayoutWidget_5);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_8->addWidget(label_4);

        combo_textura = new QComboBox(horizontalLayoutWidget_5);
        combo_textura->setObjectName(QStringLiteral("combo_textura"));

        horizontalLayout_8->addWidget(combo_textura);

        checkbox_ladrilho = new QCheckBox(horizontalLayoutWidget_5);
        checkbox_ladrilho->setObjectName(QStringLiteral("checkbox_ladrilho"));
        sizePolicy1.setHeightForWidth(checkbox_ladrilho->sizePolicy().hasHeightForWidth());
        checkbox_ladrilho->setSizePolicy(sizePolicy1);

        horizontalLayout_8->addWidget(checkbox_ladrilho);

        horizontalLayoutWidget_6 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_6->setObjectName(QStringLiteral("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(470, 270, 391, 31));
        horizontalLayout_11 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_11->setObjectName(QStringLiteral("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        checkbox_fixa = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_fixa->setObjectName(QStringLiteral("checkbox_fixa"));

        horizontalLayout_11->addWidget(checkbox_fixa);

        checkbox_selecionavel = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_selecionavel->setObjectName(QStringLiteral("checkbox_selecionavel"));

        horizontalLayout_11->addWidget(checkbox_selecionavel);

        checkbox_visibilidade = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_visibilidade->setObjectName(QStringLiteral("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy2);

        horizontalLayout_11->addWidget(checkbox_visibilidade);

        checkbox_faz_sombra = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_faz_sombra->setObjectName(QStringLiteral("checkbox_faz_sombra"));

        horizontalLayout_11->addWidget(checkbox_faz_sombra);

        layoutWidget_2 = new QWidget(ifg__qt__DialogoForma);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(70, 100, 370, 131));
        horizontalLayout_18 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_18->setObjectName(QStringLiteral("horizontalLayout_18"));
        horizontalLayout_18->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(layoutWidget_2);
        label_11->setObjectName(QStringLiteral("label_11"));
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy3);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_11);

        lista_rotulos = new QPlainTextEdit(layoutWidget_2);
        lista_rotulos->setObjectName(QStringLiteral("lista_rotulos"));

        horizontalLayout_18->addWidget(lista_rotulos);

        groupBox_4 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_4->setObjectName(QStringLiteral("groupBox_4"));
        groupBox_4->setGeometry(QRect(360, 230, 111, 141));
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
        groupBox_5 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_5->setObjectName(QStringLiteral("groupBox_5"));
        groupBox_5->setGeometry(QRect(450, 120, 401, 131));
        verticalLayoutWidget_2 = new QWidget(groupBox_5);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 40, 391, 80));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QStringLiteral("horizontalLayout_19"));
        combo_transicao = new QComboBox(verticalLayoutWidget_2);
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->addItem(QString());
        combo_transicao->setObjectName(QStringLiteral("combo_transicao"));

        horizontalLayout_19->addWidget(combo_transicao);

        combo_id_cenario = new QComboBox(verticalLayoutWidget_2);
        combo_id_cenario->setObjectName(QStringLiteral("combo_id_cenario"));

        horizontalLayout_19->addWidget(combo_id_cenario);

        checkbox_transicao_posicao = new QCheckBox(verticalLayoutWidget_2);
        checkbox_transicao_posicao->setObjectName(QStringLiteral("checkbox_transicao_posicao"));

        horizontalLayout_19->addWidget(checkbox_transicao_posicao);

        botao_transicao_mapa = new QPushButton(verticalLayoutWidget_2);
        botao_transicao_mapa->setObjectName(QStringLiteral("botao_transicao_mapa"));
        sizePolicy1.setHeightForWidth(botao_transicao_mapa->sizePolicy().hasHeightForWidth());
        botao_transicao_mapa->setSizePolicy(sizePolicy1);

        horizontalLayout_19->addWidget(botao_transicao_mapa);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QStringLiteral("horizontalLayout_14"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QStringLiteral("horizontalLayout_15"));
        label_12 = new QLabel(verticalLayoutWidget_2);
        label_12->setObjectName(QStringLiteral("label_12"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy4);
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_12);

        spin_trans_x = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_x->setObjectName(QStringLiteral("spin_trans_x"));
        spin_trans_x->setDecimals(1);
        spin_trans_x->setMinimum(-1000);
        spin_trans_x->setMaximum(1000);
        spin_trans_x->setSingleStep(0.5);

        horizontalLayout_15->addWidget(spin_trans_x);


        horizontalLayout_14->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QStringLiteral("horizontalLayout_16"));
        label_13 = new QLabel(verticalLayoutWidget_2);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_16->addWidget(label_13);

        spin_trans_y = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_y->setObjectName(QStringLiteral("spin_trans_y"));
        spin_trans_y->setDecimals(1);
        spin_trans_y->setMinimum(-1000);
        spin_trans_y->setMaximum(1000);
        spin_trans_y->setSingleStep(0.5);

        horizontalLayout_16->addWidget(spin_trans_y);


        horizontalLayout_14->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QStringLiteral("horizontalLayout_17"));
        label_14 = new QLabel(verticalLayoutWidget_2);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_17->addWidget(label_14);

        spin_trans_z = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_z->setObjectName(QStringLiteral("spin_trans_z"));
        spin_trans_z->setDecimals(1);
        spin_trans_z->setMinimum(-1000);
        spin_trans_z->setMaximum(1000);
        spin_trans_z->setSingleStep(0.5);

        horizontalLayout_17->addWidget(spin_trans_z);


        horizontalLayout_14->addLayout(horizontalLayout_17);


        verticalLayout_2->addLayout(horizontalLayout_14);

        checkbox_colisao = new QCheckBox(ifg__qt__DialogoForma);
        checkbox_colisao->setObjectName(QStringLiteral("checkbox_colisao"));
        checkbox_colisao->setGeometry(QRect(470, 310, 83, 22));
        layoutWidget_3 = new QWidget(ifg__qt__DialogoForma);
        layoutWidget_3->setObjectName(QStringLiteral("layoutWidget_3"));
        layoutWidget_3->setGeometry(QRect(330, 380, 221, 141));
        horizontalLayout_20 = new QHBoxLayout(layoutWidget_3);
        horizontalLayout_20->setObjectName(QStringLiteral("horizontalLayout_20"));
        horizontalLayout_20->setContentsMargins(0, 0, 0, 0);
        label_16 = new QLabel(layoutWidget_3);
        label_16->setObjectName(QStringLiteral("label_16"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(label_16->sizePolicy().hasHeightForWidth());
        label_16->setSizePolicy(sizePolicy5);
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_16);

        lista_tesouro = new QPlainTextEdit(layoutWidget_3);
        lista_tesouro->setObjectName(QStringLiteral("lista_tesouro"));

        horizontalLayout_20->addWidget(lista_tesouro);

        checkbox_dois_lados = new QCheckBox(ifg__qt__DialogoForma);
        checkbox_dois_lados->setObjectName(QStringLiteral("checkbox_dois_lados"));
        checkbox_dois_lados->setGeometry(QRect(560, 310, 101, 22));
        layoutWidget1 = new QWidget(ifg__qt__DialogoForma);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(580, 20, 126, 25));
        horizontalLayout_21 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_21->setObjectName(QStringLiteral("horizontalLayout_21"));
        horizontalLayout_21->setContentsMargins(0, 0, 0, 0);
        label_17 = new QLabel(layoutWidget1);
        label_17->setObjectName(QStringLiteral("label_17"));

        horizontalLayout_21->addWidget(label_17);

        combo_tipo_forma = new QComboBox(layoutWidget1);
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->addItem(QString());
        combo_tipo_forma->setObjectName(QStringLiteral("combo_tipo_forma"));

        horizontalLayout_21->addWidget(combo_tipo_forma);

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
        QWidget::setTabOrder(botao_luz, botoes);
        QWidget::setTabOrder(botoes, campo_id);

        retranslateUi(ifg__qt__DialogoForma);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoForma, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoForma, SLOT(accept()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoForma);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoForma)
    {
        ifg__qt__DialogoForma->setWindowTitle(QApplication::translate("ifg::qt::DialogoForma", "Dialog", nullptr));
        label->setText(QApplication::translate("ifg::qt::DialogoForma", "Id", nullptr));
        label_3->setText(QApplication::translate("ifg::qt::DialogoForma", "Cor:", nullptr));
        checkbox_cor->setText(QString());
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor", nullptr));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Dimens\303\265es (quadrados)", nullptr));
        label_7->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam X", nullptr));
        label_5->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam Y", nullptr));
        label_6->setText(QApplication::translate("ifg::qt::DialogoForma", "Altura", nullptr));
        groupBox_2->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o e Transla\303\247\303\243o em Z", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o do objeto ao redor do eixo Z.", nullptr));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ifg::qt::DialogoForma", " Transla\303\247\303\243o", nullptr));
        label_15->setText(QApplication::translate("ifg::qt::DialogoForma", "quad", nullptr));
        label_9->setText(QApplication::translate("ifg::qt::DialogoForma", "Alfa", nullptr));
        label_8->setText(QApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida:", nullptr));
        label_10->setText(QApplication::translate("ifg::qt::DialogoForma", "Max", nullptr));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Luz", nullptr));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor da Luz", nullptr));
        groupBox_3->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em Y", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_y->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("ifg::qt::DialogoForma", "Textura", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_ladrilho->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_ladrilho->setText(QApplication::translate("ifg::qt::DialogoForma", "ladrilho", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_fixa->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_fixa->setText(QApplication::translate("ifg::qt::DialogoForma", "Fixa", nullptr));
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoForma", "Selecion\303\241vel", nullptr));
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoForma", "Vis\303\255vel", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_faz_sombra->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_faz_sombra->setText(QApplication::translate("ifg::qt::DialogoForma", "Faz Sombra", nullptr));
        label_11->setText(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos Especial", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_4->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em X", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_x->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", nullptr));
#endif // QT_NO_TOOLTIP
        groupBox_5->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Transi\303\247\303\243o de Cen\303\241rio", nullptr));
        combo_transicao->setItemText(0, QApplication::translate("ifg::qt::DialogoForma", "Sem Transi\303\247\303\243o", nullptr));
        combo_transicao->setItemText(1, QApplication::translate("ifg::qt::DialogoForma", "Cen\303\241rio", nullptr));
        combo_transicao->setItemText(2, QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));

        checkbox_transicao_posicao->setText(QApplication::translate("ifg::qt::DialogoForma", "Posi\303\247\303\243o?", nullptr));
        botao_transicao_mapa->setText(QApplication::translate("ifg::qt::DialogoForma", "Clicar", nullptr));
        label_12->setText(QApplication::translate("ifg::qt::DialogoForma", "X", nullptr));
        label_13->setText(QApplication::translate("ifg::qt::DialogoForma", "Y", nullptr));
        label_14->setText(QApplication::translate("ifg::qt::DialogoForma", "Z", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_colisao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_colisao->setText(QApplication::translate("ifg::qt::DialogoForma", "Colis\303\243o", nullptr));
        label_16->setText(QApplication::translate("ifg::qt::DialogoForma", "Tesouro", nullptr));
#ifndef QT_NO_TOOLTIP
        lista_tesouro->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_dois_lados->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nos dois lados da primitiva ser\303\243o desenhados.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_dois_lados->setText(QApplication::translate("ifg::qt::DialogoForma", "Dois Lados", nullptr));
        label_17->setText(QApplication::translate("ifg::qt::DialogoForma", "Tipo", nullptr));
        combo_tipo_forma->setItemText(0, QApplication::translate("ifg::qt::DialogoForma", "Cilindro", nullptr));
        combo_tipo_forma->setItemText(1, QApplication::translate("ifg::qt::DialogoForma", "Cone", nullptr));
        combo_tipo_forma->setItemText(2, QApplication::translate("ifg::qt::DialogoForma", "Cubo", nullptr));
        combo_tipo_forma->setItemText(3, QApplication::translate("ifg::qt::DialogoForma", "Esfera", nullptr));
        combo_tipo_forma->setItemText(4, QApplication::translate("ifg::qt::DialogoForma", "Pir\303\242mide", nullptr));
        combo_tipo_forma->setItemText(5, QApplication::translate("ifg::qt::DialogoForma", "Hemisf\303\251rio", nullptr));

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
