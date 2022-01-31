#ifndef HMDBQTCONFIG_H
#define HMDBQTCONFIG_H

#include <QSettings>


class HmdbQtConfig
{
public:
    QString mainDatabase();
    void setMainDatabase(QString path);

    QString msmsDatabase();
    void setMSMSDatabase(QString path);

protected:
    QSettings config{"HMDB-qt", "Desktop"};
};

#endif // HMDBQTCONFIG_H
