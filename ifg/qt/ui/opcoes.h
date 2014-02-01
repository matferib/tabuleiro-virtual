/********************************************************************************
** Form generated from reading UI file 'opcoes.ui'
**
** Created
**      by: Qt User Interface Compiler version 4.8.4
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

        retranslateUi(ifg__qt__DialogoOpcoes);
        QObject::connect(buttonBox, SIGNAL(accepted()), ifg__qt__DialogoOpcoes, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ifg__qt__DialogoOpcoes, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoOpcoes);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        ifg__qt__DialogoOpcoes->setWindowTitle(QApplication::translate("ifg::qt::DialogoOpcoes", "Dialog", 0, QApplication::UnicodeUTF8));
        checkbox_mostrar_fps->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Mostrar Tempo de Desenho da cena", 0, QApplication::UnicodeUTF8));
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
