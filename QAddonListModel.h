//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_QADDONLISTMODEL_H
#define LINION_QADDONLISTMODEL_H

#include <QDir>
#include <QModelIndex>
#include "ItemData.h"

class QAddonListModel : public QAbstractListModel {

public:

    enum ObjectDataRole {
        VersionRole = Qt::UserRole + 1,
        PathRole
    };

    explicit QAddonListModel(QString &addonFolderPath, QObject *parent=nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

    const QList<ItemData> & fillFolderList();

private:
    QList<ItemData> addonList;

    QString addonFolderPath;

    const QString &cleanColorizers(QString &input) const;
};


#endif //LINION_QADDONLISTMODEL_H
