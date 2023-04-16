//
// Created by arcady on 10.11.2021.
//

#ifndef QJSONMODEL_QPROXYMODEL_H
#define QJSONMODEL_QPROXYMODEL_H
#include <QSortFilterProxyModel>

class QProxyModel : public QSortFilterProxyModel{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    //Q_PROPERTY(QAbstractTableModel* sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)

    Q_PROPERTY(int sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)

public:
    explicit QProxyModel(QObject* parent= nullptr);

    int sortRole();
    void setSortRole(int role);

    QString filter();
    Q_INVOKABLE void setFilter(const QString& filterText);

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


#endif //QJSONMODEL_QPROXYMODEL_H
