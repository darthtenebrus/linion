//
// Created by esorochinskiy on 26.11.22.
//

#include "DialogItemDelegate.h"
#include <QPainter>
#define ITEM_WIDTH 110
#define ITEM_HEIGHT 72

void DialogItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.midlight());
    }

    QItemDelegate::paint(painter, option, index);
}

QSize DialogItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return {ITEM_WIDTH, ITEM_HEIGHT};
}

DialogItemDelegate::DialogItemDelegate(QObject *parent) : QItemDelegate(parent) {}

DialogItemDelegate::~DialogItemDelegate() = default;
