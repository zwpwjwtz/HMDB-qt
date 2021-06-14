#ifndef CONTROLQUERYFIELD_H
#define CONTROLQUERYFIELD_H

#include <QWidget>


class QStandardItemModel;

namespace Ui {
class ControlQueryField;
}

class ControlQueryField : public QWidget
{
    Q_OBJECT

public:
    QList<QString> selectedFieldIDList;
    QList<QString> selectedFieldNameList;

    explicit ControlQueryField(QWidget *parent = nullptr);
    ~ControlQueryField();

protected:
    void changeEvent(QEvent* e);
    void keyPressEvent(QKeyEvent* event);
    void showEvent(QShowEvent* event);

private slots:
    void on_buttonOK_clicked();
    void on_buttonCancel_clicked();

private:
    Ui::ControlQueryField *ui;
    QStandardItemModel* modelFieldList;

    void addItem(const QString& ID, const QString& text, bool enabled = false);
    void loadDefaultList();
};

#endif // CONTROLQUERYFIELD_H
