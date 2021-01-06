#ifndef FORMQUERY_H
#define FORMQUERY_H

#include <QWidget>
#include <QStandardItemModel>
#include "hmdb/hmdbquery.h"


namespace Ui {
class FormQuery;
}

class FormQuery : public QWidget
{
    Q_OBJECT

public:
    explicit FormQuery(QWidget *parent = nullptr);
    ~FormQuery();

    bool checkDatabase();

public slots:
    void setDataDirectory(QString dir);

protected:
    virtual void resizeEvent(QResizeEvent* event);

private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_buttonQueryID_clicked();
    void on_buttonSetDatabase_clicked();
    void on_checkRelativeTolerance_stateChanged(int arg1);
    void on_buttonQueryMass_clicked();
    void on_buttonQueryName_clicked();

private:
    Ui::FormQuery *ui;

    bool resultLoaded;
    QList<int> listColumnWidth;
    QString dataDir;
    HmdbQuery database;
    QStandardItemModel modelResult;
    
    void showQueryResult(const HmdbQueryRecord& record);

    void saveColumnWidth();
    void restoreColumnWidth();
};

#endif // FORMQUERY_H
