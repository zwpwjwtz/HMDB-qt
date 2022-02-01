#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include "formquery.h"
#include "ui_formquery.h"
#include "global.h"
#include "dialogconfig.h"
#include "widgets/controlmssearchoption.h"
#include "widgets/controlqueryfield.h"
#include "widgets/formmetaboliteviewer.h"
#include "threads/hmdbqueryworker.h"
#include "utils/stdc.h"

#define HMDB_QUERY_TYPE_ID        0
#define HMDB_QUERY_TYPE_NAME      1
#define HMDB_QUERY_TYPE_MASS      2
#define HMDB_QUERY_TYPE_MSMS      3

#define HMDB_QUERY_MSMS_TOLERANCE_DEFAULT    0.1
#define HMDB_QUERY_MSMS_INTENSITY_DEFAULT    0


FormQuery::FormQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQuery)
{
    ui->setupUi(this);
    database = nullptr;
    widgetMSSearchOption = nullptr;
    listQueryField = new ControlQueryField();
    viewer = nullptr;

    ui->widgetSearchOption->setCurrentIndex(HMDB_QUERY_TYPE_ID);
    ui->textMSMSPeaks->setPlaceholderText(
                "mz1 Intensity1\n"
                "mz2 Intensity2\n"
                "mz3 Intensity3\n");
    ui->progressQuery->hide();

    connect(ui->frameResult, SIGNAL(fieldListRequested()),
            this, SLOT(onFieldListRequested()));
    connect(ui->frameResult, SIGNAL(showDetailsRequested(QString)),
            this, SLOT(onResultDetailsRequested(QString)));
    connect(ui->textQueryID, SIGNAL(returnPressed()),
            this, SLOT(on_buttonQueryID_clicked()));
    connect(ui->textQueryName, SIGNAL(returnPressed()),
            this, SLOT(on_buttonQueryName_clicked()));
    connect(ui->textMass, SIGNAL(returnPressed()),
            this, SLOT(on_buttonQueryMass_clicked()));
}

FormQuery::~FormQuery()
{
    stopQuery();
    delete ui;
}

void FormQuery::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    if (widgetMSSearchOption)
        widgetMSSearchOption->hide();
}

void FormQuery::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    ui->horizontalLayout->setGeometry(QRect(0, 0, width(), height()));
}

bool FormQuery::connectDatabase()
{
    if (!database)
    {
        database = new HmdbQueryWorker;
        qRegisterMetaType<HmdbQuery::DatabaseType>("HmdbQuery::DatabaseType");
        connect(database, SIGNAL(ready(HmdbQuery::DatabaseType)),
                this, SLOT(onDatabaseReady()));
        connect(database, SIGNAL(finished(bool)),
                this, SLOT(onQueryFinished(bool)));
    }
    if (database->isRunning())
    {
        QMessageBox::warning(this, tr("Pending task"),
                             tr("An operation is going on. "
                                "Please wait until it is finished."));
        return false;
    }
    return true;
}

bool FormQuery::checkDatabase()
{
    QString dataDir = appConfig.mainDatabase();
    if (dataDir.isEmpty())
    {
        QMessageBox::critical(this, tr("Database not set"),
                              tr("No database set yet. "
                                 "Please choose the location for "
                                 "a imported database." ));
        dialogConfig->exec();
        dataDir = appConfig.mainDatabase();
    }

    if (!connectDatabase())
        return false;

    database->setDataDirectory(dataDir, HmdbQuery::DatabaseType::Main);
    if (!database->isReady(HmdbQuery::DatabaseType::Main))
    {
        if (QMessageBox::question(this, tr("Database not indexed"),
                                  tr("The index for the main database "
                                     "has not been built yet. \n"
                                     "Do you want to build it now?"))
                == QMessageBox::Yes)
        {
            showBuildingIndexStart();
            database->getReady(HmdbQuery::DatabaseType::Main);
            database->start();
        }
        return false;
    }
    return true;
}

bool FormQuery::checkMSMSDatabase()
{
    QString dataDir = appConfig.msmsDatabase();
    if (dataDir.isEmpty())
    {
        QMessageBox::critical(this, tr("MS/MS Database not set"),
                              tr("No MS/MS database set yet. "
                                 "Please choose the location for "
                                 "a MS/MS database from HMDB."));
        dialogConfig->exec();
        dataDir = appConfig.msmsDatabase();
    }

    if (!connectDatabase())
        return false;

    database->setDataDirectory(dataDir, HmdbQuery::DatabaseType::MSMS);
    if (!database->isReady(HmdbQuery::DatabaseType::MSMS))
    {
        if (QMessageBox::question(this, tr("Database not indexed"),
                                  tr("The index for the MS/MS database "
                                     "has not been built yet. \n"
                                     "Do you want to build it now?"))
                == QMessageBox::Yes)
        {
            showBuildingIndexStart();
            database->getReady(HmdbQuery::DatabaseType::MSMS);
            database->start();
        }
        return false;
    }
    return true;
}

