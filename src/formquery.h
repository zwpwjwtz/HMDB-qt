#ifndef FORMQUERY_H
#define FORMQUERY_H

#include <QWidget>
#include <QStandardItemModel>
#include "utils/hmdbquery.h"


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

private:
    Ui::FormQuery *ui;

    QString dataDir;
    HmdbQuery database;
    QStandardItemModel modelResult;
    
    void showQueryResult(const HmdbQueryRecord& record);
};

#endif // FORMQUERY_H
