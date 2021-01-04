#ifndef HMDBQUERY_P_H
#define HMDBQUERY_P_H

#include <vector>
#include <string>
#include "hmdbquery.h"
#include "hmdbsearch.h"


class HmdbQueryPrivate
{
public:
    HmdbQuery* q_ptr;

    char* dataDir;
    HmdbSearch searchEngine;
    std::vector<std::string> queryPropertyList;

    HmdbQueryPrivate(HmdbQuery* parent);
    ~HmdbQueryPrivate();

    HmdbQueryRecord generateRecordByID(std::vector<const char*> IDList,
                                       std::vector<const char*> propertyList);
};

#endif // HMDBQUERY_P_H