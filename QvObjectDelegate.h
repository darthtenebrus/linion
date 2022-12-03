//
// Created by esorochinskiy on 20.11.22.
//

#ifndef LINION_QVOBJECTDELEGATE_H
#define LINION_QVOBJECTDELEGATE_H


#include <QItemDelegate>

class QvObjectDelegate : public QItemDelegate {

Q_OBJECT
protected:
    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const override;

public:
    explicit QvObjectDelegate(QObject *parent);
    ~QvObjectDelegate() override;

    [[nodiscard]]
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void makeIcon(QPainter *painter, const QString &nIcon) const;
};


#endif //LINION_QVOBJECTDELEGATE_H
