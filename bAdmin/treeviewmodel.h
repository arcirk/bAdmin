#ifndef TREEVIEWMODEL_H
#define TREEVIEWMODEL_H

#include <QAbstractItemModel>
#include <QObject>
#include "shared_struct.hpp"
#include <QIcon>
#include <QVector>

enum item_icons_enum{
    ItemGroup = 0,
    Item,
    DeletedItemGroup,
    DeletedItem
};

class TreeViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeViewModel(const arcirk::client::client_conf& conf, QObject *parent = nullptr);
    ~TreeViewModel();
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent) const override;

    void fetchRoot(const QString& table_name, const QString& root_dir = "");
    QVariant firstData(const nlohmann::json& node, int role, const QModelIndex& index) const;
    bool is_loaded();
    void set_column_aliases(const QMap<QString, QString> values);
    int get_column_index(const QString& name);
    QString get_column_name(int column) const;
    void set_columns(const QVector<QString> cols);
    void set_server_object(arcirk::server::server_objects obj);
    arcirk::server::server_objects server_object() const;
    nlohmann::json get_object(const QModelIndex &index) const;
    nlohmann::json get_objects(const QModelIndex &parent) const;
    void set_rows_icon(const QIcon &ico);
    void set_icon(const QModelIndex& index, const QIcon &ico);

    void clear();
    void reset();

    void set_table(const nlohmann::json& tableModel);

    QString current_parent_path() const;

    void refresh(const QModelIndex& parent);

    void remove(const QModelIndex &parent);
    void add(const nlohmann::json object, const QModelIndex &parent);
    void set_object(const QModelIndex &index, const nlohmann::json& object);

private:
    arcirk::server::server_objects server_object_;
    QVector<QString> columns;
    struct NodeInfo;
    typedef QVector<NodeInfo> NodeInfoList;
    NodeInfoList _nodes;
    arcirk::client::client_conf conf_;
    QString table_name_;
    QByteArray authString_;
    QMap<QString, QString> column_aliases;
    bool is_loaded_;

    QString current_parent_path_;

    void set_current_parent_path(const QString& value);

    QMap<item_icons_enum, QIcon> item_icons;
    QIcon rows_icon;
    QMap<QPair<int,int>, QIcon> row_icons;

    int findRow(const NodeInfo* nodeInfo) const;
    QVariant nameData(const nlohmann::json& rowData, int role) const;
    QString cryptPass(const QString &source, const QString &key);
    nlohmann::json getNodeData(const QString& parentUuid) const;

    QVariant get_value(const nlohmann::json &node, int col = 0) const;
    nlohmann::json http_data(const QString &parentUuid) const;
    bool field_is_exists(const nlohmann::json &object, const std::string &name) const;

};

#endif // TREEVIEWMODEL_H
