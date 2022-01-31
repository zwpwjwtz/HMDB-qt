#include <QFileDialog>
#include "dialogconfig.h"
#include "ui_dialogconfig.h"
#include "global.h"


DialogConfig::DialogConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConfig)
{
    ui->setupUi(this);

    ui->textMainDatabase->setText(appConfig.mainDatabase());
    ui->textMSMSDatabase->setText(appConfig.msmsDatabase());
}

DialogConfig::~DialogConfig()
{
    delete ui;
}

void DialogConfig::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void DialogConfig::on_buttonSetMainDatabase_clicked()
{
    QString newDir = QFileDialog::getExistingDirectory(this,
                                     "Select the main database directory "
                                     "(containing metabolite info)",
                                      ui->textMainDatabase->text());
    if (!newDir.isEmpty())
    {
        appConfig.setMainDatabase(newDir);
        ui->textMainDatabase->setText(newDir);
    }
}

void DialogConfig::on_buttonSetMSMSDatabase_clicked()
{
    QString newDir = QFileDialog::getExistingDirectory(this,
                                     "Select the MS/MS database directory "
                                     "(containing MS/MS spectra)",
                                      ui->textMSMSDatabase->text());
    if (!newDir.isEmpty())
    {
        appConfig.setMSMSDatabase(newDir);
        ui->textMSMSDatabase->setText(newDir);
    }
}
