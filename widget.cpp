#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_view = ui->graphicsView;         // 动态分配内存
    Widget::setWindowTitle("决策树生成与判断程序");
    m_scene = new QGraphicsScene(this); // 动态分配内存
    m_view->setScene(m_scene);
    m_view->setRenderHint(QPainter::Antialiasing); // 设置抗锯齿渲染

    m_view->setInteractive(false); // 启用交互模式
//    m_view->setDragMode(QGraphicsView::ScrollHandDrag); // 设置拖动模式为滚动手势拖动
//    m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate); // 设置视口更新模式
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // 显示水平滚动条
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // 显示垂直滚动条
    root = new TreeNode;
}

Widget::~Widget()
{
    delete ui;
}

// 计算数据集的经验熵
double Widget::calculateEntropy(const QVector<QVector<QString>>& data)
{
    // 统计不同分类的数量
    QMap<QString, int> count;               //这是Qt的哈希表
    for (int i = 1; i < data.size(); i++)
    {
        const QVector<QString>& instance = data[i];
        QString decision = instance.last(); //最后一列为分类类别
        count[decision]++;                  //利用Hash表计数最简单
    }

    // 计算熵
    double entropy = 0.0;
    int totalInstances = data.size()-1;      //这是总数，减一是因为data中包含了第一行，标签
    for (const QString& decision : count.keys())
    {
        double probability = static_cast<double>(count[decision]) / totalInstances;
        entropy -= probability * log2(probability);
    }

    return entropy;
}

// 计算给定属性的信息增益
double Widget::calculateInformationGain(const QVector<QVector<QString>>& data, int attributeIndex)
{
    QMap<QString, int> count; // 统计每个属性值的数量

    for (int i = 1; i < data.size(); i++)
    {
        const QVector<QString>& instance = data[i];
        QString attributeValue = instance[attributeIndex];
        count[attributeValue]++;
    }

    int totalInstances = data.size()-1;
    double informationGain = calculateEntropy(data);

    // 对每个属性值计算信息增益
    for (const QString& attributeValue : count.keys())
    {
        QVector<QVector<QString>> subset;

        for (int i = 1; i < data.size(); i++)
        {
            const QVector<QString>& instance = data[i];
            if (instance[attributeIndex] == attributeValue)
            {
                subset.append(instance);
            }
        }

        double probability = static_cast<double>(count[attributeValue]) / totalInstances;
        informationGain -= probability * calculateEntropy(subset);
    }

    return informationGain;
}


// 选择最佳属性
QString Widget::selectBestAttribute(const QVector<QVector<QString>>& data)
{

    int numAttributes = data.first().size() - 2; // 减去决策结果列和编号列

    double maxInformationGain = 0.0;
    QString bestAttribute;

    // 对每个属性计算信息增益，并选择具有最大信息增益的属性
    for (int attributeIndex = 1; attributeIndex < numAttributes+1 ; attributeIndex++) //从1开始，有编号列
    {
        double informationGain = calculateInformationGain(data, attributeIndex);
        if (informationGain > maxInformationGain)
        {
            maxInformationGain = informationGain;
            bestAttribute = data[0][attributeIndex];
        }
    }

    return bestAttribute;
}


void Widget::on_pushButton_clicked() //这是读取数据集的代码
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择训练数据",QString(), "CSV 文件 (*.csv)");
    QVector<QVector<QString>> data;
    //打开读取的文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 文件打开失败，处理错误
        return;
    }
    QTextStream in(&file);
    // 逐行读取文件内容
    while (!in.atEnd())
    {
        QString line = in.readLine(); // 读取一行数据

        // 将一行数据按照分隔符（如逗号）拆分为多个字段
        QStringList fields  = line.split(","); // 使用逗号作为字段分隔符
        for (int i = 0; i < fields.size(); ++i)
        {
            fields[i] = fields[i].trimmed();   //去除数据集中的空格
        }
        QVector<QString> row = fields.toVector();

        // 将当前行数据添加到数据集中
        data.append(row);
    }

    file.close();
    root = buildDecisionTree(data);   // 生成的数保存在root中
    // 在data中存储了逐行读取的文件内容
//    double t = calculateEntropy(data);         //测试代码
//    t = calculateInformationGain(data,1);      //测试代码，测试计算第一个特征的信息增益，因为有编号列，所以需要从1开始
//    QString l = selectBestAttribute(data);

//    visualizeTree(Tree);

}



