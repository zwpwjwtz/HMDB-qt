#include <cstring>
#include <list>
#include "hmdbsearch_p.h"
#include "hmdbxml_def.h"
#include "utils/filesystem.h"
#include "utils/stdc.h"


HmdbSearch::HmdbSearch()
{
    d_ptr = new HmdbSearchPrivate(this);
}

HmdbSearch::~HmdbSearch()
{
    delete d_ptr;
}


void HmdbSearch::setDataDirectory(const char* dir)
{
    char*& oldDir = d_ptr->dataDir;

    if (oldDir)
        delete[] oldDir;
    if (!dir)
        oldDir = nullptr;
    else
    {
        oldDir = new char[strlen(dir) + 1];
        strcpy(oldDir, dir);
    }
}

std::vector<std::string> HmdbSearch::searchID(const char* pattern)
{
    auto matchedFiles = utils_listDirectoryFiles(d_ptr->dataDir);

    char* ID;
    std::vector<std::string> matchedID;
    for (auto i=matchedFiles.cbegin(); i!=matchedFiles.cend(); i++)
    {
        if (strstr((*i).c_str(), pattern))
        {
            ID = d_ptr->getIDByFilename((*i).c_str());
            matchedID.push_back(ID);
            delete[] ID;
        }
    }
    return matchedID;
}

HmdbSearchPrivate::HmdbSearchPrivate(HmdbSearch* parent)
{
    q_ptr = parent;
    dataDir = nullptr;
}

HmdbSearchPrivate::~HmdbSearchPrivate()
{
    delete dataDir;
}

char* HmdbSearchPrivate::getIDByFilename(const char* filename)
{
    const char* pos1 = strstr(filename, "/");
    if (!pos1)
        pos1 = filename;
    const char* pos2 = utils_rstrstr(filename, HMDBXML_FILENAME_SUFFIX);
    if (!pos2)
        return nullptr;

    int IDlength = pos2 - pos1 + 1;
    char* ID = new char[IDlength + 1];
    strncpy(ID, pos1, IDlength);
    return ID;
}
