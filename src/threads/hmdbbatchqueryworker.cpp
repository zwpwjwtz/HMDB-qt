#include "hmdbbatchqueryworker.h"


HmdbBatchQueryWorker::HmdbBatchQueryWorker(QObject *parent) : QThread(parent)
{
    query.setProgressCallback(
                std::bind(&HmdbBatchQueryWorker::onQueryProgressed,
                          this, std::placeholders::_1));
}

void HmdbBatchQueryWorker::setSourcePath(QString filename)
{
    query.setSourcePath(filename.toLocal8Bit().constData());
}

void HmdbBatchQueryWorker::setDataDirectory(QString directory)
{
    query.setDataDirectory(directory.toLocal8Bit().constData());
}

void HmdbBatchQueryWorker::setResultPath(QString filename)
{
    query.setResultPath(filename.toLocal8Bit().constData());
}

void HmdbBatchQueryWorker::setQueryType(QueryType type)
{
    queryType = type;
}

void HmdbBatchQueryWorker::setQueryProperty(char** properties,
                                            int propertyCount)
{
    query.setQueryProperty(properties, propertyCount);
}

void HmdbBatchQueryWorker::setOption(const char* name, const char* value)
{
    query.setOption(name, value);
}

void HmdbBatchQueryWorker::clearOptions()
{
    query.clearOptions();
}

HmdbQueryStatus HmdbBatchQueryWorker::status()
{
    return query.status();
}

void HmdbBatchQueryWorker::run()
{
    switch (queryType)
    {
        case QueryType::ByMass:
            if (query.queryMass())
                emit finished(true);
            else
                emit finished(false);
            break;
        default:;
    }
}

void HmdbBatchQueryWorker::stop()
{
    query.stopQuery();
}

void HmdbBatchQueryWorker::onQueryProgressed(double finishedPercent)
{
    emit progressed(finishedPercent);
}
