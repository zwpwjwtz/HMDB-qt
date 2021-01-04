#ifndef HMDBRECORDGENERATOR_H
#define HMDBRECORDGENERATOR_H

#include <vector>
#include <string>
#include "hmdbquery.h"


class HmdbRecordGenerator
{
public:
    HmdbRecordGenerator();

    static const char* getEntryFilename(const char* ID,
                                        const char* dataDir = nullptr);

    static HmdbQueryRecord getRecordByID(const char* dataDir,
                                  const std::vector<std::string> &IDList,
                                  const std::vector<std::string> &propertyList);
};

#endif // HMDBRECORDGENERATOR_H
