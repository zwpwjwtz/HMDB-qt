#ifndef HMDBSEARCH_H
#define HMDBSEARCH_H

#include <vector>
#include <string>


class HmdbSearchPrivate;

class HmdbSearch
{
public:
    HmdbSearch();
    ~HmdbSearch();

    void setDataDirectory(const char* dir);

    std::vector<std::string> searchID(const char* pattern);

private:
    HmdbSearchPrivate* d_ptr;
};

#endif // HMDBSEARCH_H
