#include <QApplication>
#include "global.h"
#include "mainwindow.h"
#include "dialogconfig.h"


HmdbQtConfig appConfig;
DialogConfig* dialogConfig;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    dialogConfig = new DialogConfig;
    MainWindow w;
    w.show();

    return a.exec();
}
