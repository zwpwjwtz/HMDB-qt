#ifndef FRAMESPECTRUMVIEWER_H
#define FRAMESPECTRUMVIEWER_H

#include <QFrame>


class MPlot;
class MPlotSeriesBasic;
class MPlotRealtimeModel;

namespace Ui {
class FrameSpectrumViewer;
}

class FrameSpectrumViewer : public QFrame
{
    Q_OBJECT

public:
    explicit FrameSpectrumViewer(QWidget *parent = nullptr);
    ~FrameSpectrumViewer();

    bool setDatabase(const QString& path);

    void clear();
    bool load(QString metaboliteID, QString spectrumID);

    void setLineColor(QColor color);
    void setTitle(QString text);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FrameSpectrumViewer *ui;
    QString dataPath;
    MPlot* plot;
    QList<MPlotSeriesBasic*> layerList;
    QList<MPlotRealtimeModel*> dataList;

    void loadSpectrum(const std::vector<double>& mz,
                      const std::vector<double>& intensities);
};

#endif // FRAMESPECTRUMVIEWER_H
