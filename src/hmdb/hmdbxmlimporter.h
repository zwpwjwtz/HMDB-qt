#ifndef HMDBXMLIMPORTER_H
#define HMDBXMLIMPORTER_H


class HmdbXMLImporter
{
public:
    HmdbXMLImporter();
    ~HmdbXMLImporter();

    bool setSourceXML(const char* filename);
    bool setTargetDir(const char* directory);

    bool import();

    long count();

private:
    char* sourceXML;
    char* destDir;
    long lastCount;

    char* getEntryID(const char* content);
    bool writeEntry(const char* content, long length = 0);
};

#endif // HMDBXMLIMPORTER_H
