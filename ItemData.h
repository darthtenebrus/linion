//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_ITEMDATA_H
#define LINION_ITEMDATA_H

#include <QString>

class ItemData {

public:
    enum ItemStatus {
        Installed = 1,
        InstalledBackedUp,
        NotInstalled
    };

private:
    QString author;
    QString addonTitle;
    QString version;
    QString addonPath;
    QString description;
    ItemStatus status;
    QString DownloadTotal;
    QString DownloadMonthly;
    QString FavoriteTotal;
    QString FileInfoURL;
    QString SiteVersion;
    QString ExternalPicURL;


public:

    ItemData(const QString &author, const QString &addonTitle, const QString &version,
             const QString &addonPath, const QString &description, const ItemStatus &status,
             const QString &downloadTotal, const QString &downloadMonthly, const QString &favoriteTotal,
             const QString &fileInfoUrl, const QString &siteVersion);

    [[nodiscard]]
    const QString &getAddonTitle() const;

    [[nodiscard]]
    const QString &getVersion() const;

    [[nodiscard]]
    const QString &getAddonPath() const;

    [[nodiscard]]
    ItemStatus isStatus() const;

    [[nodiscard]]
    const QString &getAuthor() const;

    [[nodiscard]]
    const QString &getDescription() const;

    [[nodiscard]]
    const QString &getDownloadTotal() const;

    [[nodiscard]]
    const QString &getDownloadMonthly() const;

    [[nodiscard]]
    const QString &getFavoriteTotal() const;

    [[nodiscard]]
    const QString &getFileInfoUrl() const;

    [[nodiscard]]
    const QString &getSiteVersion() const;

    const QString &getExternalPicUrl() const;

    void setExternalPicUrl(const QString &externalPicUrl);
};


#endif //LINION_ITEMDATA_H
