#ifndef TABLEDELEGATE_H
#define TABLEDELEGATE_H
#include <QStyledItemDelegate>
#include <QObject>

class TableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TableDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:

};
#endif // TABLEDELEGATE_H
