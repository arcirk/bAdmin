#ifndef CRYPTCONTAINER_H
#define CRYPTCONTAINER_H

#include <QObject>
#include "shared_struct.hpp"
#include "commandline.h"
#include "commandlineparser.h"

class CryptContainer : public QObject
{
    Q_OBJECT
public:
    explicit CryptContainer(QObject *parent = nullptr);

    QString sid() const;

private:
    QString sid_;
    bool is_valid;

    void get_sid();

signals:

};

#endif // CRYPTCONTAINER_H
