#include "datashowdialog.h"
#include "ui_datashowdialog.h"
#include "QMessageBox"
#include "QFileDialog"
#include "serialmgr.h"
#include "QValueAxis"
DataShowDialog::DataShowDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DataShowDialog),
    maxX(std::numeric_limits<int>::min()),
    maxY(std::numeric_limits<int>::min()),
    index(0)
{
    ui->setupUi(this);
    chart = new QChart();

    line_series = new QLineSeries();


    chart->addSeries(line_series);
    chart->setTitle("十六进制可视化"); // 设置图表标题
    axisY = new QValueAxis();
    axisX = new QValueAxis();
    axisY->setLabelFormat("0x%X");
    axisY->setTitleText("Value (Hex)");

    // 将坐标轴附加到图表
    chart->addAxis(axisX,Qt::AlignBottom);
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
    QString file_dir = QFileDialog::getSaveFileName(this,"保存文件","save.png","所有文件 (*)");
    QPixmap pixmap = chart_view->grab();
    pixmap.save(file_dir,"PNG");


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
        line_series->append(index,decimal);
        if(maxY<decimal)
        {
            maxY = decimal;
            axisY->setRange(0,maxY);
        }
        if(index>maxX)
        {
            maxX = index;
            axisX->setRange(0,maxX);
        }
        index++;
    }


}

void DataShowDialog::on_clear_pushButton_clicked()
{
    line_series->clear();
    maxX = std::numeric_limits<int>::min();
    maxY = std::numeric_limits<int>::min();
    index = 0;
}

