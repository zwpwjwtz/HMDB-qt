#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "hmdbqueryname.h"
#include "hmdbxml_def.h"
#include "utils/filesystem.h"
#include "utils/stdc.h"
#include "utils/uconfigxml.h"

#define HMDB_QUERY_INDEX_NAME_FILENAME    "index-name"
#define HMDB_QUERY_INDEX_NAME_HEADER      "HMDBINDEX\n"
#define HMDB_QUERY_INDEX_NAME_HEADER_LEN  16
#define HMDB_QUERY_INDEX_NAME_VERSION     "1.0\n"
#define HMDB_QUERY_INDEX_NAME_TYPE        "03"
#define HMDB_QUERY_INDEX_NAME_HEADERSIZE  16
#define HMDB_QUERY_INDEX_NAME_ID_SIZE     16
#define HMDB_QUERY_INDEX_NAME_NAME_SIZE   100
#define HMDB_QUERY_INDEX_NAME_ENTRY_SIZE  128


HmdbQueryName::HmdbQueryName()
{
    indexFileName = nullptr;
}

HmdbQueryName::HmdbQueryName(const char* dataDir)
{
    indexFileName = nullptr;
    setDatabase(dataDir);
}

HmdbQueryName::~HmdbQueryName()
{
    delete[] indexFileName;
}

void HmdbQueryName::setDatabase(const char* path)
{
    if (!path)
        return;
    dataDir = path;

    if (indexFileName)
        delete[] indexFileName;

    indexFileName = new char[strlen(dataDir) +
                             strlen(HMDB_QUERY_INDEX_NAME_FILENAME) + 2];
    strcpy(indexFileName, dataDir);
    strcat(indexFileName, "/");
    strcat(indexFileName, HMDB_QUERY_INDEX_NAME_FILENAME);
}

bool HmdbQueryName::buildIndex()
{
    if (!dataDir)
        return false;

    FILE* f = fopen(indexFileName, "wb");
    if (!f)
        return false;

    // Write a file header
    fputs(HMDB_QUERY_INDEX_NAME_HEADER, f);
    fputs(HMDB_QUERY_INDEX_NAME_VERSION, f);
    fputs(HMDB_QUERY_INDEX_NAME_TYPE, f);

    // Get names of all files in the data directory
    auto dataFileList = utils_listDirectoryFiles(dataDir);

    // Write entries
    char* ID;
    char* tempName, *dataEntryName;
    char* dataFileName = nullptr;
    char buffer[HMDB_QUERY_INDEX_NAME_ENTRY_SIZE];
    for (auto i=dataFileList.cbegin(); i!=dataFileList.cend(); i++)
    {
        if (strstr((*i).c_str(), HMDBXML_FILENAME_SUFFIX))
        {
            // This is a data (XML) file; add it to index
            ID = getIDByFilename((*i).c_str());
            dataFileName = new char[strlen(dataDir) +
                                    strlen((*i).c_str()) + 2];
            strcpy(dataFileName, dataDir);
            strcat(dataFileName, "/");
            strcat(dataFileName, (*i).c_str());

            // Get the entry name, and convert it to lower-case
            tempName = getName(dataFileName);
            if (!tempName)
            {
                // No mass data found; skip it
                delete[] ID;
                delete[] tempName;
                delete[] dataFileName;
                continue;
            }
            dataEntryName = utils_tolower(tempName,
                                          HMDB_QUERY_INDEX_NAME_NAME_SIZE);

            // Concatenate ID and name to a record
            utils_strncpy(buffer, ID, HMDB_QUERY_INDEX_NAME_ID_SIZE);
            utils_strncpy(&buffer[HMDB_QUERY_INDEX_NAME_ID_SIZE],
                          dataEntryName, HMDB_QUERY_INDEX_NAME_NAME_SIZE);

            // Write the record
            fwrite(buffer, HMDB_QUERY_INDEX_NAME_ENTRY_SIZE, 1, f);
            delete[] ID;
            delete[] tempName;
            free(dataEntryName);
            delete[] dataFileName;
        }
    }

    fclose(f);
    return true;
}

bool HmdbQueryName::existIndex()
{
    if (!indexFileName)
        return false;

    FILE* f = fopen(indexFileName, "rb");
    if (!f)
        return false;

    bool fileValid = true;
    char buffer[HMDB_QUERY_INDEX_NAME_HEADER_LEN + 1];
    fread(buffer, HMDB_QUERY_INDEX_NAME_HEADER_LEN, 1, f);
    if (!strstr(buffer, HMDB_QUERY_INDEX_NAME_HEADER))
        fileValid = false;

    fclose(f);
    return fileValid;
}

bool HmdbQueryName::query(const HmdbQueryNameConditions& criteria,
                        HmdbQueryIndexRecord& result)
{
    result.IDList.clear();

    if (!existIndex())
    {
        result.status = HMDB_QUERY_INDEX_STATUS_NO_INDEX;
        return false;
    }

    int readLength;
    char buffer[HMDB_QUERY_INDEX_NAME_ENTRY_SIZE];
    char* name = utils_tolower(criteria.pattern,
                               strlen(criteria.pattern) + 1);
    std::list<std::string> matchedID;
    FILE* f = fopen(indexFileName, "rb");
    fseek(f, HMDB_QUERY_INDEX_NAME_HEADERSIZE, SEEK_CUR);
    while (!feof(f))
    {
        readLength = fread(buffer, HMDB_QUERY_INDEX_NAME_ENTRY_SIZE, 1, f);
        if (readLength <= 0)
            break;

        if (strstr(&buffer[HMDB_QUERY_INDEX_NAME_ID_SIZE], name))
        {
            matchedID.push_back(
                        std::string(buffer, HMDB_QUERY_INDEX_NAME_ID_SIZE));
        }
    }
    result.IDList.reserve(matchedID.size());
    result.IDList.insert(result.IDList.cend(),
                         matchedID.cbegin(), matchedID.cend());
    result.status = HMDB_QUERY_INDEX_STATUS_SUCCESS;

    free(name);
    fclose(f);
    return true;
}

char* HmdbQueryName::getName(const char* filename)
{
    UconfigFile dataFile;
    if (!UconfigXML::readUconfig(filename, &dataFile))
        return nullptr;

    char* name = nullptr;
    UconfigKeyObject* dataKeys;
    UconfigEntryObject* dataSubentries;
    UconfigEntryObject dataEntry =
        dataFile.rootEntry.searchSubentry(HMDBXML_ENTRY_PROP_NAME,
                                          nullptr,
                                          true,
                                          strlen(HMDBXML_ENTRY_PROP_NAME));
    if (dataEntry.subentryCount() > 0)
    {
        dataSubentries = dataEntry.subentries();
        if (dataSubentries->keyCount() > 0)
        {
            dataKeys = dataSubentries[0].keys();
            if (dataKeys[0].valueSize() > 0)
            {
                name = new char[dataKeys[0].valueSize() + 1];
                utils_strncpy(name,
                              dataKeys[0].value(),
                              dataKeys[0].valueSize());
            }
            delete[] dataKeys;
        }
        delete[] dataSubentries;
    }
    return name;
}
