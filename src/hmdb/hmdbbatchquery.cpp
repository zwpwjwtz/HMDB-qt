#include "cstring"
#include "sstream"
#include "hmdbbatchquery.h"
#include "hmdbbatchquery_p.h"
#include "hmdbxml_def.h"
#include "hmdbqueryoptions.h"
#include "hmdbrecordgenerator.h"
#include "hmdbquerymass.h"
#include "utils/filesystem.h"

#define HMDB_BATCH_MASS_BUFFER_MAX        1024

#define HMDB_BATCH_RESULT_RECORD_SEP       "\n"
#define HMDB_BATCH_RESULT_FIELD_SEP       ";"
#define HMDB_BATCH_RESULT_FIELD_SEARCH    "Search"
#define HMDB_BATCH_RESULT_FIELD_DIFF      "Difference"
#define HMDB_BATCH_RESULT_FIELD_MODIFY    "Modification"
#define HMDB_BATCH_RESULT_FIELD_MASS      \
        HMDB_BATCH_RESULT_FIELD_MODIFY    \
        HMDB_BATCH_RESULT_FIELD_SEP       \
        HMDB_BATCH_RESULT_FIELD_DIFF


HmdbBatchQuery::HmdbBatchQuery()
    : HmdbQuery (new HmdbBatchQueryPrivate(this))
{
    d = static_cast<HmdbBatchQueryPrivate*>(d_ptr);
}

void HmdbBatchQuery::setSourcePath(const char* path)
{
    char*& oldPath = d->sourcePath;
    if (oldPath)
    {
        delete[] oldPath;
        oldPath = nullptr;
    }
    if (path)
    {
        oldPath = new char[strlen(path) + 1];
        strcpy(oldPath, path);
    }
}

void HmdbBatchQuery::setResultPath(const char* path)
{
    char*& oldPath = d->targetPath;
    if (oldPath)
    {
        delete[] oldPath;
        oldPath = nullptr;
    }
    if (path)
    {
        oldPath = new char[strlen(path) + 1];
        strcpy(oldPath, path);
    }
}

bool HmdbBatchQuery::existOption(const char *name)
{
    return d->queryOptions.find(name) != d->queryOptions.cend();
}

const char* HmdbBatchQuery::getOption(const char* name)
{
    return d->queryOptions[name].c_str();
}

void HmdbBatchQuery::setOption(const char* name, const char* value)
{
    if (!value)
        value = "";
    d->queryOptions[name] = value;
}

void HmdbBatchQuery::clearOptions()
{
    d->queryOptions.clear();
}

HmdbQueryStatus HmdbBatchQuery::status()
{
    return d->status;
}

HmdbQueryError HmdbBatchQuery::error()
{
    return d->errorNumber;
}

int HmdbBatchQuery::progress()
{
    if (d->totalProgress > 0)
        return int(100.0 * d->currentProgress / d->totalProgress);
    else
        return 0;
}

void HmdbBatchQuery::setProgressCallback(std::function<void(double)> func)
{
    d->progressCallback = func;
}

void HmdbBatchQuery::setFinishedCallback(std::function<void(bool)> func)
{
    d->finishedCallback = func;
}

