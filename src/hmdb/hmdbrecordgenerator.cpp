#include <cstring>
#include "hmdbrecordgenerator.h"
#include "utils/stdc.h"
#include "utils/uconfigxml.h"
#include "hmdbxml_def.h"


HmdbRecordGenerator::HmdbRecordGenerator()
{
}

const char* HmdbRecordGenerator::getEntryFilename(const char* ID,
                                                  const char* dataDir)
{
    char* filename = new char[strlen(dataDir) +
                              strlen(ID) +
                              strlen(HMDBXML_FILENAME_SUFFIX) + 2];
    filename[0] = '\0';
    if (dataDir)
    {
        strcpy(filename, dataDir);
        strcat(filename, "/");
    }
    strcat(filename, ID);
    strcat(filename, HMDBXML_FILENAME_SUFFIX);
    return filename;
}

HmdbQueryRecord HmdbRecordGenerator::getRecordByID(const char* dataDir,
                                  const std::vector<std::string>& IDList,
                                  const std::vector<std::string>& propertyList)
{
    int i, j;
    HmdbQueryRecord record;

    // Fill the property list
    record.propertyCount = propertyList.size();
    record.properties = new HmdbQueryPropertyEntry*[record.propertyCount];
    for (i=0; i<record.propertyCount; i++)
    {
        record.properties[i] = new HmdbQueryPropertyEntry;
        record.properties[i]->name = new char[propertyList[i].size() + 1];
        strcpy(record.properties[i]->name, propertyList[i].c_str());
    }

    // Fill the entry list
    record.entryCount = IDList.size();
    record.entries = new HmdbQueryRecordEntry*[record.entryCount];

    const char* dataFileName = nullptr;
    UconfigFile dataFile;
    HmdbQueryRecordEntry* entry;
    UconfigEntryObject dataEntry;
    UconfigEntryObject* dataSubentries;
    UconfigKeyObject* dataKeys;
    for (i=0; i<IDList.size(); i++)
    {
        // Parse entry data with UconfigXML parser
        delete[] dataFileName;
        dataFileName = getEntryFilename(IDList[i].c_str(), dataDir);
        if (!UconfigXML::readUconfig(dataFileName, &dataFile, true))
        {
            // Invalid entry; skip it
            record.entries[i] = nullptr;
            continue;
        }

        // Fill entry information
        entry = new HmdbQueryRecordEntry;
        entry->ID = new char[IDList[i].size() + 1];
        strcpy(entry->ID, IDList[i].c_str());
        entry->propertyCount = record.propertyCount;
        entry->propertyValues = new char*[record.propertyCount];

        // Fill entry's properties
        for (j=0; j<entry->propertyCount; j++)
        {
            dataEntry =
                    dataFile.rootEntry.searchSubentry(propertyList[j].c_str(),
                                                      nullptr,
                                                      true,
                                                      propertyList[j].size());
            if (!dataEntry.name() || dataEntry.subentryCount() < 1)
            {
                // Specified property not found in the data file; skip it
                entry->propertyValues[j] = nullptr;
                continue;
            }

            // Extract the first key in the first subentry
            // as its property value
            dataSubentries = dataEntry.subentries();
            dataKeys = dataSubentries[0].keys();
            entry->propertyValues[j] = new char[dataKeys[0].valueSize() + 1];
            utils_strncpy(entry->propertyValues[j],
                          dataKeys[0].value(),
                          dataKeys[0].valueSize());
            delete[] dataKeys;
            delete[] dataSubentries;
        }

        // Append the entry to the list
        record.entries[i] = entry;
    }
    delete[] dataFileName;

    return record;
}
