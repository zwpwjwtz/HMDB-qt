#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "hmdbxmlimporter.h"
#include "hmdbxml_def.h"
#include "utils/stdc.h"
#include "utils/filesystem.h"


HmdbXMLImporter::HmdbXMLImporter()
{
    sourceXML = nullptr;
    destDir = nullptr;
    finishedCallback = nullptr;
    progressCallback = nullptr;
    lastCount = 0;
}

HmdbXMLImporter::~HmdbXMLImporter()
{
    delete[] sourceXML;
    delete[] destDir;
}

bool HmdbXMLImporter::setSourceXML(const char* filename)
{
    if (!utils_isFile(filename))
        return false;

    if (sourceXML)
        delete[] sourceXML;

    sourceXML = new char[strlen(filename) + 1];
    strcpy(sourceXML, filename);
    lastCount = 0;
    return true;
}

bool HmdbXMLImporter::setTargetDir(const char* directory)
{
    if (!utils_isDirectory(directory))
        return false;

    if (destDir)
        delete[] destDir;

    destDir = new char[strlen(directory) + 1];
    strcpy(destDir, directory);
    return true;
}

bool HmdbXMLImporter::import()
{
    if (!sourceXML || !destDir)
    {
        if (finishedCallback != nullptr)
            finishedCallback(false);
        return false;
    }

    char* buffer;
    long readLength;
    unsigned long totalLength = utils_fileLength(sourceXML);

    lastCount = 0;
    FILE* source = fopen(sourceXML, "rb");
    while (!feof(source))
    {
        if (utils_fseekstr(HMDBXML_LIST_ENTRY_BEGIN, source) <= 0)
            break;

        buffer = nullptr;
        readLength = utils_getdelim(&buffer,
                                    nullptr,
                                    HMDBXML_LIST_ENTRY_END,
                                    source);
        if (readLength > 0)
        {
            if (!writeEntry(buffer, readLength))
            {
                free(buffer);
                fclose(source);
                return false;
            }
            free(buffer);
            lastCount++;
        }

        if (progressCallback != nullptr && totalLength > 0)
            progressCallback(100.0 * ftell(source) / totalLength);
    }
    fclose(source);

    if (finishedCallback != nullptr)
        finishedCallback(true);
    return true;
}

long HmdbXMLImporter::count()
{
    return lastCount;
}

void HmdbXMLImporter::setProgressCallback(std::function<void(double)> func)
{
    progressCallback = func;
}

void HmdbXMLImporter::setFinishedCallback(std::function<void(bool)> func)
{
    finishedCallback = func;
}

char* HmdbXMLImporter::getEntryID(const char* content)
{
    const char* pos1 = strstr(content, HMDBXML_LIST_ENTRY_ID_BEGIN);
    if (!pos1)
        return nullptr;
    pos1 += strlen(HMDBXML_LIST_ENTRY_BEGIN) - 1;

    const char* pos2 = strstr(pos1, HMDBXML_LIST_ENTRY_ID_END);
    if (!pos2 || pos2 < pos1)
        return nullptr;

    char* ID = new char[pos2 - pos1 + 1];
    utils_strncpy(ID, pos1, pos2 - pos1);
    return ID;
}

bool HmdbXMLImporter::writeEntry(const char* content, long length)
{
    char* entryID = getEntryID(content);
    if (!entryID)
        return false;

    if (length == 0)
        length = strlen(content);

    char* outputFile = new char[strlen(destDir) + strlen(entryID) +
                                strlen(HMDBXML_FILENAME_SUFFIX) + 2];
    strcpy(outputFile, destDir);
    strcat(outputFile, "/");
    strcat(outputFile, entryID);
    strcat(outputFile, HMDBXML_FILENAME_SUFFIX);

    long writeCount = 0;
    FILE* target = fopen(outputFile, "wb");
    if (!target)
        return false;
    fwrite(HMDBXML_HEADER, strlen(HMDBXML_HEADER), 1, target);
    fwrite(HMDBXML_LIST_ENTRY_BEGIN, strlen(HMDBXML_LIST_ENTRY_BEGIN), 1, target);
    writeCount = fwrite(content, length, 1, target);
    fwrite(HMDBXML_LIST_ENTRY_END, strlen(HMDBXML_LIST_ENTRY_END), 1, target);
    fclose(target);

    delete[] entryID;
    delete[] outputFile;
    return writeCount == 1;
}
