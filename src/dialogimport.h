#ifndef DIALOGIMPORT_H
#define DIALOGIMPORT_H

#include <QWizard>


class HmdbXMLImporterWorker;

namespace Ui {
class DialogImport;
}

class DialogImport : public QWizard
{
    Q_OBJECT

public:
    explicit DialogImport(QWidget *parent = nullptr);
    ~DialogImport();

signals:
    void DirectoryChanged(QString newDir);

protected:
    virtual void showEvent(QShowEvent* event);

private slots:
    virtual void done(int r);

    void onImporterProgressed(double finishedPercent);
    void onImporterFinished(bool successful);

    void on_buttonChooseFile_clicked();
    void on_buttonChooseDir_clicked();
    void on_DialogImport_currentIdChanged(int id);
    void on_pushButton_clicked();
    void on_textImportFilename_textChanged(const QString &arg1);
    void on_textImportDirectory_textChanged(const QString &arg1);

private:
    Ui::DialogImport *ui;
    HmdbXMLImporterWorker* importer;
    bool initialized;

    bool startImport();
    void stop();
};

#endif // DIALOGIMPORT_H
