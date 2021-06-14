#include <QStandardItemModel>
#include <QKeyEvent>
#include "controlqueryfield.h"
#include "ui_controlqueryfield.h"
#include "../hmdb/hmdbxml_def.h"

#define OPENOTE_REFSELECTOR_ITEM_ROLE_ID    Qt::UserRole + 2


ControlQueryField::ControlQueryField(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlQueryField)
{
    ui->setupUi(this);

    modelFieldList = new QStandardItemModel(this);
    modelFieldList->setColumnCount(2);
    loadDefaultList();

    ui->viewFieldList->setModel(modelFieldList);
    ui->viewFieldList->horizontalHeader()
                    ->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->viewFieldList->horizontalHeader()->setStretchLastSection(true);
}

ControlQueryField::~ControlQueryField()
{
    delete ui;
}

void ControlQueryField::changeEvent(QEvent* e)
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

void ControlQueryField::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
        on_buttonCancel_clicked();
    else
        this->QWidget::keyPressEvent(event);
}

void ControlQueryField::showEvent(QShowEvent* event)
{
    if (selectedFieldIDList.isEmpty())
        loadDefaultList();
    else
    {
        for (int i=0; i<modelFieldList->rowCount(); i++)
        {
            if (selectedFieldIDList.contains(
                    modelFieldList->item(i, 1)
                        ->data(OPENOTE_REFSELECTOR_ITEM_ROLE_ID).toString()))
                modelFieldList->item(i, 0)->setCheckState(Qt::Checked);
            else
                modelFieldList->item(i, 0)->setCheckState(Qt::Unchecked);
        }
    }
}

void ControlQueryField::addItem(const QString &ID, const QString &text,
                                bool enabled)
{
    QStandardItem* checkbox = new QStandardItem();
    checkbox->setCheckable(true);
    checkbox->setEditable(false);
    checkbox->setSelectable(false);
    checkbox->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    QStandardItem* item = new QStandardItem(text);
    item->setData(text, Qt::DisplayRole);
    item->setData(ID, OPENOTE_REFSELECTOR_ITEM_ROLE_ID);
    item->setCheckable(false);
    item->setEditable(false);
    modelFieldList->appendRow(QList<QStandardItem*>() << checkbox << item);

    if (enabled)
    {
        selectedFieldIDList.push_back(ID);
        selectedFieldNameList.push_back(text);
    }
}

void ControlQueryField::loadDefaultList()
{
    addItem(HMDBXML_ENTRY_PROP_ID, "ID", true);
    addItem(HMDBXML_ENTRY_PROP_NAME, "Name", true);
    addItem(HMDBXML_ENTRY_PROP_FORMULAR, "Formula", true);
    addItem(HMDBXML_ENTRY_PROP_AVGMASS, "Mass", true);
    addItem(HMDBXML_ENTRY_PROP_MONOMASS, "Mono. Mass", true);
    addItem(HMDBXML_ENTRY_PROP_STATUS, "Status", true);
    addItem(HMDBXML_ENTRY_PROP_SMILES, "SMILES");
    addItem(HMDBXML_ENTRY_PROP_INCHI, "InChI");
    addItem(HMDBXML_ENTRY_PROP_INCHIKEY, "InChI Key");
    addItem(HMDBXML_ENTRY_PROP_STATE, "State");
}

void ControlQueryField::on_buttonOK_clicked()
{
    selectedFieldIDList.clear();
    selectedFieldNameList.clear();

    for (int i=0; i<modelFieldList->rowCount(); i++)
    {
        if (modelFieldList->item(i, 0)->checkState() == Qt::Checked)
        {
            selectedFieldIDList.push_back(
                    modelFieldList->item(i, 1)
                        ->data(OPENOTE_REFSELECTOR_ITEM_ROLE_ID).toString());
            selectedFieldNameList.push_back(
                    modelFieldList->item(i, 1)
                        ->data(Qt::DisplayRole).toString());
        }
    }

    // Force selecting the field "ID" (the main key)
    if (!selectedFieldIDList.contains(HMDBXML_ENTRY_PROP_ID))
    {
        selectedFieldIDList.push_front(HMDBXML_ENTRY_PROP_ID);
        selectedFieldNameList.push_front("ID");
    }

    close();
}

void ControlQueryField::on_buttonCancel_clicked()
{
    close();
}
