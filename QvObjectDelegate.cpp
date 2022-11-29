//
// Created by esorochinskiy on 20.11.22.
//

#include "QvObjectDelegate.h"
#include "QAddonListModel.h"
#include <QPainter>

#define ITEM_HEIGHT 64
#define ICON_LEFT_OFFSET 16
#define TEXT_TOP_OFFSET 4
#define TEXT_RIGHT_OFFSET 4
#define TEXT_LEFT_OFFSET 10

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

    painter->drawPixmap(0,0,pix);

    const QString &title = index.data(Qt::DisplayRole).toString();

    QFont f = option.font;
    f.setBold(true);
    painter->setFont(f);
    painter->translate(ICON_LEFT_OFFSET + pix.width(), pix.height() / 16);
    const QRect &brTitle = QFontMetrics(f).boundingRect(title);
    painter->drawText(0, 0, brTitle.width(), brTitle.height(), Qt::AlignTop, title);

    const QString &version = tr("Version: %1").arg(index.data(QAddonListModel::VersionRole).toString());
    painter->translate(0, brTitle.height() + TEXT_TOP_OFFSET);
    QFont newFont = option.font;
    newFont.setItalic(true);
    newFont.setPointSize(12);
    painter->setFont(newFont);
    const QRect &brVersion = QFontMetrics(newFont).boundingRect(version);
    painter->drawText(0, 0, brVersion.width(), brVersion.height(), Qt::AlignTop, version);

    const QString &author = index.data(QAddonListModel::AuthorRole).toString();
    const QString &authorStrFull = tr("Created by: %1").arg(author);
    painter->translate(brVersion.width() + TEXT_RIGHT_OFFSET, 0);
    const QRect &brAuthor = QFontMetrics(newFont).boundingRect(authorStrFull);
    painter->drawText(0,0, brAuthor.width(), brAuthor.height(), Qt::AlignTop, authorStrFull);

    painter->resetTransform();
    newFont = option.font;
    newFont.setItalic(true);
    newFont.setPointSize(8);
    painter->setFont(newFont);

    painter->translate(option.rect.left() + option.rect.width(),
    option.rect.top());
    painter->translate(- 50, (option.rect.height() - pix.height()) / 2 + pix.height() / 16);
    const QString &downTotal = index.data(QAddonListModel::DownloadTotalRole).toString();
    const QRect &brDownTotal = QFontMetrics(newFont).boundingRect(downTotal);
    painter->drawText(0, 0, brDownTotal.width(), brDownTotal.height(), Qt::AlignTop, downTotal);
    painter->translate(0, brDownTotal.height() + TEXT_TOP_OFFSET);

    const QString &downMonthly = index.data(QAddonListModel::DownloadMonthlyRole).toString();
    const QRect &brdownMonthly = QFontMetrics(newFont).boundingRect(downMonthly);
    painter->drawText(0, 0, brdownMonthly.width(), brdownMonthly.height(), Qt::AlignTop, downMonthly);
    painter->translate(0, brdownMonthly.height() + TEXT_TOP_OFFSET);

    const QString &favTotal = index.data(QAddonListModel::FavoriteTotalRole).toString();
    const QRect &brfavTotal = QFontMetrics(newFont).boundingRect(favTotal);
    painter->drawText(0, 0, brfavTotal.width(), brfavTotal.height(), Qt::AlignTop, favTotal);
    
    painter->resetTransform();
    
    painter->setPen(QColor(0x2A, 0x2A, 0x2A));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->restore();
}
