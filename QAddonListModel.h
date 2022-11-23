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

    explicit QAddonListModel(const QString &addonFolderPath, const QString &backupPath, QObject *parent = nullptr);
    ~QAddonListModel() override;

    [[nodiscard]]
    int rowCount(const QModelIndex &) const override;

    [[nodiscard]]
    QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<ItemData> addonList;

    QString addonFolderPath;
    QString backupPath;


    void refreshFolderList();
    const QString &cleanColorizers(QString &input) const;
    void processBackup(const QModelIndex &index);
    void copyPath(const QString& src, const QString& dst);

public slots:
    void refresh();
    void uninstallAddonClicked();
    void backupAddonClicked();



};


#endif //LINION_QADDONLISTMODEL_H
