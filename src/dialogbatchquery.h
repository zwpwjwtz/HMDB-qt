#ifndef DIALOGBATCHQUERY_H
#define DIALOGBATCHQUERY_H

#include <QWizard>
#include <QStandardItemModel>


namespace Ui {
class DialogBatchQuery;
}

class DialogBatchQuery : public QWizard
{
    Q_OBJECT

public:
    explicit DialogBatchQuery(QWidget *parent = nullptr);
    ~DialogBatchQuery();

protected:
    virtual void hideEvent(QHideEvent* event);
    virtual bool validateCurrentPage();

private slots:
    void on_buttonSelectSourcePath_clicked();
    void on_buttonSelectTargetPath_clicked();
    void on_buttonViewResult_clicked();

private:
    Ui::DialogBatchQuery *ui;
    QStandardItemModel modelMassModification;
    QStandardItemModel modelQueryFields;

    void resetMassModification();
    void resetQueryFields();
};

#endif // DIALOGBATCHQUERY_H
