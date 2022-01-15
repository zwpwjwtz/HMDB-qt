#include <cstring>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include "hmdbquerymassspectrum.h"
#include "hmdbdatabase.h"
#include "hmdbmassspectrum.h"
#include "hmdb_msms_xml_def.h"
#include "utils/filesystem.h"
#include "utils/stdc.h"
#include "utils/uconfigxml.h"

#define HMDB_QUERY_INDEX_MS_PATH_MAXLEN 256
#define HMDB_QUERY_INDEX_MS_FILENAME    "index-ms"
#define HMDB_QUERY_INDEX_MS_HEADER      "HMDBINDEX\n"
#define HMDB_QUERY_INDEX_MS_VERSION     "1.0\n"
#define HMDB_QUERY_INDEX_MS_TYPE        "04"
#define HMDB_QUERY_INDEX_MS_HEADER_LEN  16
#define HMDB_QUERY_INDEX_MS_ENTRY_SIZE  512

#define HMDB_QUERY_INDEX_MS_METABOLITE_ID_SIZE      16
#define HMDB_QUERY_INDEX_MS_SPECTRUM_ID_SIZE        16

#define HMDB_QUERY_INDEX_MS_PROPERTY_OFFSET         32
#define HMDB_QUERY_INDEX_MS_PROPERTY_SIZE           4
#define HMDB_QUERY_INDEX_MS_PROPERTY_NONE           0x00
#define HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_POSITIVE 0x01
#define HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_NEGATIVE 0x02

#define HMDB_QUERY_INDEX_MS_PEAKS_OFFSET            36
#define HMDB_QUERY_INDEX_MS_PEAKS_COUNT_SIZE        4
#define HMDB_QUERY_INDEX_MS_PEAKS_MZ_SIZE           8
#define HMDB_QUERY_INDEX_MS_PEAKS_MAX               59


HmdbQueryMassSpectrum::HmdbQueryMassSpectrum()
{
    indexFileName = nullptr;
}

HmdbQueryMassSpectrum::HmdbQueryMassSpectrum(const char* dataDir)
{
    indexFileName = nullptr;
    setDatabase(dataDir);
}

HmdbQueryMassSpectrum::~HmdbQueryMassSpectrum()
{
    delete[] indexFileName;
}

void HmdbQueryMassSpectrum::setDatabase(const char* path)
{
    if (!path)
        return;
    dataDir = path;

    if (indexFileName)
        delete[] indexFileName;

    indexFileName = new char[strlen(dataDir) +
                             strlen(HMDB_QUERY_INDEX_MS_FILENAME) + 2];
    strcpy(indexFileName, dataDir);
    strcat(indexFileName, "/");
    strcat(indexFileName, HMDB_QUERY_INDEX_MS_FILENAME);
}