void FormQuery::showBuildingIndexStart()
{
    ui->labelStatus->setText(tr("Building database indexes, please wait..."));
    ui->progressQuery->setRange(0, 0);
    ui->progressQuery->show();
}

void FormQuery::showQueryStart(int queryType)
{
    ui->progressQuery->setRange(0, 2);
    ui->progressQuery->setValue(0);
    ui->progressQuery->show();

    QString statusText;
    switch (queryType)
    {
        case HMDB_QUERY_TYPE_ID:
            statusText = "Querying by metabolite ID";
            break;
        case HMDB_QUERY_TYPE_NAME:
            statusText = "Querying by metabolite name";
            break;
        case HMDB_QUERY_TYPE_MASS:
            statusText = "Querying by molecular mass";
            break;
        case HMDB_QUERY_TYPE_MSMS:
            statusText = "Querying by secondary MS";
            break;
    }
    statusText.append("...");
    ui->labelStatus->setText(statusText);
}

void FormQuery::showQueryResult(const HmdbQueryRecord& record, bool showRank)
{
    ui->labelStatus->setText(QString("Query finished, loading %1 result(s)...")
                                    .arg(record.entryCount));
    ui->progressQuery->setValue(1);
    QApplication::processEvents();

    // Convert property IDs to property names
    const QList<QString>& propertNames = listQueryField->selectedFieldNameList;
    QByteArray propertyNameString;
    for (int i=0; i<record.propertyCount; i++)
    {
        delete[] record.properties[i]->name;
        propertyNameString = propertNames[i].toLocal8Bit();
        record.properties[i]->name = new char[propertyNameString.length() + 1];
        utils_strncpy(record.properties[i]->name,
                      propertyNameString.constData(),
                      propertyNameString.length());
    }

    ui->frameResult->loadResult(record, showRank);

    ui->labelStatus->setText(QString("Query finished with %1 result(s).")
                                    .arg(record.entryCount));
}

void FormQuery::startQuery()
{
    // Set query fields
    database->setQueryProperty(listQueryField->selectedFieldIDList);

    // Launch the query thread
    database->start();
}

void FormQuery::stopQuery()
{
    if (database)
    {
        if (database->isRunning())
        {
            database->terminate();
            database->wait();
        }
            delete database;
            database = nullptr;
    }
    ui->progressQuery->setValue(0);
    ui->progressQuery->setRange(0, 100);
}

bool FormQuery::parsePeakList (QByteArray content,
                               QList<double>& mzList,
                               QList<double>& intensityList)
{
    mzList.clear();
    intensityList.clear();

    int pos1 = 0, pos2;
    double tempValue;
    bool conversionOK;
    QString buffer;
    QStringList fieldList;
    while (pos1 < content.length())
    {
        pos2 = content.indexOf('\n', pos1);
        if (pos2 < 0)
            pos2 = content.length();
        buffer = content.mid(pos1, pos2 - pos1);
        pos1 = pos2 + 1;

        if (buffer.contains('\t'))
            fieldList = buffer.split('\t', Qt::SkipEmptyParts);
        else if (buffer.contains(','))
            fieldList = buffer.split(',', Qt::SkipEmptyParts);
        else if (buffer.contains(';'))
            fieldList = buffer.split(';', Qt::SkipEmptyParts);
        else
            fieldList = buffer.split(' ', Qt::SkipEmptyParts);
        if (fieldList.length() < 1)
            continue;

        // Parse the m/z
        tempValue = fieldList[0].toDouble(&conversionOK);
        if (conversionOK)
        {
            mzList.push_back(tempValue);
            if (fieldList.length() < 2)
                continue;

            // Parse the intensity (optional)
            tempValue = fieldList[1].toDouble(&conversionOK);
            if (conversionOK || intensityList.length() > 0)
            {
                if (conversionOK)
                    intensityList.push_back(tempValue);
                else
                {
                    // Missing intensity value for this line
                    intensityList.push_back(HMDB_QUERY_MSMS_INTENSITY_DEFAULT);
                }
            }
        }
    }

    return mzList.length() > 0;
}

void FormQuery::onDatabaseReady()
{
    ui->progressQuery->hide();
    ui->labelStatus->setText(tr("Database ready."));
}

void FormQuery::onFieldListRequested()
{
    listQueryField->move(QCursor::pos());
    listQueryField->show();
}

void FormQuery::onResultDetailsRequested(QString ID)
{
    if (!viewer)
        viewer = new FormMetaboliteViewer();
    viewer->showMetabolite(ID);
    viewer->show();
    viewer->raise();
}

void FormQuery::onQueryFinished(bool successful)
{
    if (successful)
    {
        if (ui->widgetSearchOption->currentIndex() == HMDB_QUERY_TYPE_MSMS)
            showQueryResult(database->result(), true);
        else
            showQueryResult(database->result());
    }
    else
    {
        ui->labelStatus->clear();
        QMessageBox::critical(this, tr("Query failed"),
                              tr("An error occurred during query."));
    }
    ui->progressQuery->hide();
}

