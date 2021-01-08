#include <cstring>
#include <cstdio>
#include "hmdbquerymass.h"
#include "hmdbxml_def.h"
#include "utils/filesystem.h"
#include "utils/stdc.h"
#include "utils/uconfigxml.h"

#define HMDB_QUERY_INDEX_MASS_FILENAME    "index-mass"
#define HMDB_QUERY_INDEX_MASS_HEADER      "HMDBINDEX\n"
#define HMDB_QUERY_INDEX_MASS_HEADER_LEN  16
#define HMDB_QUERY_INDEX_MASS_VERSION     "1.0\n"
#define HMDB_QUERY_INDEX_MASS_TYPE        "02"
#define HMDB_QUERY_INDEX_MASS_HEADERSIZE  16
#define HMDB_QUERY_INDEX_MASS_ID_SIZE     16
#define HMDB_QUERY_INDEX_MASS_MASS_SIZE   8
#define HMDB_QUERY_INDEX_MASS_ENTRY_SIZE  16 + 8 + 8


HmdbQueryMass::HmdbQueryMass()
{
    indexFileName = nullptr;
}

HmdbQueryMass::HmdbQueryMass(const char* dataDir)
{
    indexFileName = nullptr;
    setDatabase(dataDir);
}

HmdbQueryMass::~HmdbQueryMass()
{
    delete[] indexFileName;
}

void HmdbQueryMass::setDatabase(const char* path)
{
    if (!path)
        return;
    dataDir = path;

    if (indexFileName)
        delete[] indexFileName;

    indexFileName = new char[strlen(dataDir) +
                             strlen(HMDB_QUERY_INDEX_MASS_FILENAME) + 2];
    strcpy(indexFileName, dataDir);
    strcat(indexFileName, "/");
    strcat(indexFileName, HMDB_QUERY_INDEX_MASS_FILENAME);
}

bool HmdbQueryMass::buildIndex()
{
    if (!dataDir)
        return false;

    FILE* f = fopen(indexFileName, "wb");
    if (!f)
        return false;

    // Write a file header
    fputs(HMDB_QUERY_INDEX_MASS_HEADER, f);
    fputs(HMDB_QUERY_INDEX_MASS_VERSION, f);
    fputs(HMDB_QUERY_INDEX_MASS_TYPE, f);

    // Get names of all files in the data directory
    auto dataFileList = utils_listDirectoryFiles(dataDir);

    // Write entries
    char* ID;
    char* dataFileName;
    char buffer[HMDB_QUERY_INDEX_MASS_ENTRY_SIZE];
    int pos;
    double avgMass, monoMass;
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
            if (!getMass(dataFileName, avgMass, monoMass))
            {
                // No mass data found; skip it
                delete[] ID;
                delete[] dataFileName;
                continue;
            }

            // Concatenate ID, avg. mass and mono. mass to a record
            pos = strlen(ID);
            strncpy(buffer, ID, pos);
            while (pos < HMDB_QUERY_INDEX_MASS_ID_SIZE)
                buffer[pos++] = 0;
            memcpy(&buffer[pos], &avgMass, HMDB_QUERY_INDEX_MASS_MASS_SIZE);
            pos += HMDB_QUERY_INDEX_MASS_MASS_SIZE;
            memcpy(&buffer[pos], &monoMass, HMDB_QUERY_INDEX_MASS_MASS_SIZE);

            // Write the record
            fwrite(buffer, HMDB_QUERY_INDEX_MASS_ENTRY_SIZE, 1, f);
            delete[] ID;
            delete[] dataFileName;
        }
    }

    fclose(f);
    return true;
}

bool HmdbQueryMass::existIndex()
{
    if (!indexFileName)
        return false;

    FILE* f = fopen(indexFileName, "rb");
    if (!f)
        return false;

    bool fileValid = true;
    char buffer[HMDB_QUERY_INDEX_MASS_HEADER_LEN + 1];
    fread(buffer, HMDB_QUERY_INDEX_MASS_HEADER_LEN, 1, f);
    if (!strstr(buffer, HMDB_QUERY_INDEX_MASS_HEADER))
        fileValid = false;

    fclose(f);
    return fileValid;
}

