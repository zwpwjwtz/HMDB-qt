#ifndef HMDBQUERY_H
#define HMDBQUERY_H


class HmdbQueryPrivate;

struct HmdbQueryPropertyEntry
{
    char* name;

    HmdbQueryPropertyEntry();
    HmdbQueryPropertyEntry(const HmdbQueryPropertyEntry& src);
    ~HmdbQueryPropertyEntry();

    HmdbQueryPropertyEntry& operator= (const HmdbQueryPropertyEntry& src);
};

struct HmdbQueryRecordEntry
{
    char* ID;
    int propertyCount;
    char** propertyValues;

    HmdbQueryRecordEntry();
    HmdbQueryRecordEntry(const HmdbQueryRecordEntry& src);
    ~HmdbQueryRecordEntry();

    HmdbQueryRecordEntry& operator= (const HmdbQueryRecordEntry& src);
};

struct HmdbQueryRecord
{
    int propertyCount;
    HmdbQueryPropertyEntry** properties;
    int entryCount;
    HmdbQueryRecordEntry** entries;

    HmdbQueryRecord();
    ~HmdbQueryRecord();

    HmdbQueryRecord& operator= (const HmdbQueryRecord& src);
};

class HmdbQuery
{
public:
    HmdbQuery();
    ~HmdbQuery();
    
    void setDataDirectory(const char* dir);
    void setQueryProperty(char** properties, int propertyCount);
    void setDefaultQueryProperty();

    void getReady();
    bool isReady();

    HmdbQueryRecord queryID(const char* ID);
    HmdbQueryRecord queryMass(double min, double max);
    HmdbQueryRecord queryMonoMass(double min, double max);
    HmdbQueryRecord queryName(const char* name);
    
protected:
    HmdbQueryPrivate* d_ptr;
    HmdbQuery(HmdbQueryPrivate* data);
};

#endif // HMDBQUERY_H
