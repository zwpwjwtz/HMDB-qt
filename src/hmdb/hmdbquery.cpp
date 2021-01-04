#include <cstring>
#include "hmdbquery_p.h"
#include "hmdbrecordgenerator.h"
#include "hmdbxml_def.h"


HmdbQueryPropertyEntry::HmdbQueryPropertyEntry()
{
    name = nullptr;
}

HmdbQueryPropertyEntry::~HmdbQueryPropertyEntry()
{
    delete name;
}

HmdbQueryRecordEntry::HmdbQueryRecordEntry()
{
    ID = nullptr;
    propertyCount = 0;
    propertyValues = nullptr;
}

HmdbQueryRecordEntry::~HmdbQueryRecordEntry()
{
    delete ID;
    for (int i=0; i<propertyCount; i++)
        delete propertyValues[i];
    delete[] propertyValues;
}

HmdbQueryRecord::HmdbQueryRecord()
{
    propertyCount = 0;
    properties = nullptr;
    entryCount = 0;
    entries = nullptr;
}

HmdbQueryRecord::~HmdbQueryRecord()
{
    int i;
    for (i=0; i<propertyCount; i++)
        delete properties[i];
    delete[] properties;
    for (i=0; i<entryCount; i++)
        delete entries[i];
    delete[] entries;
}

HmdbQuery::HmdbQuery()
{
    d_ptr = new HmdbQueryPrivate(this);
    setDefaultQueryProperty();
}

HmdbQuery::~HmdbQuery()
{
    delete d_ptr;
}

void HmdbQuery::setDataDirectory(const char* dir)
{
    char*& oldDir = d_ptr->dataDir;

    if (oldDir)
        delete[] oldDir;
    if (!dir)
        oldDir = nullptr;
    else
    {
        oldDir = new char[strlen(dir) + 1];
        strcpy(oldDir, dir);
    }
    d_ptr->searchEngine.setDataDirectory(dir);
}


void HmdbQuery::setQueryProperty(const char** properties, int propertyCount)
{
    if (propertyCount < 0)
        return;

    auto& list = d_ptr->queryPropertyList;
    list.clear();
    for (int i=0; i<propertyCount; i++)
    {
        if (strlen(properties[i]) > 0)
            list.push_back(properties[i]);
    }
}

void HmdbQuery::setDefaultQueryProperty()
{
    auto& list = d_ptr->queryPropertyList;
    list.clear();
    list.push_back(HMDBXML_ENTRY_PROP_NAME);
    list.push_back(HMDBXML_ENTRY_PROP_FORMULAR);
    list.push_back(HMDBXML_ENTRY_PROP_AVGMASS);
    list.push_back(HMDBXML_ENTRY_PROP_MONOMASS);
}

HmdbQueryRecord HmdbQuery::queryID(const char* ID)
{
    auto matchedID = d_ptr->searchEngine.searchID(ID);

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir,
                                              matchedID,
                                              d_ptr->queryPropertyList);
}

HmdbQueryPrivate::HmdbQueryPrivate(HmdbQuery* parent)
{
    q_ptr = parent;
    dataDir = nullptr;
}

HmdbQueryPrivate::~HmdbQueryPrivate()
{
    delete dataDir;
}
