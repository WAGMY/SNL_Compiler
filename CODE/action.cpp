#include "action.h"

#include <QGraphicsSceneDragDropEvent>
#include <QPainter>
#include <QCursor>

ParseItem::ParseItem():QGraphicsTextItem()
{

}

ParseItem::ParseItem(const QString &text, QGraphicsItem *parent):QGraphicsTextItem (text,parent)
{
    myparent=parent;
    cengshu=0;

}


void ParseItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
      } else {
        setCursor(Qt::ClosedHandCursor);
      }

}

QPainterPath ParseItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void ParseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsTextItem::paint(painter,option,widget);
    auto rect=boundingRect();
    painter->drawRect(rect);
}