bool HmdbQueryMassSpectrum::buildIndex()
{
    if (!dataDir)
        return false;

    FILE* f = fopen(indexFileName, "wb");
    if (!f)
        return false;

    // Write a file header
    fputs(HMDB_QUERY_INDEX_MS_HEADER, f);
    fputs(HMDB_QUERY_INDEX_MS_VERSION, f);
    fputs(HMDB_QUERY_INDEX_MS_TYPE, f);

    // Get names of all files in the data directory
    auto dataFileList = utils_listDirectoryFiles(dataDir);

    // Write entries
    std::string ID;  // Main ID of a metabolite in HMDB
    std::string spectrumID;
    std::string dataFilePath;
    char buffer[HMDB_QUERY_INDEX_MS_ENTRY_SIZE];
    int pos;
    HmdbMassSpectrumPolarity polarity;
    int propertyBits;
    int massListLength;
    int j;
    HmdbMassSpectrum spectrum;
    std::vector<int> peakOrders;
    std::vector<double> mz, intensities;
    for (auto i=dataFileList.cbegin(); i!=dataFileList.cend(); i++)
    {
        if (!strstr((*i).c_str(), HMDB_MSMS_XML_FILENAME_SUFFIX))
            continue;

        // This is a spectrum data (XML) file
        // See if it contains all the required information
        dataFilePath.assign(dataDir).append("/").append(*i);
        if (!spectrum.open(dataFilePath) ||
            (ID = spectrum.metaboliteID()).empty() ||
            (spectrumID = spectrum.spectrumID()).empty() ||
            (polarity = spectrum.polarity())
                      == HmdbMassSpectrumPolarity::Unknown ||
            (mz = spectrum.mzList()).empty())
        {
            // No required information found; skip it
            continue;
        }

        // Try to get peak intensities
        intensities = spectrum.intensityList();
        if (!intensities.empty())
        {
            // Sort peaks according their intensities in desceding order
            getOrder(intensities, peakOrders, true);
        }
        else
        {
            // No intensity provided; sort m/z in desceding order
            getOrder(mz, peakOrders, true);
        }

        // Concatenate all information to a record
        memset(buffer, 0, HMDB_QUERY_INDEX_MS_ENTRY_SIZE);
        // The metabolite ID
        strncpy(buffer, ID.c_str(), HMDB_QUERY_INDEX_MS_METABOLITE_ID_SIZE);
        pos = HMDB_QUERY_INDEX_MS_METABOLITE_ID_SIZE;
        // The spectrum ID
        strncpy(&buffer[pos], spectrumID.c_str(),
                HMDB_QUERY_INDEX_MS_SPECTRUM_ID_SIZE);
        pos += HMDB_QUERY_INDEX_MS_SPECTRUM_ID_SIZE;
        // The properties (polarity, etc.)
        if (polarity == HmdbMassSpectrumPolarity::Negative)
            propertyBits = HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_NEGATIVE;
        else
            propertyBits = HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_POSITIVE;
        memcpy(&buffer[pos], &propertyBits,
               HMDB_QUERY_INDEX_MS_PROPERTY_SIZE);
        pos += HMDB_QUERY_INDEX_MS_PROPERTY_SIZE;
        // The peak list length
        massListLength = mz.size() > HMDB_QUERY_INDEX_MS_PEAKS_MAX ?
                    HMDB_QUERY_INDEX_MS_PEAKS_MAX :
                    mz.size();
        memcpy(&buffer[pos], &massListLength,
               HMDB_QUERY_INDEX_MS_PEAKS_COUNT_SIZE);
        pos += HMDB_QUERY_INDEX_MS_PEAKS_COUNT_SIZE;
        // The m/z list
        // N.B.: Limited m/z values can be stored because of
        //       the limited space of each index entry
        for (j=0; j<massListLength; j++)
        {
            memcpy(&buffer[pos], &mz[peakOrders[j]],
                    HMDB_QUERY_INDEX_MS_PEAKS_MZ_SIZE);
            pos += HMDB_QUERY_INDEX_MS_PEAKS_MZ_SIZE;
        }

        // Write the record
        fwrite(buffer, HMDB_QUERY_INDEX_MS_ENTRY_SIZE, 1, f);
    }

    fclose(f);
    return true;
}

bool HmdbQueryMassSpectrum::existIndex()
{
    if (!indexFileName)
        return false;

    FILE* f = fopen(indexFileName, "rb");
    if (!f)
        return false;

    bool fileValid = true;
    char buffer[HMDB_QUERY_INDEX_MS_HEADER_LEN + 1];
    fread(buffer, HMDB_QUERY_INDEX_MS_HEADER_LEN, 1, f);
    if (!strstr(buffer, HMDB_QUERY_INDEX_MS_HEADER))
        fileValid = false;

    fclose(f);
    return fileValid;
}

