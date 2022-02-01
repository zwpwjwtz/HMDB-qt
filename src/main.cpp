#include <QApplication>
#include <QTranslator>
#include "global.h"
#include "mainwindow.h"
#include "dialogconfig.h"


QTranslator appTranslator;
HmdbQtConfig appConfig;
DialogConfig* dialogConfig;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    appTranslator.load(QString(":/translations/HMDB-qt_")
                       .append(QLocale::system().name()));
    a.installTranslator(&appTranslator);

    dialogConfig = new DialogConfig;
    MainWindow w;
    w.show();

    return a.exec();
}
