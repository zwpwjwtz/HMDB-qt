#include <QVector>
#include "hmdbqueryworker.h"
#include "utils/stdc.h"

#define HMDB_QUERY_WORKER_WORK_NONE            0
#define HMDB_QUERY_WORKER_WORK_INDEX_MAIN      1
#define HMDB_QUERY_WORKER_WORK_INDEX_MSMS      2
#define HMDB_QUERY_WORKER_WORK_QUERY_ID        101
#define HMDB_QUERY_WORKER_WORK_QUERY_NAME      102
#define HMDB_QUERY_WORKER_WORK_QUERY_MASS      103
#define HMDB_QUERY_WORKER_WORK_QUERY_MONOMASS  104
#define HMDB_QUERY_WORKER_WORK_QUERY_MSMS      105


HmdbQueryWorker::HmdbQueryWorker(QObject *parent) : QThread(parent)
{}

void HmdbQueryWorker::setDataDirectory(QString directory,
                                       HmdbQuery::DatabaseType type)
{
    query.setDataDirectory(directory.toLocal8Bit().constData(), type);
}

void HmdbQueryWorker::setQueryProperty(const QList<QString>& properties)
{
    int i;
    char** propertyList = new char*[properties.count()];
    QByteArray propertyString;
    for (i=0; i<properties.count(); i++)
    {
        propertyString = properties[i].toLocal8Bit();
        propertyList[i] = new char[propertyString.length() + 1];
        utils_strncpy(propertyList[i], propertyString.constData(),
                      propertyString.length());
    }
    query.setQueryProperty(propertyList, properties.count());

    for (i=0; i<properties.count(); i++)
        delete[] propertyList[i];
    delete[] propertyList;
}

void HmdbQueryWorker::getReady(HmdbQuery::DatabaseType type)
{
    switch (type)
    {
        case HmdbQuery::Main:
            workType= HMDB_QUERY_WORKER_WORK_INDEX_MAIN;
            break;
        case HmdbQuery::MSMS:
            workType= HMDB_QUERY_WORKER_WORK_INDEX_MSMS;
            break;
        default:;
    }
}

bool HmdbQueryWorker::isReady(HmdbQuery::DatabaseType type)
{
    return query.isReady(type);
}

void HmdbQueryWorker::queryByID(QString ID)
{
    workType= HMDB_QUERY_WORKER_WORK_QUERY_ID;
    optionStringList.clear();
    optionStringList.push_back(ID);
}

void HmdbQueryWorker::queryByName(QString name)
{
    workType= HMDB_QUERY_WORKER_WORK_QUERY_NAME;
    optionStringList.clear();
    optionStringList.push_back(name);
}

void HmdbQueryWorker::queryByMass(double min, double max)
{
    workType= HMDB_QUERY_WORKER_WORK_QUERY_MASS;
    optionDoubleList.clear();
    optionDoubleList.push_back(min);
    optionDoubleList.push_back(max);
}

void HmdbQueryWorker::queryByMonoMass(double min, double max)
{
    workType= HMDB_QUERY_WORKER_WORK_QUERY_MONOMASS;
    optionDoubleList.clear();
    optionDoubleList.push_back(min);
    optionDoubleList.push_back(max);
}

void HmdbQueryWorker::queryByMSMS(double tolerance,
                                  bool relativeTolerance,
                                  HmdbQuery::MassSpectrumMode mode,
                                  QList<double> mzList,
                                  QList<double> intensityList)
{
    workType= HMDB_QUERY_WORKER_WORK_QUERY_MSMS;
    optionDoubleList.clear();
    optionIntegerList.clear();

    optionDoubleList.push_back(tolerance);
    optionIntegerList.push_back(int(relativeTolerance));
    optionIntegerList.push_back(mode);
    optionIntegerList.push_back(mzList.length());
    optionIntegerList.push_back(intensityList.count());
    optionDoubleList.append(mzList);
    optionDoubleList.append(intensityList);
}


void HmdbQueryWorker::run()
{
    switch (workType)
    {
        case HMDB_QUERY_WORKER_WORK_INDEX_MAIN:
            query.getReady(HmdbQuery::DatabaseType::Main);
            emit ready(HmdbQuery::Main);
            return;
        case HMDB_QUERY_WORKER_WORK_INDEX_MSMS:
            query.getReady(HmdbQuery::DatabaseType::MSMS);
            emit ready(HmdbQuery::MSMS);
            return;
        case HMDB_QUERY_WORKER_WORK_QUERY_ID:
            lastRecord = query.queryID(optionStringList[0]
                                       .toLocal8Bit().constData());
            break;
        case HMDB_QUERY_WORKER_WORK_QUERY_NAME:
            lastRecord = query.queryName(optionStringList[0]
                                         .toLocal8Bit().constData());
            break;
        case HMDB_QUERY_WORKER_WORK_QUERY_MASS:
            lastRecord = query.queryMass(optionDoubleList[0],
                                         optionDoubleList[1]);
            break;
        case HMDB_QUERY_WORKER_WORK_QUERY_MONOMASS:
            lastRecord = query.queryMonoMass(optionDoubleList[0],
                                             optionDoubleList[1]);
            break;
        case HMDB_QUERY_WORKER_WORK_QUERY_MSMS:
        {
            int mzCount = optionIntegerList[2];
            int intensityCount = optionIntegerList[3];
            auto mzList(optionDoubleList.mid(1, mzCount).toVector());
            auto intensityList(optionDoubleList.mid(1 + mzCount, intensityCount)
                                               .toVector());
            lastRecord = query.queryMassSpectrum(optionDoubleList[0],
                                                 bool(optionIntegerList[0]),
                                                 HmdbQuery::MassSpectrumMode(
                                                        optionIntegerList[1]),
                                                 mzCount,
                                                 mzList.data(),
                                                 intensityCount > 0 ?
                                                 intensityList.data() :
                                                 nullptr);
            break;
        }
        default:;
    }
    emit finished(true);
}

HmdbQueryRecord& HmdbQueryWorker::result()
{
    return lastRecord;
}
