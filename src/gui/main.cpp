#include "mainwindow.h"
#include <QIcon>
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon("cripto_rsa.ico"));

    MainWindow w;

    w.setWindowIcon(QIcon("cripto_rsa.ico"));

    w.show();
    return a.exec();
}
