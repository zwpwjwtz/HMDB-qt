#ifndef HMDBQUERY_H
#define HMDBQUERY_H


class HmdbQueryPrivate;

struct HmdbQueryPropertyEntry
{
    char* name;
};

struct HmdbQueryRecordEntry
{
    char* ID;
    int propertCount;
    char* properties;
};

struct HmdbQueryRecord
{
    int propertyCount;
    HmdbQueryPropertyEntry** properties;
    int entryCount;
    HmdbQueryRecordEntry** entries;
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
