#include <QStandardItemModel>
#include <QTreeView>
#include <QMessageBox>
#include "formmetaboliteviewer.h"
#include "ui_formmetaboliteviewer.h"
#include "global.h"
#include "hmdb/hmdbxml_def.h"
#include "utils/uconfigxml.h"
#include "widgets/framespectrumviewer.h"

#define HMDB_VIEWER_TREEVIEW_TEXT_NONAME    "(No name)"


FormMetaboliteViewer::FormMetaboliteViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormMetaboliteViewer)
{
    spectrumView = nullptr;
    ui->setupUi(this);
}

FormMetaboliteViewer::~FormMetaboliteViewer()
{
    delete ui;
}

void FormMetaboliteViewer::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

bool FormMetaboliteViewer::showMetabolite(const QString& ID)
{
    if (IDList.indexOf(ID) >= 0)
    {
        // If a metabolite has already been displayed in a tab,
        // switch to that tab directly
        ui->tabWidget->setCurrentIndex(IDList.indexOf(ID));
        return true;
    }

    QString dataPath = appConfig.mainDatabase();
    if (dataPath.isEmpty())
    {
        QMessageBox::critical(this, tr("Database not set"),
                              tr("Please configure the path to the "
                                 "main database first."));
        return false;
    }

    UconfigXML parser;
    UconfigFile dataFile;
    QString dataFilename(dataPath);
    dataFilename.append('/').append(ID).append(HMDBXML_FILENAME_SUFFIX);
    if (!parser.readUconfig(dataFilename.toLocal8Bit().constData(),
                            &dataFile,
                            true))
        return false;

    // Load the data in the XML file into the model
    QStandardItemModel* newModel = new QStandardItemModel;
    UconfigEntryObject* subentryList = dataFile.rootEntry.subentries();
    if (subentryList)
    {
        for (int i=0; i<dataFile.rootEntry.subentryCount(); i++)
        {
            // Only the <metabolite> entry will be displayed
            if (QByteArray(subentryList[i].name(), subentryList[i].nameSize())
                    == HMDBXML_ENTRY_NAME_METABOLITE)
                loadEntry(newModel, subentryList[i]);
        }
        delete[] subentryList;
    }

    // Add a tab and bind it to the model
    QTreeView* newView = new QTreeView(this);
    newView->setHeaderHidden(true);
    newView->setModel(newModel);
    ui->tabWidget->addTab(newView, ID);
    IDList.push_back(ID);
    connect(newView, SIGNAL(doubleClicked(const QModelIndex&)),
            this, SLOT(on_treeView_doubleClicked(const QModelIndex&)));

    // Switch to the new tab
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count() - 1);
    return true;
}

void FormMetaboliteViewer::loadEntry(QStandardItem* parent,
                                     const UconfigEntryObject& entry)
{
    // Load entries into node recursively
    QString nodeName;
    if (entry.nameSize() > 0)
        nodeName = QByteArray(entry.name(), entry.nameSize());
    else
        nodeName = HMDB_VIEWER_TREEVIEW_TEXT_NONAME;

    // Insert keys
    int i;
    QStandardItem* childNode = nullptr;
    UconfigKeyObject* keyList =
            const_cast<UconfigEntryObject&>(entry).keys();
    if (keyList)
    {
        if (childNode == nullptr)
        {
            childNode = new QStandardItem;
            childNode->setText(nodeName);
            parent->appendRow(childNode);
        }
        for (i=0; i<entry.keyCount(); i++)
            loadKey(childNode, keyList[i]);
        delete[] keyList;
    }

    // Insert subentries recursively
    int keyCount;
    int subentryCount;
    UconfigEntryObject* subentryList =
                        const_cast<UconfigEntryObject&>(entry).subentries();
    if (subentryList)
    {
        subentryCount = entry.subentryCount();
        for (i=0; i<subentryCount; i++)
        {
            if (subentryList[i].subentryCount() == 0 &&
                subentryList[i].nameSize() == 0)
            {
                // Deal with subentries that contains only one key
                keyCount = subentryList[i].keyCount();
                if (keyCount == 0)
                    continue;
                if (keyCount == 1)
                {
                    UconfigKeyObject* keyList =
                            const_cast<UconfigEntryObject&>(subentryList[i]).keys();
                    if (keyList)
                        loadSingleKey(parent, nodeName, keyList[0]);
                    delete[] keyList;
                    continue;
                }
            }

            if (childNode == nullptr)
            {
                childNode = new QStandardItem;
                childNode->setText(nodeName);
                parent->appendRow(childNode);
            }
            loadEntry(childNode, subentryList[i]);
        }
        delete[] subentryList;
    }
}

