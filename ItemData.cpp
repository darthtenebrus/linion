//
// Created by esorochinskiy on 20.11.22.
//

#include "ItemData.h"

ItemData::ItemData(const QString &addonTitle, const QString &version, const QString &addonPath, bool status)
        : addonTitle(
        addonTitle), version(version), addonPath(addonPath), status(status) {}

const QString &ItemData::getAddonTitle() const {
    return addonTitle;
}

const QString &ItemData::getVersion() const {
    return version;
}

const QString &ItemData::getAddonPath() const {
    return addonPath;
}

bool ItemData::isStatus() const {
    return status;
}
