#include "tabledelegate.h"
#include <QApplication>
#include <QPainter>

void TableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//  if(index.column() == 3) {
//       // TODO
//  } else {
//      QStyledItemDelegate::paint(painter, option, index);
//  }
  Q_ASSERT(index.isValid());

  QStyleOptionViewItem opt = option;
  initStyleOption(&opt, index);
  // disable default icon
  opt.icon = QIcon();
  // draw default item
  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter, 0);

  const QRect r = option.rect;

  // get pixmap
  QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
  QPixmap pix = icon.pixmap(r.size());

  // draw pixmap at center of item
  const QPoint p = QPoint((r.width() - pix.width())/2, (r.height() - pix.height())/2);
  painter->drawPixmap(r.topLeft() + p, pix);
}

QSize TableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
   return QStyledItemDelegate::sizeHint(option, index);
}

void TableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QStyledItemDelegate::setModelData(editor, model, index);
}
