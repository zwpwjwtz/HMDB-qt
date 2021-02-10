#ifndef DIALOGBATCHQUERY_H
#define DIALOGBATCHQUERY_H

#include <QWizard>
#include <QStandardItemModel>


class HmdbBatchQueryWorker;
class ControlMassModificationList;

namespace Ui {
class DialogBatchQuery;
}

class DialogBatchQuery : public QWizard
{
    Q_OBJECT

public:
    explicit DialogBatchQuery(QWidget *parent = nullptr);
    ~DialogBatchQuery();

public slots:
    void setDataDirectory(QString dir);

protected:
    enum WizardPage
    {
        PageQueryType,
        PageSelectFile,
        PageQueryOption,
        PageQueryField,
        PageQueryProgress,
        PageQueryFinished
    };

    virtual void showEvent(QShowEvent* event);
    virtual bool validateCurrentPage();

private slots:
    void done(int r);

    void onBatchQueryProgressed(double progress);
    void onBatchQueryFinished(bool successful);

    void on_buttonSelectSourcePath_clicked();
    void on_buttonSelectTargetPath_clicked();
    void on_buttonViewResult_clicked();
    void on_DialogBatchQuery_currentIdChanged(int id);
    void on_buttonSelectDatabase_clicked();

private:
    Ui::DialogBatchQuery *ui;
    QStandardItemModel modelQueryFields;
    ControlMassModificationList* massModificationList;
    HmdbBatchQueryWorker* searchEngine;
    QString dataDir;
    bool lastQuerySuccessful;

    void resetQueryFields();

    bool launchQuery();
    void stopQuery();
};

#endif // DIALOGBATCHQUERY_H