// 构建决策树
TreeNode* Widget::buildDecisionTree(const QVector<QVector<QString>>& data)
{
    // 创建根节点
    TreeNode* root = new TreeNode;


    // 检查数据集中的决策结果是否一致,也就是只有一类
    bool sameDecision = true;
    QString decision = data[1].last();
    for (int i = 1; i < data.size(); i++)
    {
        const QVector<QString>& instance = data[i];
        if (instance.last() != decision)
        {
            sameDecision = false;
            break;
        }
    }

    if (sameDecision)
    {
        // 所有实例的决策结果一致，设置为叶子节点
        root->decision = decision;
        return root;
    }

    if (data.first().size()-2 == 1)
    {
        // 所有属性都已经用完，设置为叶子节点
        QMap<QString, int> count;
        for (const QVector<QString>& instance : data)
        {
            QString decision = instance.last();
            count[decision]++;
        }

        QString mostCommonDecision;
        int maxCount = 0;
        for (const QString& decision : count.keys()) {
            int currentCount = count[decision];
            if (currentCount > maxCount) {
                mostCommonDecision = decision;
                maxCount = currentCount;
            }
        }

        root->decision = mostCommonDecision;
        return root;
    }

    // 选择最佳属性作为当前节点的属性
    QString bestAttribute = selectBestAttribute(data);
    root->feature = bestAttribute;

    // 获取最佳属性的所有可能取值
    QStringList attributeValues;
    int attributeIndex = 1;    //用这个变量找到最优取值的index
    for (; attributeIndex < data.first().size()-1; attributeIndex++) //减一要注意
    {
        if (data.first()[attributeIndex] == bestAttribute)
        {
            break;
        }
    }

    for (int i = 1; i < data.size(); i++)
    {
        const QVector<QString>& instance = data[i];
        QString attributeValue = instance[attributeIndex];
        if (!attributeValues.contains(attributeValue))
        {
            attributeValues.append(attributeValue);
        }
    }

    // 递归构建子节点
    for (const QString& value : attributeValues)
    {
        QVector<QVector<QString>> subset;
        QVector<QString> reducedInstance = data[0];
        reducedInstance.remove(attributeIndex);
        subset.append(reducedInstance);    //需要把第一行的编号给他加上，这样就不需要修改所有的代码

        for (const QVector<QString>& instance : data)
        {
            if (instance[attributeIndex] == value)
            {
                QVector<QString> reducedInstance = instance;
                reducedInstance.remove(attributeIndex);
                subset.append(reducedInstance);
            }
        }

        // 如果subset为空，则创建叶子节点
        if (subset.length()==1) //因为我传的subset至少有一个，所以如果size为1则为子节点
        {
            TreeNode* leaf = new TreeNode;
            QMap<QString, int> count;
            for (const QVector<QString>& instance : data)
            {
                QString decision = instance.last();
                count[decision]++;
            }

            QString mostCommonDecision;
            int maxCount = 0;
            for (const QString& decision : count.keys())
            {
                int currentCount = count[decision];
                if (currentCount > maxCount)
                {
                    mostCommonDecision = decision;
                    maxCount = currentCount;
                }
            }

            leaf->decision = mostCommonDecision;
            root->children[value] = leaf;
        }
        else
        {
            // 递归构建子节点
//            TreeNode* child = new TreeNode;
            TreeNode* child = buildDecisionTree(subset);  //很重要的区分，如果subset里面没有第一行的编号了，应该需要给它加上
            root->children[value] = child;
        }
    }

    return root;
}

void Widget::drawTree(const TreeNode* node, NodeItem* parentItem)
{
    if (!node) return;

    // 创建节点对象
    NodeItem* nodeItem = new NodeItem(node->feature, node->value, node->decision, parentItem);
    nodeItem->setPos(0, 0); // 设置节点相对于父节点的位置，这里默认为 (0, 0)
    m_scene->addItem(nodeItem);  // 添加节点到场景中
    int n = node->children.size();
    int length = qRound(800.0 / n);  // 将浮点数结果四舍五入为最接近的整数
    length = qRound(length / 100.0) * 100;  // 将结果调整为100的倍数
    if(length<100) length=200;


    // 绘制子节点
    qreal childX = -100 - (node->children.count() - 1) * 100;  // 调整子节点的起始位置
    qreal childY = 200;
    for (const QString& value : node->children.keys())
    {
        TreeNode* childNode = node->children[value];
        NodeItem* childItem = new NodeItem(childNode->feature, childNode->value, childNode->decision, nodeItem);
        childItem->setPos(childX, childY);
        m_scene->addItem(childItem);

        nodeItem->addChildItem(childItem, value);

        // 递归绘制子节点的子节点
        drawTree(childNode, childItem);

        // 调整下一个子节点的位置
        childX += length;
    }
}


// 递归将决策树转换为JSON对象
QJsonObject Widget::convertTreeToJson(const TreeNode* node)
{
    QJsonObject jsonNode;
    if (!node) return jsonNode;

    //这里区分了内部节点和叶子节点
    if (!node->feature.isEmpty())   jsonNode["feature"]  = node->feature;
    if (!node->decision.isEmpty())  jsonNode["decision"] = node->decision;

    QJsonObject children;
    for (const QString& value : node->children.keys()) //如果没有子节点，则返回
        children[value] = convertTreeToJson(node->children[value]);

    jsonNode["children"] = children;

    return jsonNode;
}

