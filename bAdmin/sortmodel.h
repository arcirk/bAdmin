#ifndef SORTMODEL_H
#define SORTMODEL_H

#include <QSortFilterProxyModel>

class SortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)
public:
    explicit SortModel(QObject* parent= nullptr);


    int sortRole();
    void setSortRole(int role);

    QString filter();
    void setFilter(const QString& filterText);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const  override;



signals:
    void filterChanged();
    void sortRoleChanged();

private:
    QString m_FilterText;
    QMap<QString, QVariant> m_filter;
    int m_sortRole;

    void set_filter();
};

#endif // SORTMODEL_H
