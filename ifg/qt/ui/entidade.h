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

class Ui_DialogoEntidade
{
public:
    QDialogButtonBox *botoes;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *campo_id;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout_14;
    QLabel *label_12;
    QDoubleSpinBox *spin_raio;
    QPushButton *botao_luz;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QLabel *label_tamanho;
    QSlider *slider_tamanho;
    QWidget *horizontalLayoutWidget_7;
    QHBoxLayout *horizontalLayout_8;
    QCheckBox *checkbox_selecionavel;
    QCheckBox *checkbox_voadora;
    QCheckBox *checkbox_visibilidade;
    QWidget *horizontalLayoutWidget_8;
    QHBoxLayout *horizontalLayout_11;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_5;
    QSpinBox *spin_pontos_vida;
    QLabel *label_6;
    QSpinBox *spin_max_pontos_vida;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout_9;
    QCheckBox *checkbox_caida;
    QCheckBox *checkbox_morta;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_4;
    QDoubleSpinBox *spin_aura;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_13;
    QLabel *label_7;
    QDoubleSpinBox *spin_translacao;
    QWidget *horizontalLayoutWidget_10;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_9;
    QComboBox *combo_salvacao;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_15;
    QHBoxLayout *horizontalLayout_17;
    QLabel *label_8;
    QLineEdit *campo_rotulo;
    QHBoxLayout *horizontalLayout_18;
    QLabel *label_10;
    QPlainTextEdit *lista_rotulos;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_19;
    QLabel *label_11;
    QPlainTextEdit *lista_eventos;
    QWidget *horizontalLayoutWidget_11;
    QHBoxLayout *horizontalLayout_20;
    QLabel *label_13;
    QComboBox *combo_visao;
    QLabel *label_14;
    QDoubleSpinBox *spin_raio_visao_escuro;
    QWidget *horizontalLayoutWidget_9;
    QHBoxLayout *horizontalLayout_21;
    QLabel *label_16;
    QComboBox *combo_modelos_3d;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_24;
    QLabel *label_15;
    QComboBox *combo_textura;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_22;
    QLabel *label_17;
    QDoubleSpinBox *spin_tex_altura;
    QHBoxLayout *horizontalLayout_27;
    QLabel *label_20;
    QDoubleSpinBox *spin_tex_trans_y;
    QHBoxLayout *horizontalLayout_26;
    QLabel *label_19;
    QDoubleSpinBox *spin_tex_trans_x;
    QHBoxLayout *horizontalLayout_25;
    QHBoxLayout *horizontalLayout_23;
    QLabel *label_18;
    QDoubleSpinBox *spin_tex_largura;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QString::fromUtf8("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(904, 522);
        ifg__qt__DialogoEntidade->setStyleSheet(QString::fromUtf8(""));
        ifg__qt__DialogoEntidade->setModal(true);
        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(480, 460, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(100, 10, 181, 41));
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

        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(410, 230, 421, 41));
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

        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(410, 270, 421, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_14 = new QHBoxLayout();
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        label_12 = new QLabel(horizontalLayoutWidget_3);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_14->addWidget(label_12);

        spin_raio = new QDoubleSpinBox(horizontalLayoutWidget_3);
        spin_raio->setObjectName(QString::fromUtf8("spin_raio"));
        spin_raio->setDecimals(1);
        spin_raio->setSingleStep(0.5);

        horizontalLayout_14->addWidget(spin_raio);

        botao_luz = new QPushButton(horizontalLayoutWidget_3);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_14->addWidget(botao_luz);


        horizontalLayout_3->addLayout(horizontalLayout_14);

        horizontalLayoutWidget_4 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(20, 230, 361, 41));
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

        horizontalLayoutWidget_7 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_7->setObjectName(QString::fromUtf8("horizontalLayoutWidget_7"));
        horizontalLayoutWidget_7->setGeometry(QRect(20, 310, 361, 41));
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

        horizontalLayoutWidget_8 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_8->setObjectName(QString::fromUtf8("horizontalLayoutWidget_8"));
        horizontalLayoutWidget_8->setGeometry(QRect(20, 270, 361, 41));
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


        horizontalLayout_11->addLayout(horizontalLayout_12);

