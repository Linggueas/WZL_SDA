#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include "QFileDialog"
#include "QFile"
#include "QDateTime"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),if_open(false),his_open(true),more_text_open(true)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->send_pushButton->setEnabled(false);
    ui->time_checkBox->setEnabled(false);
    timer = new QTimer(this);
    more_text_timer = new QTimer(this);
    connect(SerialMgr::GetInstance(),&SerialMgr::sig_read_data,this,&MainWindow::read_data);

    connect(timer,&QTimer::timeout,this,[this](){
        SerialMgr::GetInstance()->send_data("这是定时发送");
    });

    connect(ui->serial_comboBox,&MyComboBox::refresh,this,[this](){
        ui->serial_comboBox->clear();
        QList<QSerialPortInfo> portlist =  SerialMgr::GetInstance()->get_now_port();
        for(QSerialPortInfo port : portlist)
        {
            QString port_name = port.portName();
            ui->serial_comboBox->addItem(port_name);
        }
    });

    //处理多文本
    //按钮
    for(int i = 1;i<=7;i++)
    {
        QString btn_name = QString("pushButton_%1").arg(i);
        QPushButton*btn = findChild<QPushButton*>(btn_name);
        if(btn){
            more_text_button.push_back(btn);
            connect(btn,&QPushButton::clicked,this,&MainWindow::on_more_text_send);
        }
    }
    //文本框
    for(int i = 1;i<=7;i++)
    {
        QString line_name = QString("lineEdit_%1").arg(i);
        QLineEdit*line = findChild<QLineEdit*>(line_name);
        if(line)
        {
            more_text_lineedit.push_back(line);
        }
    }
    //check选项
    for(int i = 1;i<=7;i++)
    {
        QString check_name = QString("checkBox_%1").arg(i);
        QCheckBox*check = findChild<QCheckBox*>(check_name);
        if(check)
        {
            more_text_checkbox.push_back(check);
        }
    }
    //多文本循环发送
    connect(more_text_timer,&QTimer::timeout,this,&MainWindow::while_send_text);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_open_stop_pushButton_clicked()
{
    if(if_open)
    {
        if_open = false;
        ui->open_stop_pushButton->setText("打开串口");
        SerialMgr::GetInstance()->closeSerial();

        ui->serial_comboBox->setEnabled(true);
        ui->Baud_comboBox->setEnabled(true);
        ui->data_comboBox->setEnabled(true);
        ui->Verify_comboBox->setEnabled(true);
        ui->stop_comboBox->setEnabled(true);
        ui->stream_control_comboBox->setEnabled(true);
        ui->send_pushButton->setEnabled(false);
        ui->time_checkBox->setEnabled(false);
        QMessageBox::information(this,"成功","关闭成功");
        timer->stop();
    }else
    {
        bool en = SerialMgr::GetInstance()->openSerial(ui->serial_comboBox->currentText(),
                                             ui->Baud_comboBox->currentText(),
                                             ui->data_comboBox->currentText(),
                                             ui->Verify_comboBox->currentText(),
                                             ui->stop_comboBox->currentText(),
                                             ui->stream_control_comboBox->currentText());

        if(!en)
        {
            QMessageBox::critical(this,"失败","连接失败");

        }else{
            QMessageBox::information(this,"成功","连接成功");
            ui->open_stop_pushButton->setText("关闭串口");
            ui->serial_comboBox->setEnabled(false);
            ui->Baud_comboBox->setEnabled(false);
            ui->data_comboBox->setEnabled(false);
            ui->Verify_comboBox->setEnabled(false);
            ui->stop_comboBox->setEnabled(false);
            ui->stream_control_comboBox->setEnabled(false);
            ui->send_pushButton->setEnabled(true);
            ui->time_checkBox->setEnabled(true);
            if_open = true;
        }
    }
}


void MainWindow::on_send_pushButton_clicked()
{
    QString send_text = ui->send_lineEdit->text();
    if(send_text.isEmpty())
    {
        return;
    }
    if(ui->Hex_checkBox->isChecked())
    {
        SerialMgr::GetInstance()->send_data(QByteArray::fromHex(send_text.toUtf8()));
    }else
    {
        if(ui->save_new_checkBox->isChecked()){
            send_text.append("\r\n");
        }
        SerialMgr::GetInstance()->send_data(send_text);
    }
}

//清除接收
void MainWindow::on_close_rev_pushButton_clicked()
{
    ui->textEdit_rev->clear();
}

//保存接收
void MainWindow::on_save_rev_pushButton_clicked()
{
    QString file_dir = QFileDialog::getSaveFileName(this,"保存文件","接收.txt","所有文件 (*)");
    QFile file(file_dir);
    file.open(QIODevice::WriteOnly);
    file.write(ui->textEdit_rev->toPlainText().toUtf8());
    file.close();
}
void MainWindow::on_time_checkBox_clicked(bool checked)
{
    if(checked)
    {
        timer->start(ui->time_lineEdit->text().toInt());
        ui->send_pushButton->setEnabled(false);
        ui->send_lineEdit->setEnabled(false);
    }else{
        timer->stop();
        ui->send_pushButton->setEnabled(true);
        ui->send_lineEdit->setEnabled(true);
    }
}

