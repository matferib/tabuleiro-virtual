/********************************************************************************
** Form generated from reading UI file 'dialogo_bonus.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DIALOGOBONUS_H
#define DIALOGOBONUS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>

namespace ifg {
namespace qt {

class Ui_DialogoBonus
{
public:
    QGridLayout *gridLayout;
    QTableView *tabela_bonus;
    QVBoxLayout *verticalLayout;
    QPushButton *botao_adicionar_bonus;
    QPushButton *botao_remover_bonus;
    QDialogButtonBox *botoes_ok_cancela;

    void setupUi(QDialog *ifg__qt__DialogoBonus)
    {
        if (ifg__qt__DialogoBonus->objectName().isEmpty())
            ifg__qt__DialogoBonus->setObjectName(QString::fromUtf8("ifg__qt__DialogoBonus"));
        ifg__qt__DialogoBonus->resize(564, 371);
        gridLayout = new QGridLayout(ifg__qt__DialogoBonus);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        tabela_bonus = new QTableView(ifg__qt__DialogoBonus);
        tabela_bonus->setObjectName(QString::fromUtf8("tabela_bonus"));

        gridLayout->addWidget(tabela_bonus, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        botao_adicionar_bonus = new QPushButton(ifg__qt__DialogoBonus);
        botao_adicionar_bonus->setObjectName(QString::fromUtf8("botao_adicionar_bonus"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(botao_adicionar_bonus->sizePolicy().hasHeightForWidth());
        botao_adicionar_bonus->setSizePolicy(sizePolicy);
        botao_adicionar_bonus->setStyleSheet(QString::fromUtf8("color: green;\n"
"font: bold;"));

        verticalLayout->addWidget(botao_adicionar_bonus);

        botao_remover_bonus = new QPushButton(ifg__qt__DialogoBonus);
        botao_remover_bonus->setObjectName(QString::fromUtf8("botao_remover_bonus"));
        sizePolicy.setHeightForWidth(botao_remover_bonus->sizePolicy().hasHeightForWidth());
        botao_remover_bonus->setSizePolicy(sizePolicy);
        botao_remover_bonus->setStyleSheet(QString::fromUtf8("color: red;\n"
"font: bold;"));

        verticalLayout->addWidget(botao_remover_bonus);


        gridLayout->addLayout(verticalLayout, 0, 1, 1, 1);

        botoes_ok_cancela = new QDialogButtonBox(ifg__qt__DialogoBonus);
        botoes_ok_cancela->setObjectName(QString::fromUtf8("botoes_ok_cancela"));
        botoes_ok_cancela->setOrientation(Qt::Horizontal);
        botoes_ok_cancela->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(botoes_ok_cancela, 1, 0, 1, 1);


        retranslateUi(ifg__qt__DialogoBonus);
        QObject::connect(botoes_ok_cancela, &QDialogButtonBox::accepted, ifg__qt__DialogoBonus, qOverload<>(&QDialog::accept));
        QObject::connect(botoes_ok_cancela, &QDialogButtonBox::rejected, ifg__qt__DialogoBonus, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoBonus);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoBonus)
    {
        ifg__qt__DialogoBonus->setWindowTitle(QCoreApplication::translate("ifg::qt::DialogoBonus", "Dialog", nullptr));
        botao_adicionar_bonus->setText(QCoreApplication::translate("ifg::qt::DialogoBonus", "+", nullptr));
        botao_remover_bonus->setText(QCoreApplication::translate("ifg::qt::DialogoBonus", "-", nullptr));
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
