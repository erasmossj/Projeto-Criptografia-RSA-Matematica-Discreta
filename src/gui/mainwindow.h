// mainwindow.h — janela principal (camada de interface).
//
// Recriado a partir do uso em mainwindow.cpp: o arquivo original nunca foi
// versionado no repositório.
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void criptografar();
    void descriptografar();
    void toggleMode();       // alterna entre criptografar e descriptografar
    void MenuBar_Troca_C();  // menu: página de criptografia
    void MenuBar_Troca_G();  // menu: página do gerador de chaves
    void Coprimos_E_Phi();   // verifica E e Phi e calcula D
    void Primos_p_q();       // verifica P e Q e calcula N e Phi

private:
    Ui::MainWindow *ui;
    bool modoCriptografia;   // true: criptografar; false: descriptografar
    bool modoMenu;           // true: página de criptografia; false: gerador
};

#endif // MAINWINDOW_H
