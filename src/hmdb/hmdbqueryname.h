#ifndef HmdbQueryName_H
#define HmdbQueryName_H

#include "hmdbqueryindex.h"


struct HmdbQueryNameConditions : public HmdbQueryIndexConditions
{
    const char* pattern;
};

class HmdbQueryName : public HmdbQueryIndex
{
public:
    HmdbQueryName();
    HmdbQueryName(const char* dataDir);
    ~HmdbQueryName();

    void setDatabase(const char* path);

    bool buildIndex();
    bool existIndex();

    bool query(const HmdbQueryNameConditions& criteria,
               HmdbQueryIndexRecord& result);

private:
    const char* dataDir;
    char* indexFileName;

    char* getName(const char* filename);
};

#endif // HmdbQueryName_H
