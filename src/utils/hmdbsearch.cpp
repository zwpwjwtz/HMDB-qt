#include <cstring>
#include <list>
#include "hmdbsearch_p.h"
#include "hmdbxml_def.h"
#include "filesystem.h"
#include "stdc.h"


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

std::vector<const char*> HmdbSearch::searchID(const char* pattern)
{
    auto matchedFiles = utils_listDirectoryFiles(d_ptr->dataDir);

    std::vector<const char*> matchedID;
    for (unsigned long i=0; i<matchedFiles.size(); i++)
    {
        if (strstr(matchedFiles[i], pattern))
            matchedID.push_back(d_ptr->getIDByFilename(matchedFiles[i]));
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

const char* HmdbSearchPrivate::getIDByFilename(const char* filename)
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
