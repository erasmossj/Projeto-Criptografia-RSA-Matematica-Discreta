#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include "cstdio"

#include "backend/CRIPTO_RSA.h"

void ligaTudo(QWidget *parent){
    for(QWidget *child : parent->findChildren<QWidget*>()) {
        child->setEnabled(true);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    modoCriptografia = true; //cripto

    modoMenu = true;// começa no cripto

    ligaTudo(this);

    ui->Frame_1_Phi->setVisible(false);
    ui->Frame_2_E->setVisible(false);
    ui->Frame_3_D->setVisible(false);
    ui->Frame_4_N->setVisible(false);

    // Conectar os botões aos slots
    connect(ui->btn_Cripto, &QPushButton::clicked, this, &MainWindow::criptografar);
    connect(ui->btn_Descripto, &QPushButton::clicked, this, &MainWindow::descriptografar);
    connect(ui->TrocarModo, &QPushButton::clicked, this, &MainWindow::toggleMode);
    connect(ui->menu_button_crip, &QPushButton::clicked, this, &MainWindow::MenuBar_Troca_C);
    connect(ui->menu_button_gera, &QPushButton::clicked, this, &MainWindow::MenuBar_Troca_G);
    connect(ui->button_PhiE_verif, &QPushButton::clicked, this, &MainWindow::Coprimos_E_Phi);
    connect(ui->button_PQ_verif, &QPushButton::clicked, this, &MainWindow::Primos_p_q);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleMode() {
    modoCriptografia = !modoCriptografia; // Alterna
    QString nada ="";
    if (modoCriptografia) {
        ui->decripto->setCurrentIndex(0); //cripto
        ui->Resultado->setPlainText(nada);
    } else {
        ui->decripto->setCurrentIndex(1); //descripto
        ui->Resultado->setPlainText(nada);
    }
}

void MainWindow::MenuBar_Troca_C() {
    if (!modoMenu) {
        ui->menu->setCurrentIndex(0); //cripto

        QString nada = "";
        ui->saida_Phi->setText(nada);
        ui->entrada_E->setText(nada);
        ui->saida_D->setText(nada);
        ui->entrada_P->setText(nada);
        ui->entrada_Q->setText(nada);

        modoMenu = !modoMenu; // Alterna
    }
}

void MainWindow::MenuBar_Troca_G() {
    if (modoMenu) {
        ui->menu->setCurrentIndex(1); //gerador
        modoMenu = !modoMenu;
    }
}

void MainWindow::criptografar() {
    QString mensagem = ui->Codigo->toPlainText(); //Codigo
    QString chave_n = ui->ChavePrivada->text();
    QString chave_e = ui->ChavePrivada2->text();

    mpz_t n, e;
    mpz_init(n);
    mpz_init(e);

    std::string converterGMP;

    converterGMP = chave_n.toStdString();
    mpz_set_str(n, converterGMP.c_str(), 10);

    converterGMP = chave_e.toStdString();
    mpz_set_str(e, converterGMP.c_str(), 10);

    converterGMP.clear();

    FILE *mensagem_criptografada;
    mensagem_criptografada = fopen("mensagem_criptografada.txt", "w");

    QString resultado;

    std::string mensagem_str = mensagem.toStdString();

    int i = 0;

    while(i < mensagem_str.size()){
        char m_char = mensagem_str[i];

        mpz_t m, c;
        mpz_init_set_ui(m, (unsigned char) m_char);
        mpz_init(c);

        mod_exp(c, m, e, n);

        gmp_fprintf(mensagem_criptografada, "%Zd ", c);

        char* c_str = mpz_get_str(nullptr, 10, c);
        resultado.append(c_str);
        resultado.append(" ");

        free(c_str);
        mpz_clear(m);
        mpz_clear(c);

        i++;
    }

    fclose(mensagem_criptografada);

    ui->Resultado->setPlainText(resultado);

    QMessageBox::information(this, "Criptografia concluida com sucesso!", "Seu texto foi criptografado e salvo com sucesso na raiz do programa em mensagem_criptografada.txt");

    resultado.clear();
    mpz_clear(n);
    mpz_clear(e);
}

void MainWindow::descriptografar() {
    QString mensagem = ui->Codigo->toPlainText();
    QString chave_n = ui->ChavePublica->text();
    QString chave_d = ui->ChavePublica2->text();

    mpz_t n, d;
    mpz_init(n);
    mpz_init(d);

    std::string converterGMP = chave_n.toStdString();
    if (mpz_set_str(n, converterGMP.c_str(), 10) != 0) {
        QMessageBox::warning(this, "Erro", "Chave pública inválida (N).");
        mpz_clear(n);
        mpz_clear(d);
        return;
    }

    converterGMP = chave_d.toStdString();
    if (mpz_set_str(d, converterGMP.c_str(), 10) != 0) {
        QMessageBox::warning(this, "Erro", "Chave privada inválida (D).");
        mpz_clear(n);
        mpz_clear(d);
        return;
    }

    QStringList lista_inteiros = mensagem.split(" ", Qt::SkipEmptyParts);
    QString resultado;

    FILE* arquivo = fopen("mensagem_descriptografada.txt", "w");
    if (!arquivo) {
        QMessageBox::warning(this, "Erro", "Não foi possível criar o arquivo para salvar a mensagem descriptografada.");
        mpz_clear(n);
        mpz_clear(d);
        return;
    }

    auto i = lista_inteiros.begin();
    while (i != lista_inteiros.end()) {
        mpz_t c, m;
        mpz_init(c);
        mpz_init(m);

        std::string c_strInt = i->toStdString();
        if (mpz_set_str(c, c_strInt.c_str(), 10) != 0) {
            qDebug() << "Erro ao definir c com o valor:" << *i;
            QMessageBox::warning(this, "Erro de Descriptografia", "Valor inválido encontrado na mensagem.");
            mpz_clear(c);
            mpz_clear(m);
            i++;
            continue;
        }

        mod_exp(m, c, d, n);

        if (mpz_cmp_ui(m, 0) > 0) {
            char* m_str = mpz_get_str(nullptr, 10, m);
            int caracter_descriptografado = static_cast<int>(mpz_get_ui(m));
            resultado.append(QChar(caracter_descriptografado));

            fprintf(arquivo, "%c", caracter_descriptografado);

            free(m_str);
        } else {
            qDebug() << "O valor de m é zero ou negativo.";
        }

        mpz_clear(c);
        mpz_clear(m);

        i++;
    }

    fclose(arquivo);

    ui->Resultado->setPlainText(resultado);

    QMessageBox::information(this, "Descriptografia concluída com sucesso!", "Mensagem descriptografada e salva com sucesso na raiz do programa em mensagem_descriptografada.txt");

    lista_inteiros.clear();
    resultado.clear();
    mpz_clear(n);
    mpz_clear(d);
}

void MainWindow::Coprimos_E_Phi() {
    QString e_str = ui->entrada_E->text();
    QString phi_str = ui->saida_Phi->text();

    mpz_t e, phi, d, resultado_mdc;
    mpz_init(e);
    mpz_init(phi);
    mpz_init(d);
    mpz_init(resultado_mdc);

    std::string converterGMP;

    FILE *chaves;
    chaves = fopen("chaves.txt", "a");

    converterGMP = e_str.toStdString();
    mpz_set_str(e, converterGMP.c_str(), 10);

    converterGMP = phi_str.toStdString();
    mpz_set_str(phi, converterGMP.c_str(), 10);

    mdc(resultado_mdc, e, phi);

    if(mpz_cmp_ui(resultado_mdc, 1) == 0){
        inverso_modular(d, e, phi);

        gmp_fprintf(chaves, "E: %Zd\n", e);
        gmp_fprintf(chaves, "D: %Zd", d);

        char* d_str = mpz_get_str(nullptr, 10, d);

        QString d = d_str;

        free(d_str);

        QMessageBox::information(this, "Tudo Certinho", "O Sistema ja está pensando em um D");

        ui->saida_D->setText(d);
        ui->Frame_3_D->setVisible(true);

    }else{
        QMessageBox::warning(this, "Parado ai MELIANTE", "Ei, o Porque seu Phi e E nao sao Coprimos?");
    }

    mpz_clear(e);
    mpz_clear(phi);
    mpz_clear(d);
    mpz_clear(resultado_mdc);

    QMessageBox::information(this, "Tudo certo!", "As chaves e os números primos foram salvos na pasta raiz do programa em chaves.txt");

    fclose(chaves);
}

void MainWindow::Primos_p_q() {
    QString p_str = ui->entrada_P->text();// p
    QString q_str = ui->entrada_Q->text();// q

    std::string ConverterGMP;

    mpz_t p, q, n, e, d, phi;

    mpz_init(p);
    mpz_init(q);
    mpz_init(n);
    mpz_init(phi);

    FILE *chaves;
    chaves = fopen("chaves.txt", "w");

    ConverterGMP = p_str.toStdString();
    mpz_set_str(p, ConverterGMP.c_str(), 10);

    ConverterGMP = q_str.toStdString();
    mpz_set_str(q, ConverterGMP.c_str(), 10);

    gmp_fprintf(chaves, "P: %Zd\n", p);
    gmp_fprintf(chaves, "Q: %Zd\n", q);

    if(eh_primo(p) && eh_primo(q)){// se forem primos

        mpz_mul(n, p, q); // n = p * q
        mpz_sub_ui(p, p, 1); // p = p - 1
        mpz_sub_ui(q, q, 1); // q = q - 1
        mpz_mul(phi, p, q); // phi = (p - 1) * (q - 1)

        gmp_fprintf(chaves, "N: %Zd\n", n);

        char* phi_str = mpz_get_str(nullptr, 10, phi);
        char* n_char = mpz_get_str(nullptr, 10, n);

        QString phi = phi_str;

        QString n_str = n_char;

        free(phi_str);

        ui->saida_Phi->setText(phi);
        bool ok;
        QString e =
            QInputDialog::getText(this, "Insira o E", "Digite um numero CoPrimo a "+ phi +" (Phi)",
                                     QLineEdit::Normal,
                                     "",

        //  QInputDialog::getInt(this, "Insira o E", "Digite um numero CoPrimo a "+ phi +" (Phi)",
            //                              0,// inicial
            //                              0,// minimo
            //                              32000,// maximo
            //                              1,// incremento por setada
                                    &ok//butao
                                    );

        if(ok){
            ui->Frame_2_E->setVisible(true);
            ui->entrada_E->setText(e);
            ui->Frame_1_Phi->setVisible(true);
            ui->Frame_4_N->setVisible(true);
            ui->saida_N->setText(n_str);
        }
    }else{
        QMessageBox::warning(this, "Parado ai MELIANTE", "Ei, o Porque seus P e Q não são primos?");
    }

    fclose(chaves);

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(n);
    mpz_clear(phi);
    mpz_clear(e);
    mpz_clear(d);
}