bool
HmdbQueryMassSpectrum::query(const HmdbQueryMassSpectrumConditions& criteria,
                             HmdbQueryMassSpectrumRecord &result)
{
    result.IDList.clear();
    result.spectrumIDList.clear();
    result.scoreList.clear();

    if (!existIndex())
    {
        result.status = HMDB_QUERY_INDEX_STATUS_NO_INDEX;
        return false;
    }

    int readLength;
    int mode = criteria.mode;
    int propertyBits, peakCount;
    char buffer[HMDB_QUERY_INDEX_MS_ENTRY_SIZE];

    std::list<std::string> matchedSpectrumID;
    std::list<std::string> matchedID;
    std::vector<double> mzList, intensityList;
    std::list<double> score;

    // Search target spectrum against the index file
    FILE* f = fopen(indexFileName, "rb");
    fseek(f, HMDB_QUERY_INDEX_MS_HEADER_LEN, SEEK_CUR);
    while (!feof(f))
    {
        readLength = fread(buffer, HMDB_QUERY_INDEX_MS_ENTRY_SIZE, 1, f);
        if (readLength <= 0)
            break;

        // Get spectrum properties and verify them
        memcpy(&propertyBits,
               &buffer[HMDB_QUERY_INDEX_MS_PROPERTY_OFFSET],
               HMDB_QUERY_INDEX_MS_PROPERTY_SIZE);
        if (!(mode == HmdbQueryMassSpectrumConditions::Positive &&
              propertyBits & HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_POSITIVE ||
              mode == HmdbQueryMassSpectrumConditions::Negative &&
              propertyBits & HMDB_QUERY_INDEX_MS_PROPERTY_POLAR_NEGATIVE ||
              mode == HmdbQueryMassSpectrumConditions::Unknown))
            continue;

        // Compare m/z values for a coarse matching
        // Use pointers of two arrays for fast accessing
        peakCount = 0;
        memcpy(&peakCount,
               &buffer[HMDB_QUERY_INDEX_MS_PEAKS_OFFSET],
               HMDB_QUERY_INDEX_MS_PEAKS_COUNT_SIZE);
        if (!matchSpectraMZ(criteria.massTolerance,
                            criteria.relativeTolerance,
                            criteria.mzList.size(),
                            criteria.mzList.data(),
                            peakCount,
                            reinterpret_cast<double*>(
                                &buffer[HMDB_QUERY_INDEX_MS_PEAKS_OFFSET +
                                        HMDB_QUERY_INDEX_MS_PEAKS_COUNT_SIZE])))
            continue;

        // Keep all matched spectra
        matchedID.push_back(
            std::string(buffer, HMDB_QUERY_INDEX_MS_METABOLITE_ID_SIZE));
        matchedSpectrumID.push_back(
            std::string(&buffer[HMDB_QUERY_INDEX_MS_METABOLITE_ID_SIZE],
                        HMDB_QUERY_INDEX_MS_SPECTRUM_ID_SIZE));
    }

    // Calculate matching score for each matched spectrum
    // Peak intensities are used for a fine matching
    char* dataFilePath;
    HmdbMassSpectrum spectrum;
    auto i = matchedSpectrumID.cbegin();
    for (auto j=matchedID.cbegin(); j!=matchedID.cend(); j++)
    {
        dataFilePath = HmdbDatabase::getMSMSPathByID((*j).c_str(),
                                                     (*i).c_str(),
                                                     dataDir);
        if (!dataFilePath)
            continue;
        if (!spectrum.open(dataFilePath))
        {
            delete dataFilePath;
            continue;
        }
        mzList = spectrum.mzList();
        intensityList = spectrum.intensityList();

        // Use pointers of four arrays for fast accessing
        score.push_back(
            getMatchScore(criteria.massTolerance,
                          criteria.relativeTolerance,
                          criteria.mzList.size(),
                          criteria.mzList.data(),
                          criteria.intensityList.size() > 0 ?
                              criteria.intensityList.data() : nullptr,
                          mzList.size(),
                          mzList.data(),
                          intensityList.size() > 0 ?
                              intensityList.data() : nullptr));
        i++;
        delete dataFilePath;
    }

    // Dump all result to the target struct
    result.IDList.reserve(matchedID.size());
    result.IDList.insert(result.IDList.cend(),
                         matchedID.cbegin(), matchedID.cend());
    result.spectrumIDList.reserve(matchedSpectrumID.size());
    result.spectrumIDList.insert(result.spectrumIDList.cend(),
                                 matchedSpectrumID.cbegin(),
                                 matchedSpectrumID.cend());
    result.scoreList.reserve(score.size());
    result.scoreList.insert(result.scoreList.cend(),
                            score.cbegin(), score.cend());
    result.status = HMDB_QUERY_INDEX_STATUS_SUCCESS;

    fclose(f);
    return true;
}

template <typename T>
void HmdbQueryMassSpectrum::getOrder(const std::vector<T> valueList,
                                     std::vector<int>& indexList,
                                     bool desceding)
{
    indexList.resize(valueList.size());
    for (int i=0; i<valueList.size(); i++)
        indexList[i] = i;

    if (desceding)
        std::sort(indexList.begin(), indexList.end(),
                  [valueList](int index1, int index2)
                      { return valueList[index1] > valueList[index2]; });
    else
        std::sort(indexList.begin(), indexList.end(),
                  [valueList](int index1, int index2)
                      { return valueList[index1] <= valueList[index2]; });
}

