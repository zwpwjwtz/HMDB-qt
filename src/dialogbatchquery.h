#ifndef DIALOGBATCHQUERY_H
#define DIALOGBATCHQUERY_H

#include <QWizard>
#include <QStandardItemModel>


class HmdbBatchQuery;
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

    virtual void hideEvent(QHideEvent* event);
    virtual bool validateCurrentPage();

private slots:
    void onBatchQueryProgressed();

    void on_buttonSelectSourcePath_clicked();
    void on_buttonSelectTargetPath_clicked();
    void on_buttonViewResult_clicked();
    void on_DialogBatchQuery_currentIdChanged(int id);
    void on_buttonSelectDatabase_clicked();

private:
    Ui::DialogBatchQuery *ui;
    ControlMassModificationList* massModificationList;
    QStandardItemModel modelQueryFields;
    HmdbBatchQuery* searchEngine;
    bool lastQuerySuccessful;
    QString dataDir;

    void resetQueryFields();

    bool launchQuery();
};

#endif // DIALOGBATCHQUERY_H
