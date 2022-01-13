#ifndef HMDBMASSSPECTRUM_H
#define HMDBMASSSPECTRUM_H

#include <vector>
#include <string>


enum HmdbMassSpectrumPolarity
{
    Unknown = 0,
    Positive = 1,
    Negative = 2
};

class HmdbMassSpectrum
{
public:
    bool open(const std::string& filename);
    void close();

    std::string metaboliteID();
    std::string spectrumID();
    HmdbMassSpectrumPolarity polarity();
    std::vector<double> mzList();
    std::vector<double> intensityList();

private:
    std::string filename;
};

#endif // HMDBMASSSPECTRUM_H
