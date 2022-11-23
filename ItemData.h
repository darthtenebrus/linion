//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_ITEMDATA_H
#define LINION_ITEMDATA_H


#include <QString>

class ItemData {

private:
    QString author;
    QString addonTitle;
    QString version;
    QString addonPath;
    QString description;
    bool status;

public:
    ItemData(const QString &author, const QString &addonTitle, const QString &version,
             const QString &addonPath, const QString &description, bool status);

    [[nodiscard]]
    const QString &getAddonTitle() const;

    [[nodiscard]]
    const QString &getVersion() const;

    [[nodiscard]]
    const QString &getAddonPath() const;

    [[nodiscard]]
    bool isStatus() const;

    [[nodiscard]]
    const QString &getAuthor() const;

    const QString &getDescription() const;
};


#endif //LINION_ITEMDATA_H
