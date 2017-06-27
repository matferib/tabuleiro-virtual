/********************************************************************************
** Form generated from reading UI file 'bonus_individual.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef BONUS_INDIVIDUAL_H
#define BONUS_INDIVIDUAL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

namespace ifg {
namespace qt {

class Ui_BonusIndividual
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QComboBox *combo_tipo;
    QListWidget *list_por_origem;
    QPushButton *botao_adiciona_origem;
    QPushButton *botao_remover_origem;

    void setupUi(QWidget *ifg__qt__BonusIndividual)
    {
        if (ifg__qt__BonusIndividual->objectName().isEmpty())
            ifg__qt__BonusIndividual->setObjectName(QString::fromUtf8("ifg__qt__BonusIndividual"));
        ifg__qt__BonusIndividual->resize(548, 33);
        horizontalLayoutWidget = new QWidget(ifg__qt__BonusIndividual);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 0, 531, 31));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(horizontalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        combo_tipo = new QComboBox(horizontalLayoutWidget);
        combo_tipo->setObjectName(QString::fromUtf8("combo_tipo"));

        horizontalLayout->addWidget(combo_tipo);

        list_por_origem = new QListWidget(horizontalLayoutWidget);
        list_por_origem->setObjectName(QString::fromUtf8("list_por_origem"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(list_por_origem->sizePolicy().hasHeightForWidth());
        list_por_origem->setSizePolicy(sizePolicy);
        list_por_origem->setFlow(QListView::LeftToRight);

        horizontalLayout->addWidget(list_por_origem);

        botao_adiciona_origem = new QPushButton(horizontalLayoutWidget);
        botao_adiciona_origem->setObjectName(QString::fromUtf8("botao_adiciona_origem"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(botao_adiciona_origem->sizePolicy().hasHeightForWidth());
        botao_adiciona_origem->setSizePolicy(sizePolicy1);
        botao_adiciona_origem->setMinimumSize(QSize(1, 0));
        QFont font;
        font.setFamily(QString::fromUtf8("Noto Sans [unknown]"));
        font.setBold(true);
        font.setWeight(75);
        botao_adiciona_origem->setFont(font);
        botao_adiciona_origem->setStyleSheet(QString::fromUtf8("color: green"));

        horizontalLayout->addWidget(botao_adiciona_origem);

        botao_remover_origem = new QPushButton(horizontalLayoutWidget);
        botao_remover_origem->setObjectName(QString::fromUtf8("botao_remover_origem"));
        sizePolicy1.setHeightForWidth(botao_remover_origem->sizePolicy().hasHeightForWidth());
        botao_remover_origem->setSizePolicy(sizePolicy1);
        botao_remover_origem->setMinimumSize(QSize(1, 0));
        botao_remover_origem->setFont(font);
        botao_remover_origem->setStyleSheet(QString::fromUtf8("color:  red"));

        horizontalLayout->addWidget(botao_remover_origem);


        retranslateUi(ifg__qt__BonusIndividual);

        QMetaObject::connectSlotsByName(ifg__qt__BonusIndividual);
    } // setupUi

    void retranslateUi(QWidget *ifg__qt__BonusIndividual)
    {
        ifg__qt__BonusIndividual->setWindowTitle(QApplication::translate("ifg::qt::BonusIndividual", "Form", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::BonusIndividual", "Tipo", 0, QApplication::UnicodeUTF8));
        combo_tipo->clear();
        combo_tipo->insertItems(0, QStringList()
         << QApplication::translate("ifg::qt::BonusIndividual", "Alqu\303\255mico", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Armadura", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Armadura (melhoria)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Armadura Natural", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Atributo", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Base", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Circunst\303\242ncia", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Classe", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Compet\303\252ncia", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Deflex\303\243o", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Escudo", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Escudo (melhoria)", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Esquiva", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Familiar", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Inerente", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Intui\303\247\303\243o", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Melhoria", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Moral", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "N\303\255vel", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Profano", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Racial", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Template", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Resist\303\252ncia", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Sagrado", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Sinergia", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Sorte", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Talento", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("ifg::qt::BonusIndividual", "Tamanho", 0, QApplication::UnicodeUTF8)
        );
        botao_adiciona_origem->setText(QApplication::translate("ifg::qt::BonusIndividual", "+", 0, QApplication::UnicodeUTF8));
        botao_remover_origem->setText(QApplication::translate("ifg::qt::BonusIndividual", "-", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class BonusIndividual: public Ui_BonusIndividual {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // BONUS_INDIVIDUAL_H
