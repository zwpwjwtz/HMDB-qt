#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialogimport.h"
#include "formquery.h"


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

private:
    Ui::MainWindow *ui;
    DialogImport windowImport;
    FormQuery windowQuery;
};

#endif // MAINWINDOW_H
