// main.cpp — ponto de entrada da interface Qt.
// O ícone vem do recurso Qt (assets/recursos.qrc), então não depende da pasta atual.
#include "mainwindow.h"
#include <QIcon>
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/cripto_rsa.ico"));

    MainWindow w;

    w.setWindowIcon(QIcon(":/cripto_rsa.ico"));

    w.show();
    return a.exec();
}
