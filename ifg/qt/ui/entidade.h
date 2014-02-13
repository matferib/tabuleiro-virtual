/********************************************************************************
** Form generated from reading UI file 'entidade.ui'
**
** Created: Thu Feb 13 00:44:14 2014
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef ENTIDADE_H
#define ENTIDADE_H

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
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
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
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_3;
    QPushButton *botao_cor;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkbox_luz;
    QPushButton *botao_luz;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QLabel *label_tamanho;
    QSlider *slider_tamanho;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *horizontalLayout_6;
    QLineEdit *linha_textura;
    QPushButton *botao_textura;
    QWidget *horizontalLayoutWidget_7;
    QHBoxLayout *horizontalLayout_8;
    QHBoxLayout *horizontalLayout_9;
    QCheckBox *checkbox_voadora;
    QCheckBox *checkbox_visibilidade;
    QHBoxLayout *horizontalLayout_10;
    QLabel *label_4;
    QSpinBox *spin_aura;
    QWidget *horizontalLayoutWidget_8;
    QHBoxLayout *horizontalLayout_11;
    QHBoxLayout *horizontalLayout_12;
    QLabel *label_5;
    QSpinBox *spin_pontos_vida;
    QLabel *label_6;
    QSpinBox *spin_max_pontos_vida;

    void setupUi(QDialog *ifg__qt__DialogoEntidade)
    {
        if (ifg__qt__DialogoEntidade->objectName().isEmpty())
            ifg__qt__DialogoEntidade->setObjectName(QString::fromUtf8("ifg__qt__DialogoEntidade"));
        ifg__qt__DialogoEntidade->resize(461, 392);
        ifg__qt__DialogoEntidade->setStyleSheet(QString::fromUtf8(""));
        ifg__qt__DialogoEntidade->setModal(true);
        botoes = new QDialogButtonBox(ifg__qt__DialogoEntidade);
        botoes->setObjectName(QString::fromUtf8("botoes"));
        botoes->setGeometry(QRect(100, 330, 341, 32));
        botoes->setOrientation(Qt::Horizontal);
        botoes->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
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
        horizontalLayoutWidget_2->setGeometry(QRect(20, 180, 361, 41));
        horizontalLayout_2 = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_3 = new QLabel(horizontalLayoutWidget_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_7->addWidget(label_3);


        horizontalLayout_2->addLayout(horizontalLayout_7);

        botao_cor = new QPushButton(horizontalLayoutWidget_2);
        botao_cor->setObjectName(QString::fromUtf8("botao_cor"));

        horizontalLayout_2->addWidget(botao_cor);

        horizontalLayoutWidget_3 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(20, 220, 361, 41));
        horizontalLayout_3 = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        checkbox_luz = new QCheckBox(horizontalLayoutWidget_3);
        checkbox_luz->setObjectName(QString::fromUtf8("checkbox_luz"));
        checkbox_luz->setLayoutDirection(Qt::RightToLeft);

        horizontalLayout_3->addWidget(checkbox_luz);

        botao_luz = new QPushButton(horizontalLayoutWidget_3);
        botao_luz->setObjectName(QString::fromUtf8("botao_luz"));
        botao_luz->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(botao_luz);

        horizontalLayoutWidget_4 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_4->setObjectName(QString::fromUtf8("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(20, 60, 361, 41));
        horizontalLayout_4 = new QHBoxLayout(horizontalLayoutWidget_4);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(horizontalLayoutWidget_4);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_2);

        label_tamanho = new QLabel(horizontalLayoutWidget_4);
        label_tamanho->setObjectName(QString::fromUtf8("label_tamanho"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_tamanho->sizePolicy().hasHeightForWidth());
        label_tamanho->setSizePolicy(sizePolicy2);
        label_tamanho->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_5->addWidget(label_tamanho);


        horizontalLayout_4->addLayout(horizontalLayout_5);

        slider_tamanho = new QSlider(horizontalLayoutWidget_4);
        slider_tamanho->setObjectName(QString::fromUtf8("slider_tamanho"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(slider_tamanho->sizePolicy().hasHeightForWidth());
        slider_tamanho->setSizePolicy(sizePolicy3);
        slider_tamanho->setMaximum(8);
        slider_tamanho->setPageStep(2);
        slider_tamanho->setSliderPosition(4);
        slider_tamanho->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(slider_tamanho);

        horizontalLayoutWidget_5 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_5->setObjectName(QString::fromUtf8("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(20, 260, 361, 41));
        horizontalLayout_6 = new QHBoxLayout(horizontalLayoutWidget_5);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalLayout_6->setContentsMargins(0, 0, 0, 0);
        linha_textura = new QLineEdit(horizontalLayoutWidget_5);
        linha_textura->setObjectName(QString::fromUtf8("linha_textura"));
        linha_textura->setReadOnly(false);

        horizontalLayout_6->addWidget(linha_textura);

        botao_textura = new QPushButton(horizontalLayoutWidget_5);
        botao_textura->setObjectName(QString::fromUtf8("botao_textura"));

        horizontalLayout_6->addWidget(botao_textura);

        horizontalLayoutWidget_7 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_7->setObjectName(QString::fromUtf8("horizontalLayoutWidget_7"));
        horizontalLayoutWidget_7->setGeometry(QRect(20, 140, 361, 41));
        horizontalLayout_8 = new QHBoxLayout(horizontalLayoutWidget_7);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_9 = new QHBoxLayout();
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        checkbox_voadora = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_voadora->setObjectName(QString::fromUtf8("checkbox_voadora"));

        horizontalLayout_9->addWidget(checkbox_voadora);

        checkbox_visibilidade = new QCheckBox(horizontalLayoutWidget_7);
        checkbox_visibilidade->setObjectName(QString::fromUtf8("checkbox_visibilidade"));
        checkbox_visibilidade->setEnabled(true);

        horizontalLayout_9->addWidget(checkbox_visibilidade);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        label_4 = new QLabel(horizontalLayoutWidget_7);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_10->addWidget(label_4);

        spin_aura = new QSpinBox(horizontalLayoutWidget_7);
        spin_aura->setObjectName(QString::fromUtf8("spin_aura"));

        horizontalLayout_10->addWidget(spin_aura);


        horizontalLayout_9->addLayout(horizontalLayout_10);


        horizontalLayout_8->addLayout(horizontalLayout_9);

        horizontalLayoutWidget_8 = new QWidget(ifg__qt__DialogoEntidade);
        horizontalLayoutWidget_8->setObjectName(QString::fromUtf8("horizontalLayoutWidget_8"));
        horizontalLayoutWidget_8->setGeometry(QRect(20, 100, 361, 41));
        horizontalLayout_11 = new QHBoxLayout(horizontalLayoutWidget_8);
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalLayout_11->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        label_5 = new QLabel(horizontalLayoutWidget_8);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_5);

        spin_pontos_vida = new QSpinBox(horizontalLayoutWidget_8);
        spin_pontos_vida->setObjectName(QString::fromUtf8("spin_pontos_vida"));
        spin_pontos_vida->setMinimum(-100);
        spin_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_pontos_vida);

        label_6 = new QLabel(horizontalLayoutWidget_8);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_12->addWidget(label_6);

        spin_max_pontos_vida = new QSpinBox(horizontalLayoutWidget_8);
        spin_max_pontos_vida->setObjectName(QString::fromUtf8("spin_max_pontos_vida"));
        spin_max_pontos_vida->setMinimum(-100);
        spin_max_pontos_vida->setMaximum(999);

        horizontalLayout_12->addWidget(spin_max_pontos_vida);


        horizontalLayout_11->addLayout(horizontalLayout_12);


        retranslateUi(ifg__qt__DialogoEntidade);
        QObject::connect(botoes, SIGNAL(accepted()), ifg__qt__DialogoEntidade, SLOT(accept()));
        QObject::connect(botoes, SIGNAL(rejected()), ifg__qt__DialogoEntidade, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoEntidade);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoEntidade)
    {
        ifg__qt__DialogoEntidade->setWindowTitle(QApplication::translate("ifg::qt::DialogoEntidade", "Propriedades da Entidade", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Id", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Cor:", 0, QApplication::UnicodeUTF8));
        botao_cor->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor", 0, QApplication::UnicodeUTF8));
        checkbox_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Possui Luz", 0, QApplication::UnicodeUTF8));
        botao_luz->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Cor da Luz", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Tamanho", 0, QApplication::UnicodeUTF8));
        label_tamanho->setText(QApplication::translate("ifg::qt::DialogoEntidade", "(m\303\251dio)", 0, QApplication::UnicodeUTF8));
        linha_textura->setPlaceholderText(QApplication::translate("ifg::qt::DialogoEntidade", "Caminho para textura ou vazio", 0, QApplication::UnicodeUTF8));
        botao_textura->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Escolher Textura", 0, QApplication::UnicodeUTF8));
        checkbox_voadora->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Voadora", 0, QApplication::UnicodeUTF8));
        checkbox_visibilidade->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Vis\303\255vel", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Aura:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Pontos de Vida:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ifg::qt::DialogoEntidade", "Max", 0, QApplication::UnicodeUTF8));
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

#endif // ENTIDADE_H
