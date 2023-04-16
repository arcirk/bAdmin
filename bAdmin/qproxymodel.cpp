//
// Created by arcady on 10.11.2021.
//

#include "qproxymodel.h"
#include "qjsontablemodel.h"
#include <QJsonDocument>
#include <QJsonObject>

QProxyModel::QProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
    m_sortRole = Qt::UserRole;
}

int QProxyModel::sortRole()
{
    return m_sortRole;
}

void QProxyModel::setSortRole(int role)
{
    m_sortRole = role;
    this->setDynamicSortFilter(true);
}

bool QProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {

    auto model = (QJsonTableModel*)sourceModel();

    if (m_filter.size() > 0){
        QMapIterator<QString, QVariant> i(m_filter);
        while (i.hasNext()){
            i.next();
            int col = model->getColumnIndex(i.key());
            if(col == -1)
                continue;
            QModelIndex index = model->index(sourceRow, col, sourceParent);

            //if (i.value().typeId() == QMetaType::QString){
            if (i.value().userType() == QMetaType::QString){
                QString value = model->data(index).toString();
                QString filter = i.value().toString();
                if(filter.left(1) != "!"){
//                    if (filter != value)
//                        return false;
                    if(value.indexOf(filter) == -1)
                        return false;
                }else{
                    filter = filter.right(filter.length() - 1);
//                    if (filter == value)
//                        return false;
                    if(value.indexOf(filter) == -1)
                        return false;
                }
            }
            else if (i.value().userType() == QMetaType::Int){
                if (i.value().toInt() != model->data(index).toInt())
                    return false;
            }
            else if (i.value().userType() == QMetaType::Bool){
                if (i.value().toBool() != model->data(index).toBool())
                    return false;
            }
            else if (i.value().userType() == QMetaType::Double){
                if (i.value().toDouble() != model->data(index).toDouble())
                    return false;
            }
            else if (i.value().userType() == QMetaType::LongLong){
                if (i.value().toLongLong() != model->data(index).toLongLong())
                    return false;
            }
            //qDebug() << i.value().typeId() << ":" << i.value().typeName();
        }

    }

    return true;
}

QString QProxyModel::filter() {
    return m_FilterText;
}

void QProxyModel::setFilter(const QString &filterText) {

    m_FilterText = filterText;
    set_filter();
}

void QProxyModel::set_filter() {

    if(sourceModel()){
        beginResetModel();
    }
    m_filter.clear();
    auto doc = QJsonDocument::fromJson(m_FilterText.toUtf8());
    if (doc.isNull())
        return;

    QJsonObject obj = doc.object();

    if (obj.isEmpty())
        return;

    for (auto itr = obj.begin(); itr < obj.end(); ++itr) {
        QString key = itr.key();
        QVariant value = itr->toVariant();
        //qDebug() << key << ":" << value.typeId() << ":" << value.typeName();
        m_filter.insert(key, value);

    }
    if(sourceModel()){
        endResetModel();
    }
}
