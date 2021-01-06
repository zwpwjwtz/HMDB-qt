#ifndef HMDBQUERYMASS_H
#define HMDBQUERYMASS_H

#include "hmdbqueryindex.h"


struct HmdbQueryMassConditions : public HmdbQueryIndexConditions
{
    bool monoisotopic;
    double minMZ;
    double maxMZ;
};

class HmdbQueryMass : public HmdbQueryIndex
{
public:
    HmdbQueryMass();
    HmdbQueryMass(const char* dataDir);
    ~HmdbQueryMass();

    void setDatabase(const char* path);

    bool buildIndex();
    bool existIndex();

    bool query(const HmdbQueryMassConditions& criteria,
               HmdbQueryIndexRecord& result);

private:
    const char* dataDir;
    char* indexFileName;

    bool getMass(const char* filename,
                 double& averageMass,
                 double& monoisotopicMass);
};

#endif // HMDBQUERYMASS_H
