//
// Created by arcady on 10.11.2021.
//

#ifndef WS_SOLUTION_QJSONTABLEMODEL_H
#define WS_SOLUTION_QJSONTABLEMODEL_H
#include <QHash>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QIcon>
#include <QList>

class QJsonTableModel : public QAbstractTableModel{

    Q_OBJECT
    Q_PROPERTY(QString jsonText READ jsonText WRITE setJsonText NOTIFY jsonTextChanged)
    Q_PROPERTY(int currentRow READ currentRow WRITE setCurrentRow NOTIFY currentRowChanged)

public:
    typedef QMap<QString,QString> Heading;
    typedef QVector<Heading> Header;

    explicit QJsonTableModel(QObject * parent = nullptr);

    bool setJson( const QJsonDocument& json );

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int getColumnIndex(const QString& name);
    Q_INVOKABLE QString value(const QModelIndex &index, int role = Qt::DisplayRole);

    QString jsonText() const;
    void setJsonText(const QString& source);
    void setColumnAliases(const QMap<QString, QString>& columnAliases);

    Q_INVOKABLE void reset();

    void setRowsIcon(const QIcon& ico);
    void setDeletionMarkRowsIcon(const QIcon& ico);
    void setIcon(const QModelIndex& index, const QIcon& ico);
    void setRowKey(int row, const QPair<QString, QString>& key);
    QPair<QString, QString> rowKey(int index) const;
    void setFormatColumn(int column, const QString& fmt);

    void removeRow(int row);
    void addRow(const QJsonObject& row);
    void addRow(const QString& rowJson);
    void insertRow(int pos, const QString& rowJson);
    Q_INVOKABLE void addRow(const QString& barcode, const QString& parent, int quantity);
    Q_INVOKABLE void moveTop(int row);

    int row(const QPair<QString, QString>& key);
    QJsonObject getRowObject(int row);
    Q_INVOKABLE QString getObjectToString(int row);
    void updateRow(const QJsonObject& obj, int index);
    Q_INVOKABLE void updateRow(const QString& barcode, const int quantity, int index);

    void moveUp(int row);
    void moveDown(int row);

    Q_INVOKABLE QModelIndex findInTable(const QString &value, int column, bool findData);

    Q_INVOKABLE int max(const QString& field);

    Q_INVOKABLE QModelIndex emptyIndex(){
        return QModelIndex();
    };

    int currentRow();
    void setCurrentRow(int row);

    Q_INVOKABLE void clear();

signals:
    void jsonTextChanged();
    void currentRowChanged();
private:
    bool setJson( const QJsonArray& array );
    static QString fromBase64(const QString& str);
    virtual QJsonObject getJsonObject( const QModelIndex &index ) const;

    QJsonObject getEmptyRow();

    Header m_header{};
    QJsonArray m_json;
    QJsonArray _header;
    QMap<QString, QString> m_colAliases;
    QIcon _rowsIcon;
    QIcon _deletionMarkRowsIcon;

    QMap<QPair<int,int>, QIcon> m_rowIcon;
    QVector<QPair<QString, QString>> m_rowKeys;
    QMap<int, QString> m_fmtText;

    int m_currentRow;

};
#endif //WS_SOLUTION_QJSONTABLEMODEL_H