void FormMetaboliteViewer::loadEntry(QStandardItemModel* parent,
                                     const UconfigEntryObject& entry)
{
    QString nodeName;
    if (entry.nameSize() > 0)
        nodeName = QByteArray(entry.name(), entry.nameSize());
    else
        nodeName = HMDB_VIEWER_TREEVIEW_TEXT_NONAME;

    QStandardItem* node = new QStandardItem;
    node->setText(nodeName);
    parent->appendRow(node);

    // Insert subentries recursively
    int i;
    int subentryCount;
    UconfigEntryObject* subentryList =
                        const_cast<UconfigEntryObject&>(entry).subentries();
    if (subentryList)
    {
        subentryCount = entry.subentryCount();
        for (i=0; i<subentryCount; i++)
            loadEntry(node, subentryList[i]);
        delete[] subentryList;
    }
}

void FormMetaboliteViewer::loadKey(QStandardItem* parent,
                                   const UconfigKeyObject& key)
{
    QString nodeName;
    if (key.nameSize() > 0)
        nodeName = QByteArray(key.name(), key.nameSize());
    else
        nodeName = HMDB_VIEWER_TREEVIEW_TEXT_NONAME;

    QStandardItem* childNode = new QStandardItem;
    childNode->setText(
                nodeName.append(": ")
                        .append(QByteArray(key.value(), key.valueSize())));
    parent->appendRow(childNode);
}

void FormMetaboliteViewer::loadSingleKey(QStandardItem* parent,
                                         const QString& entryName,
                                         const UconfigKeyObject& key)
{
    parent->appendRow(makeKeyEntry(entryName, key));
}

void FormMetaboliteViewer::loadSingleKey(QStandardItemModel* parent,
                                         const QString& entryName,
                                         const UconfigKeyObject& key)
{
    parent->appendRow(makeKeyEntry(entryName, key));
}

QStandardItem* FormMetaboliteViewer::makeKeyEntry(const QString& entryName,
                                                  const UconfigKeyObject& key)
{
    QString nodeName(entryName);
    if (key.nameSize() > 0)
        nodeName.append("\n").append(QByteArray(key.name(), key.nameSize()));
    if (nodeName.isEmpty())
        nodeName = HMDB_VIEWER_TREEVIEW_TEXT_NONAME;

    QStandardItem* node = new QStandardItem;
    node->setText(
                nodeName.append(": ")
                        .append(QByteArray(key.value(), key.valueSize())));
    return node;
}

void FormMetaboliteViewer::on_tabWidget_tabCloseRequested(int index)
{
    QTreeView* view = dynamic_cast<QTreeView*>(ui->tabWidget->widget(index));
    if (!view)
        return;

    QAbstractItemModel* model = view->model();
    ui->tabWidget->removeTab(index);
    delete model;

    IDList.removeAt(index);
}

void FormMetaboliteViewer::on_treeView_doubleClicked(const QModelIndex& index)
{
    // See if it is a spectrum entry
    QString entryText = index.data().toString();
    if (entryText == HMDBXML_ENTRY_PROP_SPECTRUM)
    {
        // See if it is a MS/MS spectrum
        QModelIndex subentryIndex = index.model()->index(0, 0, index);
        if (!subentryIndex.data().toString().contains(HMDBXML_ENTRY_VALUE_MSMS))
        {
            QMessageBox::information(this, tr("Spectrum not support"),
                                     tr("Spectrum type not supported "
                                        "by the viewer."));
            return;
        }

        subentryIndex = index.model()->index(1, 0, index);
        QString spectrumID = subentryIndex.data().toString();
        spectrumID =
            spectrumID.mid(spectrumID.indexOf(HMDBXML_ENTRY_PROP_SPECTRUMID) +
                           strlen(HMDBXML_ENTRY_PROP_SPECTRUMID) + 1)
                      .trimmed();

        QString msmsDataPath = appConfig.msmsDatabase();
        if (msmsDataPath.isEmpty())
        {
            QMessageBox::critical(this, tr("MS/MS database not set"),
                                  tr("Please configure the path to the "
                                     "MS/MS database first."));
            return;
        }

        // Show the mass spectrum viewer
        if (!spectrumView)
            spectrumView = new FrameSpectrumViewer();
        spectrumView->setDatabase(msmsDataPath);
        spectrumView->clear();
        QString metaboliteID(IDList[ui->tabWidget->currentIndex()]);
        if (!spectrumView->load(metaboliteID, spectrumID))
        {
            QMessageBox::information(this, tr("Spectrum not exists"),
                                     QString(tr("The data file of the spectrum "
                                             "(ID %1) cannot be found. \n"
                                             "Please check your database."))
                                            .arg(spectrumID));
            return;
        }
        spectrumView->setTitle(metaboliteID);
        spectrumView->show();
    }
}
