//
// Created by esorochinskiy on 20.11.22.
//

#include "ItemData.h"

ItemData::ItemData(const QString &author, const QString &addonTitle, const QString &version,
                   const QString &addonPath, const QString &description, const ItemStatus &status,
                   const QString &downloadTotal, const QString &downloadMonthly,
                   const QString &favoriteTotal, const QString &fileInfoUrl, const QString &siteVersion)
        : addonTitle(addonTitle), version(version), addonPath(addonPath), status(status), author(author),
        description(description),DownloadTotal(downloadTotal), DownloadMonthly(downloadMonthly),
        FavoriteTotal(favoriteTotal), FileInfoURL(fileInfoUrl), SiteVersion(siteVersion) {}

const QString &ItemData::getAddonTitle() const {
    return addonTitle;
}

const QString &ItemData::getVersion() const {
    return version;
}

const QString &ItemData::getAddonPath() const {
    return addonPath;
}

ItemData::ItemStatus ItemData::isStatus() const {
    return status;
}

const QString &ItemData::getAuthor() const {
    return author;
}

const QString &ItemData::getDescription() const {
    return description;
}

const QString &ItemData::getDownloadTotal() const {
    return DownloadTotal;
}

const QString &ItemData::getDownloadMonthly() const {
    return DownloadMonthly;
}

const QString &ItemData::getFavoriteTotal() const {
    return FavoriteTotal;
}

const QString &ItemData::getFileInfoUrl() const {
    return FileInfoURL;
}

const QString &ItemData::getSiteVersion() const {
    return SiteVersion;
}

const QString &ItemData::getExternalPicUrl() const {
    return ExternalPicURL;
}

void ItemData::setExternalPicUrl(const QString &externalPicUrl) {
    ExternalPicURL = externalPicUrl;
}
