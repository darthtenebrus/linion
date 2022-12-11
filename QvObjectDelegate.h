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

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

public:
    explicit QvObjectDelegate(QObject *parent);
    ~QvObjectDelegate() override;

    [[nodiscard]]
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void makeIcon(QPainter *painter, const QString &nIcon) const;
    void paintObjectBtn(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
    QString getButtonText(const QModelIndex &index) const;

    QRect buttonRect;
    QPoint buttonPressedPoint_;
    QString installText = tr("Install");
    QString newVerText = tr("Update Available");

    [[nodiscard]]
    QPoint calculateNormal(const QStyleOptionViewItem &) const;
    bool validateButton(const QModelIndex &index, const QStyleOptionViewItem &option, QEvent *event);

signals:
    void reinstallButtonClicked();


};


#endif //LINION_QVOBJECTDELEGATE_H
