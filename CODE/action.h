#ifndef PARSEITEM_H
#define PARSEITEM_H

#include <QGraphicsItem>
#include <QObject>

class ParseItem : public QGraphicsTextItem
{
public:
    ParseItem();
    ParseItem(const QString &text,QGraphicsItem *parent=Q_NULLPTR);

public:
    QGraphicsItem *myparent;
    int cengshu;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
public:
    QPainterPath shape() const override;

public:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif // PARSEITEM_H