// 将决策树写入JSON文件
bool Widget::writeTreeToJsonFile(const TreeNode* root, const QString& filePath)
{
    QJsonObject jsonTree = convertTreeToJson(root);

    QJsonDocument jsonDocument(jsonTree);
    QByteArray jsonData = jsonDocument.toJson();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(jsonData);
    file.close();

    return true;
}

// 辅助函数，递归从JSON对象构建树结构
TreeNode* Widget::constructTreeFromJson(const QJsonObject& jsonObj)
{
    TreeNode* node = new TreeNode;

    // 读取特征和决策值
    node->feature = jsonObj["feature"].toString();
    node->decision = jsonObj["decision"].toString();

    // 递归读取子节点
    QJsonObject childrenObj = jsonObj["children"].toObject();
    QStringList keys = childrenObj.keys();
    for (const QString& key : keys)
    {
        QJsonValue childValue = childrenObj[key];
        if (childValue.isObject())
        {
            QJsonObject childObj = childValue.toObject();
            TreeNode* childNode = constructTreeFromJson(childObj);
            node->children[key] = childNode;
        }
    }

    return node;
}

// 从JSON文件中读取并更新树的函数
void Widget::readTreeFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // 文件打开失败，处理错误
        return;
    }

    // 从文件中读取JSON数据
    QString jsonContent = file.readAll();
    file.close();

    // 解析JSON数据
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonContent.toUtf8());
    if (!jsonDoc.isObject())
    {
        // JSON格式无效，处理错误
        return;
    }

    // 获取根对象
    QJsonObject jsonObj = jsonDoc.object();

    // 使用解析得到的树结构更新类`root`
    root = constructTreeFromJson(jsonObj);
}

void Widget::visualizeTree(const TreeNode* root)
{
        m_scene->clear();
        drawTree(root, nullptr);
}

void Widget::on_pushButton_2_clicked()
{
    if(!root->children.isEmpty()) visualizeTree(root);
}

void Widget::on_pushButton_3_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择测试数据",QString(), "CSV 文件 (*.csv)");
    testdata.clear(); //防止多次选择测试数据

    //打开读取的文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // 文件打开失败，处理错误
        return;
    }
    QTextStream in(&file);
    // 逐行读取文件内容
    while (!in.atEnd())
    {
        QString line = in.readLine(); // 读取一行数据

        // 将一行数据按照分隔符（如逗号）拆分为多个字段
        QStringList fields  = line.split(","); // 使用逗号作为字段分隔符
        for (int i = 0; i < fields.size(); ++i)
        {
            fields[i] = fields[i].trimmed();   //去除数据集中的空格
        }
        QVector<QString> row = fields.toVector();

        // 将当前行数据添加到数据集中
        testdata.append(row);
    }

    file.close();
    int numRows = testdata.size();                       // 获得行数
    int numColumns = testdata.empty() ? 0 : testdata[0].size();   // 获得列数，假设所有行具有相同的列数
    for (int i = 1;i<numRows; ++i) //第一行是列标
    {
        QString classification;
        TreeNode* Temp = root;

        for(int j = 1; j < numColumns; ++j)//第一列是序号
        {
            int k = 0;
            for ( ; k < numColumns; k++)//找此时的节点是什么值
            {
                if(testdata[0][k]==Temp->feature)    //此时的k应该是当前需要判断的特征
                    break;
            }

            if (!Temp->children.contains(testdata[i][k])) // 检查节点的子节点是否包含当前值,这是鸢尾花数据集给我的想法
            {
                 break; // 如果不包含，则退出当前行的判断
            }
            Temp = Temp->children[testdata[i][k]];
            if(!Temp->decision.isEmpty())
            {
                classification=Temp->decision;
                break;
            }
        }
        testdata[i].append(classification); //这里，虽然推出了当前行的判断，但是还是会加入一个空值，所以如果为空，则表示判断不了
    }

}

void Widget::on_pushButton_4_clicked()
{
    // 假设要输出 data 的第一行
    if(testdata.isEmpty()) return;
    testdata[0].append("分类");
    int numRows = testdata.size();
    int numColumns = testdata.empty() ? 0 : testdata[0].size();

    //表格显示之前，必须设置行数和列数
    ui->tableWidget->setRowCount(numRows);
    ui->tableWidget->setColumnCount(numColumns);


    for(int i = 0; i <numRows; i++)
    {
        for(int j = 0 ; j <numColumns; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(testdata[i][j]);
            item->setTextAlignment(Qt::AlignCenter); // 设置数据居中对齐
            ui->tableWidget->setItem(i, j, item);
        }
    }

}

void Widget::on_pushButton_5_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, "保存决策树", QString(), "JSON文件 (*.json)");
//    QString filePath = "保存的决策树.json";
    if (writeTreeToJsonFile(root, filePath))
         qDebug() << "决策树已成功写入JSON文件：" << filePath;
    else
         qDebug() << "写入JSON文件时出现错误。";
}

void Widget::on_pushButton_6_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "选择树的JSON文件", QString(), "JSON 文件 (*.json)");
    if (!filePath.isEmpty())
    {
        readTreeFromJson(filePath);
    }
}
