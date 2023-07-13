#ifndef TABLEVIEWDELEGATE_H
#define TABLEVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>

class TableViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TableViewDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:

};

#endif // TABLEVIEWDELEGATE_H
