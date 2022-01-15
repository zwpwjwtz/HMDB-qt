#include <string>
#include "hmdbdatabase.h"
#include "hmdbxml_def.h"
#include "hmdb_msms_xml_def.h"
#include "utils/stdc.h"
#include "utils/filesystem.h"


char* HmdbDatabase::getIDByFilename(const char* filename)
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

char* HmdbDatabase::getMSMSPathByID(const char* ID,
                                    const char* spectrumID,
                                    const char* dataDir)
{
    char* pathString = nullptr;
    std::string path(dataDir);

    // Try the first combination
    path.append("/")
        .append(ID)
        .append(HMDB_MSMS_XML_FILENAME_ID_SEP2)
        .append(spectrumID)
        .append(HMDB_MSMS_XML_FILENAME_END1)
        .append(HMDB_MSMS_XML_FILENAME_SUFFIX);

    if (!utils_isFile(path.c_str()))
    {
        // Try the second one
        path.assign(dataDir);
        path.append("/")
                .append(ID)
                .append(HMDB_MSMS_XML_FILENAME_ID_SEP2)
                .append(spectrumID)
                .append(HMDB_MSMS_XML_FILENAME_END2)
                .append(HMDB_MSMS_XML_FILENAME_SUFFIX);
    }
    if (utils_isFile(path.c_str()))
    {
        pathString = new char[path.size() + 1];
        utils_strncpy(pathString, path.c_str(), path.size());
    }

    return pathString;
}
