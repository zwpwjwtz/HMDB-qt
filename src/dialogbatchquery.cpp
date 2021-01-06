#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include "dialogbatchquery.h"
#include "ui_dialogbatchquery.h"
#include "hmdb/hmdbxml_def.h"

#define HMDB_BATCH_TARGET_SUFFIX        "_QueryResult"


DialogBatchQuery::DialogBatchQuery(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DialogBatchQuery)
{
    ui->setupUi(this);

    resetMassModification();
    resetQueryFields();

    ui->viewMassModification->setModel(&modelMassModification);
    ui->viewMassModification->horizontalHeader()
                            ->setSectionResizeMode(QHeaderView::Stretch);
    ui->viewQueryFields->setModel(&modelQueryFields);
    ui->viewQueryFields->horizontalHeader()
                            ->setSectionResizeMode(QHeaderView::Stretch);
}

DialogBatchQuery::~DialogBatchQuery()
{
    delete ui;
}

void DialogBatchQuery::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    restart();
}

bool DialogBatchQuery::validateCurrentPage()
{
    switch (currentId())
    {
        case 1: // Select data files
            if (ui->textSourcePath->text().isEmpty() ||
                ui->textTargetPath->text().isEmpty())
            {
                QMessageBox::warning(this, "Missing fields",
                                     "Please select a source data file "
                                     "and the target file.");
                return false;
            }
            break;
        case 3:
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

void DialogBatchQuery::resetMassModification()
{
    QStandardItemModel& model = modelMassModification;
    model.clear();
    model.setColumnCount(3);
    model.setHorizontalHeaderItem(0, new QStandardItem("Mass"));
    model.setHorizontalHeaderItem(1, new QStandardItem("Formula"));
    model.setHorizontalHeaderItem(2, new QStandardItem("Description"));

    // Define some default fields
    QFile resource(":/resource/MassModification-negative.csv");
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
        row.push_back(new QStandardItem(QString(properties[0])));
        row.push_back(new QStandardItem(QString(properties[1])));
        row.push_back(new QStandardItem(QString(properties[2])));
        row[0]->setCheckable(true);
        model.appendRow(row);
        if (properties[3] == "0")
            row[0]->setCheckState(Qt::Checked);
        row.clear();
    }
}

void DialogBatchQuery::resetQueryFields()
{
    QStandardItemModel& model = modelQueryFields;
    model.clear();
    model.setColumnCount(2);
    model.setHorizontalHeaderItem(0, new QStandardItem("Field Name"));
    model.setHorizontalHeaderItem(1, new QStandardItem("Field ID"));

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
        row.push_back(new QStandardItem(QString(properties[0])));
        row.push_back(new QStandardItem(QString(properties[1])));
        row[0]->setCheckable(true);
        if (properties[2] == "Y")
            row[0]->setCheckState(Qt::Checked);
        model.appendRow(row);
        row.clear();
    }
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
    QDesktopServices::openUrl(ui->textTargetPath->text().prepend("file:///"));
}
