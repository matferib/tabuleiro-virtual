/********************************************************************************
** Form generated from reading UI file 'listapaginada.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef LISTAPAGINADA_H
#define LISTAPAGINADA_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QListWidget>

namespace ifg {
namespace qt {

class Ui_ListaPaginada
{
public:
    QDialogButtonBox *botoes;
    QListWidget *lista;

    void setupUi(QDialog *ifg__qt__ListaPaginada)
    {
        if (ifg__qt__ListaPaginada->objectName().isEmpty())
            ifg__qt__ListaPaginada->setObjectName(QStringLiteral("ifg__qt__ListaPaginada"));
        ifg__qt__ListaPaginada->resize(400, 300);
        botoes = new QDialogButtonBox(ifg__qt__ListaPaginada);
        botoes->setObjectName(QStringLiteral("botoes"));
        botoes->setGeometry(QRect(30, 240, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        lista = new QListWidget(ifg__qt__ListaPaginada);
        lista->setObjectName(QStringLiteral("lista"));
        lista->setGeometry(QRect(20, 20, 351, 211));

        retranslateUi(ifg__qt__ListaPaginada);

        QMetaObject::connectSlotsByName(ifg__qt__ListaPaginada);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__ListaPaginada)
    {
        ifg__qt__ListaPaginada->setWindowTitle(QString());
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class ListaPaginada: public Ui_ListaPaginada {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // LISTAPAGINADA_H
