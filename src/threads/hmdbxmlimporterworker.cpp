#include "hmdbxmlimporterworker.h"


HmdbXMLImporterWorker::HmdbXMLImporterWorker(QObject *parent) : QThread(parent)
{
    importer.setProgressCallback(
                std::bind(&HmdbXMLImporterWorker::onImporterProgressed,
                          this, std::placeholders::_1));
}

bool HmdbXMLImporterWorker::setSourceXML(QString filename)
{
    return importer.setSourceXML(filename.toLocal8Bit().constData());
}

bool HmdbXMLImporterWorker::setTargetDir(QString directory)
{
    return importer.setTargetDir(directory.toLocal8Bit().constData());
}

void HmdbXMLImporterWorker::run()
{
    if (importer.import())
        emit finished(true);
    else
        emit finished(false);
}

void HmdbXMLImporterWorker::onImporterProgressed(double finishedPercent)
{
    emit progressed(finishedPercent);
}
