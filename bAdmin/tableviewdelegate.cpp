#include "tableviewdelegate.h"
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QBrush>

void TableViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QStyle* style = QApplication::style();
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt);
    if(index.column() == 0 && index.row() == 10){
        qDebug() << index.column();
    }
    QColor itemForegroundColor = index.data(Qt::ForegroundRole).value<QColor>();

    if (opt.state & QStyle::State_Selected &&
        opt.state & QStyle::State_Active){
        if (itemForegroundColor.isValid())
        {
            painter->setPen(itemForegroundColor);
        }else
            painter->setPen(opt.palette.color(QPalette::HighlightedText));
        painter->setBrush(opt.palette.highlight());
    }else{
        if (itemForegroundColor.isValid())
        {
            painter->setPen(itemForegroundColor);
        }else
            painter->setPen(opt.palette.color(QPalette::Text));
        painter->setBrush(QBrush(Qt::white));// opt.palette.window());
    }

    //style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget); // slightly changed


    painter->fillRect(opt.rect, painter->brush());
    painter->drawText(textRect, opt.displayAlignment, index.data().toString());
    //QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, 0);
    const QRect r = option.rect;

//    // get pixmap
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QPixmap pix = icon.pixmap(r.size());

//    // draw pixmap at center of item
    const QPoint p = QPoint(0, (r.height() - pix.height())/2);
    painter->drawPixmap(r.topLeft() + p, pix);
}

QSize TableViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
   return QStyledItemDelegate::sizeHint(option, index);
}

void TableViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QStyledItemDelegate::setModelData(editor, model, index);
}
