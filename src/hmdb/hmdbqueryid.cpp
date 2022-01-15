#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "hmdbqueryid.h"
#include "hmdbdatabase.h"
#include "hmdbxml_def.h"
#include "utils/filesystem.h"
#include "utils/stdc.h"

#define HMDB_QUERY_INDEX_ID_FILENAME    "index-id"
#define HMDB_QUERY_INDEX_ID_HEADER      "HMDBINDEX\n"
#define HMDB_QUERY_INDEX_ID_HEADER_LEN  16
#define HMDB_QUERY_INDEX_ID_VERSION     "1.0\n"
#define HMDB_QUERY_INDEX_ID_TYPE        "01"
#define HMDB_QUERY_INDEX_ID_SEP_ENTRY   "\n"


HmdbQueryID::HmdbQueryID()
{
    indexFileName = nullptr;
}

HmdbQueryID::HmdbQueryID(const char* dataDir)
{
    indexFileName = nullptr;
    setDatabase(dataDir);
}

HmdbQueryID::~HmdbQueryID()
{
    delete[] indexFileName;
}

void HmdbQueryID::setDatabase(const char* path)
{
    if (!path)
        return;
    dataDir = path;

    if (indexFileName)
        delete[] indexFileName;

    indexFileName = new char[strlen(dataDir) +
                             strlen(HMDB_QUERY_INDEX_ID_FILENAME) + 2];
    strcpy(indexFileName, dataDir);
    strcat(indexFileName, "/");
    strcat(indexFileName, HMDB_QUERY_INDEX_ID_FILENAME);
}

bool HmdbQueryID::buildIndex()
{
    if (!dataDir)
        return false;

    FILE* f = fopen(indexFileName, "wb");
    if (!f)
        return false;

    // Write a file header
    fputs(HMDB_QUERY_INDEX_ID_HEADER, f);
    fputs(HMDB_QUERY_INDEX_ID_VERSION, f);
    fputs(HMDB_QUERY_INDEX_ID_TYPE, f);

    // Get names of all files in the data directory
    auto dataFileList = utils_listDirectoryFiles(dataDir);

    // Write entries
    char* ID;
    const int separatorLength = strlen(HMDB_QUERY_INDEX_ID_SEP_ENTRY);
    for (auto i=dataFileList.cbegin(); i!=dataFileList.cend(); i++)
    {
        if (strstr((*i).c_str(), HMDBXML_FILENAME_SUFFIX))
        {
            // This is a data (XML) file; add it to index
            ID = HmdbDatabase::getIDByFilename((*i).c_str());
            fputs(ID, f);
            fwrite(HMDB_QUERY_INDEX_ID_SEP_ENTRY, separatorLength, 1, f);
            delete[] ID;
        }
    }

    fclose(f);
    return true;
}

bool HmdbQueryID::existIndex()
{
    if (!indexFileName)
        return false;

    FILE* f = fopen(indexFileName, "rb");
    if (!f)
        return false;

    bool fileValid = true;
    char buffer[HMDB_QUERY_INDEX_ID_HEADER_LEN + 1];
    fread(buffer, HMDB_QUERY_INDEX_ID_HEADER_LEN, 1, f);
    if (!strstr(buffer, HMDB_QUERY_INDEX_ID_HEADER))
        fileValid = false;

    fclose(f);
    return fileValid;
}

bool HmdbQueryID::query(const HmdbQueryIDConditions& criteria,
                        HmdbQueryIndexRecord& result)
{
    result.IDList.clear();

    if (!existIndex())
    {
        result.status = HMDB_QUERY_INDEX_STATUS_NO_INDEX;
        return false;
    }

    int readLength;
    char* buffer = nullptr;
    FILE* f = fopen(indexFileName, "rb");
    std::list<std::string> matchedID;
    fseek(f, HMDB_QUERY_INDEX_ID_HEADER_LEN, SEEK_CUR);
    while (!feof(f))
    {
        readLength = utils_getdelim(&buffer, nullptr,
                                    HMDB_QUERY_INDEX_ID_SEP_ENTRY, f);
        if (readLength <= 0)
            break;

        if (strstr(buffer, criteria.pattern))
            matchedID.push_back(std::string(buffer, readLength));
        free(buffer);
        buffer = nullptr;
    }
    result.IDList.reserve(matchedID.size());
    result.IDList.insert(result.IDList.cend(),
                         matchedID.cbegin(), matchedID.cend());
    result.status = HMDB_QUERY_INDEX_STATUS_SUCCESS;
    free(buffer);
    fclose(f);
    return true;
}
