/********************************************************************************
** Form generated from reading UI file 'dialogo_bonus.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DIALOGOBONUS_H
#define DIALOGOBONUS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTableView>

namespace ifg {
namespace qt {

class Ui_DialogoBonus
{
public:
    QDialogButtonBox *botoes_ok_cancela;
    QPushButton *botao_adicionar_bonus;
    QPushButton *botao_remover_bonus;
    QTableView *tabela_bonus;

    void setupUi(QDialog *ifg__qt__DialogoBonus)
    {
        if (ifg__qt__DialogoBonus->objectName().isEmpty())
            ifg__qt__DialogoBonus->setObjectName(QString::fromUtf8("ifg__qt__DialogoBonus"));
        ifg__qt__DialogoBonus->resize(564, 371);
        botoes_ok_cancela = new QDialogButtonBox(ifg__qt__DialogoBonus);
        botoes_ok_cancela->setObjectName(QString::fromUtf8("botoes_ok_cancela"));
        botoes_ok_cancela->setGeometry(QRect(270, 320, 221, 41));
        botoes_ok_cancela->setOrientation(Qt::Horizontal);
        botoes_ok_cancela->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        botao_adicionar_bonus = new QPushButton(ifg__qt__DialogoBonus);
        botao_adicionar_bonus->setObjectName(QString::fromUtf8("botao_adicionar_bonus"));
        botao_adicionar_bonus->setGeometry(QRect(500, 120, 31, 27));
        botao_adicionar_bonus->setStyleSheet(QString::fromUtf8("color: green;\n"
"font: bold;"));
        botao_remover_bonus = new QPushButton(ifg__qt__DialogoBonus);
        botao_remover_bonus->setObjectName(QString::fromUtf8("botao_remover_bonus"));
        botao_remover_bonus->setGeometry(QRect(500, 150, 31, 27));
        botao_remover_bonus->setStyleSheet(QString::fromUtf8("color: red;\n"
"font: bold;"));
        tabela_bonus = new QTableView(ifg__qt__DialogoBonus);
        tabela_bonus->setObjectName(QString::fromUtf8("tabela_bonus"));
        tabela_bonus->setGeometry(QRect(10, 20, 481, 291));

        retranslateUi(ifg__qt__DialogoBonus);
        QObject::connect(botoes_ok_cancela, SIGNAL(accepted()), ifg__qt__DialogoBonus, SLOT(accept()));
        QObject::connect(botoes_ok_cancela, SIGNAL(rejected()), ifg__qt__DialogoBonus, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoBonus);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoBonus)
    {
        ifg__qt__DialogoBonus->setWindowTitle(QApplication::translate("ifg::qt::DialogoBonus", "Dialog", 0, QApplication::UnicodeUTF8));
        botao_adicionar_bonus->setText(QApplication::translate("ifg::qt::DialogoBonus", "+", 0, QApplication::UnicodeUTF8));
        botao_remover_bonus->setText(QApplication::translate("ifg::qt::DialogoBonus", "-", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoBonus: public Ui_DialogoBonus {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // DIALOGOBONUS_H