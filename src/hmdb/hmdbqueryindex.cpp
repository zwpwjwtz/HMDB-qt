#include <cstring>
#include "hmdbqueryindex.h"
#include "hmdbxml_def.h"
#include "utils/stdc.h"


char* HmdbQueryIndex::getIDByFilename(const char* filename)
{
    const char* pos1 = utils_rstrstr(filename, "/");
    if (!pos1)
        pos1 = filename;
    const char* pos2 = utils_rstrstr(filename, HMDBXML_FILENAME_SUFFIX);
    if (!pos2)
        return nullptr;

    int IDlength = pos2 - pos1;
    char* ID = new char[IDlength + 1];
    utils_strncpy(ID, pos1, IDlength);
    return ID;
}
