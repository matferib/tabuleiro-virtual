/********************************************************************************
** Form generated from reading UI file 'opcoes.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OPCOES_H
#define OPCOES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>

namespace ifg {
namespace qt {

class Ui_DialogoOpcoes
{
public:
    QVBoxLayout *verticalLayout;
    QCheckBox *checkbox_mostrar_fps;
    QCheckBox *checkbox_texturas_sempre_de_frente;
    QCheckBox *checkbox_iluminacao_mestre;
    QCheckBox *checkbox_rosa_dos_ventos;
    QCheckBox *checkbox_anti_aliasing;
    QCheckBox *checkbox_grade;
    QCheckBox *checkbox_controle;
    QCheckBox *checkbox_mapeamento_oclusao;
    QCheckBox *checkbox_mapeamento_de_sombras;
    QCheckBox *checkbox_ataque_vs_defesa_posicao_real;
    QCheckBox *checkbox_tab_ativa_ataque;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        if (ifg__qt__DialogoOpcoes->objectName().isEmpty())
            ifg__qt__DialogoOpcoes->setObjectName(QStringLiteral("ifg__qt__DialogoOpcoes"));
        ifg__qt__DialogoOpcoes->resize(452, 496);
        verticalLayout = new QVBoxLayout(ifg__qt__DialogoOpcoes);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        checkbox_mostrar_fps = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mostrar_fps->setObjectName(QStringLiteral("checkbox_mostrar_fps"));

        verticalLayout->addWidget(checkbox_mostrar_fps);

        checkbox_texturas_sempre_de_frente = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_texturas_sempre_de_frente->setObjectName(QStringLiteral("checkbox_texturas_sempre_de_frente"));

        verticalLayout->addWidget(checkbox_texturas_sempre_de_frente);

        checkbox_iluminacao_mestre = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_iluminacao_mestre->setObjectName(QStringLiteral("checkbox_iluminacao_mestre"));

        verticalLayout->addWidget(checkbox_iluminacao_mestre);

        checkbox_rosa_dos_ventos = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_rosa_dos_ventos->setObjectName(QStringLiteral("checkbox_rosa_dos_ventos"));

        verticalLayout->addWidget(checkbox_rosa_dos_ventos);

        checkbox_anti_aliasing = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_anti_aliasing->setObjectName(QStringLiteral("checkbox_anti_aliasing"));

        verticalLayout->addWidget(checkbox_anti_aliasing);

        checkbox_grade = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_grade->setObjectName(QStringLiteral("checkbox_grade"));

        verticalLayout->addWidget(checkbox_grade);

        checkbox_controle = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_controle->setObjectName(QStringLiteral("checkbox_controle"));

        verticalLayout->addWidget(checkbox_controle);

        checkbox_mapeamento_oclusao = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_oclusao->setObjectName(QStringLiteral("checkbox_mapeamento_oclusao"));

        verticalLayout->addWidget(checkbox_mapeamento_oclusao);

        checkbox_mapeamento_de_sombras = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_de_sombras->setObjectName(QStringLiteral("checkbox_mapeamento_de_sombras"));

        verticalLayout->addWidget(checkbox_mapeamento_de_sombras);

        checkbox_ataque_vs_defesa_posicao_real = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_ataque_vs_defesa_posicao_real->setObjectName(QStringLiteral("checkbox_ataque_vs_defesa_posicao_real"));

        verticalLayout->addWidget(checkbox_ataque_vs_defesa_posicao_real);

        checkbox_tab_ativa_ataque = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_tab_ativa_ataque->setObjectName(QStringLiteral("checkbox_tab_ativa_ataque"));

        verticalLayout->addWidget(checkbox_tab_ativa_ataque);

        buttonBox = new QDialogButtonBox(ifg__qt__DialogoOpcoes);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ifg__qt__DialogoOpcoes);
        QObject::connect(buttonBox, SIGNAL(accepted()), ifg__qt__DialogoOpcoes, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ifg__qt__DialogoOpcoes, SLOT(reject()));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoOpcoes);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        ifg__qt__DialogoOpcoes->setWindowTitle(QApplication::translate("ifg::qt::DialogoOpcoes", "Dialog", nullptr));
        checkbox_mostrar_fps->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Mostrar Tempo de Desenho da cena", nullptr));
        checkbox_texturas_sempre_de_frente->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Texturas de Entidades Sempre de frente para c\303\242mera", nullptr));
        checkbox_iluminacao_mestre->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Ilumina\303\247\303\243o do mestre igual \303\240 dos jogadores", nullptr));
        checkbox_rosa_dos_ventos->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Desenha rosa dos ventos", nullptr));
        checkbox_anti_aliasing->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Anti Serrilhamento", nullptr));
        checkbox_grade->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Desenhar Grade", nullptr));
        checkbox_controle->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Desenhar Controle Virtual", nullptr));
        checkbox_mapeamento_oclusao->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Oclus\303\243o", nullptr));
        checkbox_mapeamento_de_sombras->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Mapeamento de sombras", nullptr));
        checkbox_ataque_vs_defesa_posicao_real->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Ataque vs Defesa posi\303\247\303\243o Real", nullptr));
        checkbox_tab_ativa_ataque->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Tab ativa ataque", nullptr));
    } // retranslateUi

};

} // namespace qt
} // namespace ifg

namespace ifg {
namespace qt {
namespace Ui {
    class DialogoOpcoes: public Ui_DialogoOpcoes {};
} // namespace Ui
} // namespace qt
} // namespace ifg

#endif // OPCOES_H
