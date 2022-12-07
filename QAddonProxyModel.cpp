//
// Created by esorochinskiy on 07.12.22.
//

#include "QAddonProxyModel.h"
#include "QAddonListModel.h"

QAddonProxyModel::QAddonProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {

}

bool QAddonProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {

    const QString &ver1 = sourceModel()->data(source_left, QAddonListModel::VersionRole).toString();
    const QString &site1 = sourceModel()->data(source_left, QAddonListModel::SiteVersionRole).toString();

    const QString &ver2 = sourceModel()->data(source_right, QAddonListModel::VersionRole).toString();
    const QString &site2 = sourceModel()->data(source_right, QAddonListModel::SiteVersionRole).toString();


    bool preCondition = ((ver1 != site1) && (ver2 == site2));
    bool postCondition = ((ver1 == site1) && (ver2 != site2));

    if (preCondition) {
        return preCondition;
    }

    if (postCondition) {
        return false;
    }

    const QString &title_left = sourceModel()->data(source_left, Qt::DisplayRole).toString();
    const QString &title_right = sourceModel()->data(source_right, Qt::DisplayRole).toString();

    return title_left < title_right;

}