void FormQuery::on_comboBox_currentIndexChanged(int index)
{
    ui->widgetSearchOption->setCurrentIndex(index);
}

void FormQuery::on_buttonQueryID_clicked()
{
    QString ID = ui->textQueryID->text().trimmed();
    ui->textQueryID->setText(ID);
    if (ID.isEmpty())
        return;

    if (!checkDatabase())
        return;

    showQueryStart(HMDB_QUERY_TYPE_ID);
    database->queryByID(ID);
    startQuery();
}

void FormQuery::on_checkRelativeTolerance_stateChanged(int arg1)
{
    Q_UNUSED(arg1)
    if (ui->checkRelativeTolerance->isChecked())
        ui->labelMassToleranceUnit->setText("(ppm)");
    else
        ui->labelMassToleranceUnit->setText("(Da)");
}

void FormQuery::on_buttonQueryMass_clicked()
{
    if (!checkDatabase())
        return;

    bool conversionOK;
    double mass = ui->textMass->text().toDouble(&conversionOK);
    if (!conversionOK)
    {
        QMessageBox::warning(this, tr("Invalid mass value"),
                             tr("Please input a numeric mass value!"));
        return;
    }

    double delta = ui->textMassTolerance->text().toDouble(&conversionOK);
    if (!conversionOK)
    {
        QMessageBox::warning(this, tr("Invalid tolerance value"),
                             tr("Please input a numeric tolerance value!"));
        return;
    }
    if (ui->checkRelativeTolerance->isChecked())
    {
        // Convert "Da" to "ppm"
        delta = abs(mass * delta * 1E-6);
    }

    double min = mass - delta;
    double max = mass + delta;
    showQueryStart(HMDB_QUERY_TYPE_MASS);
    if (ui->radioAvgMass->isChecked())
        database->queryByMass(min, max);
    else
        database->queryByMonoMass(min, max);
    startQuery();
}

void FormQuery::on_buttonQueryName_clicked()
{
    QString name = ui->textQueryName->text().trimmed();
    ui->textQueryName->setText(name);
    if (name.isEmpty())
        return;

    if (!checkDatabase())
        return;

    showQueryStart(HMDB_QUERY_TYPE_NAME);
    database->queryByName(name);
    startQuery();
}

void FormQuery::on_buttonSelectMSMSFile_clicked()
{
    QString newFile =
            QFileDialog::getOpenFileName(this,
                                         tr("Select a mass spectrum file"),
                                         msmsFilePath);
    if (newFile.isEmpty())
        return;
    msmsFilePath = newFile;
    ui->textMSMSFile->setText(msmsFilePath);
}

void FormQuery::on_radioMSMSFromFile_toggled(bool checked)
{
    ui->textMSMSFile->setEnabled(checked);
    ui->buttonSelectMSMSFile->setEnabled(checked);
    ui->textMSMSPeaks->setEnabled(!checked);
}

void FormQuery::on_buttonOptionQueryMSMS_clicked()
{
    if (!widgetMSSearchOption)
        widgetMSSearchOption = new ControlMSSearchOption();
    widgetMSSearchOption->move(QCursor::pos());
    widgetMSSearchOption->show();
}

void FormQuery::on_buttonQueryMSMS_clicked()
{
    if (!checkDatabase() || !checkMSMSDatabase())
        return;

    QList<double> mzList, intensityList;
    if (ui->radioMSMSFromFile->isChecked())
    {
        QFile msPeakFile(ui->textMSMSFile->text());
        if (!msPeakFile.open(QFile::ReadOnly))
        {
            QMessageBox::critical(this, tr("Input file not accessible"),
                                  tr("The spectrum file for MS/MS search is not"
                                     " readable. Please select another one."));
            return;
        }
        parsePeakList(msPeakFile.readAll(), mzList, intensityList);
    }
    else
        parsePeakList(ui->textMSMSPeaks->toPlainText().toLocal8Bit(),
                      mzList, intensityList);
    if (mzList.length() < 1)
    {
        QMessageBox::warning(this, tr("No peak in the spectrum"),
                             tr("No peak (m/z value) was found "
                                "in the give spectrum."));
        return;
    }

    HmdbQuery::MassSpectrumMode ionizationMode = HmdbQuery::Positive;
    double massTolerance = HMDB_QUERY_MSMS_TOLERANCE_DEFAULT;
    bool relativeTolerance = false;
    if (widgetMSSearchOption)
    {
        // User has specified some search options
        ionizationMode =
            HmdbQuery::MassSpectrumMode(widgetMSSearchOption->ionizationMode());
        massTolerance = widgetMSSearchOption->relativeMassTolerance() ?
                               widgetMSSearchOption->massTolerance() * 1E-6 :
                               widgetMSSearchOption->massTolerance();
        relativeTolerance = widgetMSSearchOption->relativeMassTolerance();
    }
    showQueryStart(HMDB_QUERY_TYPE_MSMS);
    database->queryByMSMS(massTolerance, relativeTolerance, ionizationMode,
                          mzList, intensityList);
    startQuery();
}
