/********************************************************************************
** Form generated from reading UI file 'entidades25115.ui'
**
** Created: Wed Jan 8 02:09:50 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTIDADES25115_H
#define ENTIDADES25115_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
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

class Ui_DialogoEntidade
{
public:
    QDialogButtonBox *botoes;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *campo_id;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkbox_cor;
    QPushButton *botao_cor;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_luz;
    QPushButton *botao_luz;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QString::fromUtf8("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(400, 186);
        ifg__qt__DialogoEntidade->setStyleSheet(QString::fromUtf8(""));
        ifg__qt__DialogoEntidade->setModal(true);
        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(30, 140, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        horizontalLayoutWidget = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(100, 10, 181, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label, 0, Qt::AlignRight);

        campo_id = new QLineEdit(horizontalLayoutWidget);
        campo_id->setObjectName(QString::fromUtf8("campo_id"));
        campo_id->setReadOnly(true);

        horizontalLayout->addWidget(campo_id, 0, Qt::AlignLeft);

        horizontalLayoutWidget_2 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(20, 50, 361, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        checkbox_cor = new QCheckBox(horizontalLayoutWidget_2);
        checkbox_cor->setObjectName(QString::fromUtf8("checkbox_cor"));
        checkbox_cor->setEnabled(true);

        horizontalLayout_2->addWidget(checkbox_cor);

        botao_cor = new QPushButton(horizontalLayoutWidget_2);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(20, 90, 361, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkbox_luz = new QCheckBox(horizontalLayoutWidget_3);
        checkbox_luz->setObjectName(QString::fromUtf8("checkbox_luz"));

        horizontalLayout_3->addWidget(checkbox_luz);

        botao_luz = new QPushButton(horizontalLayoutWidget_3);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(botao_luz);


        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Id", 0, QApplication::UnicodeUTF8));
        checkbox_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Cor", 0, QApplication::UnicodeUTF8));
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0, QApplication::UnicodeUTF8));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Luz", 0, QApplication::UnicodeUTF8));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoEntidade: public Ui_DialogoEntidade {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // ENTIDADES25115_H
