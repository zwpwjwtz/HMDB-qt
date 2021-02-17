#ifndef ControlMSSearchOption_H
#define ControlMSSearchOption_H

#include <QWidget>


namespace Ui {
class ControlMSSearchOption;
}

class ControlMSSearchOption : public QWidget
{
    Q_OBJECT

public:
    enum IonizationMode
    {
        None = 0,
        Positive = 1,
        Negative = 2
    };

    explicit ControlMSSearchOption(QWidget *parent = nullptr);
    ~ControlMSSearchOption();

    IonizationMode ionizationMode();
    void setIonizationMode(IonizationMode mode);

    double massTolerance();
    void setMassTolerance(double tolerance);

    bool relativeMassTolerance();
    void setRelativeMassTolerance(bool relativeTolerance);

private slots:
    void on_buttonOK_clicked();
    void on_buttonCancel_clicked();

private:
    Ui::ControlMSSearchOption *ui;
    int _ionizationMode;
    double _massTolerance;
    bool _relativeMassTolerance;
};

#endif // ControlMSSearchOption_H
