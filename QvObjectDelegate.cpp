//
// Created by esorochinskiy on 20.11.22.
//

#include "QvObjectDelegate.h"
#include "QAddonListModel.h"
#include <QPainter>

#define ITEM_HEIGHT 64
#define ICON_LEFT_OFFSET 16
#define TEXT_TOP_OFFSET 6
#define TEXT_RIGHT_OFFSET 4

QvObjectDelegate::QvObjectDelegate(QObject *parent) : QItemDelegate(parent) {}

QvObjectDelegate::~QvObjectDelegate() {

}

QSize QvObjectDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(-1, ITEM_HEIGHT);
}

void QvObjectDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const {
    QItemDelegate::drawFocus(painter, option, option.rect);
}

void QvObjectDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    painter->save();
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, option.palette.midlight());
    }

    painter->setClipRect(option.rect);
    painter->translate(option.rect.topLeft());
    QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>();
    painter->translate(ICON_LEFT_OFFSET, (sizeHint(option, index).height() - pix.height()) / 2);

    drawDecoration(painter, option, pix.rect(), pix);

    const QString &title = index.data(Qt::DisplayRole).toString();
    painter->translate(ICON_LEFT_OFFSET + pix.width(), 0);
    QFont f = option.font;
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(0, 0, title);

    const QString &version = tr("Version: %1").arg(index.data(QAddonListModel::VersionRole).toString());
    painter->translate(0, QFontMetrics(f).boundingRect(title).height() + TEXT_TOP_OFFSET);
    QFont newFont = option.font;
    newFont.setItalic(true);
    newFont.setPointSize(12);
    painter->setFont(newFont);
    painter->drawText(0, 0, version);

    const QString &author = index.data(QAddonListModel::AuthorRole).toString();
    const QString &authorStrFull = tr("Created by: %1").arg(author);
    painter->translate(QFontMetrics(newFont).boundingRect(version).width() + TEXT_RIGHT_OFFSET, 0);
    painter->drawText(0,0, authorStrFull);

    painter->resetTransform();
    painter->setPen(QColor(0x2A, 0x2A, 0x2A));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->restore();
}
