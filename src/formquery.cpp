#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include "formquery.h"
#include "ui_formquery.h"

#define HMDB_QUERY_TYPE_ID        1
#define HMDB_QUERY_TYPE_NAME      2
#define HMDB_QUERY_TYPE_MASS      3
#define HMDB_QUERY_TYPE_MONOMASS  4


FormQuery::FormQuery(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQuery)
{
    ui->setupUi(this);

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
    return true;
}

void FormQuery::setDataDirectory(QString dir)
{
    dataDir = dir;
    ui->textDatabase->setText(dataDir);
    database.setDataDirectory(dataDir.toLocal8Bit().constData());
}

void FormQuery::resizeEvent(QResizeEvent* event)
{
    ui->horizontalLayoutWidget->setGeometry(0, 0, width(), height());
}

void FormQuery::showQueryResult(const HmdbQueryRecord& record)
{
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
        for (j=0; j<entry->propertCount; j++)
        {
            rowItems.push_back(new QStandardItem(entry->properties[j]));
        }
        modelResult.appendRow(rowItems);
    }
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
