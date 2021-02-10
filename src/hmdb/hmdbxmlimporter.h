#ifndef HMDBXMLIMPORTER_H
#define HMDBXMLIMPORTER_H

#include <functional>


class HmdbXMLImporter
{
public:
    HmdbXMLImporter();
    ~HmdbXMLImporter();

    bool setSourceXML(const char* filename);
    bool setTargetDir(const char* directory);

    bool import();
    void stop();

    long count();

    void setProgressCallback(std::function<void(double)> func);
    void setFinishedCallback(std::function<void(bool)> func);

private:
    char* sourceXML;
    char* destDir;
    long lastCount;
    std::function<void(double)> progressCallback;
    std::function<void(bool)> finishedCallback;

    char* getEntryID(const char* content);
    bool writeEntry(const char* content, long length = 0);
};

#endif // HMDBXMLIMPORTER_H
