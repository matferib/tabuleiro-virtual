/********************************************************************************
** Form generated from reading UI file 'entradastring.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTRADASTRING_H
#define ENTRADASTRING_H

#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QVariant>

namespace ifg {
namespace qt {

class Ui_EntradaString
{
public:
    QDialogButtonBox *botoes;
    QLineEdit *nome;
    QLabel *label_titulo;

    void setupUi(QDialog *ifg__qt__EntradaString)
    {
        if (ifg__qt__EntradaString->objectName().isEmpty())
            ifg__qt__EntradaString->setObjectName(QString::fromUtf8("ifg__qt__EntradaString"));
        ifg__qt__EntradaString->resize(317, 118);
        botoes = new QDialogButtonBox(ifg__qt__EntradaString);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(30, 70, 221, 41));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        nome = new QLineEdit(ifg__qt__EntradaString);
        nome->setObjectName(QString::fromUtf8("nome"));
        nome->setGeometry(QRect(10, 30, 291, 27));
        label_titulo = new QLabel(ifg__qt__EntradaString);
        label_titulo->setObjectName(QString::fromUtf8("label_titulo"));
        label_titulo->setGeometry(QRect(16, 10, 281, 20));
        label_titulo->setAlignment(Qt::AlignCenter);

        retranslateUi(ifg__qt__EntradaString);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__EntradaString, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__EntradaString, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__EntradaString);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__EntradaString)
    {
        ifg__qt__EntradaString->setWindowTitle(QApplication::translate("ifg::qt::EntradaString", "Dialog", 0));
        label_titulo->setText(
            QApplication::translate("ifg::qt::EntradaString", "TextLabel", 0));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class EntradaString: public Ui_EntradaString {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // ENTRADASTRING_H
