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
    int rank;

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
    HmdbQueryRecord(const HmdbQueryRecord& src);
    ~HmdbQueryRecord();

    HmdbQueryRecord& operator= (const HmdbQueryRecord& src);
};

class HmdbQuery
{
public:
    enum DatabaseType
    {
        Main = 0,
        MSMS = 1
    };
    enum MassSpectrumMode
    {
        Unknown = 0,
        Positive = 1,
        Negative = 2
    };

    HmdbQuery();
    ~HmdbQuery();

    void setDataDirectory(const char* dir, DatabaseType type = Main);

    void setDefaultQueryProperty();
    void setQueryProperty(char** properties, int propertyCount);

    void getReady(DatabaseType type = Main);
    bool isReady(DatabaseType type = Main);

    HmdbQueryRecord queryID(const char* ID);
    HmdbQueryRecord queryMass(double min, double max);
    HmdbQueryRecord queryMonoMass(double min, double max);
    HmdbQueryRecord queryName(const char* name);
    HmdbQueryRecord queryMassSpectrum(double tolerance,
                                      bool relativeTolerance,
                                      MassSpectrumMode mode,
                                      int peaksCount,
                                      double* mzList,
                                      double* intensityList = nullptr);
    
protected:
    HmdbQueryPrivate* d_ptr;
    HmdbQuery(HmdbQueryPrivate* data);
};

#endif // HMDBQUERY_H
