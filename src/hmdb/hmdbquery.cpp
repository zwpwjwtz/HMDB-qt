#include <cstring>
#include "hmdbquery_p.h"
#include "hmdbrecordgenerator.h"
#include "hmdbxml_def.h"
#include "hmdbqueryid.h"
#include "hmdbquerymass.h"
#include "hmdbqueryname.h"


HmdbQueryPropertyEntry::HmdbQueryPropertyEntry()
{
    name = nullptr;
}

HmdbQueryPropertyEntry::
HmdbQueryPropertyEntry(const HmdbQueryPropertyEntry& src)
{
    if (src.name)
    {
        name = new char[strlen(src.name) + 1];
        strcpy(name, src.name);
    }
    else
        name = nullptr;
}

HmdbQueryPropertyEntry::~HmdbQueryPropertyEntry()
{
    delete[] name;
}

HmdbQueryPropertyEntry&
HmdbQueryPropertyEntry::operator= (const HmdbQueryPropertyEntry& src)
{
    if (name)
        delete[] name;
    if (src.name)
    {
        name = new char[strlen(src.name) + 1];
        strcpy(name, src.name);
    }
    else
        name = nullptr;
    return *this;
}

HmdbQueryRecordEntry::HmdbQueryRecordEntry()
{
    ID = nullptr;
    propertyCount = 0;
    propertyValues = nullptr;
}

HmdbQueryRecordEntry::HmdbQueryRecordEntry(const HmdbQueryRecordEntry& src)
{
    if (src.ID)
    {
        ID = new char[strlen(src.ID) + 1];
        strcpy(ID, src.ID);
    }
    else
        ID = nullptr;

    propertyCount = src.propertyCount;
    propertyValues = new char*[propertyCount];
    for (int i=0; i<propertyCount; i++)
    {
        if (src.propertyValues[i])
        {
            propertyValues[i] = new char[strlen(src.propertyValues[i]) + 1];
            strcpy(propertyValues[i], src.propertyValues[i]);
        }
        else
            propertyValues[i] = nullptr;
    }
}

HmdbQueryRecordEntry::~HmdbQueryRecordEntry()
{
    delete[] ID;
    if (propertyValues)
    {
        for (int i=0; i<propertyCount; i++)
            delete[] propertyValues[i];
        delete[] propertyValues;
    }
}

HmdbQueryRecordEntry&
HmdbQueryRecordEntry:: operator= (const HmdbQueryRecordEntry& src)
{
    if (ID)
        delete[] ID;
    if (src.ID)
    {
        ID = new char[strlen(src.ID) + 1];
        strcpy(ID, src.ID);
    }
    else
        ID = nullptr;

    int i;
    if (propertyValues)
    {
        for (i=0; i<propertyCount; i++)
            delete[] propertyValues[i];
        delete[] propertyValues;
    }
    propertyCount = src.propertyCount;
    propertyValues = new char*[propertyCount];
    for (i=0; i<propertyCount; i++)
    {
        if (src.propertyValues[i])
        {
            propertyValues[i] = new char[strlen(src.propertyValues[i]) + 1];
            strcpy(propertyValues[i], src.propertyValues[i]);
        }
        else
            propertyValues[i] = nullptr;
    }
    return *this;
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

HmdbQueryRecord& HmdbQueryRecord::operator=(const HmdbQueryRecord& src)
{
    int i;
    if (properties)
    {
        for (i=0; i<propertyCount; i++)
            delete properties[i];
        delete[] properties;
    }
    propertyCount = src.propertyCount;
    properties = new HmdbQueryPropertyEntry*[propertyCount];
    for (i=0; i<propertyCount; i++)
        properties[i] = new HmdbQueryPropertyEntry(*src.properties[i]);

    if (entries)
    {
        for (i=0; i<entryCount; i++)
            delete entries[i];
        delete[] entries;
    }
    entryCount = src.entryCount;
    entries = new HmdbQueryRecordEntry*[entryCount];
    for (i=0; i<entryCount; i++)
        entries[i] = new HmdbQueryRecordEntry(*src.entries[i]);

    return *this;
}

HmdbQuery::HmdbQuery()
{
    d_ptr = new HmdbQueryPrivate(this);
    setDefaultQueryProperty();
}

HmdbQuery::HmdbQuery(HmdbQueryPrivate* data)
{
    d_ptr = data;
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
}


void HmdbQuery::setQueryProperty(char** properties, int propertyCount)
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

void HmdbQuery::getReady()
{
    HmdbQueryID searchID(d_ptr->dataDir);
    if (!searchID.existIndex())
        searchID.buildIndex();

    HmdbQueryMass searchMass(d_ptr->dataDir);
    if (!searchMass.existIndex())
        searchMass.buildIndex();

    HmdbQueryName searchName(d_ptr->dataDir);
    if (!searchName.existIndex())
        searchName.buildIndex();
}

bool HmdbQuery::isReady()
{
    HmdbQueryID searchID(d_ptr->dataDir);
    HmdbQueryMass searchMass(d_ptr->dataDir);
    HmdbQueryName searchName(d_ptr->dataDir);
    return searchID.existIndex() &&
           searchMass.existIndex() &&
           searchName.existIndex();
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
    HmdbQueryID searchEngine(d_ptr->dataDir);
    HmdbQueryIDConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.pattern = ID;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir,
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryMass(double min, double max)
{
    HmdbQueryMass searchEngine(d_ptr->dataDir);
    HmdbQueryMassConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.minMZ = min;
    conditions.maxMZ = max;
    conditions.monoisotopic = false;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir,
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryMonoMass(double min, double max)
{
    HmdbQueryMass searchEngine(d_ptr->dataDir);
    HmdbQueryMassConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.minMZ = min;
    conditions.maxMZ = max;
    conditions.monoisotopic = true;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir,
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryName(const char* name)
{
    HmdbQueryName searchEngine(d_ptr->dataDir);
    HmdbQueryNameConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.pattern = name;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir,
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryPrivate::HmdbQueryPrivate(HmdbQuery* parent)
{
    q_ptr = parent;
    dataDir = nullptr;
}

HmdbQueryPrivate::~HmdbQueryPrivate()
{
    delete[] dataDir;
}

const char* HmdbQueryPrivate::getPropertyValue(const HmdbQueryRecord& record,
                                               const char* ID,
                                               const char* propertyName)
{
    if (record.entryCount <= 0 || record.propertyCount <= 0)
        return nullptr;

    int i;
    int propertyIndex = -1;
    for (i=0; i<record.propertyCount; i++)
    {
        if (strcmp(record.properties[i]->name, propertyName) == 0)
        {
            propertyIndex = i;
            break;
        }
    }
    if (propertyIndex == -1)
    {
        // No property of given name
        return nullptr;
    }

    for (i=0; i<record.entryCount; i++)
    {
        if (strcmp(ID, record.entries[i]->ID) == 0)
            return record.entries[i]->propertyValues[propertyIndex];
    }

    // No entry of given ID
    return nullptr;
}
