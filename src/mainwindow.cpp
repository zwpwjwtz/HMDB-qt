#include <QDesktopServices>
#include <QUrl>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "dialogconfig.h"

#define HMDB_DOWNLOAD_URL    "https://hmdb.ca/downloads"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_buttonImport_clicked()
{
    windowImport.show();
}

void MainWindow::on_buttonDownload_clicked()
{
    QDesktopServices::openUrl(QUrl(HMDB_DOWNLOAD_URL));
}

void MainWindow::on_buttonQuery_clicked()
{
    windowQuery.show();
}

void MainWindow::on_buttonBatchQuery_clicked()
{
    windowBatchQuery.show();
}

void MainWindow::on_buttonAbout_clicked()
{
    windowAbout.show();
}

void MainWindow::on_buttonConfigure_clicked()
{
    dialogConfig->show();
}
