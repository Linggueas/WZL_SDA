#include "datashowdialog.h"
#include "ui_datashowdialog.h"
#include "QMessageBox"
#include "QFileDialog"
#include "serialmgr.h"
#include <QWheelEvent>
#include <QCoreApplication>

const int DataShowDialog::MAX_DISPLAY_POINTS;

DataShowDialog::DataShowDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataShowDialog),
    maxX(std::numeric_limits<int>::min()),
    maxY(std::numeric_limits<int>::min()),
    index(0)
{
    ui->setupUi(this);

    // 初始化折线图系列
    line_series = new QLineSeries();
    line_series->setName("折线图");

    // 初始化图表
    chart = new QChart();
    chart->setTitle("十六进制可视化");
    chart->addSeries(line_series);

    // 初始化坐标轴
    axisY = new QValueAxis();
    axisX = new QValueAxis();
    axisY->setLabelFormat("0x%X");
    axisY->setTitleText("Value (Hex)");
    axisX->setTitleText("Index");

    // 添加坐标轴
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    line_series->attachAxis(axisX);
    line_series->attachAxis(axisY);

    chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);

    ui->stackedWidget->addWidget(chart_view);
    ui->stackedWidget->setCurrentWidget(chart_view);
}

DataShowDialog::~DataShowDialog()
{
    delete chart;
    delete chart_view;
    delete line_series;
    delete axisX;
    delete axisY;
    delete ui;
}

bool DataShowDialog::eventFilter(QObject *watched, QEvent *event)
{
    return QDialog::eventFilter(watched, event);
}

void DataShowDialog::on_mess_pushButton_clicked()
{
    QMessageBox::information(this,"提示","请发送单个HEX,\n否则会出现界面的乱码");
}


void DataShowDialog::on_start_pushButton_clicked()
{
    connect(SerialMgr::GetInstance().get(),&SerialMgr::sig_read_data,this,&DataShowDialog::on_start_show_data);
    ui->stop_pushButton->setEnabled(true);
    ui->start_pushButton->setEnabled(false);
}


void DataShowDialog::on_stop_pushButton_clicked()
{
    ui->stop_pushButton->setEnabled(false);
    disconnect(SerialMgr::GetInstance().get(),&SerialMgr::sig_read_data,this,&DataShowDialog::on_start_show_data);
    ui->start_pushButton->setEnabled(true);
}


void DataShowDialog::on_save_pushButton_clicked()
{
    QString file_dir = QFileDialog::getSaveFileName(this,"保存文件","save.png","PNG文件 (*.png);;所有文件 (*)");
    if(file_dir.isEmpty())
    {
        return;
    }

    // 直接抓取当前显示的图表视图
    QPixmap pixmap = chart_view->grab();

    // 保存图片
    if(!pixmap.save(file_dir, "PNG"))
    {
        QMessageBox::critical(this, "错误", "保存图片失败！");
    }
    else
    {
        QMessageBox::information(this, "成功", "图片保存成功！");
    }
}
//数据显示
void DataShowDialog::on_start_show_data(QByteArray data)
{
    QByteArray bdata = data.toHex();
    QString hexString = QString::fromUtf8(bdata);
    bool ok;
    int decimal = hexString.toInt(&ok, 16);
    if(ok)
    {
        // 存储数据点
        dataPoints.append(QPointF(index, decimal));

        // 更新Y轴范围
        if(maxY < decimal)
        {
            maxY = decimal;
            axisY->setRange(0, maxY);
        }

        // 更新X轴最大值
        if(index > maxX)
        {
            maxX = index;
        }

        // 自动滚动到最新数据
        autoScroll();

        index++;
    }
}

