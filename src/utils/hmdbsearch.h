#ifndef HMDBSEARCH_H
#define HMDBSEARCH_H

#include <vector>


class HmdbSearchPrivate;

class HmdbSearch
{
public:
    HmdbSearch();
    ~HmdbSearch();

    void setDataDirectory(const char* dir);

    std::vector<const char*> searchID(const char* pattern);

private:
    HmdbSearchPrivate* d_ptr;
};

#endif // HMDBSEARCH_H
