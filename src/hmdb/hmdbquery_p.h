#ifndef HMDBQUERY_P_H
#define HMDBQUERY_P_H

#include <map>
#include <vector>
#include <string>
#include "hmdbquery.h"


class HmdbQueryPrivate
{
public:
    std::map<int, std::string> dataDir;

    std::vector<std::string> queryPropertyList;
    std::map<std::string, std::string> queryOptions;

    int maxQueryResult;

    HmdbQueryPrivate(HmdbQuery* parent = nullptr);
    ~HmdbQueryPrivate();

    static const char* getPropertyValue(const HmdbQueryRecord& record,
                                        const char *ID,
                                        const char* propertyName);

protected:
    HmdbQuery* q_ptr;
};

#endif // HMDBQUERY_P_H
