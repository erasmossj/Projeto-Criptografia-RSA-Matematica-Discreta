// mainwindow.cpp — janela principal (camada de interface).
//
// Só interface: lê os campos, chama o núcleo (include/rsa.h), grava os
// arquivos pelo módulo arquivos.h e mostra o resultado ou o erro.
// As contas com GMP que antes ficavam aqui estão em src/core/rsa.c.
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "arquivos.h"
#include "rsa.h"

#include <QByteArray>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>

static void ligaTudo(QWidget *parent){
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
    const QByteArray mensagem = ui->Codigo->toPlainText().toUtf8(); //Codigo
    const QByteArray chave_n = ui->ChavePrivada->text().toUtf8();
    const QByteArray chave_e = ui->ChavePrivada2->text().toUtf8();

    char *cifrado = nullptr;
    const rsa_status status = rsa_cifrar(reinterpret_cast<const unsigned char *>(mensagem.constData()),
                                         static_cast<size_t>(mensagem.size()),
                                         chave_n.constData(), chave_e.constData(), &cifrado);
    if (status != RSA_OK) {
        QMessageBox::warning(this, "Erro", status == RSA_ERRO_EXPOENTE_INVALIDO
                                               ? "Chave inválida (E)."
                                               : rsa_descricao_status(status));
        return;
    }

    const QByteArray resultado(cifrado);
    rsa_liberar(cifrado);

    if (!arquivos::salvarMensagemCriptografada(resultado)) {
        QMessageBox::warning(this, "Erro", "Não foi possível criar o arquivo para salvar a mensagem criptografada.");
        return;
    }

    ui->Resultado->setPlainText(QString::fromUtf8(resultado));

    QMessageBox::information(this, "Criptografia concluida com sucesso!", "Seu texto foi criptografado e salvo com sucesso na raiz do programa em mensagem_criptografada.txt");
}

void MainWindow::descriptografar() {
    const QByteArray mensagem = ui->Codigo->toPlainText().toUtf8();
    const QByteArray chave_n = ui->ChavePublica->text().toUtf8();
    const QByteArray chave_d = ui->ChavePublica2->text().toUtf8();

    unsigned char *texto = nullptr;
    size_t tamanho = 0;
    size_t invalidos = 0;
    const rsa_status status = rsa_decifrar(mensagem.constData(), chave_n.constData(), chave_d.constData(),
                                           &texto, &tamanho, &invalidos);
    if (status == RSA_ERRO_N_INVALIDO) {
        QMessageBox::warning(this, "Erro", "Chave pública inválida (N).");
        return;
    }
    if (status == RSA_ERRO_EXPOENTE_INVALIDO) {
        QMessageBox::warning(this, "Erro", "Chave privada inválida (D).");
        return;
    }
    if (status != RSA_OK) {
        QMessageBox::warning(this, "Erro", rsa_descricao_status(status));
        return;
    }

    const QByteArray resultado(reinterpret_cast<const char *>(texto), static_cast<qsizetype>(tamanho));
    rsa_liberar(texto);

    if (!arquivos::salvarMensagemDescriptografada(resultado)) {
        QMessageBox::warning(this, "Erro", "Não foi possível criar o arquivo para salvar a mensagem descriptografada.");
        return;
    }

    for (size_t i = 0; i < invalidos; i++) {
        QMessageBox::warning(this, "Erro de Descriptografia", "Valor inválido encontrado na mensagem.");
    }

    // a criptografia cifra os bytes UTF-8 do texto, então a volta é lida como UTF-8
    ui->Resultado->setPlainText(QString::fromUtf8(resultado));

    QMessageBox::information(this, "Descriptografia concluída com sucesso!", "Mensagem descriptografada e salva com sucesso na raiz do programa em mensagem_descriptografada.txt");
}

void MainWindow::Coprimos_E_Phi() {
    const QByteArray e_str = ui->entrada_E->text().toUtf8();
    const QByteArray phi_str = ui->saida_Phi->text().toUtf8();

    char *e = nullptr;
    char *d = nullptr;
    const rsa_status status = rsa_calcular_d(e_str.constData(), phi_str.constData(), &e, &d);

    if (status == RSA_OK) {
        const bool salvo = arquivos::anexarChavesED(QByteArray(e), QByteArray(d));
        const QString d_texto = QString::fromUtf8(d);

        QMessageBox::information(this, "Tudo Certinho", "O Sistema ja está pensando em um D");

        ui->saida_D->setText(d_texto);
        ui->Frame_3_D->setVisible(true);

        if (!salvo) {
            QMessageBox::warning(this, "Erro", "Não foi possível salvar as chaves em chaves.txt.");
        }
    } else if (status == RSA_ERRO_MEMORIA) {
        QMessageBox::warning(this, "Erro", rsa_descricao_status(status));
    } else {
        QMessageBox::warning(this, "Parado ai MELIANTE", "Ei, o Porque seu Phi e E nao sao Coprimos?");
    }

    rsa_liberar(e);
    rsa_liberar(d);

    QMessageBox::information(this, "Tudo certo!", "As chaves e os números primos foram salvos na pasta raiz do programa em chaves.txt");
}

void MainWindow::Primos_p_q() {
    const QByteArray p_str = ui->entrada_P->text().toUtf8();// p
    const QByteArray q_str = ui->entrada_Q->text().toUtf8();// q

    rsa_modulo modulo;
    const rsa_status status = rsa_calcular_modulo(p_str.constData(), q_str.constData(), &modulo);

    // chaves.txt é recriado a cada verificação, com P e Q (e N, se forem primos)
    const QByteArray p_arquivo = modulo.p ? QByteArray(modulo.p) : p_str.trimmed();
    const QByteArray q_arquivo = modulo.q ? QByteArray(modulo.q) : q_str.trimmed();
    const QByteArray n_arquivo = modulo.n ? QByteArray(modulo.n) : QByteArray();
    if (!arquivos::iniciarChaves(p_arquivo, q_arquivo, n_arquivo)) {
        QMessageBox::warning(this, "Erro", "Não foi possível salvar as chaves em chaves.txt.");
    }

    if (status == RSA_OK) {// se forem primos
        const QString phi = QString::fromUtf8(modulo.phi);
        const QString n_str = QString::fromUtf8(modulo.n);

        ui->saida_Phi->setText(phi);
        bool ok;
        QString e =
            QInputDialog::getText(this, "Insira o E", "Digite um numero CoPrimo a "+ phi +" (Phi)",
                                  QLineEdit::Normal,
                                  "",
                                  &ok//butao
                                  );

        if(ok){
            ui->Frame_2_E->setVisible(true);
            ui->entrada_E->setText(e);
            ui->Frame_1_Phi->setVisible(true);
            ui->Frame_4_N->setVisible(true);
            ui->saida_N->setText(n_str);
        }
    } else if (status == RSA_ERRO_MEMORIA) {
        QMessageBox::warning(this, "Erro", rsa_descricao_status(status));
    } else {
        QMessageBox::warning(this, "Parado ai MELIANTE", "Ei, o Porque seus P e Q não são primos?");
    }

    rsa_modulo_liberar(&modulo);
}
