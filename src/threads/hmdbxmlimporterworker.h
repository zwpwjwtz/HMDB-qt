#ifndef HMDBXMLIMPORTERWORKER_H
#define HMDBXMLIMPORTERWORKER_H

#include <QThread>
#include "hmdb/hmdbxmlimporter.h"


class HmdbXMLImporterWorker : public QThread
{
    Q_OBJECT
public:
    explicit HmdbXMLImporterWorker(QObject *parent = nullptr);

    bool setSourceXML(QString filename);
    bool setTargetDir(QString directory);

    virtual void run();

signals:
    void progressed(double finishedPercent);
    void finished(bool successful);

private:
    HmdbXMLImporter importer;

    void onImporterProgressed(double finishedPercent);
};

#endif // HMDBXMLIMPORTERWORKER_H
