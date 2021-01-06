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

HmdbQueryPropertyEntry::~HmdbQueryPropertyEntry()
{
    delete[] name;
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
