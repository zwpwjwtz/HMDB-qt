#ifndef hmdbqueryworker_h
#define hmdbqueryworker_h

#include <QThread>
#include "hmdb/hmdbquery.h"


class HmdbQueryWorker : public QThread
{
    Q_OBJECT
public:
    explicit HmdbQueryWorker(QObject *parent = nullptr);

    void setDataDirectory(QString directory,
                          HmdbQuery::DatabaseType type);

    void setQueryProperty(char** properties, int propertyCount);

    void getReady(HmdbQuery::DatabaseType type);
    bool isReady(HmdbQuery::DatabaseType type);

    void queryByID(QString ID);
    void queryByName(QString name);
    void queryByMass(double min, double max);
    void queryByMonoMass(double min, double max);
    void queryByMSMS(double tolerance,
                     bool relativeTolerance,
                     HmdbQuery::MassSpectrumMode mode,
                     QList<double> mzList,
                     QList<double> intensityList);

    void run();

    HmdbQueryRecord& result();

signals:
    void ready(HmdbQuery::DatabaseType databaseType);
    void finished(bool successful);

private:
    QString sourcePath;
    QString targetPath;

    int workType;
    QList<QString> optionStringList;
    QList<int> optionIntegerList;
    QList<double> optionDoubleList;

    HmdbQuery query;
    HmdbQueryRecord lastRecord;
};

#endif // hmdbqueryworker_h
