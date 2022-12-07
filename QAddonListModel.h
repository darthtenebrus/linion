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
        FileInfoURLRole,
        SiteVersionRole,
        StatusRole
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

    void setModelData(const PreferencesType &hash);
    void setHeaderTitle(const QString &);
    void disconnectWatcher();
    void connectWatcher();
    void refreshFromExternal();

private:
    static QString listUrl;
    QString headerTitle;
    QNetworkAccessManager *manager;
    QNetworkReply *m_currentReply {nullptr};
    QByteArray m_buffer;
    QList<ItemData> addonList;
    QList<QJsonObject> esoSiteList;

    QString addonFolderPath;
    QString backupPath;
    QFileSystemWatcher *qsw {nullptr};

    bool useTar;
    bool useZip;
    QString tarCommand;
    QString zipCommand;
    QString zipExtractCommand;

    void refreshFolderList();
    void refreshESOSiteList();
    void refreshFromSiteList();
    void setTopIndex();
    const QString &cleanColorizers(QString &input) const;
    void processBackup(const QString &pPath) const;
    void copyPath(const QString&, const QString&) const;
    void prepareAndCleanDestDir(const QDir &dir) const;
    ItemData * prepareAndFillDataByAddonName(const QString &addonName) const;

    [[nodiscard]]
    ItemData::ItemStatus checkBackupStatus(const QString &qString) const;


signals:
    void percent(int current, int total, const QString &msg = "");
    void backToInstalled(bool);
    void addonsListChanged();


public slots:
    void refresh();
    void uninstallAddonClicked();
    void backupAddonClicked();
    void reinstallAddonClicked();
    void backupAllClicked();

    int columnCount(const QModelIndex &parent) const override;

private slots:
    void replyFinished(QNetworkReply *replyFinished);
    void onPercentDownload(qint64, qint64);
};


#endif //LINION_QADDONLISTMODEL_H
