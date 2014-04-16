/********************************************************************************
** Form generated from reading UI file 'opcoes.ui'
**
** Created: Wed Apr 16 17:01:02 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OPCOES_H
#define OPCOES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>

namespace ifg {
namespace qt {

class Ui_DialogoOpcoes
{
public:
    QDialogButtonBox *buttonBox;
    QCheckBox *checkbox_mostrar_fps;
    QCheckBox *checkbox_texturas_sempre_de_frente;
    QCheckBox *checkbox_iluminacao_mestre;
    QCheckBox *checkbox_rosa_dos_ventos;

    void setupUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        if (ifg__qt__DialogoOpcoes->objectName().isEmpty())
            ifg__qt__DialogoOpcoes->setObjectName(QString::fromUtf8("ifg__qt__DialogoOpcoes"));
        ifg__qt__DialogoOpcoes->resize(400, 300);
        buttonBox = new QDialogButtonBox(ifg__qt__DialogoOpcoes);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        checkbox_mostrar_fps = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mostrar_fps->setObjectName(QString::fromUtf8("checkbox_mostrar_fps"));
        checkbox_mostrar_fps->setGeometry(QRect(10, 20, 271, 22));
        checkbox_texturas_sempre_de_frente = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_texturas_sempre_de_frente->setObjectName(QString::fromUtf8("checkbox_texturas_sempre_de_frente"));
        checkbox_texturas_sempre_de_frente->setGeometry(QRect(10, 50, 381, 22));
        checkbox_iluminacao_mestre = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_iluminacao_mestre->setObjectName(QString::fromUtf8("checkbox_iluminacao_mestre"));
        checkbox_iluminacao_mestre->setGeometry(QRect(10, 80, 381, 22));
        checkbox_rosa_dos_ventos = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_rosa_dos_ventos->setObjectName(QString::fromUtf8("checkbox_rosa_dos_ventos"));
        checkbox_rosa_dos_ventos->setGeometry(QRect(10, 110, 271, 22));

        retranslateUi(ifg__qt__DialogoOpcoes);
        QObject::connect(buttonBox, SIGNAL(accepted()), ifg__qt__DialogoOpcoes, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ifg__qt__DialogoOpcoes, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoOpcoes);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        ifg__qt__DialogoOpcoes->setWindowTitle(QApplication::translate("ifg::qt::DialogoOpcoes", "Dialog", 0, QApplication::UnicodeUTF8));
        checkbox_mostrar_fps->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Mostrar Tempo de Desenho da cena", 0, QApplication::UnicodeUTF8));
        checkbox_texturas_sempre_de_frente->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Texturas de Entidades Sempre de frente para c\303\242mera", 0, QApplication::UnicodeUTF8));
        checkbox_iluminacao_mestre->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Ilumina\303\247\303\243o do mestre igual \303\240 dos jogadores", 0, QApplication::UnicodeUTF8));
        checkbox_rosa_dos_ventos->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Desenha rosa dos ventos", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoOpcoes: public Ui_DialogoOpcoes {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // OPCOES_H