bool HmdbQueryMass::query(const HmdbQueryMassConditions& criteria,
                        HmdbQueryIndexRecord& result)
{
    result.IDList.clear();

    if (!existIndex())
    {
        result.status = HMDB_QUERY_INDEX_STATUS_NO_INDEX;
        return false;
    }

    int readLength;
    char buffer[HMDB_QUERY_INDEX_MASS_ENTRY_SIZE];
    bool monoisotopic = criteria.monoisotopic;
    double mass;
    std::list<std::string> matchedID;
    FILE* f = fopen(indexFileName, "rb");
    fseek(f, HMDB_QUERY_INDEX_MASS_HEADERSIZE, SEEK_CUR);
    while (!feof(f))
    {
        readLength = fread(buffer, HMDB_QUERY_INDEX_MASS_ENTRY_SIZE, 1, f);
        if (readLength <= 0)
            break;

        // Get mass data
        if (monoisotopic)
            memcpy(&mass,
                   &buffer[HMDB_QUERY_INDEX_MASS_ID_SIZE +
                           HMDB_QUERY_INDEX_MASS_MASS_SIZE],
                   HMDB_QUERY_INDEX_MASS_MASS_SIZE);
        else
            memcpy(&mass,
                   &buffer[HMDB_QUERY_INDEX_MASS_ID_SIZE],
                   HMDB_QUERY_INDEX_MASS_MASS_SIZE);

        // Compare with the required mass range
        if (mass > 0 && mass >= criteria.minMZ && mass <= criteria.maxMZ)
        {
            matchedID.push_back(
                        std::string(buffer, HMDB_QUERY_INDEX_MASS_ID_SIZE));
        }
    }
    result.IDList.reserve(matchedID.size());
    result.IDList.insert(result.IDList.cend(),
                         matchedID.cbegin(), matchedID.cend());

    result.status = HMDB_QUERY_INDEX_STATUS_SUCCESS;
    fclose(f);
    return true;
}

bool HmdbQueryMass::getMass(const char* filename,
                            double& averageMass,
                            double& monoisotopicMass)
{
    averageMass = 0;
    monoisotopicMass = 0;

    UconfigFile dataFile;
    if (!UconfigXML::readUconfig(filename, &dataFile))
        return false;

    UconfigEntryObject* dataSubentries;
    UconfigKeyObject* dataKeys;

    // Get the averaged mass
    char* massValue;
    UconfigEntryObject dataEntry =
        dataFile.rootEntry.searchSubentry(HMDBXML_ENTRY_PROP_AVGMASS,
                                          nullptr,
                                          true,
                                          strlen(HMDBXML_ENTRY_PROP_AVGMASS));
    if (dataEntry.subentryCount() > 0)
    {
        dataSubentries = dataEntry.subentries();
        if (dataSubentries->keyCount() > 0)
        {
            dataKeys = dataSubentries[0].keys();
            massValue = new char[dataKeys[0].valueSize() + 1];
            utils_strncpy(massValue,
                          dataKeys[0].value(),
                          dataKeys[0].valueSize());
            averageMass = atof(massValue);
            delete[] dataKeys;
        }
        delete[] dataSubentries;
    }

    // Get the monoisotopic mass
    dataEntry =
        dataFile.rootEntry.searchSubentry(HMDBXML_ENTRY_PROP_MONOMASS,
                                          nullptr,
                                          true,
                                          strlen(HMDBXML_ENTRY_PROP_MONOMASS));
    if (dataEntry.subentryCount() > 0)
    {
        dataSubentries = dataEntry.subentries();
        if (dataSubentries->keyCount() > 0)
        {
            dataKeys = dataSubentries[0].keys();
            monoisotopicMass = atof(dataKeys[0].value());
            delete[] dataKeys;
        }
        delete[] dataSubentries;
    }
    return true;
}

