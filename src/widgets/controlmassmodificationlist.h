#ifndef CONTROLMASSMODIFICATIONLIST_H
#define CONTROLMASSMODIFICATIONLIST_H

#include <QWidget>
#include <QStandardItemModel>


class QMenu;
namespace Ui {
class ControlMassModificationList;
}

class ControlMassModificationList : public QWidget
{
    Q_OBJECT

public:
    explicit ControlMassModificationList(QWidget *parent = nullptr);
    ~ControlMassModificationList();

    bool loadList(bool reset, QString filename);

    QList<int> selectedIndexes();

    QString mz(int index);
    QString formula(int index);
    QString name(int index);

private:
    Ui::ControlMassModificationList *ui;
    QMenu* menuLoadMassModification;

    QString massModificationListDir;
    QStandardItemModel model;

private slots:
    void onMenuLoadActionClicked(QAction* action);

    void on_buttonAdd_clicked();
    void on_buttonRemove_clicked();
    void on_buttonLoad_clicked();
    void on_buttonSaveAs_clicked();
};

#endif // CONTROLMASSMODIFICATIONLIST_H