bool HmdbQueryMassSpectrum::matchSpectraMZ(double tolerance,
                                           bool relativeTolerance,
                                           int mzCount1, const double* mzList1,
                                           int mzCount2, const double* mzList2)
{
    int i, j;
    bool matched = false;

    if (relativeTolerance)
    {
        double delta;
        for (i=0; i<mzCount1; i++)
        {
            delta = mzCount1 * tolerance;
            for (j=0; j<mzCount2; j++)
            {
                if (std::abs(mzList1[i] - mzList2[j]) <= delta)
                {
                    matched = true;
                    break;
                }
            }
        }
    }
    else
    {
        for (i=0; i<mzCount1; i++)
        {
            for (j=0; j<mzCount2; j++)
            {
                if (std::abs(mzList1[i] - mzList2[j]) <= tolerance)
                {
                    matched = true;
                    break;
                }
            }
        }
    }
    return matched;
}

double HmdbQueryMassSpectrum::getMatchScore(double mzTolerance,
                                            bool relativeTolerance,
                                            int mzCount1,
                                            const double* mzList1,
                                            const double* intensityList1,
                                            int mzCount2,
                                            const double* mzList2,
                                            const double* intensityList2)
{
    // Step 1: Get common m/z values with given tolerance
    int i, j;
    int mzCount;
    double delta;
    int* mzListIndex2 = new int[mzCount2];

    // Step 1.1: Add all element in mzList1 to the new list
    mzCount = mzCount1;

    // Step 1.2: Search each element in mzList2 in mzList1
    for (i=0; i<mzCount2; i++)
    {
        if (relativeTolerance)
            delta = mzList2[i] * mzTolerance;
        else
            delta = mzTolerance;

        for (j=0; j<mzCount1; j++)
        {
            if (std::abs(mzList1[j] - mzList2[i]) <= delta)
            {
                // Matched found; remember the index of element in mzList1
                mzListIndex2[i] = j;
                break;
            }
        }
        if (j >= mzCount1)
        {
            // No match found; append it to the new list
            mzListIndex2[i] = mzCount;
            mzCount++;
        }
    }

    // Step 1.3: Check additional information
    if (intensityList1 == nullptr || intensityList2 == nullptr)
    {
        // No intensity provided
        // Compare only similarity of two m/z list
        delete[] mzListIndex2;
        return double(mzCount1 + mzCount2 - mzCount) / mzCount;
    }

    // Step 2: Calculate the cosine similarity of two spectra
    double* intensity1 = new double[mzCount];
    double* intensity2 = new double[mzCount];
    double norm1 = 0.0, norm2 = 0.0, intensityProduct = 0.0;

    // Step 2.1: Initialize two intensity vectors with given intensity lists
    memset(intensity1, 0, mzCount * sizeof(double));
    memset(intensity2, 0, mzCount * sizeof(double));
    memcpy(intensity1, intensityList1, mzCount1 * sizeof(double));
    for (i=0; i<mzCount2; i++)
    {
        if (intensity2[mzListIndex2[i]] < intensityList2[i])
            intensity2[mzListIndex2[i]] = intensityList2[i];
    }

    // Step 2.2: Calculate a dot product of two vectors
    for (i=0; i<mzCount; i++)
        intensityProduct += intensity1[i] * intensity2[i];

    // Step 2.3: Calculate the norm of two vectors
    for (i=0; i<mzCount1; i++)
        norm1 += intensityList1[i] * intensityList1[i];
    norm1 = std::sqrt(norm1);
    for (i=0; i<mzCount2; i++)
        norm2 += intensityList2[i] * intensityList2[i];
    norm2 = std::sqrt(norm2);

    // Step 2.4: Calculate cosine similarity and normalize it
    if (norm1 == 0.0 || norm2 == 0.0)
        intensityProduct = 0.0;
    else
        intensityProduct = intensityProduct / (norm1 * norm2);

    delete[] intensity1;
    delete[] intensity2;
    delete[] mzListIndex2;
    return intensityProduct;
}
