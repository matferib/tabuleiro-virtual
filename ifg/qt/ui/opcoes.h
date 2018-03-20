/********************************************************************************
** Form generated from reading UI file 'opcoes.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OPCOES_H
#define OPCOES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>

namespace ifg {
namespace qt {

class Ui_DialogoOpcoes
{
public:
    QDialogButtonBox *buttonBox;
    QCheckBox *checkbox_mostrar_fps;
    QCheckBox *checkbox_texturas_sempre_de_frente;
    QCheckBox *checkbox_iluminacao_mestre;
    QCheckBox *checkbox_rosa_dos_ventos;
    QCheckBox *checkbox_anti_aliasing;
    QCheckBox *checkbox_grade;
    QCheckBox *checkbox_controle;
    QGroupBox *groupBox;
    QCheckBox *checkbox_iluminacao_por_pixel;
    QCheckBox *checkbox_mapeamento_oclusao;
    QCheckBox *checkbox_mapeamento_de_sombras;
    QCheckBox *checkbox_ataque_vs_defesa_posicao_real;

    void setupUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        if (ifg__qt__DialogoOpcoes->objectName().isEmpty())
            ifg__qt__DialogoOpcoes->setObjectName(QStringLiteral("ifg__qt__DialogoOpcoes"));
        ifg__qt__DialogoOpcoes->resize(423, 436);
        buttonBox = new QDialogButtonBox(ifg__qt__DialogoOpcoes);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(210, 380, 191, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        checkbox_mostrar_fps = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mostrar_fps->setObjectName(QStringLiteral("checkbox_mostrar_fps"));
        checkbox_mostrar_fps->setGeometry(QRect(10, 20, 271, 22));
        checkbox_texturas_sempre_de_frente = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_texturas_sempre_de_frente->setObjectName(QStringLiteral("checkbox_texturas_sempre_de_frente"));
        checkbox_texturas_sempre_de_frente->setGeometry(QRect(10, 50, 381, 22));
        checkbox_iluminacao_mestre = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_iluminacao_mestre->setObjectName(QStringLiteral("checkbox_iluminacao_mestre"));
        checkbox_iluminacao_mestre->setGeometry(QRect(10, 80, 381, 22));
        checkbox_rosa_dos_ventos = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_rosa_dos_ventos->setObjectName(QStringLiteral("checkbox_rosa_dos_ventos"));
        checkbox_rosa_dos_ventos->setGeometry(QRect(10, 110, 271, 22));
        checkbox_anti_aliasing = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_anti_aliasing->setObjectName(QStringLiteral("checkbox_anti_aliasing"));
        checkbox_anti_aliasing->setGeometry(QRect(10, 140, 271, 22));
        checkbox_grade = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_grade->setObjectName(QStringLiteral("checkbox_grade"));
        checkbox_grade->setGeometry(QRect(10, 170, 271, 22));
        checkbox_controle = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_controle->setObjectName(QStringLiteral("checkbox_controle"));
        checkbox_controle->setGeometry(QRect(10, 200, 271, 22));
        groupBox = new QGroupBox(ifg__qt__DialogoOpcoes);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 330, 201, 51));
        checkbox_iluminacao_por_pixel = new QCheckBox(groupBox);
        checkbox_iluminacao_por_pixel->setObjectName(QStringLiteral("checkbox_iluminacao_por_pixel"));
        checkbox_iluminacao_por_pixel->setGeometry(QRect(10, 30, 271, 22));
        checkbox_mapeamento_oclusao = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_oclusao->setObjectName(QStringLiteral("checkbox_mapeamento_oclusao"));
        checkbox_mapeamento_oclusao->setGeometry(QRect(10, 230, 271, 22));
        checkbox_mapeamento_de_sombras = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_de_sombras->setObjectName(QStringLiteral("checkbox_mapeamento_de_sombras"));
        checkbox_mapeamento_de_sombras->setGeometry(QRect(10, 260, 271, 22));
        checkbox_ataque_vs_defesa_posicao_real = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_ataque_vs_defesa_posicao_real->setObjectName(QStringLiteral("checkbox_ataque_vs_defesa_posicao_real"));
        checkbox_ataque_vs_defesa_posicao_real->setGeometry(QRect(10, 290, 271, 22));

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
        groupBox->setTitle(QApplication::translate("ifg::qt::DialogoOpcoes", "requer reinicializa\303\247\303\243o", nullptr));
        checkbox_iluminacao_por_pixel->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Ilumina\303\247\303\243o por pixel (lento)", nullptr));
        checkbox_mapeamento_oclusao->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Oclus\303\243o", nullptr));
        checkbox_mapeamento_de_sombras->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Mapeamento de sombras", nullptr));
        checkbox_ataque_vs_defesa_posicao_real->setText(QApplication::translate("ifg::qt::DialogoOpcoes", "Ataque vs Defesa posi\303\247\303\243o Real", nullptr));
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
