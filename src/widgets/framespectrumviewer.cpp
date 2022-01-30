#include "framespectrumviewer.h"
#include "ui_framespectrumviewer.h"
#include "hmdb/hmdbdatabase.h"
#include "hmdb/hmdbmassspectrum.h"
#include "hmdb/hmdbxml_def.h"
#include "MPlot/MPlotSeries.h"
#include "MPlot/MPlotTools.h"
#include "utils/filesystem.h"

#define HMDB_SPECTRUM_VIEW_MZ_EPSILON  1E-5


FrameSpectrumViewer::FrameSpectrumViewer(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameSpectrumViewer)
{
    plot = new MPlot;
    plot->axisLeft()->showAxisName(false);
    plot->axisLeft()->showGrid(false);
    plot->axisTop()->setTicks(0);
    plot->axisRight()->setTicks(0);
    plot->axisBottom()->showAxisName(false);
    plot->legend()->enableDefaultLegend(false);
    plot->setMargin(MPlot::StandardAxis::Left, 8);
    plot->setMargin(MPlot::StandardAxis::Top, 0);
    plot->setMargin(MPlot::StandardAxis::Right, 2);
    plot->setMargin(MPlot::StandardAxis::Bottom, 8);
    plot->background()->setBrush(QBrush(QColor(0x00F8F8F8)));
    plot->plotArea()->setBrush(QBrush(QColor(0x00FFFFFF)));

    toolList.push_back(new MPlotDragZoomerTool);
    toolList.push_back(new MPlotWheelZoomerTool);
    dataCursor = new MPlotDataPositionCursorTool(false);
    toolList.push_back(dataCursor);
    for (int i=0; i<toolList.count(); i++)
        plot->addTool(toolList[i]);
    dataCursor->setCursorVisibility(true);
    dataCursor->setCursorMarker(MPlotMarkerShape::Cross);
    dataCursor->setDataPositionIndicator(plot->axisScaleBottom(),
                                         plot->axisScaleLeft());
    connect(dataCursor, SIGNAL(positionChanged(const QPointF&)),
            this, SLOT(onDataCursorPositionChanged(const QPointF&)));

    ui->setupUi(this);
    ui->viewSpectrum->setPlot(plot);
    ui->viewSpectrum->installEventFilter(this);
    ui->labelTitle->installEventFilter(this);
    ui->labelAxisX->installEventFilter(this);
    ui->labelAxisY->installEventFilter(this);
    ui->labelAxisY->setText("Intensity");
}

FrameSpectrumViewer::~FrameSpectrumViewer()
{
    clear();
    delete ui;

    for (int i=0; i<toolList.count(); i++)
    {
        plot->removeTool(toolList[i]);
        delete toolList[i];
    }
    delete plot;
}

void FrameSpectrumViewer::changeEvent(QEvent *e)
{
    QFrame::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void FrameSpectrumViewer::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    ui->viewSpectrum->resize(ui->frameSpectrum->size());
    ui->labelDataPosition->move(ui->frameSpectrum->width() / 10,
                                ui->frameSpectrum->height() / 50);
}

bool FrameSpectrumViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (obj == ui->viewSpectrum)
            setDataCursorVisible(
                dynamic_cast<QMouseEvent*>(event)->button() == Qt::LeftButton);
        else
            setDataCursorVisible(false);
    }
    return false;
}

bool FrameSpectrumViewer::setDatabase(const QString& path)
{
    if (!utils_isDirectory(path.toLocal8Bit().constData()))
        return false;

    dataPath = path;
    return true;
}

void FrameSpectrumViewer::clear()
{
    for (int i=0; i<dataList.count(); i++)
    {
        layerList[i]->setModel(nullptr);
        delete dataList[i];
        delete layerList[i];
    }
    dataList.clear();
    layerList.clear();
}

bool FrameSpectrumViewer::load(QString metaboliteID, QString spectrumID)
{
    if (dataPath.isEmpty())
        return false;

    char* dataFilename =
            HmdbDatabase::getMSMSPathByID(metaboliteID.toStdString().c_str(),
                                          spectrumID.toStdString().c_str(),
                                          dataPath.toStdString().c_str());
    if (!dataFilename)
        return false;

    HmdbMassSpectrum spectrum;
    bool spectrumOK = spectrum.open(dataFilename);
    delete dataFilename;
    if (!spectrumOK)
        return false;

    loadSpectrum(spectrum.mzList(), spectrum.intensityList());
    plot->legend()->setTitleText(QString("Spectrum %1").arg(spectrumID));
    return true;
}

void FrameSpectrumViewer::setLineColor(QColor color)
{
    for (int i=0; i<layerList.count(); i++)
        layerList[i]->linePen().setColor(color);
}

void FrameSpectrumViewer::setTitle(QString text)
{
    ui->labelTitle->setText(text);
}

void FrameSpectrumViewer::setDataCursorVisible(bool visible)
{
    dataCursor->setCursorVisibility(visible);
    ui->labelDataPosition->setVisible(visible);
}

void FrameSpectrumViewer::loadSpectrum(const std::vector<double>& mz,
                                       const std::vector<double>& intensities)
{
    ui->labelDataPosition->clear();
    for (int i=0; i<mz.size(); i++)
    {
        // Plot each "peak" as a "delta function"
        MPlotRealtimeModel* model = new MPlotRealtimeModel();
        model->insertPointBack(mz[i] - HMDB_SPECTRUM_VIEW_MZ_EPSILON, 0);
        model->insertPointBack(mz[i], intensities[i]);
        model->insertPointBack(mz[i] + HMDB_SPECTRUM_VIEW_MZ_EPSILON, 0);
        dataList.push_back(model);

        MPlotSeriesBasic* layer = new MPlotSeriesBasic();
        layer->setMarker(MPlotMarkerShape::None);
        layer->setModel(model);
        layerList.push_back(layer);
        plot->addItem(layer);
    }
}

void FrameSpectrumViewer::onDataCursorPositionChanged(const QPointF& position)
{
    ui->labelDataPosition->setText(QString("(%1, %2)").arg(position.x())
                                                      .arg(position.y()));
}
