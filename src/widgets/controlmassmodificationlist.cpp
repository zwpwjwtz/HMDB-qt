#include <QMessageBox>
#include <QMenu>
#include <QFileDialog>
#include "controlmassmodificationlist.h"
#include "ui_controlmassmodificationlist.h"

#define HMDB_MASSMOD_FIELD_MZ           0
#define HMDB_MASSMOD_FIELD_FORMULA      1
#define HMDB_MASSMOD_FIELD_NAME         2
#define HMDB_MASSMOD_FIELD_LEVEL        3

#define HMDB_MASSMOD_MENU_INDEX_NULL    0
#define HMDB_MASSMOD_MENU_INDEX_POS     1
#define HMDB_MASSMOD_MENU_INDEX_NEG     2
#define HMDB_MASSMOD_MENU_INDEX_CUSTOM  3

#define HMDB_MASSMOD_LIST_POSITIVE    ":/resource/MassModification-positive.csv"
#define HMDB_MASSMOD_LIST_NEGATIVE    ":/resource/MassModification-negative.csv"


ControlMassModificationList::ControlMassModificationList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlMassModificationList)
{
    ui->setupUi(this);

    loadList(true, HMDB_MASSMOD_LIST_POSITIVE);
    ui->viewMassModification->setModel(&model);

    menuLoadMassModification = nullptr;
}

ControlMassModificationList::~ControlMassModificationList()
{
    delete menuLoadMassModification;
    delete ui;
}

bool ControlMassModificationList::loadList(bool reset, QString filename)
{
    if (reset)
    {
        model.clear();
        model.setColumnCount(4);
        model.setHorizontalHeaderItem(0, new QStandardItem("Enabled"));
        model.setHorizontalHeaderItem(1, new QStandardItem("Mass"));
        model.setHorizontalHeaderItem(2, new QStandardItem("Formula"));
        model.setHorizontalHeaderItem(3, new QStandardItem("Description"));
        ui->viewMassModification->horizontalHeader()
                            ->setSectionResizeMode(QHeaderView::Interactive);
        ui->viewMassModification->horizontalHeader()
                                ->setStretchLastSection(true);
    }

    QFile resource(filename);
    if (!resource.open(QFile::ReadOnly))
        return false;

    QByteArray buffer;
    QList<QByteArray> properties;
    QList<QStandardItem*> row;
    resource.readLine();  // Skip the header line
    while(!resource.atEnd())
    {
        buffer = resource.readLine();
        buffer.truncate(buffer.lastIndexOf('\n'));
        properties = buffer.split(',');
        if (properties.length() == 0)
            continue;

        // Insert this field
        row << new QStandardItem()
          << new QStandardItem(QString(properties[HMDB_MASSMOD_FIELD_MZ]))
          << new QStandardItem(QString(properties[HMDB_MASSMOD_FIELD_FORMULA]))
          << new QStandardItem(QString(properties[HMDB_MASSMOD_FIELD_NAME]));
        row[0]->setCheckable(true);
        if (properties[HMDB_MASSMOD_FIELD_LEVEL] == "0")
            row[0]->setCheckState(Qt::Checked);
        model.appendRow(row);
        row.clear();
    }
    return true;
}

QList<int> ControlMassModificationList::selectedIndexes()
{
    QList<int> indexList;
    for (int i=0; i<model.rowCount(); i++)
    {
        if (model.item(i, 0)->checkState() == Qt::Checked)
            indexList.push_back(i);
    }
    return indexList;
}

QString ControlMassModificationList::mz(int index)
{
    return model.item(index, 1)->text();
}

QString ControlMassModificationList::formula(int index)
{
    return model.item(index, 2)->text();
}

QString ControlMassModificationList::name(int index)
{
    return model.item(index, 3)->text();
}

void ControlMassModificationList::onMenuLoadActionClicked(QAction* action)
{
    QString newFilename;

    switch (action->data().toInt())
    {
        case HMDB_MASSMOD_MENU_INDEX_POS:
            newFilename = HMDB_MASSMOD_LIST_POSITIVE;
            break;
        case HMDB_MASSMOD_MENU_INDEX_NEG:
            newFilename = HMDB_MASSMOD_LIST_NEGATIVE;
            break;
        case HMDB_MASSMOD_MENU_INDEX_CUSTOM:
            newFilename = QFileDialog::getOpenFileName(this,
                                 "Select a customized mass modificatin list",
                                 massModificationListDir);
            if (newFilename.isEmpty())
                return;
            break;
        default:;
    }
    if (QMessageBox::question(this, "Clear list content",
                              "Clear current list before loading the new one?")
            == QMessageBox::Yes)
        loadList(true, newFilename);
    else
        loadList(false, newFilename);

}


void ControlMassModificationList::on_buttonAdd_clicked()
{
    QList<QStandardItem*> newRow;
    newRow << new QStandardItem()
           << new QStandardItem("0")
           << new QStandardItem()
           << new QStandardItem("");
    if (model.rowCount() > 0)
        model.insertRow(ui->viewMassModification->currentIndex().row(), newRow);
    else
        model.appendRow(newRow);
}

void ControlMassModificationList::on_buttonRemove_clicked()
{
    QModelIndex index = ui->viewMassModification->currentIndex();
    if (index.isValid())
        model.removeRow(index.row());
}

void ControlMassModificationList::on_buttonLoad_clicked()
{
    if (!menuLoadMassModification)
    {
        menuLoadMassModification = new QMenu(this);

        QAction* newAction = new QAction("Empty list");
        newAction->setData(HMDB_MASSMOD_MENU_INDEX_NULL);
        menuLoadMassModification->addAction(newAction);

        menuLoadMassModification->addSeparator();

        newAction = new QAction("Default (Positive)");
        newAction->setData(HMDB_MASSMOD_MENU_INDEX_POS);
        menuLoadMassModification->addAction(newAction);

        newAction = new QAction("Default (Negative)");
        newAction->setData(HMDB_MASSMOD_MENU_INDEX_NEG);
        menuLoadMassModification->addAction(newAction);

        menuLoadMassModification->addSeparator();

        newAction = new QAction("Customized");
        newAction->setData(HMDB_MASSMOD_MENU_INDEX_CUSTOM);
        menuLoadMassModification->addAction(newAction);

        connect(menuLoadMassModification, SIGNAL(triggered(QAction*)),
                this, SLOT(onMenuLoadActionClicked(QAction*)));
    }
    menuLoadMassModification->popup(cursor().pos());
}

void ControlMassModificationList::on_buttonSaveAs_clicked()
{
    QString newFilename =
            QFileDialog::getSaveFileName(this, "Save mass modification list as",
                                         massModificationListDir);
    if (!newFilename.isEmpty())
    {
        QFile newFile(newFilename);
        if (!newFile.open(QFile::WriteOnly))
            return;

        QByteArray buffer;
        newFile.write("m/z,Formula,Name,Level\n");
        for (int i=0; i<model.rowCount(); i++)
        {
            buffer.append(model.item(i, 1)->text().toLocal8Bit()).append(',')
                  .append(model.item(i, 2)->text().toLocal8Bit()).append(',')
                  .append(model.item(i, 3)->text().toLocal8Bit()).append(',');
            if (model.item(i, 0)->checkState() == Qt::Checked)
                buffer.append('0');
            else
                buffer.append('1');
            newFile.write(buffer.append('\n'));
            buffer.clear();
        }
        newFile.close();
        massModificationListDir = newFilename;
    }
}
