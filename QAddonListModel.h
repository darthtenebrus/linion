//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_QADDONLISTMODEL_H
#define LINION_QADDONLISTMODEL_H

#include <QDir>
#include <QModelIndex>
#include <QFileSystemWatcher>
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

    [[nodiscard]]
    const QList<ItemData> &getAddonList() const;

    [[nodiscard]]
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void sort(int column, Qt::SortOrder order) override;

private:
    QList<ItemData> addonList;

    QString addonFolderPath;
    QString backupPath;
    QFileSystemWatcher *qsw;


    void refreshFolderList();
    const QString &cleanColorizers(QString &input) const;
    void processBackup(const QString &pPath) const;
    void copyPath(const QString&, const QString&) const;

signals:
    void percent(int current, int total, const QString &msg);

public slots:
    void refresh();
    void uninstallAddonClicked();
    void backupAddonClicked();
    void backupAllClicked();



};


#endif //LINION_QADDONLISTMODEL_H
