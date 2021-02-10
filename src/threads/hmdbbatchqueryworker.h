#ifndef HMDBBATCHQUERYWORKER_H
#define HMDBBATCHQUERYWORKER_H

#include <QThread>
#include "hmdb/hmdbbatchquery.h"


class HmdbBatchQueryWorker : public QThread
{
    Q_OBJECT
public:
    enum QueryType
    {
        None = 0,
        ByMass = 1
    };

    explicit HmdbBatchQueryWorker(QObject *parent = nullptr);

    void setSourcePath(QString filename);
    void setDataDirectory(QString directory);
    void setResultPath(QString filename);

    void setQueryType(QueryType type);

    void setQueryProperty(char** properties, int propertyCount);

    void setOption(const char* name, const char* value = nullptr);
    void clearOptions();

    HmdbQueryStatus status();
    void run();
    void stop();

signals:
    void progressed(double finishedPercent);
    void finished(bool successful);

private:
    HmdbBatchQuery query;
    QueryType queryType;
    QString sourcePath;
    QString targetPath;

    void onQueryProgressed(double finishedPercent);
};

#endif // HMDBBATCHQUERYWORKER_H
