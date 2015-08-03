/********************************************************************************
** Form generated from reading UI file 'forma.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef FORMA_H
#define FORMA_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

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
    QPushButton *botao_cor;
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_7;
    QDoubleSpinBox *spin_escala_x;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_5;
    QDoubleSpinBox *spin_escala_y;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_6;
    QDoubleSpinBox *spin_escala_z;
    QGroupBox *groupBox_2;
    QDial *dial_rotacao;
    QDoubleSpinBox *spin_translacao;
    QLabel *label_2;
    QSpinBox *spin_rotacao;
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
    QLineEdit *linha_textura;
    QPushButton *botao_textura;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_11;
    QCheckBox *checkbox_fixa;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_visibilidade;
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
    QCheckBox *checkbox_transicao_cenario;
    QLineEdit *linha_transicao_cenario;
    QCheckBox *checkbox_transicao_posicao;
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

    void setupUi(QDialog *ifg__qt__DialogoForma)
    {
        if (ifg__qt__DialogoForma->objectName().isEmpty())
            ifg__qt__DialogoForma->setObjectName(QString::fromUtf8("ifg__qt__DialogoForma"));
        ifg__qt__DialogoForma->resize(496, 804);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(180, 10, 181, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(horizontalLayoutWidget);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);

        botoes = new QDialogButtonBox(ifg__qt__DialogoForma);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(120, 760, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(240, 450, 221, 41));
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


        horizontalLayout_2->addLayout(horizontalLayout_7);

        botao_cor = new QPushButton(horizontalLayoutWidget_2);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        groupBox = new QGroupBox(ifg__qt__DialogoForma);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(70, 440, 161, 141));
        verticalLayoutWidget = new QWidget(groupBox);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 20, 160, 121));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_7 = new QLabel(verticalLayoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        spin_escala_x = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_x->setObjectName(QString::fromUtf8("spin_escala_x"));
        spin_escala_x->setDecimals(1);
        spin_escala_x->setMinimum(-1000);
        spin_escala_x->setMaximum(1000);
        spin_escala_x->setSingleStep(0.5);

        horizontalLayout_4->addWidget(spin_escala_x);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_5 = new QLabel(verticalLayoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_5->addWidget(label_5);

        spin_escala_y = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_y->setObjectName(QString::fromUtf8("spin_escala_y"));
        spin_escala_y->setDecimals(1);
        spin_escala_y->setMinimum(-1000);
        spin_escala_y->setMaximum(1000);
        spin_escala_y->setSingleStep(0.5);

        horizontalLayout_5->addWidget(spin_escala_y);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_6 = new QLabel(verticalLayoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        spin_escala_z = new QDoubleSpinBox(verticalLayoutWidget);
        spin_escala_z->setObjectName(QString::fromUtf8("spin_escala_z"));
        spin_escala_z->setDecimals(1);
        spin_escala_z->setMinimum(-50);
        spin_escala_z->setMaximum(50);
        spin_escala_z->setSingleStep(0.5);

        horizontalLayout_6->addWidget(spin_escala_z);


        verticalLayout->addLayout(horizontalLayout_6);

        groupBox_2 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(70, 240, 181, 131));
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
        dial_rotacao->setNotchTarget(45);
        dial_rotacao->setNotchesVisible(true);
        spin_translacao = new QDoubleSpinBox(groupBox_2);
        spin_translacao->setObjectName(QString::fromUtf8("spin_translacao"));
        spin_translacao->setGeometry(QRect(80, 60, 91, 24));
        spin_translacao->setDecimals(1);
        spin_translacao->setMaximum(100);
        spin_translacao->setSingleStep(0.1);
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(80, 40, 111, 16));
        spin_rotacao = new QSpinBox(groupBox_2);
        spin_rotacao->setObjectName(QString::fromUtf8("spin_rotacao"));
        spin_rotacao->setGeometry(QRect(20, 100, 51, 24));
        spin_rotacao->setMaximum(360);
        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(240, 490, 221, 41));
        horizontalLayout_9 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_9 = new QLabel(horizontalLayoutWidget_3);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_9);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        slider_alfa = new QSlider(horizontalLayoutWidget_3);
        slider_alfa->setObjectName(QString::fromUtf8("slider_alfa"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(slider_alfa->sizePolicy().hasHeightForWidth());
        slider_alfa->setSizePolicy(sizePolicy1);
        slider_alfa->setMaximum(100);
        slider_alfa->setOrientation(Qt::Horizontal);

        horizontalLayout_12->addWidget(slider_alfa);


        horizontalLayout_9->addLayout(horizontalLayout_12);

        layoutWidget = new QWidget(ifg__qt__DialogoForma);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(140, 70, 277, 26));
        horizontalLayout_13 = new QHBoxLayout(layoutWidget);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 0, 0, 0);
        label_8 = new QLabel(layoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        sizePolicy.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_8);

        spin_pontos_vida = new QSpinBox(layoutWidget);
        spin_pontos_vida->setObjectName(QString::fromUtf8("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_pontos_vida);

        label_10 = new QLabel(layoutWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy);
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_10);

        spin_max_pontos_vida = new QSpinBox(layoutWidget);
        spin_max_pontos_vida->setObjectName(QString::fromUtf8("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_13->addWidget(spin_max_pontos_vida);

        horizontalLayoutWidget_4 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(240, 530, 221, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkbox_luz = new QCheckBox(horizontalLayoutWidget_4);
        checkbox_luz->setObjectName(QString::fromUtf8("checkbox_luz"));
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        botao_luz = new QPushButton(horizontalLayoutWidget_4);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(botao_luz);

        groupBox_3 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(250, 230, 111, 141));
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
        dial_rotacao_y->setNotchTarget(45);
        dial_rotacao_y->setNotchesVisible(true);
        spin_rotacao_y = new QSpinBox(groupBox_3);
        spin_rotacao_y->setObjectName(QString::fromUtf8("spin_rotacao_y"));
        spin_rotacao_y->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_y->setMinimum(-180);
        spin_rotacao_y->setMaximum(180);
        horizontalLayoutWidget_5 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_5->setObjectName(QString::fromUtf8("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(80, 390, 361, 41));
        horizontalLayout_8 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        linha_textura = new QLineEdit(horizontalLayoutWidget_5);
        linha_textura->setObjectName(QString::fromUtf8("linha_textura"));
        linha_textura->setReadOnly(false);

        horizontalLayout_8->addWidget(linha_textura);

        botao_textura = new QPushButton(horizontalLayoutWidget_5);
        botao_textura->setObjectName(QString::fromUtf8("botao_textura"));

        horizontalLayout_8->addWidget(botao_textura);

        horizontalLayoutWidget_6 = new QWidget(ifg__qt__DialogoForma);
        horizontalLayoutWidget_6->setObjectName(QString::fromUtf8("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(70, 590, 391, 31));
        horizontalLayout_11 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        checkbox_fixa = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_fixa->setObjectName(QString::fromUtf8("checkbox_fixa"));

        horizontalLayout_11->addWidget(checkbox_fixa);

        checkbox_selecionavel = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_selecionavel->setObjectName(QString::fromUtf8("checkbox_selecionavel"));

        horizontalLayout_11->addWidget(checkbox_selecionavel);

        checkbox_visibilidade = new QCheckBox(horizontalLayoutWidget_6);
        checkbox_visibilidade->setObjectName(QString::fromUtf8("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(checkbox_visibilidade->sizePolicy().hasHeightForWidth());
        checkbox_visibilidade->setSizePolicy(sizePolicy2);

        horizontalLayout_11->addWidget(checkbox_visibilidade);

        layoutWidget_2 = new QWidget(ifg__qt__DialogoForma);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(70, 100, 370, 131));
        horizontalLayout_18 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        horizontalLayout_18->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(layoutWidget_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        QSizePolicy sizePolicy3(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy3);
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_18->addWidget(label_11);

        lista_rotulos = new QPlainTextEdit(layoutWidget_2);
        lista_rotulos->setObjectName(QString::fromUtf8("lista_rotulos"));

        horizontalLayout_18->addWidget(lista_rotulos);

        groupBox_4 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setGeometry(QRect(360, 230, 111, 141));
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
        dial_rotacao_x->setNotchTarget(45);
        dial_rotacao_x->setNotchesVisible(true);
        spin_rotacao_x = new QSpinBox(groupBox_4);
        spin_rotacao_x->setObjectName(QString::fromUtf8("spin_rotacao_x"));
        spin_rotacao_x->setGeometry(QRect(30, 100, 51, 24));
        spin_rotacao_x->setMinimum(-180);
        spin_rotacao_x->setMaximum(180);
        groupBox_5 = new QGroupBox(ifg__qt__DialogoForma);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setGeometry(QRect(70, 620, 401, 131));
        verticalLayoutWidget_2 = new QWidget(groupBox_5);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 40, 381, 80));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        checkbox_transicao_cenario = new QCheckBox(verticalLayoutWidget_2);
        checkbox_transicao_cenario->setObjectName(QString::fromUtf8("checkbox_transicao_cenario"));

        horizontalLayout_19->addWidget(checkbox_transicao_cenario);

        linha_transicao_cenario = new QLineEdit(verticalLayoutWidget_2);
        linha_transicao_cenario->setObjectName(QString::fromUtf8("linha_transicao_cenario"));

        horizontalLayout_19->addWidget(linha_transicao_cenario);

        checkbox_transicao_posicao = new QCheckBox(verticalLayoutWidget_2);
        checkbox_transicao_posicao->setObjectName(QString::fromUtf8("checkbox_transicao_posicao"));

        horizontalLayout_19->addWidget(checkbox_transicao_posicao);


        verticalLayout_2->addLayout(horizontalLayout_19);

        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_12 = new QLabel(verticalLayoutWidget_2);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label_12->sizePolicy().hasHeightForWidth());
        label_12->setSizePolicy(sizePolicy4);

        horizontalLayout_15->addWidget(label_12);

        spin_trans_x = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_x->setObjectName(QString::fromUtf8("spin_trans_x"));
        spin_trans_x->setDecimals(1);
        spin_trans_x->setMinimum(-1000);
        spin_trans_x->setMaximum(1000);
        spin_trans_x->setSingleStep(0.5);

        horizontalLayout_15->addWidget(spin_trans_x);


        horizontalLayout_14->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_13 = new QLabel(verticalLayoutWidget_2);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_16->addWidget(label_13);

        spin_trans_y = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_y->setObjectName(QString::fromUtf8("spin_trans_y"));
        spin_trans_y->setDecimals(1);
        spin_trans_y->setMinimum(-1000);
        spin_trans_y->setMaximum(1000);
        spin_trans_y->setSingleStep(0.5);

        horizontalLayout_16->addWidget(spin_trans_y);


        horizontalLayout_14->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        label_14 = new QLabel(verticalLayoutWidget_2);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_17->addWidget(label_14);

        spin_trans_z = new QDoubleSpinBox(verticalLayoutWidget_2);
        spin_trans_z->setObjectName(QString::fromUtf8("spin_trans_z"));
        spin_trans_z->setDecimals(1);
        spin_trans_z->setMinimum(-1000);
        spin_trans_z->setMaximum(1000);
        spin_trans_z->setSingleStep(0.5);

        horizontalLayout_17->addWidget(spin_trans_z);


        horizontalLayout_14->addLayout(horizontalLayout_17);


        verticalLayout_2->addLayout(horizontalLayout_14);

        horizontalLayoutWidget->raise();
        botoes->raise();
        horizontalLayoutWidget_2->raise();
        groupBox->raise();
        groupBox_2->raise();
        horizontalLayoutWidget_3->raise();
        layoutWidget->raise();
        horizontalLayoutWidget_4->raise();
        groupBox_3->raise();
        horizontalLayoutWidget_5->raise();
        horizontalLayoutWidget_6->raise();
        layoutWidget_2->raise();
        groupBox_4->raise();
        groupBox_5->raise();
        verticalLayoutWidget_2->raise();

        retranslateUi(ifg__qt__DialogoForma);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoForma, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoForma, SLOT(accept()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoForma);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoForma)
    {
        ifg__qt__DialogoForma->setWindowTitle(QApplication::translate("ifg::qt::DialogoForma", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoForma", "Id", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ifg::qt::DialogoForma", "Cor:", 0, QApplication::UnicodeUTF8));
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Dimens\303\265es", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam X", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoForma", "Tam Y", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ifg::qt::DialogoForma", "Altura", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o e Transla\303\247\303\243o em Z", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_rotacao->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o do objeto ao redor do eixo Z.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_2->setText(QApplication::translate("ifg::qt::DialogoForma", "Transla\303\247\303\243o", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("ifg::qt::DialogoForma", "Alfa", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ifg::qt::DialogoForma", "Pontos de Vida:", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("ifg::qt::DialogoForma", "Max", 0, QApplication::UnicodeUTF8));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Luz", 0, QApplication::UnicodeUTF8));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Cor da Luz", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em Y", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_y->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        linha_textura->setPlaceholderText(QApplication::translate("ifg::qt::DialogoForma", "Caminho para textura ou vazio", 0, QApplication::UnicodeUTF8));
        botao_textura->setText(QApplication::translate("ifg::qt::DialogoForma", "Escolher Textura", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_fixa->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "Se verdadeiro, nao sera movel. Selecionavel apenas com duplo clique.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_fixa->setText(QApplication::translate("ifg::qt::DialogoForma", "Fixa", 0, QApplication::UnicodeUTF8));
        checkbox_selecionavel->setText(QApplication::translate("ifg::qt::DialogoForma", "Selecion\303\241vel", 0, QApplication::UnicodeUTF8));
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoForma", "Vis\303\255vel", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos Especial", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        groupBox_4->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Rota\303\247\303\243o em X", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_rotacao_x->setToolTip(QApplication::translate("ifg::qt::DialogoForma", "<html><head/><body><p>Rota\303\247\303\243o do objeto ao redor do eixo Y.</p></body></html>", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        groupBox_5->setTitle(QApplication::translate("ifg::qt::DialogoForma", "Transi\303\247\303\243o de Cen\303\241rio", 0, QApplication::UnicodeUTF8));
        checkbox_transicao_cenario->setText(QApplication::translate("ifg::qt::DialogoForma", "Possui transi\303\247\303\243o?", 0, QApplication::UnicodeUTF8));
        linha_transicao_cenario->setPlaceholderText(QApplication::translate("ifg::qt::DialogoForma", "Id do cen\303\241rio", 0, QApplication::UnicodeUTF8));
        checkbox_transicao_posicao->setText(QApplication::translate("ifg::qt::DialogoForma", "Posi\303\247\303\243o?", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("ifg::qt::DialogoForma", "X", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("ifg::qt::DialogoForma", "Y", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("ifg::qt::DialogoForma", "Z", 0, QApplication::UnicodeUTF8));
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
