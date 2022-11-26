//
// Created by esorochinskiy on 26.11.22.
//

#ifndef LINION_DIALOGITEMDELEGATE_H
#define LINION_DIALOGITEMDELEGATE_H


#include <QItemDelegate>

class DialogItemDelegate : public QItemDelegate {

public:
    explicit DialogItemDelegate(QObject *parent);

    ~DialogItemDelegate() override;

public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

};


#endif //LINION_DIALOGITEMDELEGATE_H
