/********************************************************************************
** Form generated from reading UI file 'cenario.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef CENARIO_H
#define CENARIO_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

namespace ifg {
namespace qt {

class Ui_DialogoIluminacao
{
public:
    QDialogButtonBox *botoes;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *combo_fundo;
    QCheckBox *checkbox_ladrilho;
    QCheckBox *checkbox_mestre_apenas;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_11;
    QLineEdit *linha_largura;
    QLabel *label_10;
    QLineEdit *linha_altura;
    QCheckBox *checkbox_tamanho_automatico;
    QCheckBox *checkbox_grade;
    QPushButton *botao_cor_ambiente;
    QGroupBox *groupBox;
    QPushButton *botao_cor_direcional;
    QDial *dial_inclinacao;
    QLabel *label_3;
    QDial *dial_posicao;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_9;
    QLabel *label_8;
    QLabel *label_2;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_nevoa;
    QLabel *label_12;
    QLineEdit *linha_nevoa_min;
    QLabel *label_13;
    QLineEdit *linha_nevoa_max;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_14;
    QComboBox *combo_ceu;
    QCheckBox *checkbox_luz_ceu;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_15;
    QLineEdit *campo_id;
    QLineEdit *campo_descricao;
    QLabel *label_16;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_17;
    QComboBox *combo_id_cenario;
    QPushButton *botao_clonar;

    void setupUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        if (ifg__qt__DialogoIluminacao->objectName().isEmpty())
            ifg__qt__DialogoIluminacao->setObjectName(QStringLiteral("ifg__qt__DialogoIluminacao"));
        ifg__qt__DialogoIluminacao->resize(517, 704);
        botoes = new QDialogButtonBox(ifg__qt__DialogoIluminacao);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setGeometry(QRect(130, 660, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(20, 400, 461, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout->addWidget(label);

        combo_fundo = new QComboBox(horizontalLayoutWidget);
        combo_fundo->setObjectName(QStringLiteral("combo_fundo"));

        horizontalLayout->addWidget(combo_fundo);

        checkbox_ladrilho = new QCheckBox(horizontalLayoutWidget);
        checkbox_ladrilho->setObjectName(QStringLiteral("checkbox_ladrilho"));

        horizontalLayout->addWidget(checkbox_ladrilho);

        checkbox_mestre_apenas = new QCheckBox(horizontalLayoutWidget);
        checkbox_mestre_apenas->setObjectName(QStringLiteral("checkbox_mestre_apenas"));

        horizontalLayout->addWidget(checkbox_mestre_apenas);

        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_2->setObjectName(QStringLiteral("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(20, 560, 461, 80));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(horizontalLayoutWidget_2);
        label_11->setObjectName(QStringLiteral("label_11"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy1);
        label_11->setIndent(-1);

        horizontalLayout_2->addWidget(label_11);

        linha_largura = new QLineEdit(horizontalLayoutWidget_2);
        linha_largura->setObjectName(QStringLiteral("linha_largura"));
        sizePolicy1.setHeightForWidth(linha_largura->sizePolicy().hasHeightForWidth());
        linha_largura->setSizePolicy(sizePolicy1);
        linha_largura->setMaximumSize(QSize(50, 16777215));
        linha_largura->setInputMask(QStringLiteral("099"));

        horizontalLayout_2->addWidget(linha_largura);

        label_10 = new QLabel(horizontalLayoutWidget_2);
        label_10->setObjectName(QStringLiteral("label_10"));
        sizePolicy1.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(label_10);

        linha_altura = new QLineEdit(horizontalLayoutWidget_2);
        linha_altura->setObjectName(QStringLiteral("linha_altura"));
        sizePolicy1.setHeightForWidth(linha_altura->sizePolicy().hasHeightForWidth());
        linha_altura->setSizePolicy(sizePolicy1);
        linha_altura->setMaximumSize(QSize(50, 16777215));
        linha_altura->setInputMask(QStringLiteral("099"));

        horizontalLayout_2->addWidget(linha_altura);

        checkbox_tamanho_automatico = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_tamanho_automatico->setObjectName(QStringLiteral("checkbox_tamanho_automatico"));

        horizontalLayout_2->addWidget(checkbox_tamanho_automatico);

        checkbox_grade = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_grade->setObjectName(QStringLiteral("checkbox_grade"));

        horizontalLayout_2->addWidget(checkbox_grade);

        botao_cor_ambiente = new QPushButton(ifg__qt__DialogoIluminacao);
        botao_cor_ambiente->setObjectName(QStringLiteral("botao_cor_ambiente"));
        botao_cor_ambiente->setGeometry(QRect(130, 70, 176, 27));
        groupBox = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(20, 100, 421, 221));
        botao_cor_direcional = new QPushButton(groupBox);
        botao_cor_direcional->setObjectName(QStringLiteral("botao_cor_direcional"));
        botao_cor_direcional->setGeometry(QRect(110, 20, 176, 27));
        dial_inclinacao = new QDial(groupBox);
        dial_inclinacao->setObjectName(QStringLiteral("dial_inclinacao"));
        dial_inclinacao->setGeometry(QRect(40, 70, 121, 121));
        dial_inclinacao->setMinimum(0);
        dial_inclinacao->setMaximum(360);
        dial_inclinacao->setValue(135);
        dial_inclinacao->setSliderPosition(135);
        dial_inclinacao->setOrientation(Qt::Horizontal);
        dial_inclinacao->setInvertedAppearance(true);
        dial_inclinacao->setInvertedControls(true);
        dial_inclinacao->setWrapping(true);
        dial_inclinacao->setNotchTarget(30);
        dial_inclinacao->setNotchesVisible(true);
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(300, 190, 41, 21));
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignCenter);
        dial_posicao = new QDial(groupBox);
        dial_posicao->setObjectName(QStringLiteral("dial_posicao"));
        dial_posicao->setGeometry(QRect(260, 70, 121, 121));
        dial_posicao->setMinimum(0);
        dial_posicao->setMaximum(360);
        dial_posicao->setValue(0);
        dial_posicao->setSliderPosition(0);
        dial_posicao->setOrientation(Qt::Horizontal);
        dial_posicao->setInvertedAppearance(true);
        dial_posicao->setInvertedControls(true);
        dial_posicao->setWrapping(true);
        dial_posicao->setNotchTarget(45);
        dial_posicao->setNotchesVisible(true);
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(300, 50, 41, 21));
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(240, 110, 20, 41));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setAlignment(Qt::AlignCenter);
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(380, 110, 20, 41));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignCenter);
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 120, 50, 17));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy2);
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(80, 50, 43, 17));
        sizePolicy2.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy2);
        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(70, 200, 78, 17));
        sizePolicy2.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy2);
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(170, 120, 64, 17));
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);
        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_3->setObjectName(QStringLiteral("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(20, 329, 221, 71));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkbox_nevoa = new QCheckBox(horizontalLayoutWidget_3);
        checkbox_nevoa->setObjectName(QStringLiteral("checkbox_nevoa"));
        checkbox_nevoa->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_3->addWidget(checkbox_nevoa);

        label_12 = new QLabel(horizontalLayoutWidget_3);
        label_12->setObjectName(QStringLiteral("label_12"));
        label_12->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(label_12);

        linha_nevoa_min = new QLineEdit(horizontalLayoutWidget_3);
        linha_nevoa_min->setObjectName(QStringLiteral("linha_nevoa_min"));
        linha_nevoa_min->setEnabled(false);
        linha_nevoa_min->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_3->addWidget(linha_nevoa_min);

        label_13 = new QLabel(horizontalLayoutWidget_3);
        label_13->setObjectName(QStringLiteral("label_13"));
        label_13->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(label_13);

        linha_nevoa_max = new QLineEdit(horizontalLayoutWidget_3);
        linha_nevoa_max->setObjectName(QStringLiteral("linha_nevoa_max"));
        linha_nevoa_max->setEnabled(false);
        linha_nevoa_max->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_3->addWidget(linha_nevoa_max);

        horizontalLayoutWidget_4 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_4->setObjectName(QStringLiteral("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(20, 480, 461, 80));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        label_14 = new QLabel(horizontalLayoutWidget_4);
        label_14->setObjectName(QStringLiteral("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label_14);

        combo_ceu = new QComboBox(horizontalLayoutWidget_4);
        combo_ceu->setObjectName(QStringLiteral("combo_ceu"));

        horizontalLayout_4->addWidget(combo_ceu);

        checkbox_luz_ceu = new QCheckBox(horizontalLayoutWidget_4);
        checkbox_luz_ceu->setObjectName(QStringLiteral("checkbox_luz_ceu"));

        horizontalLayout_4->addWidget(checkbox_luz_ceu);

        horizontalLayoutWidget_5 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_5->setObjectName(QStringLiteral("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(20, 20, 421, 41));
        horizontalLayout_5 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        label_15 = new QLabel(horizontalLayoutWidget_5);
        label_15->setObjectName(QStringLiteral("label_15"));

        horizontalLayout_5->addWidget(label_15, 0, Qt::AlignRight);

        campo_id = new QLineEdit(horizontalLayoutWidget_5);
        campo_id->setObjectName(QStringLiteral("campo_id"));
        campo_id->setMaximumSize(QSize(50, 16777215));
        campo_id->setReadOnly(true);

        horizontalLayout_5->addWidget(campo_id, 0, Qt::AlignLeft);

        campo_descricao = new QLineEdit(horizontalLayoutWidget_5);
        campo_descricao->setObjectName(QStringLiteral("campo_descricao"));
        campo_descricao->setReadOnly(false);

        horizontalLayout_5->addWidget(campo_descricao);

        label_16 = new QLabel(horizontalLayoutWidget_5);
        label_16->setObjectName(QStringLiteral("label_16"));

        horizontalLayout_5->addWidget(label_16);

        horizontalLayoutWidget_6 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_6->setObjectName(QStringLiteral("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(240, 330, 241, 71));
        horizontalLayout_6 = new QHBoxLayout(horizontalLayoutWidget_6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        label_17 = new QLabel(horizontalLayoutWidget_6);
        label_17->setObjectName(QStringLiteral("label_17"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_17->sizePolicy().hasHeightForWidth());
        label_17->setSizePolicy(sizePolicy3);

        horizontalLayout_6->addWidget(label_17);

        combo_id_cenario = new QComboBox(horizontalLayoutWidget_6);
        combo_id_cenario->setObjectName(QStringLiteral("combo_id_cenario"));

        horizontalLayout_6->addWidget(combo_id_cenario);

        botao_clonar = new QPushButton(horizontalLayoutWidget_6);
        botao_clonar->setObjectName(QStringLiteral("botao_clonar"));

        horizontalLayout_6->addWidget(botao_clonar);


        retranslateUi(ifg__qt__DialogoIluminacao);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoIluminacao, SLOT(reject()));
        QObject::connect(checkbox_nevoa, SIGNAL(clicked(bool)), linha_nevoa_min, SLOT(setEnabled(bool)));
        QObject::connect(checkbox_nevoa, SIGNAL(clicked(bool)), linha_nevoa_max, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoIluminacao);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        ifg__qt__DialogoIluminacao->setWindowTitle(QApplication::translate("ifg::qt::DialogoIluminacao", "Propriedades Tabuleiro", nullptr));
        label->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Fundo", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_ladrilho->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_ladrilho->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Ladrilho", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_mestre_apenas->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_mestre_apenas->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Mestre Apenas", nullptr));
        label_11->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Tamanho", nullptr));
#ifndef QT_NO_TOOLTIP
        linha_largura->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "largura", nullptr));
#endif // QT_NO_TOOLTIP
        label_10->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "x", nullptr));
#ifndef QT_NO_TOOLTIP
        linha_altura->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "altura", nullptr));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_tamanho_automatico->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_tamanho_automatico->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Autom\303\241tico", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_grade->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_grade->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Grade", nullptr));
        botao_cor_ambiente->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz  Ambiente", nullptr));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoIluminacao", "Luz Direcional", nullptr));
        botao_cor_direcional->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz Direcional", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_inclinacao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Inclina\303\247\303\243o da Luz", nullptr));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "S", nullptr));
#ifndef QT_NO_TOOLTIP
        dial_posicao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Posi\303\247\303\243o da fonte de luz", nullptr));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "N", nullptr));
        label_5->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "W", nullptr));
        label_6->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "E", nullptr));
        label_7->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Poente", nullptr));
        label_9->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "A pino", nullptr));
        label_8->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Abaixo solo", nullptr));
        label_2->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Nascente", nullptr));
        checkbox_nevoa->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "N\303\251voa", nullptr));
        label_12->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Min", nullptr));
#ifndef QT_NO_TOOLTIP
        linha_nevoa_min->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Dist\303\242ncia m\303\255nma da n\303\251voa, em m.", nullptr));
#endif // QT_NO_TOOLTIP
        label_13->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Max", nullptr));
#ifndef QT_NO_TOOLTIP
        linha_nevoa_max->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Dist\303\242ncia onde o efeito m\303\241ximo da n\303\251voa \303\251 aplicado.", nullptr));
#endif // QT_NO_TOOLTIP
        label_14->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "C\303\251u", nullptr));
#ifndef QT_NO_TOOLTIP
        checkbox_luz_ceu->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", nullptr));
#endif // QT_NO_TOOLTIP
        checkbox_luz_ceu->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Aplicar luz ambiente", nullptr));
        label_15->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Id", nullptr));
        campo_descricao->setPlaceholderText(QApplication::translate("ifg::qt::DialogoIluminacao", "Descri\303\247\303\243o do cen\303\241rio", nullptr));
        label_16->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Descri\303\247\303\243o", nullptr));
        label_17->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Clonar", nullptr));
        botao_clonar->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "OK", nullptr));
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
