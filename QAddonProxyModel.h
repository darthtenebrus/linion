//
// Created by esorochinskiy on 07.12.22.
//

#ifndef LINION_QADDONPROXYMODEL_H
#define LINION_QADDONPROXYMODEL_H


#include <QSortFilterProxyModel>

class QAddonProxyModel : public QSortFilterProxyModel {
Q_OBJECT
protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

public:
    explicit QAddonProxyModel(QObject *parent = nullptr);
};


#endif //LINION_QADDONPROXYMODEL_H
