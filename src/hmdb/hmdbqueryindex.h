#ifndef HMDBQUERYINDEX_H
#define HMDBQUERYINDEX_H

#include <string>
#include <vector>

#define HMDB_QUERY_INDEX_STATUS_NONE        0
#define HMDB_QUERY_INDEX_STATUS_SUCCESS     1
#define HMDB_QUERY_INDEX_STATUS_NO_INDEX    2


struct HmdbQueryIndexConditions
{
    int maxRecord;
};

struct HmdbQueryIndexRecord
{
    int status;
    std::vector<std::string> IDList;
};

class HmdbQueryIndex
{
public:
    virtual void setDatabase(const char* path) = 0;

    virtual bool buildIndex() = 0;
    virtual bool existIndex() = 0;

    bool query(const HmdbQueryIndexConditions& criteria,
               HmdbQueryIndexRecord& result);
};

#endif // HMDBQUERYINDEX_H
