#include <cstring>
#include "hmdbrecordgenerator.h"


HmdbRecordGenerator::HmdbRecordGenerator()
{
}

HmdbQueryRecord HmdbRecordGenerator::getRecordByID(const char* dataDir,
                                          std::vector<const char*> IDList,
                                          std::vector<const char*> propertyList)
{
    HmdbQueryRecord record;
    record.entryCount = IDList.size();
    record.entries = new HmdbQueryRecordEntry*[record.entryCount];
    record.propertyCount = 0;

    HmdbQueryRecordEntry* entry;
    for (int i=0; i<IDList.size(); i++)
    {
        entry = new HmdbQueryRecordEntry;
        entry->ID = new char[strlen(IDList[i]) + 1];
        strcpy(entry->ID, IDList[i]);
        entry->propertCount = 0;
        record.entries[i] = entry;
    }
    return record;
}
