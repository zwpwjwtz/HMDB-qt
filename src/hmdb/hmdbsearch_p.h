#ifndef HMDBSEARCH_PRIVATE_H
#define HMDBSEARCH_PRIVATE_H

#include "hmdbsearch.h"


class HmdbSearchPrivate
{
public:
    HmdbSearch* q_ptr;

    char* dataDir;

    HmdbSearchPrivate(HmdbSearch* parent);
    ~HmdbSearchPrivate();

    char* getIDByFilename(const char* filename);
};

#endif // HMDBSEARCH_PRIVATE_H
