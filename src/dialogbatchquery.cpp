#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include "dialogbatchquery.h"
#include "widgets/controlmassmodificationlist.h"
#include "ui_dialogbatchquery.h"
#include "hmdb/hmdbxml_def.h"
#include "threads/hmdbbatchqueryworker.h"
#include "hmdb/hmdbqueryoptions.h"

#define HMDB_BATCH_TARGET_SUFFIX        "_QueryResult"


DialogBatchQuery::DialogBatchQuery(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DialogBatchQuery)
{
    ui->setupUi(this);

    resetQueryFields();

    ui->viewQueryFields->setModel(&modelQueryFields);
    ui->viewQueryFields->horizontalHeader()
                            ->setSectionResizeMode(QHeaderView::Stretch);

    massModificationList = new ControlMassModificationList();
    ui->frameMassModification->addWidget(massModificationList);
    searchEngine = nullptr;
    lastQuerySuccessful = false;
}

void DialogBatchQuery::setDataDirectory(QString dir)
{
    dataDir = dir;
}

DialogBatchQuery::~DialogBatchQuery()
{
    stopQuery();
    delete massModificationList;
    delete ui;
}

void DialogBatchQuery::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    if (!dataDir.isEmpty())
        ui->textDatabase->setText(dataDir);
    if (!(searchEngine && searchEngine->isRunning()))
        restart();
}

bool DialogBatchQuery::validateCurrentPage()
{
    switch (currentId())
    {
        case PageQueryType:
            if (ui->textDatabase->text().isEmpty())
            {
                QMessageBox::warning(this, "Missing database",
                                     "Please select a database for query.");
                return false;
            }
            break;
        case PageSelectFile:
            if (ui->textSourcePath->text().isEmpty() ||
                ui->textTargetPath->text().isEmpty())
            {
                QMessageBox::warning(this, "Missing fields",
                                     "Please select a source data file "
                                     "and the target file.");
                return false;
            }
            break;
        case PageQueryField:
        {
            int checkedRowCount = 0;
            for (int i=0; i<modelQueryFields.rowCount(); i++)
            {
                if (modelQueryFields.item(0, 0)->checkState() == Qt::Checked)
                    checkedRowCount++;
            }
            if (checkedRowCount == 0)
            {
                QMessageBox::warning(this, "No query field selected",
                                     "Please select at least one field to "
                                     "perform the query.");
                return false;
            }
            break;
        }
        default:;
    }
    return true;
}

void DialogBatchQuery::done(int r)
{
    if (r == QDialog::Rejected)
    {
        if (searchEngine && searchEngine->isRunning())
        {
            if (QMessageBox::warning(this, "Stop running tasks",
                                     "A query is being processed. "
                                     "Do you want to terminate it now?",
                                     QMessageBox::Yes | QMessageBox::No)
                    != QMessageBox::Yes)
                return;
        }
        stopQuery();
        hide();
    }
}

void DialogBatchQuery::resetQueryFields()
{
    QStandardItemModel& model = modelQueryFields;
    model.clear();
    model.setColumnCount(3);
    model.setHorizontalHeaderItem(0, new QStandardItem("Enabled"));
    model.setHorizontalHeaderItem(1, new QStandardItem("Field Name"));
    model.setHorizontalHeaderItem(2, new QStandardItem("Field ID"));

    // Define some default fields
    QFile resource(":/resource/HMDB-Fields.csv");
    if (!resource.open(QFile::ReadOnly))
        return;

    QByteArray buffer;
    QList<QByteArray> properties;
    QList<QStandardItem*> row;
    resource.readLine();  // Skip the header line
    while(!resource.atEnd())
    {
        buffer = resource.readLine();
        buffer.truncate(buffer.lastIndexOf('\n'));
        properties = buffer.split(',');
        if (properties.length() == 0)
            continue;

        // Insert this field
        row.push_back(new QStandardItem());
        row.push_back(new QStandardItem(QString(properties[0])));
        row.push_back(new QStandardItem(QString(properties[1])));
        row[0]->setCheckable(true);
        if (properties[2] == "Y")
            row[0]->setCheckState(Qt::Checked);
        model.appendRow(row);
        row.clear();
    }
}

