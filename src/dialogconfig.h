#ifndef DIALOGCONFIG_H
#define DIALOGCONFIG_H

#include <QDialog>

namespace Ui {
class DialogConfig;
}

class DialogConfig : public QDialog
{
    Q_OBJECT

public:
    explicit DialogConfig(QWidget *parent = nullptr);
    ~DialogConfig();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DialogConfig *ui;

private slots:
    void on_buttonSetMainDatabase_clicked();
    void on_buttonSetMSMSDatabase_clicked();
};

#endif // DIALOGCONFIG_H
