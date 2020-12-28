#ifndef HMDBRECORDGENERATOR_H
#define HMDBRECORDGENERATOR_H

#include <vector>
#include "hmdbquery.h"


class HmdbRecordGenerator
{
public:
    HmdbRecordGenerator();

    static HmdbQueryRecord getRecordByID(const char* dataDir,
                                  std::vector<const char*> IDList,
                                  std::vector<const char*> propertyList);
};

#endif // HMDBRECORDGENERATOR_H
