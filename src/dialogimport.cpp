#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include "dialogimport.h"
#include "ui_dialogimport.h"
#include "global.h"
#include "threads/hmdbxmlimporterworker.h"


DialogImport::DialogImport(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DialogImport)
{
    initialized = false;

    ui->setupUi(this);

    importer = nullptr;
}

DialogImport::~DialogImport()
{
    stop();
    delete ui;
}

void DialogImport::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    if (!initialized)
    {
        button(QWizard::NextButton)->setEnabled(false);
        initialized = true;
    }
    if (!(importer && importer->isRunning()))
        restart();
}

bool DialogImport::startImport()
{
    if (!importer)
    {
        importer = new HmdbXMLImporterWorker(this);
        connect(importer, SIGNAL(progressed(double)),
                this, SLOT(onImporterProgressed(double)),
                Qt::QueuedConnection);
        connect(importer, SIGNAL(finished(bool)),
                this, SLOT(onImporterFinished(bool)),
                Qt::QueuedConnection);
    }
    if (!importer->setSourceXML(ui->textImportFilename->text()))
    {
        QMessageBox::critical(this, "Source file not accessible",
                              "The selected source file is not readable. "
                              "Please choose another one.");
        return false;
    }
    if (!importer->setTargetDir(ui->textImportDirectory->text()))
    {
        QMessageBox::critical(this, "Target directory not accessible",
                              "The target directory is not writable. "
                              "Please choose another one.");
        return false;
    }

    // Lock command buttons...
    button(QWizard::BackButton)->setEnabled(false);
    button(QWizard::NextButton)->setEnabled(false);
    ui->progressImport->setValue(0);

    // Start importing in a new thread
    importer->start();
    return true;
}

void DialogImport::stop()
{
    if (importer)
    {
        if (importer->isRunning())
        {
            importer->terminate();
            importer->wait();
        }
        delete importer;
        importer = nullptr;
    }
    ui->progressImport->setValue(0);
}

void DialogImport::done(int r)
{
    if (r == QDialog::Rejected)
    {
        if (importer && importer->isRunning())
        {
            if (QMessageBox::warning(this, "Stop importing",
                                     "An importing operation is going on. "
                                     "Do you want to terminate it now?",
                                     QMessageBox::Yes | QMessageBox::No)
                    != QMessageBox::Yes)
                return;
        }
        stop();
        hide();
    }
}

void DialogImport::onImporterProgressed(double finishedPercent)
{
    ui->progressImport->setValue(int(finishedPercent));
}

void DialogImport::onImporterFinished(bool successful)
{
    if (successful)
    {
        if (QMessageBox::information(this, "Finished importing",
                                 "Importing succeeded. "
                                 "Would you like to set the imported database "
                                 "as default database for query?",
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::Yes)
                == QMessageBox::Yes)
            appConfig.setMainDatabase(ui->textImportDirectory->text());
    }
    else
        QMessageBox::critical(this, "Import failed",
                              "An error occurred during import.");
    next();
    button(QWizard::BackButton)->setEnabled(false);
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
            if (!startImport())
                back();
            break;
        default:;
    }
}

void DialogImport::on_pushButton_clicked()
{
    QDesktopServices::openUrl(
                ui->textImportDirectory->text().prepend("file:///"));
}

void DialogImport::on_textImportFilename_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    button(QWizard::NextButton)->setEnabled(
                !ui->textImportFilename->text().isEmpty());
}

void DialogImport::on_textImportDirectory_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    button(QWizard::NextButton)->setEnabled(
                !ui->textImportDirectory->text().isEmpty());
}