bool DialogBatchQuery::launchQuery()
{
    if (!searchEngine)
    {
        searchEngine = new HmdbBatchQueryWorker;
        connect(searchEngine, SIGNAL(progressed(double)),
                this, SLOT(onBatchQueryProgressed(double)),
                Qt::QueuedConnection);
        connect(searchEngine, SIGNAL(finished(bool)),
                this, SLOT(onBatchQueryFinished(bool)),
                Qt::QueuedConnection);
    }
    else
    {
        if (searchEngine->status() == HmdbQueryStatus::Working)
            searchEngine->stop();
    }

    dataDir = ui->textDatabase->text();
    searchEngine->setDataDirectory(dataDir.toLocal8Bit().constData());
    searchEngine->setSourcePath(
                ui->textSourcePath->text().toLocal8Bit().constData());
    searchEngine->setResultPath(
                ui->textTargetPath->text().toLocal8Bit().constData());

    // Load query properties
    int i;
    QStringList queryFields;
    for (i=0; i<modelQueryFields.rowCount(); i++)
    {
        if (modelQueryFields.item(i, 0)->checkState() == Qt::Checked)
        {
            queryFields.push_back(
                        modelQueryFields.item(i, 2)->text());
        }
    }
    char** queryProperties = new char*[queryFields.length()];
    for (i=0; i<queryFields.length(); i++)
    {
        queryProperties[i] = new char[queryFields[i].length() + 1];
        strcpy(queryProperties[i], queryFields[i].toLocal8Bit().constData());
    }
    searchEngine->setQueryProperty(queryProperties, queryFields.length());
    for (i=0; i<queryFields.length(); i++)
        delete[] queryProperties[i];
    delete[] queryProperties;

    switch(ui->comboQueryType->currentIndex())
    {
        case 0: // Query by Mass
        {
            searchEngine->setQueryType(HmdbBatchQueryWorker::ByMass);

            bool conversionOK;
            ui->textMassTolerance->text().toDouble(&conversionOK);
            if (!conversionOK)
            {
                QMessageBox::warning(this, "Invalid mass tolerance value",
                                     "Please input a numeric value for the "
                                     "mass tolerance.");
                return false;
            }

            // Load options to our search engine
            searchEngine->clearOptions();
            if (ui->radioMonoMass->isChecked())
                searchEngine->setOption(HMDB_QUERY_OPTION_MASS_MONOISOTOPIC);
            if (ui->checkRelativeMassTolerance->isChecked())
                searchEngine->setOption(HMDB_QUERY_OPTION_MASS_RELATIVE_TOL);
            searchEngine->setOption(HMDB_QUERY_OPTION_MASS_TOLERANCE,
                       ui->textMassTolerance->text().toLocal8Bit().constData());

            // More options (mass modification types)
            QList<int> indexList = massModificationList->selectedIndexes();
            QStringList massModification, massModificationNames;
            for (i=0; i<indexList.length(); i++)
            {
                massModification.push_back(
                                massModificationList->mz(indexList[i]));
                massModificationNames.push_back(
                                massModificationList->formula(indexList[i]));
            }
            searchEngine->setOption(HMDB_QUERY_OPTION_MASS_MODIFICATION,
                           massModification.join(HMDB_QUERY_OPTION_FIELD_SEP)
                                           .toLocal8Bit().constData());
            searchEngine->setOption(HMDB_QUERY_OPTION_MASS_MOD_NAME,
                        massModificationNames.join(HMDB_QUERY_OPTION_FIELD_SEP)
                                             .toLocal8Bit().constData());

            break;
        }
        default:;
    }

    // Lock command buttons...
    button(QWizard::BackButton)->setEnabled(false);
    button(QWizard::NextButton)->setEnabled(false);
    ui->progressBatch->setValue(0);

    // Start the query in a new thread
    searchEngine->start();
    return true;
}

void DialogBatchQuery::stopQuery()
{
    if (searchEngine)
    {
        if (searchEngine->isRunning())
        {
            searchEngine->terminate();
            searchEngine->wait();
        }
        delete searchEngine;
        searchEngine = nullptr;
    }
    ui->progressBatch->setValue(0);
}

void DialogBatchQuery::onBatchQueryProgressed(double progress)
{
    ui->progressBatch->setValue(int(progress));
}

void DialogBatchQuery::onBatchQueryFinished(bool successful)
{
    lastQuerySuccessful = successful;
    next();
}

void DialogBatchQuery::on_buttonSelectSourcePath_clicked()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, "Choose a data file");
    if (fileName.isEmpty())
        return;

    ui->textSourcePath->setText(fileName);

    int fileExtensionPos = fileName.lastIndexOf('.');
    if (fileExtensionPos > 0)
    {
        // Automatically fill the target filename
        QString targetFileName(
                    fileName.mid(0, fileExtensionPos)
                            .append(HMDB_BATCH_TARGET_SUFFIX)
                            .append(fileName.mid(fileExtensionPos)));
        ui->textTargetPath->setText(targetFileName);
    }
}

void DialogBatchQuery::on_buttonSelectTargetPath_clicked()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, "Choose a target file");
    if (fileName.isEmpty())
        return;

    ui->textTargetPath->setText(fileName);
}

void DialogBatchQuery::on_buttonViewResult_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->textTargetPath->text()));
}

void DialogBatchQuery::on_DialogBatchQuery_currentIdChanged(int id)
{
    switch (id)
    {
        case PageQueryProgress:
            if (!launchQuery())
                back();
            break;
        case PageQueryFinished:
            button(QWizard::BackButton)->setEnabled(false);
            if (lastQuerySuccessful)
                ui->labelQueryResult->setText("Query completed successfully.");
            else
                ui->labelQueryResult->setText("Something went wrong.");
            break;
        default:;
    }
}

void DialogBatchQuery::on_buttonSelectDatabase_clicked()
{
    QString newDir = QFileDialog::getExistingDirectory(this,
                                              "Select a database directory",
                                              dataDir);
    if (!newDir.isEmpty())
        ui->textDatabase->setText(newDir);
}
