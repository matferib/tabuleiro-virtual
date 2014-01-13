/********************************************************************************
** Form generated from reading UI file 'iluminacaoQ10049.ui'
**
** Created: Mon Jan 13 00:30:39 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ILUMINACAOQ10049_H
#define ILUMINACAOQ10049_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
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
    QPushButton *botao_cor;
    QDial *dial_inclinacao;
    QDial *dial_posicao;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *label_2;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *label_9;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *linha_textura;
    QPushButton *botao_textura;

    void setupUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        if (ifg__qt__DialogoIluminacao->objectName().isEmpty())
            ifg__qt__DialogoIluminacao->setObjectName(QString::fromUtf8("ifg__qt__DialogoIluminacao"));
        ifg__qt__DialogoIluminacao->resize(456, 446);
        botoes = new QDialogButtonBox(ifg__qt__DialogoIluminacao);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(70, 390, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        botao_cor = new QPushButton(ifg__qt__DialogoIluminacao);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));
        botao_cor->setGeometry(QRect(140, 30, 176, 27));
        dial_inclinacao = new QDial(ifg__qt__DialogoIluminacao);
        dial_inclinacao->setObjectName(QString::fromUtf8("dial_inclinacao"));
        dial_inclinacao->setGeometry(QRect(60, 120, 121, 121));
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
        dial_posicao = new QDial(ifg__qt__DialogoIluminacao);
        dial_posicao->setObjectName(QString::fromUtf8("dial_posicao"));
        dial_posicao->setGeometry(QRect(290, 120, 121, 121));
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
        label_3 = new QLabel(ifg__qt__DialogoIluminacao);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(330, 240, 41, 21));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignCenter);
        label_4 = new QLabel(ifg__qt__DialogoIluminacao);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(330, 100, 41, 21));
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(ifg__qt__DialogoIluminacao);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(270, 160, 20, 41));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setAlignment(Qt::AlignCenter);
        label_6 = new QLabel(ifg__qt__DialogoIluminacao);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(410, 160, 20, 41));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignCenter);
        label_2 = new QLabel(ifg__qt__DialogoIluminacao);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 170, 64, 17));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_7 = new QLabel(ifg__qt__DialogoIluminacao);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(20, 170, 50, 17));
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);
        label_8 = new QLabel(ifg__qt__DialogoIluminacao);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(90, 240, 78, 17));
        sizePolicy1.setHeightForWidth(label_8->sizePolicy().hasHeightForWidth());
        label_8->setSizePolicy(sizePolicy1);
        label_9 = new QLabel(ifg__qt__DialogoIluminacao);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(100, 100, 43, 17));
        sizePolicy1.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy1);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoIluminacao);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(30, 290, 391, 80));
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


        retranslateUi(ifg__qt__DialogoIluminacao);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoIluminacao, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoIluminacao, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoIluminacao);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        ifg__qt__DialogoIluminacao->setWindowTitle(QApplication::translate("ifg::qt::DialogoIluminacao", "Ilumina\303\247\303\243o Ambiente", 0, QApplication::UnicodeUTF8));
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Escolher Cor", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        dial_inclinacao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Inclina\303\247\303\243o da Luz", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        dial_posicao->setToolTip(QApplication::translate("ifg::qt::DialogoIluminacao", "Posi\303\247\303\243o da fonte de luz", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "S", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "N", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "W", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "E", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Nascente", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Poente", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Abaixo solo", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "A pino", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Fundo", 0, QApplication::UnicodeUTF8));
        botao_textura->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Escolher", 0, QApplication::UnicodeUTF8));
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

#endif // ILUMINACAOQ10049_H
