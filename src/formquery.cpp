#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include "formquery.h"
#include "ui_formquery.h"

#define HMDB_QUERY_TYPE_ID        1
#define HMDB_QUERY_TYPE_NAME      2
#define HMDB_QUERY_TYPE_MASS      3


FormQuery::FormQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQuery)
{
    ui->setupUi(this);

    resultLoaded = false;

    ui->viewSearchResult->setModel(&modelResult);
}

FormQuery::~FormQuery()
{
    delete ui;
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
        return false;
    }
    if (!database.isReady())
    {
        if (QMessageBox::question(this, "Database not indexed",
                                  "The index for HMDB has not been built yet. \n"
                                  "Do you want to build it now?")
                != QMessageBox::Yes)
            return false;
        database.getReady();
    }
    return database.isReady();
}

void FormQuery::setDataDirectory(QString dir)
{
    dataDir = dir;
    ui->textDatabase->setText(dataDir);
    database.setDataDirectory(dataDir.toLocal8Bit().constData());
}

void FormQuery::resizeEvent(QResizeEvent* event)
{
    ui->horizontalLayout->setGeometry(QRect(0, 0, width(), height()));
}

void FormQuery::showQueryResult(const HmdbQueryRecord& record)
{
    if (resultLoaded)
        saveColumnWidth();

    modelResult.clear();
    modelResult.setHorizontalHeaderLabels(QStringList() <<
                                          "ID" <<
                                          "Name" <<
                                          "Formula" <<
                                          "Mass" <<
                                          "Mono. Mass");

    int i, j;
    HmdbQueryRecordEntry* entry;
    QList<QStandardItem*> rowItems;
    for (i=0; i<record.entryCount; i++)
    {
        entry = record.entries[i];

        rowItems.clear();
        rowItems.push_back(new QStandardItem(entry->ID));
        for (j=0; j<entry->propertyCount; j++)
        {
            rowItems.push_back(new QStandardItem(entry->propertyValues[j]));
        }
        modelResult.appendRow(rowItems);
    }

    // Resize each column when first loaded
    if (!resultLoaded)
    {
        ui->viewSearchResult->resizeColumnsToContents();
        resultLoaded = true;
    }
    else
        restoreColumnWidth();
}

void FormQuery::saveColumnWidth()
{
    listColumnWidth.clear();
    for (int i=0; i<modelResult.columnCount(); i++)
        listColumnWidth.push_back(ui->viewSearchResult->columnWidth(i));
}

void FormQuery::restoreColumnWidth()
{
    for (int i=0; i<modelResult.columnCount(); i++)
        ui->viewSearchResult->setColumnWidth(i, listColumnWidth[i]);
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
    if (!checkDatabase())
        return;
    
    showQueryResult(database.queryID(
                        ui->textQueryID->text().toLocal8Bit().constData()));
}

void FormQuery::on_checkRelativeTolerance_stateChanged(int arg1)
{
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
    if (ui->radioAvgMass->isChecked())
        showQueryResult(database.queryMass(min, max));
    else
        showQueryResult(database.queryMonoMass(min, max));

}
