//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_QADDONLISTMODEL_H
#define LINION_QADDONLISTMODEL_H

#include <QDir>
#include <QModelIndex>
#include "ItemData.h"

class QAddonListModel : public QAbstractListModel {
Q_OBJECT
public:

    enum ObjectDataRole {
        VersionRole = Qt::UserRole + 1,
        PathRole,
        AuthorRole,
        DescriptionRole
    };

    explicit QAddonListModel(const QString &addonFolderPath, QObject *parent = nullptr);
    ~QAddonListModel() override;

    [[nodiscard]]
    int rowCount(const QModelIndex &) const override;

    [[nodiscard]]
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<ItemData> addonList;

    QString addonFolderPath;

    const QList<ItemData> &refreshFolderList();
    const QString &cleanColorizers(QString &input) const;

public slots:
    void refresh();
    void uninstallAddonClicked();
};


#endif //LINION_QADDONLISTMODEL_H
