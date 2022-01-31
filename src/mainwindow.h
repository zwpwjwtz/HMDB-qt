#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialogimport.h"
#include "formquery.h"
#include "dialogbatchquery.h"
#include "dialogabout.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_buttonImport_clicked();
    void on_buttonDownload_clicked();
    void on_buttonQuery_clicked();
    void on_buttonBatchQuery_clicked();
    void on_buttonAbout_clicked();

private:
    Ui::MainWindow *ui;
    DialogImport windowImport;
    FormQuery windowQuery;
    DialogBatchQuery windowBatchQuery;
    DialogAbout windowAbout;
};

#endif // MAINWINDOW_H
