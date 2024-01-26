#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileDialog>
#include <iostream>
#include <cmath>
#include <QVector>
#include <QTextStream>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPainter>
#include "nodeitem.h"
#include <QTableWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


namespace Ui
{
    class Widget;
}



class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    double calculateEntropy(const QVector<QVector<QString>>& data);     //计算数据集熵的函数
    double calculateInformationGain(const QVector<QVector<QString>>& data, int attributeIndex); //计算信息增益
    TreeNode* buildDecisionTree(const QVector<QVector<QString>>& data); //构造决策树
    QString selectBestAttribute(const QVector<QVector<QString>>& data); //选择最佳属性
    void drawTree(const TreeNode* node, NodeItem* parentItem);
    void visualizeTree(const TreeNode* root);
    bool writeTreeToJsonFile(const TreeNode* root, const QString& filePath);
    QJsonObject convertTreeToJson(const TreeNode* node);
    TreeNode* constructTreeFromJson(const QJsonObject& jsonObj);
    void readTreeFromJson(const QString& filePath);
    QGraphicsView *m_view;
    QGraphicsScene *m_scene;


private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::Widget *ui;

    TreeNode* root;
    QVector<QVector<QString>> testdata; //测试数据
};

#endif // WIDGET_H
