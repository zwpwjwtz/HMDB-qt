#ifndef FORMQUERY_H
#define FORMQUERY_H

#include <QWidget>
#include "hmdb/hmdbquery.h"


class HmdbQueryWorker;
class ControlMSSearchOption;
class ControlQueryField;
class FormMetaboliteViewer;

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

protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void resizeEvent(QResizeEvent* event);

private slots:
    void onDatabaseReady();
    void onFieldListRequested();
    void onQueryFinished(bool successful);
    void onResultDetailsRequested(QString ID);

    void on_comboBox_currentIndexChanged(int index);
    void on_buttonQueryID_clicked();
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
    ControlQueryField* listQueryField;
    FormMetaboliteViewer* viewer;

    QString msmsFilePath;
    HmdbQueryWorker* database;

    void showBuildingIndexStart();
    void showQueryStart(int queryType);
    void showQueryResult(const HmdbQueryRecord& record, bool showRank = false);
    void startQuery();
    void stopQuery();

    static bool parsePeakList (QByteArray content,
                               QList<double>& mzList,
                               QList<double>& intensityList);
};

#endif // FORMQUERY_H
