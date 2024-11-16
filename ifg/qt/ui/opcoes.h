/********************************************************************************
** Form generated from reading UI file 'opcoes.ui'
**
** Created by: Qt User Interface Compiler version 6.7.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef OPCOES_H
#define OPCOES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
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
    QCheckBox *checkbox_desativar_som;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkbox_resolucao_fixa;
    QComboBox *combo_tamanho_buffer_principal;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QComboBox *combo_tamanho_texturas;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QSlider *slider_escala;
    QLabel *label_escala;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QComboBox *combo_fps;
    QCheckBox *checkbox_desabilitar_retina;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        if (ifg__qt__DialogoOpcoes->objectName().isEmpty())
            ifg__qt__DialogoOpcoes->setObjectName("ifg__qt__DialogoOpcoes");
        ifg__qt__DialogoOpcoes->resize(452, 496);
        verticalLayout = new QVBoxLayout(ifg__qt__DialogoOpcoes);
        verticalLayout->setObjectName("verticalLayout");
        checkbox_mostrar_fps = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mostrar_fps->setObjectName("checkbox_mostrar_fps");

        verticalLayout->addWidget(checkbox_mostrar_fps);

        checkbox_texturas_sempre_de_frente = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_texturas_sempre_de_frente->setObjectName("checkbox_texturas_sempre_de_frente");

        verticalLayout->addWidget(checkbox_texturas_sempre_de_frente);

        checkbox_iluminacao_mestre = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_iluminacao_mestre->setObjectName("checkbox_iluminacao_mestre");

        verticalLayout->addWidget(checkbox_iluminacao_mestre);

        checkbox_rosa_dos_ventos = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_rosa_dos_ventos->setObjectName("checkbox_rosa_dos_ventos");

        verticalLayout->addWidget(checkbox_rosa_dos_ventos);

        checkbox_anti_aliasing = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_anti_aliasing->setObjectName("checkbox_anti_aliasing");

        verticalLayout->addWidget(checkbox_anti_aliasing);

        checkbox_grade = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_grade->setObjectName("checkbox_grade");

        verticalLayout->addWidget(checkbox_grade);

        checkbox_controle = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_controle->setObjectName("checkbox_controle");

        verticalLayout->addWidget(checkbox_controle);

        checkbox_mapeamento_oclusao = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_oclusao->setObjectName("checkbox_mapeamento_oclusao");

        verticalLayout->addWidget(checkbox_mapeamento_oclusao);

        checkbox_mapeamento_de_sombras = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_mapeamento_de_sombras->setObjectName("checkbox_mapeamento_de_sombras");

        verticalLayout->addWidget(checkbox_mapeamento_de_sombras);

        checkbox_ataque_vs_defesa_posicao_real = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_ataque_vs_defesa_posicao_real->setObjectName("checkbox_ataque_vs_defesa_posicao_real");

        verticalLayout->addWidget(checkbox_ataque_vs_defesa_posicao_real);

        checkbox_tab_ativa_ataque = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_tab_ativa_ataque->setObjectName("checkbox_tab_ativa_ataque");

        verticalLayout->addWidget(checkbox_tab_ativa_ataque);

        checkbox_desativar_som = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_desativar_som->setObjectName("checkbox_desativar_som");

        verticalLayout->addWidget(checkbox_desativar_som);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        checkbox_resolucao_fixa = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_resolucao_fixa->setObjectName("checkbox_resolucao_fixa");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(checkbox_resolucao_fixa->sizePolicy().hasHeightForWidth());
        checkbox_resolucao_fixa->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(checkbox_resolucao_fixa);

        combo_tamanho_buffer_principal = new QComboBox(ifg__qt__DialogoOpcoes);
        combo_tamanho_buffer_principal->addItem(QString());
        combo_tamanho_buffer_principal->addItem(QString());
        combo_tamanho_buffer_principal->addItem(QString());
        combo_tamanho_buffer_principal->addItem(QString());
        combo_tamanho_buffer_principal->setObjectName("combo_tamanho_buffer_principal");

        horizontalLayout->addWidget(combo_tamanho_buffer_principal);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        label_2 = new QLabel(ifg__qt__DialogoOpcoes);
        label_2->setObjectName("label_2");

        horizontalLayout_2->addWidget(label_2);

        combo_tamanho_texturas = new QComboBox(ifg__qt__DialogoOpcoes);
        combo_tamanho_texturas->addItem(QString());
        combo_tamanho_texturas->addItem(QString());
        combo_tamanho_texturas->addItem(QString());
        combo_tamanho_texturas->addItem(QString());
        combo_tamanho_texturas->setObjectName("combo_tamanho_texturas");

        horizontalLayout_2->addWidget(combo_tamanho_texturas);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        label = new QLabel(ifg__qt__DialogoOpcoes);
        label->setObjectName("label");

        horizontalLayout_3->addWidget(label);

        slider_escala = new QSlider(ifg__qt__DialogoOpcoes);
        slider_escala->setObjectName("slider_escala");
        slider_escala->setMaximum(4);
        slider_escala->setOrientation(Qt::Horizontal);

        horizontalLayout_3->addWidget(slider_escala);

        label_escala = new QLabel(ifg__qt__DialogoOpcoes);
        label_escala->setObjectName("label_escala");

        horizontalLayout_3->addWidget(label_escala);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        label_3 = new QLabel(ifg__qt__DialogoOpcoes);
        label_3->setObjectName("label_3");

        horizontalLayout_4->addWidget(label_3);

        combo_fps = new QComboBox(ifg__qt__DialogoOpcoes);
        combo_fps->addItem(QString());
        combo_fps->addItem(QString());
        combo_fps->addItem(QString());
        combo_fps->addItem(QString());
        combo_fps->setObjectName("combo_fps");

        horizontalLayout_4->addWidget(combo_fps);


        verticalLayout->addLayout(horizontalLayout_4);

        checkbox_desabilitar_retina = new QCheckBox(ifg__qt__DialogoOpcoes);
        checkbox_desabilitar_retina->setObjectName("checkbox_desabilitar_retina");

        verticalLayout->addWidget(checkbox_desabilitar_retina);

        buttonBox = new QDialogButtonBox(ifg__qt__DialogoOpcoes);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ifg__qt__DialogoOpcoes);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ifg__qt__DialogoOpcoes, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ifg__qt__DialogoOpcoes, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ifg__qt__DialogoOpcoes);
    } // setupUi

    void retranslateUi(QDialog *ifg__qt__DialogoOpcoes)
    {
        ifg__qt__DialogoOpcoes->setWindowTitle(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Dialog", nullptr));
        checkbox_mostrar_fps->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Mostrar Tempo de Desenho da cena", nullptr));
        checkbox_texturas_sempre_de_frente->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Texturas de Entidades Sempre de frente para c\303\242mera", nullptr));
        checkbox_iluminacao_mestre->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Ilumina\303\247\303\243o do mestre igual \303\240 dos jogadores", nullptr));
        checkbox_rosa_dos_ventos->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Desenha rosa dos ventos", nullptr));
        checkbox_anti_aliasing->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Anti Serrilhamento", nullptr));
        checkbox_grade->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Desenhar Grade", nullptr));
        checkbox_controle->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Desenhar Controle Virtual", nullptr));
        checkbox_mapeamento_oclusao->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Oclus\303\243o", nullptr));
        checkbox_mapeamento_de_sombras->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Mapeamento de sombras", nullptr));
        checkbox_ataque_vs_defesa_posicao_real->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Ataque vs Defesa posi\303\247\303\243o Real", nullptr));
        checkbox_tab_ativa_ataque->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Tab ativa ataque", nullptr));
        checkbox_desativar_som->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Desativar som", nullptr));
        checkbox_resolucao_fixa->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Renderizar 3d para textura", nullptr));
        combo_tamanho_buffer_principal->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "512", nullptr));
        combo_tamanho_buffer_principal->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "1024", nullptr));
        combo_tamanho_buffer_principal->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "2048", nullptr));
        combo_tamanho_buffer_principal->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "4096", nullptr));

#if QT_CONFIG(tooltip)
        combo_tamanho_buffer_principal->setToolTip(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Se verdadeiro, tabuleiro ser\303\241 renderizado para esta resolu\303\247\303\243o ao inv\303\251s do tamanho da janela.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        label_2->setToolTip(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Tamanho das texturas usadas para sombreamento e oclus\303\243o", nullptr));
#endif // QT_CONFIG(tooltip)
        label_2->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Tamanho de Texturas de Mapeamento", nullptr));
        combo_tamanho_texturas->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "128", nullptr));
        combo_tamanho_texturas->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "256", nullptr));
        combo_tamanho_texturas->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "512", nullptr));
        combo_tamanho_texturas->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "1024", nullptr));

        label->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Escala de Fonte", nullptr));
#if QT_CONFIG(tooltip)
        slider_escala->setToolTip(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Escala de fontes. Em sistemas normais \303\251 1, para sistemas com unidades l\303\263gica, como retina, esse valor pode variar. Use 0 para autom\303\241tico.", nullptr));
#endif // QT_CONFIG(tooltip)
        label_escala->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "0", nullptr));
        label_3->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Taxa de Atualiza\303\247\303\243o de Quadros (FPS)", nullptr));
        combo_fps->setItemText(0, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "20", nullptr));
        combo_fps->setItemText(1, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "30", nullptr));
        combo_fps->setItemText(2, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "45", nullptr));
        combo_fps->setItemText(3, QCoreApplication::translate("ifg::qt::DialogoOpcoes", "60", nullptr));

#if QT_CONFIG(tooltip)
        checkbox_desabilitar_retina->setToolTip(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Se marcado, n\303\243o usar\303\241 display de retina.", nullptr));
#endif // QT_CONFIG(tooltip)
        checkbox_desabilitar_retina->setText(QCoreApplication::translate("ifg::qt::DialogoOpcoes", "Desabilita Retina (n\303\243o funciona... ainda)", nullptr));
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
