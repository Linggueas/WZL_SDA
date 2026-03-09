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

    if(totalPoints <= MAX_DISPLAY_POINTS)
    {
        // 数据点少于最大显示数，显示所有数据
        line_series->clear();
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
        line_series->clear();
        for(int i = startIdx; i < totalPoints; i++)
        {
            line_series->append(dataPoints[i]);
        }
        axisX->setRange(startIdx, maxX);
    }
}

// 鼠标滚轮事件 - 缩放查看
void DataShowDialog::wheelEvent(QWheelEvent *event)
{
    if(dataPoints.isEmpty()) return;

    int delta = event->angleDelta().y();
    int totalPoints = dataPoints.size();

    if(delta > 0)
    {
        // 向上滚动 - 查看更新的数据
        int startIdx = qMax(0, totalPoints - MAX_DISPLAY_POINTS);
        line_series->clear();
        for(int i = startIdx; i < totalPoints; i++)
        {
            line_series->append(dataPoints[i]);
        }
        axisX->setRange(startIdx, maxX);
    }
    else
    {
        // 向下滚动 - 查看更旧的数据
        int endIdx = qMin(totalPoints, MAX_DISPLAY_POINTS);
        line_series->clear();
        for(int i = 0; i < endIdx; i++)
        {
            line_series->append(dataPoints[i]);
        }
        axisX->setRange(0, endIdx - 1);
    }
    event->accept();
}

void DataShowDialog::on_clear_pushButton_clicked()
{
    line_series->clear();
    dataPoints.clear();
    maxX = std::numeric_limits<int>::min();
    maxY = std::numeric_limits<int>::min();
    index = 0;
    axisX->setRange(0, MAX_DISPLAY_POINTS - 1);
}

// 重置视图到最新数据
void DataShowDialog::on_resetView_pushButton_clicked()
{
    autoScroll();
}
