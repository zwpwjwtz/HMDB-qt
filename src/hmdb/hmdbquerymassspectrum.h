#ifndef HMDBQUERYMASSSPECTRUM_H
#define HMDBQUERYMASSSPECTRUM_H

#include "hmdbqueryindex.h"


struct HmdbQueryMassSpectrumConditions : public HmdbQueryIndexConditions
{
    enum MassSpectrumMode
    {
        Unknown = 0,
        Positive = 1,
        Negative = 2
    };

    MassSpectrumMode mode;
    double massTolerance;
    bool relativeTolerance;
    std::vector<double> mzList;
    std::vector<double> intensityList;
};

struct HmdbQueryMassSpectrumRecord : public HmdbQueryIndexRecord
{
    std::vector<double> scoreList;
    std::vector<std::string> spectrumIDList;
};

class HmdbQueryMassSpectrum : public HmdbQueryIndex
{
public:
    HmdbQueryMassSpectrum();
    HmdbQueryMassSpectrum(const char* dataDir);
    virtual ~HmdbQueryMassSpectrum();

    virtual void setDatabase(const char* path);

    virtual bool buildIndex();
    virtual bool existIndex();

    bool query(const HmdbQueryMassSpectrumConditions& criteria,
               HmdbQueryMassSpectrumRecord& result);

private:
    const char* dataDir;
    char* indexFileName;

    template <typename T>
    static void getOrder(const std::vector<T> valueList,
                         std::vector<int>& indexList,
                         bool desceding = false);

    static bool matchSpectraMZ(double tolerance, bool relativeTolerance,
                               int mzCount1, const double* mzList1,
                               int mzCount2, const double* mzList2);
    static double getMatchScore(double mzTolerance,
                                bool relativeTolerance,
                                int mzCount1,
                                const double* mzList1,
                                const double* intensityList1,
                                int mzCount2,
                                const double* mzList2,
                                const double* intensityList2);
};

#endif // HMDBQUERYMASSSPECTRUM_H