bool HmdbBatchQuery::queryMass()
{
    if (!d->checkData())
    {
        if (d->finishedCallback != nullptr)
            d->finishedCallback(false);
        return false;
    }

    d->totalProgress = utils_fileLength(d->sourcePath);
    d->currentProgress = 0;

    // Open the input & output file
    FILE* sourceFile = fopen(d->sourcePath, "rb");
    FILE* targetFile = fopen(d->targetPath, "wb");

    // Write a file header
    d->writeResultHeader(targetFile, HMDB_BATCH_RESULT_FIELD_MASS);

    // Read the source data (CSV file) line by line,
    // and write each query result one by one
    int i, j, k;
    int matchedCount;
    double mass, modification, matchedMass, difference;
    double delta = atof(getOption(HMDB_QUERY_OPTION_MASS_TOLERANCE));
    bool relativeDelta = existOption(HMDB_QUERY_OPTION_MASS_RELATIVE_TOL);
    char* massValueEnd;
    const char* matchedMassValue;
    char* matchedMassValueEnd;
    char buffer[HMDB_BATCH_MASS_BUFFER_MAX];
    auto& properties = d->queryPropertyList;
    auto modificationList =
                    HmdbBatchQueryPrivate::splitString(
                            d->queryOptions[HMDB_QUERY_OPTION_MASS_MODIFICATION],
                            HMDB_QUERY_OPTION_FIELD_SEP);
    auto modificationNameList =
                    HmdbBatchQueryPrivate::splitString(
                            d->queryOptions[HMDB_QUERY_OPTION_MASS_MOD_NAME],
                            HMDB_QUERY_OPTION_FIELD_SEP);
    HmdbQueryMass searchEngine(d->dataDir);
    HmdbQueryMassConditions conditions;
    conditions.monoisotopic = existOption(HMDB_QUERY_OPTION_MASS_MONOISOTOPIC);
    const char* queryMassField = conditions.monoisotopic ?
                    HMDBXML_ENTRY_PROP_MONOMASS : HMDBXML_ENTRY_PROP_AVGMASS;
    HmdbQueryIndexRecord indexQueryResult;
    HmdbQueryRecord fullQueryResult;

    while(!feof(sourceFile))
    {
        if (!fgets(buffer, HMDB_BATCH_MASS_BUFFER_MAX, sourceFile))
            continue;

        // Get the requested mass value from the beginning of each line
        mass = strtod(buffer, &massValueEnd);
        if (buffer == massValueEnd)
            continue;

        // Calculate a mass range for each type of modification,
        // then make queries separately
        matchedCount = 0;
        for (i=0; i<modificationList.size(); i++)
        {
            indexQueryResult.IDList.clear();
            modification = atof(modificationList[i].c_str());
            if (relativeDelta)
            {
                conditions.minMZ = mass - modification - mass * delta * 1E-6;
                conditions.maxMZ = mass - modification + mass * delta * 1E-6;
            }
            else
            {
                conditions.minMZ = mass - modification - delta;
                conditions.maxMZ = mass - modification + delta;
            }

            if (searchEngine.query(conditions, indexQueryResult) &&
                indexQueryResult.IDList.size() > 0)
            {
                fullQueryResult = HmdbRecordGenerator::getRecordByID(d->dataDir,
                                                       indexQueryResult.IDList,
                                                       properties);

                // Query succeeded; write each record as a line
                for (j=0; j<fullQueryResult.entryCount; j++)
                {
                    // The queried mass
                    fwrite(buffer, massValueEnd - buffer, 1, targetFile);
                    fputs(HMDB_BATCH_RESULT_FIELD_SEP, targetFile);

                    // The modification name
                    if (i < modificationNameList.size())
                        fputs(modificationNameList[i].c_str(), targetFile);
                    fputs(HMDB_BATCH_RESULT_FIELD_SEP, targetFile);

                    // The difference between the queried and the matched mass
                    matchedMassValue = d->getPropertyValue(fullQueryResult,
                                                fullQueryResult.entries[j]->ID,
                                                queryMassField);
                    if (matchedMassValue)
                    {
                        matchedMass = strtod(matchedMassValue,
                                             &matchedMassValueEnd);
                        if (matchedMassValueEnd != matchedMassValue)
                        {
                            difference = matchedMass + modification - mass;
                            if (relativeDelta)
                                difference = difference * 1E6 / mass;
                            fprintf(targetFile, "%lf", difference);
                        }
                    }

                    // Append properties as columns
                    // Assume the number of properties in each record equals to
                    // the required number of properties
                    for (k=0; k<fullQueryResult.entries[j]->propertyCount; k++)
                    {
                        fputs(HMDB_BATCH_RESULT_FIELD_SEP, targetFile);
                        if (fullQueryResult.entries[j]->propertyValues[k])
                            fputs(fullQueryResult.entries[j]->propertyValues[k],
                                  targetFile);
                    }
                    fputs(HMDB_BATCH_RESULT_RECORD_SEP, targetFile);
                    matchedCount++;
                }
            }
        }
        if (matchedCount == 0)
        {
            // All attempts failed; write a empty line
            fwrite(buffer, massValueEnd - buffer, 1, targetFile);
            fputs(HMDB_BATCH_RESULT_FIELD_SEP, targetFile);

            for (j=0; j<properties.size()+2; j++)
                fputs(HMDB_BATCH_RESULT_FIELD_SEP, targetFile);
            fputs(HMDB_BATCH_RESULT_RECORD_SEP, targetFile);
        }

        // Report the progress
        d->currentProgress = ftell(sourceFile);
        if (d->totalProgress != 0 && d->progressCallback != nullptr)
            d->progressCallback(100.0 * d->currentProgress / d->totalProgress);
    }
    fclose(sourceFile);
    fclose(targetFile);

    if (d->finishedCallback != nullptr)
        d->finishedCallback(true);
    return true;
}

void HmdbBatchQuery::stopQuery()
{
    d->status = HmdbQueryStatus::None;
    if (d->finishedCallback != nullptr)
        d->finishedCallback(false);
}

HmdbBatchQueryPrivate::HmdbBatchQueryPrivate(HmdbBatchQuery* parent)
    : HmdbQueryPrivate (parent)
{
    HmdbQueryPrivate* parentData = static_cast<HmdbQueryPrivate*>(this);
    dataDir = parentData->dataDir;
    queryPropertyList = parentData->queryPropertyList;

    sourcePath = nullptr;
    targetPath = nullptr;
    status = HmdbQueryStatus::None;
    currentProgress = 0;
    totalProgress = 0;
    finishedCallback = nullptr;
    progressCallback = nullptr;
}

HmdbBatchQueryPrivate::~HmdbBatchQueryPrivate()
{
    delete sourcePath;
    delete targetPath;
}

bool HmdbBatchQueryPrivate::checkData()
{
    if (!utils_isDirectory(dataDir))
    {
        errorNumber = NoDatabase;
        return false;
    }
    if (!utils_isFile(sourcePath))
    {
        errorNumber = NoSourceFile;
        return false;
    }
    if (!utils_isFile(targetPath))
    {
        errorNumber = NoTargetFile;
        return false;
    }
    return true;
}

void HmdbBatchQueryPrivate::writeResultHeader(FILE *f, const char* addColumn)
{
    fputs(HMDB_BATCH_RESULT_FIELD_SEARCH, f);

    if (addColumn)
    {
        fputs(HMDB_BATCH_RESULT_FIELD_SEP, f);
        fputs(addColumn, f);
    }

    for (int i=0; i<queryPropertyList.size(); i++)
    {
        fputs(HMDB_BATCH_RESULT_FIELD_SEP, f);
        fputs(queryPropertyList[i].c_str(), f);
    }

    fputs(HMDB_BATCH_RESULT_RECORD_SEP, f);
}


std::vector<std::string>
HmdbBatchQueryPrivate::splitString(std::string str, std::string delimiter)
{
    std::vector<std::string> substringList;

    std::string::size_type pos, lastPos = 0;
    while (lastPos < str.size())
    {
        pos = str.find(delimiter, lastPos);
        if (pos == std::string::npos)
            pos = str.size();
        if (pos > lastPos)
            substringList.push_back(std::string(str, lastPos, pos - lastPos));
        lastPos = pos + 1;
    }
    return substringList;
}
