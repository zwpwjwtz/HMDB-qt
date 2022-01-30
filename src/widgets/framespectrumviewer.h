#ifndef FRAMESPECTRUMVIEWER_H
#define FRAMESPECTRUMVIEWER_H

#include <QFrame>


class MPlot;
class MPlotSeriesBasic;
class MPlotRealtimeModel;
class MPlotAbstractTool;
class MPlotDataPositionCursorTool;

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
    void setDataCursorVisible(bool visible);

protected:
    void changeEvent(QEvent *e);
    void resizeEvent(QResizeEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private:
    Ui::FrameSpectrumViewer *ui;
    QString dataPath;
    MPlot* plot;
    QList<MPlotSeriesBasic*> layerList;
    QList<MPlotRealtimeModel*> dataList;
    QList<MPlotAbstractTool*> toolList;
    MPlotDataPositionCursorTool* dataCursor;

    void loadSpectrum(const std::vector<double>& mz,
                      const std::vector<double>& intensities);

private slots:
    void onDataCursorPositionChanged(const QPointF& position);
};

#endif // FRAMESPECTRUMVIEWER_H
