#ifndef HMDBBATCHQUERY_P_H
#define HMDBBATCHQUERY_P_H

#include "hmdbquery_p.h"
#include "hmdbbatchquery.h"


class HmdbBatchQueryPrivate : public HmdbQueryPrivate
{
public:
    char* sourcePath;
    char* targetPath;

    HmdbQueryStatus status;
    HmdbQueryError errorNumber;

    int currentProgress;
    int totalProgress;

    HmdbBatchQueryPrivate(HmdbBatchQuery* parent = nullptr);
    ~HmdbBatchQueryPrivate();

    bool checkData();
    void writeResultHeader(FILE* f, const char* addColumn = nullptr);

    static std::vector<std::string> splitString(std::string str,
                                                std::string delimiter);
};

#endif // HMDBBATCHQUERY_P_H
