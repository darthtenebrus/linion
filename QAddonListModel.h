//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_QADDONLISTMODEL_H
#define LINION_QADDONLISTMODEL_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QModelIndex>
#include <QFileSystemWatcher>
#include <QTreeView>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "ItemData.h"
#include "preferences.h"

class QAddonListModel : public QAbstractListModel {
Q_OBJECT
public:

    enum ObjectDataRole {
        VersionRole = Qt::UserRole + 1,
        PathRole,
        AuthorRole,
        DescriptionRole,
        DownloadTotalRole,
        DownloadMonthlyRole,
        FavoriteTotalRole,
        FileInfoURLRole
    };

    explicit QAddonListModel(const PreferencesType &settings, QObject *parent);
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
    void setModelData(const PreferencesType &hash);

private:

    static QString listUrl;

    QNetworkAccessManager *manager;
    QList<ItemData> addonList;
    QList<QJsonObject> esoSiteList;

    QString addonFolderPath;
    QString backupPath;
    QFileSystemWatcher *qsw;

    bool useTar;
    bool useZip;
    QString tarCommand;
    QString zipCommand;
    QString zipExtractCommand;

    void refreshFolderList();
    void refreshESOSiteList();
    void setTopIndex();
    const QString &cleanColorizers(QString &input) const;
    void processBackup(const QString &pPath) const;
    void copyPath(const QString&, const QString&) const;

    [[nodiscard]]
    ItemData::ItemStatus checkBackupStatus(const QString &qString) const;


signals:
    void percent(int current, int total, const QString &msg);

public slots:
    void refresh();
    void uninstallAddonClicked();
    void backupAddonClicked();
    void reinstallAddonClicked();
    void backupAllClicked();

private slots:
    void replyFinished(QNetworkReply *replyFinished);
};


#endif //LINION_QADDONLISTMODEL_H
