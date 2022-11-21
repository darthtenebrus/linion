//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_ITEMDATA_H
#define LINION_ITEMDATA_H


#include <QString>

class ItemData {

private:
    QString addonTitle;
    QString version;
    QString addonPath;
    bool status;

public:
    ItemData(const QString &addonTitle, const QString &version, const QString &addonPath, bool status);

    [[nodiscard]] const QString &getAddonTitle() const;
    [[nodiscard]] const QString &getVersion() const;
    [[nodiscard]] const QString &getAddonPath() const;

    bool isStatus() const;
};


#endif //LINION_ITEMDATA_H