// 自动滚动到最新数据
void DataShowDialog::autoScroll()
{
    int totalPoints = dataPoints.size();
    
    // 清除当前所有系列
    chart->removeAllSeries();
    
    // 根据当前选择的图表类型重新创建系列
    int chartTypeIndex = ui->chartTypeComboBox->currentIndex();
    
    switch(chartTypeIndex)
    {
    case 0: // 折线图
    {
        line_series = new QLineSeries();
        line_series->setName("折线图");
        
        if(totalPoints <= MAX_DISPLAY_POINTS)
        {
            // 数据点少于最大显示数，显示所有数据
            for(const QPointF &point : dataPoints)
            {
                line_series->append(point);
            }
            axisX->setRange(0, qMax(MAX_DISPLAY_POINTS - 1, maxX));
        }
        else
        {
            // 数据点超过最大显示数，只显示最近的数据
            int startIdx = totalPoints - MAX_DISPLAY_POINTS;
            for(int i = startIdx; i < totalPoints; i++)
            {
                line_series->append(dataPoints[i]);
            }
            axisX->setRange(startIdx, maxX);
        }
        
        chart->addSeries(line_series);
        line_series->attachAxis(axisX);
        line_series->attachAxis(axisY);
        break;
    }
    case 1: // 柱状图
    {
        QBarSeries *bar_series = new QBarSeries();
        QBarSet *bar_set = new QBarSet("柱状图");
        
        if(totalPoints <= MAX_DISPLAY_POINTS)
        {
            // 数据点少于最大显示数，显示所有数据
            for(const QPointF &point : dataPoints)
            {
                *bar_set << point.y();
            }
            axisX->setRange(0, qMax(MAX_DISPLAY_POINTS - 1, maxX));
        }
        else
        {
            // 数据点超过最大显示数，只显示最近的数据
            int startIdx = totalPoints - MAX_DISPLAY_POINTS;
            for(int i = startIdx; i < totalPoints; i++)
            {
                *bar_set << dataPoints[i].y();
            }
            axisX->setRange(startIdx, maxX);
        }
        
        bar_series->append(bar_set);
        chart->addSeries(bar_series);
        bar_series->attachAxis(axisX);
        bar_series->attachAxis(axisY);
        break;
    }
    case 2: // 散点图
    {
        QScatterSeries *scatter_series = new QScatterSeries();
        scatter_series->setName("散点图");
        scatter_series->setMarkerSize(8);
        
        if(totalPoints <= MAX_DISPLAY_POINTS)
        {
            // 数据点少于最大显示数，显示所有数据
            for(const QPointF &point : dataPoints)
            {
                scatter_series->append(point);
            }
            axisX->setRange(0, qMax(MAX_DISPLAY_POINTS - 1, maxX));
        }
        else
        {
            // 数据点超过最大显示数，只显示最近的数据
            int startIdx = totalPoints - MAX_DISPLAY_POINTS;
            for(int i = startIdx; i < totalPoints; i++)
            {
                scatter_series->append(dataPoints[i]);
            }
            axisX->setRange(startIdx, maxX);
        }
        
        chart->addSeries(scatter_series);
        scatter_series->attachAxis(axisX);
        scatter_series->attachAxis(axisY);
        break;
    }
    default:
        break;
    }
}

// 鼠标滚轮事件 - 缩放查看
void DataShowDialog::wheelEvent(QWheelEvent *event)
{
    if(dataPoints.isEmpty()) return;

    int delta = event->angleDelta().y();
    int totalPoints = dataPoints.size();
    
    // 计算当前显示的起始索引
    int currentStartIdx = 0;
    if(totalPoints > MAX_DISPLAY_POINTS)
    {
        // 尝试从当前X轴范围获取起始索引
        currentStartIdx = static_cast<int>(axisX->min());
        // 确保起始索引在有效范围内
        currentStartIdx = qMax(0, qMin(currentStartIdx, totalPoints - MAX_DISPLAY_POINTS));
    }
    
    // 根据滚轮滚动方向调整起始索引
    // 每次滚动调整1个数据点，这样滚动速度会变慢，便于在中间位置停留
    if(delta > 0)
    {
        // 向上滚动 - 查看更新的数据，起始索引增加
        currentStartIdx = qMin(currentStartIdx + 1, totalPoints - MAX_DISPLAY_POINTS);
    }
    else
    {
        // 向下滚动 - 查看更旧的数据，起始索引减少
        currentStartIdx = qMax(currentStartIdx - 1, 0);
    }

    // 清除当前所有系列
    chart->removeAllSeries();
    
    // 根据当前选择的图表类型重新创建系列
    int chartTypeIndex = ui->chartTypeComboBox->currentIndex();
    
    // 计算结束索引
    int endIdx = qMin(currentStartIdx + MAX_DISPLAY_POINTS, totalPoints);
    
    switch(chartTypeIndex)
    {
    case 0: // 折线图
    {
        line_series = new QLineSeries();
        line_series->setName("折线图");
        for(int i = currentStartIdx; i < endIdx; i++)
        {
            line_series->append(dataPoints[i]);
        }
        chart->addSeries(line_series);
        line_series->attachAxis(axisX);
        line_series->attachAxis(axisY);
        break;
    }
    case 1: // 柱状图
    {
        QBarSeries *bar_series = new QBarSeries();
        QBarSet *bar_set = new QBarSet("柱状图");
        for(int i = currentStartIdx; i < endIdx; i++)
        {
            *bar_set << dataPoints[i].y();
        }
        bar_series->append(bar_set);
        chart->addSeries(bar_series);
        bar_series->attachAxis(axisX);
        bar_series->attachAxis(axisY);
        break;
    }
    case 2: // 散点图
    {
        QScatterSeries *scatter_series = new QScatterSeries();
        scatter_series->setName("散点图");
        scatter_series->setMarkerSize(8);
        for(int i = currentStartIdx; i < endIdx; i++)
        {
            scatter_series->append(dataPoints[i]);
        }
        chart->addSeries(scatter_series);
        scatter_series->attachAxis(axisX);
        scatter_series->attachAxis(axisY);
        break;
    }
    default:
        break;
    }
    
    // 更新X轴范围
    axisX->setRange(currentStartIdx, endIdx - 1);
    
    event->accept();
}

