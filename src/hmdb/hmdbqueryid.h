#ifndef HMDBQUERYID_H
#define HMDBQUERYID_H

#include "hmdbqueryindex.h"


struct HmdbQueryIDConditions : public HmdbQueryIndexConditions
{
    const char* pattern;
};

class HmdbQueryID : public HmdbQueryIndex
{
public:
    HmdbQueryID();
    HmdbQueryID(const char* dataDir);
    ~HmdbQueryID();

    void setDatabase(const char* path);

    bool buildIndex();
    bool existIndex();

    bool query(const HmdbQueryIDConditions& criteria,
               HmdbQueryIndexRecord& result);

private:
    const char* dataDir;
    char* indexFileName;
};

#endif // HMDBQUERYID_H
