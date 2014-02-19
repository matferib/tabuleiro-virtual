/********************************************************************************
** Form generated from reading UI file 'iluminacao.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ILUMINACAO_H
#define ILUMINACAO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

namespace ifg {
namespace qt {

class Ui_DialogoIluminacao
{
public:
    QDialogButtonBox *botoes;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *linha_textura;
    QPushButton *botao_textura;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_11;
    QLineEdit *linha_largura;
    QLabel *label_10;
    QLineEdit *linha_altura;
    QCheckBox *checkbox_tamanho_automatico;
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

    void setupUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        if (ifg__qt__DialogoIluminacao->objectName().isEmpty())
            ifg__qt__DialogoIluminacao->setObjectName(QString::fromUtf8("ifg__qt__DialogoIluminacao"));
        ifg__qt__DialogoIluminacao->resize(456, 578);
        botoes = new QDialogButtonBox(ifg__qt__DialogoIluminacao);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(70, 510, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(20, 310, 411, 80));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        linha_textura = new QLineEdit(horizontalLayoutWidget);
        linha_textura->setObjectName(QString::fromUtf8("linha_textura"));

        horizontalLayout->addWidget(linha_textura);

        botao_textura = new QPushButton(horizontalLayoutWidget);
        botao_textura->setObjectName(QString::fromUtf8("botao_textura"));

        horizontalLayout->addWidget(botao_textura);

        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(20, 390, 411, 80));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_11 = new QLabel(horizontalLayoutWidget_2);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_11->sizePolicy().hasHeightForWidth());
        label_11->setSizePolicy(sizePolicy);
        label_11->setIndent(-1);

        horizontalLayout_2->addWidget(label_11);

        linha_largura = new QLineEdit(horizontalLayoutWidget_2);
        linha_largura->setObjectName(QString::fromUtf8("linha_largura"));
        sizePolicy.setHeightForWidth(linha_largura->sizePolicy().hasHeightForWidth());
        linha_largura->setSizePolicy(sizePolicy);
        linha_largura->setInputMask(QString::fromUtf8("099; "));

        horizontalLayout_2->addWidget(linha_largura);

        label_10 = new QLabel(horizontalLayoutWidget_2);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        sizePolicy.setHeightForWidth(label_10->sizePolicy().hasHeightForWidth());
        label_10->setSizePolicy(sizePolicy);

        horizontalLayout_2->addWidget(label_10);

        linha_altura = new QLineEdit(horizontalLayoutWidget_2);
        linha_altura->setObjectName(QString::fromUtf8("linha_altura"));
        sizePolicy.setHeightForWidth(linha_altura->sizePolicy().hasHeightForWidth());
        linha_altura->setSizePolicy(sizePolicy);
        linha_altura->setInputMask(QString::fromUtf8("099; "));

        horizontalLayout_2->addWidget(linha_altura);

        checkbox_tamanho_automatico = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_tamanho_automatico->setObjectName(QString::fromUtf8("checkbox_tamanho_automatico"));

        horizontalLayout_2->addWidget(checkbox_tamanho_automatico);

        botao_cor_ambiente = new QPushButton(ifg__qt__DialogoIluminacao);
        botao_cor_ambiente->setObjectName(QString::fromUtf8("botao_cor_ambiente"));
        botao_cor_ambiente->setGeometry(QRect(130, 20, 176, 27));
        groupBox = new QGroupBox(ifg__qt__DialogoIluminacao);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(20, 70, 421, 221));
        botao_cor_direcional = new QPushButton(groupBox);
        botao_cor_direcional->setObjectName(QString::fromUtf8("botao_cor_direcional"));
        botao_cor_direcional->setGeometry(QRect(110, 20, 176, 27));
        dial_inclinacao = new QDial(groupBox);
        dial_inclinacao->setObjectName(QString::fromUtf8("dial_inclinacao"));
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
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(300, 190, 41, 21));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy1);
        label_3->setAlignment(Qt::AlignCenter);
        dial_posicao = new QDial(groupBox);
        dial_posicao->setObjectName(QString::fromUtf8("dial_posicao"));
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
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(300, 50, 41, 21));
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(240, 110, 20, 41));
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);
        label_5->setAlignment(Qt::AlignCenter);
        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(380, 110, 20, 41));
        sizePolicy1.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy1);
        label_6->setAlignment(Qt::AlignCenter);
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 120, 50, 17));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy2);
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(80, 50, 43, 17));
        sizePolicy2.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy2);
        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(70, 200, 78, 17));
        sizePolicy2.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy2);
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(170, 120, 64, 17));
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);

        retranslateUi(ifg__qt__DialogoIluminacao);
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoIluminacao, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoIluminacao);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        ifg__qt__DialogoIluminacao->setWindowTitle(QApplication::translate("ifg::qt::DialogoIluminacao", "Ilumina\303\247\303\243o Ambiente", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Fundo", 0, QApplication::UnicodeUTF8));
        botao_textura->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Escolher", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Tamanho", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_largura->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "largura", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_10->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "x", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        linha_altura->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "altura", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        checkbox_tamanho_automatico->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Se marcado, o tamanho do tabuleiro ser\303\241 computado a partir do tamanho da textura.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        checkbox_tamanho_automatico->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Autom\303\241tico", 0, QApplication::UnicodeUTF8));
        botao_cor_ambiente->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz  Ambiente", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoIluminacao", "Luz Direcional", 0, QApplication::UnicodeUTF8));
        botao_cor_direcional->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Cor da Luz Direcional", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_inclinacao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Inclina\303\247\303\243o da Luz", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "S", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_posicao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Posi\303\247\303\243o da fonte de luz", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "N", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "W", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "E", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Poente", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "A pino", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Abaixo solo", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Nascente", 0, QApplication::UnicodeUTF8));
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

#endif // ILUMINACAO_H
