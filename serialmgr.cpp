#include "serialmgr.h"
#include <QThread>
std::shared_ptr<SerialMgr> SerialMgr::ser_mgr = nullptr;
SerialMgr::SerialMgr(QObject *parent)
    : QObject{parent}
{
    serialport = new QSerialPort(this);
    connect(serialport,&QSerialPort::readyRead,this,&SerialMgr::read_data);
}

SerialMgr::~SerialMgr()
{
    //delete ser_mgr;
}


void SerialMgr::get_now_port()
{
    QList<QSerialPortInfo> portlist = QSerialPortInfo::availablePorts();
    now_port.clear();
    for(int i = 0;i<portlist.size();i++)
    {
        now_port.push_back(portlist[i]);
    }
    emit sig_get_port(portlist);
}

std::shared_ptr<SerialMgr> SerialMgr::GetInstance()
{
    if(ser_mgr == nullptr)
    {
        ser_mgr = std::shared_ptr<SerialMgr>(new SerialMgr(nullptr));
    }
    return ser_mgr;
}

void SerialMgr::openSerial(QString serialNum, QString serialBaud, QString serialData, QString serialVerify, QString serialStop, QString serialStream)
{
    serialport->setPortName(serialNum);
    serialport->setBaudRate(serialBaud.toInt());
    serialport->setDataBits(QSerialPort::DataBits(serialData.toInt()));
    //停止位
    serialport->setStopBits(QSerialPort::StopBits(serialStop.toInt()));
    //配置校验
    if(serialVerify == "None")
    {
        serialport->setParity(QSerialPort::Parity::NoParity);
    }else if(serialVerify == "Even")
    {
        serialport->setParity(QSerialPort::Parity::EvenParity);
    }
    else if(serialVerify == "Mark")
    {
        serialport->setParity(QSerialPort::Parity::MarkParity);
    }else if(serialVerify == "odd")
    {
        serialport->setParity(QSerialPort::Parity::OddParity);
    }
    else if(serialVerify == "space")
    {
        serialport->setParity(QSerialPort::Parity::SpaceParity);
    }
    //配置流控
    if(serialStream == "No")
    {
        serialport->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    }
    if(serialStream == "Soft")
    {
        serialport->setFlowControl(QSerialPort::FlowControl::SoftwareControl);
    }
    if(serialStream == "Hard")
    {
        serialport->setFlowControl(QSerialPort::FlowControl::HardwareControl);
    }

    if(!serialport->open(QIODevice::ReadWrite))
    {
        QThread::msleep(50);
        emit connect_state(false);
        return;
    }
    emit connect_state(true);

}



void SerialMgr::closeSerial()
{
    serialport->close();
}


//发送
void SerialMgr::send_data(const QString &data)
{
    serialport->write(data.toUtf8());
}


//接收

void SerialMgr::read_data()
{
    QByteArray data = serialport->readAll();
    emit sig_read_data(data);
}
