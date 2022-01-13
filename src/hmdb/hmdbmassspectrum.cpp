#include <cstring>
#include "hmdbmassspectrum.h"
#include "hmdb_msms_xml_def.h"
#include "utils/stdc.h"


bool HmdbMassSpectrum::open(const std::string& filename)
{
    this->filename = filename;
    return true;
}

void HmdbMassSpectrum::close()
{
    this->filename.clear();
}

std::string HmdbMassSpectrum::metaboliteID()
{
    std::string ID;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return ID;

    char* buffer = nullptr;
    utils_fseekstr(HMDB_MSMS_XML_ENTRY_ID_BEGIN, f);
    utils_getdelim(&buffer, nullptr,
                   HMDB_MSMS_XML_ENTRY_ID_END, f);
    fclose(f);

    if (buffer != nullptr)
    {
        ID.assign(buffer);
        free(buffer);
    }
    return ID;
}

std::string HmdbMassSpectrum::spectrumID()
{
    std::string ID;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return ID;

    char* buffer = nullptr;
    utils_fseekstr(HMDB_MSMS_XML_ENTRY_SPECTRUM_ID_BEGIN, f);
    utils_getdelim(&buffer, nullptr,
                   HMDB_MSMS_XML_ENTRY_SPECTRUM_ID_END, f);
    fclose(f);

    if (buffer != nullptr)
    {
        ID.assign(buffer);
        free(buffer);
    }
    return ID;
}

HmdbMassSpectrumPolarity HmdbMassSpectrum::polarity()
{
    HmdbMassSpectrumPolarity polarity = Unknown;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return polarity;

    char* polarityString = nullptr;
    utils_fseekstr(HMDB_MSMS_XML_ENTRY_POLARITY_BEGIN, f);
    utils_getdelim(&polarityString, nullptr,
                   HMDB_MSMS_XML_ENTRY_POLARITY_END, f);
    if (!polarityString)
    {
        fclose(f);
        return polarity;
    }

    char* polarityStringLowerCased =
                    utils_tolower(polarityString, strlen(polarityString));
    if (strncmp(polarityStringLowerCased, HMDB_MSMS_XML_ENTRY_POLARITY_NEGATIVE,
                strlen(HMDB_MSMS_XML_ENTRY_POLARITY_NEGATIVE)) == 0)
        polarity = Negative;
    else
        polarity = Positive;

    free(polarityStringLowerCased);
    free(polarityString);
    fclose(f);
    return polarity;
}

std::vector<double> HmdbMassSpectrum::mzList()
{
    std::vector<double> mz;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return mz;

    char* buffer;
    int readLength;
    while (!feof(f))
    {
        if (utils_fseekstr(HMDB_MSMS_XML_ENTRY_MZ_BEGIN, f) <= 0)
            break;

        buffer = nullptr;
        readLength = utils_getdelim(&buffer, nullptr,
                                    HMDB_MSMS_XML_ENTRY_MZ_END, f);
        if (readLength > 0)
        {
            mz.push_back(atof(buffer));
            free(buffer);
        }
        else
        {
            if (buffer)
                free(buffer);
            break;
        }
    }

    fclose(f);
    return mz;
}

std::vector<double> HmdbMassSpectrum::intensityList()
{
    std::vector<double> intensities;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f)
        return intensities;

    char* buffer;
    int readLength;
    while (!feof(f))
    {
        if (utils_fseekstr(HMDB_MSMS_XML_ENTRY_INT_BEGIN, f) <= 0)
            break;

        buffer = nullptr;
        readLength = utils_getdelim(&buffer, nullptr,
                                    HMDB_MSMS_XML_ENTRY_INT_END, f);
        if (readLength > 0)
        {
            intensities.push_back(atof(buffer));
            free(buffer);
        }
        else
        {
            if (buffer)
                free(buffer);
            break;
        }
    }

    fclose(f);
    return intensities;
}
