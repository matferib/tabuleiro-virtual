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

namespace ifg {
namespace qt {

class Ui_DialogoBonus
{
public:
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ifg__qt__DialogoBonus)
    {
        if (ifg__qt__DialogoBonus->objectName().isEmpty())
            ifg__qt__DialogoBonus->setObjectName(QString::fromUtf8("ifg__qt__DialogoBonus"));
        ifg__qt__DialogoBonus->resize(564, 624);
        buttonBox = new QDialogButtonBox(ifg__qt__DialogoBonus);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(320, 570, 221, 41));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        retranslateUi(ifg__qt__DialogoBonus);
        QObject::connect(buttonBox, SIGNAL(accepted()), ifg__qt__DialogoBonus, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ifg__qt__DialogoBonus, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoBonus);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoBonus)
    {
        ifg__qt__DialogoBonus->setWindowTitle(QApplication::translate("ifg::qt::DialogoBonus", "Dialog", 0, QApplication::UnicodeUTF8));
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
