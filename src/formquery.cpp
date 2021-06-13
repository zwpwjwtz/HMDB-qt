#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include "formquery.h"
#include "ui_formquery.h"
#include "widgets/controlmssearchoption.h"
#include "threads/hmdbqueryworker.h"


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
    widgetMSSearchOption = nullptr;
    database = nullptr;

    ui->widgetSearchOption->setCurrentIndex(HMDB_QUERY_TYPE_ID);
    ui->textMSMSPeaks->setPlaceholderText(
                "mz1 Intensity1\n"
                "mz2 Intensity2\n"
                "mz3 Intensity3\n");
    ui->progressQuery->hide();
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
        QMessageBox::warning(this, "Pending task",
                             "An operation is going on. "
                             "Please wait until it is finished.");
        return false;
    }
    return true;
}

bool FormQuery::checkDatabase()
{
    if (dataDir.isEmpty())
    {
        QMessageBox::critical(this, "Database not set",
                              "No database set yet. "
                              "Please choose the location for a imported database." 
                              );
        on_buttonSetDatabase_clicked();
    }

    if (!connectDatabase())
        return false;

    database->setDataDirectory(dataDir, HmdbQuery::DatabaseType::Main);
    if (!database->isReady(HmdbQuery::DatabaseType::Main))
    {
        if (QMessageBox::question(this, "Database not indexed",
                                  "The index for HMDB has not been built yet. \n"
                                  "Do you want to build it now?")
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
    if (msmsDataDir.isEmpty())
    {
        QMessageBox::critical(this, "MS/MS Database not set",
                              "No MS/MS database set yet. "
                              "Please choose the location for a imported database."
                              );
        QString newDir = QFileDialog::getExistingDirectory(this,
                                         "Select a database directory",
                                          dataDir);
        if (newDir.isEmpty())
            return false;
        msmsDataDir = newDir;
    }

    if (!connectDatabase())
        return false;

    database->setDataDirectory(msmsDataDir, HmdbQuery::DatabaseType::MSMS);
    if (!database->isReady(HmdbQuery::DatabaseType::MSMS))
    {
        if (QMessageBox::question(this, "Database not indexed",
                                  "The index for the MS/MS database "
                                  "has not been built yet. \n"
                                  "Do you want to build it now?")
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

void FormQuery::setDataDirectory(QString dir)
{
    dataDir = dir;
    ui->textDatabase->setText(dataDir);
}

void FormQuery::showBuildingIndexStart()
{
    ui->labelStatus->setText("Building database indexes, please wait...");
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

    ui->frameResult->loadResult(record, showRank);

    ui->labelStatus->setText(QString("Query finished with %1 result(s).")
                                    .arg(record.entryCount));
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
            fieldList = buffer.split('\t', QString::SkipEmptyParts);
        else if (buffer.contains(','))
            fieldList = buffer.split(',', QString::SkipEmptyParts);
        else if (buffer.contains(';'))
            fieldList = buffer.split(';', QString::SkipEmptyParts);
        else
            fieldList = buffer.split(' ', QString::SkipEmptyParts);
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
    ui->labelStatus->setText("Database ready.");
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
        QMessageBox::critical(this, "Query failed",
                              "An error occurred during query.");
    }
    ui->progressQuery->hide();
}

void FormQuery::on_comboBox_currentIndexChanged(int index)
{
    ui->widgetSearchOption->setCurrentIndex(index);
}

void FormQuery::on_buttonSetDatabase_clicked()
{
    QString newDir = QFileDialog::getExistingDirectory(this,
                                     "Select a database directory",
                                      dataDir);
    if (!newDir.isEmpty())
        setDataDirectory(newDir);
}

void FormQuery::on_buttonQueryID_clicked()
{
    if (ui->textQueryID->text().isEmpty())
        return;

    if (!checkDatabase())
        return;

    showQueryStart(HMDB_QUERY_TYPE_ID);
    database->queryByID(ui->textQueryID->text());
    database->start();
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
        QMessageBox::warning(this, "Invalid mass value",
                             "Please input a numeric mass value!");
        return;
    }

    double delta = ui->textMassTolerance->text().toDouble(&conversionOK);
    if (!conversionOK)
    {
        QMessageBox::warning(this, "Invalid tolerance value",
                             "Please input a numeric tolerance value!");
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
    database->start();

}

void FormQuery::on_buttonQueryName_clicked()
{
    if (ui->textQueryName->text().isEmpty())
        return;

    if (!checkDatabase())
        return;

    showQueryStart(HMDB_QUERY_TYPE_NAME);
    database->queryByName(ui->textQueryName->text());
    database->start();
}

void FormQuery::on_buttonSelectMSMSFile_clicked()
{
    QString newFile = QFileDialog::getOpenFileName(this,
                                               "Select a mass spectrum file",
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
            QMessageBox::critical(this, "Input file not accessible",
                                  "The spectrum file for MS/MS search is not "
                                  "readable. Please select another one.");
            return;
        }
        parsePeakList(msPeakFile.readAll(), mzList, intensityList);
    }
    else
        parsePeakList(ui->textMSMSPeaks->toPlainText().toLocal8Bit(),
                      mzList, intensityList);
    if (mzList.length() < 1)
    {
        QMessageBox::warning(this, "No peak in the spectrum",
                             "No peak (m/z value) found in the give spectrum.");
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
    database->start();
}
