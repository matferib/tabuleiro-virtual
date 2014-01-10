/********************************************************************************
** Form generated from reading UI file 'iluminacaoJ25115.ui'
**
** Created: Thu Jan 9 23:32:37 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ILUMINACAOJ25115_H
#define ILUMINACAOJ25115_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

namespace ifg {
namespace qt {

class Ui_DialogoIluminacao
{
public:
    QDialogButtonBox *botoes;
    QPushButton *botao_cor;
    QDial *dial_inclinacao;
    QDial *dial_posicao;
    QLabel *label;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_6;

    void setupUi(QDialog *ifg__qt__DialogoIluminacao)
    {
        if (ifg__qt__DialogoIluminacao->objectName().isEmpty())
            ifg__qt__DialogoIluminacao->setObjectName(QString::fromUtf8("ifg__qt__DialogoIluminacao"));
        ifg__qt__DialogoIluminacao->resize(428, 343);
        botoes = new QDialogButtonBox(ifg__qt__DialogoIluminacao);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(50, 290, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        botao_cor = new QPushButton(ifg__qt__DialogoIluminacao);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));
        botao_cor->setGeometry(QRect(140, 30, 176, 27));
        dial_inclinacao = new QDial(ifg__qt__DialogoIluminacao);
        dial_inclinacao->setObjectName(QString::fromUtf8("dial_inclinacao"));
        dial_inclinacao->setGeometry(QRect(30, 120, 121, 121));
        dial_inclinacao->setMinimum(0);
        dial_inclinacao->setMaximum(360);
        dial_inclinacao->setValue(180);
        dial_inclinacao->setSliderPosition(180);
        dial_inclinacao->setOrientation(Qt::Horizontal);
        dial_inclinacao->setInvertedAppearance(true);
        dial_inclinacao->setInvertedControls(false);
        dial_inclinacao->setWrapping(true);
        dial_inclinacao->setNotchesVisible(true);
        dial_posicao = new QDial(ifg__qt__DialogoIluminacao);
        dial_posicao->setObjectName(QString::fromUtf8("dial_posicao"));
        dial_posicao->setGeometry(QRect(260, 120, 121, 121));
        dial_posicao->setMinimum(0);
        dial_posicao->setMaximum(360);
        dial_posicao->setValue(180);
        dial_posicao->setSliderPosition(180);
        dial_posicao->setOrientation(Qt::Horizontal);
        dial_posicao->setInvertedAppearance(true);
        dial_posicao->setInvertedControls(false);
        dial_posicao->setWrapping(true);
        dial_posicao->setNotchTarget(45);
        dial_posicao->setNotchesVisible(true);
        label = new QLabel(ifg__qt__DialogoIluminacao);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(60, 100, 66, 17));
        label_3 = new QLabel(ifg__qt__DialogoIluminacao);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(300, 240, 41, 21));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignCenter);
        label_4 = new QLabel(ifg__qt__DialogoIluminacao);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(300, 100, 41, 21));
        sizePolicy.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy);
        label_4->setAlignment(Qt::AlignCenter);
        label_5 = new QLabel(ifg__qt__DialogoIluminacao);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(240, 160, 20, 41));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setAlignment(Qt::AlignCenter);
        label_6 = new QLabel(ifg__qt__DialogoIluminacao);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(380, 160, 20, 41));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignCenter);

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
        label->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "Inclina\303\247\303\243o", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "S", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "N", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "W", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ifg::qt::DialogoIluminacao", "E", 0, QApplication::UnicodeUTF8));
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

#endif // ILUMINACAOJ25115_H
