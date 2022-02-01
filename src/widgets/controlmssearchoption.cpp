#include <QMessageBox>
#include "controlmssearchoption.h"
#include "ui_controlmssearchoption.h"


ControlMSSearchOption::ControlMSSearchOption(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlMSSearchOption)
{
    ui->setupUi(this);
}

ControlMSSearchOption::~ControlMSSearchOption()
{
    delete ui;
}

ControlMSSearchOption::IonizationMode ControlMSSearchOption::ionizationMode()
{
    return ControlMSSearchOption::IonizationMode(_ionizationMode);
}

void ControlMSSearchOption::setIonizationMode(IonizationMode mode)
{
    _ionizationMode = mode;
}

double ControlMSSearchOption::massTolerance()
{
    return _massTolerance;
}

void ControlMSSearchOption::setMassTolerance(double tolerance)
{
    _massTolerance = tolerance;
}

bool ControlMSSearchOption::relativeMassTolerance()
{
    return _relativeMassTolerance;
}

void ControlMSSearchOption::setRelativeMassTolerance(bool relativeTolerance)
{
    _relativeMassTolerance = relativeTolerance;
}

void ControlMSSearchOption::on_buttonOK_clicked()
{
    if (ui->radioNegativeIon->isChecked())
        _ionizationMode = Negative;
    else
        _ionizationMode = Positive;

    bool conversionOK;
    _massTolerance = ui->textMassTolerance->text().toDouble(&conversionOK);
    if (!conversionOK)
    {
        QMessageBox::warning(this, tr("Invalid input"),
                             tr("The mass tolerance must be a numeric value."));
        return;
    }
    _relativeMassTolerance = ui->radioRelativeTolerance->isChecked();
    hide();
}

void ControlMSSearchOption::on_buttonCancel_clicked()
{
    hide();
}
