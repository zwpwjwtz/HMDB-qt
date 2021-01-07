#ifndef HMDBBATCHQUERY_H
#define HMDBBATCHQUERY_H

#include "hmdbquery.h"


enum HmdbQueryStatus
{
    None = 0,
    Working = 1,
    Finished = 2,
    Error = 3
};

enum HmdbQueryError
{
    OK = 0,
    NoDatabase = 1,
    NoSourceFile = 2,
    NoTargetFile = 3
};

class HmdbBatchQueryPrivate;

class HmdbBatchQuery : public HmdbQuery
{
public:
    HmdbBatchQuery();

    void setSourcePath(const char* path);
    void setResultPath(const char* path);

    bool existOption(const char* name);
    const char* getOption(const char* name);
    void setOption(const char* name, const char* value = nullptr);
    void clearOptions();

    HmdbQueryStatus status();
    HmdbQueryError error();

    int currentProgress();
    int totalProgress();

    bool queryMass();
    void stopQuery();

private:
    HmdbBatchQueryPrivate* d;
};

#endif // HMDBBATCHQUERY_H
