#ifndef DIALOGIMPORT_H
#define DIALOGIMPORT_H

#include <QWizard>
#include "hmdb/hmdbxmlimporter.h"


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
    void on_buttonChooseFile_clicked();
    void on_buttonChooseDir_clicked();
    void on_DialogImport_currentIdChanged(int id);
    void on_pushButton_clicked();
    void on_textImportFilename_textChanged(const QString &arg1);
    void on_textImportDirectory_textChanged(const QString &arg1);

private:
    Ui::DialogImport *ui;
    bool initialized;

    QString importXML;
    QString importDir;
    HmdbXMLImporter importer;

    void startImport();
};

#endif // DIALOGIMPORT_H
