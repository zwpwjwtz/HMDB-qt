#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QUrl>
#include "framequeryresult.h"
#include "ui_framequeryresult.h"
#include "../hmdb/hmdbquery.h"

#define HMDB_METABOLITES_URL_PREFIX    "https://hmdb.ca/metabolites/"

#define HMDB_QUERY_RESULT_OPENLINK_MAX 10

#define UCONFIG_EDITOR_FILE_SUFFIX_ALL "All (*)"
#define UCONFIG_EDITOR_FILE_SUFFIX_CSV "CSV (*.csv)"


FrameQueryResult::FrameQueryResult(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::FrameQueryResult)
{
    ui->setupUi(this);

    resultLoaded = false;
    modelResult = new QStandardItemModel(this);
    modelResultProxy = new QSortFilterProxyModel(this);
    modelResultProxy->setSourceModel(modelResult);
    ui->viewSearchResult->setModel(modelResultProxy);

    connect(ui->viewSearchResult->horizontalHeader(),
            SIGNAL(sectionClicked(int)),
            this,
            SLOT(onViewHeaderSearchResultClicked(int)));
}

FrameQueryResult::~FrameQueryResult()
{
    delete ui;
}

void FrameQueryResult::changeEvent(QEvent *e)
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

void FrameQueryResult::loadResult(const HmdbQueryRecord& record, bool showRank)
{
    if (resultLoaded)
        saveColumnWidth();

    modelResult->clear();
    columnSortAscending.clear();

    QStringList headerLables;
    headerLables << "ID" << "Name" << "Formula" << "Mass" << "Mono. Mass";
    if (showRank)
        headerLables << "Rank";
    modelResult->setHorizontalHeaderLabels(headerLables);

    int i, j;
    double tempValue;
    bool conversionOK;
    HmdbQueryRecordEntry* entry;
    QList<QStandardItem*> rowItems;
    for (i=0; i<record.entryCount; i++)
    {
        entry = record.entries[i];
        if (!entry)
        {
            // Invalid record entry; skip it
            continue;
        }

        rowItems.clear();
        rowItems.push_back(new QStandardItem(entry->ID));
        for (j=0; j<entry->propertyCount; j++)
        {
            rowItems.push_back(new QStandardItem(entry->propertyValues[j]));
        }
        if (showRank)
            rowItems.push_back(new QStandardItem(QString::number(entry->rank)));

        // Mark numeric values so that they can be sorted properly if needed
        for (j=0; j<rowItems.count(); j++)
        {
            tempValue = rowItems[j]->text().toDouble(&conversionOK);
            if (conversionOK)
                rowItems[j]->setData(tempValue, Qt::DisplayRole);
        }

        modelResult->appendRow(rowItems);
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

void FrameQueryResult::saveColumnWidth()
{
    listColumnWidth.clear();
    for (int i=0; i<modelResult->columnCount(); i++)
        listColumnWidth.push_back(ui->viewSearchResult->columnWidth(i));
}

void FrameQueryResult::restoreColumnWidth()
{
    for (int i=0; i<modelResult->columnCount(); i++)
    {
        if (i >= listColumnWidth.length())
            break;
        ui->viewSearchResult->setColumnWidth(i, listColumnWidth[i]);
    }
}

void FrameQueryResult::onViewHeaderSearchResultClicked(int columnIndex)
{
    if (columnSortAscending.length() < 1)
    {
        for (int i=0; i<modelResult->columnCount(); i++)
            columnSortAscending.push_back(false);
    }
    columnSortAscending[columnIndex] = !columnSortAscending[columnIndex];
    modelResult->sort(columnIndex,
                      columnSortAscending[columnIndex] ?
                          Qt::AscendingOrder : Qt::DescendingOrder);
}

void FrameQueryResult::on_buttonOpenLink_clicked()
{
    QModelIndexList selectedIndexes =
                    ui->viewSearchResult->selectionModel()->selectedIndexes();
    if (selectedIndexes.count() < 1)
        return;

    // Sort the selected indexes (by row as default),
    // and keep only one index for each line
    std::sort(selectedIndexes.begin(), selectedIndexes.end());
    QList<QModelIndex>::iterator i = selectedIndexes.begin() + 1;
    while (i != selectedIndexes.end())
    {
        if (i->row() == (i - 1)->row())
            i = selectedIndexes.erase(i);
        else
            i++;
    }

    QString recordID;
    for (i=selectedIndexes.begin(); i!=selectedIndexes.end(); i++)
    {
        if (i - selectedIndexes.begin() + 1 > HMDB_QUERY_RESULT_OPENLINK_MAX)
        {
            QMessageBox::warning(this, "Too many records selected",
                                 QString("You have selected %1 records.\n"
                                         "Only links to the first %2 records "
                                         "will be opened.")
                                        .arg(selectedIndexes.count())
                                        .arg(HMDB_QUERY_RESULT_OPENLINK_MAX));
            break;
        }

        recordID = i->siblingAtColumn(0).data().toString();
        if (!recordID.isEmpty())
        {
            QDesktopServices::openUrl(
                            recordID.prepend(HMDB_METABOLITES_URL_PREFIX));
        }
    }
}

void FrameQueryResult::on_buttonSave_clicked()
{
    QString filter;
    filter.append(UCONFIG_EDITOR_FILE_SUFFIX_ALL).append(";;")
          .append(UCONFIG_EDITOR_FILE_SUFFIX_CSV).append(";;");

    if (lastSavePathFilter.isEmpty())
        lastSavePathFilter = UCONFIG_EDITOR_FILE_SUFFIX_CSV;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save query result as CSV",
                                                    lastSavePath,
                                                    filter,
                                                    &lastSavePathFilter);
    if (fileName.isEmpty())
        return;

    QFile f(fileName);
    if (!f.open(QFile::WriteOnly))
    {
        QMessageBox::critical(this, "Failed writing to file",
                              QString("Failed opening file %1 to write.")
                                     .arg(fileName));
        return;
    }

    // Write the table header
    f.write("ID,Name,Formula,Mass,Mono. Mass\n");

    int i, j;
    int columnCount = modelResultProxy->columnCount();
    for (i=0; i<modelResultProxy->rowCount(); i++)
    {
        for (j=0; j<columnCount; j++)
        {
            if (j > 0)
                f.putChar(',');
            f.putChar('"');
            f.write(modelResultProxy->index(i, j)
                                    .data().toString().toLocal8Bit());
            f.putChar('"');
        }
        f.putChar('\n');
    }
    f.close();

    lastSavePath = QFileInfo(fileName).path();
}
