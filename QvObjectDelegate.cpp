//
// Created by esorochinskiy on 20.11.22.
//

#include <QApplication>
#include "QvObjectDelegate.h"
#include "QAddonListModel.h"
#include <QPainter>
#include <QKeyEvent>

#define ITEM_HEIGHT 80
#define ICON_LEFT_OFFSET 16
#define TEXT_TOP_OFFSET 4
#define TEXT_RIGHT_OFFSET 4


QvObjectDelegate::QvObjectDelegate(QObject *parent) : QItemDelegate(parent) {
    QFontMetrics appFm(QApplication::fontMetrics());
    buttonRect = appFm.boundingRect(newVerText).adjusted(-15, -10, 15, 10);
    buttonRect.moveTo(0, 0);

}

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
    const QPixmap &pix = index.data(Qt::DecorationRole).value<QPixmap>();
    painter->translate(ICON_LEFT_OFFSET, (sizeHint(option, index).height() - pix.height()) / 2);

    painter->drawPixmap(0, 0, pix);

    const QString &title = index.data(Qt::DisplayRole).toString();

    QFont f = option.font;
    f.setBold(true);
    f.setPixelSize(13);
    painter->setFont(f);
    painter->translate(ICON_LEFT_OFFSET + pix.width(), pix.height() / 16);
    const QRect &brTitle = QFontMetrics(f).boundingRect(title);
    painter->drawText(0, 0, brTitle.width(), brTitle.height(), Qt::AlignTop, title);

    const QString &cversion = index.data(QAddonListModel::VersionRole).toString();

    const QString &version = tr("Version: %1").arg(cversion);
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
    painter->drawText(0, 0, brAuthor.width(), brAuthor.height(), Qt::AlignTop, authorStrFull);
    
    paintObjectBtn(painter, option, index);

    /*
    newFont.setBold(true);
    const QString &siteversionFull = tr("Available: %1").arg(siteversion);
    const QRect &brsiteversion = QFontMetrics(newFont).boundingRect(siteversionFull);
    painter->resetTransform();
    painter->translate(option.rect.bottomRight());
    painter->translate(- 100 - brsiteversion.width() - TEXT_RIGHT_OFFSET * 2,
                       - brsiteversion.height() - TEXT_TOP_OFFSET * 8);
    
    painter->fillRect(0,0, brsiteversion.width() + TEXT_RIGHT_OFFSET,
                      brsiteversion.height() + TEXT_RIGHT_OFFSET * 4, Qt::green);
    auto pen = painter->pen();
    painter->setPen(Qt::black);
    painter->drawText(TEXT_RIGHT_OFFSET * 2,TEXT_RIGHT_OFFSET * 2,
                      brsiteversion.width(), brsiteversion.height(),
                      Qt::AlignTop, siteversionFull);
    painter->setPen(pen);
     */

    painter->resetTransform();
    newFont = option.font;
    newFont.setItalic(true);
    newFont.setPixelSize(12);
    painter->setFont(newFont);

    painter->translate(option.rect.right(),
                       option.rect.top());
    painter->translate(-55, (option.rect.height() - pix.height()) / 2 - pix.height() / 8);

    makeIcon(painter, "download");

    const QString &downTotal = index.data(QAddonListModel::DownloadTotalRole).toString();
    const QRect &brDownTotal = QFontMetrics(newFont).boundingRect(downTotal);
    painter->drawText(0, 0, brDownTotal.width(), brDownTotal.height(), Qt::AlignTop, downTotal);
    painter->translate(0, brDownTotal.height() + TEXT_TOP_OFFSET);

    makeIcon(painter, "view-calendar-month");

    const QString &downMonthly = index.data(QAddonListModel::DownloadMonthlyRole).toString();
    const QRect &brdownMonthly = QFontMetrics(newFont).boundingRect(downMonthly);
    painter->drawText(0, 0, brdownMonthly.width(), brdownMonthly.height(), Qt::AlignTop, downMonthly);
    painter->translate(0, brdownMonthly.height() + TEXT_TOP_OFFSET);

    makeIcon(painter, "favorite");

    const QString &favTotal = index.data(QAddonListModel::FavoriteTotalRole).toString();
    const QRect &brfavTotal = QFontMetrics(newFont).boundingRect(favTotal);
    painter->drawText(0, 0, brfavTotal.width(), brfavTotal.height(), Qt::AlignTop, favTotal);

    painter->resetTransform();

    painter->setPen(QColor(0x2A, 0x2A, 0x2A));
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    painter->restore();
}