        layoutWidget = new QWidget(ifg__qt__DialogoEntidade);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(20, 350, 361, 39));
        horizontalLayout_9 = new QHBoxLayout(layoutWidget);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, 0, 0, 0);
        checkbox_caida = new QCheckBox(layoutWidget);
        checkbox_caida->setObjectName(QString::fromUtf8("checkbox_caida"));

        horizontalLayout_9->addWidget(checkbox_caida);

        checkbox_morta = new QCheckBox(layoutWidget);
        checkbox_morta->setObjectName(QString::fromUtf8("checkbox_morta"));

        horizontalLayout_9->addWidget(checkbox_morta);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_4 = new QLabel(layoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_4);

        spin_aura = new QDoubleSpinBox(layoutWidget);
        spin_aura->setObjectName(QString::fromUtf8("spin_aura"));
        spin_aura->setDecimals(1);
        spin_aura->setSingleStep(1.5);

        horizontalLayout_10->addWidget(spin_aura);


        horizontalLayout_9->addLayout(horizontalLayout_10);

        horizontalLayoutWidget_6 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_6->setObjectName(QString::fromUtf8("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(20, 430, 361, 31));
        horizontalLayout_13 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(horizontalLayoutWidget_6);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_13->addWidget(label_7);

        spin_translacao = new QDoubleSpinBox(horizontalLayoutWidget_6);
        spin_translacao->setObjectName(QString::fromUtf8("spin_translacao"));
        spin_translacao->setDecimals(1);
        spin_translacao->setMinimum(-100);
        spin_translacao->setMaximum(100);
        spin_translacao->setSingleStep(0.1);

        horizontalLayout_13->addWidget(spin_translacao);

        horizontalLayoutWidget_10 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_10->setObjectName(QString::fromUtf8("horizontalLayoutWidget_10"));
        horizontalLayoutWidget_10->setGeometry(QRect(20, 460, 361, 35));
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

        verticalLayoutWidget = new QWidget(ifg__qt__DialogoEntidade);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(20, 70, 361, 161));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));

        horizontalLayout_15->addLayout(horizontalLayout_17);

        label_8 = new QLabel(verticalLayoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        sizePolicy2.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy2);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_15->addWidget(label_8);

        campo_rotulo = new QLineEdit(verticalLayoutWidget);
        campo_rotulo->setObjectName(QString::fromUtf8("campo_rotulo"));
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

        layoutWidget_2 = new QWidget(ifg__qt__DialogoEntidade);
        layoutWidget_2->setObjectName(QString::fromUtf8("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(410, 80, 421, 141));
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

        horizontalLayoutWidget_11 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_11->setObjectName(QString::fromUtf8("horizontalLayoutWidget_11"));
        horizontalLayoutWidget_11->setGeometry(QRect(20, 400, 361, 35));
        horizontalLayout_20 = new QHBoxLayout(horizontalLayoutWidget_11);
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        horizontalLayout_20->setContentsMargins(0, 0, 0, 0);
        label_13 = new QLabel(horizontalLayoutWidget_11);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_13);

        combo_visao = new QComboBox(horizontalLayoutWidget_11);
        combo_visao->setObjectName(QString::fromUtf8("combo_visao"));

        horizontalLayout_20->addWidget(combo_visao);

        label_14 = new QLabel(horizontalLayoutWidget_11);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_20->addWidget(label_14);

        spin_raio_visao_escuro = new QDoubleSpinBox(horizontalLayoutWidget_11);
        spin_raio_visao_escuro->setObjectName(QString::fromUtf8("spin_raio_visao_escuro"));
        spin_raio_visao_escuro->setDecimals(1);
        spin_raio_visao_escuro->setSingleStep(1.5);

        horizontalLayout_20->addWidget(spin_raio_visao_escuro);

        horizontalLayoutWidget_9 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_9->setObjectName(QString::fromUtf8("horizontalLayoutWidget_9"));
        horizontalLayoutWidget_9->setGeometry(QRect(410, 390, 421, 41));
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

        widget = new QWidget(ifg__qt__DialogoEntidade);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(410, 310, 141, 78));
        horizontalLayout_24 = new QHBoxLayout(widget);
        horizontalLayout_24->setObjectName(QString::fromUtf8("horizontalLayout_24"));
        horizontalLayout_24->setContentsMargins(0, 0, 0, 0);
        label_15 = new QLabel(widget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_24->addWidget(label_15);

        combo_textura = new QComboBox(widget);
        combo_textura->setObjectName(QString::fromUtf8("combo_textura"));

        horizontalLayout_24->addWidget(combo_textura);

        gridLayoutWidget = new QWidget(ifg__qt__DialogoEntidade);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(550, 310, 281, 81));
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


        gridLayout->addLayout(horizontalLayout_27, 0, 1, 1, 1);

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


        horizontalLayout_25->addLayout(horizontalLayout_23);


        gridLayout->addLayout(horizontalLayout_25, 1, 0, 1, 1);


        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        ifg__qt__DialogoEntidade->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Id", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Cor:", 0, QApplication::UnicodeUTF8));
        checkbox_cor->setText(QString());
#ifndef QT_NO_TOOLTIP
        botao_cor->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Cor da entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Raio", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_raio->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da luz, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor da Luz", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", 0, QApplication::UnicodeUTF8));
        label_tamanho->setText(QApplication::translate("ifg::qt::DialogoEntidade", "(m\303\251dio)", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        slider_tamanho->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho da entidade", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
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
        label_5->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de Vida:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de vida para entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_6->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Max", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_max_pontos_vida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "M\303\241ximo de pontos de vida para entidade.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_caida->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marco, entidade cair\303\241.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_caida->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Ca\303\255da", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        checkbox_morta->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Se marcado, a entidade estar\303\241 morta.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_morta->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Morta", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Aura:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_aura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_7->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Deslocamento Vertical", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_translacao->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Para colocar a entidade acima do plano do tabuleiro.", 0, QApplication::UnicodeUTF8));
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
        label_8->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulo", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos Especial", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_rotulos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "R\303\263tulos para a entidade. Aparece sobre ela quando a entidade \303\251 detalhada.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_11->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Lista de Eventos", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        lista_eventos->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Cada linha deve conter um evento com formato <descricao:rodadas> sem as <>.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tipo de Vis\303\243o", 0, QApplication::UnicodeUTF8));
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
        spin_raio_visao_escuro->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_16->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Modelo 3D", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Textura", 0, QApplication::UnicodeUTF8));
        label_17->setText(QApplication::translate("ifg::qt::DialogoEntidade", "altura", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_altura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_20->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans y", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_y->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_19->setText(QApplication::translate("ifg::qt::DialogoEntidade", "trans x", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_trans_x->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_18->setText(QApplication::translate("ifg::qt::DialogoEntidade", "largura", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        spin_tex_largura->setToolTip(QApplication::translate("ifg::qt::DialogoEntidade", "Raio da vis\303\243o no escuro, em metros.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
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