void DataShowDialog::on_clear_pushButton_clicked()
{
    // 清除所有系列
    chart->removeAllSeries();
    
    // 重新创建坐标轴
    chart->removeAxis(axisX);
    chart->removeAxis(axisY);
    
    axisY = new QValueAxis();
    axisX = new QValueAxis();
    axisY->setLabelFormat("0x%X");
    axisY->setTitleText("Value (Hex)");
    axisX->setTitleText("Index");
    
    // 添加坐标轴
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    // 根据当前选择的图表类型重新创建系列
    int chartTypeIndex = ui->chartTypeComboBox->currentIndex();
    
    switch(chartTypeIndex)
    {
    case 0: // 折线图
    {
        line_series = new QLineSeries();
        line_series->setName("折线图");
        chart->addSeries(line_series);
        line_series->attachAxis(axisX);
        line_series->attachAxis(axisY);
        break;
    }
    case 1: // 柱状图
    {
        QBarSeries *bar_series = new QBarSeries();
        QBarSet *bar_set = new QBarSet("柱状图");
        bar_series->append(bar_set);
        chart->addSeries(bar_series);
        bar_series->attachAxis(axisX);
        bar_series->attachAxis(axisY);
        break;
    }
    case 2: // 散点图
    {
        QScatterSeries *scatter_series = new QScatterSeries();
        scatter_series->setName("散点图");
        scatter_series->setMarkerSize(8);
        chart->addSeries(scatter_series);
        scatter_series->attachAxis(axisX);
        scatter_series->attachAxis(axisY);
        break;
    }
    default:
        break;
    }
    
    // 清除数据
    dataPoints.clear();
    maxX = std::numeric_limits<int>::min();
    maxY = std::numeric_limits<int>::min();
    index = 0;
    axisX->setRange(0, MAX_DISPLAY_POINTS - 1);
    axisY->setRange(0, 100); // 设置默认Y轴范围
}

// 重置视图到最新数据
void DataShowDialog::on_resetView_pushButton_clicked()
{
    autoScroll();
}

void DataShowDialog::on_chartTypeComboBox_currentIndexChanged(int chartTypeIndex)
{
    // 保存当前数据
    QVector<QPointF> tempData = dataPoints;
    int tempMaxX = maxX;
    int tempMaxY = maxY;
    int tempIndex = index;
    
    // 清除当前图表
    chart->removeAllSeries();
    chart->removeAxis(axisX);
    chart->removeAxis(axisY);
    
    // 重新创建坐标轴
    axisY = new QValueAxis();
    axisX = new QValueAxis();
    axisY->setLabelFormat("0x%X");
    axisY->setTitleText("Value (Hex)");
    axisX->setTitleText("Index");
    
    // 添加坐标轴
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    // 根据选择的图表类型创建相应的系列
    switch(chartTypeIndex)
    {
    case 0: // 折线图
    {
        line_series = new QLineSeries();
        line_series->setName("折线图");
        chart->addSeries(line_series);
        line_series->attachAxis(axisX);
        line_series->attachAxis(axisY);
        break;
    }
    case 1: // 柱状图
    {
        QBarSeries *bar_series = new QBarSeries();
        QBarSet *bar_set = new QBarSet("柱状图");
        
        for(const QPointF &point : tempData)
        {
            *bar_set << point.y();
        }
        
        bar_series->append(bar_set);
        chart->addSeries(bar_series);
        bar_series->attachAxis(axisX);
        bar_series->attachAxis(axisY);
        break;
    }
    case 2: // 散点图
    {
        QScatterSeries *scatter_series = new QScatterSeries();
        scatter_series->setName("散点图");
        scatter_series->setMarkerSize(8);
        
        for(const QPointF &point : tempData)
        {
            scatter_series->append(point);
        }
        
        chart->addSeries(scatter_series);
        scatter_series->attachAxis(axisX);
        scatter_series->attachAxis(axisY);
        break;
    }
    default:
        break;
    }
    
    // 恢复数据
    dataPoints = tempData;
    maxX = tempMaxX;
    maxY = tempMaxY;
    index = tempIndex;
    
    // 更新Y轴范围
    if(maxY > 0)
    {
        axisY->setRange(0, maxY);
    }
    
    // 更新图表视图
    autoScroll();
}
