#include <cstring>
#include "hmdbquery_p.h"
#include "hmdbrecordgenerator.h"
#include "hmdbxml_def.h"
#include "hmdbqueryid.h"
#include "hmdbquerymass.h"
#include "hmdbqueryname.h"
#include "hmdbquerymassspectrum.h"


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
    rank = 0;
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

    rank = src.rank;
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

    rank = src.rank;

    return *this;
}

HmdbQueryRecord::HmdbQueryRecord()
{
    propertyCount = 0;
    properties = nullptr;
    entryCount = 0;
    entries = nullptr;
}

HmdbQueryRecord::HmdbQueryRecord(const HmdbQueryRecord& src)
{
    int i;
    propertyCount = src.propertyCount;
    properties = new HmdbQueryPropertyEntry*[propertyCount];
    for (i=0; i<propertyCount; i++)
    {
        if (src.properties[i])
            properties[i] = new HmdbQueryPropertyEntry(*src.properties[i]);
        else
            properties[i] = nullptr;
    }

    entryCount = src.entryCount;
    entries = new HmdbQueryRecordEntry*[entryCount];
    for (i=0; i<entryCount; i++)
    {
        if (src.entries[i])
            entries[i] = new HmdbQueryRecordEntry(*src.entries[i]);
        else
            entries[i] = nullptr;
    }
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
    {
        if (src.properties[i])
            properties[i] = new HmdbQueryPropertyEntry(*src.properties[i]);
        else
            properties[i] = nullptr;
    }

    if (entries)
    {
        for (i=0; i<entryCount; i++)
            delete entries[i];
        delete[] entries;
    }
    entryCount = src.entryCount;
    entries = new HmdbQueryRecordEntry*[entryCount];
    for (i=0; i<entryCount; i++)
    {
        if (src.entries[i])
            entries[i] = new HmdbQueryRecordEntry(*src.entries[i]);
        else
            entries[i] = nullptr;
    }

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

void HmdbQuery::setDataDirectory(const char* dir, DatabaseType type)
{
    d_ptr->dataDir[type] = dir;
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

void HmdbQuery::getReady(DatabaseType type)
{
    switch (type)
    {
        case Main:
        {
            HmdbQueryID searchID(d_ptr->dataDir.at(Main).c_str());
            if (!searchID.existIndex())
                searchID.buildIndex();

            HmdbQueryMass searchMass(d_ptr->dataDir.at(Main).c_str());
            if (!searchMass.existIndex())
                searchMass.buildIndex();

            HmdbQueryName searchName(d_ptr->dataDir.at(Main).c_str());
            if (!searchName.existIndex())
                searchName.buildIndex();
            break;
        }
        case MSMS:
        {
            HmdbQueryMassSpectrum
                    searchMassSpectrum(d_ptr->dataDir.at(MSMS).c_str());
            if (!searchMassSpectrum.existIndex())
                searchMassSpectrum.buildIndex();
            break;
        }
        default:;
    }
}

bool HmdbQuery::isReady(HmdbQuery::DatabaseType type)
{
    if (d_ptr->dataDir.find(type) == d_ptr->dataDir.cend())
        return false;
    switch (type)
    {
        case Main:
        {
            HmdbQueryID searchID(d_ptr->dataDir.at(Main).c_str());
            HmdbQueryMass searchMass(d_ptr->dataDir.at(Main).c_str());
            HmdbQueryName searchName(d_ptr->dataDir.at(Main).c_str());
            return searchID.existIndex() &&
                    searchMass.existIndex() &&
                    searchName.existIndex();
        }
        case MSMS:
        {
            HmdbQueryMassSpectrum
                    searchMassSpectrum(d_ptr->dataDir.at(MSMS).c_str());
            return searchMassSpectrum.existIndex();
        }
        default:
            return false;
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
    HmdbQueryID searchEngine(d_ptr->dataDir.at(Main).c_str());
    HmdbQueryIDConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.pattern = ID;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir.at(Main).c_str(),
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryMass(double min, double max)
{
    HmdbQueryMass searchEngine(d_ptr->dataDir.at(Main).c_str());
    HmdbQueryMassConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.minMZ = min;
    conditions.maxMZ = max;
    conditions.monoisotopic = false;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir.at(Main).c_str(),
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryMonoMass(double min, double max)
{
    HmdbQueryMass searchEngine(d_ptr->dataDir.at(Main).c_str());
    HmdbQueryMassConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.minMZ = min;
    conditions.maxMZ = max;
    conditions.monoisotopic = true;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir.at(Main).c_str(),
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryName(const char* name)
{
    HmdbQueryName searchEngine(d_ptr->dataDir.at(Main).c_str());
    HmdbQueryNameConditions conditions;
    HmdbQueryIndexRecord result;
    conditions.pattern = name;
    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();

    return HmdbRecordGenerator::getRecordByID(d_ptr->dataDir.at(Main).c_str(),
                                              result.IDList,
                                              d_ptr->queryPropertyList);
}

HmdbQueryRecord HmdbQuery::queryMassSpectrum(double tolerance,
                                             bool relativeTolerance,
                                             MassSpectrumMode mode,
                                             int peaksCount,
                                             double* mzList,
                                             double* intensityList)
{
    HmdbQueryMassSpectrum searchEngine(d_ptr->dataDir.at(MSMS).c_str());
    HmdbQueryMassSpectrumConditions conditions;
    HmdbQueryMassSpectrumRecord result;

    conditions.mzList.reserve(peaksCount);
    conditions.intensityList.reserve(peaksCount);
    for (int i=0; i<peaksCount; i++)
    {
        conditions.massTolerance = tolerance;
        conditions.relativeTolerance = relativeTolerance;
        conditions.mzList.push_back(mzList[i]);
        if (intensityList)
            conditions.intensityList.push_back(intensityList[i]);
        conditions.mode =
                HmdbQueryMassSpectrumConditions::MassSpectrumMode(mode);
    }

    if (!searchEngine.query(conditions, result))
        return HmdbQueryRecord();
    HmdbQueryRecord fullResult =
            HmdbRecordGenerator::getRecordByID(d_ptr->dataDir.at(Main).c_str(),
                                               result.IDList,
                                               d_ptr->queryPropertyList);
    // Convert match scores to ranks (0 ~ 100)
    for (int i=0; i<fullResult.entryCount; i++)
    {
        if (i >= result.scoreList.size())
            break;
        if (fullResult.entries[i] == nullptr)
            continue;
        fullResult.entries[i]->rank = result.scoreList[i] * 100;
    }
    return fullResult;
}

HmdbQueryPrivate::HmdbQueryPrivate(HmdbQuery* parent)
{
    q_ptr = parent;
}

HmdbQueryPrivate::~HmdbQueryPrivate()
{}

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
