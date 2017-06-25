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
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_dialogo_bonus
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label_9;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_7;
    QSpinBox *spin_alquimico;
    QLabel *label_20;
    QLabel *label_25;
    QLabel *label_17;
    QLabel *label_19;
    QLabel *label_22;
    QLabel *label_24;
    QLabel *label_18;
    QLabel *label_21;
    QLabel *label_12;
    QLabel *label_11;
    QLabel *label_15;
    QLabel *label_13;
    QSpinBox *spin_base;
    QLabel *label_8;
    QLabel *label_4;
    QLabel *label_10;
    QLabel *label_29;
    QLabel *label_6;
    QLabel *label_2;
    QLabel *label;
    QLabel *label_28;
    QSpinBox *spin_atributo;
    QSpinBox *spin_deflexao;
    QSpinBox *spin_competencia;
    QSpinBox *spin_classe;
    QSpinBox *spin_circunstancia;
    QSpinBox *spin_escudo_melhoria;
    QSpinBox *spin_escudo;
    QSpinBox *spin_inerente;
    QLabel *label_27;
    QSpinBox *spin_esquiva;
    QSpinBox *spin_familiar;
    QLabel *label_16;
    QLabel *label_23;
    QLabel *label_26;
    QLabel *label_14;
    QSpinBox *spin_intuicao;
    QSpinBox *spin_melhoria_2;
    QSpinBox *spin_moral;
    QSpinBox *spin_niveis_negativos;
    QSpinBox *spin_nivel;
    QSpinBox *spin_profano;
    QSpinBox *spin_racial;
    QSpinBox *spin_template;
    QSpinBox *spin_resistencia;
    QSpinBox *spin_sagrado;
    QSpinBox *spin_sinergia;
    QSpinBox *spin_sorte;
    QSpinBox *spin_talento;
    QSpinBox *spin_tamanho;
    QSpinBox *spin_armadura_natural;
    QSpinBox *spin_melhoria;
    QSpinBox *spin_armadura;

    void setupUi(QDialog *dialogo_bonus)
    {
        if (dialogo_bonus->objectName().isEmpty())
            dialogo_bonus->setObjectName(QString::fromUtf8("dialogo_bonus"));
        dialogo_bonus->resize(564, 624);
        buttonBox = new QDialogButtonBox(dialogo_bonus);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(320, 570, 221, 41));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        gridLayoutWidget = new QWidget(dialogo_bonus);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(40, 30, 491, 531));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label_9 = new QLabel(gridLayoutWidget);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_9, 8, 0, 1, 1);

        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        label_5 = new QLabel(gridLayoutWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        label_7 = new QLabel(gridLayoutWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_7, 6, 0, 1, 1);

        spin_alquimico = new QSpinBox(gridLayoutWidget);
        spin_alquimico->setObjectName(QString::fromUtf8("spin_alquimico"));
        spin_alquimico->setMinimum(-99);

        gridLayout->addWidget(spin_alquimico, 0, 1, 1, 1);

        label_20 = new QLabel(gridLayoutWidget);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_20, 4, 2, 1, 1);

        label_25 = new QLabel(gridLayoutWidget);
        label_25->setObjectName(QString::fromUtf8("label_25"));
        label_25->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_25, 9, 2, 1, 1);

        label_17 = new QLabel(gridLayoutWidget);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_17, 1, 2, 1, 1);

        label_19 = new QLabel(gridLayoutWidget);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_19, 3, 2, 1, 1);

        label_22 = new QLabel(gridLayoutWidget);
        label_22->setObjectName(QString::fromUtf8("label_22"));
        label_22->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_22, 6, 2, 1, 1);

        label_24 = new QLabel(gridLayoutWidget);
        label_24->setObjectName(QString::fromUtf8("label_24"));
        label_24->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_24, 8, 2, 1, 1);

        label_18 = new QLabel(gridLayoutWidget);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_18, 2, 2, 1, 1);

        label_21 = new QLabel(gridLayoutWidget);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_21, 5, 2, 1, 1);

        label_12 = new QLabel(gridLayoutWidget);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_12, 11, 0, 1, 1);

        label_11 = new QLabel(gridLayoutWidget);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_11, 10, 0, 1, 1);

        label_15 = new QLabel(gridLayoutWidget);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_15, 14, 0, 1, 1);

        label_13 = new QLabel(gridLayoutWidget);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_13, 12, 0, 1, 1);

        spin_base = new QSpinBox(gridLayoutWidget);
        spin_base->setObjectName(QString::fromUtf8("spin_base"));
        spin_base->setMinimum(-99);

        gridLayout->addWidget(spin_base, 5, 1, 1, 1);

        label_8 = new QLabel(gridLayoutWidget);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_8, 7, 0, 1, 1);

        label_4 = new QLabel(gridLayoutWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_10 = new QLabel(gridLayoutWidget);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_10, 9, 0, 1, 1);

        label_29 = new QLabel(gridLayoutWidget);
        label_29->setObjectName(QString::fromUtf8("label_29"));
        label_29->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_29, 13, 2, 1, 1);

        label_6 = new QLabel(gridLayoutWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_6, 5, 0, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_28 = new QLabel(gridLayoutWidget);
        label_28->setObjectName(QString::fromUtf8("label_28"));
        label_28->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_28, 12, 2, 1, 1);

        spin_atributo = new QSpinBox(gridLayoutWidget);
        spin_atributo->setObjectName(QString::fromUtf8("spin_atributo"));
        spin_atributo->setMinimum(-99);

        gridLayout->addWidget(spin_atributo, 4, 1, 1, 1);

        spin_deflexao = new QSpinBox(gridLayoutWidget);
        spin_deflexao->setObjectName(QString::fromUtf8("spin_deflexao"));
        spin_deflexao->setMinimum(-99);

        gridLayout->addWidget(spin_deflexao, 9, 1, 1, 1);

        spin_competencia = new QSpinBox(gridLayoutWidget);
        spin_competencia->setObjectName(QString::fromUtf8("spin_competencia"));
        spin_competencia->setMinimum(-99);

        gridLayout->addWidget(spin_competencia, 8, 1, 1, 1);

        spin_classe = new QSpinBox(gridLayoutWidget);
        spin_classe->setObjectName(QString::fromUtf8("spin_classe"));
        spin_classe->setMinimum(-99);

        gridLayout->addWidget(spin_classe, 7, 1, 1, 1);

        spin_circunstancia = new QSpinBox(gridLayoutWidget);
        spin_circunstancia->setObjectName(QString::fromUtf8("spin_circunstancia"));
        spin_circunstancia->setMinimum(-99);

        gridLayout->addWidget(spin_circunstancia, 6, 1, 1, 1);

        spin_escudo_melhoria = new QSpinBox(gridLayoutWidget);
        spin_escudo_melhoria->setObjectName(QString::fromUtf8("spin_escudo_melhoria"));
        spin_escudo_melhoria->setMinimum(-99);

        gridLayout->addWidget(spin_escudo_melhoria, 11, 1, 1, 1);

        spin_escudo = new QSpinBox(gridLayoutWidget);
        spin_escudo->setObjectName(QString::fromUtf8("spin_escudo"));
        spin_escudo->setMinimum(-99);

        gridLayout->addWidget(spin_escudo, 10, 1, 1, 1);

        spin_inerente = new QSpinBox(gridLayoutWidget);
        spin_inerente->setObjectName(QString::fromUtf8("spin_inerente"));
        spin_inerente->setMinimum(-99);

        gridLayout->addWidget(spin_inerente, 14, 1, 1, 1);

        label_27 = new QLabel(gridLayoutWidget);
        label_27->setObjectName(QString::fromUtf8("label_27"));
        label_27->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_27, 11, 2, 1, 1);

        spin_esquiva = new QSpinBox(gridLayoutWidget);
        spin_esquiva->setObjectName(QString::fromUtf8("spin_esquiva"));
        spin_esquiva->setMinimum(-99);

        gridLayout->addWidget(spin_esquiva, 12, 1, 1, 1);

        spin_familiar = new QSpinBox(gridLayoutWidget);
        spin_familiar->setObjectName(QString::fromUtf8("spin_familiar"));
        spin_familiar->setMinimum(-99);

        gridLayout->addWidget(spin_familiar, 13, 1, 1, 1);

        label_16 = new QLabel(gridLayoutWidget);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_16, 0, 2, 1, 1);

        label_23 = new QLabel(gridLayoutWidget);
        label_23->setObjectName(QString::fromUtf8("label_23"));
        label_23->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_23, 7, 2, 1, 1);

        label_26 = new QLabel(gridLayoutWidget);
        label_26->setObjectName(QString::fromUtf8("label_26"));
        label_26->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_26, 10, 2, 1, 1);

        label_14 = new QLabel(gridLayoutWidget);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label_14, 13, 0, 1, 1);

        spin_intuicao = new QSpinBox(gridLayoutWidget);
        spin_intuicao->setObjectName(QString::fromUtf8("spin_intuicao"));
        spin_intuicao->setMinimum(-99);

        gridLayout->addWidget(spin_intuicao, 0, 3, 1, 1);

        spin_melhoria_2 = new QSpinBox(gridLayoutWidget);
        spin_melhoria_2->setObjectName(QString::fromUtf8("spin_melhoria_2"));
        spin_melhoria_2->setMinimum(-99);

        gridLayout->addWidget(spin_melhoria_2, 1, 3, 1, 1);

        spin_moral = new QSpinBox(gridLayoutWidget);
        spin_moral->setObjectName(QString::fromUtf8("spin_moral"));
        spin_moral->setMinimum(-99);

        gridLayout->addWidget(spin_moral, 2, 3, 1, 1);

        spin_niveis_negativos = new QSpinBox(gridLayoutWidget);
        spin_niveis_negativos->setObjectName(QString::fromUtf8("spin_niveis_negativos"));
        spin_niveis_negativos->setMinimum(-99);

        gridLayout->addWidget(spin_niveis_negativos, 3, 3, 1, 1);

        spin_nivel = new QSpinBox(gridLayoutWidget);
        spin_nivel->setObjectName(QString::fromUtf8("spin_nivel"));
        spin_nivel->setMinimum(-99);

        gridLayout->addWidget(spin_nivel, 4, 3, 1, 1);

        spin_profano = new QSpinBox(gridLayoutWidget);
        spin_profano->setObjectName(QString::fromUtf8("spin_profano"));
        spin_profano->setMinimum(-99);

        gridLayout->addWidget(spin_profano, 5, 3, 1, 1);

        spin_racial = new QSpinBox(gridLayoutWidget);
        spin_racial->setObjectName(QString::fromUtf8("spin_racial"));
        spin_racial->setMinimum(-99);

        gridLayout->addWidget(spin_racial, 6, 3, 1, 1);

        spin_template = new QSpinBox(gridLayoutWidget);
        spin_template->setObjectName(QString::fromUtf8("spin_template"));
        spin_template->setMinimum(-99);

        gridLayout->addWidget(spin_template, 7, 3, 1, 1);

        spin_resistencia = new QSpinBox(gridLayoutWidget);
        spin_resistencia->setObjectName(QString::fromUtf8("spin_resistencia"));
        spin_resistencia->setMinimum(-99);

        gridLayout->addWidget(spin_resistencia, 8, 3, 1, 1);

        spin_sagrado = new QSpinBox(gridLayoutWidget);
        spin_sagrado->setObjectName(QString::fromUtf8("spin_sagrado"));
        spin_sagrado->setMinimum(-99);

        gridLayout->addWidget(spin_sagrado, 9, 3, 1, 1);

        spin_sinergia = new QSpinBox(gridLayoutWidget);
        spin_sinergia->setObjectName(QString::fromUtf8("spin_sinergia"));
        spin_sinergia->setMinimum(-99);

        gridLayout->addWidget(spin_sinergia, 10, 3, 1, 1);

        spin_sorte = new QSpinBox(gridLayoutWidget);
        spin_sorte->setObjectName(QString::fromUtf8("spin_sorte"));
        spin_sorte->setMinimum(-99);

        gridLayout->addWidget(spin_sorte, 11, 3, 1, 1);

        spin_talento = new QSpinBox(gridLayoutWidget);
        spin_talento->setObjectName(QString::fromUtf8("spin_talento"));
        spin_talento->setMinimum(-99);

        gridLayout->addWidget(spin_talento, 12, 3, 1, 1);

        spin_tamanho = new QSpinBox(gridLayoutWidget);
        spin_tamanho->setObjectName(QString::fromUtf8("spin_tamanho"));
        spin_tamanho->setMinimum(-99);

        gridLayout->addWidget(spin_tamanho, 13, 3, 1, 1);

        spin_armadura_natural = new QSpinBox(gridLayoutWidget);
        spin_armadura_natural->setObjectName(QString::fromUtf8("spin_armadura_natural"));
        spin_armadura_natural->setMinimum(-99);

        gridLayout->addWidget(spin_armadura_natural, 3, 1, 1, 1);

        spin_melhoria = new QSpinBox(gridLayoutWidget);
        spin_melhoria->setObjectName(QString::fromUtf8("spin_melhoria"));
        spin_melhoria->setMinimum(-99);

        gridLayout->addWidget(spin_melhoria, 2, 1, 1, 1);

        spin_armadura = new QSpinBox(gridLayoutWidget);
        spin_armadura->setObjectName(QString::fromUtf8("spin_armadura"));
        spin_armadura->setMinimum(-99);

        gridLayout->addWidget(spin_armadura, 1, 1, 1, 1);

        QWidget::setTabOrder(spin_alquimico, spin_armadura);
        QWidget::setTabOrder(spin_armadura, spin_melhoria);
        QWidget::setTabOrder(spin_melhoria, spin_armadura_natural);
        QWidget::setTabOrder(spin_armadura_natural, spin_atributo);
        QWidget::setTabOrder(spin_atributo, spin_base);
        QWidget::setTabOrder(spin_base, spin_circunstancia);
        QWidget::setTabOrder(spin_circunstancia, spin_classe);
        QWidget::setTabOrder(spin_classe, spin_competencia);
        QWidget::setTabOrder(spin_competencia, spin_deflexao);
        QWidget::setTabOrder(spin_deflexao, spin_escudo);
        QWidget::setTabOrder(spin_escudo, spin_escudo_melhoria);
        QWidget::setTabOrder(spin_escudo_melhoria, spin_esquiva);
        QWidget::setTabOrder(spin_esquiva, spin_familiar);
        QWidget::setTabOrder(spin_familiar, spin_inerente);
        QWidget::setTabOrder(spin_inerente, spin_intuicao);
        QWidget::setTabOrder(spin_intuicao, spin_melhoria_2);
        QWidget::setTabOrder(spin_melhoria_2, spin_moral);
        QWidget::setTabOrder(spin_moral, spin_niveis_negativos);
        QWidget::setTabOrder(spin_niveis_negativos, spin_nivel);
        QWidget::setTabOrder(spin_nivel, spin_profano);
        QWidget::setTabOrder(spin_profano, spin_racial);
        QWidget::setTabOrder(spin_racial, spin_template);
        QWidget::setTabOrder(spin_template, spin_resistencia);
        QWidget::setTabOrder(spin_resistencia, spin_sagrado);
        QWidget::setTabOrder(spin_sagrado, spin_sinergia);
        QWidget::setTabOrder(spin_sinergia, spin_sorte);
        QWidget::setTabOrder(spin_sorte, spin_talento);
        QWidget::setTabOrder(spin_talento, spin_tamanho);
        QWidget::setTabOrder(spin_tamanho, buttonBox);

        retranslateUi(dialogo_bonus);
        QObject::connect(buttonBox, SIGNAL(accepted()), dialogo_bonus, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), dialogo_bonus, SLOT(reject()));

        QMetaObject::connectSlotsByName(dialogo_bonus);
    } // setupUi

    void retranslateUi(QDialog *dialogo_bonus)
    {
        dialogo_bonus->setWindowTitle(QApplication::translate("dialogo_bonus", "Dialog", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("dialogo_bonus", "Compet\303\252ncia", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("dialogo_bonus", "Armadura Melhoria", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("dialogo_bonus", "Atributo", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("dialogo_bonus", "Circunst\303\242ncia", 0, QApplication::UnicodeUTF8));
        label_20->setText(QApplication::translate("dialogo_bonus", "N\303\255vel", 0, QApplication::UnicodeUTF8));
        label_25->setText(QApplication::translate("dialogo_bonus", "Sagrado", 0, QApplication::UnicodeUTF8));
        label_17->setText(QApplication::translate("dialogo_bonus", "Melhoria", 0, QApplication::UnicodeUTF8));
        label_19->setText(QApplication::translate("dialogo_bonus", "N\303\255veis Negativos", 0, QApplication::UnicodeUTF8));
        label_22->setText(QApplication::translate("dialogo_bonus", "Racial", 0, QApplication::UnicodeUTF8));
        label_24->setText(QApplication::translate("dialogo_bonus", "Resist\303\252ncia", 0, QApplication::UnicodeUTF8));
        label_18->setText(QApplication::translate("dialogo_bonus", "Moral", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("dialogo_bonus", "Profano", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("dialogo_bonus", "Escudo Melhoria", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("dialogo_bonus", "Escudo", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("dialogo_bonus", "Inerente", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("dialogo_bonus", "Esquiva", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("dialogo_bonus", "Classe", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("dialogo_bonus", "Armadura Natural", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("dialogo_bonus", "Deflex\303\243o", 0, QApplication::UnicodeUTF8));
        label_29->setText(QApplication::translate("dialogo_bonus", "Tamanho", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("dialogo_bonus", "Base", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("dialogo_bonus", "Armadura", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("dialogo_bonus", "Alqu\303\255mico", 0, QApplication::UnicodeUTF8));
        label_28->setText(QApplication::translate("dialogo_bonus", "Talento", 0, QApplication::UnicodeUTF8));
        label_27->setText(QApplication::translate("dialogo_bonus", "Sorte", 0, QApplication::UnicodeUTF8));
        label_16->setText(QApplication::translate("dialogo_bonus", "Intui\303\247\303\243o", 0, QApplication::UnicodeUTF8));
        label_23->setText(QApplication::translate("dialogo_bonus", "Template", 0, QApplication::UnicodeUTF8));
        label_26->setText(QApplication::translate("dialogo_bonus", "Sinergia", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("dialogo_bonus", "Familiar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class dialogo_bonus: public Ui_dialogo_bonus {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DIALOGOBONUS_H
