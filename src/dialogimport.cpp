#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include "dialogimport.h"
#include "ui_dialogimport.h"


DialogImport::DialogImport(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DialogImport)
{
    initialized = false;

    ui->setupUi(this);
}

DialogImport::~DialogImport()
{
    delete ui;
}

void DialogImport::showEvent(QShowEvent* event)
{
    if (!initialized)
    {
        button(QWizard::NextButton)->setEnabled(false);
        initialized = true;
    }
}

void DialogImport::startImport()
{
    ui->progressImport->setValue(-1);

    importer.setSourceXML(importXML.toLocal8Bit().constData());
    importer.setTargetDir(importDir.toLocal8Bit().constData());
    if (!importer.import())
    {
        QMessageBox::critical(this, "Import failed",
                              "An error occurred during import.");
        return;
    }

    ui->progressImport->setValue(100);
    emit DirectoryChanged(importDir);
}

void DialogImport::on_buttonChooseFile_clicked()
{
    QString fileName;

    fileName = QFileDialog::getOpenFileName(this, "Choose a XML file");
    if (fileName.isEmpty())
        return;

    ui->textImportFilename->setText(fileName);
}

void DialogImport::on_buttonChooseDir_clicked()
{
    QString dirName;

    dirName = QFileDialog::getExistingDirectory(this, "Choose a directory");
    if (dirName.isEmpty())
        return;

    ui->textImportDirectory->setText(dirName);
}

void DialogImport::on_DialogImport_currentIdChanged(int id)
{
    switch (id)
    {
        case 2:
            importXML = ui->textImportFilename->text();
            importDir = ui->textImportDirectory->text();
            qApp->processEvents();
            startImport();
            break;
        default:;
    }
}

void DialogImport::on_pushButton_clicked()
{
    QDesktopServices::openUrl(importDir.prepend("file:///"));
}

void DialogImport::on_textImportFilename_textChanged(const QString &arg1)
{
    button(QWizard::NextButton)->setEnabled(
                !ui->textImportFilename->text().isEmpty());
}

void DialogImport::on_textImportDirectory_textChanged(const QString &arg1)
{
    button(QWizard::NextButton)->setEnabled(
                !ui->textImportDirectory->text().isEmpty());
}
