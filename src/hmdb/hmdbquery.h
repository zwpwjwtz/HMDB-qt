#ifndef HMDBQUERY_H
#define HMDBQUERY_H


class HmdbQueryPrivate;

struct HmdbQueryPropertyEntry
{
    char* name;

    HmdbQueryPropertyEntry();
    ~HmdbQueryPropertyEntry();
};

struct HmdbQueryRecordEntry
{
    char* ID;
    int propertyCount;
    char** propertyValues;

    HmdbQueryRecordEntry();
    ~HmdbQueryRecordEntry();
};

struct HmdbQueryRecord
{
    int propertyCount;
    HmdbQueryPropertyEntry** properties;
    int entryCount;
    HmdbQueryRecordEntry** entries;

    HmdbQueryRecord();
    ~HmdbQueryRecord();
};

class HmdbQuery
{
public:
    HmdbQuery();
    ~HmdbQuery();
    
    void setDataDirectory(const char* dir);
    void setQueryProperty(const char** properties, int propertyCount);
    void setDefaultQueryProperty();

    HmdbQueryRecord queryID(const char* ID);
    
private:
    HmdbQueryPrivate* d_ptr;
};

#endif // HMDBQUERY_H
