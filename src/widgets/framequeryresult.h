#ifndef FRAMEQUERYRESULT_H
#define FRAMEQUERYRESULT_H

#include <QFrame>


class HmdbQueryRecord;
class QStandardItemModel;
class QSortFilterProxyModel;

namespace Ui {
class FrameQueryResult;
}

class FrameQueryResult : public QFrame
{
    Q_OBJECT

public:
    explicit FrameQueryResult(QWidget *parent = nullptr);
    ~FrameQueryResult();

    void loadResult(const HmdbQueryRecord& record, bool showRank);

protected:
    void changeEvent(QEvent *e);

signals:
    void fieldListRequested();
    void showDetailsRequested(QString ID);

private:
    Ui::FrameQueryResult *ui;
    QStandardItemModel* modelResult;
    QSortFilterProxyModel* modelResultProxy;

    bool resultLoaded;
    QString lastSavePath;
    QString lastSavePathFilter;
    QList<int> listColumnWidth;
    QList<bool> columnSortAscending;

    void saveColumnWidth();
    void restoreColumnWidth();

private slots:
    void onViewHeaderSearchResultClicked(int columnIndex);

    void on_buttonOpenLink_clicked();
    void on_buttonSave_clicked();
    void on_buttonItemList_clicked();
    void on_viewSearchResult_doubleClicked(const QModelIndex& index);
    void on_buttonView_clicked();
};

#endif // FRAMEQUERYRESULT_H