//读取
void MainWindow::read_data(QByteArray data)
{
    if(ui->time_checkBox_2->isChecked())
    {
        ui->textEdit_his->append(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss")+":");
        ui->textEdit_rev->append(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss")+":");
    }
    if(ui->hex_rev_checkBox->isChecked())
    {
        QString hexData = data.toHex().toUpper();
        QString hexWithSpace;
        for (int i = 0; i < hexData.size(); i += 2) {
            if (i > 0) hexWithSpace.append(' '); // 添加空格分隔
            hexWithSpace.append(hexData.mid(i, 2)); // 取2个字符
        }
        ui->textEdit_rev->append(hexWithSpace);
        ui->textEdit_his->append(hexWithSpace);
    }else
    {
        ui->textEdit_rev->append(QString(data));
        ui->textEdit_his->append(QString(data));
    }
    ui->textEdit_rev->moveCursor(QTextCursor::End);
    ui->textEdit_rev->ensureCursorVisible();
    ui->textEdit_his->moveCursor(QTextCursor::End);
    ui->textEdit_his->ensureCursorVisible();
}


//grouphis_open
//这个懒得改名了
void MainWindow::on_pushButton_13_clicked()
{
    if(his_open){
        ui->groupBox_his->hide();
        his_open = false;
        ui->pushButton_13->setText("显示历史");
    }else{
        ui->groupBox_his->show();
        his_open = true;
        ui->pushButton_13->setText("隐藏历史");
    }
}


void MainWindow::on_show_text_pushButton_clicked()
{
    if(more_text_open)
    {
        ui->groupBox_text->hide();
        more_text_open = false;
        ui->show_text_pushButton->setText("显示面板");
    }else{
        ui->groupBox_text->show();
        more_text_open = true;
        ui->show_text_pushButton->setText("隐藏面板");
    }
}

void MainWindow::on_more_text_send()
{
    QPushButton*btn = qobject_cast<QPushButton*>(sender());
    auto it = std::find(more_text_button.begin(),more_text_button.end(),btn);
    int index = std::distance(more_text_button.begin(),it);
    if(more_text_lineedit[index]->text().isEmpty())
    {
        return;
    }
    if(more_text_checkbox[index]->isChecked()){
        SerialMgr::GetInstance()->
            send_data(QByteArray::fromHex(more_text_lineedit[index]->text().toUtf8()));
    }else{
        QString send_text = more_text_lineedit[index]->text();
        if(ui->save_new_checkBox->isChecked())
        {
            send_text.append("\r\n");
        }
        SerialMgr::GetInstance()->send_data(send_text);
    }
}


void MainWindow::on_while_checkBox_clicked(bool checked)
{
    if(checked)
    {
        more_text_timer->start(ui->spinBox->text().toInt());
        more_text_index = 0;
        for(int i = 0;i<7;i++){
            more_text_button[i]->setEnabled(false);
        }
    }else
    {
        more_text_timer->stop();
        more_text_index = 0;
        for(int i = 0;i<7;i++){
            more_text_button[i]->setEnabled(true);
        }
    }
}

void MainWindow::while_send_text()
{

    if(more_text_lineedit[more_text_index]->text().isEmpty())
    {
        more_text_index++;
        if(more_text_index == 7){
            more_text_index = 0;
        }
        return;
    }
    if(more_text_checkbox[more_text_index]->isChecked())
    {
        SerialMgr::GetInstance()->
            send_data(QByteArray::fromHex(more_text_lineedit[more_text_index]->text().toUtf8()));
    }else
    {
        QString send_text = more_text_lineedit[more_text_index]->text();
        if(ui->save_new_checkBox->isChecked())
        {
            send_text.append("\r\n");
        }
        SerialMgr::GetInstance()->send_data(send_text);

    }
    more_text_index++;
    if(more_text_index == 7){
        more_text_index = 0;
    }

}


void MainWindow::on_Reset_pushButton_clicked()
{
    for(int i = 0;i<more_text_button.size();i++)
    {
        more_text_lineedit[i]->clear();
        more_text_checkbox[i]->setChecked(false);
    }
}

//保存格式为json文件
void MainWindow::on_save_pushButton_clicked()
{
    QJsonArray json_array;
    for(int i = 0;i<7;i++)
    {
        QJsonObject json_obj;
        json_obj["text"] = more_text_lineedit[i]->text();
        json_obj["ischeck"] = more_text_checkbox[i]->isChecked();
        json_array.push_back(json_obj);
    }
    QJsonDocument json_doc(json_array);
    QString file_dir = QFileDialog::getSaveFileName(this,"保存文件","模板.json","所有文件 (*)");
    QFile file(file_dir);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(json_doc.toJson());
    }
}

//通过json文件获取
void MainWindow::on_down_pushButton_clicked()
{
    QString file_dir = QFileDialog::getOpenFileName(this,"保存文件","模板.json","所有文件 (*)");
    QFile file(file_dir);
    if(file.open(QIODevice::ReadOnly))
    {
        QJsonDocument json_doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray json_arr = json_doc.array();
        for(int i = 0;i<json_arr.size();i++)
        {
            QJsonObject json_obj = json_arr[i].toObject();
            more_text_lineedit[i]->setText(json_obj["text"].toString());
            more_text_checkbox[i]->setChecked(json_obj["ischeck"].toBool());
        }
    }
}

