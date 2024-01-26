#ifndef NODEITEM_H
#define NODEITEM_H

#include <iostream>
#include <cmath>
#include <QVector>
#include <QTextStream>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include <QDebug>
#include <QSet>
#include <QGraphicsSceneMouseEvent> //为了实现拖动
#include <QDrag>
#include <QMimeData>
#include <QApplication>



struct TreeNode
{
    QString feature; //节点属性
    QString value;   //节点值
    QString decision;//决策结果
    QMap<QString, TreeNode*> children; //子节点
};

class NodeItem : public QGraphicsItem
{
public:
    NodeItem(const QString& feature, const QString& value, const QString& decision, QGraphicsItem* parent = nullptr)
        : QGraphicsItem(parent), m_feature(feature), m_value(value), m_decision(decision)
    {
        setFlag(ItemSendsGeometryChanges);
        setFlag(ItemIsMovable, true);
    }

    QRectF boundingRect() const override
    {
        return QRectF(-50, -20, 100, 40); // 返回节点项的边界矩形
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    {
        Q_UNUSED(option)
        Q_UNUSED(widget)

        QFont font;
        font.setPointSize(12);  // 设置字体大小为12
        painter->setFont(font);

        painter->setPen(Qt::black);
        painter->setBrush(Qt::lightGray);
        painter->drawRect(boundingRect()); // 绘制节点的矩形

        painter->setPen(Qt::black);
        painter->drawText(boundingRect(), Qt::AlignCenter, m_decision); // 绘制节点的决策文本

        // 绘制特征信息
        QRectF featureRect = boundingRect().adjusted(10, 10, -10, -10);
        QPointF featurePos = QPointF(pos().x(), pos().y()) + featureRect.center();
        painter->drawText(featurePos, m_feature);

        // 绘制连线特征值
        for (QGraphicsLineItem* line : m_childLines)
        {
            QPointF midPoint = (line->line().p1() + line->line().p2()) / 2;
            painter->drawText(midPoint, line->data(0).toString());
        }
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
    {
        if (change == ItemPositionChange && scene()) {
            // 更新子节点的位置
            foreach (NodeItem* child, m_childItems) {
                child->setPos(child->pos() + value.toPointF());
            }
        }

        return QGraphicsItem::itemChange(change, value);
    }

    void addChildItem(NodeItem* childItem, const QString& featureValue)
    {
        childItem->setParentItem(this);
        m_childItems.append(childItem);

        // 创建连线并保存特征值
        QGraphicsLineItem* line = new QGraphicsLineItem(mapFromItem(this, QPointF(0, 30)).x(), mapFromItem(this, QPointF(0, 30)).y(), mapFromItem(childItem, QPointF(0, 0)).x(), mapFromItem(childItem, QPointF(0, 0)).y(), this);
        line->setData(0, featureValue);
        m_childLines.append(line);
    }

private:
    QString m_feature;
    QString m_value;
    QString m_decision;
    QList<NodeItem*> m_childItems;
    QList<QGraphicsLineItem*> m_childLines;
};


#endif // NODEITEM_H