void QvObjectDelegate::makeIcon(QPainter *painter, const QString &nIcon) const {
    painter->translate(-16, 0);
    const QIcon &downIcon = QIcon::fromTheme(nIcon);
    const QPixmap &downPix = downIcon.pixmap(14, 14);
    painter->drawPixmap(0, 0, downPix);
    painter->translate(16, 0);
}

void QvObjectDelegate::paintObjectBtn(QPainter *p, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const {
    const QString &text = getButtonText(index);
    if (text.isEmpty()) {
        return;
    }

    auto *view = qobject_cast<QTreeView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();

    p->resetTransform();

    const QPoint &pNormal = calculateNormal(option);
    p->translate(pNormal.x(), pNormal.y());

    QStyleOptionButton opt;
    opt.rect = buttonRect;
    opt.state |= QStyle::State_Active;
#ifdef _DEBUG
    qDebug() << selectedSet.contains(index);
#endif
    if (selectedSet.contains(index)) {
        opt.state |= QStyle::State_Enabled;
        opt.state |= QStyle::State_Active;
        if (opt.rect.contains(buttonPressedPoint_)) {
            opt.state |= QStyle::State_Sunken;
        }
    } else {
        opt.state &= ~QStyle::State_Enabled;
        opt.state &= ~QStyle::State_Active;
    }
    opt.text = text;

    QStyle *style = QApplication::style();
    if (style) {
        style->drawControl(QStyle::CE_PushButton, &opt, p);
    }
    opt.state &= ~QStyle::State_Sunken;

}

QPoint QvObjectDelegate::calculateNormal(const QStyleOptionViewItem &option) const {

    QPoint p(option.rect.bottomRight().x() - 80 - buttonRect.width() - TEXT_RIGHT_OFFSET * 2,
             option.rect.bottomRight().y() - (option.rect.height() + buttonRect.height()) / 2);
    return p;
}

bool QvObjectDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) {
    auto *view = qobject_cast<QTreeView *>(parent());
    const QModelIndexList &selectedSet = view->selectionModel()->selectedIndexes();
    if (!selectedSet.contains(index)) {
        return false;
    }

    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseMove)
        || (event->type() == QEvent::MouseButtonPress)) {
        return validateButton(index, option, event);
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent *>(event)->key() != Qt::Key_Space
            && static_cast<QKeyEvent *>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }
    return false;
}

bool QvObjectDelegate::validateButton(const QModelIndex &index, const QStyleOptionViewItem &option, QEvent *event) {
    const QString &txt = getButtonText(index);
    if (txt.isEmpty()) {
        return false;
    }
    
    QRect checkRect(buttonRect);

    auto *me = dynamic_cast<QMouseEvent *>(event);
    if (event->type() == QEvent::MouseButtonRelease) {
        buttonPressedPoint_ = QPoint();
    }

    if (me->button() == Qt::LeftButton) {
        const QPoint &pNormal = calculateNormal(option);
        const QPoint &newX = me->pos() - pNormal;
        if (checkRect.contains(newX)) {

            if (event->type() == QEvent::MouseButtonPress) {
                buttonPressedPoint_ = newX;
                return true;
            }
            emit reinstallButtonClicked();
            return true;
        }
        return false;
    } else {
        return false;
    }
}

QString QvObjectDelegate::getButtonText(const QModelIndex &index) const {
    const QString &cversion = index.data(QAddonListModel::VersionRole).toString();
    const QString &siteversion = index.data(QAddonListModel::SiteVersionRole).toString();
    ItemData::ItemStatus cStatus = index.data(QAddonListModel::StatusRole).value<ItemData::ItemStatus>();
    if (cStatus != ItemData::NotInstalled && cversion != siteversion) {
        return newVerText;
    } else if (cStatus == ItemData::NotInstalled) {
        return installText;
    } else {
        return {};
    }
}
