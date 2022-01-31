#include "hmdbqtconfig.h"
#define HMDB_CONFIG_DATABASE_PATH_MAIN  "database-main-path"
#define HMDB_CONFIG_DATABASE_PATH_MSMS  "database-msms-path"


QString HmdbQtConfig::mainDatabase()
{
    return config.value(HMDB_CONFIG_DATABASE_PATH_MAIN).toString();
}

void HmdbQtConfig::setMainDatabase(QString path)
{
    config.setValue(HMDB_CONFIG_DATABASE_PATH_MAIN, path);
}

QString HmdbQtConfig::msmsDatabase()
{
    return config.value(HMDB_CONFIG_DATABASE_PATH_MSMS).toString();
}

void HmdbQtConfig::setMSMSDatabase(QString path)
{
    config.setValue(HMDB_CONFIG_DATABASE_PATH_MSMS, path);
}

