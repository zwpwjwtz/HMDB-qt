#ifndef FORMMETABOLITEVIEWER_H
#define FORMMETABOLITEVIEWER_H

#include <QWidget>


class QStandardItem;
class QStandardItemModel;
class UconfigKeyObject;
class UconfigEntryObject;

namespace Ui {
class FormMetaboliteViewer;
}

class FormMetaboliteViewer : public QWidget
{
    Q_OBJECT

public:
    explicit FormMetaboliteViewer(QWidget *parent = nullptr);
    ~FormMetaboliteViewer();

    bool setDatabase(const QString& path);
    bool showMetabolite(const QString& ID);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::FormMetaboliteViewer *ui;
    QString dataPath;
    QList<QString> IDList;

    void loadEntry(QStandardItem* parent, const UconfigEntryObject& entry);
    void loadEntry(QStandardItemModel* parent, const UconfigEntryObject& entry);
    void loadKey(QStandardItem* parent, const UconfigKeyObject& key);
    void loadSingleKey(QStandardItem* parent,
                       const QString& entryName,
                       const UconfigKeyObject& key);
    void loadSingleKey(QStandardItemModel* parent,
                       const QString& entryName,
                       const UconfigKeyObject& key);
    QStandardItem* makeKeyEntry(const QString& entryName,
                                const UconfigKeyObject& key);

private slots:
    void on_tabWidget_tabCloseRequested(int index);

};

#endif // FORMMETABOLITEVIEWER_H
