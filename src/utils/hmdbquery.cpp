#include <cstring>
#include "hmdbquery_p.h"
#include "hmdbrecordgenerator.h"
#include "hmdbxml_def.h"


HmdbQuery::HmdbQuery()
{
    d_ptr = new HmdbQueryPrivate(this);
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
    list.push_back(HMDBXML_ENTRY_PROP_ID);
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
