/********************************************************************************
** Form generated from reading UI file 'listapaginada.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef LISTAPAGINADA_H
#define LISTAPAGINADA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QListWidget>

namespace ifg {
namespace qt {

class Ui_ListaPaginada
{
public:
    QGridLayout *gridLayout;
    QListWidget *lista;
    QDialogButtonBox *botoes;

    void setupUi(QDialog *ifg__qt__ListaPaginada)
    {
        if (ifg__qt__ListaPaginada->objectName().isEmpty())
            ifg__qt__ListaPaginada->setObjectName(QString::fromUtf8("ifg__qt__ListaPaginada"));
        ifg__qt__ListaPaginada->resize(623, 312);
        gridLayout = new QGridLayout(ifg__qt__ListaPaginada);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lista = new QListWidget(ifg__qt__ListaPaginada);
        lista->setObjectName(QString::fromUtf8("lista"));

        gridLayout->addWidget(lista, 0, 0, 1, 1);

        botoes = new QDialogButtonBox(ifg__qt__ListaPaginada);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(botoes, 1, 0, 1, 1);


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
