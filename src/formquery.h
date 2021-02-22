#ifndef FORMQUERY_H
#define FORMQUERY_H

#include <QWidget>
#include "hmdb/hmdbquery.h"


class QStandardItemModel;
class QSortFilterProxyModel;
class HmdbQueryWorker;
class ControlMSSearchOption;

namespace Ui {
class FormQuery;
}

class FormQuery : public QWidget
{
    Q_OBJECT

public:
    explicit FormQuery(QWidget *parent = nullptr);
    ~FormQuery();

    bool connectDatabase();
    bool checkDatabase();
    bool checkMSMSDatabase();

public slots:
    void setDataDirectory(QString dir);

protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

private slots:
    void onDatabaseReady();
    void onQueryFinished(bool successful);
    void onViewHeaderSearchResultClicked(int columnIndex);

    void on_comboBox_currentIndexChanged(int index);
    void on_buttonQueryID_clicked();
    void on_buttonSetDatabase_clicked();
    void on_checkRelativeTolerance_stateChanged(int arg1);
    void on_buttonQueryMass_clicked();
    void on_buttonQueryName_clicked();
    void on_buttonSelectMSMSFile_clicked();
    void on_radioMSMSFromFile_toggled(bool checked);
    void on_buttonOptionQueryMSMS_clicked();
    void on_buttonQueryMSMS_clicked();

private:
    Ui::FormQuery *ui;
    ControlMSSearchOption* widgetMSSearchOption;

    bool resultLoaded;
    QList<int> listColumnWidth;
    QList<bool> columnSortAscending;
    QString dataDir;
    QString msmsDataDir;
    QString msmsFilePath;
    HmdbQueryWorker* database;
    QStandardItemModel* modelResult;
    QSortFilterProxyModel* modelResultProxy;

    void showBuildingIndexStart();
    void showQueryStart(int queryType);
    void showQueryResult(const HmdbQueryRecord& record, bool showRank = false);
    void stopQuery();

    void saveColumnWidth();
    void restoreColumnWidth();

    static bool parsePeakList (QByteArray content,
                               QList<double>& mzList,
                               QList<double>& intensityList);
};

#endif // FORMQUERY_H
