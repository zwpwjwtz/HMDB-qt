#ifndef HMDBDATABASE_H
#define HMDBDATABASE_H


class HmdbDatabase
{
public:
    static char* getIDByFilename(const char* filename);
    static char* getMSMSPathByID(const char* ID,
                                 const char* spectrumID,
                                 const char* dataDir);
};

#endif // HMDBDATABASE_H
